/**
  ******************************************************************************
  * @file           : oled_font.h
  * @brief          : LED字符点阵
  ******************************************************************************
  */

#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#define GB162_F16_COUNT 9  //汉字个数

//汉字用的结构体
typedef struct FONT_GB162
{
  unsigned char Msk[32];      
  unsigned char Index[2];    
  unsigned char num;  
}FONT_GB162;

extern const unsigned char ASCII_F6X8[][6];
extern const unsigned char ASCII_F8X16[];
extern const struct FONT_GB162 GB162_F16[];
extern const unsigned char  INTRO_BMP[];

#endif
