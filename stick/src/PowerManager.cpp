#include "PowerManager.h"

void powerOff() {
    float batvol = M5.Axp.GetVBusVoltage();
    bool isCharging = batvol > 4.0; // 4.0 초과 -> 전원, 4.0 이하 -> 배터리
    if (!isCharging) {
        Serial.println("배터리 사용중. 디바이스를 종료합니다...");

        // OLED 디스플레이에 메시지 표시
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("Charging...");
        M5.Lcd.println("Shutting down");
        Serial.println("Shutting down");

        delay(2000);

        // power off 되었다는 메시지 전송
        const char* topic = str("status").c_str();
        client.publish(topic, "Power Off");

        // AXP192를 통해 전원 차단
        M5.Axp.PowerOff();
    }
}