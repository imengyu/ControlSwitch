/**
  ******************************************************************************
  * @file           : loop.h
  * @brief          : 程序主循环。放在这里不用让MX每次更新的时候把代码写坏
  ******************************************************************************
  */

#include "commdef.h"
#include "main.h"
#include "loop.h"
#include "led.h"
#include "delay.h"
#include "keypad.h"
#include "oled.h"
#include "oled_font.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "24c02.h"
#include "stdio.h"
#include "string.h"
#include "uimode.h"
#include "ds1302.h"
#include "dht11.h"

//*******************************************************************************

//阿里云MQTT接入信息

#define MQTT_BROKERADDRESS "a1GJvSV1TYm.iot-as-mqtt.cn-shanghai.aliyuncs.com" //节点
#define MQTT_CLIENTID "000001|securemode=3,signmethod=hmacsha1|"
#define MQTT_USARNAME "DEBUG&a1GJvSV1TYm"
#define MQTT_PASSWD "4550C1212BB2E3E255FB1DE26499C275E95062EF"

//通信 Topic 
#define MQTT_TOPIC_SET_PROP "/sys/a1GJvSV1TYm/DEBUG/thing/service/property/set"
#define MQTT_TOPIC_POST_PROP "/sys/a1GJvSV1TYm/DEBUG/thing/event/property/post"

//*******************************************************************************

extern UART_HandleTypeDef huart1; 
extern UART_HandleTypeDef huart3; 

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

//*******************************************************************************

extern char UartRxBuf[RX_LEN];
extern uint8_t UartReceiveFlag;
extern uint8_t UartReceiveLength;

extern Timedata ReadTime;



//*******************************************************************************
//系统状态变量

uint8_t downKey = 0;
uint8_t keyMode = 0;
uint8_t tim1sActived = 0;
uint16_t loopTick = 0;
uint16_t lastKeyTick = 0;
uint8_t serverConnected = 0;
uint8_t isInLowMode = 0;
uint8_t mainSwModified = 0;

uint8_t currentUIMode = 0;
uint8_t oldUIMode = 0;
uint8_t currentUICanUseMenu = 1;
uint8_t uiIndex = 0;

uint8_t testTick = 0;

//主开关数据
uint8_t switchMainDebug = 0;
uint8_t switchMainA = 0;
uint8_t switchMainB = 0;
uint8_t switchMainC = 0;

//温度数据
extern float humidness;
extern float temperature;

//*******************************************************************************
//系统设置变量

char wifiName[32];
char wifiPassword[32];

void MAIN_UI_ConnectWifi(void) {

  char ipBuf[16];

  currentUIMode = UI_MODE_CONNCT_TIP;
  currentUICanUseMenu = 1;

  ESP8266_GetCurrentIP(ipBuf);

  Debug_Info("CSW", "CurrentIP : %s\n\r", ipBuf);

  OLED_Clear();
  OLED_ShowString(0, 0, "Use CLWApp to", 8);
  OLED_ShowString(0, 1, "Connect WIFI", 8);
  OLED_ShowString(0, 3, "IP", 8);
  OLED_ShowString(0, 4, ipBuf, 8);
  OLED_ShowString(0, 5, "PORT", 8);
  OLED_ShowString(40, 5, CONFIG_SERVER_PORT, 8);
}
void MAIN_UI_ConnectingWIFI() {
  currentUIMode = UI_MODE_CONNCTING;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connecting WIFI", 8);
  OLED_ShowString(0, 1, "...", 8);
}
void MAIN_UI_ConnectingServer() {
  currentUIMode = UI_MODE_CONNCTING;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connecting ", 8);
  OLED_ShowString(0, 1, "Server...", 8);
}
void MAIN_UI_ConnectWifiFailed(void) {
  currentUIMode = UI_MODE_CONNCT_ERR;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connect WIFI Failed", 8);

  OLED_ShowString(0, 3, "OK  > Retry", 8);
  OLED_ShowString(0, 4, "CAN > Config", 8);
}
void MAIN_UI_ConnectServerFailed(void) {
  currentUIMode = UI_MODE_CONNCT_SERVER_ERR;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connect Server Failed !", 8);

  OLED_ShowString(0, 3, "OK  > Retry", 8);
  OLED_ShowString(0, 4, "CAN > Config", 8);
}

