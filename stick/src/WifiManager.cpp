#include "WiFiManager.h"

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect(String clientId) {
    // 전원 공급이 끊긴 경우 종료
	powerOff();
	
	if (WiFi.status() != WL_CONNECTED) {
		rec_cnt += 1;
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.print(rec_cnt);
		M5.Lcd.println(": WiFi disconnected, attempting to reconnect...");
		Serial.println((String) checkTime() + ": " + rec_cnt + " WiFi disconnected, attempting to reconnect...");
		WiFi.disconnect();
		WiFi.begin(ssid, password);
	}
	while (!client.connected()) {
		if (client.connect(clientId.c_str())) {
			topic = str("Register").c_str();
			client.publish(topic, check);
			rec_cnt = 0;
		} 
		else {
			Serial.println("1분 대기");
			delay(60000);
		}
	}
}

int connect_first(String clientId) {
	int result = 0;
	if (client.connect(clientId.c_str())) {
		topic = str("Confirm").c_str();
		client.subscribe(topic, 1);
		
		topic = str("Register").c_str();
		client.publish(topic, check);

		result = 1;
	}
	return result;
}