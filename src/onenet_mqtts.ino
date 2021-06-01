#include "config.h"

/*-------------------------------连接平台-------------------------------*/
void onenet_connect()
{
  //连接OneNet并上传数据
  Serial.println("connecting to OneNet IOT...");
  client.setServer(mqtt_server, port);                   //设置客户端连接的服务器,连接Onenet服务器, 使用6002端口
  client.connect(mqtt_devid, mqtt_pubid, mqtt_password); //客户端连接到指定的产品的指定设备.同时输入鉴权信息
}



/******************************************************************************
*******************************************************************************/
bool sendTempAndHumi()
{

  char param[400];
  char jsonBuf[400];
  for (uint16_t i = 0; i < 400; i++)
  {
    jsonBuf[i] = 0;
  }
  //确定数据
  sht20getTempAndHumi();
  if ((f_locat == 1) && (locationE != locationE_))
  {
    locationN = locationN_;
    locationE = locationE_;
    locationA = locationA_;
    Serial.printf("Temp=%f,Humi=%f,Time=%d,locationE=%f,locationN=%f.\n", currentTemp, currentHumi, now_unixtime, locationE, locationN);
  }
  if (current_rec_State == START_RECING) //这个有疑问
  {
    Serial.println("fasong start_time! ");
    start_time = now_unixtime;
    last_time = now_unixtime;
  }
  else
  {
    Serial.println("fasong last_time! ");
    last_time = now_unixtime;
  }
  if (f_locat == 1) //确定获取到了位置
  {
    //发送温度、湿度、坐标E、坐标N
    sprintf(param, "{\"temp\":{\"value\":%.2f,\"time\":%u000},\"humi\":{\"value\":%.2f,\"time\":%u000},\"le\":{\"value\":%.2f,\"time\":%u000},\"ln\":{\"value\":%.2f,\"time\":%u000}}",
            currentTemp, now_unixtime, currentHumi, now_unixtime, locationE, now_unixtime, locationN, now_unixtime);
  }
  else
  {
    sprintf(param, "{\"temp\":{\"value\":%.2f,\"time\":%u000},\"humi\":{\"value\":%.2f,\"time\":%u000}}",
            currentTemp, now_unixtime, currentHumi, now_unixtime);
  }
  // sprintf(param, "{\"start_time\":{\"value\":%u,\"time\":%u000},\"last_time\":{\"value\":%u,\"time\":%u000}}",
  //         start_time, now_unixtime, last_time, now_unixtime);
  //加上JSON头组成一个完整的JSON格式
  sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, param);

  //再从mqtt客户端中发布post消息
  if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
  {
    Serial.println("FASONG1 OK");

    //再发送 strat/lasttime
    //sprintf(param, "{\"start_time\":{\"value\":%u,\"time\":%u000},\"last_time\":{\"value\":%u,\"time\":%u000}}",
    //      start_time, now_unixtime, last_time, now_unixtime);
    delay(1000);
    onenet_connect();
    if (client.connected())
    {
      Serial.println("liangwang  OK");
      sprintf(param, "{\"start_time\":{\"value\":%u,\"time\":%u000},\"last_time\":{\"value\":%u,\"time\":%u000}}",

              start_time, now_unixtime, last_time, now_unixtime);
      //加上JSON头组成一个完整的JSON格式
      sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, param);

      if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
      {
        Serial.println("FASONG2 OK");
        current_rec_State = KEEP_RECING;
        return true;
      }
      else
      {
        Serial.println("Publish message2 to cloud failed!");
        return false;
      }
    }
  }
  else
  {
    Serial.println("Publish message to cloud failed!");
    return false;
  }
}