void MAIN_UI_Menu(uint8_t restoreOldMode) {
  if(restoreOldMode)
    oldUIMode = currentUIMode;
  currentUIMode = UI_MODE_MENU;
  currentUICanUseMenu = 0;
  uiIndex = 0;

  OLED_Clear();
  OLED_ShowString(0, uiIndex++, "CSW Menu", 8);

  OLED_ShowString(0, uiIndex++, "1 > Reboot", 8);
  OLED_ShowString(0, uiIndex++, "2 > ResetSetting", 8);
  OLED_ShowString(0, uiIndex++, "3 > ConfigMode", 8);
  
  if(serverConnected) {
    OLED_ShowString(0, uiIndex++, "8 > ForceUpdate", 8);
    OLED_ShowString(0, uiIndex++, "9 > Disconnect", 8);
  }

  OLED_ShowString(0, 7, "C > Back", 8);
}
void MAIN_UI_Def() {
  currentUIMode = UI_MODE_DEF;
  currentUICanUseMenu = 1;
  OLED_Clear();
}
void MAIN_UI_InitFail() {
  currentUIMode = UI_MODE_BOOT_FAIL;
  currentUICanUseMenu = 0;
  OLED_Clear();
  OLED_ShowString(0, 0, "!!! ERROR !!!", 8);
  OLED_ShowString(0, 2, "ESP8266 init failed", 8);
  OLED_ShowString(0, 6, "Press any key to RESET", 8); 
}
void MAIN_UI_WriteTime() {
  char buf[11];
  sprintf(buf, "%04d/%02d/%02d", ReadTime.year, ReadTime.month, ReadTime.day);
  OLED_ShowString(0, 6, buf, 8); 
  sprintf(buf, "%02d:%02d:%02d", ReadTime.hour, ReadTime.minute, ReadTime.second);
  OLED_ShowString(0, 7, buf, 8); 
}
void MAIN_UI_WriteData() {
  char buf[11];
  sprintf(buf, "%4.2f°C", temperature);
  OLED_ShowString(0, 4, buf, 8); 
  sprintf(buf, "%4.2f%%RH", humidness);
  OLED_ShowString(0, 5, buf, 8); 
}
void MAIN_UI_GoUIMode(uint8_t mode) {
  switch(mode) {
    case UI_MODE_DEF: MAIN_UI_Def(); break;
    case UI_MODE_CONNCTING: MAIN_UI_ConnectingWIFI(); break;
    case UI_MODE_CONNCT_TIP: MAIN_UI_ConnectWifi(); break;
  }
}
void MAIN_UI_Disconnected() {
  currentUIMode = UI_MODE_MESSAGE;

  OLED_Clear();
  OLED_ShowString(0, 0, "Disconnected !", 8);
}

//*******************************************************************************

void MAIN_UI_AskReboot() {
  currentUIMode = UI_MODE_ASK_REBOOT;

  OLED_Clear();
  OLED_ShowString(0, 0, "Do you want REBOOT?", 8);

  OLED_ShowString(0, 2, "OK  > Reboot", 8);
  OLED_ShowString(0, 3, "CAN > Back", 8);
}
void MAIN_UI_AskResetSettings() {
  currentUIMode = UI_MODE_ASK_RESETSET;

  OLED_Clear();
  OLED_ShowString(0, 0, "Do you want to Reset all settings?", 8);

  OLED_ShowString(0, 3, "OK  > Yes", 8);
  OLED_ShowString(0, 4, "CAN > Back", 8);
}

//*******************************************************************************

//*******************************************************************************

//*******************************************************************************

//读取设置
void MAIN_ReadSettings() {
  uint8_t buf[8];

  //WIFI 配置
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_Read(ADDR_WIFI_NAME, 32, (uint8_t*)&wifiName);
  AT24C02_Read(ADDR_WIFI_PASS, 32, (uint8_t*)&wifiPassword);

  //主控设置
  AT24C02_Read(0x40, 8, buf);
  switchMainDebug = buf[0];
  switchMainA = buf[1];
  switchMainB = buf[2];
  switchMainC = buf[3];

  MAIN_UpdateSwSettings(0);
  MAIN_UpdateSwSettings(1);
  MAIN_UpdateSwSettings(2);
  MAIN_UpdateSwSettings(3);

  mainSwModified = 0;
}
//设置还原
void MAIN_ResetSettings() {
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_FlushAll();
}
//保存主控设置
void MAIN_SaveMainSwSettings() {
  uint8_t buf[8];
  buf[0] = switchMainDebug;
  buf[2] = switchMainA;
  buf[3] = switchMainB;
  buf[4] = switchMainC;

  AT24C02_WriteOnePage(8, 0, buf);

  mainSwModified = 0;
}


//*******************************************************************************

