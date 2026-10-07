#!/bin/bash

mkdir -p data/results/memory

echo "language,n,max_rss_bytes,max_rss_mib" > data/results/memory/java_memory.csv

for n in 50 100 200 300 500
do
    echo "Measuring Java n=$n..."

    /usr/bin/time -l java -cp java MemoryBenchmark "$n" \
        > /tmp/java_memory_stdout.txt \
        2> /tmp/java_memory_stderr.txt

    rss=$(grep "maximum resident set size" /tmp/java_memory_stderr.txt | awk '{print $1}')

    if [ -z "$rss" ]; then
        echo "ERROR: no se pudo obtener RSS para n=$n"
        cat /tmp/java_memory_stderr.txt
        exit 1
    fi

    mib=$(awk -v rss="$rss" 'BEGIN {printf "%.6f", rss/(1024*1024)}')

    echo "Java,$n,$rss,$mib" >> data/results/memory/java_memory.csv

    echo "  RSS=$rss bytes ($mib MiB)"
done

echo
cat data/results/memory/java_memory.csv
