#ifndef GLOBALS_H
#define GLOBALS_H

#include <REG.h>
#include <wit_c_sdk.h>
#include <Arduino.h>
#include <M5StickC.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include <string>
#include <ctime>
#include <ArduinoJson.h>
#include <WebServer.h>

using namespace std;

#define RX 32
#define TX 33
#define ACC_UPDATE		0x01

// 외부 변수 선언
extern volatile char s_cDataUpdate;
extern volatile char s_cCmd;

extern const char* ssid;
extern const char* password;
extern const char* mqtt_server;

extern const char* location;
extern const char* subLocation;
extern const char* part;
extern const char* sensorType;
extern int sensorIndex;
extern const char* valueType[3];
extern const char* topic;
extern const char* sensorValue;
extern char ch[3][10];
extern const char* check;
extern String time_set;
extern WiFiClient espClient;
extern PubSubClient client;
extern WebServer webserver;

// NTP 클라이언트 설정
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// 타이머 변수
extern unsigned long initialEpoch;
extern unsigned long initialMillis;

// JSON 데이터 생성
extern DynamicJsonDocument doc;
extern JsonArray data;
extern String str(const char* rc);
extern const uint32_t c_uiBaud[8];
extern String clientId;

// 카운터 변수
extern int cnt;
extern int rec_cnt;
extern int i;
extern float fAcc[3];

#endif