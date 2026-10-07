public class MemoryBenchmark {

    private static final int WARMUP_RUNS = 3;

    private static void runMultiplication(double[][] A, double[][] B) {
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
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("Usage: java MemoryBenchmark <n>");
            System.exit(1);
        }

        int n = Integer.parseInt(args[0]);

        double[][] A = MatrixIO.loadMatrix(
                "data/input/n" + n + "_A.csv"
        );

        double[][] B = MatrixIO.loadMatrix(
                "data/input/n" + n + "_B.csv"
        );

        for (int i = 0; i < WARMUP_RUNS; i++) {
            runMultiplication(A, B);
        }

        runMultiplication(A, B);
    }
}
