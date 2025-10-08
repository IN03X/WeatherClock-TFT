#include <WiFi.h>
#include <Time.h>
#include <DS1302.h>
#include "Wifi_functions.h"
#include "Time_functions.h"
#include "DHT_CODE.h"
#include <TFT_eSPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "font/clock_num_big_64.h"
#include "font/clock_timer_hour_52.h"
#include "font/clock_num_mid_35.h"
#include "font/clock_num_50.h"
#include "font/city_wea_24.h"
#include "img/temperature.h"
#include "img/humidity.h"
#include "img/temperature_black.h"
#include "img/humidity_black.h"
//#include "font/clock_tips_28.h"
//#include "font/clock_timer_24.h"
//#include "font/clock_future_weather_20.h"
#include "font/clock_other_title_35.h"
//#include "font/clock_title_45.h"
//#include "font/air_24.h"
#include "font/qingniaohei_24.h"
#include "font/tianqiyubao_20.h"

#include "img/wea/xue.h"
#include "img/wea/lei.h"
#include "img/wea/shachen.h"
#include "img/wea/wu.h"
#include "img/wea/bingbao.h"
#include "img/wea/yun.h"
#include "img/wea/yu.h"
#include "img/wea/yin.h"
#include "img/wea/qing.h"

#define DHTPIN 32
#define DHTTYPE DHT11
#define BUTTON 22  //触摸按钮

// 初始化 DHT 对象
DHT dht(DHTPIN, DHTTYPE);

// 初始化 DS1302
DS1302 rtc(27, 4, 2);  // RST, DAT, CLK

// 初始化 TFT 屏幕
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite clk = TFT_eSprite(&tft); // 创建 TFT_eSprite 对象

// 定义时间信息
struct tm timeinfo;

// 定义计数器和更新间隔（以秒为单位）
unsigned long updateInterval = 86400;  // 一天的秒数
unsigned long loopCounter = 0;

int Button_down=0; 

//时间显示第一次运行
static bool firstRun_TIME = true;
static bool firstRun_WEA = true;

//表示显示时间或天气的全局变量
enum DisplayMode { DISPLAY_TIME, DISPLAY_WEATHER };
volatile DisplayMode currentMode = DISPLAY_TIME;

//天气查询
String url = "http://apis.juhe.cn/simpleWeather/query";
String url2 = "http://apis.juhe.cn/simpleWeather/wids";
String city = "南京";
String key = ""; // Set Your APIkey

struct WeatherInfo {
    String date;
    String temperature;
    String weather;
    String wid; // 新增字段，用于存储天气的图标ID
};

// 定义结构体数组，用于存储今天和未来三天的天气信息
WeatherInfo weatherData[4];

//天气的显示位置数组
int x[4], y[4];
int iconX[4], iconY[4];
int dateWidth[4], weatherWidth[4], tempWidth[4];
int dateX[4], weatherX[4], tempX[4];




// dayAsString 函数
String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sun.";
    case Time::kMonday: return "Mon.";
    case Time::kTuesday: return "Tue.";
    case Time::kWednesday: return "Wed.";
    case Time::kThursday: return "Thur.";
    case Time::kFriday: return "Fri.";
    case Time::kSaturday: return "Sat.";
  }
  return "(unknown day)";
}

// 获取对应天气图标的数据指针
const uint16_t* getWeatherImage(String wid) {
    if (wid == "00") return qing;           // 晴
    if (wid == "01") return yun;            // 多云
    if (wid == "02") return yin;            // 阴
    if (wid == "03" || wid == "07" || wid == "08" || wid == "09" || wid == "10" || wid == "11" || wid == "21" || wid == "22" || wid == "23" || wid == "24") 
        return yu;                          // 阵雨、小雨、中雨、大雨、暴雨、大暴雨、小到中雨、中到大雨、大到暴雨、暴雨到大暴雨
    if (wid == "04") return lei;            // 雷阵雨
    if (wid == "05") return bingbao;        // 雷阵雨伴有冰雹
    if (wid == "06" || wid == "13" || wid == "14" || wid == "15" || wid == "16" || wid == "17" || wid == "26" || wid == "27" || wid == "28") 
        return xue;                         // 雨夹雪、阵雪、小雪、中雪、大雪、暴雪、小到中雪、中到大雪、大到暴雪
    if (wid == "18") return wu;             // 雾
    if (wid == "19") return yu;             // 冻雨（映射为雨）
    if (wid == "20" || wid == "29" || wid == "30" || wid == "31") 
        return shachen;                     // 沙尘暴、浮尘、扬沙、强沙尘暴
    if (wid == "53") return wu;             // 霾（映射为雾）

    return qing;  // 默认显示晴的图标
}




