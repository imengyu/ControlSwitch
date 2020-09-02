#include "esp8266_mqtt.h"
#include "esp8266.h"
#include "string.h"
#include "utils.h"
#include "delay.h"

//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

uint16_t MQTT_TxLen;

extern char UartTxBuf[TX_LEN];
extern char UartRxBuf[RX_LEN];

//MQTT发送数据
void MQTT_SendBuf(uint8_t *buf,uint16_t len)
{
	ESP8266_ATSendBuf(buf,len);
}	

//发送心跳包
void MQTT_SentHeart(void)
{
	MQTT_SendBuf((uint8_t *)parket_heart,sizeof(parket_heart));
}

//MQTT无条件断开
void MQTT_Disconnect(void)
{
	MQTT_SendBuf((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

//MQTT初始化
void MQTT_Init(void)
{
	USART1_ClearTX(); //清空发送缓冲
	USART1_ClearRX(); //清空接收缓冲
	
	//无条件先主动断开
	MQTT_Disconnect(); Delay_MS(100);
	MQTT_Disconnect(); Delay_MS(100);
}

//MQTT连接服务器的打包函数
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	uint16_t ClientIDLen,UsernameLen,PasswordLen,DataLen = 0;
  uint8_t encodedByte = 0,cnt = 2,wait;
	
	ClientIDLen = strlen(ClientID);
	UsernameLen = strlen(Username);
	PasswordLen = strlen(Password);
	MQTT_TxLen = 0;
	//可变报头+Payload  每个字段包含两个字节的长度标识
  DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);

	//固定报头
	//控制报文类型
  UartTxBuf[MQTT_TxLen++] = 0x10;		//MQTT Message Type CONNECT
	//剩余长度(不包括固定头部)
	
	do
	{
		encodedByte = DataLen % 0x80;
		DataLen = DataLen / 0x80;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 0x80;
		UartTxBuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//可变报头
	//协议名
	UartTxBuf[MQTT_TxLen++] = 0;        		// Protocol Name Length MSB    
	UartTxBuf[MQTT_TxLen++] = 4;        		// Protocol Name Length LSB    
	UartTxBuf[MQTT_TxLen++] = 'M';        	// ASCII Code for M    
	UartTxBuf[MQTT_TxLen++] = 'Q';        	// ASCII Code for Q    
	UartTxBuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	UartTxBuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	//协议级别
	UartTxBuf[MQTT_TxLen++] = 4;        		// MQTT Protocol version = 4    
	//连接标志
	UartTxBuf[MQTT_TxLen++] = 0xc2;        	// conn flags 
	UartTxBuf[MQTT_TxLen++] = 0x01;         // Keep-alive Time Length MSB    
	UartTxBuf[MQTT_TxLen++] = 0x2c;         // Keep-alive Time Length LSB  300S心跳包  

	UartTxBuf[MQTT_TxLen++] = BYTE1(ClientIDLen);// Client ID length MSB    
	UartTxBuf[MQTT_TxLen++] = BYTE0(ClientIDLen);// Client ID length LSB  	
	memcpy(&UartTxBuf[MQTT_TxLen],ClientID,ClientIDLen);
	MQTT_TxLen += ClientIDLen;
	
	if(UsernameLen > 0)
	{   
		UartTxBuf[MQTT_TxLen++] = BYTE1(UsernameLen);		//username length MSB    
		UartTxBuf[MQTT_TxLen++] = BYTE0(UsernameLen);    	//username length LSB    
		memcpy(&UartTxBuf[MQTT_TxLen],Username,UsernameLen);
		MQTT_TxLen += UsernameLen;
	}
	
	if(PasswordLen > 0)
	{    
		UartTxBuf[MQTT_TxLen++] = BYTE1(PasswordLen);		//password length MSB    
		UartTxBuf[MQTT_TxLen++] = BYTE0(PasswordLen);    	//password length LSB  
		memcpy(&UartTxBuf[MQTT_TxLen],Password,PasswordLen);
		MQTT_TxLen += PasswordLen; 
	}    
	
	UartTxBuf[MQTT_TxLen++] = '\n';
	
	while(cnt--)
	{
		USART1_ClearRX();
		MQTT_SendBuf((uint8_t*)UartTxBuf,MQTT_TxLen);
		wait=30;//等待3s时间
		while(wait--)
		{
			//CONNECT
			if(UartRxBuf[0] == parket_connetAck[0] && UartRxBuf[1]==parket_connetAck[1]) //连接成功			   
				return 1;//连接成功
			Delay_MS(100);			
		}
	}
	return 0;
}

//MQTT订阅/取消订阅数据打包函数
//topic       主题 
//qos         消息等级 
//whether     订阅/取消订阅请求包
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether)
{    
	uint16_t topiclen = strlen(topic), DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
  uint8_t encodedByte,cnt=2,wait;

	MQTT_TxLen=0;

	//固定报头
	//控制报文类型
	if(whether) UartTxBuf[MQTT_TxLen++] = 0x82; //消息类型和标志订阅
	else	UartTxBuf[MQTT_TxLen++] = 0xA2;    //取消订阅

	//剩余长度
	do
	{
		encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		UartTxBuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	//可变报头
	UartTxBuf[MQTT_TxLen++] = 0;				//消息标识符 MSB
	UartTxBuf[MQTT_TxLen++] = 0x01;           //消息标识符 LSB
	//有效载荷
	UartTxBuf[MQTT_TxLen++] = BYTE1(topiclen);//主题长度 MSB
	UartTxBuf[MQTT_TxLen++] = BYTE0(topiclen);//主题长度 LSB   
	memcpy(&UartTxBuf[MQTT_TxLen],topic,topiclen);
	MQTT_TxLen += topiclen;

	if(whether)
	{
		UartTxBuf[MQTT_TxLen++] = qos;//QoS级别
	}

	UartTxBuf[MQTT_TxLen++] = '\n';
	
	while(cnt--)
	{
		USART1_ClearRX();
		MQTT_SendBuf((uint8_t*)UartTxBuf,MQTT_TxLen);
		wait=30;//等待3s时间
		while(wait--)
		{
			if(UartRxBuf[0] == parket_subAck[0] && UartRxBuf[1] == parket_subAck[1]) //订阅成功			   
			{
				return 1;//订阅成功
			}
			Delay_MS(100);			
		}
	}
	if(cnt) return 1;	//订阅成功
	return 0;
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos)
{  
	uint16_t topicLength = strlen(topic);    
	uint16_t messageLength = strlen(message);     
	static uint16_t id=0;
	uint16_t DataLen;
  uint8_t encodedByte;

	MQTT_TxLen=0;
	//有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
	//QOS为0时没有标识符
	//数据长度             主题名   报文标识符   有效载荷
	if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
	else	DataLen = (2+topicLength) + messageLength;   

    //固定报头
	//控制报文类型
	UartTxBuf[MQTT_TxLen++] = 0x30;    // MQTT Message Type PUBLISH  

	//剩余长度
	do
	{
		encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		UartTxBuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	UartTxBuf[MQTT_TxLen++] = BYTE1(topicLength);//主题长度MSB
	UartTxBuf[MQTT_TxLen++] = BYTE0(topicLength);//主题长度LSB 
	memcpy(&UartTxBuf[MQTT_TxLen],topic,topicLength);//拷贝主题
	MQTT_TxLen += topicLength;
        
	//报文标识符
	if(qos)
	{
			UartTxBuf[MQTT_TxLen++] = BYTE1(id);
			UartTxBuf[MQTT_TxLen++] = BYTE0(id);
			id++;
	}
	memcpy(&UartTxBuf[MQTT_TxLen],message,messageLength);
  MQTT_TxLen += messageLength;
        
	MQTT_SendBuf((uint8_t*)UartTxBuf,MQTT_TxLen);
  return MQTT_TxLen;
}

