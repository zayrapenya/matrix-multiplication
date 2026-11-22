package mm;

/** Dense kernels and input generator. Same algorithm and generator as Assignment 3 (C), ported to Java. */
public final class Kernels {
    private Kernels() {}

    /** splitmix64 -> doubles in [0,1); identical arithmetic to part3/src/matrix_parallel.c. */
    public static void fill(double[] m, long seed) {
        long s = seed;
        for (int i = 0; i < m.length; i++) {
            s += 0x9E3779B97F4A7C15L;
            long z = s;
            z = (z ^ (z >>> 30)) * 0xBF58476D1CE4E5B9L;
            z = (z ^ (z >>> 27)) * 0x94D049BB133111EBL;
            z = z ^ (z >>> 31);
            m[i] = (double) (z >>> 11) * (1.0 / 9007199254740992.0);
        }
    }

    public static long seedA(int n) { return 42L + 17L * n; }
    public static long seedB(int n) { return 42L + 17L * n + 1000003L; }

    /** i-k-j kernel for `rows` rows. a and c are offset by aOff / cOff (row-major, n columns). C rows are zeroed here. */
    public static void multiplyRows(double[] a, int aOff, double[] b, double[] c, int cOff, int rows, int n) {
        for (int i = 0; i < rows; i++) {
            int ci = cOff + i * n;
            java.util.Arrays.fill(c, ci, ci + n, 0.0);
            for (int k = 0; k < n; k++) {
                double aik = a[aOff + i * n + k];
                int bk = k * n;
                for (int j = 0; j < n; j++) c[ci + j] += aik * b[bk + j];
            }
        }
    }

    public static void multiplySeq(double[] a, double[] b, double[] c, int n) {
        multiplyRows(a, 0, b, c, 0, n, n);
    }

    /** Java threads, row blocks, same kernel. Thread creation/join is inside the call (as in Assignment 3). */
    public static void multiplyParallel(double[] a, double[] b, double[] c, int n, int p) {
        p = Math.min(Math.max(p, 1), n);
        Thread[] th = new Thread[p];
        for (int t = 0; t < p; t++) {
            final int r0 = (int) ((long) n * t / p), r1 = (int) ((long) n * (t + 1) / p);
            th[t] = new Thread(() -> multiplyRows(a, r0 * n, b, c, r0 * n, r1 - r0, n));
            th[t].start();
        }
        for (Thread t : th) {
            try { t.join(); } catch (InterruptedException e) { throw new RuntimeException(e); }
        }
    }

    /** Naive i-j-k reference, for correctness tests only. */
    public static void multiplyNaive(double[] a, double[] b, double[] c, int n) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                double s = 0.0;
                for (int k = 0; k < n; k++) s += a[i * n + k] * b[k * n + j];
                c[i * n + j] = s;
            }
    }

    public static double sum(double[] c) {
        double s = 0.0;
        for (double v : c) s += v;
        return s;
    }
}
