package cloudApplication;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

import database.ActuatorDA;

public class COAPclient {
	
	public static boolean setHeatPump(int idActuator, String mode) {
		String addressActuator = ActuatorDA.retrieveActuatorAddress(idActuator, "heat_pump");
		
		if(addressActuator != null) {
			
			CoapClient client = new CoapClient(
					"coap://[" + addressActuator + "]/res_heat_pump");
			
			CoapResponse response = client.put("mode=" + mode, MediaTypeRegistry.TEXT_PLAIN);
			
			//System.out.println(response.getPayload());
			return true;
			
		}else {
			System.out.println("Something went wrong with the command");
			return false;
		}
	}
	
	public static String getHeatPump(int idActuator) {
		String addressActuator = ActuatorDA.retrieveActuatorAddress(idActuator, "heat_pump");
		
		if(addressActuator != null) {
			
			CoapClient client = new CoapClient(
					"coap://[" + addressActuator + "]/res_heat_pump");
			
			CoapResponse response = client.get();
			
			return response.getResponseText();
			
		}else {
			System.out.println("Something went wrong with the command");
			return "";
		}
	}
	
	public static boolean setWaterFilter(int idActuator, String mode) {
		String addressActuator = ActuatorDA.retrieveActuatorAddress(idActuator, "water_filter");
		
		if(addressActuator != null) {
			
			CoapClient client = new CoapClient(
					"coap://[" + addressActuator + "]/res_water_filter");
			
			CoapResponse response = client.put("mode=" + mode, MediaTypeRegistry.TEXT_PLAIN);
			
			//System.out.println(response.getPayload());
			return true;
			
		}else {
			System.out.println("Something went wrong with the command");
			return false;
		}
	}
	
	public static String getWaterFilter(int idActuator) {
		String addressActuator = ActuatorDA.retrieveActuatorAddress(idActuator, "water_filter");
		
		if(addressActuator != null) {
			
			CoapClient client = new CoapClient(
					"coap://[" + addressActuator + "]/res_water_filter");
			
			CoapResponse response = client.get();
			
			return response.getResponseText();
			
		}else {
			System.out.println("Something went wrong with the command");
			return "";
		}
	}

}
