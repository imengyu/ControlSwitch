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

uchar ledOn = FALSE;
uchar ledLoopCount = 0;
uchar downKey = 0;

/**
 * 程序初始化
 */
void MAIN_Init(void)
{
  LED_init();
  OLED_Init();
  OLED_Draw12864BMP(INTRO_BMP);

  KEYPAD_Init();
  if (!ESP8266_Init())
  {
    OLED_Clear();
    OLED_ShowString(0, 0, "!!! ERROR !!!", 16);
    OLED_ShowString(0, 2, "ESP8266 failed to init", 16);
    return;
  }

  delay_ms(200);
  OLED_Clear();
  OLED_ShowString(0, 2, "Hello world!", 16);
}
/**
 * 程序循环
 */
void MAIN_Loop(void)
{

  //LED 控制
  if (ledLoopCount < 32)
  {
    if (ledOn)
    {
      LED_MainOff();
      ledOn = 0;
      delay_ms(500);
    }
    else
    {
      LED_MainOn();
      ledOn = 1;
      delay_ms(200);
    }

    ledLoopCount++;
  }

  downKey = KEYPAD_Scan();
  if (0 < downKey && downKey < 17)
  {

    uchar num[3];
    sprintf(num, "%d", downKey);
    OLED_ShowString(0, 4, "Key code : ", 16);
    OLED_ShowString(80, 4, num, 16);

    switch (downKey)
    {
    case 1:

      break;
    case 2:

      break;
    case 3:

      break;
    case 4:

      break;
    case 5:

      break;
    case 6:

      break;
    case 7:

      break;
    case 8:

      break;
    case 9:

      break;
    case 10:

      break;
    case 11:

      break;
    case 12:

      break;
    case 13:

      break;
    case 14:

      break;
    case 15:

      break;
    case 16:

      break;
    default:
      break;
    }
  }
}
