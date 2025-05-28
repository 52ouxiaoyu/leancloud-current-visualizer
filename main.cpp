#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include "esp_task_wdt.h"

// WiFi 配置
const char* ssid = "desloc_testingwifi_google";
const char* password = "ASDFGvcxz";

// LeanCloud 配置（中国区）
const char* LEANCLOUD_API_URL = "https://bojar1gs.lc-cn-n1-shared.com/1.1/classes/CurrentData";
const char* APP_ID = "bOJAr1GSAwLB55qdhahyz0tp-gzGzoHsz";
const char* APP_KEY = "gc9TLLQQwb6pYMFF8kpFJlLh";

// NTP 时间服务器
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600; // 中国时间
const int daylightOffset_sec = 0;

Adafruit_INA219 ina219;
String deviceId;
String currentList = "";
int sampleCount = 0;

#define WDT_TIMEOUT 10 // 看门狗超时时间（秒）

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {  // 使用时间控制替代固定次数
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi 已连接");
  } else {
    Serial.println("WiFi 连接失败，重启设备");
    ESP.restart();
  }
}

String getISOTimeUTC() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "";
  }
  
  char isoTime[30];
  // 添加Z后缀表示UTC时间
  strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
  return String(isoTime);
}

// 使用动态分配的 String 替代固定字符串拼接
void uploadToLeanCloud(const String& currentList) {
  String isoTime = getISOTimeUTC();
  if (isoTime.isEmpty()) {
    Serial.println("获取时间失败，跳过本次上传");
    return;
  }
  
  String payload;
  payload.reserve(512);  // 预分配内存
  payload = "{\"currents\":\"" + currentList + "\",";
  payload += "\"timestamp\": {\"__type\": \"Date\", \"iso\": \"" + isoTime + "\"},";
  payload += "\"deviceId\": \"" + deviceId + "\"";
  payload += "}";

  Serial.println("上传内容：");
  Serial.println(payload);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  // 将 http 对象移到这个作用域内
    http.begin(LEANCLOUD_API_URL);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-LC-Id", APP_ID);
    http.addHeader("X-LC-Key", APP_KEY);

    int responseCode = http.POST(payload);
    String response = http.getString();

    if (responseCode >= 200 && responseCode < 300) {
      Serial.println("上传成功: " + String(responseCode));
      Serial.println("返回: " + response);
    } else {
      Serial.println("上传失败: " + String(responseCode));
      Serial.println("返回: " + response);
    }

    http.end();
  } else {
    Serial.println("WiFi 未连接，尝试重连");
    connectToWiFi();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  ina219.begin();

  connectToWiFi();
  configTime(0, 0, ntpServer);  // 移除时区偏移，直接使用 NTP 时间
  delay(2000);

  deviceId = WiFi.macAddress();
  deviceId.replace(":", "");

  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
}

float getCurrentValue() {
  float current = ina219.getCurrent_mA();
  // 过滤异常值（比如超过10A的电流）
  if(current >= -10000 && current <= 10000) {
    return current;
  }
  return 0.0;  // 如果是异常值则返回0
}

void loop() {
  esp_task_wdt_reset();

  float current_mA = getCurrentValue();
  Serial.print("采样电流: ");
  Serial.println(current_mA);

  if (currentList.length() > 0) {
    currentList += "_";
  }
  currentList += String(current_mA, 2);

  sampleCount++;

  if (sampleCount >= 60) {
    uploadToLeanCloud(currentList);
    sampleCount = 0;
    currentList = "";
  }

  delay(1000);  // 每秒采样一次
}
