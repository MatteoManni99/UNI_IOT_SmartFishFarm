package database;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

public class QualityDA {
	
	public static void insertQuality(int sensorId, int quality)  {
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement(
			"INSERT INTO sensor_quality(idsensor, quality) VALUES (?, ?)");
			
			preparedStmt.setInt(1, sensorId);
			preparedStmt.setFloat(2, quality);
			
			preparedStmt.execute();
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	
	public static void cleanSensorQuality() {
		
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement("DELETE FROM sensor_quality");
			preparedStmt.execute();
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	
	public static int pickLastQuality() {
		int ret = -1;
		try {
			Connection conn = DriverManager.getConnection(
					"jdbc:mysql://localhost:3306/fishfarm","root","password");
			PreparedStatement preparedStmt;
			preparedStmt = conn.prepareStatement("SELECT quality FROM sensor_quality ORDER BY date DESC LIMIT 1");
			ResultSet quality = preparedStmt.executeQuery();
			if(quality.next()) {
				ret = quality.getInt("quality");
			}
			
			conn.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return ret;
	}
}
	

