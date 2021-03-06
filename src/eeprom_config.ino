#include "config.h"
/************************dataType_bytes************************
 * dataType                 bytes
 * ------------------------------------------------------------
 * char                     1
 * short                    2
 * int                      4
 * time_t                   4
 * int32_t                  8
 * long                     4
 * float                    4
 * double                   8
 * ************************************************************/

/************************EEPROM_table***************************
 * adress                   length(byte)        description
 * ------------------------------------------------------------
 * 1                        1                   firstLoad_flag
 * 2                        8                   sleeptime                             休眠时间
 * 10                       1                   temp Upper/Lower limit enabled
 * 11                       4                   temp Upper limit
 * 15                       4                   temp Lower limit
 * 19                       4                   factory date year
 * 23                       4                   factory date month
 * 27                       4                   factory date day
 * 31                       4                   factory time hour
 * 35                       4                   factory time min
 * 39                       8                   time unixtime                          最后一次发送时间
 * 43                       4                   screen_On_last_span                    亮屏时间
 
 * ************************************************************/
//获取是否是首次开机
void get_eeprom_firstBootFlag()
{ 
  firstBootFlag =(EEPROM.read(1) == 1 ? false : true);
  Serial.printf("EEPROM 1: %d \r\n", firstBootFlag);
}
//根据开机状态初始化EEPROM
//首次开机按config默认值初始化eeprom
//否者，读取eeprom设备设定值
void eeprom_config_init()
{
  if (firstBootFlag)
  {
    Serial.println("this is the first load,begin to write default:");
    EEPROM.write(1, 1);
    EEPROM.writeInt(2, FACTORY_SLEEPTIME);
    EEPROM.write(10, FACTORY_TEMP_LIMIT_ENABLE);
    EEPROM.writeFloat(11, FACTORY_TEMP_UPPER_LIMIT);
    EEPROM.writeFloat(15, FACTORY_TEMP_LOWER_LIMIT);
    EEPROM.writeInt(19, FACTORY_DATE_YEAR);
    EEPROM.writeInt(23, FACTORY_DATE_MONTH);
    EEPROM.writeInt(27, FACTORY_DATE_DAY);
    EEPROM.writeInt(31, FACTORY_TIME_HOUR);
    EEPROM.writeInt(35, FACTORY_TIME_MIN);
    EEPROM.writeULong(39, 0);
    EEPROM.commit();
    firstBootFlag = false;

    // screen_On_Start = sys_sec;
    // screen_On_now = sys_sec;
  }
  else
  {
    Serial.println("this is not the first load");

    sleeptime =         (time_t)EEPROM.readLong(2);        Serial.printf("sleeptime:%ld\r\n", sleeptime);
    tempLimit_enable =  EEPROM.read(10) == 0 ? false : true;
    tempUpperLimit =    EEPROM.readFloat(11);              Serial.printf("tempUpperLimit:%.2f\r\n", tempUpperLimit);
    tempLowerLimit =    EEPROM.readFloat(15);              Serial.printf("tempLowerLimit:%.2f\r\n", tempLowerLimit);
  }
}
//记忆设定值
void eeprom_config_save_parameter(void)
{
    EEPROM.writeInt(2, sleeptime);
   // EEPROM.write(10, FACTORY_TEMP_LIMIT_ENABLE);
    
    EEPROM.commit();
}