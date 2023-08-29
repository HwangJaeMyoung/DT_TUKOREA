#include <Arduino.h>
#define RX_PIN 14 // 수신 핀 (rx)
#define TX_PIN 12 // 송신 핀 (tx)
#include <SoftwareSerial.h>

SoftwareSerial mySerial(RX_PIN, TX_PIN); // SoftwareSerial 객체 생성


void setup() {
  Serial.begin(115200); // 시리얼 통신 초기화
  mySerial.begin(115200); // SoftwareSerial 통신 초기화
  // mySerial.write(0x64);
  // mySerial.write(0x64);

}

void loop() {
  if (mySerial.available()) {
    // Serial.write(mySerial.read());
    // byte buffer[1];
    // mySerial.readBytes(buffer, 1);
    // Serial.println(buffer[0]);
    // delay(1000);
    
    byte buffer[11];
    mySerial.readBytes(buffer, 11); // 데이터 읽기
    if(buffer[0]==0x55){


      if(buffer[1]==0x51){
        for (int i = 0; i < 11; i++) {
          Serial.print(buffer[i]);
          Serial.print(" ");
        }
        Serial.println("");

        byte axL = buffer[2];
        byte axH = buffer[3];
        byte ayL = buffer[4];
        byte ayH = buffer[5];
        byte azL = buffer[6];
        byte azH = buffer[7];
        byte TL = buffer[8];
        byte TH = buffer[9];
        byte sum = buffer[10];

        byte calculatedSum = 0;
        for (int i = 0; i < 10; i++) {
          calculatedSum += buffer[i];
        }
        if (calculatedSum == sum) {
          float Ax = ((axH << 8) | axL)/32768.0*16*9.8;
          float Ay = ((ayH << 8) | ayL)/32768.0*16*9.8;
          float Az = ((azH << 8) | azL)/32768.0*16*9.8;

          // // 온도 계산 공식 적용
          Serial.print(((TH << 8) | TL) );
          float temperature = ((TH << 8) | TL) / 340.0 + 36.53;
          Serial.print("X-axis acceleration: ");
          Serial.println(Ax);
          Serial.print("Y-axis acceleration: ");
          Serial.println(Ay);
          Serial.print("Z-axis acceleration: ");
          Serial.println(Az);
          Serial.print("Temperature: ");
          Serial.println(temperature);
          
        } else {
          Serial.println("Checksum is invalid.");
        }
      }

    }
    
    // Serial.println(buffer[1]);
    // Serial.println(buffer[0]);
    // Serial.println(0x55);

    // if (buffer[1] ==0x51){
    //   byte axL = buffer[2];
    //   byte axH = buffer[3];
    //   byte ayL = buffer[4];
    //   byte ayH = buffer[5];
    //   byte azL = buffer[6];
    //   byte azH = buffer[7];
    //   byte TL = buffer[8];
    //   byte TH = buffer[9];
    //   byte sum = buffer[10];
      
    //   // 계산 공식 적용


    //   // // Checksum 검증
    //   byte calculatedSum = 0;
    //   for (int i = 0; i < 10; i++) {
    //     calculatedSum += buffer[i];
    //   }
    //   calculatedSum = calculatedSum+ 0x55 + 0x51; 

    //   // 시리얼 모니터에 결과 출력
    //   Serial.print("X-axis acceleration: ");
    //   Serial.println(Ax);
    //   Serial.print("Y-axis acceleration: ");
    //   Serial.println(Ay);
    //   Serial.print("Z-axis acceleration: ");
    //   Serial.println(Az);
    //   Serial.print("Temperature: ");
    //   Serial.println(temperature);

    //   if (calculatedSum == sum) {
    //     Serial.println("Checksum is valid.");
    //   } else {
    //     Serial.println("Checksum is invalid.");
    //   }
    // }
  } 
}


// // #include <WiFi.h>
// // #include <HTTPClient.h>
// #include <SoftwareSerial.h>
// #include "I2Cdev.h"
// #include "MPU6050.h"
// #include <Wire.h>


// MPU6050 mpu;


// #define PIN_TX 1  
// #define PIN_RX 3 

// int16_t ax, ay, az, gx, gy,gz;
// const char* ssid = "1309";
// const char* password = "00000000";

// const char* serverAddress = "http://59.14.135.171:21330/update_number/";


// void setup() {
  
//   Serial.begin(115200);
//   Serial1.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
//   mpu.initialize();
//   Wire.begin();
//   // WiFi.begin(ssid, password);

// //   while (WiFi.status() != WL_CONNECTED) {
// //     delay(1000);
// //     Serial.println("WiFi 연결 중...");
// //   }

// //   Serial.println("WiFi 연결 성공");
// }

// void loop() {
//   // 가속도와 자이로스코프 데이터 읽기
//   int16_t ax, ay, az, gx, gy, gz;
//   mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

//   // 시리얼 모니터에 출력
//   Serial.print("가속도: ");
//   Serial.print(ax); Serial.print(", ");
//   Serial.print(ay); Serial.print(", ");
//   Serial.print(az); Serial.print(" | ");

//   Serial.print("자이로스코프: ");
//   Serial.print(gx); Serial.print(", ");
//   Serial.print(gy); Serial.print(", ");
//   Serial.println(gz);

//   // 센서 데이터를 아두이노의 TX 핀을 통해 전송
//   Serial1.print("센서 데이터: ");
//   Serial1.print(ax); Serial1.print(", ");
//   Serial1.print(ay); Serial1.print(", ");
//   Serial1.print(az); Serial1.print(" | ");
//   Serial1.print(gx); Serial1.print(", ");
//   Serial1.print(gy); Serial1.print(", ");
//   Serial1.println(gz);

//   delay(1000);
// }

// void sendNumberToServer(int number) {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     // 서버 주소로 POST 요청 보냄
//     http.begin(serverAddress);
//     http.addHeader("Content-Type", "application/x-www-form-urlencoded");

//     String postData = "number=" + String(number);  // 서버로 보낼 데이터 생성

//     int httpResponseCode = http.POST(postData);

//     if (httpResponseCode > 0) {
//       String response = http.getString();
//       Serial.println("서버 응답: " + response);
//     } 
//     else {
//       Serial.println("오류 발생. 오류 코드: " + String(httpResponseCode));
//     }

//     http.end();
//   } else {
//     Serial.println("WiFi 연결이 끊겼습니다.");
//   }
// }
