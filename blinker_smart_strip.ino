///////////////////////////////////////////////////////////////////
////                                                           ////
////                     blinker smart strip                   ////
////                      blinker  智能插排                     ////
////                            0.1                            ////
///////////////////////////////////////////////////////////////////

#define BLINKER_PRINT Serial
#define BLINKER_MQTT
//#define BLINKER_ESP_SMARTCONFIG //在api.h里改了库，这里不用宏定义了
#include <Blinker.h>
#include "EEPROM.h"

int addr = 0;//eeprom地址变量
#define EEPROM_SIZE 64//eeprom空间大小

char auth[] = "cd27fec96c1a";
char ssid[30] ;
char pswd[30] ;
bool Button1_status=0,Button2_status=0,Button3_status=0,Button4_status=0;   //按键状态标志位
int16_t start_time_year,start_time_month,start_time_day,start_time_hour,start_time_min,start_time_sec;  //开机时间
int16_t operation_time_year,operation_time_month,operation_time_day,operation_time_hour,operation_time_min,operation_time_sec; //运行时间
bool start_flag=0;
#define LED_1 23
#define LED_2 22
#define btn_re1 "btn_re1" //为统一操作，app上的按钮用btn，硬件上的物理按钮用key
#define btn_re2 "btn_re2"
#define btn_re3 "btn_re3"
#define btn_re4 "btn_re4"
#define btn_response "btn_response"
#define num_1 "num_1"
#define num_2 "num_2"
#define col_rgb "col_rgb"
void btn_re1_callback(const String & state);
void btn_re2_callback(const String & state);
void btn_re3_callback(const String & state);
void btn_re4_callback(const String & state);
void btn_response_callback(const String & state);
void col_rgb_callback(uint8_t r_value, uint8_t g_value, 
                    uint8_t b_value, uint8_t bright_value);
BlinkerButton Button1(btn_re1,btn_re1_callback);
BlinkerButton Button2(btn_re2,btn_re2_callback);
BlinkerButton Button3(btn_re3,btn_re3_callback);
BlinkerButton Button4(btn_re4,btn_re4_callback);
BlinkerButton Button5(btn_response,btn_response_callback);
BlinkerNumber Num1(num_1);
BlinkerNumber Num2(num_2);
BlinkerRGB RGB(col_rgb,col_rgb_callback);
BlinkerTimer BlinkerLoop;
void blink() {
    digitalWrite(LED_2, !digitalRead(LED_2));
    //BLINKER_LOG2("Now ntp time: ", Blinker.time());
}
void smart_config_setup()
{
  BLINKER_LOG2("进入smart config",1);
  if (!EEPROM.begin(EEPROM_SIZE))//初始化eeprom空间
    {
      BLINKER_LOG2("EEPROM初始化失败",1); delay(100000);
    }
  if(EEPROM.read(1)!=1)//如果首次初始化，则将空间清零
    {
      BLINKER_LOG2("EEPROM首次初始化",1);
      for (addr = 0; addr < EEPROM_SIZE; addr++)
      {
        EEPROM.write(addr, 0);
      }
      EEPROM.write(0, 1);//使能smart config
      EEPROM.write(1, 1);//标记已经初始化过
      EEPROM.commit();
      BLINKER_LOG2("EEPROM初始化成功",1);
    }  
    if(EEPROM.read(0)==1)//如果使能smart config
    {
      BLINKER_LOG2("smart config 开始",1);
      Blinker.begin(auth);
      for (addr = 2; addr < EEPROM_SIZE; addr++)
      {
        EEPROM.write(addr, 0);//eeprom部分空间清零
      }
      memcpy(ssid, WiFi.SSID().c_str(), 30);
      memcpy(pswd, WiFi.psk().c_str(), 30);
      EEPROM.put(2, ssid);
      EEPROM.put(32, pswd);//将连接上的wifi的ssid密码存入eeprom
      EEPROM.write(0, 0);//禁能smart config 下次上电直接连接
      EEPROM.commit();
    }
    else//如果禁能了smart config
    {
      BLINKER_LOG2("不需要smart config",1);
      EEPROM.get(2, ssid);  //从eeprom中获取ssid密码
      EEPROM.get(32, pswd);
      Blinker.begin(auth,ssid,pswd);//直接连接
    }
}
void setup() {
    Serial.begin(115200);
    //Serial2.begin(115200);

    smart_config_setup();
    
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    
   
    Blinker.setTimezone(8.0);   //设定时区


    BlinkerLoop.loop(0.2, blink);   //0.2s定时器初始化
    Blinker.attachHeartbeat(refresh_screen);    //心跳包回调函数初始化

}

void loop()
{
    Blinker.run();
    if(!start_flag&&Blinker.connected())
    {
        Blinker.delay(500);
        start_time_year = Blinker.year();
        start_time_month = Blinker.month();
        start_time_day = Blinker.mday();
        start_time_hour = Blinker.hour();
        start_time_min = Blinker.minute();
        start_time_sec = Blinker.second();
        start_flag = 1;
    }
    if (Blinker.available()) {
        BLINKER_LOG2("Blinker.readString(): ", Blinker.readString()); 

    }
   
}