void MAIN_UpdateSwSettings(uint8_t index) {
  mainSwModified = 1;
  switch (index)
  {
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    break;
  }
}

void MAIN_SwitchLowMode(uint8_t mode) {
  if(isInLowMode != mode) {
    isInLowMode = mode;
    if(isInLowMode) {
      isInLowMode = 0;
      OLED_Display_On();
    } else {
      isInLowMode = 1;
      OLED_Display_Off();
    }
  }
  
}

//*******************************************************************************

//连接阿里物联网服务器
void MAIN_ConnectServer() {
  serverConnected = 0;
  Debug_Info("CSW", "MAIN_ConnectServer...\n");

  MAIN_UI_ConnectingServer();

  if(ESP8266_ConnectServer("TCP", MQTT_BROKERADDRESS, 1883) != 0) {

    MQTT_Init();
    if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0) {
      MAIN_UI_Def();
      serverConnected = 1;
      //订阅MQTT主题
			if(MQTT_SubscribeTopic(MQTT_TOPIC_SET_PROP, 0, 1) == 0)
		    Debug_Error("CSW", "MQTT_SubscribeTopic %s failed! \n", MQTT_TOPIC_SET_PROP);

      Debug_Info("CSW", "ConnectServer success!\n");
		}
		else {
      Debug_Error("CSW", "Connect aliyun iot mqtt failed! \n");
      MAIN_UI_ConnectServerFailed();
    }
	}
	else {
    Debug_Error("CSW", "Connect aliyun iot server failed! \n");
    MAIN_UI_ConnectServerFailed();
  }
}
//断开与服务器的连接
void MAIN_DisConnectServer() {
  if(serverConnected == 1) {
    MQTT_Disconnect();
    ESP8266_ExitUnvarnishedTrans();
    ESP8266_DisconnectServer();
    serverConnected = 0;
     Debug_Info("CSW", "DisconnectServer!\n");
  }

}

//开始工作
void MAIN_StartWorker() {
	loopTick = 59;
}

//连接WIFI
void MAIN_DoConnectWIFI() {
  serverConnected = 0;
  MAIN_UI_ConnectingWIFI();
  if(ESP8266_ConnectAP(wifiName, wifiPassword) == 0)
    MAIN_UI_ConnectWifiFailed();
  else {
		Debug_Info("CSW", "Connect WIFI %s success!\n", wifiName);
    MAIN_ConnectServer();
    MAIN_StartWorker();
  }
}
//转为配置模式
void MAIN_ToConfigMode() {
  serverConnected = 0;
  ESP8266_SetConfigServer();
  MAIN_UI_ConnectWifi();
}
//软件复位
void MAIN_Reboot() {
  __set_FAULTMASK(1);//关闭所有中断
  NVIC_SystemReset();//复位函数
}

//*******************************************************************************

//读取DHT11数据
void MAIN_ReadDHT11() {
  DHT11_Read_Data_Float();

  if(isInLowMode == 0 && currentUIMode == UI_MODE_DEF) 
     MAIN_UI_WriteData();
}
//刷新DS1302时间数据
void MAIN_RefeshDS1302() {
  //DS1302_Read_Time();
  //if(isInLowMode == 0 && currentUIMode == UI_MODE_DEF) 
  //  MAIN_UI_WriteTime();
}

//*******************************************************************************

//上报数据至服务器
void MAIN_PostDataToServer() {
  char mqtt_message[200];
  sprintf(mqtt_message,
"{\"method\":\"thing.event.property.pos\",id:\"%d\",\"params\":{\
\"CurrentTemperature\":%.1f,\
\"RelativeHumidity\":%.1f,\
\"PowerState\":%d,\
},\"version\":\"1.0\"}",
    genNoDuplicateInteger(),
    (float)temperature,
    (float)humidness,
    switchMainDebug == 1 ? 1 : 0
	);

	MQTT_PublishData(MQTT_TOPIC_POST_PROP, mqtt_message, 0);
}

//*******************************************************************************

