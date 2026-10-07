#!/bin/bash

mkdir -p data/results/memory

echo "language,n,max_rss_bytes,max_rss_mib" > data/results/memory/python_memory.csv

for n in 50 100 200 300 500
do
    echo "Measuring Python n=$n..."

    /usr/bin/time -l python3 python/memory_benchmark.py "$n" \
        > /tmp/memory_stdout.txt \
        2> /tmp/memory_stderr.txt

    rss=$(grep "maximum resident set size" /tmp/memory_stderr.txt | awk '{print $1}')

    if [ -z "$rss" ]; then
        echo "ERROR: no se pudo obtener RSS para n=$n"
        cat /tmp/memory_stderr.txt
        exit 1
    fi

    mib=$(python3 -c "print(f'{$rss / (1024 * 1024):.6f}')")

    echo "Python,$n,$rss,$mib" >> data/results/memory/python_memory.csv

    echo "  RSS=$rss bytes ($mib MiB)"
done

echo
cat data/results/memory/python_memory.csv
