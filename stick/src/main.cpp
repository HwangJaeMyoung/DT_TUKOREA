#include <REG.h>
#include <wit_c_sdk.h>
#include <Arduino.h>
#include <M5StickC.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string>
#include <ctime>

using namespace std;

#define RX 32
#define TX 33
#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80

static volatile char s_cDataUpdate = 0, s_cCmd = 0xff; 

const char* ssid = "DT_TUKOREA";
const char* password = "DiK_WiMiS_30!";
const char* mqtt_server = "10.101.21.198";

const char* location = "2Campus";
const char* subLocation = "SMTLINE";
const char* part = "2";
const char* sensorType = "Vibration";
int sensorIndex = 1;
const char* valueType[3] = {"X", "Y", "Z"};
const char* value;
const char* sensorValue = "Value";
char ch[3][10];
const char* check = "1";

String clientId;
WiFiClient espClient;
PubSubClient client(espClient);

// NTP 클라이언트 설정
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC 기준, GMT+9 (한국 표준시) 설정

void sendJsonData(const String& topic);
void CopeCmdData(unsigned char ucData);
static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
void setup_wifi();
void callback(const char* topic, byte* payload, unsigned int length);
void reconnect();
int connect_first();
String str(const char* rc);
String timeset(char ch[3][10]);
const uint32_t c_uiBaud[8] = {1200, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

void setup() {
    // Serial(컴퓨터), Serial2(센서) 포트 열고 비트 수 설정
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX, TX);

    // wifi 설정
    setup_wifi();

    // MQTT 서버 설정
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Arduino 출력 설정
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.println("        X     Y     Z");

    // 센서 설정 및 스캔(WT901CTTL 센서 참고)
    // WitInit(WIT_PROTOCOL_NORMAL, 0x50);
    // WitSerialWriteRegister(SensorUartSend);
    // WitRegisterCallBack(SensorDataUpdata);
    // WitDelayMsRegister(Delayms);
    // WitSetOutputRate(RRATE_200HZ);
    // AutoScanSensor();

    // 센서 connect 시도
    if (connect_first() == 0){
        exit(1);
    }
}
int i;
float fAcc[3], fGyro[3], fAngle[3];
void loop() {
    // disconnect인 경우 reconnect 반복
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    delay(1000);

    // JSON 데이터를 전송하는 함수를 호출
    sendJsonData(str(sensorValue));
}

void CopeCmdData(unsigned char ucData)
{
    static unsigned char s_ucData[50], s_ucRxCnt = 0;
    
    s_ucData[s_ucRxCnt++] = ucData;
    if(s_ucRxCnt<3) return;
    if(s_ucRxCnt >= 50) s_ucRxCnt = 0;
    if(s_ucRxCnt >= 3)
    {
        if((s_ucData[1] == '\r') && (s_ucData[2] == '\n'))
        {
            s_cCmd = s_ucData[0];
            memset(s_ucData,0,50);
            s_ucRxCnt = 0;
        }
        else 
        {
            s_ucData[0] = s_ucData[1];
            s_ucData[1] = s_ucData[2];
            s_ucRxCnt = 2;
            
        }
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
        switch(uiReg)
        {
            case AZ:
                s_cDataUpdate |= ACC_UPDATE;
            break;
            case GZ:
                s_cDataUpdate |= GYRO_UPDATE;
            break;
            case HZ:
                s_cDataUpdate |= MAG_UPDATE;
            break;
            case Yaw:
                s_cDataUpdate |= ANGLE_UPDATE;
            break;
            default:
                s_cDataUpdate |= READ_UPDATE;
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

void callback(const char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    client.unsubscribe(str("Confirm").c_str());
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect(clientId.c_str())) {
            value = str("Register").c_str();
            client.publish(value, check);
        } else {
            Serial.println("1분 대기");
            delay(60000);
        }
    }
}

int connect_first() {
    int result = 0;
    if (client.connect(clientId.c_str())) {
        value = str("Confirm").c_str();
        client.subscribe(value, 1);
        
        value = str("Register").c_str();
        client.publish(value, check);

        result = 1;
    }
    return result;
}

String str(const char* rc){
    String rcstr = "ICCMS/";
    rcstr += location;
    rcstr += "/";
    rcstr += subLocation;
    rcstr += "/";
    rcstr += part;
    rcstr += "/";
    rcstr += sensorType;
    rcstr += "/";
    rcstr += String(sensorIndex).c_str();
    rcstr += "/";
    rcstr += rc;
    return rcstr;
}

String timeset(char ch[3][10]){
    time_t timer;
    struct tm* t;
    time(&timer);
    t = localtime(&timer);
    String rcstr = "";
    rcstr += ch[0];
    rcstr += "_";
    rcstr += ch[1];
    rcstr += "_";
    rcstr += ch[2];
    return rcstr;
}

// JSON 데이터를 MQTT로 전송하는 함수
void sendJsonData(const String& topic) {
    // NTP 클라이언트 업데이트
    timeClient.update();

    // 현재 시간 가져오기
    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);

    // 현재 시간 문자열로 변환
    char time[25];
    sprintf(time, "%04d_%02d_%02d-%02d_%02d_%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour + 8, ptm->tm_min, ptm->tm_sec);

    // JSON 데이터 생성
    StaticJsonDocument<200> doc;
    doc["topic"] = topic;
    doc["time"] = time;
    doc["value"]["X"] = 1.000000;
    doc["value"]["Y"] = 2.000000;
    doc["value"]["Z"] = 3.000000;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);

    // MQTT로 JSON 데이터 전송
    client.publish(topic.c_str(), buffer, n);
    Serial.printf("JSON sent to topic %s: %s\n", topic.c_str(), buffer);
}