// 获取并显示时间与温湿度
void getTimeAndDisplay() {
    
    static Time lastTime(0, 0, 0, 0, 0, 0, Time::kSunday);
    static float lastTemp = -100.0;
    static float lastHum = -100.0;

    Time tim = rtc.time();
    const String day = dayAsString(tim.day);

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // 优化字体加载
    static bool fontLoaded = false;
    if (!fontLoaded) {
        tft.loadFont(qingniaohei_24);
        fontLoaded = true;
    }

    /*if(firstRun){
      tft.setSwapBytes(true);
      tft.pushImage(10,183,40,40,humidity);
      tft.pushImage(10,143,40,40,temperature);
      tft.fillRect(0,63,240,2,TFT_PINK);
        
    }*/

    // 年月日更新
    if (firstRun_TIME || tim.yr != lastTime.yr || tim.mon != lastTime.mon || tim.date != lastTime.date) {
        clk.createSprite(220, 30);
        clk.fillSprite(TFT_WHITE);
        clk.loadFont(qingniaohei_24);
        clk.setTextColor(TFT_BLACK);
        clk.setCursor(10, 10, 1);
        clk.printf("%04d-%02d-%02d", tim.yr, tim.mon, tim.date);
        clk.pushSprite(10, 15);  // 左上角位置
        clk.unloadFont();
        clk.deleteSprite();
        tft.fillRect(0,58,240,4,TFT_PINK);

        lastTime.yr = tim.yr;
        lastTime.mon = tim.mon;
        lastTime.date = tim.date;
    }

    // 时、分、秒更新
    if (firstRun_TIME || tim.hr != lastTime.hr || tim.min != lastTime.min || tim.sec != lastTime.sec) {
        clk.createSprite(165, 60);
        clk.fillSprite(TFT_WHITE);
        clk.loadFont(clock_num_big_64);
        clk.setTextDatum(CC_DATUM);
        clk.setTextColor(TFT_BLACK);
        clk.drawNumber(tim.hr / 10, 20, 32);
        clk.drawNumber(tim.hr % 10, 55, 32);
        clk.drawString(":", 80, 32);
        clk.setTextColor(0xFC60);
        clk.drawNumber(tim.min / 10, 110, 32);
        clk.drawNumber(tim.min % 10, 145, 32);
        clk.unloadFont();
        clk.pushSprite(5, 75);  // 保持时分秒在正中间的位置
        clk.deleteSprite();

        clk.createSprite(55, 40);
        clk.fillSprite(TFT_WHITE);
        clk.loadFont(clock_num_mid_35);
        //clock_other_title_35
        clk.setTextDatum(CC_DATUM);
        clk.setTextColor(TFT_RED);
        clk.drawNumber(tim.sec / 10, 12, 20);
        clk.drawNumber(tim.sec % 10, 32, 20);
        clk.unloadFont();
        clk.pushSprite(180, 95);  // 保持时分秒在正中间的位置
        clk.deleteSprite();

        lastTime.hr = tim.hr;
        lastTime.min = tim.min;
        lastTime.sec = tim.sec;
    }

    // 星期更新
    if (firstRun_TIME || tim.day != lastTime.day) {
        int rectWidth;
        if (day.c_str() == "Thur.") {
            rectWidth = 100; // 对于"Thur."设置较宽的矩形
        } else {
            rectWidth = 76; // 对于其他星期设置标准宽度
        }

        clk.createSprite(rectWidth + 20, 36); // 创建足够宽度的 Sprite
        clk.fillSprite(TFT_WHITE);

        // 绘制绿色圆角矩形
        clk.fillRoundRect(10, 0, rectWidth, 36, 10, TFT_GREEN);

        // 设置文本属性并绘制文本
        clk.setTextDatum(CC_DATUM);
        clk.setTextColor(TFT_BLUE);
        clk.loadFont(clock_num_mid_35); // 使用指定字体
        //clock_other_title_35
        clk.drawString(day.c_str(), rectWidth / 2 + 10, 20); // 在矩形内绘制文本
        clk.unloadFont();

        // 确定 Sprite 的绘制位置
        if (day.c_str() == "Thur.") {
            clk.pushSprite(130, 16);
        } else {
            clk.pushSprite(150, 16);  // 右上角位置
        }

        clk.deleteSprite();
        lastTime.day = tim.day;
    }

    // 通用的进度条绘制函数
    auto drawProgressBar = [&](int x, int y, float value, int maxValue, uint16_t color) {
        int length = constrain((int)((value / maxValue) * 63), 0, 63);
        clk.createSprite(65, 9);
        clk.fillSprite(TFT_WHITE);
        clk.drawRoundRect(0, 0, 65, 9, 2, TFT_BLACK);
        clk.drawRoundRect(1, 0, 63, 9, 2, TFT_BLACK);
        clk.drawFastHLine(1, 1, 61, TFT_BLACK);
        clk.drawFastHLine(1, 7, 61, TFT_BLACK);
        clk.fillRoundRect(1, 2, length, 5, 2, color);
        clk.pushSprite(x, y);
        clk.deleteSprite();
    };

          // 温度更新
      if (firstRun_TIME || t != lastTemp) {
        drawProgressBar(57, 163, t, 40, (t >= 30) ? TFT_RED : (t >= 25) ? 0xFC60 : (t > 0) ? 0x0E27 : TFT_DARKGREY);
        tft.setSwapBytes(true);
        tft.pushImage(10,143,40,40,temperature);
        
        clk.createSprite(80, 28); // 增加了宽度以容纳符号
        clk.fillSprite(TFT_WHITE);
        clk.setTextColor(TFT_BLACK);
        clk.loadFont(qingniaohei_24); // 使用指定字体
        clk.setTextDatum(TL_DATUM);
        clk.setCursor(0, 3); // 从起始位置开始
        clk.printf("%.1f", t);
        clk.drawString("℃", clk.textWidth("%.1f", t), 3); // 符号接在数字后面
        clk.unloadFont();
        clk.pushSprite(137, 157);  // 推送到屏幕的指定位置
        clk.deleteSprite();

        lastTemp = t;
    }

      // 湿度更新
    if (firstRun_TIME || h != lastHum) {
        drawProgressBar(57, 203, h, 100, 0x2C3E);
        tft.setSwapBytes(true);
        tft.pushImage(10,183,40,40,humidity);

        clk.createSprite(80, 24); // 增加了宽度以容纳符号
        clk.fillSprite(TFT_WHITE);
        clk.setTextColor(TFT_BLACK);
        clk.loadFont(qingniaohei_24); // 使用指定字体
        clk.setTextDatum(TL_DATUM);
        clk.setCursor(0, 0); // 从起始位置开始
        clk.printf("%.1f", h);
        clk.drawString("%", clk.textWidth("%.1f", h)+3, 0); // 符号接在数字后面
        clk.unloadFont();
        clk.pushSprite(137, 200);  // 推送到屏幕的指定位置
        clk.deleteSprite();

        lastHum = h;
    }


    char buf[50];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d %s",
            tim.yr, tim.mon, tim.date, tim.hr, tim.min, tim.sec, day.c_str());
    Serial.println(buf);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");

    firstRun_TIME = false;
}


