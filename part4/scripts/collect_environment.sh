#!/usr/bin/env bash
OUT="$(dirname "$0")/../results/environment.txt"
{
  echo "date: $(date -u +%FT%TZ)"
  echo "os: $(uname -srm)"
  if [ "$(uname)" = "Darwin" ]; then
    sw_vers
    echo "cpu: $(sysctl -n machdep.cpu.brand_string)"
    echo "physical_cores: $(sysctl -n hw.physicalcpu)"
    echo "logical_processors: $(sysctl -n hw.logicalcpu)"
    for l in 0 1; do echo "perflevel$l: $(sysctl -n hw.perflevel$l.name 2>/dev/null) physical=$(sysctl -n hw.perflevel$l.physicalcpu 2>/dev/null)"; done
    echo "ram_bytes: $(sysctl -n hw.memsize)"
    echo "power: $(pmset -g batt 2>/dev/null | head -1)"
  else
    lscpu | grep -E 'Model name|^CPU\(s\)|Thread'; grep MemTotal /proc/meminfo
  fi
  echo "java: $(java -version 2>&1 | head -1)"
  echo "hazelcast: $(grep -o '<hazelcast.version>[^<]*' "$(dirname "$0")/../pom.xml" | sed 's/<hazelcast.version>//')"
  echo "maven: $(mvn -v 2>/dev/null | head -1)"
  echo "python: $(python3 --version 2>&1)"
} > "$OUT"
cat "$OUT"