//处理云平台接收的数据
void MAIN_Handler_CloudCommand(char*buf, uint16_t len) {
  char* pos = 0;
  buf = strstr(buf, "params\":");
  if(buf != 0) {
    pos = strstr(buf, "powerstate");
    if(pos != 0) {
      if(*(pos+13) == '1') {
        switchMainDebug = 1;
        LED_MainOn();
      }else if(*(pos+13) == '0') {
        switchMainDebug = 0;
        LED_MainOff();
      }
    }
  }
}
//按键处理
void MAIN_Handler_KeyCode(char keyChar) {

  //any key handler
  if(currentUIMode == UI_MODE_BOOT_FAIL) {
    MAIN_Reboot();
    return;
  }else if(currentUIMode == UI_MODE_MESSAGE) {
    MAIN_UI_Menu(0);
    return;
  }

  //UI Key handler
  switch (keyChar)
  {
  case KEY_MENU: if(currentUICanUseMenu) MAIN_UI_Menu(1); break;
  case KEY_CAN: {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_UI_GoUIMode(oldUIMode); break;
    case UI_MODE_ASK_REBOOT: MAIN_UI_Menu(0); break;
    case UI_MODE_ASK_RESETSET: MAIN_UI_Menu(0); break;
    case UI_MODE_CONNCT_ERR: MAIN_ToConfigMode(); break;
    case UI_MODE_CONNCT_SERVER_ERR: MAIN_ToConfigMode(); break;
    }
    break;
  }
  case KEY_OK: {
    switch (currentUIMode)
    {
    case UI_MODE_CONNCT_ERR: MAIN_DoConnectWIFI(); break;
    case UI_MODE_MENU: MAIN_UI_Def(); break;
    case UI_MODE_ASK_REBOOT: MAIN_Reboot(); break;
    case UI_MODE_ASK_RESETSET: MAIN_ResetSettings(); MAIN_UI_Menu(0); break;
    case UI_MODE_CONNCT_SERVER_ERR: MAIN_ConnectServer(); break;
    }
    break;
  }
  case '1': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_UI_AskReboot(); break;
    }
    break;
  } 
  case '2': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_UI_AskResetSettings(); break;
    }
    break;
  }
  case '3': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_ToConfigMode(); break;
    }
    break;
  }
  case '8': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: 
      loopTick = 59;
      MAIN_UI_GoUIMode(oldUIMode);
      break;
    }
    break;
  }
  case '9': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: 
      MAIN_DisConnectServer();
      MAIN_UI_Disconnected();
      break;
    }
    break;
  }
  default:
    break;
  }
}
//按键处理
void MAIN_Handler_Key(uint8_t key) {

  char num[3];
  sprintf(num, "%d", downKey);
  OLED_ShowString(111, 7, num, 8);

  switch (downKey)
  {
  case 1: MAIN_Handler_KeyCode('1'); break;
  case 2: MAIN_Handler_KeyCode('2'); break;
  case 3: MAIN_Handler_KeyCode('3'); break;
  case 4: MAIN_Handler_KeyCode(KEY_MOD); break;
  case 5: MAIN_Handler_KeyCode('4'); break;
  case 6: MAIN_Handler_KeyCode('5'); break;
  case 7: MAIN_Handler_KeyCode('6'); break;
  case 8: MAIN_Handler_KeyCode(KEY_UP); break;
  case 9: MAIN_Handler_KeyCode('7'); break;
  case 10: MAIN_Handler_KeyCode('8'); break;
  case 11: MAIN_Handler_KeyCode('9'); break;
  case 12: MAIN_Handler_KeyCode(KEY_DOWN); break;
  case 13: MAIN_Handler_KeyCode(KEY_MENU); break;
  case 14: MAIN_Handler_KeyCode('0'); break;
  case 15: MAIN_Handler_KeyCode(KEY_CAN); break;
  case 16: MAIN_Handler_KeyCode(KEY_OK); break;
  default: break;
  }
}
//处理wifi接收的数据
void MAIN_Handler_WifiCommand(char*buf, uint16_t len) {

  uint16_t pos, end;
  char ip[17];

  end = len;
  for(pos = 2; pos < len; pos++)
    if(buf[pos] == ':') {
      end = pos + 1;
      break;
    }else if(pos == len - 1) {
      end = pos;
      break;
    }

  Debug_Debug("CSW", "Handler_WifiCommand (%d) : %s", len, buf);

  switch (buf[0])
  {
  case 'C': {//配置指令
  
    if(strncmp(buf + 2, "PASS", 4) == 0){
      strncpy(wifiPassword, buf + end, len - end);
      AT24C02_WriteOnePage(4, 0, (uint8_t*)wifiPassword);//按页写入24C02
      AT24C02_WriteOnePage(5, 0, (uint8_t*)wifiPassword + 0x8);
      AT24C02_WriteOnePage(6, 0, (uint8_t*)wifiPassword + 0x16);
      AT24C02_WriteOnePage(7, 0, (uint8_t*)wifiPassword + 0x18);
    }
    else if(strncmp(buf + 2, "WIFI", 4) == 0) {
      strncpy(wifiName, buf + end, len - end);
      AT24C02_WriteOnePage(0, 0, (uint8_t*)wifiName);//按页写入24C02
      AT24C02_WriteOnePage(1, 0, (uint8_t*)wifiName + 0x8);
      AT24C02_WriteOnePage(2, 0, (uint8_t*)wifiName + 0x16);
      AT24C02_WriteOnePage(3, 0, (uint8_t*)wifiName + 0x18);
    }
    else if(strncmp(buf + 2, "TIM", 3) == 0) {

      ReadTime.year = (buf[pos + 2] << 8) + buf[pos + 3];
      ReadTime.month = buf[pos + 4];
      ReadTime.day = buf[pos + 5];
      ReadTime.hour = buf[pos + 6];
      ReadTime.minute = buf[pos + 7];
      ReadTime.second = buf[pos + 8];
      ReadTime.weekday = buf[pos + 9];

      DS1302_Burst_Writetime();//写入DS1302
    }
    break;
  }
  case 'M': {//控制指令

    USART1_ClearRX();

    ESP8266_GetFirstConnectIP(ip);

    if(strncmp(buf + 2, "CONNECT", end - 2) == 0) {
  
      Delay_MS(200);
			
      if(ESP8266_StartTcp(ip, 5000) == 1)
        ESP8266_SendData("OK", 2);
			
			Delay_MS(500);
			
      MAIN_DoConnectWIFI();//开始连接
    }
    if(strncmp(buf + 2, "TEST", end - 2) == 0) {

      Delay_MS(100);
      if(ESP8266_StartTcp(ip, 5000) == 1)
        ESP8266_SendData("OK", 2);
    }
    break;
  }
  case 0x30: {//MQTT Alink 云平台到设备消息
    MAIN_Handler_CloudCommand(buf + 4, len - 4);
    break;
  }
  default:
    break;
  }

}

