package database;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class TemperatureDA {
	
	//the sensor capture the value of temperature as (actual temperature)*10, so we have to divide by 10
	public static float fromIntToFloat(int a){
		return ((float)a/10);
	}
	
	public static void insertTemperature(int sensorId, int temperature)  {
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement(
			"INSERT INTO sensor_temp(idsensor, temperature) VALUES (?, ?)");
			
			preparedStmt.setInt(1, sensorId);
			preparedStmt.setFloat(2, fromIntToFloat(temperature));
			
			preparedStmt.execute();
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	
	public static void cleanSensorTemp() {
		
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement("DELETE FROM sensor_temp");
			preparedStmt.execute();
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	
	public static float pickLastTemp() {
		float ret = -1000;
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement("SELECT temperature FROM sensor_temp ORDER BY date DESC LIMIT 1");
			ResultSet temp = preparedStmt.executeQuery();
			if(temp.next()) {
				ret = temp.getFloat("temperature");
			}
			
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return ret;
	}
}
	

