// #define BLYNK_TEMPLATE_ID ""
// #define BLYNK_TEMPLATE_NAME ""
// #define BLYNK_AUTH_TOKEN ""

#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiManager.h>    // dùng thư viện này để config wifi
#include "DHTesp.h" 
#include <ESP32Servo.h>

#define DHT_PIN 15
#define LDR_PIN 34
#define RELAY_LIGHT_PIN 4
#define LDR_LED_PIN 2   // chân led cho cảm biến ánh sáng
#define servo 25
#define PIR_SENSOR 14
#define MOTION_LED 32

#define BLYNK_PRINT Serial

DHTesp dhtSensor; 
BlynkTimer timer;
Servo s;

unsigned long timeDelay = millis();
unsigned long timeUpdata = millis();

BLYNK_CONNECTED() {
  Blynk.syncAll();
}
BLYNK_WRITE(V1){
  int p = param.asInt();
  Serial.println("Write relay 1");
  Serial.println(p);

  digitalWrite(RELAY_LIGHT_PIN, p);
}

/* Servo đóng mở cửa */
String correctPassword = "KhoaLon123"; // Mật khẩu đúng để kiểm tra
String inputPassword = "";             // Mật khẩu được nhập
bool switchOn = false;                 // Trạng thái công tắc

// Xử lý khi có thay đổi trên Virtual Pin V3 (Switch)
BLYNK_WRITE(V3) {
  switchOn = param.asInt(); // Lấy trạng thái công tắc (0 hoặc 1)
  if (switchOn) {
    Serial.println("Switch ON - Awaiting Password");
    if (inputPassword.equals(correctPassword)) {
      Serial.println("Password correct! Activating servo...");
      s.write(90); // Điều khiển servo quay 90 độ
      inputPassword = ""; // Xóa mật khẩu sau khi sử dụng
    } else {
      Serial.println("Incorrect Password! Resetting switch to OFF.");
      s.write(0); // Servo ở vị trí ban đầu
      Blynk.virtualWrite(V3, 0); // Đặt Switch về OFF trên ứng dụng
    }
  } else {
    Serial.println("Switch OFF - Resetting Servo");
    s.write(0); // Đặt servo về vị trí 0 độ
  }
}

// Xử lý khi có thay đổi trên Virtual Pin V4 (Password Input)
BLYNK_WRITE(V4) {
  inputPassword = param.asStr(); // Lấy mật khẩu từ input
  Serial.print("Password entered: ");
  Serial.println(inputPassword);
}

/* DHT nhiệt độ độ ẩm */
void sendDhtSensor()
{
  // Get the data
  TempAndHumidity  data = dhtSensor.getTempAndHumidity(); 
  int temp = data.temperature; 
  int humid = data.humidity; 

  // if (isnan(humid) || isnan(temp)) {
  //   Serial.println("Failed to read from DHT sensor!");
  //   return;
  // }

  String stemp = String(temp) + "C"; 
  String shumid = String(humid) + "%"; 

  Serial.println(stemp + "---" + shumid);

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V13, humid);  //V13 is for Humidity
  Blynk.virtualWrite(V12, temp);  //V12 is for Temperature
}

/* ánh sáng */
void sendLightSensor()
{
  int LDR = analogRead(LDR_PIN);  
    if(LDR > 1000)
  {
    digitalWrite(LDR_LED_PIN, HIGH);
    // Blynk.notify("Light ON");
    Serial.println(LDR);
    delay(1000);
  }
  else
  {
    digitalWrite(LDR_LED_PIN, LOW);
    Serial.println(LDR);
    delay(1000);
  }

  //Blynk.virtualWrite(V5, LDR);
}

void sendPirSensor() {
  int pir_value = digitalRead(PIR_SENSOR);
  if (pir_value == 1)
  {
    Serial.println("Motion detected");
    digitalWrite(MOTION_LED, HIGH);
    delay(100);
  }
  else
  {
    Serial.println("Motion ended");
    digitalWrite(MOTION_LED, LOW);
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);//uart giao tiếp PC
  WiFiManager wifiManager;

  wifiManager.startConfigPortal("ESP32-Config");  // tên wifi

  // Connected
  Serial.println("Connected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  pinMode(RELAY_LIGHT_PIN, OUTPUT);
  pinMode(LDR_LED_PIN, OUTPUT);
  pinMode(PIR_SENSOR, INPUT_PULLUP);
  pinMode(MOTION_LED, OUTPUT);
  
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);   // ko cần dùng ssid vs pass, chỉ cần auth token
  Blynk.connect();

  // setup servo
  s.attach(servo); // Gắn servo vào chân 25
  s.write(0);      // Đặt servo về vị trí ban đầu

  //setup for dht sensor 
  dhtSensor.setup(DHT_PIN, DHTesp::DHT11);

  timer.setInterval(1000L, sendDhtSensor);
  timer.setInterval(500L, sendLightSensor);
  timer.setInterval(500L, sendPirSensor);
}

void loop() {
  Blynk.run();
  if(millis()-timeUpdata>1000){
    Blynk.virtualWrite(V0,millis()/1000);
    timeUpdata = millis();
  }
  timer.run(); // Initiates SimpleTimer
    
}