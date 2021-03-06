/**
  ******************************************************************************
  * @file           : loop.h
  * @brief          : header for loop.c file.
  *                   程序主循环。放在这里不用让MX每次更新的时候把代码写坏
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __LOOP_H
#define __LOOP_H



/**
 * 程序初始化
 */
void MAIN_Init(void);
/**
 * 程序循环
 */
void MAIN_Loop(void);


void MAIN_UpdateSwSettings(uint8_t index);
void MAIN_SwitchLowMode(uint8_t mode);



#endif

