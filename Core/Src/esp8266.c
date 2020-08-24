/**
  ******************************************************************************
  * @file           : esp8266.c
  * @brief          : esp8266控制模块
  ******************************************************************************
  * 使用 UART1
  ******************************************************************************
  */

#include "esp8266.h"
#include "delay.h"

extern UART_HandleTypeDef huart1; 

uint8_t UartTxBuf[64];
USART_RECEIVETYPE UsartType;

void USART1_Init(void) {
  HAL_UART_Receive_DMA(&huart1, UsartType.RX_pData, RX_LEN);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  Delay_MS(120);
}
/**
  * @brief  This function handles USART1 IDLE interrupt.
  */
void USART1_Handler(UART_HandleTypeDef *huart)
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
void USART1_Clear() {
  memset(UsartType.RX_pData, 0, UsartType.RX_Size); 	 //清空接收缓冲
  UsartType.RX_flag = 0;  
}

/**
 * 查找字符串中是否包含另一个字符串
 * @param dest 字符串
 * @param src 要查找的字符串
 * @param retry_nms 超时时间
 * @return 如果找到，返回0，如果未找到或超时，返回1
 */
uint8_t FindStr(char* dest, char* src, uint16_t retry_nms)
{
	retry_nms/=10; //超时时间

	while(strstr(dest,src) == 0 && retry_nms--)//等待串口接收完毕或超时退出
	{		
		Delay_MS(10);
	}

	if(retry_nms) return 1;                       

	return 0; 
}

/**
 * ESP8266初始化
 */
uint8_t ESP8266_Init(void) {
  

  USART1_Init();
 
  ESP8266_ATSendString("AT+RST\r\n");
  Delay_MS(120);

  if(ESP8266_Check()==0)              //使用AT指令检查ESP8266是否存在
		return 0;

  ESP8266_ATSendString("AT+CWMODE=1 \r\n");
  Delay_MS(120);


  return 1;
}

/**
 * 功能：检查ESP8266是否正常
 * 参数：None
 * 返回值：ESP8266返回状态
 *        非0 ESP8266正常
 *        0 ESP8266有问题  
 *
 */
uint8_t ESP8266_Check(void)
{
	uint8_t check_cnt=5;
	while(check_cnt--)
	{
		USART1_Clear();
		ESP8266_ATSendString("AT\r\n");     		 			//发送AT握手指令	
    Delay_MS(120);
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
  if(str != UartTxBuf)
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
		USART1_Clear();
		ESP8266_ATSendString("AT+CWMODE=1\r\n"); //设置为STATION模式	
    Delay_MS(120);
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
		USART1_Clear();//清空接收缓冲
		sprintf((char*)UartTxBuf,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,pswd);//连接目标AP
		ESP8266_ATSendString((char*)UartTxBuf);	
    Delay_MS(120);
		if(FindStr((char*)UsartType.RX_pData,"OK",8000)!=0) //连接成功且分配到IP
		{
			return 1;
		}
	}
	return 0;
}

/**
 * 获取当前AP列表
 * @param buf 接收缓冲区，约350B
 */
void ESP8266_GetAPList(char*buf) {
  char* str;
  uint16_t pos, cut_start, cut_end, cur;

  USART1_Clear();
  ESP8266_ATSendString("AT+CWLAP\r\n");	
  Delay_MS(300);

  // ESP8266 返回格式：
  /*
+CWLAP:(4,"TP-LINK_803",-85,"60:3a:7c:42:c0:57",1,21,0)
+CWLAP:(4,"Xiaomi_D460",-51,"50:d2:f5:fa:d4:61",10,38,0)
+CWLAP:(4,"TP-LINK_00B9",-72,"9c:a6:15:b8:00:b9",6,28,0)
  */
  //解析字符串中的 +CWLAP ，并提取WIFI名称，按 \n 分隔保存至字符串缓冲区
  str = (char*)UsartType.RX_pData;
  pos = cur = 0;
  while (pos < 588 - 30) {
    if (str[pos] == '+' && str[pos + 1] == 'C' && str[pos + 2] == 'W') {
      cut_end = cut_start = pos + 11;
      while (str[cut_end] != '"' && cut_end - cut_start < 26) cut_end++;
      
      for (; cut_start < cut_end; cut_start++) {
        buf[cur] = str[cut_start];
        cur++;
      }

      buf[cur] = '\n';
      cur++;
      pos += 10;
    }
    pos++;
  }
  buf[cur] = '\0';
}

/**
 * 获取当前地址
 * @param buf 接收缓冲区，约16B
 */
