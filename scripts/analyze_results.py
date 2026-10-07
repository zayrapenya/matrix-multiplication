import csv
import os
import matplotlib.pyplot as plt


RESULTS_FILE = "data/results/combined_results.csv"
FIGURES_DIR = "report/figures"


def load_results():
    rows = []

    with open(RESULTS_FILE, newline="") as f:
        reader = csv.DictReader(f)

        for row in reader:
            rows.append({
                "language": row["language"],
                "n": int(row["n"]),
                "time": float(row["median_seconds"]),
                "memory": float(row["max_rss_mib"]),
            })

    return rows


def plot_metric(rows, metric, ylabel, filename, title, log_scale=False):
    languages = ["Python", "Java", "C"]

    plt.figure(figsize=(8, 5))

    for language in languages:
        data = [r for r in rows if r["language"] == language]
        data.sort(key=lambda r: r["n"])

        x = [r["n"] for r in data]
        y = [r[metric] for r in data]

        plt.plot(x, y, marker="o", label=language)

    if log_scale:
        plt.yscale("log")

    plt.xlabel("Matrix size (n)")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()

    plt.savefig(
        os.path.join(FIGURES_DIR, filename),
        dpi=300,
    )

    plt.close()


def main():
    os.makedirs(FIGURES_DIR, exist_ok=True)

    rows = load_results()

    plot_metric(
        rows,
        "time",
        "Median execution time (seconds)",
        "execution_time.png",
        "Matrix multiplication execution time",
    )

    plot_metric(
        rows,
        "time",
        "Median execution time (seconds)",
        "execution_time_log.png",
        "Matrix multiplication execution time (log scale)",
        log_scale=True,
    )

    plot_metric(
        rows,
        "memory",
        "Maximum resident set size (MiB)",
        "memory_usage.png",
        "Matrix multiplication memory usage",
    )

    plot_metric(
        rows,
        "memory",
        "Maximum resident set size (MiB)",
        "memory_usage_log.png",
        "Matrix multiplication memory usage (log scale)",
        log_scale=True,
    )

    print("Figures generated successfully.")
    print(f"Output directory: {FIGURES_DIR}")


if __name__ == "__main__":
    main()
