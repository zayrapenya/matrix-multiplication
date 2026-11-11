#!/usr/bin/env bash
# Run from anywhere. Overridable via env vars, e.g.  SIZES="500 1000" REPS=5 bash run_benchmarks.sh
set -uo pipefail
cd "$(dirname "$0")/.."
CC=${CC:-clang}
SIZES=${SIZES:-"200 500 800 1000 1500 2000 3000"}
THREADS=${THREADS:-"1 2 4 6 8 12 16"}
REPS=${REPS:-7}
WARMUP=${WARMUP:-2}
BUDGET_S=${BUDGET_S:-120}          # max wall time per configuration (process is killed beyond this)
VEC_STUDY=${VEC_STUDY:-0}          # 1 = also run the optional auto-vectorisation on/off study
VEC_SIZES=${VEC_SIZES:-"1000 2000"}
VEC_THREADS=${VEC_THREADS:-"1 4"}

mkdir -p bin results
CFLAGS="-O3 -Wall -Wextra -pthread"
$CC $CFLAGS src/matrix_parallel.c src/benchmark.c -o bin/benchmark || exit 1
$CC $CFLAGS src/matrix_parallel.c src/test_parallel.c -o bin/test_parallel -lm || exit 1
if [ "$VEC_STUDY" = "1" ]; then
  $CC $CFLAGS -fno-vectorize -fno-slp-vectorize src/matrix_parallel.c src/benchmark.c -o bin/benchmark_novec || exit 1
  # evidence that the loop is vectorised in the default build
  $CC $CFLAGS -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -c src/matrix_parallel.c -o /dev/null 2> results/vectorization_report.txt
fi

bash scripts/collect_environment.sh > /dev/null
./bin/test_parallel | tee results/test_output.txt || { echo "Correctness tests failed - aborting"; exit 1; }

OUT=results/raw_benchmarks.csv
FAIL=results/failures.csv
echo "method,n,threads,rep,time_s,checksum,peak_rss_kib,build" > "$OUT"
echo "method,n,threads,build,reason" > "$FAIL"

run() {  # bin method n threads label
  if ! "$1" "$2" "$3" "$4" "$REPS" "$WARMUP" "$BUDGET_S" "$5" >> "$OUT"; then
    echo "$2,$3,$4,$5,timeout_or_failure(budget=${BUDGET_S}s)" >> "$FAIL"
    echo "  !! $2 n=$3 p=$4 failed or exceeded budget"
  fi
}

for n in $SIZES; do
  echo "n=$n"
  run ./bin/benchmark seq "$n" 1 autovec
  for p in $THREADS; do run ./bin/benchmark par "$n" "$p" autovec; done
done

if [ "$VEC_STUDY" = "1" ]; then
  for n in $VEC_SIZES; do
    echo "vec study n=$n"
    run ./bin/benchmark_novec seq "$n" 1 novec
    for p in $VEC_THREADS; do run ./bin/benchmark_novec par "$n" "$p" novec; done
    for p in $VEC_THREADS; do run ./bin/benchmark par "$n" "$p" autovec_vecstudy; done
  done
fi
echo "Done -> $OUT"