void ESP8266_GetCurrentIP(char*buf) {
  char* str;
  uint16_t pos, cut_start, cut_end, cur;

  USART1_Clear();
  ESP8266_ATSendString("AT+CIFSR\r\n");
  Delay_MS(120);

  // ESP8266 返回格式：
  //+CIFSR:APIP,"192.168.4.1"
  //+CIFSR:APMAC,"a2:20:a6:19:c7:0a"
  //
  //解析字符串中的 APIP ，获取ip ，并以保存至字符串缓冲区
  cur = 0;
  str = (char*)UsartType.RX_pData;
  pos = strstr(str, "APIP");
  if(pos != 0) {
    cut_end = cut_start = pos + 6;
    while (str[cut_end] != '"' && cut_end - cut_start < 26) cut_end++;
    for (; cut_start < cut_end; cut_start++) {
      buf[cur] = str[cut_start];
      cur++;
    }
    buf[cur] = '\0';
  }
}


/**
 * 设置为AP模式，并开启一个配置服务器
 */
uint8_t ESP8266_SetConfigServer() {
  uint8_t cnt=2;

  USART1_Clear();
	ESP8266_ATSendString("AT+CWMODE=2\r\n"); //AP模式
	while(cnt--)
	{
    Delay_MS(200);

		if(FindStr((char*)UsartType.RX_pData,"OK",200) != 0) break;            		
	}
	if(cnt == 0) return 0;

  USART1_Clear();
	ESP8266_ATSendString("AT+RST\r\n");//重启模块使设置生效
  Delay_MS(120);

	if(!ESP8266_Check()) return 0;       		

  USART1_Clear();
	ESP8266_ATSendString("AT+CIPMUX=1\r\n");//允许多连接
  Delay_MS(100);
	if(FindStr((char*)UsartType.RX_pData,"OK",200) == 0)
    return 0;

  memset(UartTxBuf, 0, sizeof(UartTxBuf));
	sprintf((char*)UartTxBuf, "AT+CIPSERVER=1,%s\r\n", CONFIG_SERVER_PORT);//开启服务器
  USART1_Clear();
	ESP8266_ATSendString(UartTxBuf);
  Delay_MS(100);
	if(FindStr((char*)UsartType.RX_pData,"OK",200) == 0)
    return 0;

  memset(UartTxBuf, 0, sizeof(UartTxBuf));
	sprintf((char*)UartTxBuf, "AT+CWSAP=\"%s\",\"%s\",1,3\r\n", CONFIG_AP_NAME, CONFIG_AP_PASS);//设置热点
  USART1_Clear();
	ESP8266_ATSendString(UartTxBuf);

  cnt=5;
  while(cnt--)
	{
    Delay_MS(120);

		if(FindStr((char*)UsartType.RX_pData,"OK",200) != 0) break;  //判断是否成功         		
	}
	if(cnt == 0) return 0;
  
  return 1;
}

/**
 * 关闭配置服务器
 */
uint8_t ESP8266_CloseConfigServer() {
  USART1_Clear();
	ESP8266_ATSendString("AT+CIPSERVER=0\r\n");//关闭服务器
  Delay_MS(120);
	if(FindStr((char*)UsartType.RX_pData,"OK",200) == 0)
    return 0;

  return 1;
}

extern void MAIN_Handler_WifiCommand(char*buf, uint16_t len);

/**
 * 处理 ESP8266 返回的数据
 */
void ESP8266_ReceiveHandle() {
  uint16_t pos, cut_start, cut_end, cur, len;
  char buf[128];
  char* str;
  
  str = (char*)UsartType.RX_pData;
  len = strlen(str);
  pos = 0;

  // ESP8266 返回格式：
  /*
+IPD,0,4:Test1
+IPD,0,4:Test0,CLOSED
  */
  //解析字符串中的 +IPD ，获取发送来的数据 ，并以保存至字符串缓冲区

  while (pos < len - 3) {
    if (str[pos] == '+' && str[pos + 1] == 'I' && str[pos + 2] == 'P' && str[pos + 3] == 'D') {
      cut_start = pos;
      while (str[pos] != ':' && pos - cut_start < 10) pos++;
      cut_start = pos + 1;
      cut_end = cut_start;
      while (pos - cut_start < 32) {
          if (str[pos] == ',' || str[pos] == '\r' || str[pos] == '\n' || str[pos] == '+')
              break;
          pos++;
      }
      cut_end = pos;
      pos-=2;

      for (cur = 0; cut_start < cut_end; cur++)
        buf[cur] = str[cut_start];
      buf[cur] = '\0';

      //传递给主子程序处理
      MAIN_Handler_WifiCommand(buf, cut_end - cut_start);
    }
    pos++;
  }
}


