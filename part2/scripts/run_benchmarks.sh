#!/bin/bash
set -e

cd "$(dirname "$0")/../.."

BIN="./part2/benchmark"
OUT="part2/results/raw_benchmarks.csv"

echo "method,n,zero_pct,rep,elapsed_seconds,conversion_seconds,resident_memory_bytes,nnz_A,nnz_B,nnz_C,checksum" > "$OUT"

run_case() {
    method="$1"
    n="$2"
    zero_pct="$3"

    echo "Ejecutando: método=$method, n=$n, ceros=$zero_pct%"

    METHOD="$method" "$BIN" "$n" "$zero_pct" 5 >> "$OUT"
}

# Parte A: comparar los métodos densos con entradas sin ceros.
for n in 100 200 500 800; do
    for method in basic reordered blocked; do
        run_case "$method" "$n" 0
    done
done

# Parte B: comparar baseline denso y CSR con las mismas entradas dispersas.
for n in 100 200 500 800; do
    for zero_pct in 50 90 95 99; do
        for method in basic csr; do
            run_case "$method" "$n" "$zero_pct"
        done
    done
done

echo "Experimentos terminados."
echo "Resultados guardados en: $OUT"