//*******************************************************************************

/**
 * 程序初始化
 */
void MAIN_Init(void)
{
  Debug_Info("CSW", "Booting\n");

  DHT11_Init();

  Delay_MS(1200);
  DHT11_Read_Data_Float();

  Debug_Info("CSW", "temperature : %f , humidness : %f \n", temperature, humidness);  


  //启动定时器
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3); 

	return;

  //Debug_Info("CSW", "PCLK1Freq : %d\n", HAL_RCC_GetPCLK1Freq());  

  //初始化外设
  LED_init();
  OLED_Init();
  DS1302_Init();
  KEYPAD_Init();
  DHT11_Init();

  MAIN_ReadDHT11();
  return;

  //初始化 ESP8266
  if (ESP8266_Init() == 0)
  {
    Debug_Error("CSW", "ESP8266_Init Failed !\n");
    MAIN_UI_InitFail();
    return;
  }

  //读取设置
  MAIN_ReadSettings();

  //如果没有配置 WIFI，则转为配置模式
  if(strcmp(wifiName, "") == 0) {
    MAIN_ToConfigMode();
    return;
  }

  //连接WIFI
  MAIN_DoConnectWIFI();
}
/**
 * 程序循环
 */
void MAIN_Loop(void)
{
  //扫描键盘
  uint8_t key = KEYPAD_Scan();
  if(key != downKey) {
    downKey = key;
    if (0 < downKey && downKey < 17) {
      lastKeyTick = 0;
      MAIN_SwitchLowMode(0);
      MAIN_Handler_Key(downKey);
    }
  }

  //1S处理函数
  if(tim1sActived == 1) {

    //计数器
    if(loopTick < 0xfffe) loopTick++;
    else loopTick = 0;

    //每300秒发送MQTT心跳包
    if(serverConnected && loopTick % 300 == 0) MQTT_SentHeart();

    //每一分钟采集上报一次数据
    if(loopTick % 60 == 0) {

      MAIN_ReadDHT11();

      //上报数据
      if(serverConnected) MAIN_PostDataToServer();

      //如果更改了状态，则保存状态
      if(mainSwModified) MAIN_SaveMainSwSettings();
    }

    //每秒读取DS1302
    MAIN_RefeshDS1302();

    //键盘长时间无响应时间
    if(lastKeyTick < 0xfffe) lastKeyTick++;
    else lastKeyTick = 0x0fff;

    //如果100s无响应，则关闭显示器
    if(lastKeyTick == 100)
      MAIN_SwitchLowMode(1);

    tim1sActived = 0;
  }

  //ESP8266 UART接收
  if(USART1_GetReceiveFlag()) {

    ESP8266_ReceiveHandle();
  } 

  Delay_MS(40);
}

//*******************************************************************************

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM2) tim1sActived = 1;
}

