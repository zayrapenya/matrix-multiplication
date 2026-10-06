import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Arrays;

public class Benchmark {

    private static final int WARMUP_RUNS = 3;
    private static final int MEASURED_RUNS = 5;

    private static final int[] SIZES = {50, 100, 200, 300, 500};

    private static double runMultiplication(
            double[][] A,
            double[][] B) {

        double[][] C = MatrixMultiplication.multiply(A, B);

        double checksum = 0.0;

        for (double[] row : C) {
            for (double value : row) {
                checksum += value;
            }
        }

        if (Double.isInfinite(checksum)) {
            throw new RuntimeException("Invalid result.");
        }

        return checksum;
    }

    private static double[] benchmark(
            double[][] A,
            double[][] B) {

        for (int i = 0; i < WARMUP_RUNS; i++) {
            runMultiplication(A, B);
        }

        double[] times = new double[MEASURED_RUNS];

        for (int i = 0; i < MEASURED_RUNS; i++) {

            long start = System.nanoTime();

            runMultiplication(A, B);

            long end = System.nanoTime();

            times[i] = (end - start) / 1_000_000_000.0;
        }

        return times;
    }

    private static double median(double[] values) {

        double[] sorted = values.clone();
        Arrays.sort(sorted);

        int middle = sorted.length / 2;

        if (sorted.length % 2 == 0) {
            return (sorted[middle - 1] + sorted[middle]) / 2.0;
        }

        return sorted[middle];
    }

    private static double percentile(double[] values, double p) {

        double[] sorted = values.clone();
        Arrays.sort(sorted);

        double position = p * (sorted.length - 1);
        int lower = (int) Math.floor(position);
        int upper = (int) Math.ceil(position);

        if (lower == upper) {
            return sorted[lower];
        }

        double weight = position - lower;

        return sorted[lower]
                + weight * (sorted[upper] - sorted[lower]);
    }

    public static void main(String[] args) throws Exception {

        try (PrintWriter writer =
                new PrintWriter(new FileWriter(
                        "data/results/java_results.csv"))) {

            writer.println(
                "language,n,warmup_runs,measured_runs," +
                "median_seconds,iqr_seconds"
            );

            for (int n : SIZES) {

                System.out.println("Running Java n=" + n + "...");

                double[][] A =
                    MatrixIO.loadMatrix(
                        "data/input/n" + n + "_A.csv"
                    );

                double[][] B =
                    MatrixIO.loadMatrix(
                        "data/input/n" + n + "_B.csv"
                    );

                double[] times = benchmark(A, B);

                double median = median(times);

                double q1 = percentile(times, 0.25);
                double q3 = percentile(times, 0.75);

                double iqr = q3 - q1;

                writer.printf(
                    "Java,%d,%d,%d,%.9f,%.9f%n",
                    n,
                    WARMUP_RUNS,
                    MEASURED_RUNS,
                    median,
                    iqr
                );

                System.out.printf(
                    "  median=%.9fs IQR=%.9fs%n",
                    median,
                    iqr
                );
            }
        }

        System.out.println(
            "\nResults saved to data/results/java_results.csv"
        );
    }
}
