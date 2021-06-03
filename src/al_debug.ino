#include "config.h"
//调试开关
void set_dbug(uint8_t a)
{
  dbug = a;
}

//设置GSM发送间隔：（S）
void SET_SLEEPTIME(time_t t)
{
  sleeptime = t;
  eeprom_config_save_parameter();
  sleeptime = (time_t)EEPROM.readLong(2);
  Serial.printf("sleeptime:%ld\r\n", sleeptime);
}

//设置实时时钟
void SET_ds1302(int year, char momtch, char day, char hour, char minute, char second)
{
  now1.year = year;
  now1.month = momtch;
  now1.day = day;
  now1.hour = hour;
  now1.minute = minute;
  now1.second = second;
  ds_rtc.setDateTime(&now1);
  now_unixtime = unixtime();
}
//设定飞航模式 1飞航 2工作
void set_fxmode(char a, char b, char c)
{
  f_Flight_Mode = a;
  workingState = b;
  dbug = c;
}
//读LIST文件系统
void read_list()
{
  Serial.printf("postMsgId = %d\n;", postMsgId);
  alFFS_readlist();
}

//文件系统使用情况
void spiffs_size()
{
  Serial.printf("FFS SIZE:%d\n", SPIFFS.totalBytes());
  Serial.printf("FFS USE SIZE:%d\n", SPIFFS.usedBytes());
}
//格式化文件系统
void FFS_fromat()
{
  bool i = SPIFFS.format();
  if (!i)
    Serial.printf("FFS_fromat is fail!\n");
  else
    Serial.printf("FFS_fromat ok!\n");
}
//核对系统时间
void sys_time()
{
  //111111screen_loopEnabled = false;
  setupModem();
  modemToGPRS();
  if (getLBSLocation())
    Serial.println("sys_time ok!\n");
  //11111screen_loopEnabled = false;
  screenState = MAIN_TEMP_SCREEN;
}

void switch_sleep(bool x)
{
     // if (oledState == OLED_ON)
      if (!x)
  {
    oledState = OLED_OFF;
    display.displayOff();
    Serial.println("displayOff");
       eeprom_config_save_parameter();
    delay(500);
   // esp_sleep_enable_ext0_wakeup(WEAKUPKEY1, LOW); //使能按键唤醒
    esp_sleep_enable_ext0_wakeup(WEAKUPKEY2, LOW); //使能按键唤醒
    digitalWrite(MODEM_POWER_ON, LOW);
    gpio_hold_en(GPIO_NUM_32);                     //锁定电源管脚
    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();
  }
  else 
  {
    display.displayOn();
    oledState = OLED_ON;
    Serial.println("displayOn");
  }
}

