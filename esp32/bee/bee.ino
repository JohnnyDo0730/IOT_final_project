#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/adc.h>     // 放在檔案最上面

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
LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD顯示器

const int LED_PIN = 17; // 電燈
const unsigned long KEEP_MS = 10000;  // 10 秒保持亮燈
unsigned long lastMotionMS = 0;       // 上次有人移動的時間

const int FAN_PWM_PIN = 25; // 風扇

const int BUZZER_PIN = 16; // 蜂鳴器

// ====== 迴圈控制 ======
const unsigned long LOOP_DELAY = 500;   // 每輪 500 ms
int  loopCnt = 0;                       // 計數器

void setup() {
  Serial.begin(115200);
  lcd.init();  lcd.backlight();
  pinMode(pinWaterLevel, INPUT_PULLDOWN);
  pinMode(pinDHT11,      INPUT_PULLUP);
  pinMode(PIR_PIN,       INPUT);
  pinMode(LED_PIN,       OUTPUT);
  pinMode(CO_PIN, INPUT);     // ADC 腳不用特別設定，但寫上可讀性佳
  analogSetPinAttenuation(CO_PIN, ADC_11db); // 單一腳位

  ledcAttach(FAN_PWM_PIN, 25000, 8); // fan: pin, sinal freq, pwm bit

  pinMode(BUZZER_PIN, OUTPUT);

  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  lcd.clear();
  lcd.print("WiFi OK");
  delay(2000);
  lcd.clear();
}

// 一氧化碳檢測
int readCOppm() {             // 簡化：取 5 次平均回原始 ADC 值
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(CO_PIN);
    delay(2);
  }
  return sum / 5;             // 回傳 0–4095
}

// 計算風扇 Duty (0~255)
int calcFanDuty(int temp, int hum, int coRaw) 
{
  if (coRaw > 3000)
  {
    return 255
  }

  int dutyT = 0, dutyH = 0;

  int temp_low = 32, temp_high = 36
  if (temp > temp_low) 
  {
    if (temp >= temp_high) dutyT = 255;
    else dutyT = (int)(255 * (temp - temp_low)/(temp_high - temp_low));
  }

  int hum_low = 20, hum_high = 50
  if (hum > hum_low) 
  {
    if (hum >= hum_high) dutyH = 255;
    else dutyH = (int)(255 * (hum - hum_low)/(hum_high - hum_low));
  }
  
  int duty = max(dutyT, dutyH);
  if (duty > 255) duty = 255;
  if (duty < 0) duty = 0;
  return duty;
}

bool getBuzzerState(int t, int h, int coRaw, int waterState)
{
  bool overTemp   = (t > 35);         // 溫度 > 35°C
  bool overHum    = (h > 50);         // 濕度 > 90%
  bool overCO     = (coRaw > 3000);   // CO 超過 2000 (你可依校正調整)
  bool overWater  = (waterState == 1); // 水位過高 (1 = Wet)

  return (overTemp || overHum || overCO || overWater) 
}

void loop() {
  // 預設蜂鳴器為靜音
  digitalWrite(BUZZER_PIN, LOW);

  // 每輪讀 PIR
  int pirState = digitalRead(PIR_PIN);
  Serial.println(pirState ? "有人移動" : "沒人");
  if (pirState == HIGH) {
    lastMotionMS = millis();
    digitalWrite(LED_PIN, HIGH);
  }
  if ((pirState == LOW) && (millis() - lastMotionMS > KEEP_MS)) {
    digitalWrite(LED_PIN, LOW);
  }

  // 每 6 輪 (≈3s) 才讀其他感測器 & 顯示
  if (loopCnt % 2 == 0) {
    byte t = 0, h = 0;
    if (dht11.read(pinDHT11, &t, &h, NULL) == SimpleDHTErrSuccess) 
    {
      int waterState = digitalRead(pinWaterLevel);
      int coRaw = readCOppm();
      int fanDuty = calcFanDuty(t, h);

      // 更新風扇
      ledcWrite(FAN_PWM_PIN, fanDuty);

      // 更新 LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.printf("T:%dC H:%d%%", t, h);
      lcd.setCursor(0,1);
      lcd.printf("W:%5s CO:%4d", waterState ? "Full" : "Emtpy", coRaw);

      // 更新蜂鳴器
      if (getBuzzerState(t, h, coRaw, waterState)) 
      {
        digitalWrite(BUZZER_PIN, HIGH);   // 任一條件達成才鳴叫
        Serial.println("⚠️ 警報！有超標項目");
      }
      else 
      {
        digitalWrite(BUZZER_PIN, LOW);    // 都沒超標，保持靜音
      }

      // 終端輸出
      Serial.printf("Temp:%dC Hum:%d%% Water:%s CO_raw:%d\n",
                    t, h, waterState ? "WET":"DRY", coRaw);

      // 發送給伺服端
      sendData(t, h, waterState, coRaw);
    }
    else 
    {
      Serial.println("DHT11 read fail");
    }
  }

  loopCnt = (loopCnt + 1) % 10000;
  delay(LOOP_DELAY);
}

void sendData(float temperature, float humidity, int waterLevel, int coRaw) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["water_level"] = waterLevel;
  jsonDoc["co_raw"] = coRaw;

  String payload;
  serializeJson(jsonDoc, payload);

  HTTPClient http;
  Serial.println("Sending to Server: " + String(serverUrl));
  //Serial.println("Payload: " + payload);

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
