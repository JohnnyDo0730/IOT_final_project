#include <SimpleDHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ====== 感測器設定 ======
int pinDHT11 = 4;                  // DHT11 溫濕度感測器接腳 (GPIO15)
SimpleDHT11 dht11;

int pinWaterLevel = 2;            // 數位水位感測器 S 腳接 GPIO4

// ====== WiFi 與伺服器設定 ======
const char ssid[] = "JohnnyDo_iPhone";           // WiFi 名稱
const char password[] = "07300519";              // WiFi 密碼
const char* serverUrl = "http://172.20.10.14:5000/esp/post_data"; // Flask API 位址

void setup() {
  Serial.begin(115200);
  delay(10);

  // 設定腳位
  pinMode(pinWaterLevel, INPUT_PULLDOWN);  // 數位水位感測器腳位
  pinMode(pinDHT11, INPUT_PULLUP);

  // 連線 WiFi
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  byte temperature = 0;
  byte humidity = 0;
  int err = SimpleDHTErrSuccess;

  Serial.println("=================================");

  
  // 讀取 DHT11 資料
  if ((err = dht11.read(pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT11 failed, err=");
    Serial.println(err);
    
  }

  // 讀取水位感測器狀態（0=乾燥/無水，1=有水）
  int waterState = digitalRead(pinWaterLevel);
  Serial.print("WaterState: ");
  Serial.print(waterState);
  Serial.println();


  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C");
  Serial.println();
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println();

  /*
  // 顯示感測資料
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Water Level: ");
  Serial.print(waterState == LOW ? "WET (有水)" : "DRY (無水)");
  Serial.println();
  */
  // 傳送資料
  sendData(temperature, humidity, waterState);
  
  delay(5000);  // 每 2 秒傳送一次
}


void sendData(float temperature, float humidity, int waterLevel) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["water_level"] = waterLevel; // 1 = 有水, 0 = 無水

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
