import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

public class MatrixIO {

    public static double[][] loadMatrix(String path) throws Exception {

        List<double[]> rows = new ArrayList<>();

        try (BufferedReader reader = new BufferedReader(new FileReader(path))) {

            String line;

            while ((line = reader.readLine()) != null) {

                String[] values = line.split(",");
                double[] row = new double[values.length];

                for (int i = 0; i < values.length; i++) {
                    row[i] = Double.parseDouble(values[i]);
                }

                rows.add(row);
            }
        }

        double[][] matrix = new double[rows.size()][];

        for (int i = 0; i < rows.size(); i++) {
            matrix[i] = rows.get(i);
        }

        return matrix;
    }
}
