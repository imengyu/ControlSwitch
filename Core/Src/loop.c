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
#include "24c02.h"
#include "stdio.h"
#include "string.h"
#include "uimode.h"
#include "ds1302.h"
#include "dht11.h"

extern UART_HandleTypeDef huart1; 
extern UART_HandleTypeDef huart3; 

extern TIM_HandleTypeDef htim2;

//*******************************************************************************

extern char UartRxBuf[RX_LEN];
extern uint8_t UartReceiveFlag;

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

uint8_t currentUIMode = 0;
uint8_t oldUIMode = 0;
uint8_t currentUICanUseMenu = 1;

float humidness;
float temperature;

//*******************************************************************************
//系统设置变量

char wifiName[32];
char wifiPassword[32];

void MAIN_UI_GoUIMode(uint8_t mode) {
  switch(mode) {
    case UI_MODE_DEF: MAIN_UI_Def(); break;
    case UI_MODE_CONNCTING: MAIN_UI_ConnectingWIFI(); break;
    case UI_MODE_CONNCT_TIP: MAIN_UI_ConnectWifi(); break;
  }
}
void MAIN_UI_ConnectWifi(void) {

  char ipBuf[16];

  currentUIMode = UI_MODE_CONNCT_TIP;
  currentUICanUseMenu = 1;

  ESP8266_GetCurrentIP(ipBuf);

  printf("CurrentIP : %s\n\r", ipBuf);

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
  OLED_ShowString(0, 0, "Connecting", 8);
  OLED_ShowString(0, 0, "WIFI...", 8);
}
void MAIN_UI_ConnectWifiFailed(void) {
  currentUIMode = UI_MODE_CONNCT_ERR;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connect WIFI Failed", 8);

  OLED_ShowString(0, 3, "OK -> Retry", 8);
  OLED_ShowString(0, 4, "CAN -> Config", 8);
}
void MAIN_UI_Menu(uint8_t restoreOldMode) {
  if(restoreOldMode)
    oldUIMode = currentUIMode;
  currentUIMode = UI_MODE_MENU;
  currentUICanUseMenu = 0;

  OLED_Clear();
  OLED_ShowString(0, 0, "CSW Menu", 8);

  OLED_ShowString(0, 1, "1 > Reboot", 8);
  OLED_ShowString(0, 2, "2 > ResetSetting", 8);
  OLED_ShowString(0, 3, "3 > ConfigMode", 8);
  OLED_ShowString(0, 3, "C > Back", 8);
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
//读取设置
void MAIN_ReadSettings() {
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_Read(ADDR_WIFI_NAME, 32, (uint8_t*)&wifiName);
  AT24C02_Read(ADDR_WIFI_PASS, 32, (uint8_t*)&wifiPassword);
}
//设置还原
void MAIN_ResetSettings() {
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_FlushAll();
}
//连接阿里物联网服务器
void MAIN_ConnectServer() {
  printf("MAIN_ConnectServer");
  OLED_Clear();
  OLED_ShowString(0, 0, "OK!", 8);
  OLED_ShowString(0, 1, "> MAIN_ConnectServer", 8);
}
//开始工作
void MAIN_StartWorker() {

}
//连接WIFI
void MAIN_DoConnectWIFI() {
  serverConnected = 0;
  MAIN_UI_ConnectingWIFI();
  if(ESP8266_ConnectAP(wifiName, wifiPassword) == 0) {
    MAIN_ConnectServer();
    MAIN_StartWorker();
  }else MAIN_UI_ConnectWifiFailed();
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
  uint8_t buffer[5];
  if (DHT11_Read_Data(buffer) == 0)
  {
    humidness = buffer[0] + buffer[1] / 10.0f;
    temperature = buffer[2] + buffer[3] / 10.0f;
  }

  if(isInLowMode == 0 && currentUIMode == UI_MODE_DEF) 
    MAIN_UI_WriteData();
}
//刷新DS1302时间数据
void MAIN_RefeshDS1302() {
  DS1302_Read_Time();
  if(isInLowMode == 0 && currentUIMode == UI_MODE_DEF) 
    MAIN_UI_WriteTime();
}

//*******************************************************************************

//按键处理
void MAIN_Handler_KeyCode(char keyChar) {
  if(currentUIMode == UI_MODE_BOOT_FAIL) {
    MAIN_Reboot();
    return;
  }
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
    }
    break;
  }
  case KEY_OK: {
    switch (currentUIMode)
    {
    case UI_MODE_CONNCT_ERR: MAIN_DoConnectWIFI(); break;
    case UI_MODE_MENU: MAIN_UI_Def(); break;
    case UI_MODE_ASK_REBOOT: MAIN_Reboot(); break;
    case UI_MODE_ASK_RESETSET: MAIN_ResetSettings(); break;
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
  for(pos = 1; pos < len; pos++)
    if(buf[pos] == ':' || pos == len - 1) {
      end = pos;
      break;
    }

  switch (buf[0])
  {
  case 'C': {//配置指令
  
    if(strncmp(buf+1, "PASS", end - 1) == 0){
      strncpy(wifiPassword, buf + end, len - end);
      AT24C02_WriteOnePage(1, 0, (uint8_t*)wifiPassword);//写入24C02
    }
    else if(strncmp(buf+1, "WIFI", end - 1) == 0) {
      strncpy(wifiName, buf + end, len - end);
      AT24C02_WriteOnePage(0, 0, (uint8_t*)wifiName);//写入24C02
    }

    break;
  }
  case 'M': {//控制指令

    ESP8266_GetFirstConnectIP(ip);

    if(strncmp(buf+1, "COONECT", end - 1) == 0) {
  
      if(ESP8266_StartTcp(ip, 5000) == 1)
        ESP8266_SendData("OK", 3);

      MAIN_DoConnectWIFI();//开始连接
    }
    if(strncmp(buf+1, "TEST", end - 1) == 0) {
      if(ESP8266_StartTcp(ip, 5000) == 1)
        ESP8266_SendData("OK", 3);
    }
    break;
  }
  }

}

//*******************************************************************************

/**
 * 程序初始化
 */
void MAIN_Init(void)
{
  printf("CSW Boot");

  //初始化外设
  LED_init();
  OLED_Init();
  DS1302_Init();
  KEYPAD_Init();
  DHT11_Reset();
  
  HAL_TIM_Base_Start_IT(&htim2); //启动定时器

  //初始化 ESP8266
  if (ESP8266_Init() == 0)
  {
    printf("ESP8266_Init Failed !");
    MAIN_UI_InitFail();
    return;
  }

  printf("ESP8266_Init ok");

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
    else loopTick == 0;

    //每一分钟采集一次数据
    if(loopTick % 60 == 0) {
      MAIN_ReadDHT11();
      
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
    Delay_MS(100);
    ESP8266_ReceiveHandle();
  }

  Delay_MS(40);
}

//*******************************************************************************

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM2) {
    tim1sActived = 1;
  }
}

