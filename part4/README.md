# Assignment 4 – Distributed matrix multiplication with Hazelcast (Big Data, ULPGC 2026-27)

Copy this folder into the repo as `part4/` (branch `assignment-4`).

## Environment label (important)
Everything runs **on one machine** (MacBook Air M1, 8 GB): the "workers" are Hazelcast member **JVM processes on 127.0.0.1**.
This studies framework, serialization and process overhead. It does **not** measure inter-machine network performance or
multi-machine memory capacity. The assignment allows local execution only if the instructor permits it: get that permission
and mention it in the report.

## Design
- **Worker** = one Hazelcast data member (own JVM) with a 1-thread executor, so the number of workers = number of compute threads. Members form a cluster over TCP-IP on localhost; the driver is a Hazelcast *client*, not a data member.
- **Strong scaling**: n fixed, workers W = 1, 2, 4, 8.
- **Partitioning**: A is split into W row blocks (one per worker); each block travels inside the task to its worker (`submitToMember`). B is uploaded to an `IMap` as 16 row panels (no backups) and every task reads all panels. C blocks are returned as task results and assembled at the driver.
- Same i–k–j kernel and same splitmix64 inputs/seeds as Assignment 3 (verified: same checksums as the C version for n = 100 and 500).
- **Baselines in the same language and JVM settings**: Java sequential kernel (`seq`) and Java threads (`par`). The C results from Assignment 3 are context only (different language).

## Timing boundaries (never mix them unqualified)
- `seq`/`par`: kernel only (inputs generated and C allocated outside; kernel zeroes C; `par` includes thread create/join).
- `dist` `end_to_end_s` = upload B + compute phase + assemble C at the driver. These three phases are consecutive, so they add up.
- Inside the compute phase, `max_fetch_s` / `max_kernel_s` / `mean_kernel_s` are measured **inside the tasks** and overlap with the compute phase wall time (and with each other, across workers): do not add them.
- Warm-up runs (3, JIT) are excluded; 5 measured repetitions; each repetition re-uploads B (map cleared between runs).
- Memory: **sampled RSS** (`ps`, every 0.2 s) of driver and members – not a true peak.
- Data transferred: only an analytic estimate, `(3 + W)·8n²` bytes (B upload + A out + C back + B read by each of W tasks); it is computed, not measured. Loopback traffic is not network performance.

## Requirements
JDK 11+ (`java -version`), Maven (`brew install maven`), Python 3 with pandas + matplotlib. Hazelcast 5.3.8 (change `hazelcast.version` in `pom.xml` if your course uses another 5.x version and record it).

## Run (from the repo root)
```bash
cd part4
mvn -q package                                   # builds target/matmul-hz.jar
SIZES="500" WORKERS="1 2" THREADS="1 2" REPS=3 bash scripts/run_distributed.sh   # smoke test
bash scripts/run_distributed.sh                   # full run
python3 scripts/analyze_results.py
```
Close other apps first and plug in the charger. With 8 GB RAM, W = 8 is the memory limit; if it fails, reduce `WORKERS`, `SIZES` or `MEMBER_XMX`, and report the failure as a result (it is logged in `results/failures.csv`).

## Files
- `src/main/java/mm/`: `Kernels` (kernels, generator), `RowBlockTask` (work on a member), `Dist` (driver, tests), `Member`, `Bench`, `Common` (Hazelcast config).
- `scripts/run_distributed.sh`, `scripts/analyze_results.py`, `scripts/collect_environment.sh`.
- Outputs: `results/raw_benchmarks.csv`, `memory.csv`, `summary.csv`, `scaling.csv`, `environment.txt`, `test_output.txt`, `failures.csv`, `logs/`; `figures/`.

## Generative AI disclosure
Remember to state in the report which AI tool was used, for what (planning, code drafting, debugging) and which parts, as the course policy requires.