void btn_re1_callback(const String & state)
{
    String get_weather = Blinker.weather();
    BLINKER_LOG2("weather: ", get_weather);
    Blinker.delay(6000);
    BLINKER_LOG2("btn1 get button state: ", state);
    Button1.icon("fal fa-power-off");
    //Button1.text("开关1");
    Button1_status = ! Button1_status;
    if(Button1_status)
    {
      Button1.color("#FFA500");
      Button1.print("on");
    }
    else
    {
      Button1.color("#696969");
      Button1.print("off");
    }
}
void btn_re2_callback(const String & state)
{
    if (!EEPROM.begin(EEPROM_SIZE))//初始化eeprom空间
    {
      BLINKER_LOG2("EEPROM初始化失败",1); delay(100000);
    }
    //EEPROM.write(0, 1);//使能smart config 下次上电进入smart config
    //EEPROM.commit();
    //Blinker.delay(1000);
    BLINKER_LOG2("btn2 get button state: ", state);
    Button2.icon("fal fa-power-off");
    //Button2.text("开关2");
    Button2_status = ! Button2_status;
    if(Button2_status)
    {
      Button2.color("#FFA500");
      Button2.print("on");
    }
    else
    {
      Button2.color("#696969");
      Button2.print("off");
    }
    //esp_restart();
}
void btn_re3_callback(const String & state)
{
    BLINKER_LOG2("btn3 get button state: ", state);
    Button3.icon("fal fa-power-off");
    //Button3.text("开关3");
    Button3_status = ! Button3_status;
    if(Button3_status)
    {
      digitalWrite(LED_1, 0);
      Button3.color("#FFA500");
      Button3.print("on");
    }
    else
    {
      digitalWrite(LED_1, 1);
      Button3.color("#696969");
      Button3.print("off");
    }
}
void btn_re4_callback(const String & state)
{
    BLINKER_LOG2("btn4 get button state: ", state);
    Button4.icon("fal fa-power-off");
    //Button4.text("开关4","开关4");
    Button4_status = ! Button4_status;
    if(Button4_status)
    {
      Button4.color("#FFA500");
      Button4.print("on");
    }
    else
    {
      Button4.color("#696969");
      Button4.print("off");
    }
}
void btn_response_callback(const String & state)
{
    BLINKER_LOG2("btn5 get button state: ", state);
    Blinker.beginFormat();
    time_refresh();
    Blinker.endFormat();
}

void refresh_screen(void)   //心跳包回调函数
{
    if(Button1_status)
    {
      Button1.color("#FFA500");
      Button1.print("on");
    }
    else
    {
      Button1.color("#696969");
      Button1.print("off");
    }
    if(Button2_status)
    {
      Button2.color("#FFA500");
      Button2.print("on");
    }
    else
    {
      Button2.color("#696969");
      Button2.print("off");
    }
    if(Button3_status)
    {
      Button3.color("#FFA500");
      Button3.print("on");
    }
    else
    {
      Button3.color("#696969");
      Button3.print("off");
    }
    if(Button4_status)
    {
      Button4.color("#FFA500");
      Button4.print("on");
    }
    else
    {
      Button4.color("#696969");
      Button4.print("off");
    }

    time_refresh();
}

void time_refresh(void) //时间更新函数
{
    operation_time_year = Blinker.year() - start_time_year;
    operation_time_month = Blinker.month() - start_time_month;
    operation_time_day = Blinker.mday() - start_time_day;
    operation_time_hour = Blinker.hour() - start_time_hour;
    operation_time_min = Blinker.minute() - start_time_min;
    operation_time_sec = Blinker.second() - start_time_sec;
    if(operation_time_sec<0)
    {
        operation_time_sec +=60;
        operation_time_min -=1;
    }
    if(operation_time_min<0)
    {
        operation_time_min +=60;
        operation_time_hour -=1;
    }
    if(operation_time_hour<0)
    {
        operation_time_hour +=24;
        operation_time_day -=1;
    }
    if(operation_time_day<0)
    {
        operation_time_month +=12;
        operation_time_year -=1;
    }
    Num1.icon("fal fa-clock");
    Num1.color("#008000");
    Num2.icon("fal fa-clock");
    Num2.color("#008000");
    if(operation_time_year==0&&operation_time_month==0&&operation_time_day==0&&operation_time_hour==0)
    {
        Num1.unit("min");
        Num1.print(operation_time_min);
        Num2.unit("sec");
        Num2.print(operation_time_sec);
    }
    else if(operation_time_year==0&&operation_time_month==0&&operation_time_day==0)
    {
        Num1.unit("hour");
        Num1.print(operation_time_hour);
        Num2.unit("min");
        Num2.print(operation_time_min);
    }
    else if(operation_time_year==0&&operation_time_month==0)
    {
        Num1.unit("day");
        Num1.print(operation_time_day);
        Num2.unit("hour");
        Num2.print(operation_time_hour);
    }
    else if(operation_time_year==0)
    {
        Num1.unit("month");
        Num1.print(operation_time_month);
        Num2.unit("day");
        Num2.print(operation_time_day);
    }
    else
    {
        Num1.unit("year");
        Num1.print(operation_time_year);
        Num2.unit("month");
        Num2.print(operation_time_month);
    }
}

void col_rgb_callback(uint8_t r_value, uint8_t g_value, 
                    uint8_t b_value, uint8_t bright_value)
{
    BLINKER_LOG2("R value: ", r_value);
    BLINKER_LOG2("G value: ", g_value);
    BLINKER_LOG2("B value: ", b_value);
    BLINKER_LOG2("Rrightness value: ", bright_value);
    RGB.print(0,0,0,0);
}
