/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : 延时函数
  ******************************************************************************
  */

#include "delay.h"

extern TIM_HandleTypeDef htim3;

uint16_t delayMsTick = 0;

/**
 * @brief 延时毫秒
 * @param  nms 毫秒
 * @retval None
 */
void Delay_MS(uint16_t nms)
{
  HAL_Delay(nms);
}

/**
 * @brief 延时毫秒
 * @param  nms 毫秒
 * @retval None
 */
void Delay_MS_TIM(uint16_t nms)
{
  delayMsTick = 0;

  while(delayMsTick < nms) __NOP();
}

/**
 * @brief 定时器延时微秒
 * @param  nms 微秒（量程0-65534ms）
 * @retval None
 */
void Delay_US_TIM(uint16_t us)
{
  uint16_t differ = 0xffff - us - 5;
  
  __HAL_TIM_SET_COUNTER(&htim3, differ);//htim3
  __HAL_TIM_ENABLE(&htim3);
	
  while(differ < 0xffff - 5)
    differ = __HAL_TIM_GET_COUNTER(&htim3);
	
  __HAL_TIM_DISABLE(&htim3);
}
