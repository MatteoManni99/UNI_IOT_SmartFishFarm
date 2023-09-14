package cloudApplication.resurces;

import java.sql.SQLException;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.JSONObject;

import database.ActuatorDA;

public class CoapResRegistration extends CoapResource{

	public CoapResRegistration(String name) {
		super(name);
	}
	
	//POST HANDLER
	public void handlePOST(CoapExchange exchange) {
		
		Response response = new Response(ResponseCode.CONTENT);
		
		JSONObject json = new JSONObject(new String(exchange.getRequestPayload()));
		
		int actuator_id = json.getInt("actuator_id");
		String type = json.getString("type");
		String address = exchange.getSourceAddress().toString().substring(1);
		
		try {
			if(!ActuatorDA.isPresent(actuator_id, type, address)) {
				ActuatorDA.insertActuator(actuator_id, type, address);
			}
			response.setPayload("REGIST");
		} catch (SQLException e) {
			response.setPayload("ERROR");
		}
		
		exchange.respond(response);
	}
}
