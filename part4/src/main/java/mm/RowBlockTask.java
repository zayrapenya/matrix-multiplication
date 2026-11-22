package mm;

import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.HazelcastInstanceAware;
import com.hazelcast.map.IMap;

import java.io.Serializable;
import java.util.concurrent.Callable;

/**
 * Computes C[block] = A[block] * B on a member. The A row block travels inside the task (driver -> member);
 * B is read from the distributed map "B" (16 row panels, spread over the members); the C block is
 * returned to the driver as the task result.
 */
public class RowBlockTask implements Callable<RowBlockTask.Result>, Serializable, HazelcastInstanceAware {
    private static final long serialVersionUID = 1L;

    private final String runId;
    private final int n, rows, nPanels;
    private final double[] aBlock;
    private transient HazelcastInstance hz;

    public RowBlockTask(String runId, int n, int rows, int nPanels, double[] aBlock) {
        this.runId = runId; this.n = n; this.rows = rows; this.nPanels = nPanels; this.aBlock = aBlock;
    }

    @Override public void setHazelcastInstance(HazelcastInstance hz) { this.hz = hz; }

    public static final class Result implements Serializable {
        private static final long serialVersionUID = 1L;
        public final double[] c;
        public final double fetchS, kernelS;
        public Result(double[] c, double fetchS, double kernelS) { this.c = c; this.fetchS = fetchS; this.kernelS = kernelS; }
    }

    @Override
    public Result call() {
        long t0 = System.nanoTime();
        IMap<String, double[]> map = hz.getMap(Common.B_MAP);
        double[] b = new double[n * n];
        for (int k = 0; k < nPanels; k++) {
            double[] panel = map.get(runId + "/" + k);
            int r0 = (int) ((long) n * k / nPanels);
            System.arraycopy(panel, 0, b, r0 * n, panel.length);
        }
        long t1 = System.nanoTime();
        double[] c = new double[rows * n];
        Kernels.multiplyRows(aBlock, 0, b, c, 0, rows, n);
        long t2 = System.nanoTime();
        return new Result(c, (t1 - t0) * 1e-9, (t2 - t1) * 1e-9);
    }
}
