package mm;

import com.hazelcast.client.config.ClientConfig;
import com.hazelcast.config.Config;
import com.hazelcast.config.JoinConfig;
import com.hazelcast.config.NetworkConfig;
import com.hazelcast.config.TcpIpConfig;

/** Hazelcast configuration shared by members and the driver (all on 127.0.0.1). */
public final class Common {
    private Common() {}

    public static final String CLUSTER = "mm-cluster";
    public static final String EXECUTOR = "mm";
    public static final String B_MAP = "B";
    public static final int B_PANELS = 16;
    public static final int BASE_PORT = 5701;

    public static Config memberConfig() {
        Config c = new Config();
        c.setClusterName(CLUSTER);
        c.setProperty("hazelcast.logging.type", "none");
        c.setProperty("hazelcast.phone.home.enabled", "false");
        NetworkConfig nc = c.getNetworkConfig();
        nc.setPort(BASE_PORT).setPortAutoIncrement(true).setPortCount(50);
        nc.getInterfaces().setEnabled(true).addInterface("127.0.0.1");
        JoinConfig join = nc.getJoin();
        join.getMulticastConfig().setEnabled(false);
        join.getAutoDetectionConfig().setEnabled(false);
        TcpIpConfig tcp = join.getTcpIpConfig().setEnabled(true);
        for (int p = BASE_PORT; p < BASE_PORT + 16; p++) tcp.addMember("127.0.0.1:" + p);
        // One compute thread per member: a "worker" = one JVM process with one compute thread.
        c.getExecutorConfig(EXECUTOR).setPoolSize(1);
        // No backup copies of B (halves memory; no fault tolerance needed for this benchmark).
        c.getMapConfig(B_MAP).setBackupCount(0);
        return c;
    }

    public static ClientConfig clientConfig() {
        ClientConfig cc = new ClientConfig();
        cc.setClusterName(CLUSTER);
        cc.setProperty("hazelcast.logging.type", "none");
        cc.getNetworkConfig().addAddress("127.0.0.1:" + BASE_PORT);
        cc.getConnectionStrategyConfig().getConnectionRetryConfig().setClusterConnectTimeoutMillis(60000);
        return cc;
    }
}
