/**
  ******************************************************************************
  * @file           : keypad.h
  * @brief          : header for keypad.c file.
  *                   4x4矩阵键盘
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __KEYPAD_H
#define __KEYPAD_H

#include "stm32f1xx_hal.h"


#define KEY_PORT_ROW GPIOB
#define KEY_row0_Pin	GPIO_PIN_12
#define KEY_row1_Pin	GPIO_PIN_13
#define KEY_row2_Pin	GPIO_PIN_14
#define KEY_row3_Pin	GPIO_PIN_15

#define KEY_PORT_COL GPIOA
#define KEY_col0_Pin	GPIO_PIN_0
#define KEY_col1_Pin	GPIO_PIN_1
#define KEY_col2_Pin	GPIO_PIN_2
#define KEY_col3_Pin	GPIO_PIN_3

#define KEY_CLO0_OUT_LOW  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col0_Pin,GPIO_PIN_RESET) 
#define KEY_CLO1_OUT_LOW  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col1_Pin,GPIO_PIN_RESET)
#define KEY_CLO2_OUT_LOW  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col2_Pin,GPIO_PIN_RESET)
#define KEY_CLO3_OUT_LOW  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col3_Pin,GPIO_PIN_RESET)

#define KEY_CLO0_OUT_HIGH  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col0_Pin,GPIO_PIN_SET) 
#define KEY_CLO1_OUT_HIGH  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col1_Pin,GPIO_PIN_SET)
#define KEY_CLO2_OUT_HIGH  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col2_Pin,GPIO_PIN_SET)
#define KEY_CLO3_OUT_HIGH  HAL_GPIO_WritePin(KEY_PORT_COL,KEY_col3_Pin,GPIO_PIN_SET)

void KEYPAD_Init(void);
char KEYPAD_Scan(void);



#define KEY_UP 130
#define KEY_DOWN 131
#define KEY_OK 132
#define KEY_CAN 133
#define KEY_MOD 134
#define KEY_MENU 135

#endif

