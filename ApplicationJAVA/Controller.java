import java.util.Scanner;

import cloudApplication.COAPclient;
import cloudApplication.COAPserver;
import cloudApplication.MQTTclient;
import database.ActuatorDA;
import database.QualityDA;
import database.TemperatureDA;

public class Controller{
	
	private static String commandList = "Command List:\n"
			+ "0 X \t-> set the temperature sampling period to X sec\n"
			+ "1 X \t-> set the water quality period to X sec\n"
			+ "2 X Y \t-> change the heat pump X to mode Y\n"
			+ "3 \t-> switch temperature autopilot on/off\n"
			+ "4 X Y \t-> change the water filter X to mode Y\n"
			+ "quit \t-> shutdown the application\n"
			+ "help \t-> show the command list";
	
	private static boolean tempAutopilot = false;
	private static float maxTemp = 21;
	private static float minTemp = 19;
	
	public static void main(String[] args) {
		//clean DB
		TemperatureDA.cleanSensorTemp();
		QualityDA.cleanSensorQuality();
		ActuatorDA.cleanActuator();
		
		//MQTTclient instantiation
		MQTTclient mqttclient = new MQTTclient();
		mqttclient.subscibeTo("temp");
		mqttclient.subscibeTo("quality");
		
		//COAPserver instantiation
		COAPserver.startServer();
		
		//autoPilot declaration
		TemperatureAutoPilot autoPilot = null;
		
		System.out.println(commandList);
		//user input loop
		Scanner sc = new Scanner(System.in);
		while(true) {
			System.out.print("type a new command \n");
			String[] userInput = sc.nextLine().split(" ");
			
			//show the command list
			if (userInput[0].equals("help")) {
				System.out.println(commandList);
            }
			
			//setting new temperature sampling frequency
			else if (userInput[0].equals("0")) {
				
            	System.out.println("setting temperature sampling period to " + userInput[1] + "sec");
                mqttclient.publish("temp_sampling", userInput[1]);
            }
			
			//setting new temperature sampling frequency
			else if (userInput[0].equals("1")) {
				
            	System.out.println("setting quality sampling period to " + userInput[1] + "sec");
                mqttclient.publish("quality_sampling", userInput[1]);
            }
            
            //setting heat_pump
            else if(userInput[0].equals("2")) {
            	if (!isTempAutopilot()) {
            		String pre = COAPclient.getHeatPump(Integer.parseInt(userInput[1]));
            		
            		if(COAPclient.setHeatPump(Integer.parseInt(userInput[1]), userInput[2])) {
            			String post = COAPclient.getHeatPump(Integer.parseInt(userInput[1]));
            			if(!pre.equals(post)) {
            				System.out.println("heat pump " + userInput[1] + ": " + pre + "->" + post);
            				
                		} else System.out.println("something went wrong in the mode change");
            			
            		} else System.out.println("command error");

            	} else System.out.println("deactivate autopilot first");
            }
            
			//switching temperature autopilot
            else if(userInput[0].equals("3")) {
            	System.out.println("switching temperature autopilot into " + !isTempAutopilot());
            	
            	setTempAutopilot(!isTempAutopilot());
            	if (isTempAutopilot()) {
            		autoPilot = new TemperatureAutoPilot(maxTemp, minTemp);
            		autoPilot.start();
            	}
            	else {
            		autoPilot.pause();
            	}
            }
			
			//setting water_filter
            else if(userInput[0].equals("4")) {
            	String pre = COAPclient.getWaterFilter(Integer.parseInt(userInput[1]));
            	if(COAPclient.setWaterFilter(Integer.parseInt(userInput[1]), userInput[2])) {
            		String post = COAPclient.getWaterFilter(Integer.parseInt(userInput[1]));
            		//this publish is only for simulation purpose
            		mqttclient.publish("quality_sampling", "-1");
            		System.out.println("water filter " + userInput[1] + ": " + pre + "->" + post);
            	}else System.out.println("command error");
            }
            
            else if(userInput[0].equals("quit")){
            	break;
            }
		}
		
		COAPserver.stopServer();
		sc.close();
		System.exit(0);
	}

	public static boolean isTempAutopilot() {
		return tempAutopilot;
	}

	public static void setTempAutopilot(boolean tempAutopilot) {
		Controller.tempAutopilot = tempAutopilot;
	}
}

