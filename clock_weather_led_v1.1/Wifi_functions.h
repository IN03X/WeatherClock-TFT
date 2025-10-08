#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include <TFT_eSPI.h>

// WiFi 账号密码
extern const char *ssid;
extern const char *password;

// 连接 Wi-Fi
int WiFi_init(TFT_eSPI &tft);

// 断开 Wi-Fi 连接
void WiFi_disconnect();

// 检查 Wi-Fi 状态
bool WiFi_isConnected();

#endif // WIFI_FUNCTIONS_H
