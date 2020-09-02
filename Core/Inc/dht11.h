#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f1xx_hal.h"

#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_PIN_8

//IO方向设置
//#define DHT11_IO_IN()  {DHT11_GPIO_PORT->CRH &= 0xFFFFFFF0;	DHT11_GPIO_PORT->CRH |= 4;}	// PA8 IN  MODE
//#define DHT11_IO_OUT() {DHT11_GPIO_PORT->CRH &= 0xFFFFFFF0;	DHT11_GPIO_PORT->CRH |= 3;}	// PA8 OUT MODE

void DHT11_Init(void);
void DHT11_Reset(void);
uint8_t DHT11_Check(void);
uint8_t DHT11_Read_Bit(void);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read_Data(uint8_t *buffer);
uint8_t DHT11_Read_Data_Float(void);

#endif
