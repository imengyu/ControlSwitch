#include "dht11.h"
#include "delay.h"
#include "utils.h"

float humidness;
float temperature;
uint8_t dht11IO = 0;
uint8_t dht11test = 0;

#define DHT11_OUT(x) HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, x)

/**
 * DHT11初始化
 */
void DHT11_Init(void)
{
  humidness = 0;
  temperature = 0;
}

uint8_t DHT11_DO_IN(void) {
  return HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
}

void DHT11_IO_OUT(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}
void DHT11_IO_IN(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * DHT11开始
 * @return 返回1出现错误，返回0成功
 */
uint8_t DHT11_Start(void)
{ 
  uint8_t retry;

  // 按照DHT11手册步骤
  DHT11_IO_OUT();
  DHT11_OUT(GPIO_PIN_SET);
  Delay_MS(20);
  DHT11_OUT(GPIO_PIN_RESET);//拉低18ms
  Delay_MS(20);
  DHT11_OUT(GPIO_PIN_SET);//拉高
  DHT11_IO_IN();
  
  retry = 0; //等待变为低电平
  while (DHT11_DO_IN() == 1 && retry < 200) 
  {
    retry++;
    Delay_US(1);
  }
  if(retry >= 200) {
    Debug_Debug("DHT11", "err1 %d",retry);
    return 1;
  }

  retry = 0; //等待变为高电平
  while (DHT11_DO_IN() == 0 && retry < 200) 
  {
    retry++;
    Delay_US(1);
  }
  if(retry >= 200) {
    Debug_Debug("DHT11", "err2 %d",retry);
    return 1;
  }

  //开始传输数据
  return 0;
}


uint8_t DHT11_Read_Byte(void)
{
  uint8_t retry;
  uint8_t i;
  uint8_t data = 0;
  for (i = 0; i < 8; i++)
  {
    //等待变为低电平
    retry = 0;
    while (DHT11_DO_IN() == 1 && retry < 50) 
    {
      retry++;
      Delay_US(1);
    }
    //等待变为高电平
    retry = 0;
    while (DHT11_DO_IN() == 0 && retry < 50) 
    {
      retry++;
      Delay_US(1);
    }

    //高电平 26-28us 为0 , 70us为1
    Delay_US(30); //wait 40us
    
    data <<= 1;
    data |= DHT11_DO_IN();
  }
  return data;
}

uint8_t DHT11_Read_Data_Float(void)
{
  uint8_t buf[5];
  uint8_t i;
  if(DHT11_Start() == 0) {

    for (i = 0; i < 5; i++) //读取40位数据
    {
      buf[i] = DHT11_Read_Byte();
    }
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
    {
      humidness = ((buf[0] << 8) + buf[1]) / 10.0f;
      temperature = ((buf[2] << 8) + buf[3]) / 10.0f;
      return 0;
    }
  }
  return 1;
}

