/**
  ******************************************************************************
  * @file           : esp8266.c
  * @brief          : esp8266控制模块
  ******************************************************************************
  * 硬件使用：
  *   使用 ESP8266-01S 模块
  *   连接至 UART1
  ******************************************************************************
  */

#include "esp8266.h"
#include "delay.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


extern UART_HandleTypeDef huart1; 
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

char UartTxBuf[64];
char UartRxBuf[RX_LEN];
uint8_t UartReceiveFlag = 0;
uint8_t UartReceiveLength = 0;

char* USART1_GetTX() { return (char*)UartTxBuf; }
char* USART1_GetRX() { return (char*)UartRxBuf; }
uint8_t USART1_GetReceiveFlag() { return UartReceiveFlag; }

void USART1_Init(void) {
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  HAL_UART_Receive_DMA(&huart1, (uint8_t*)UartRxBuf, RX_LEN);
}
void USART1_Handler(UART_HandleTypeDef *huart)
{  
  if(huart->Instance == USART1) {
    uint32_t tmp_flag = 0;
    uint32_t temp;
    tmp_flag =__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE); //获取IDLE标志位
    if((tmp_flag != RESET)) //idle标志被置位
    { 
      __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除标志位
      temp = huart1.Instance->SR;   //清除状态寄存器SR（F0的HAL库USART_TypeDef结构体中名字为ISR：USART Interrupt and status register），读取SR可以清楚该寄存器
      temp = huart1.Instance->DR;   //读取数据寄存器中的数据,读取DR（F0中为RDR：USART Receive Data register）
      HAL_UART_DMAStop(&huart1); 
      temp  = hdma_usart1_rx.Instance->CNDTR;  //获取DMA中未传输的数据个数，NDTR寄存器分析见下面
      UartReceiveLength =  RX_LEN - temp;  //总计数减去未传输的数据个数，得到已经接收的数据个数
      UartReceiveFlag = 1; //接受完成标志位置1  
      UartRxBuf[UartReceiveLength] = '\0';//末尾加0

      HAL_UART_Receive_DMA(&huart1, (uint8_t*)UartRxBuf, RX_LEN);//重新打开DMA接收  
    }
  }
} 
/**
 * 清空接收缓冲
 */
void USART1_ClearRX() {
  memset(UartRxBuf, 0, sizeof(UartRxBuf));//清空接收缓冲
  UartReceiveLength = 0;
  UartReceiveFlag = 0;
}
/**
 * 清空发送缓冲
 */
void USART1_ClearTX() {
  memset(UartTxBuf, 0, sizeof(UartTxBuf));//清空发送缓冲
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
	retry_nms /= 10; //超时时间

	while(strstr(dest, src) == 0 && retry_nms--)//等待串口接收完毕或超时退出
		Delay_MS(10);

	if(retry_nms) return 1;                       

	return 0; 
}

/**
 * ESP8266初始化
 */
