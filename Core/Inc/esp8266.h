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

#define RX_LEN 256  

uint8_t ESP8266_Init(void);
uint8_t ESP8266_Check(void);

/**
 * 清空接收缓冲
 */
void USART1_ClearRX(void);
/**
 * 清空发送缓冲
 */
void USART1_ClearTX(void);
char* USART1_GetTX(void);
char* USART1_GetRX(void);
uint8_t USART1_GetReceiveFlag(void);
void USART1_Handler(UART_HandleTypeDef *huart);
void USART1_Receive(void);

/**
 * ESP8266 发送AT指令
 * @param str 指令字符串
 */
void ESP8266_ATSendString(char*str);
/**
 * ESP8266 发送AT指令
 * @param str 指令字符串缓冲
 * @param len 指令字符串长度
 */
void ESP8266_ATSendBuf(uint8_t *str, uint16_t len);
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
 * 使用指定协议(TCP/UDP)连接到服务器
 * 说明： 
  * 失败的原因有以下几种(UART通信和ESP8266正常情况下)
  * 1. 远程服务器IP和端口号有误
  * 2. 未连接AP
  * 3. 服务器端禁止添加(一般不会发生)
 * @param mode 协议类型 "TCP","UDP"
 * @param ip 目标服务器IP
 * @param port 目标是服务器端口号
 * @return 连接结果,非0连接成功,0连接失败
 */
uint8_t ESP8266_ConnectServer(char* mode, char* ip, uint16_t port);
/**
 * 主动和服务器断开连接      
 * @retval 连接结果,非0断开成功,0断开失败
 */
uint8_t ESP8266_DisconnectServer(void);

/**
 * 退出透传
 */
void ESP8266_ExitUnvarnishedTrans(void);
/**
 * 开启透传模式
 */
uint8_t ESP8266_OpenTransmission(void);

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
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_SetConfigServer(void);
/**
 * 关闭配置服务器
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_CloseConfigServer(void);
/**
 * 打开TCP连接
 * @param ip 目标IP
 * @param port 目标端口
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_StartTcp(char*ip, uint8_t port);
/**
 * ESP8266发送数据
 * @param data 数据
 * @param len 数据长度
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_SendData(char*data, uint16_t len);

/**
 * 获取的一个连接的设备IP地址
 * @param buf 接收缓冲区，约16B
 */
void ESP8266_GetFirstConnectIP(char*buf);

//ESP8266接收处理
void ESP8266_ReceiveHandle(void);

#define CONFIG_AP_NAME "CSW_Config"  
#define CONFIG_AP_PASS "12345678"  
#define CONFIG_SERVER_PORT "8080"  

#endif