void showWeather() {
    // 清屏并开始显示天气信息
    clk.setSwapBytes(true);
    clk.loadFont(tianqiyubao_20); // 使用指定字体
    delay(30);

    // 提前初始化Sprite并加载字体，确保数据结构正确初始化
    clk.createSprite(10, 10);
    clk.fillSprite(TFT_WHITE);
    clk.setTextDatum(TL_DATUM);
    clk.setTextColor(TFT_BLACK, TFT_WHITE);

    clk.drawString(" ", 0, 0); // 确保初始化完成
    clk.pushSprite(0, 0);
    clk.deleteSprite();
    
    // 定义每一块区域的大小
    int blockWidth = 120;
    int blockHeight = 120;
    int iconSize = 50; // 图标的大小，假设图标是50x50的
    int dateY = 53, weatherY = 75, tempY = 98;
   
    // 计算每个块的位置和文本的X坐标
    for (int i = 0; i < 4; i++) {
        x[i] = (i % 2) * blockWidth; // 左右位置
        y[i] = (i / 2) * blockHeight; // 上下位置

        // 计算图标的位置
        iconX[i] = (blockWidth - iconSize) / 2; // 图标水平居中
        iconY[i] = 5; // 图标距顶部5像素

        // 计算日期文本的宽度和X坐标
        String date = weatherData[i].date.substring(5); // 提取 "MM-DD" 部分
        dateWidth[i] = clk.textWidth(date); // 计算日期文本的宽度
        dateX[i] = (blockWidth - dateWidth[i]) / 2; // 日期文本水平居中

        // 计算天气文本的宽度和X坐标
        String weather = weatherData[i].weather;
        weatherWidth[i] = clk.textWidth(weather); // 计算天气文本的宽度
        weatherX[i] = (blockWidth - weatherWidth[i]) / 2; // 天气文本水平居中

        // 计算温度文本的宽度和X坐标
        String temperature = weatherData[i].temperature;
        tempWidth[i] = clk.textWidth(temperature); // 计算温度文本的宽度
        tempX[i] = (blockWidth - tempWidth[i]) / 2; // 温度文本水平居中
    }

    // 显示天气信息
    for (int i = 0; i < 4; i++) {
        // 创建Sprite对象
        clk.createSprite(blockWidth, blockHeight);
        clk.fillSprite(TFT_WHITE);

        // 获取天气图标的数据指针
        const uint16_t* weatherIcon = getWeatherImage(weatherData[i].wid);
        // 显示天气图标
        clk.pushImage(iconX[i], iconY[i], iconSize, iconSize, weatherIcon);
        
        // 显示日期（只显示月和日）
        String date = weatherData[i].date.substring(5); // 提取 "MM-DD" 部分
        clk.drawString(date, dateX[i], dateY);
        Serial.println(dateX[i]);

        // 显示天气
        String weather = weatherData[i].weather;
        clk.drawString(weather, weatherX[i], weatherY);
        Serial.println(weatherX[i]);

        // 显示温度
        String temperature = weatherData[i].temperature;
        clk.drawString(temperature, tempX[i], tempY);
        Serial.println(tempX[i]);

        // 在每个区域内绘制分界线
        if (i == 0 || i == 2) {
            clk.fillRect(119, 0, 1, blockHeight, 0x2C3E); // 区域0和2的垂直分界线
        } else if (i == 1 || i == 3) {
            clk.fillRect(0, 0, 2, blockHeight, 0x2C3E); // 区域1和3的垂直分界线
        }

        // 绘制水平分界线
        if (i == 2 || i == 3) {
            clk.fillRect(0, 4, blockWidth, 3, 0x2C3E); // 下两个区域的水平分界线
        }

        // 将Sprite推送到屏幕上的相应位置
        clk.pushSprite(x[i], y[i]);
        
        // 删除Sprite对象
        clk.deleteSprite();
    }
    clk.unloadFont();
    
}

