import cloudApplication.COAPclient;
import database.TemperatureDA;

public class TemperatureAutoPilot extends Thread {
	private boolean whileCondition = true;
	private float maxTemp;
	private float minTemp;
	
	public TemperatureAutoPilot(float maxTemp_, float minTemp_) {
		maxTemp = maxTemp_;
		minTemp = minTemp_;
	}
	
	public void pause() {
		whileCondition = false;
	}
	
	@Override
	public void run(){
		int idActuator = 1;
		
    	String lastMode = COAPclient.getHeatPump(idActuator);
    	
    	if(!lastMode.contentEquals("OFF")){
    		pause();
        	System.out.println("error: heat pump is not in OFF mode");
        	Controller.setTempAutopilot(false);
        }else System.out.println("tempAutopilot is running...");
    	
    	while (whileCondition) {
    		try {
    	
                float lastTemp = TemperatureDA.pickLastTemp();
                System.out.println(lastTemp);
                
        		if(lastTemp == -1000) {
        			System.out.println("error, no temperature record in the database");
        		}else if (lastTemp > maxTemp) {
        			//the temperature is hot
        			if(!lastMode.contentEquals("ON_COLD")) {
        				System.out.println("autopilot is enabling the heatpump cold mode");
        				COAPclient.setHeatPump(idActuator, "ON_COLD");
        				lastMode = COAPclient.getHeatPump(idActuator);
        			}
        		}else if (lastTemp < minTemp) {
        			//the temperature is cold
        			if(!lastMode.contentEquals("ON_HOT")) {
        				System.out.println("autopilot is enabling the heatpump hot mode");
        				COAPclient.setHeatPump(idActuator, "ON_HOT");
        				lastMode = COAPclient.getHeatPump(idActuator);
        			}
        		}else {
        			//the temperature is right
        			if(!lastMode.contentEquals("OFF")) {
        				System.out.println("autopilot shutdown the heatpump");
        				COAPclient.setHeatPump(idActuator, "OFF");
        				lastMode = COAPclient.getHeatPump(idActuator);
        			}
        		}
        		
				Thread.sleep(2000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    	}
    	
        System.out.println("tempAutopilot has been stopped.");
    }
}
