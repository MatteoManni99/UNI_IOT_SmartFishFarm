package cloudApplication;

import org.eclipse.californium.core.CoapServer;

import cloudApplication.resurces.CoapResRegistration;

public class COAPserver extends CoapServer {
	
	private static COAPserver server = null;

    public static void startServer(){
        if(server == null) server = new COAPserver();

        server.add(new CoapResRegistration("registrator"));
        server.start();
    }

	public static void stopServer() {
		server.stop();
		server.destroy();
	}
}

