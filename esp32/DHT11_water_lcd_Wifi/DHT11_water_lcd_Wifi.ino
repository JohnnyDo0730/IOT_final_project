#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ====== 感測器設定 ======
int pinDHT11 = 4;
SimpleDHT11 dht11;

int pinWaterLevel = 2; // 水位感測器

// ====== WiFi 與伺服器設定 ======
const char ssid[] = "JohnnyDo_iPhone";
const char password[] = "07300519";
const char* serverUrl = "http://172.20.10.14:5000/esp/post_data";

// ====== LCD 設定 ======
LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD 地址可能為 0x3F，視模組而定

void setup() {
  Serial.begin(115200);
  delay(10);

  // LCD 初始化
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // 設定腳位
  pinMode(pinWaterLevel, INPUT_PULLDOWN);
  pinMode(pinDHT11, INPUT_PULLUP);

  // 連線 WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;

  // 讀取 DHT11
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err=");
    Serial.println(err);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT11 Error:");
    lcd.setCursor(0, 1);
    lcd.print("Err ");
    lcd.print(err);
    delay(2000);
    //return;
  }

  // 讀取水位
  int waterState = digitalRead(pinWaterLevel);

  // 顯示到 LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)temperature);
  lcd.print("C H:");
  lcd.print((int)humidity);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Water:");
  lcd.print(waterState == 1 ? "Wet" : "Dry");

  // 顯示到序列埠
  Serial.println("=================================");
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Water: ");
  Serial.println(waterState == 1 ? "WET" : "DRY");

  // 傳送資料到 Flask
  sendData(temperature, humidity, waterState);

  delay(5000);
}

void sendData(float temperature, float humidity, int waterLevel) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["water_level"] = waterLevel;

  String payload;
  serializeJson(jsonDoc, payload);

  HTTPClient http;
  Serial.println("Sending to Server: " + String(serverUrl));
  Serial.println("Payload: " + payload);

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.printf("HTTP Request failed: %s\n",
                  http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  Serial.println();
}
