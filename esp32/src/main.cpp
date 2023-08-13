#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "esp_wpa2.h"
#include <PubSubClient.h>
#include <Wire.h>


const char* ssid = "heensan";
const char* password = "whitemountain";

const char* serverAddress = "http://59.14.135.171:21330/update_number/";
const char* mqtt_server = "192.168.0.16";

const int xPin = 0;
const int yPin = 2;
const int zPin = 14;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

void sendNumberToServer(int number);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
// char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup() {
  Serial.begin(9600);
  delay(10);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi 연결 중...");
  }

  Serial.println("WiFi 연결 성공");

  // client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);
}

void loop() {
  // if (!client.connected()) {
  //   reconnect();
  // }
  // client.loop();

  // int randomNumber = random(0, 100);  // 0부터 99까지의 랜덤한 숫자 생성
  // Serial.println("랜덤 숫자: " + String(randomNumber));
  // sendNumberToServer(randomNumber);
  // delay(1000);  // 5초마다 숫자를 서버로 보냄

  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  int xAng = map(xRead, minVal, maxVal, -90, 90);
  int yAng = map(yRead, minVal, maxVal, -90, 90);
  int zAng = map(zRead, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" | y: ");
  Serial.print(y);
  Serial.print(" | z: ");
  Serial.println(z);
  delay(100);

  // unsigned long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   ++value;
  //   char* msg = (char *)randomNumber;
  //   snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish("outTopic", msg);
  // }
}

void sendNumberToServer(int number) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // 서버 주소로 POST 요청 보냄
    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "number=" + String(number);  // 서버로 보낼 데이터 생성

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("서버 응답: " + response);
    } else {
      Serial.println("오류 발생. 오류 코드: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi 연결이 끊겼습니다.");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}