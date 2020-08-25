/**
  ******************************************************************************
  * @file           : led.h
  * @brief          : header for led.c file.
  *                   板载LED控制.
  ******************************************************************************
  * LED 在开发板上位于 C13
  ******************************************************************************
  */

#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

#define LED_IO GPIO_PIN_13

/**
  * @brief 初始化 LED
  */
void LED_init(void);
/**
  * @brief 关闭板载 LED
  */
#define LED_MainOff() HAL_GPIO_WritePin(GPIOC, LED_IO, GPIO_PIN_SET)
/**
  * @brief 开启板载 LED
  */
#define LED_MainOn() HAL_GPIO_WritePin(GPIOC, LED_IO, GPIO_PIN_RESET)

#endif



