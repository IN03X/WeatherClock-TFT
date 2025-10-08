#ifndef DHT_CODE_H
#define DHT_CODE_H

#include "DHT.h"

// 初始化DHT11传感器
void DHT11_init(DHT& dht);

// 打印DHT11传感器的湿度和温度信息
void DHT11_printinfo(DHT& dht);

#endif // DHT_CODE_H
