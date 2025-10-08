/*#include <WiFi.h>

// WiFi 账号密码
const char *ssid = "HUAWEI-LHBKG5";
const char *password = "13556400";

// 连接 Wi-Fi
void WiFi_init() {
  // 设置 ESP32 工作模式
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);  
  }
  Serial.println("WiFi 连接成功");
}

// 断开 Wi-Fi 连接
void WiFi_disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("Wi-Fi 已关闭");
}

// 检查 Wi-Fi 状态
bool WiFi_isConnected() {
  return WiFi.status() == WL_CONNECTED;
}*/
#include <WiFi.h>
#include <TFT_eSPI.h>

const char *ssid = "360T6G";
const char *password = "wrb054528";

int WiFi_init(TFT_eSPI &tft) {
  const int max_try_times=15;
  int try_times=0;
  tft.fillRect(0, 0, 240, 40, TFT_WHITE); // 清除之前的内容
  tft.setCursor(10, 50);
  tft.setTextColor(TFT_RED);
  tft.println("正在连接 WiFi . . . .");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && try_times<=max_try_times) {
    delay(500);
    try_times++;
  }
  //tft.fillRect(0, 0, 240, 40, TFT_WHITE); // 清除之前的内容
  if(try_times>=max_try_times){
    tft.setCursor(10, 140);
    tft.setTextColor(TFT_GREEN);
    tft.println("WiFi 连接失败！");
    delay(500);
    tft.setCursor(10, 170);
    tft.println("使用本地时间！");
    delay(1000);
    return 0;
  }
  tft.setCursor(10, 140);
  tft.setTextColor(TFT_GREEN);
  tft.println("WiFi 连接成功！");
  delay(1000);
  tft.fillScreen(TFT_WHITE);
  return 1;
}

void WiFi_disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("Wi-Fi 已关闭");
}
