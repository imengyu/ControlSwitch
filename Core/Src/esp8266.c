/**
  ******************************************************************************
  * @file           : esp8266.c
  * @brief          : esp8266控制模块
  ******************************************************************************
  * 使用 UART1
  ******************************************************************************
  */

#include "esp8266.h"

extern UART_HandleTypeDef huart1; 

#define WIFI_CONNECTED 0b0001

uint8_t UartTxBuf[64];
USART_RECEIVETYPE UsartType;
uint8_t WifiSettings = 0;

void UART_IRQL_INIT(void) {
  HAL_UART_Receive_DMA(&huart1, UsartType.RX_pData, RX_LEN);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_Delay(120);
}
/**
  * @brief  This function handles USART1 IDLE interrupt.
  */
void UsartReceive_IDLE(UART_HandleTypeDef *huart)
{  
    uint32_t temp;  
  
    if((__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE) != RESET))  
    {   
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);  
        HAL_UART_DMAStop(&huart1);  
        temp = huart1.hdmarx->Instance->CNDTR;  
        UsartType.RX_Size = RX_LEN - temp;   
        UsartType.RX_flag = 1;  
        HAL_UART_Receive_DMA(&huart1,UsartType.RX_pData,RX_LEN);  
    }  
} 
/**
 * 清空接收缓冲
 */
void UsartReceiveClear() {
  memset(UsartType.RX_pData, 0, UsartType.RX_Size); 	 //清空接收缓冲
  UsartType.RX_flag = 0;  
}

/**
 * 查找字符串中是否包含另一个字符串
 */
uint8_t FindStr(char* dest, char* src,uint16_t retry_nms)
{
	retry_nms/=10;                   //超时时间

	while(strstr(dest,src)==0 && retry_nms--)//等待串口接收完毕或超时退出
	{		
		HAL_Delay(10);
	}

	if(retry_nms) return 1;                       

	return 0; 
}

/**
 * ESP8266初始化
 */
uint8_t ESP8266_Init(void) {
  

  UART_IRQL_INIT();
 
  ESP8266_ATSendString("AT+RST\r\n");
  HAL_Delay(120);

  if(ESP8266_Check()==0)              //使用AT指令检查ESP8266是否存在
		return 0;

  ESP8266_ATSendString("AT+CWMODE=1 \r\n");
  HAL_Delay(120);


  return 1;
}

/**
 * 功能：检查ESP8266是否正常
 * 参数：None
 * 返回值：ESP8266返回状态
 *        非0 ESP8266正常
 *        0 ESP8266有问题  
 */
uint8_t ESP8266_Check(void)
{
	uint8_t check_cnt=5;
	while(check_cnt--)
	{
		UsartReceiveClear();
		ESP8266_ATSendString("AT\r\n");     		 			//发送AT握手指令	
		if(FindStr((char*)UsartType.RX_pData,"OK",200) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * ESP8266 发送AT指令
 * @param str 指令字符串
 */
void ESP8266_ATSendString(char*str) {
  strcpy(UartTxBuf, str);
  HAL_UART_Transmit(&huart1, UartTxBuf, strlen(UartTxBuf), 100);
}

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
uint8_t ESP8266_ConnectAP(char* ssid, char* pswd)
{
	uint8_t cnt=5;
	while(cnt--)
	{
		UsartReceiveClear();
		ESP8266_ATSendString("AT+CWMODE_CUR=1\r\n");              //设置为STATION模式	
		if(FindStr((char*)UsartType.RX_pData,"OK",200) != 0)
		{
			break;
		}             		
	}
	if(cnt == 0)
		return 0;

	cnt=2;
	while(cnt--)
	{                    
		memset(UartTxBuf,0,sizeof(UartTxBuf));//清空发送缓冲
		UsartReceiveClear();//清空接收缓冲
		sprintf((char*)UartTxBuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);//连接目标AP
		ESP8266_ATSendString((char*)UartTxBuf);	
		if(FindStr((char*)UsartType.RX_pData,"OK",8000)!=0)                      //连接成功且分配到IP
		{
			return 1;
		}
	}
	return 0;
}


