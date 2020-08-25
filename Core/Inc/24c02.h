/**
  ******************************************************************************
  * @file           : 24c02.h
  * @brief          : header for 24c02.c file.
  *                   AT24Cxx 读写模块.
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __24C02_H
#define __24C02_H

#include "stm32f1xx_hal.h"

#define ADDR_AT24C02_Write 0xA0
#define ADDR_AT24C02_Read 0xA1


/**************************************************************
--24C02存放数据定义
**************************************************************/

#define ADDR_WIFI_NAME 0x0
#define ADDR_WIFI_PASS 0x20

/**************************************************************
--24C02驱动函数
**************************************************************/

/**
 * @brief 从24c02读取 1Byte 数据
 * @param ReadAddr 读取地址 0-0xff
 * @retval 数据
 */
uint8_t AT24C02_ReadOneByte(uint16_t ReadAddr);
/**
 * @brief 从24c02读取数据
 * @param ReadAddr 读取地址 0-0xff
 * @param readSize 读取大小 
 * @param readBuffer 读取缓冲区 
 * @retval None
 */
void AT24C02_Read(uint16_t ReadAddr, uint8_t readSize, uint8_t* readBuffer);
/**
 * @brief 写入1Byte数据至24c02
 * @param WriteAddr 地址 0-0xff
 * @retval None
 */
void AT24C02_WriteOneByte(uint16_t WriteAddr, uint8_t DataToWrite);
/**
 * @brief 写入一页数据至24c02
 * @param PageAddr 页 0-32
 * @param WriteAddr 地址 0-8
 * @param DataToWrite 数据，8B大小
 * @retval None
 */
void AT24C02_WriteOnePage(uint16_t PageAddr, uint16_t WriteAddr, uint8_t *DataToWrite);

/**
 * @brief 擦除24c02所有数据（为0）
 * @retval None
 */
void AT24C02_FlushAll(void);

#endif
