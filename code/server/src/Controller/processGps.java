package Controller;

import java.io.UnsupportedEncodingException;

import Database.HazardBean;
import Database.PSCACBean;
import Vo.FCMVo;
import Vo.PSCACVo;

public class processGps {

	void receiveGPS(String gps) throws UnsupportedEncodingException {
		PSCACBean database = new PSCACBean();
		PSCACVo pscac = new PSCACVo();

		pscac = database.getDBFaddress(gps);
		if (pscac.getId() == null)
			database.insertDB(gps);
	}



	void receiveAlarm(PSCACVo vo) throws Exception {
		PSCACBean database = new PSCACBean();
		
		//위험 빈도 생성
		HazardBean Hazarddatabase = new HazardBean();
		Hazarddatabase.insertHazard(vo);

		FCMVo fcmvo = new FCMVo();
		SendPushServer send = new SendPushServer();

		/*
		 * String gps; gps = database.getDBFId(vo.getId()).getGps();
		 * vo.setGps(gps);
		 */

		String latitude, longtitud;

		latitude = database.getDBFId(vo.getId()).getLatitude();
		longtitud = database.getDBFId(vo.getId()).getLongtitud();

		// vo.setGps(database.getDBFId(vo.getId()).getGps());
		// fcmvo.setMsg("\"content\":" + vo.getGps());
		// fcmvo.setTitle("Alarm");
		System.out.println(latitude);

		fcmvo.setLatitude(latitude);
		fcmvo.setLongitude(longtitud);

		System.out.println(vo.getStatus());
		String status = vo.getStatus();
		fcmvo.setAlarm(status);
		fcmvo.setTitle("alert");

		send.pushFCMNotification(fcmvo);

	}

	public static void main(String[] args) {
		PSCACVo vo = new PSCACVo();
		processGps pg = new processGps(); // 생성자 호출
		try {
			pg.receiveAlarm(vo);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}
}
