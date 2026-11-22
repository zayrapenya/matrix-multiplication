#!/usr/bin/env bash
# Runs: correctness test, Java baselines (seq, threads) and the Hazelcast distributed runs (W local member JVMs).
# Override with env vars, e.g.  SIZES="500 1000" WORKERS="1 2" REPS=3 bash scripts/run_distributed.sh
# (no `set -u` / bash-4 features: macOS ships bash 3.2)
cd "$(dirname "$0")/.."
export LC_ALL=C
JAR=target/matmul-hz.jar
SIZES=${SIZES:-"500 1000 1500 2000"}
WORKERS=${WORKERS:-"1 2 4 8"}
THREADS=${THREADS:-"1 2 4 8"}
REPS=${REPS:-5}
WARMUP=${WARMUP:-3}              # JIT warm-up; Java needs more than C
BUDGET_S=${BUDGET_S:-300}        # per configuration (one driver process); exceeded -> killed and logged
MEMBER_XMX=${MEMBER_XMX:-1g}
DRIVER_XMX=${DRIVER_XMX:-3g}

JOPTS="--add-modules java.se --add-exports java.base/jdk.internal.ref=ALL-UNNAMED --add-opens java.base/java.lang=ALL-UNNAMED --add-opens java.base/sun.nio.ch=ALL-UNNAMED --add-opens java.management/sun.management=ALL-UNNAMED --add-opens jdk.management/com.sun.management.internal=ALL-UNNAMED -Djava.net.preferIPv4Stack=true"

[ -f "$JAR" ] || { echo "Missing $JAR - run: mvn -q package"; exit 1; }
mkdir -p results/logs results/tmp
bash scripts/collect_environment.sh > /dev/null

RAW=results/raw_benchmarks.csv
MEM=results/memory.csv
FAIL=results/failures.csv
echo "mode,n,workers,rep,end_to_end_s,upload_s,compute_s,assemble_s,max_fetch_s,max_kernel_s,mean_kernel_s,checksum" > $RAW
echo "mode,n,workers,max_total_rss_mib,max_driver_rss_mib,max_member_rss_mib" > $MEM
echo "mode,n,workers,reason" > $FAIL

MPIDS=""
start_members() {  # $1 = W
  MPIDS=""
  for i in $(seq 1 "$1"); do
    java $JOPTS -Xmx$MEMBER_XMX -cp $JAR mm.Member > results/logs/member_$1_$i.log 2>&1 &
    MPIDS="$MPIDS $!"
    sleep 1
  done
}
stop_members() {
  for p in $MPIDS; do kill -9 "$p" 2>/dev/null; done
  wait 2>/dev/null
  MPIDS=""
}

# Sampled RSS (every 0.2 s) of the driver (first pid) and the members. NOT a true peak: limitation stated in the report.
sampler() {  # $1 = outfile, rest = pids (driver first)
  out=$1; shift; idx=0
  while true; do
    idx=$((idx+1))
    for p in "$@"; do
      r=$(ps -o rss= -p "$p" 2>/dev/null | tr -d ' ')
      [ -n "$r" ] && echo "$idx,$p,$r" >> "$out"
    done
    sleep 0.2
  done
}
summarise_mem() {  # $1 = samples file, $2 = driver pid, $3.. = mode n workers
  f=$1; dp=$2; mode=$3; n=$4; w=$5
  awk -F, -v dp="$dp" -v mode="$mode" -v n="$n" -v w="$w" '
    { tot[$1]+=$3; if ($2==dp) { if ($3>d) d=$3 } else { if ($3>m) m=$3 } }
    END { t=0; for (i in tot) if (tot[i]>t) t=tot[i];
          printf "%s,%s,%s,%.1f,%.1f,%s\n", mode, n, w, t/1024, d/1024, (m>0 ? sprintf("%.1f", m/1024) : "") }' "$f" >> $MEM
}

run_config() {  # mode n workers javaargs...   (members must already be running for dist)
  mode=$1; n=$2; w=$3; shift 3
  tmp=results/tmp/out_${mode}_${n}_${w}.csv
  smp=results/tmp/rss_${mode}_${n}_${w}.txt
  : > "$smp"
  java $JOPTS -Xmx$DRIVER_XMX -cp $JAR mm.Bench "$@" > "$tmp" 2> results/logs/driver_${mode}_${n}_${w}.log &
  dpid=$!
  sampler "$smp" $dpid $MPIDS &
  spid=$!
  wait $dpid; rc=$?
  kill $spid 2>/dev/null; wait $spid 2>/dev/null
  if [ $rc -eq 0 ]; then
    cat "$tmp" >> $RAW
    summarise_mem "$smp" "$dpid" "$mode" "$n" "$w"
  else
    echo "$mode,$n,$w,exit_code_$rc(budget=${BUDGET_S}s;see_logs)" >> $FAIL
    echo "  !! $mode n=$n workers/threads=$w failed (exit $rc)"
  fi
}

echo "== correctness tests =="
for w in 1 2 3; do
  start_members $w
  java $JOPTS -Xmx$DRIVER_XMX -cp $JAR mm.Bench test $w | tee -a results/test_output.txt
  rc=${PIPESTATUS[0]}
  stop_members
  [ "$rc" -eq 0 ] || { echo "Correctness tests failed - aborting"; exit 1; }
done

for n in $SIZES; do
  echo "== n=$n baselines =="
  run_config seq "$n" 1 seq "$n" "$REPS" "$WARMUP" "$BUDGET_S"
  for p in $THREADS; do run_config par "$n" "$p" par "$n" "$p" "$REPS" "$WARMUP" "$BUDGET_S"; done
done

for n in $SIZES; do
  for w in $WORKERS; do
    echo "== n=$n distributed, workers=$w =="
    start_members "$w"
    run_config dist "$n" "$w" dist "$n" "$w" "$REPS" "$WARMUP" "$BUDGET_S"
    stop_members
  done
done
echo "Done -> $RAW"
