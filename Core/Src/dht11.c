#include "dht11.h"
#include "delay.h"

void DHT11_GPIO_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; //浮空输入
    
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

void DHT11_GPIO_Onput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //推挽输出

    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

void DHT11_Reset(void)
{
  // 按照DHT11手册步骤
  DHT11_GPIO_Output();
  DHT11_OUT_L;
  Delay_US(19000);
  DHT11_OUT_H;
  Delay_US(30);
  DHT11_GPIO_Input();
}

uint16_t DHT11_Scan(void)
{
  return DHT11_IN;
}

uint16_t DHT11_Read_Bit(void)
{
    while (DHT11_IN == RESET);
    Delay_US(40);
    if (DHT11_IN == SET)
    {
        while (DHT11_IN == SET);
        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t DHT11_Read_Byte(void)
{
    uint16_t i;
    uint16_t data = 0;
    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= DHT11_Read_Bit();
    }
    return data;
}

uint16_t DHT11_Read_Data(uint8_t *buffer)
{
    uint16_t i = 0;
    
    DHT11_reset();
    if (DHT11_Scan() == RESET)
    {
        //检测到DHT11响应
        while (DHT11_Scan() == RESET);
        while (DHT11_Scan() == SET);
        for (i = 0; i < 5; i++)
        {
            buffer[i] = DHT11_Read_Byte();
        }
        
        while (DHT11_Scan() == RESET);
        DHT11_GPIO_Output();
        DHT11_OUT_H;
        
        uint8_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
        if (checksum != buffer[4])
        {
            // checksum error
            return 1;
        }
    }
    
    return 0;
}