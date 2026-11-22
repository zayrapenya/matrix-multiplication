package mm;

import com.hazelcast.core.Hazelcast;

/** One worker: a Hazelcast data member (own JVM process). Started by scripts/run_distributed.sh. */
public final class Member {
    public static void main(String[] args) throws Exception {
        Hazelcast.newHazelcastInstance(Common.memberConfig());
        System.out.println("member started");
        Thread.currentThread().join();   // run until killed
    }
}