uint8_t ESP8266_Init(void) {
  
  USART1_Init();
  USART1_ClearRX();

  ESP8266_ATSendString("AT+RST\r\n");
  Delay_MS(520);
	
  if(ESP8266_Check() == 0) //使用AT指令检查ESP8266是否存在
		return 0;

  ESP8266_ATSendString("AT+CWMODE=1\r\n");
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
	uint8_t check_cnt = 5;

  USART1_ClearRX();
	ESP8266_ATSendString("AT\r\n"); //发送AT握手指令	

	while(check_cnt--)
	{
		if(FindStr((char*)UartRxBuf,"OK", 200) != 0)
			return 1;
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
#if ENABLE_UART_OUTPUT
  printf("ESP8266 < %s\n", str);
#endif
  HAL_UART_Transmit(&huart1, (uint8_t*)UartTxBuf, strlen(UartTxBuf), 100);
}

/**
 * ESP8266 发送AT指令
 * @param str 指令字符串缓冲
 * @param len 指令字符串长度
 */
void ESP8266_ATSendBuf(uint8_t *str, uint16_t len) {
#if ENABLE_UART_OUTPUT
  printf("ESP8266 < %s\n", str);
#endif
  HAL_UART_Transmit(&huart1, str, len, 100);
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
	uint8_t cnt = 5;
	while(cnt--)
	{
		USART1_ClearRX();
		ESP8266_ATSendString("AT+CWMODE=1\r\n"); //设置为STATION模式	
    Delay_MS(120);
		if(FindStr((char*)UartRxBuf,"OK",200) != 0)
			break;          		
	}
	if(cnt == 0)
		return 0;

	cnt=2;
	while(cnt--)
	{                    
		USART1_ClearTX();
		USART1_ClearRX();//清空接收缓冲
		sprintf((char*)UartTxBuf,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,pswd);//连接目标AP
		ESP8266_ATSendString((char*)UartTxBuf);	
    Delay_MS(120);
		if(FindStr((char*)UartRxBuf,"OK", 8000) != 0) //连接成功且分配到IP
			return 1;

	}
	return 0;
}

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
uint8_t ESP8266_ConnectServer(char* mode, char* ip, uint16_t port)
{
	uint8_t cnt;
   
	ESP8266_ExitUnvarnishedTrans();                   //多次连接需退出透传
	Delay_MS(500);

	//连接服务器
	cnt=2;
	while(cnt--)
	{
		USART1_ClearTX();
		USART1_ClearRX();//清空接收缓冲   
		sprintf((char*)UartTxBuf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
		ESP8266_ATSendString((char*)UartTxBuf);
		if(FindStr((char*)UartRxBuf, "CONNECT", 8000) !=0 )
			break;
	}
	if(cnt == 0) 
		return 0;
	
	//设置透传模式
	if(ESP8266_OpenTransmission() == 0) 
    return 0;
	
	//开启发送状态
	cnt=2;
	while(cnt--)
	{
		USART1_ClearRX(); //清空接收缓冲   
		ESP8266_ATSendString("AT+CIPSEND\r\n");//开始处于透传发送状态
		if(FindStr((char*)UartRxBuf, ">", 200)!=0)
			return 1;
	}
	return 0;
}

/**
 * 主动和服务器断开连接      
 * @retval 连接结果,非0断开成功,0断开失败
 */
uint8_t ESP8266_DisconnectServer(void)
{
	uint8_t cnt;
	
	ESP8266_ExitUnvarnishedTrans();	//退出透传
	HAL_Delay(500);
	
	while(cnt--)
	{
		USART1_ClearRX(); //清空接收缓冲   
		ESP8266_ATSendString("AT+CIPCLOSE\r\n");//关闭链接

		if(FindStr((char*)UartRxBuf,"CLOSED",200)!=0)//操作成功,和服务器成功断开
			break;
	}
	if(cnt) 
    return 1;
	return 0;
}

/**
 * 获取当前AP列表
 * @param buf 接收缓冲区，约350B
 */
void ESP8266_GetAPList(char*buf) {
  char* str;
  uint16_t pos, cut_start, cut_end, cur;

  USART1_ClearRX();
  ESP8266_ATSendString("AT+CWLAP\r\n");	
  Delay_MS(300);

  // ESP8266 返回格式：
  /*
+CWLAP:(4,"TP-LINK_803",-85,"60:3a:7c:42:c0:57",1,21,0)
+CWLAP:(4,"Xiaomi_D460",-51,"50:d2:f5:fa:d4:61",10,38,0)
+CWLAP:(4,"TP-LINK_00B9",-72,"9c:a6:15:b8:00:b9",6,28,0)
  */
  //解析字符串中的 +CWLAP ，并提取WIFI名称，按 \n 分隔保存至字符串缓冲区
  str = (char*)UartRxBuf;
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
  char* str,* pos, *cut_start, *cut_end;
  uint16_t cur;

  USART1_ClearRX();
  ESP8266_ATSendString("AT+CIFSR\r\n");
  Delay_MS(200);

  // ESP8266 返回格式：
  //+CIFSR:APIP,"192.168.4.1"
  //+CIFSR:APMAC,"a2:20:a6:19:c7:0a"
  //
  //解析字符串中的 APIP ，获取ip ，并以保存至字符串缓冲区
  cur = 0;
  str = (char*)UartRxBuf;
  pos = (char*)strstr(str, "APIP");
  if (pos != 0) {
      cut_end = cut_start = (pos + 6);
      while (*cut_end != '"' && cut_end - cut_start < 26) cut_end++;
      for (; cut_start < cut_end; cut_start++) {
          buf[cur] = *cut_start;
          cur++;
      }
      buf[cur] = '\0';
  }
}

/**
 * 获取的一个连接的设备IP地址
 * @param buf 接收缓冲区，约16B
 */
void ESP8266_GetFirstConnectIP(char*buf) {
  char* str,* pos, *cut_start, *cut_end;
  uint16_t cur;

  USART1_ClearRX();
  ESP8266_ATSendString("AT+CWLIF\r\n");
  Delay_MS(200);

  // ESP8266 返回格式：
  /*
AT+CWLIF
192.168.4.2,a4:50:46:34:a9:95

OK

  */
  //解析字符串中的 APIP ，获取ip ，并以保存至字符串缓冲区
  cur = 0;
  str = (char*)UartRxBuf;
  pos = (char*)strstr(str, "CWLIF");
  if (pos != 0) {
      cut_end = cut_start = (pos + 8);
      while (*cut_end != ',' && cut_end - cut_start < 26) cut_end++;
      for (; cut_start < cut_end; cut_start++) {
          buf[cur] = *cut_start;
          cur++;
      }
      buf[cur] = '\0';
  }
}


//**********************************************************

/**
 * 设置为AP模式，并开启一个配置服务器
 */
uint8_t ESP8266_SetConfigServer() {
  uint8_t cnt=2;

  ESP8266_ExitUnvarnishedTrans();
	Delay_MS(500);

  USART1_ClearRX();
	ESP8266_ATSendString("AT+CWMODE=2\r\n"); //AP模式
	while(cnt--)
	{
    Delay_MS(200);

		if(FindStr((char*)UartRxBuf,"OK",200) != 0) break;            		
	}
	if(cnt == 0) return 0;

  USART1_ClearRX();
	ESP8266_ATSendString("AT+RST\r\n");//重启模块使设置生效
  Delay_MS(120);

	if(!ESP8266_Check()) return 0;       		

  USART1_ClearRX();
	ESP8266_ATSendString("AT+CIPMUX=1\r\n");//允许多连接
  Delay_MS(100);
	if(FindStr((char*)UartRxBuf,"OK",200) == 0)
    return 0;

  USART1_ClearTX();
	sprintf((char*)UartTxBuf, "AT+CIPSERVER=1,%s\r\n", CONFIG_SERVER_PORT);//开启服务器
  USART1_ClearRX();
	ESP8266_ATSendString((char*)UartTxBuf);
  Delay_MS(100);
	if(FindStr((char*)UartRxBuf,"OK",200) == 0)
    return 0;

  USART1_ClearTX();
	sprintf((char*)UartTxBuf, "AT+CWSAP=\"%s\",\"%s\",1,3\r\n", CONFIG_AP_NAME, CONFIG_AP_PASS);//设置热点
  USART1_ClearRX();
	ESP8266_ATSendString((char*)UartTxBuf);

  cnt=5;
  while(cnt--)
	{
    Delay_MS(120);

		if(FindStr((char*)UartRxBuf,"OK",200) != 0) break;  //判断是否成功         		
	}

	if(cnt == 0) return 0;

  printf("CSW > ConfigServer seted ! AP : %s PASS %s\n", CONFIG_AP_NAME, CONFIG_AP_PASS);
  
  return 1;
}

/**
 * 关闭配置服务器
 */
uint8_t ESP8266_CloseConfigServer() {
  USART1_ClearRX();
	ESP8266_ATSendString("AT+CIPSERVER=0\r\n");//关闭服务器
  Delay_MS(120);
	if(FindStr((char*)UartRxBuf,"OK",200) == 0)
    return 0;

  return 1;
}

//**********************************************************

/**
 * 退出透传
 */
void ESP8266_ExitUnvarnishedTrans(void)
{
	ESP8266_ATSendString("+++"); Delay_MS(50);
	ESP8266_ATSendString("+++"); Delay_MS(50);	
}

/**
 * 开启透传模式
 */
uint8_t ESP8266_OpenTransmission(void)
{
	//设置透传模式
	uint8_t cnt=2;
	while(cnt--)
	{
		USART1_ClearRX();
		ESP8266_ATSendString("AT+CIPMODE=1\r\n");  

		if(FindStr((char*)UartRxBuf,"OK",200)!=0)
			return 1;
	}
	return 0;
}


//*********************************************************

/**
 * 打开TCP连接
 * @param ip 目标IP
 * @param port 目标端口
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_StartTcp(char*ip, uint16_t port) {
  USART1_ClearTX();
	sprintf((char*)UartTxBuf, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ip, port);
  USART1_ClearRX();
	ESP8266_ATSendString((char*)UartTxBuf);
  Delay_MS(100);
	if(FindStr((char*)UartRxBuf,"OK",200) == 0)
    return 0;

  return 1;
}
/**
 * ESP8266发送数据
 * @param data 数据
 * @param len 数据长度
 * @return 返回1成功，返回0不成功
 */
uint8_t ESP8266_SendData(char*data, uint16_t len) {
  USART1_ClearTX();
  USART1_ClearRX();
	sprintf((char*)UartTxBuf, "AT+CIPSEND=0,%d\r\n", len);

	ESP8266_ATSendString((char*)UartTxBuf);
  Delay_MS(100);
	if(FindStr((char*)UartRxBuf,">",200) == 0)
    return 0;

  USART1_ClearTX();
  USART1_ClearRX();
  sprintf((char*)UartTxBuf, "%s\n", data);

	ESP8266_ATSendString((char*)UartTxBuf);
  Delay_MS(200);
	if(FindStr((char*)UartRxBuf,"SEND OK",200) == 0)
    return 0;

  return 1;
}



extern void MAIN_Handler_WifiCommand(char*buf, uint16_t len);

/**
 * 处理 ESP8266 返回的数据
 * 
 * （单条命令长度最大为64）
 */
void ESP8266_ReceiveHandle() {
  UartReceiveFlag = 0;

  uint16_t pos, cut_start, cut_end, cur, len, dataLen;
  char buf[64];
  char* str;
  
  str = (char*)UartRxBuf;
  len = strlen(str);
  pos = 0;

  // ESP8266 返回格式：
  /*
+IPD,0,4:Test
+IPD,0,4:Test0,CLOSED
  */
  //解析字符串中的 +IPD ，获取发送来的数据 ，并以保存至字符串缓冲区

  pos = 0;
  len = strlen(str);
  dataLen = 0;
  while (pos < len - 3) {
    if (str[pos] == '+' && str[pos + 1] == 'I' && str[pos + 2] == 'P' && str[pos + 3] == 'D') { //找到 +IPD
      pos = pos + 5;
      while (str[pos] != ',') pos++; cut_start = pos + 1; //找到第二个逗号
      while (str[pos] != ':') pos++; cut_end = pos; //找到冒号

      for (cur = 0; cut_start < cut_end; cur++, cut_start++) //拷贝出数据长度
        buf[cur] = str[cut_start];
      buf[cur] = '\0';
      dataLen = atoi(buf);

      if (dataLen > 0) {
        cut_start = pos + 1;
        cut_end = cut_start + dataLen;

        memset(buf, 0, 10);
        for (cur = 0; cut_start < cut_end; cur++, cut_start++) //拷贝出数据长度
          buf[cur] = str[cut_start];
        buf[cur] = '\0';

        //传递给主子程序处理
        MAIN_Handler_WifiCommand(buf, dataLen);
      }
    }
    pos++;
  }

  USART1_ClearRX();
}


