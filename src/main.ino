#include "config.h"

int rollback = 0;

/**************************************************************
//第二核创建按键任务
***************************************************************/
void codeForTask1(void *parameter)
{

  while (1) //这是核1 的loop
  {
    vTaskDelay(10);
    button.tick(); //扫描按键
    button2.tick();
  }
  vTaskDelete(NULL);
}
/*************************************************************
//ds1302任务
**************************************************************/
void ds1302_task(void *parameter)
{
  uint8_t sec = 0;
  while (1)
  {
    ds_rtc.getDateTime(&now1); //读取时间参数到NOW
    now_unixtime = unixtime();
    if (now1.second == sec + 1)
    {
      sys_sec++;
      // Serial.printf("sec:%d\n",sys_sec);
    }
    sec = now1.second;
    vTaskDelay(500);
  }
  vTaskDelete(NULL);
}
/*****************************************************************
//通讯协议任务
*****************************************************************/
void xieyi_Task(void *parameter)
{
  while (1) //这是核1 的loop
  {
    xieyi_scan();
    vTaskDelay(100);
  }
  vTaskDelete(NULL);
}













/*****************************************************************************************************************/
void setup()
{
  gpio_hold_dis(GPIO_NUM_32); //解锁电源引脚
  gpio_deep_sleep_hold_dis();
 

  hardware_init();            //硬件初始化
  get_eeprom_firstBootFlag(); //获取EEPROM第1位,判断是否是初次开机
  eeprom_config_init();       //初始化EEPROM
  software_init();            //软件初始化

  xTaskCreatePinnedToCore(xieyi_Task, "xieyi_Task", 3000, NULL, 2, &xieyi_task, tskNO_AFFINITY); //创建DS1302任务
  xTaskCreatePinnedToCore(ds1302_task, "ds1302_task", 2000, NULL, 2, &ds_task, tskNO_AFFINITY);  //创建DS1302任务
  xTaskCreatePinnedToCore(codeForTask1, "task1", 1000, NULL, 2, &task1, tskNO_AFFINITY);
  if (rollback)
  {
    /*************如果rollback置1, 会恢复出厂设置,数据全清***********/
    Serial.println("clean EEPROM");
    EEPROM.write(1, 0);
    EEPROM.commit();
    Serial.println("OK");
    ESP.deepSleep(300000000);
    modem.sleepEnable();
  }
  else
  {
    alFFS_init(); //初始化FFS
    wakeup_init_time();
  }

  if (oledState == OLED_ON)
  {
    showWelcome();
    postMsgId = 0; //清记录条数
  }
  else
  {
    if (workingState == WORKING && (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)) //不是开机，是定时唤醒。
    {
      send_Msg_var_GSM_while_OLED_on(0);
      go_sleep_a_while_with_ext0(); //休眠
    }
  }
}
/******************************************************************************************************************/

void loop()
{
  if (oledState == OLED_ON)
  {
    get18b20Temp();
    key_loop();
    screen_show(); //OLED最终显示
    send_Msg_var_GSM_while_OLED_on(1);
  }
}






















