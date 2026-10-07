#!/bin/bash

mkdir -p data/results/memory

echo "language,n,max_rss_bytes,max_rss_mib" > data/results/memory/c_memory.csv

for n in 50 100 200 300 500
do
    echo "Measuring C n=$n..."

    /usr/bin/time -l ./c/memory_benchmark "$n" \
        > /tmp/c_memory_stdout.txt \
        2> /tmp/c_memory_stderr.txt

    rss=$(grep "maximum resident set size" /tmp/c_memory_stderr.txt | awk '{print $1}')

    if [ -z "$rss" ]; then
        echo "ERROR: no se pudo obtener RSS para n=$n"
        cat /tmp/c_memory_stderr.txt
        exit 1
    fi

    mib=$(awk -v rss="$rss" 'BEGIN {printf "%.6f", rss/(1024*1024)}')

    echo "C,$n,$rss,$mib" >> data/results/memory/c_memory.csv

    echo "  RSS=$rss bytes ($mib MiB)"
done

echo
cat data/results/memory/c_memory.csv
