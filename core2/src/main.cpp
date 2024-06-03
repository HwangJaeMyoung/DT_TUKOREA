#include <REG.h>
#include <wit_c_sdk.h>
#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include <string>
#include <ctime>
#include <ArduinoJson.h>

using namespace std;

#define RX 32
#define TX 33
#define ACC_UPDATE		0x01

static volatile char s_cDataUpdate = 0, s_cCmd = 0xff; 

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

// NTP 클라이언트 설정
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400, 60000); // UTC 기준, GMT+9 (한국 표준시) 설정

unsigned long initialEpoch;
unsigned long initialMillis;

// JSON 데이터 생성
DynamicJsonDocument doc(5000);
JsonArray data = doc.createNestedArray("data");

static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
void setup_wifi();
void reconnect(String clientId);
int connect_first(String clientId);
String str(const char* rc);
const uint32_t c_uiBaud[8] = {1200, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

String clientId = str(sensorValue) + String(random(0xffff), HEX);
int cnt = 0;

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
	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
  	WitDelayMsRegister(Delayms);
	WitSetOutputRate(RRATE_200HZ);
	AutoScanSensor();

	// 센서 connect 시도
	if (connect_first(clientId) == 0){
		exit(1);
	}
	
	// NTPClient 시작
	timeClient.begin();
	timeClient.update();
  
	// 초기 에포크 시간과 밀리초 저장
	initialEpoch = timeClient.getEpochTime();
	initialMillis = millis();

	cnt = 0;

	M5.begin();
}
int i;
float fAcc[3];
void loop() {
	// disconnect인 경우 reconnect 반복
	if (!client.connected()) {
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

		unsigned long currentMillis = millis();
		unsigned long elapsedMillis = currentMillis - initialMillis;
		unsigned long currentEpoch = initialEpoch + elapsedMillis / 1000;

		// 시간 구조체 생성
		tmElements_t tm;
		breakTime(currentEpoch, tm);

		// 밀리초 단위 계산
		int milliseconds = elapsedMillis % 1000;

		// 현재 시간 문자열로 변환
		char time[30];
		sprintf(time, "%04d_%02d_%02d-%02d_%02d_%02d.%03d",
			tmYearToCalendar(tm.Year), tm.Month, tm.Day,
			tm.Hour, tm.Minute, tm.Second, milliseconds);

		JsonObject obj = data.createNestedObject();
		obj["time"] = time;
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
	}
}
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
  Serial2.write(p_data, uiSize);
  Serial2.flush();
}
static void Delayms(uint16_t ucMs)
{
  delay(ucMs);
}
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    for(i = 0; i < uiRegNum; i++)
    {
		if(uiReg == AZ){
			s_cDataUpdate |= ACC_UPDATE;
			break;
		}
		uiReg++;
    }
}

static void AutoScanSensor(void)
{
	int i, iRetry;
	
	for(i = 0; i < sizeof(c_uiBaud)/sizeof(c_uiBaud[0]); i++)
	{
		Serial2.begin(c_uiBaud[i]);
    	Serial2.flush();
		iRetry = 2;
		s_cDataUpdate = 0;
		do
		{
			WitReadReg(AX, 3);
			delay(200);
      while (Serial2.available())
      {
        WitSerialDataIn(Serial2.read());
      }
			if(s_cDataUpdate != 0)
			{
				return ;
			}
			iRetry--;
		}while(iRetry);		
	}
}
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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(String clientId) {
  while (!client.connected()) {
    if (client.connect(clientId.c_str())) {
		topic = str("Register").c_str();
		client.publish(topic, check);
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

String str(const char* rc){
	char rcstr[50];
	sprintf(rcstr, "ICCMS/%s/%s/%s/%s/%d/%s", location, subLocation, part, sensorType, sensorIndex, rc);

	return String(rcstr);
}