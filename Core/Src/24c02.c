/**
  ******************************************************************************
  * @file           : 24c02.c
  * @brief          : AT24C02 读写模块.
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#include "24c02.h"
#include "string.h"

extern I2C_HandleTypeDef hi2c1;

void AT24CXX_Init(void) {

}
/**
 * @brief 从24c02读取 1Byte 数据
 * @param ReadAddr 读取地址 0-0xff
 * @retval 数据
 */
uint8_t AT24C02_ReadOneByte(uint16_t ReadAddr) {
  uint8_t buffer;
  HAL_I2C_Mem_Read(&hi2c1, ADDR_AT24C02_Read, ReadAddr, I2C_MEMADD_SIZE_8BIT, &buffer, 1, 1000);
  return buffer;
}
/**
 * @brief 从24c02读取数据
 * @param ReadAddr 读取地址 0-0xff
 * @param readSize 读取大小 
 * @param readBuffer 读取缓冲区 
 * @retval None
 */
void AT24C02_Read(uint16_t ReadAddr, uint8_t readSize, uint8_t* readBuffer) {
  HAL_I2C_Mem_Read(&hi2c1, ADDR_AT24C02_Read, ReadAddr, I2C_MEMADD_SIZE_8BIT, readBuffer, readSize, 1000);
}
/**
 * @brief 写入1Byte数据至24c02
 * @param WriteAddr 地址 0-0xff
 * @retval None
 */
void AT24C02_WriteOneByte(uint16_t WriteAddr, uint8_t DataToWrite) {
  HAL_I2C_Mem_Write(&hi2c1, ADDR_AT24C02_Write, WriteAddr, I2C_MEMADD_SIZE_8BIT, &DataToWrite, 1, 1000);
  HAL_Delay(5);
}
/**
 * @brief 写入一页数据至24c02
 * @param PageAddr 页 0-32
 * @param WriteAddr 地址 0-8
 * @param DataToWrite 数据，8B大小
 * @retval None
 */
void AT24C02_WriteOnePage(uint16_t PageAddr, uint16_t WriteAddr, uint8_t *DataToWrite) {
  HAL_I2C_Mem_Write(&hi2c1, ADDR_AT24C02_Write, PageAddr * 8, I2C_MEMADD_SIZE_8BIT, DataToWrite, 8, 1000);
  HAL_Delay(5);
}
/**
 * @brief 擦除24c02所有数据（为0）
 * @retval None
 */
void AT24C02_FlushAll() {
  uint8_t n;
  uint8_t buf[8];

  memset(buf, 0, sizeof(buf));

  for(n = 0; n < 8; n++)
    AT24C02_WriteOnePage(n, 0, buf);
}

