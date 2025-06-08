#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/adc.h>     // 放在檔案最上面

// ====== 感測器設定 ======
const int pinDHT11      = 4;
const int pinWaterLevel = 15;
const int PIR_PIN       = 18;
SimpleDHT11 dht11;

/***** 新增腳位 *****/
const int CO_PIN = 34;        // MQ-7 AO → GPIO34 (ADC1)


// ====== WiFi 與伺服器設定 ======
const char ssid[]     = "JohnnyDo_iPhone";
const char password[] = "07300519";
const char* serverUrl = "http://172.20.10.14:5000/esp/post_data";

// ====== LCD ======
LiquidCrystal_I2C lcd(0x3F, 16, 2);


//輸出
const int LED_PIN = 2;
const unsigned long KEEP_MS = 10000;  // 10 秒保持亮燈
unsigned long lastMotionMS = 0;       // 上次有人移動的時間

const int FAN_PWM_PIN = 25;
const int BUZZER_PIN = 16;

// ====== 迴圈控制 ======
const unsigned long LOOP_DELAY = 800;   // 每輪 200 ms
int  loopCnt = 0;                       // 計數器

void setup() {
  Serial.begin(115200);
  lcd.init();  lcd.backlight();
  pinMode(pinWaterLevel, INPUT_PULLDOWN);
  pinMode(pinDHT11,      INPUT_PULLUP);
  pinMode(PIR_PIN,       INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(CO_PIN, INPUT);     // ADC 腳不用特別設定，但寫上可讀性佳
  analogSetPinAttenuation(CO_PIN, ADC_11db); // 單一腳位

  //ledcAttachPin(FAN_PWM_PIN, 0);      // 綁定 Channel 0
  //ledcSetup(0, 25000, 8);             // 頻率25kHz（風扇規格），8位元
  ledcAttach(FAN_PWM_PIN,25000,8);
  // 一般 PWM 頻率設 20~25 kHz，不會有雜音

  pinMode(BUZZER_PIN, OUTPUT);

  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  lcd.clear();  lcd.print("WiFi OK");

  delay(2000); lcd.clear();
}

int readCOppm() {             // 簡化：取 5 次平均回原始 ADC 值
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(CO_PIN);
    delay(2);
  }
  return sum / 5;             // 回傳 0–4095
}

// 計算風扇 Duty (0~255)
int calcFanDuty(int temp, int hum) {
  int dutyT = 0, dutyH = 0;
  if (temp > 32) {
    if (temp >= 36) dutyT = 255;
    else dutyT = (int)((temp - 32) * 63.75);
  }
  if (hum > 20) {
    if (hum >= 50) dutyH = 255;
    else dutyH = (int)((hum - 20) * 8.5);
  }
  int duty = max(dutyT, dutyH);
  if (duty > 255) duty = 255;
  if (duty < 0) duty = 0;
  return duty;
}


void loop() {
  digitalWrite(BUZZER_PIN, HIGH); // 開始叫
  delay(1000);                    // 叫 1 秒
  digitalWrite(BUZZER_PIN, LOW);  // 靜音

  // ── ① 先讀 PIR ──
  int pirState = digitalRead(PIR_PIN);
  Serial.println(pirState ? "有人移動" : "沒人");

  // 若偵測到人體 → 記錄時間 & 立即亮燈
  if (pirState == HIGH) {
    lastMotionMS = millis();
    digitalWrite(LED_PIN, HIGH);
  }

  // 若目前沒偵測到移動，且距離最後一次偵測已超過 KEEP_MS，就關燈
  if ((pirState == LOW) && (millis() - lastMotionMS > KEEP_MS)) {
    digitalWrite(LED_PIN, LOW);
  }

  // ── ② 每 5 輪 (≈1s) 才讀其他感測器 & 顯示 ──
  if (loopCnt % 2 == 0) {
    byte t = 0, h = 0;                      // DHT11
    if (dht11.read(pinDHT11, &t, &h, NULL) == SimpleDHTErrSuccess) {
      int waterState = digitalRead(pinWaterLevel);

      int coRaw = readCOppm();           // <── 新增

      // === 新增風扇調速 ===
      int fanDuty = calcFanDuty(t, h);  // t=溫度, h=濕度
      bool fanWrite = ledcWrite(FAN_PWM_PIN, fanDuty);            // 用 Channel 0，duty 0~255
      Serial.printf("Fan Set: %d, Fan PWM Duty: %d\n", fanWrite, fanDuty);

      // 更新 LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.printf("T:%dC H:%d%% W:%s", t, h, waterState ? "Wet" : "Dry");
      lcd.setCursor(0,1);
      lcd.printf("CO:%4d  ", coRaw);          //    raw or ppm

      /* 序列埠列印 */
      Serial.printf("Temp:%dC Hum:%d%% Water:%s CO_raw:%d\n",
                    t, h, waterState ? "WET":"DRY", coRaw);

      /* 危險狀態示範 */
      if (coRaw > 2000) {                     // ★依實際校準調整門檻
        Serial.println("⚠️ CO 偏高！");
        // tone(BUZZER, 2000, 500);            // 如有蜂鳴器
        // sendData(t, h, waterState, coRaw);  // 上傳含 CO
      }

      // 若要送資料 → 開啟下一行
      // sendData(t, h, waterState);
    } else {
      Serial.println("DHT11 read fail");
    }
  }

  // ── ③ 迴圈計數 & 延遲 ──
  loopCnt = (loopCnt + 1) % 10000;   // 防溢位
  delay(LOOP_DELAY);
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
