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

extern UART_HandleTypeDef huart1; 

//*******************************************************************************
//系统状态变量

uint8_t downKey = 0;
uint8_t keyMode = 0;

uint8_t currentUIMode = 0;

//*******************************************************************************
//系统设置变量

uint8_t wifiName[32];
uint8_t wifiPassword[32];

void MAIN_UI_ConnectWifi(void) {

  char ipBuf[16];

  currentUIMode = UI_MODE_CONNCT_TIP;

  ESP8266_GetCurrentIP(ipBuf);

  OLED_Clear();
  OLED_ShowString(0, 0, "Use CLW App to Connect WIFI", 8);
  OLED_ShowString(0, 2, "IP", 8);
  OLED_ShowString(0, 3, ipBuf, 8);
  OLED_ShowString(0, 4, "PORT", 8);
  OLED_ShowString(40, 4, CONFIG_SERVER_PORT, 8);
}
void MAIN_UI_ConnectingWIFI() {
  currentUIMode = UI_MODE_CONNCTING;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connecting", 8);
  OLED_ShowString(0, 0, "WIFI...", 8);
}
void MAIN_UI_ConnectWifiFailed(void) {
  currentUIMode = UI_MODE_CONNCT_ERR;

  OLED_Clear();
  OLED_ShowString(0, 0, "Connect WIFI Failed", 8);

  OLED_ShowString(0, 3, "OK -> Retry", 8);
  OLED_ShowString(0, 4, "CAN -> Config", 8);
}
void MAIN_UI_Menu() {
  currentUIMode = UI_MODE_MENU;

  OLED_Clear();
  OLED_ShowString(0, 0, "CSW Menu", 8);

  OLED_ShowString(0, 1, "1 > Reboot", 8);
  OLED_ShowString(0, 2, "2 > ResetSetting", 8);
  OLED_ShowString(0, 3, "3 > ConfigMode", 8);
  OLED_ShowString(0, 3, "C > Back", 8);
}
void MAIN_UI_Def() {
  currentUIMode = UI_MODE_DEF;
}

//*******************************************************************************

//*******************************************************************************

//读取设置
void MAIN_ReadSettings() {
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_Read(ADDR_WIFI_NAME, 32, &wifiName);
  AT24C02_Read(ADDR_WIFI_PASS, 32, &wifiPassword);
}
//设置还原
void MAIN_ResetSettings() {
  memset(wifiName, 0, sizeof(wifiName));
  memset(wifiPassword, 0, sizeof(wifiName));

  AT24C02_FlushAll();
}
//连接WIFI
void MAIN_DoConnectWIFI() {
  MAIN_UI_ConnectingWIFI();
  if(ESP8266_ConnectAP(wifiName, wifiPassword) == 0) {
    MAIN_ConnectServer();
    MAIN_StartWorker();
  }else MAIN_UI_ConnectWifiFailed();
}
//转为配置模式
void MAIN_ToConfigMode() {
  ESP8266_SetConfigServer();
  MAIN_UI_ConnectWifi();
}
//软件复位
void MAIN_Reboot() {
  __set_FAULTMASK(1);//关闭所有中断
  NVIC_SystemReset();//复位函数
}

//连接阿里物联网服务器
void MAIN_ConnectServer() {

}
//开始工作
void MAIN_StartWorker() {

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
  case KEY_MENU: MAIN_UI_Menu(); break;
  case KEY_CAN: {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_UI_Def(); break;
    case UI_MODE_CONNCT_ERR: MAIN_ToConfigMode(); break;
    }
    break;
  }
  case KEY_OK: {
    switch (currentUIMode)
    {
    case UI_MODE_CONNCT_ERR: MAIN_DoConnectWIFI(); break;
    case UI_MODE_MENU: MAIN_UI_Def(); break;
    }
    break;
  }
  case '1': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_Reboot(); break;
    }
    break;
  } 
  case '2': {
    switch (currentUIMode)
    {
    case UI_MODE_MENU: MAIN_ResetSettings(); break;
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

  uchar num[3];
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
  char buffer[16];

  switch (buf[0])
  {
  case 'C': {//控制和配置指令
    end = len;
    for(pos = 1; pos < len; pos++)
      if(buf[pos] == ':' || pos == len - 1) {
        end = pos;
        break;
      }

    if(strncmp(buf+1, "PASS", end - 1) == 0){
      strncpy(wifiPassword, buf + end, len - end);
      AT24C02_WriteOnePage(1, 0, wifiPassword);//写入24C02
    }
    else if(strncmp(buf+1, "WIFI", end - 1) == 0) {
      strncpy(wifiName, buf + end, len - end);
      AT24C02_WriteOnePage(0, 0, wifiName);//写入24C02
    }
    else if(strncmp(buf+1, "COONECT", end - 1) == 0)
      MAIN_DoConnectWIFI();//连接指令

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
  //初始化外设
  LED_init();
  OLED_Init();
  OLED_Draw12864BMP(INTRO_BMP);
  AT24C02_Init();
  KEYPAD_Init();

  //初始化 ESP8266
  if (!ESP8266_Init())
  {
    currentUIMode = UI_MODE_BOOT_FAIL;
    OLED_Clear();
    OLED_ShowString(0, 0, "!!! ERROR !!!", 16);
    OLED_ShowString(0, 2, "ESP8266 init failed", 8);
    OLED_ShowString(0, 4, "Press any key to RESET", 8);
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
  uint8_t key = KEYPAD_Scan();
  if(key != downKey) {
    downKey = key;
    if (0 < downKey && downKey < 17)
      MAIN_Handler_Key(downKey);
  }
}


/**
 * UART 回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	UNUSED(huart);
	if(huart == &huart1) 
    ESP8266_ReceiveHandle();
}