#include "DHT_CODE.h"

void DHT11_init(DHT& dht) {
  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void DHT11_printinfo(DHT& dht) {
  float humid = dht.readHumidity();
  // 读取温度
  float temp = dht.readTemperature();

  // 显示内容
  Serial.print("湿度: ");
  Serial.print(humid);
  Serial.print("% 温度: ");
  Serial.print(temp);
  Serial.println("°C ");
}
