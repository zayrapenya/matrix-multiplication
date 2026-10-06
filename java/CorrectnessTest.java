public class CorrectnessTest {

    private static void assertMatrixEquals(
            double[][] actual,
            double[][] expected,
            double tolerance) {

        if (actual.length != expected.length) {
            throw new AssertionError("Different number of rows.");
        }

        for (int i = 0; i < expected.length; i++) {

            if (actual[i].length != expected[i].length) {
                throw new AssertionError("Different number of columns.");
            }

            for (int j = 0; j < expected[i].length; j++) {

                if (Math.abs(actual[i][j] - expected[i][j]) > tolerance) {
                    throw new AssertionError(
                        "Mismatch at (" + i + "," + j + ")"
                    );
                }
            }
        }
    }

    public static void main(String[] args) {

        double[][] A = {
            {1.0, 2.0},
            {3.0, 4.0}
        };

        double[][] B = {
            {5.0, 6.0},
            {7.0, 8.0}
        };

        double[][] expected = {
            {19.0, 22.0},
            {43.0, 50.0}
        };

        double[][] result = MatrixMultiplication.multiply(A, B);

        assertMatrixEquals(result, expected, 1e-9);

        System.out.println("All Java correctness tests passed.");
    }
}
