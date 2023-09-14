package cloudApplication;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.MqttPersistenceException;
import org.eclipse.paho.client.mqttv3.MqttSecurityException;
import org.json.JSONObject;

import database.QualityDA;
import database.TemperatureDA;

public class MQTTclient implements MqttCallback{
	
	private MqttClient mqttClient = null;
    private String broker = "tcp://127.0.0.1:1883";
    private String clientId = "controller";
	
    //constructor
	public MQTTclient() {
		try {
			mqttClient =  new MqttClient(broker,clientId);
		} catch (MqttException e) {
			e.printStackTrace();
		}
		mqttClient.setCallback( this );
		try {
			mqttClient.connect();
		} catch (MqttSecurityException e) {
			e.printStackTrace();
		} catch (MqttException e) {
			e.printStackTrace();
		}
	}
	
	
	public void subscibeTo(String topic) {
		try {
			mqttClient.subscribe(topic);
		} catch (MqttException e) {
			e.printStackTrace();
		}
	}

	public void connectionLost(Throwable cause) {
		System.out.println("connection lost: " + cause);
	}

	public void messageArrived(String topic, MqttMessage message) throws Exception {
		
		JSONObject json = new JSONObject(new String(message.getPayload()));
		
		if(topic.equals("temp")){
			TemperatureDA.insertTemperature(json.getInt("sensor_id"), json.getInt("temperature"));
		}else if(topic.equals("quality")){
			QualityDA.insertQuality(json.getInt("sensor_id"), json.getInt("quality"));
		}
	}

	public void deliveryComplete(IMqttDeliveryToken token) {

	}
	
	public void publish(String topic, String samplingPeriod){
		JSONObject payload = new JSONObject();
		payload.put("sampling_period", Integer.parseInt(samplingPeriod));
		MqttMessage message = new MqttMessage(payload.toString().getBytes());
		
        try {
        	mqttClient.publish(topic, message);
		} catch (MqttPersistenceException e) {
			e.printStackTrace();
		} catch (MqttException e) {
			e.printStackTrace();
		}
	}

}
