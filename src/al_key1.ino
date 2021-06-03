#include "config.h"

void nullfunc()
{ 
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
//双击
void doubleclick()
{
  //1.如果屏亮着改变keystate状态
  //2更新屏亮起始时间
  //3.更新息屏时间
  if (oledState == OLED_ON)
  {
    Serial.println("doubleclick");
    keyState = DOUBLECLICK;
  }
  screen_On_Start = sys_sec;
  screen_On_now = sys_sec;
}
//单击
void click()
{
  //1.如果屏亮着keyState = CLICK
  //2.更新屏亮参数
  if (oledState == OLED_ON)
  {
    Serial.println("click");
    keyState = CLICK;
  }
  screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
//长按开始
void longPressStart()
{
  //1.如果平亮着keyState = LONGPRESS_START;
  //2.更新屏亮参数
  if (oledState == OLED_ON)
  {
    Serial.println("longPressStart");
    keyState = LONGPRESS_START;
  }
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
}
//长按过程中
void duringLongPress()
{
  //1.如果平亮着更新屏亮参数
  //2.获取当前长按状态函数，显示按下时长，更新按键参数，keyState = LONGPRESS_DURRING;
  if (oledState == OLED_ON)
  {
   
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  }
  if (button.isLongPressed())
  {
    Serial.print("duringLongPress:");
    Serial.println(button.getPressedTicks());
    keyScreen_Start = sys_sec; //处理长按界面下的长按
    keyScreen_Now = sys_sec;   //处理长按界面下的长按
    delay(50);
    keyState = LONGPRESS_DURRING;
  }
}
//长按停止
void longPressStop()
{
  //1.更新屏亮参数
  // 2.keyState = LONGPRESS_END;
  screen_On_Start = sys_sec;
  screen_On_now = sys_sec;
  Serial.println("longPressStop");
  keyState = LONGPRESS_END;
}




//button2长按停止
void duringLongPres2()
{
  Serial.printf("into \n");
  //delay(3000);
  //eeprom_config_save_parameter();
  //1.更新屏亮参数
  // 2.keyState = LONGPRESS_END;
  if (oledState == OLED_ON)
  {
     oledState = OLED_OFF;
     display.displayOff();
     delay(50);
     display.displayOff();
     delay(100);
    
    delay(5000);
    Serial.println("displayOff");
    esp_sleep_enable_ext0_wakeup(WEAKUPKEY2, LOW); //使能按键唤醒
    digitalWrite(MODEM_POWER_ON, LOW);
    gpio_hold_en(GPIO_NUM_32);                     //锁定电源管脚
    Serial.println("displayOfflsdjfsdkjff");
    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();

  }
  // else
  // {
  //     workingState = NOT_WORKING;
  //   Serial.println("workingState = NOT_WORKING;2");

  //   oledState = OLED_ON;
  //   list_first_flag=true;
  //   lose_first_flag=true;
  //   postMsgId = 0; //清记录条数
  //   lose_count=0;
  //   deleteFile(SPIFFS, "/list.json");
  //   deleteFile(SPIFFS, "/lose.hex");
  //   ds_rtc.getDateTime(&now1); 
  //   Serial.printf("time now1: %d-%d-%d %d:%d:%d\r\n", now1.year, now1.month, now1.day,now1.hour, now1.minute, now1.second);
  //   last_rec_stamp = now_unixtime;//刷新最后发送时间
  //   eeprom_config_save_parameter();
  // }



}



//按键扫描
void key_loop()
{
  //根据按键状态，调整状态
  switch (keyState)
  {
  case CLICK:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN)
    {
      //1111111screen_loopEnabled = false;
      screenState = TIPS_SCREEN;
      keyScreen_Start = sys_sec;
      keyScreen_Now = sys_sec;
      keyState = NOKEYDOWN;
    }
    else if (screenState == TIPS_SCREEN || screenState == BLE_SCREEN || screenState == REC_STOP_SCREEN|| screenState == REC_START_SCREEN)
    {
      //1111111screen_loopEnabled = false;
      Serial.println("anjianfanhui");
      keyState = NOKEYDOWN;
    }
    break;
  case DOUBLECLICK:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN || screenState == TIPS_SCREEN)
    {
      if(f_Flight_Mode==true)//正在飞行模式，关闭。
      {
         f_Flight_Mode=false;
         workingState=NOT_WORKING;
         //1111111screen_loopEnabled = false;
         screenState = fxmod_OFF;
         qualifiedState = QUALITIFY_WRONG ;
         Serial.printf("flight off\n");
         keyScreen_Start = sys_sec;
         keyScreen_Now = sys_sec;
         keyState = NOKEYDOWN;

      }
      else if(f_Flight_Mode==false)//关闭时飞行模式，打开
      {
         f_Flight_Mode=true;
         workingState=WORKING;
         //1111111screen_loopEnabled = false;
         screenState = fxmod_ON;
         
         qualifiedState = QUALITIFY_RIGHT;
         Serial.printf("flight on\n");
         keyScreen_Start = sys_sec;
         keyScreen_Now = sys_sec;
         keyState = NOKEYDOWN;
      }
    }
    break;
  case LONGPRESS_END:
    if (screenState == MAIN_TEMP_SCREEN || screenState == MAIN_HUMI_SCREEN || screenState == TIPS_SCREEN)
    {
      if (workingState == NOT_WORKING)
      {
        //1111111screen_loopEnabled = false;
        screenState = REC_START_SCREEN;
        workingState = WORKING;
        current_rec_State = START_RECING;
        alFFS_thisRec_firstData_flag = true;
        Serial.println("workingState = WORKING;");
        keyState = NOKEYDOWN;
      }
      else if (workingState == WORKING)
      {
      //1111111screen_loopEnabled = false;
   
        screenState = REC_STOP_SCREEN;
        workingState = NOT_WORKING;
        current_rec_State = END_RECING;
        //alFFS_endRec();
        Serial.println("workingState = NOT_WORKING;1");
        keyScreen_Start = sys_sec;
        keyScreen_Now = sys_sec;
        keyState = NOKEYDOWN;
      }
    }
    keyState = NOKEYDOWN; //处理长按界面下的长按
    break;
  default:
    break;
  }

}

//按键初始化，在SETup中调用一次
void key_init()
{
  button.reset(); //清除一下按钮状态机的状态
  button.attachClick(click);
  button.attachDoubleClick(doubleclick);
  button.attachLongPressStart(longPressStart);
  button.attachDuringLongPress(duringLongPress);
  button.attachLongPressStop(longPressStop);
  button2.reset(); //清除一下按钮状态机的状态
  // button2.attachClick(click2);
  // button2.attachDoubleClick(doubleclick2);
  // button2.attachLongPressStart(longPressStart2);
  button2.attachDuringLongPress(duringLongPres2);
  //button2.attachLongPressStop(longPressStop2);
}
//解除按键功能
void key_attach_null()//
{
  button.attachClick(nullfunc);
  button.attachDoubleClick(nullfunc);
  button.attachLongPressStart(nullfunc);
  button.attachDuringLongPress(nullfunc);
  button.attachLongPressStop(nullfunc);
  button2.attachClick(nullfunc);
  button2.attachDoubleClick(nullfunc);
  button2.attachLongPressStart(nullfunc);
  button2.attachDuringLongPress(nullfunc);
  button2.attachLongPressStop(nullfunc);
}
//oled关闭按键按下显示等待。。。。
void oledoff_upload_but_click()
{
  Serial.println("oledoff_upload_but_click");
  display.displayOn();
  display.drawString(0, 28, "wait");
  delay(200);
  display.displayOff();
}