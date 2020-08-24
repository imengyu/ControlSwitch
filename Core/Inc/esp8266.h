/**
  ******************************************************************************
  * @file           : esp8266.h
  * @brief          : header for esp8266.c file.
  *                   esp8266控制模块.
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f1xx_hal.h"

#define RX_LEN 1024  
             
typedef struct  
{  
  uint8_t  RX_flag : 1;        //IDLE receive flag
  uint16_t RX_Size;          //receive length
  uint8_t  RX_pData[RX_LEN]; //DMA receive buffer
}USART_RECEIVETYPE; 

uint8_t ESP8266_Init(void);
uint8_t ESP8266_Check(void);

/**
 * ESP8266 发送AT指令
 * @param str 指令字符串
 */
void ESP8266_ATSendString(char*str);
/**
 * 功能：连接热点
 * 参数：
 *         ssid:热点名
 *         pwd:热点密码
 * 返回值：
 *         连接结果,非0连接成功,0连接失败
 * 说明： 
 *         失败的原因有以下几种(UART通信和ESP8266正常情况下)
 *         1. WIFI名和密码不正确
 *         2. 路由器连接设备太多,未能给ESP8266分配IP
 * @param ssid 热点名
 * @param pswd 热点密码
 * @return 连接结果,非0连接成功,0连接失败
 */
uint8_t ESP8266_ConnectAP(char* ssid, char* pswd);
/**
 * 获取当前AP列表
 * @param buffer 接收缓冲区，约128B
 */
void ESP8266_GetAPList(char*buffer);
/**
 * 获取当前地址
 * @param buf 接收缓冲区，约16B
 */
void ESP8266_GetCurrentIP(char*buf);
/**
 * 设置为AP模式，并开启一个配置服务器
 */
uint8_t ESP8266_SetConfigServer();
/**
 * 关闭配置服务器
 */
uint8_t ESP8266_CloseConfigServer();

void ESP8266_ReceiveHandle();

#define CONFIG_AP_NAME "CSW_Config"  
#define CONFIG_AP_PASS "12345678"  
#define CONFIG_SERVER_PORT "8080"  

#endif