//触摸按键检测函数
void IRAM_ATTR handleButtonPress() {
    if (digitalRead(BUTTON) == LOW) {
        Serial.println("Button pressed!");
        // 切换模式
        currentMode = (currentMode == DISPLAY_TIME) ? DISPLAY_WEATHER : DISPLAY_TIME;
        tft.fillScreen(TFT_WHITE);
        firstRun_TIME=true;
    }
}

void getWeather(){
  // 创建 HTTPClient 对象
  HTTPClient http;
  // 指定访问 URL
  http.begin(url + "?city=" + city + "&key=" + key);
  // 接收 HTTP 响应状态码
  int http_code = http.GET();
  Serial.printf("HTTP 状态码：%d\n", http_code);
  // 获取响应正文
  String response = http.getString();
  Serial.print("响应数据：");
  Serial.println(response);
  // 关闭连接
  http.end();

  //查询今天和未来的天气
  // 创建 DynamicJsonDocument 对象
  DynamicJsonDocument doc(2048);  // 提升容量以适应较大的 JSON 数据
  // 解析 JSON 数据
  deserializeJson(doc, response);

  // 提取今天和未来三天的天气信息并存储到 weatherData 数组中
  for (int i = 0; i < 4; i++) {
      String date = doc["result"]["future"][i]["date"].as<String>();
      String temperature = doc["result"]["future"][i]["temperature"].as<String>();
      String weather = doc["result"]["future"][i]["weather"].as<String>();
      String widDay = doc["result"]["future"][i]["wid"]["day"].as<String>(); // 提取 day 部分

      weatherData[i].date = date;
      weatherData[i].temperature = temperature;
      weatherData[i].weather = weather;
      weatherData[i].wid = widDay; // 使用 day 部分

      // 打印每一天的天气信息
      Serial.printf("日期：%s, 温度：%s, 天气：%s, wid:%s\n",
                    weatherData[i].date.c_str(), weatherData[i].temperature.c_str(), weatherData[i].weather.c_str(), weatherData[i].wid.c_str());
  }

}



