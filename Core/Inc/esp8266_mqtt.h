#ifndef __ESP8266_MQTT_H
#define __ESP8266_MQTT_H

#include "stm32f1xx_hal.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))


//MQTT连接服务器
 uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);
//MQTT消息订阅
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);
//MQTT消息发布
uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos);
//MQTT发送心跳包
void MQTT_SentHeart(void);
//MQTT断开
void MQTT_Disconnect(void);
//MQTT初始化
void MQTT_Init(void);

#endif

