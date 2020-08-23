/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : 延时函数
  ******************************************************************************
  */

#include "delay.h"

extern TIM_HandleTypeDef htim1;

/**
 * @brief 延时毫秒（定时器1）
 * @param  nms 毫秒（量程0-8191ms）
 * @retval None
 */
void delay_ms(uint16_t nms)
{
  HAL_Delay(nms);
}
/**
 * @brief 延时微秒
 * @param  nms 微秒（量程0-8191ms）
 * @retval None
 */
void delay_us(uint32_t us)
{
  uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
  while (delay--)
	{
		;
	}
}