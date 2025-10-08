#include "Time_functions.h"
#include <TFT_eSPI.h>
#include "font/city_wea_24.h"
#define NTP "ntp.aliyun.com"

void setRTCTime(struct tm *timeinfo, DS1302 &rtc) {
  rtc.writeProtect(false);
  rtc.halt(false);
  Time t(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
         timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, (Time::Day)(timeinfo->tm_wday + 1));
  rtc.time(t);
}

struct tm syncWithNTP(DS1302 &rtc, TFT_eSPI &tft) {
  tft.fillRect(0, 40, 240, 40, TFT_WHITE); // 清除之前的内容
  tft.setCursor(10, 50);
  //tft.loadFont(city_wea_24);
  tft.setTextColor(TFT_BLACK);
  tft.println("正在连接 NTP 服务器");

  configTime(8*3600, 0, NTP);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    setRTCTime(&timeinfo, rtc);
    tft.setCursor(10, 140);
    tft.setTextColor(TFT_GREEN);
    tft.println("时间同步成功！");
    delay(1000);
  } else {
    tft.setCursor(10, 140);
    tft.setTextColor(TFT_RED);
    tft.println("时间同步失败！");
    delay(500);
    tft.setCursor(10, 170);
    tft.println("使用本地时间！");
    delay(1000);
  }
  //tft.unloadFont();
  tft.fillScreen(TFT_WHITE);

  return timeinfo; // 返回当前时间
}

