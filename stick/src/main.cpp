#include "Globals.h"
#include "WebServerManager.h"
#include "SensorManager.h"
#include "WiFiManager.h"

volatile char s_cDataUpdate = 0;
volatile char s_cCmd = 0xff;

const char* ssid = "DT_TUKOREA";
const char* password = "DiK_WiMiS_30!";
const char* mqtt_server = "10.101.21.198";

const char* location = "2Campus";
const char* subLocation = "SMTLINE";
const char* part = "1";
const char* sensorType = "Vibration";
int sensorIndex = 1;
const char* valueType[3] = {"X", "Y", "Z"};
const char* topic;
const char* sensorValue = "Value";
char ch[3][10];
const char* check = "1";
String time_set;
WiFiClient espClient;
PubSubClient client(espClient);
WebServer webserver(80);

// NTP 클라이언트 설정
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400, 60000); // UTC 기준, GMT+9 (한국 표준시) 설정

unsigned long initialEpoch = 1;
unsigned long initialMillis;

// JSON 데이터 생성
DynamicJsonDocument doc(5000);
JsonArray data = doc.createNestedArray("data");
String str(const char* rc);
const uint32_t c_uiBaud[8] = {1200, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

String clientId = str(sensorValue) + String(random(0xffff), HEX);

int cnt = 0;
int rec_cnt = 0;
int i;
float fAcc[3];

void setup() {
	// Serial(컴퓨터), Serial2(센서) 포트 열고 비트 수 설정
  	Serial.begin(115200);
  	Serial2.begin(115200, SERIAL_8N1, RX, TX);

	// wifi 설정
	setup_wifi();

	// MQTT 서버 설정
	client.setServer(mqtt_server, 1883);
	client.setBufferSize(5000);
	
	// 센서 설정 및 스캔(WT901CTTL 센서 참고)
	initialize_sensors();

	// 센서 connect 시도
	if (connect_first(clientId) == 0){
		Serial.println((String) checkTime() + ": connect_first exit");
		exit(1);
	}
	
	M5.begin();
	for (int i = 0; i < 4; i++) {
    	M5.Lcd.setRotation(i);
    	M5.Lcd.fillScreen(TFT_BLACK);
  	}
	M5.Lcd.setRotation(0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
	M5.Lcd.setCursor(0, 0);
	M5.Lcd.println("running");
	
	
	// 1970년 1월 2일 이전으로 시간이 나오는 경우 다시 설정
	do{
		// NTPClient 시작
		timeClient.begin();
		timeClient.update();
	
		// 초기 에포크 시간과 밀리초 저장
		initialEpoch = timeClient.getEpochTime();
		initialMillis = millis();

		Serial.println("Time setting..");
	} while(initialEpoch < 86400);

	Serial.println((String) checkTime() + ": setup completed");

	webserver.on("/", functionA);
  	webserver.onNotFound(handleNotFound);
	webserver.begin();
}
void loop() {
	if(!digitalRead(M5_BUTTON_HOME))
	{
		Serial.println("Button HOME is Pressed");
		while(1){
			if(!digitalRead(M5_BUTTON_RST)){
				Serial.println("Button RST is Pressed");
				break;
			}
		}
	}
	
	if (espClient.read() != -1) {
		Serial.println("Connected");
		espClient.write("Connected\n");
	}

	webserver.handleClient();

	powerOff();

	// disconnect인 경우 reconnect 반복
	if (!client.connected()) {
		Serial.println((String) checkTime() + ": " + rec_cnt + "times reconnect in loop now");
		reconnect(clientId);
	}
	client.loop();

	// connect된 경우 센서 데이터를 arduino로 읽기
	while (Serial2.available())
	{
		WitSerialDataIn(Serial2.read());
	}
	
	// X, Y, Z값을 arduino 및 컴퓨터에 출력하여 값 확인하고 topic에 맞춰 MQTT 통신으로 전달
	if(s_cDataUpdate & ACC_UPDATE)
	{
		for(int i = 0; i < 3; i++){
			fAcc[i] = sReg[AX+i];
		}

		JsonObject obj = data.createNestedObject();
		obj["time"] = checkTime();
		JsonObject value = obj.createNestedObject("value");
		value["x"] = fAcc[0];
		value["y"] = fAcc[1];
		value["z"] = fAcc[2];
		
		cnt += 1;

		s_cDataUpdate &= ~ACC_UPDATE;
	}
	s_cDataUpdate = 0;
	if (cnt == 50){
		// JSON 데이터 직렬화
		char jsonString[5000];
		serializeJson(doc, jsonString);

		// JSON 데이터 MQTT로 전송
		client.publish(str(sensorValue).c_str(), (const char*)jsonString);

		// JSON 초기화
		doc.clear();
		data = doc.createNestedArray("data");
		cnt = 0;
		
		Serial.println((String) checkTime() + ": send the JSON file to server");
	}
}