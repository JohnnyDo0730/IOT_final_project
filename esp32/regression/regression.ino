#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/adc.h>

// 超音波腳位
const int TRIG_PIN = 17;
const int ECHO_PIN = 5;

// ====== 感測器設定 ======
const int pinDHT11      =  4;
const int pinWaterLevel = 15;
const int PIR_PIN       = 18;
const int CO_PIN        = 34;
SimpleDHT11 dht11;

// ====== WiFi 與伺服器設定 ======
const char ssid[]     = "JohnnyDo_iPhone";
const char password[] = "07300519";
const char* serverUrl = "http://172.20.10.14:5000/esp/post_data";

// ====== 輸出元件設定 ======
LiquidCrystal_I2C lcd(0x3F, 16, 2);

const int LED_PIN = 19;
const unsigned long KEEP_MS = 10000;
unsigned long lastMotionMS = 0;
bool isLightOn = false;

const int FAN_PWM_PIN = 25;
const int BUZZER_PIN  = 16;

// ====== 超音波跌倒參數 ======
float last_distance = 0;
const float FALL_THRESHOLD = 130.0;    // 超過這個cm就警報（依實際調整）
const int DIST_BUFFER = 3;
float distanceBuffer[DIST_BUFFER] = {0};
int distIdx = 0;
int fallCnt = 0;   // 跌倒異常次數計數

// ====== 迴圈控制 ======
const unsigned long USONIC_DELAY = 200;  // 超音波建議間隔 >60ms   (120)
const int SENSOR_PERIOD = 5;             // 每5輪執行一次其他感測
int  loopCnt = 0;

void setup() {
  Serial.begin(115200);
  lcd.init();  lcd.backlight();

  pinMode(pinWaterLevel, INPUT_PULLDOWN);
  pinMode(pinDHT11,      INPUT_PULLUP);
  pinMode(PIR_PIN,       INPUT);
  pinMode(LED_PIN,       OUTPUT);
  pinMode(CO_PIN, INPUT);
  analogSetPinAttenuation(CO_PIN, ADC_11db);
  ledcAttach(FAN_PWM_PIN, 25000, 8);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  lcd.clear(); lcd.print("WiFi OK");
  delay(2000); lcd.clear();
}

// 超音波讀距離，單位cm
float readUltrasonicDistance(float last_dist) {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 最長25ms避免死等
  if (duration == 0) return last_dist; // 失敗或超出量測範圍
  float distance = duration * 0.034 / 2.0;
  return distance;
}

// 一氧化碳檢測
int readCOppm() {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(CO_PIN);
    delay(2);
  }
  return sum / 5;
}

// 風扇Duty
int calcFanDuty(int temperature, int humidity, int co_raw) {
    // 替換以下係數與截距為你實際訓練得到的值
    float fan_speed = 1.92135466 * temperature + 0.67442817 * humidity + 0.01006322 * co_raw -23.082783873332133;

    // 限制風扇速度在 0~100% 範圍
    if (fan_speed < 0.0) fan_speed = 0.0;
    if (fan_speed > 100.0) fan_speed = 100.0;

    // 將 0~100% 轉換為 0~255 的 PWM 值
    int pwm_value = (int)(fan_speed / 100.0 * 255.0);
    return pwm_value;
}


// 蜂鳴器狀態
bool getBuzzerState(int t, int h, int coRaw, int waterState) {
  bool overTemp   = (t > 35);
  bool overHum    = (h > 50);
  bool overCO     = (coRaw > 3000);
  bool overWater  = (waterState == 1);
  return (overTemp || overHum || overCO || overWater);
}

void loop() {
  // 1. 超音波距離檢測
  float distance = readUltrasonicDistance(last_distance);
  float distDiff = 0;
  distDiff = distance - last_distance;
  // 計算最近3次是否有劇烈遠離
  distanceBuffer[distIdx] = distDiff;
  distIdx = (distIdx + 1) % DIST_BUFFER;
  last_distance = distance;

  // 2. 每次都處理紅外線
  int pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    lastMotionMS = millis();
    digitalWrite(LED_PIN, HIGH);
    isLightOn = true;
  }
  if ((pirState == LOW) && (millis() - lastMotionMS > KEEP_MS)) {
    digitalWrite(LED_PIN, LOW);
    isLightOn = false;
  }

  Serial.printf("Dist: %4f , PIR: %s\n",distance, pirState ? "有人移動":"沒人");

  // 3. 每 SENSOR_PERIOD 輪才更新溫濕度等
  if (loopCnt % SENSOR_PERIOD == 0) {
    byte t = 0, h = 0;
    if (dht11.read(pinDHT11, &t, &h, NULL) == SimpleDHTErrSuccess) {
      int waterState = digitalRead(pinWaterLevel);
      int coRaw = readCOppm();
      int fanDuty = calcFanDuty(t, h, coRaw);

      // 風扇
      ledcWrite(FAN_PWM_PIN, fanDuty);

      // LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.printf("Temp:%dC Hum:%d%%", t, h);
      lcd.setCursor(0,1);
      lcd.printf("W:%5s CO:%4d", waterState ? "Full" : "Emtpy", coRaw);

      // 危險狀態蜂鳴器
      if (getBuzzerState(t, h, coRaw, waterState)) {
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("⚠️ 一般超標警報！");
      }
      else
      {
        digitalWrite(BUZZER_PIN, LOW);
      }

      Serial.printf("Temp:%dC Hum:%d%% Water:%5s CO_raw:%4d\n",
                    t, h, waterState ? "Full":"Empty", coRaw);

      sendData(packPayload(t, h, waterState, coRaw, isLightOn, distance));
    } else {
      Serial.println("DHT11 read fail");
    }
  }
  else
  {
    sendData(packPayload(isLightOn, distance));
  }

  // 傳送資料後再等待
  if (distDiff >= FALL_THRESHOLD) {
    Serial.println("⚠️ 嚴重跌倒警報！");
    digitalWrite(BUZZER_PIN, HIGH);
    // 可在此補充發通知等
    delay(6000);
  }


  loopCnt = (loopCnt + 1) % 10000;
  delay(USONIC_DELAY); // 以超音波頻率作為主循環
}

// 封包格式(高頻率)
String packPayload(bool isLightOn, float distance) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["is_light_on"] = isLightOn;
  jsonDoc["distance"] = distance;

  String payload;
  serializeJson(jsonDoc, payload);
  return payload;
}

// 封包格式(低頻率)
String packPayload(float temperature, float humidity, int waterLevel, int coRaw, bool isLightOn, float distance) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["water_level"] = waterLevel;
  jsonDoc["co_raw"] = coRaw;
  jsonDoc["is_light_on"] = isLightOn;
  jsonDoc["distance"] = distance;

  String payload;
  serializeJson(jsonDoc, payload);
  return payload;
}

// 發送資料到伺服器
void sendData(String payload) {
  HTTPClient http;
  //Serial.println("Sending to Server: " + String(serverUrl));

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    //Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    //Serial.println(response);
  } else {
    /*Serial.printf("HTTP Request failed: %s\n",
                  http.errorToString(httpResponseCode).c_str());*/
  }
  http.end();
  Serial.println("Send Success");
}
