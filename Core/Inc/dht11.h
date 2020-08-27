#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f1xx_hal.h"

#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_PIN_8

#define DHT11_OUT_H HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET)
#define DHT11_OUT_L HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET)
#define DHT11_IN    HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN)

void DHT11_GPIO_Input(void);
void DHT11_GPIO_Output(void);

void DHT11_Init(void);
void DHT11_Reset(void);
uint16_t DHT11_Scan(void);
uint16_t DHT11_Read_Bit(void);
uint16_t DHT11_Read_Byte(void);
uint16_t DHT11_Read_Data(uint8_t *buffer);

#endif
