package database;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class ActuatorDA {
	
	public static void insertActuator(int actuatorId, String type, String address) throws SQLException{
		
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement(
			"INSERT INTO actuator(idactuator, type, address) VALUES (?, ?, ?)");
			
			preparedStmt.setInt(1, actuatorId);
			preparedStmt.setString(2, type);
			preparedStmt.setString(3, address);
			preparedStmt.execute();
			conn.close();
	}
	
	public static void deleteActuator(int actuatorId) {
		
	}

	public static String retrieveActuatorAddress(int actuatorId, String type) {
		String address = null;
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement(
					"SELECT * from actuator WHERE idactuator=? AND type=?");
			
			preparedStmt.setInt(1, actuatorId);
			preparedStmt.setString(2, type);
			ResultSet actuator = preparedStmt.executeQuery();
			if(actuator.next()) {
				address = actuator.getString("address");
			}
			conn.close();
			
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return address;
	}
	
	public static void cleanActuator() {
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement("DELETE from actuator");
			preparedStmt.execute();
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}

	public static boolean isPresent(int actuator_id, String type, String address) {
        try{
        	Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
        	boolean ret = false;
        	PreparedStatement preparedStmt;
        	preparedStmt = conn.prepareStatement("SELECT * FROM actuator WHERE idactuator = ? AND type = ? AND address = ?");
        	
        	preparedStmt.setInt(1, actuator_id);
			preparedStmt.setString(2, type);
			preparedStmt.setString(3, address);
            preparedStmt.execute();
            ResultSet resultSet = preparedStmt.executeQuery();
            ret = resultSet.next(); // If the result set has at least one row, the record is present.
            conn.close();
            return ret;
            
        } catch (SQLException e) {
        	e.printStackTrace();
        	return false;
        }
	}
}
	
