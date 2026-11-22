package mm;

import java.util.Locale;

/**
 * Usage:
 *   test  <workers>
 *   seq   <n> <reps> <warmup> <budget_s>
 *   par   <n> <threads> <reps> <warmup> <budget_s>
 *   dist  <n> <workers> <reps> <warmup> <budget_s>
 * CSV columns: mode,n,workers,rep,end_to_end_s,upload_s,compute_s,assemble_s,max_fetch_s,max_kernel_s,mean_kernel_s,checksum
 * For seq/par only the kernel time (end_to_end_s) is reported: inputs generated and C allocated outside the timer;
 * the kernel zeroes C inside the timer; for par, thread creation/join is inside the timer.
 */
public final class Bench {
    public static void main(String[] a) throws Exception {
        String mode = a[0];
        if (mode.equals("test")) { System.exit(Dist.test(Integer.parseInt(a[1])) == 0 ? 0 : 1); }

        int n = Integer.parseInt(a[1]);
        int p = Integer.parseInt(a[2]);                       // threads (par) or workers (dist); ignored by seq
        int reps, warm, budget;
        if (mode.equals("seq")) { reps = Integer.parseInt(a[2]); warm = Integer.parseInt(a[3]); budget = Integer.parseInt(a[4]); p = 1; }
        else { reps = Integer.parseInt(a[3]); warm = Integer.parseInt(a[4]); budget = Integer.parseInt(a[5]); }

        final int budgetS = budget;
        if (budgetS > 0) {
            Thread wd = new Thread(() -> {
                try { Thread.sleep(budgetS * 1000L); } catch (InterruptedException e) { return; }
                System.err.println("budget of " + budgetS + " s exceeded");
                Runtime.getRuntime().halt(3);
            });
            wd.setDaemon(true); wd.start();
        }

        if (mode.equals("dist")) { Dist.bench(n, p, reps, warm); return; }

        double[] A = new double[n * n], B = new double[n * n], C = new double[n * n];
        Kernels.fill(A, Kernels.seedA(n));
        Kernels.fill(B, Kernels.seedB(n));
        for (int i = -warm; i < reps; i++) {
            long t0 = System.nanoTime();
            if (mode.equals("par")) Kernels.multiplyParallel(A, B, C, n, p); else Kernels.multiplySeq(A, B, C, n);
            long t1 = System.nanoTime();
            double cs = Kernels.sum(C);                       // consume the result
            if (i >= 0)
                System.out.println(String.format(Locale.ROOT, "%s,%d,%d,%d,%.9f,,,,,,,%.10e", mode, n, p, i + 1, (t1 - t0) * 1e-9, cs));
        }
    }
}