/*****************************************************************************************
 *                            //发送数据
******************************************************************************************/
//#define D1
void send_Msg_var_GSM_while_OLED_on(bool a)
{

  if (workingState == WORKING && f_Flight_Mode == false) //工作模式（正常记录）
  {

    if (dbug)
      Serial.println("zhengchang jilu! "), Serial.println("GSM transmission will start at:" + (String)(sleeptime - (now_unixtime - last_rec_stamp))), Serial.printf("last_rec_stamp=%d, sleeptime=%d\n ", last_rec_stamp, sleeptime);
    if (now_unixtime - last_rec_stamp >= sleeptime) //记录间隔到了吗？
    {
      if (dbug)
        Serial.printf("zhengchang mode time Ok!\n ");
      //1.需要联网测网络
      if (a)
      {
        display.clear();
        display.setFont(Roboto_Condensed_12);
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 5, "Initializing modem...");
        display.drawProgressBar(5, 50, 118, 8, 5);
        display.display();
      }
      setupModem();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "Waiting for network...");
        display.drawProgressBar(5, 50, 118, 8, 40);
        display.display();
      }
      modemToGPRS();
      if (a)
      {
        display.clear();
        display.drawString(64, 5, "getting LBS...");
        display.drawProgressBar(5, 50, 118, 8, 70);
        display.display();
      }
      Serial.printf("huoqu shijian weizhi %d!\n", getLBSLocation()); //获取时间位置正确,发送时间位置
      get18b20Temp();
      alFFS_savelist();
      if (dbug)
        Serial.println("duquwendu,jiyiLIST OK!");

      if ((f_Flight_Mode == false) && (f_lose == false) && (workingState = WORKING)) //正常记录和发送
      {
        if (dbug)
          Serial.println("zhengchang fasong !");
        if (a)
        {
          display.clear();
          display.drawString(64, 5, "connecting to OneNet");
          display.drawProgressBar(5, 50, 118, 8, 90);
          display.display();
        }
        //2.连接ONENET
        onenet_connect();
        if (client.connected())
        {
          char subscribeTopic[75];                           //订阅主题
          char topicTemplate[] = "$sys/%s/%s/cmd/request/#"; //信息模板
          snprintf(subscribeTopic, sizeof(subscribeTopic), topicTemplate, mqtt_pubid, mqtt_devid);
          client.subscribe(subscribeTopic); //订阅命令下发主题
          if (a)
          {
            display.clear();
            display.drawString(64, 5, "uploading...");
          }
          if (sendTempAndHumi()) //数据发送成功
          {

            if (a)
            {
              display.drawProgressBar(5, 50, 118, 8, 100);
              display.display();
              delay(200);
              display.setTextAlignment(TEXT_ALIGN_LEFT);
            }
            key_init();
            Serial.printf("time jiange:%d\n", (now_unixtime - last_rec_stamp));
            last_rec_stamp = now_unixtime; //发送完成可以记录发送时间
            screenState = MAIN_TEMP_SCREEN;
            screen_On_Start = sys_sec;
            screen_On_now = sys_sec;
            digitalWrite(MODEM_POWER_ON, LOW);
          }
          else //数据发送失败
          {
            f_lose = true; //置位标志位
            Serial.printf("sendTempAndHumi3 fial! f_lose=%d\n", f_lose);
            //1.直接写漏发文件和正常记录文件
            alFFS_savelose();

            key_init();
            last_rec_stamp = now_unixtime; //发送失败也要更新发送时间，开启下一次发送
            screenState = MAIN_TEMP_SCREEN;
            screen_On_Start = sys_sec;
            screen_On_now = sys_sec;
            digitalWrite(MODEM_POWER_ON, LOW);
          }
        }
        else //数据发送失败
        {
          f_lose = true;                                               //置位标志位 //置漏发标志
          Serial.printf("onenet_connect fial! f_lose = %d\n", f_lose); //这里应加入F_LOSE=1?
          //1.直接写漏发文件
          alFFS_savelose();

          key_init();
          last_rec_stamp = now_unixtime; //这里是联网不成功退出，要更新发送时间
          screenState = MAIN_TEMP_SCREEN;
          screen_On_Start = sys_sec;
          screen_On_now = sys_sec;
          digitalWrite(MODEM_POWER_ON, LOW);
        }
      }
      else if (f_Flight_Mode == false && f_lose == true) //有漏发文件非飞行模式
      {
        if (dbug)
          Serial.printf("fei feixingmoshi youloufa f_lose=%d\n", f_lose);
        //将本条加入到LOSE
        alFFS_savelose();
        f_lose = true;
        jiexi_lose(a);
        //补发本条
        key_init();
        last_rec_stamp = now_unixtime; //这里应该不需要等发送完漏发在更新
        screenState = MAIN_TEMP_SCREEN;
        screen_On_Start = sys_sec;
        screen_On_now = sys_sec;
        digitalWrite(MODEM_POWER_ON, LOW);
      }
      else //其他状态（这种方式是错误）
      {
        while (1)
        {
          Serial.printf("qitamoshi zhong tuicu! qingjiancha !!!!!!!\n");
          delay(500);
        }
      }
    }
    else if (dbug)
      Serial.printf("zhengchang mode time no!\n"); //发送时间未到。
  }
  else if (workingState == WORKING && f_Flight_Mode == true) //飞行模式开启（不上传网络）
  {
    if (dbug)
      Serial.println("jilu no send");

    if (dbug)
      Serial.println("GSM transmission will start at:" + (String)(sleeptime - (now_unixtime - last_rec_stamp)));
    if ((now_unixtime - last_rec_stamp >= sleeptime)) //记录间隔到了吗？
    {
      //1.直接写漏发文件和正常记录文件

      alFFS_savelist();
      Serial.println("*************************3******************************");
      Serial.printf("f_lose=%d\n", f_lose);

      alFFS_savelose();

      //置位标志位
      f_lose = true;
      f_Flight_Mode = true;

      key_init();
      last_rec_stamp = now_unixtime; //间隔到了，不上传也要更新，方便开启下一次
                                     // screen_loopEnabled = true;
      screen_On_Start = sys_sec;
      screen_On_now = sys_sec;
    }
    //else  Serial.println("feixing mode timer no");
  }
  else if ((workingState == NOT_WORKING) && (f_lose == true) && (f_Flight_Mode == false)) //发现有漏传文件，开启自动上传漏发工作模式
  {
    Serial.println("bufa louchuan");
    //这里要显示补发图片

    old_workingstate = NOT_WORKING;
    workingState = WORKING;
    key_init();
    screen_On_Start = sys_sec;
    screen_On_now = sys_sec;
  }
}


void aaa()
{
  /* gpio32 route to digital io_mux */
REG_CLR_BIT(RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32P_MUX_SEL);

/* gpio33 route to digital io_mux */
REG_CLR_BIT(RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32N_MUX_SEL);
}