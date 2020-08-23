/**
  ******************************************************************************
  * @file           : delay.h
  * @brief          : header for delay.c file.
  *                   延时函数.
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f1xx_hal.h"

/**
 * @brief 延时毫秒（定时器1）
 * @param  nms 毫秒（量程0-8191ms）
 * @retval None
 */
void delay_ms(uint16_t nms);
/**
 * @brief 延时微秒
 * @param  nms 微秒
 * @retval None
 */
void delay_us(uint32_t us);


#endif