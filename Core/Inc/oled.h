/**
  ******************************************************************************
  * @file           : oled.h
  * @brief          : header for oled.c file.
  *                   0.96寸 oled 控制函数
  ******************************************************************************
  *
  * 
  ******************************************************************************
  */

#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"

#define OLED_CMD 0x00
#define OLED_DATA 0x40
#define OLED0561_ADD	0x78  // OLED的I2C地址（禁止修改）

#define DISPLAY_MODE 0 //设置显示方式, 1,反相显示; 0,正常显示

#include "oled.h"

/* OLED初始化函数 */

/**
 * OLED初始化
 */
void OLED_Init(void);

/* 功能函数 */

/**
 * OLED清屏
 */
void OLED_Clear(void);     
/**
 * 开启OLED显示
 */
void OLED_Display_On(void);
/**
 * 关闭OLED显示
 */
void OLED_Display_Off(void);
/**
 * OLED全屏填充
 * @param fill_Data 填充数据
 */
void OLED_Fill(unsigned char fill_Data);

/**
 * 在指定地方显示一个中文字符，中文字符必须在oled_font.h中实现声明
 * @param x 0至127
 * @param y 0至63
 * @param k 中文字符在GB162_F16的索引
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowChineseChar(uint8_t x, uint8_t y, uint8_t k);
/**
 * 在指定地方显示一个字符，包括部分字符
 * @param x 0至127
 * @param y 0至63
 * @param chr 字符
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size);
/**
 * 在指定地方显示数字
 * @param x 0至127
 * @param y 0至63
 * @param num 数字的位数，负号算一位
 * @param len 字符
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowNum(uint8_t x, uint8_t y, long num, uint8_t len, uint8_t size);
/**
 * @brief 显示一个汉字串和字符号串 (汉字须在oled_font.c中事先录入点阵)
 * @param x 0至127
 * @param y 0至63
 * @param chr 字符串
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t size);
/**
 * 显示显示BMP图片128×64
 * @param BMP 图片点阵
 */
void OLED_Draw12864BMP(uint8_t BMP[]);
/**
 * 显示显示BMP图片128×64
 * @param x0 起始点坐标x （0～127）
 * @param y0 起始点坐标y （0～7）
 * @param x1 图片点阵宽度
 * @param y1 图片点阵高度
 * @param BMP 图片点阵
 */
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]);

/***************************************************/

/* 其他函数，不需要调用 */
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_WR_Data(uint8_t data);
void OLED_WR_Cmd(uint8_t cmd);


#endif

