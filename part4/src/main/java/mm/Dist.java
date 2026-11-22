package mm;

import com.hazelcast.client.HazelcastClient;
import com.hazelcast.cluster.Member;
import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.IExecutorService;
import com.hazelcast.map.IMap;

import java.util.*;
import java.util.concurrent.Future;

/** Driver side of the distributed multiplication (Hazelcast client, not a data member). */
public final class Dist {
    private Dist() {}

    public static final class Run {
        double upload, compute, assemble, total, maxFetch, maxKernel, meanKernel, checksum;
        double[] c;
    }

    public static HazelcastInstance connect(int workers) throws Exception {
        HazelcastInstance client = HazelcastClient.newHazelcastClient(Common.clientConfig());
        long deadline = System.nanoTime() + 120_000_000_000L;
        while (client.getCluster().getMembers().size() < workers) {
            if (System.nanoTime() > deadline) {
                System.err.println("expected " + workers + " members, found " + client.getCluster().getMembers().size());
                System.exit(4);
            }
            Thread.sleep(200);
        }
        if (client.getCluster().getMembers().size() != workers) {
            System.err.println("cluster has " + client.getCluster().getMembers().size() + " members, expected " + workers);
            System.exit(4);
        }
        return client;
    }

    /** One complete distributed multiplication. Phases at the driver are consecutive, so they add up to `total`. */
    public static Run runOnce(HazelcastInstance hz, double[] A, double[] B, int n, int workers, String runId) throws Exception {
        List<Member> members = new ArrayList<>(hz.getCluster().getMembers());
        IMap<String, double[]> bMap = hz.getMap(Common.B_MAP);
        IExecutorService ex = hz.getExecutorService(Common.EXECUTOR);
        int nPanels = Math.min(Common.B_PANELS, n);
        int blocks = workers;                       // one row block of A per worker
        Run r = new Run();

        long t0 = System.nanoTime();
        // phase 1: slice B into panels and upload them to the distributed map
        Map<String, double[]> panels = new HashMap<>();
        for (int k = 0; k < nPanels; k++) {
            int r0 = (int) ((long) n * k / nPanels), r1 = (int) ((long) n * (k + 1) / nPanels);
            panels.put(runId + "/" + k, Arrays.copyOfRange(B, r0 * n, r1 * n));
        }
        bMap.putAll(panels);
        long t1 = System.nanoTime();

        // phase 2: ship A row blocks inside the tasks, compute on the members, receive the C blocks
        List<Future<RowBlockTask.Result>> futures = new ArrayList<>();
        int[] start = new int[blocks], len = new int[blocks];
        for (int t = 0; t < blocks; t++) {
            int r0 = (int) ((long) n * t / blocks), r1 = (int) ((long) n * (t + 1) / blocks);
            start[t] = r0; len[t] = r1 - r0;
            if (len[t] == 0) { futures.add(null); continue; }
            RowBlockTask task = new RowBlockTask(runId, n, len[t], nPanels, Arrays.copyOfRange(A, r0 * n, r1 * n));
            futures.add(ex.submitToMember(task, members.get(t % members.size())));
        }
        List<RowBlockTask.Result> results = new ArrayList<>();
        for (Future<RowBlockTask.Result> f : futures) results.add(f == null ? null : f.get());
        long t2 = System.nanoTime();

        // phase 3: assemble C at the driver
        double[] C = new double[n * n];
        double sumK = 0; int cnt = 0;
        for (int t = 0; t < blocks; t++) {
            RowBlockTask.Result res = results.get(t);
            if (res == null) continue;
            System.arraycopy(res.c, 0, C, start[t] * n, res.c.length);
            r.maxFetch = Math.max(r.maxFetch, res.fetchS);
            r.maxKernel = Math.max(r.maxKernel, res.kernelS);
            sumK += res.kernelS; cnt++;
        }
        long t3 = System.nanoTime();

        r.upload = (t1 - t0) * 1e-9; r.compute = (t2 - t1) * 1e-9; r.assemble = (t3 - t2) * 1e-9; r.total = (t3 - t0) * 1e-9;
        r.meanKernel = cnt == 0 ? 0 : sumK / cnt;
        r.c = C;
        r.checksum = Kernels.sum(C);
        bMap.clear();                               // outside the timed region; frees memory between runs
        return r;
    }

    public static void bench(int n, int workers, int reps, int warmup) throws Exception {
        double[] A = new double[n * n], B = new double[n * n];
        Kernels.fill(A, Kernels.seedA(n));
        Kernels.fill(B, Kernels.seedB(n));
        HazelcastInstance hz = connect(workers);
        try {
            for (int i = -warmup; i < reps; i++) {
                Run r = runOnce(hz, A, B, n, workers, "r" + (i + warmup));
                if (i >= 0) {
                    System.out.println(String.format(Locale.ROOT, "dist,%d,%d,%d,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.9f,%.10e",
                            n, workers, i + 1, r.total, r.upload, r.compute, r.assemble, r.maxFetch, r.maxKernel, r.meanKernel, r.checksum));
                }
            }
        } finally {
            hz.shutdown();
        }
    }

    /** Correctness tests: distributed result against the sequential kernel (and the naive reference). */
    public static int test(int workers) throws Exception {
        int[] sizes = {1, 2, 3, 7, 16, 33, 100};
        int failures = 0;
        double worstNaive = 0, worstDist = 0;
        HazelcastInstance hz = connect(workers);
        try {
            for (int n : sizes) {
                double[] A = new double[n * n], B = new double[n * n], Cs = new double[n * n], Cn = new double[n * n];
                Kernels.fill(A, Kernels.seedA(n));
                Kernels.fill(B, Kernels.seedB(n));
                Kernels.multiplySeq(A, B, Cs, n);
                Kernels.multiplyNaive(A, B, Cn, n);
                for (int i = 0; i < Cs.length; i++)
                    worstNaive = Math.max(worstNaive, Math.abs(Cs[i] - Cn[i]) / Math.max(1.0, Math.abs(Cn[i])));
                double[] Cp = new double[n * n];
                Kernels.multiplyParallel(A, B, Cp, n, 3);
                if (!Arrays.equals(Cs, Cp)) { failures++; System.out.println("FAIL threads != seq n=" + n); }

                Run r = runOnce(hz, A, B, n, workers, "t" + n);
                for (int i = 0; i < Cs.length; i++) worstDist = Math.max(worstDist, Math.abs(Cs[i] - r.c[i]));
                if (!Arrays.equals(Cs, r.c)) { failures++; System.out.println("FAIL distributed != seq n=" + n + " workers=" + workers); }

                double[] I = new double[n * n], Z = new double[n * n];
                for (int i = 0; i < n; i++) I[i * n + i] = 1.0;
                if (!Arrays.equals(A, runOnce(hz, A, I, n, workers, "i" + n).c)) { failures++; System.out.println("FAIL A*I n=" + n); }
                for (double v : runOnce(hz, A, Z, n, workers, "z" + n).c) if (v != 0.0) { failures++; System.out.println("FAIL A*0 n=" + n); break; }
            }
        } finally {
            hz.shutdown();
        }
        System.out.println(String.format(Locale.ROOT, "workers=%d  max rel. error seq vs naive: %.3e  max abs diff dist vs seq: %.3e", workers, worstNaive, worstDist));
        System.out.println(failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED (" + failures + ")");
        return failures;
    }
}