void setup() {
  int WiFi_init_judge=0;
  //初始化触摸按钮
  pinMode(BUTTON,INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonPress, FALLING);
  Serial.begin(115200);
  // 初始化 DHT11
  DHT11_init(dht);

  // 初始化 TFT 屏幕
  tft.init();
  tft.fillScreen(TFT_WHITE);
  tft.loadFont(qingniaohei_24);
  //tft.setRotation(1); // 设置屏幕旋转

  // 初始化 clk（TFT_eSprite）
  clk.setColorDepth(8); // 设置颜色深度为6位
  clk.createSprite(240, 240); // 创建一个240x240像素的缓冲区，大小与屏幕匹配

  // 初始化 Wi-Fi
  WiFi_init_judge=WiFi_init(tft);

  // 获取 NTP 时间并更新 DS1302
  struct tm currentTime;
  if (WiFi_init_judge == 1) {
    currentTime = syncWithNTP(rtc, tft);
  }//如果后面需要用到currenttime可以直接使用，表示现在的时间

  getWeather();
  // 关闭 Wi-Fi 以节省功耗
  WiFi_disconnect();

}

void loop() {
  // 根据当前模式选择要显示的内容
  if (currentMode == DISPLAY_TIME) {
    // if(firstRun_WEA == false){
    //   // firstRun_WEA=true;
    //   tft.fillScreen(TFT_WHITE);
    // }
    getTimeAndDisplay();
    
  } else if (currentMode == DISPLAY_WEATHER) {
    if(firstRun_WEA == true){
      // 模拟按键中断处理（强制调用两次）
      getTimeAndDisplay();
      tft.fillScreen(TFT_WHITE);
      tft.loadFont(qingniaohei_24);
      tft.setTextColor(TFT_BLACK);
      tft.setCursor(10, 50);
      tft.println("正在连接服务器...");
      delay(500);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(10, 140);
      tft.println("连接成功！");
      tft.unloadFont();
      delay(500); 
      tft.fillScreen(TFT_WHITE);
      showWeather();
      firstRun_WEA=false;
    }
    else{
      showWeather();
    }
    
  }

  // 延时一秒
  delay(1000);

  // 每隔一段时间重新同步
  loopCounter++;

  // 检查是否达到了更新间隔
  if (loopCounter >= updateInterval) {
    Serial.println("正在重新同步时间...");
    WiFi_init(tft);

    // 重新获取 NTP 时间并更新 DS1302
    syncWithNTP(rtc, tft);

    // 提取今天和未来三天的天气信息并存储到 weatherData 数组中
    getWeather();

    // 关闭 Wi-Fi 并重置计数器
    WiFi_disconnect();
    loopCounter = 0;  // 重置计数器
  }
}
