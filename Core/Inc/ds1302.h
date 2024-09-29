#ifndef DS1302_H
#define DS1302_H

#include "stm32f1xx_hal.h"
#include "utils.h"

#define DS1302_PORT GPIOB

#define DS1302_CS GPIO_PIN_3
#define DS1302_IO GPIO_PIN_4
#define DS1302_CLK GPIO_PIN_5

#define DS1302_CS_1 HAL_GPIO_WritePin(DS1302_PORT, DS1302_CS, GPIO_PIN_SET)
#define DS1302_CLK_1 HAL_GPIO_WritePin(DS1302_PORT, DS1302_CLK, GPIO_PIN_SET)

#define DS1302_CS_0 HAL_GPIO_WritePin(DS1302_PORT, DS1302_CS, GPIO_PIN_RESET)
#define DS1302_CLK_0 HAL_GPIO_WritePin(DS1302_PORT, DS1302_CLK, GPIO_PIN_RESET)

#define DS1302_DAT_Input() {DS1302_PORT->CRL &= 0XFFFFFF0F;DS1302_PORT->CRL |= (uint32_t)8<<4;} //上拉输入模式
#define DS1302_DAT_Output() {DS1302_PORT->CRL &= 0XFFFFFF0F;DS1302_PORT->CRL |= (uint32_t)3<<4;} //输出，速度是50Mhz

#define DS1302_DAT(x) HAL_GPIO_WritePin(DS1302_PORT, DS1302_IO, x)
#define DS1302_DAT_READ() HAL_GPIO_ReadPin(DS1302_PORT, DS1302_IO)

//DS1302地址定义
#define ds1302_sec_add	0x80	//秒数据地址
#define ds1302_min_add	0x82	//分数据地址
#define ds1302_hr_add	0x84	//时数据地址
#define ds1302_date_add	0x86	//日数据地址
#define ds1302_month_add	0x88	//月数据地址
#define ds1302_day_add	0x8a	//星期数据地址
#define ds1302_year_add	0x8c	//年数据地址
#define ds1302_control_add	0x8e	//控制数据地址
#define ds1302_charger_add	0x90 
#define ds1302_clkburst_add	0xbe

typedef struct
{
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t weekday;
}Timedata;

void DS1302_IOit(void);
void DS1302_Write_Byte(uint8_t temp);
void DS1302_Write(uint8_t addr,uint8_t data);
uint8_t DS1302_Read_Byte(void);
uint8_t DS1302_Read(uint8_t addr);

void DS1302_Brust_Readtime(void);
void DS1302_Burst_Writetime(void);
void DS1302_Brust_Read(uint8_t *pdat);
void DS1302_Burst_Write(uint8_t *pdata);

#endif
