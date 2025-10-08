#ifndef TIME_FUNCTIONS_H
#define TIME_FUNCTIONS_H

#include <TFT_eSPI.h>
#include <DS1302.h> // 使用的是 DS1302 库

// 设置 DS1302 的时间
void setRTCTime(struct tm *timeinfo, DS1302 &rtc);

// 使用 NTP 更新 DS1302 时间
struct tm syncWithNTP(DS1302 &rtc, TFT_eSPI &tft);

#endif // TIME_FUNCTIONS_H
