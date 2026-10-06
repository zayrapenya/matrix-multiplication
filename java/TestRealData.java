public class TestRealData {

    public static void main(String[] args) throws Exception {

        double[][] A = MatrixIO.loadMatrix("data/input/n50_A.csv");
        double[][] B = MatrixIO.loadMatrix("data/input/n50_B.csv");

        if (A.length != 50 || B.length != 50) {
            throw new AssertionError("Unexpected matrix size.");
        }

        double[][] C = MatrixMultiplication.multiply(A, B);

        double checksum = 0.0;

        for (double[] row : C) {
            for (double value : row) {
                checksum += value;
            }
        }

        System.out.println("Java real data test passed.");
        System.out.println("Matrix size: 50x50");
        System.out.printf("Result checksum: %.12f%n", checksum);
    }
}
