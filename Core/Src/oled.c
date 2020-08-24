/**
  ******************************************************************************
  * @file           : oled.c
  * @brief          : 0.96寸 oled 控制函数
  ******************************************************************************
  * OLED 使用 I2C 
  * 连接在 I2C1 上
  ******************************************************************************
  */

#include "oled.h"
#include "oled_font.h"
#include "delay.h"

extern I2C_HandleTypeDef hi2c1;

/* OLED初始化函数 */

/**
 * OLED初始化
 */
void OLED_Init(void)
{
  Delay_MS(100);

  OLED_WR_Cmd(0xAE); //display off
  OLED_WR_Cmd(0x20); //Set Memory Addressing Mode
  OLED_WR_Cmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  OLED_WR_Cmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
  OLED_WR_Cmd(0xc8); //Set COM Output Scan Direction
  OLED_WR_Cmd(0x00); //---set low column address
  OLED_WR_Cmd(0x10); //---set high column address
  OLED_WR_Cmd(0x40); //--set start line address
  OLED_WR_Cmd(0x81); //--set contrast control register
  OLED_WR_Cmd(0xff); //亮度调节 0x00~0xff
  OLED_WR_Cmd(0xa1); //--set segment re-map 0 to 127
  OLED_WR_Cmd(0xa6); //--set normal display
  OLED_WR_Cmd(0xa8); //--set multiplex ratio(1 to 64)
  OLED_WR_Cmd(0x3F); //
  OLED_WR_Cmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  OLED_WR_Cmd(0xd3); //-set display offset
  OLED_WR_Cmd(0x00); //-not offset
  OLED_WR_Cmd(0xd5); //--set display clock divide ratio/oscillator frequency
  OLED_WR_Cmd(0xf0); //--set divide ratio
  OLED_WR_Cmd(0xd9); //--set pre-charge period
  OLED_WR_Cmd(0x22); //
  OLED_WR_Cmd(0xda); //--set com pins hardware configuration
  OLED_WR_Cmd(0x12);
  OLED_WR_Cmd(0xdb); //--set vcomh
  OLED_WR_Cmd(0x20); //0x20,0.77xVcc
  OLED_WR_Cmd(0x8d); //--set DC-DC enable
  OLED_WR_Cmd(0x14); //
#if DISPLAY_MODE == 1
  OLED_WR_Cmd(0xA7); //设置显示方式;bit0:1,反相显示;0,正常显示
#else
  OLED_WR_Cmd(0xA6); //设置显示方式;bit0:1,反相显示;0,正常显示
#endif
  OLED_WR_Cmd(0xAF); //--turn on oled panel

  OLED_Clear();
  OLED_Set_Pos(0, 0);
}

/**
 * 向OLED(I2C2)写入一个字节数据
 * @param data 数据
 */
void OLED_WR_Data(uint8_t data)
{
  HAL_I2C_Mem_Write(&hi2c1, OLED0561_ADD, OLED_DATA, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}
/**
 * 向OLED(I2C2)写入一个命令
 * @param data 命令
 */
void OLED_WR_Cmd(uint8_t cmd)
{
  HAL_I2C_Mem_Write(&hi2c1, OLED0561_ADD, OLED_CMD, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 100);
}

/* 功能函数 */

/**
 * OLED清屏
 */
void OLED_Clear(void)
{
  OLED_Fill(0);
}
/**
 * OLED全屏填充
 * @param fill_Data 填充数据
 */
void OLED_Fill(unsigned char fill_Data)
{
  unsigned char m, n;
  for (m = 0; m < 8; m++)
  {
    OLED_WR_Cmd(0xb0 + m); //page0-page1
    OLED_WR_Cmd(0x00);     //low column start address
    OLED_WR_Cmd(0x10);     //high column start address
    for (n = 0; n < 128; n++)
    {
      OLED_WR_Data(fill_Data);
    }
  }
}
/**
 * 开启OLED显示
 */
void OLED_Display_On(void)
{
  OLED_WR_Cmd(0x8d);
  OLED_WR_Cmd(0x14);
  OLED_WR_Cmd(0xaf);
}
/**
 * 关闭OLED显示
 */
void OLED_Display_Off(void)
{
  OLED_WR_Cmd(0x8d);
  OLED_WR_Cmd(0x10);
  OLED_WR_Cmd(0xaf);
}

/**
 * 在指定地方显示一个字符，包括部分字符
 * @param x 0至127
 * @param y 0至63
 * @param chr 字符
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowChar(uint8_t x, uint8_t y, char chr, uint8_t size)
{
  uint8_t c = 0, i = 0;
  c = chr - ' ';
  if (x > 127)
  {
    x = 0;
    y = y + 2;
  }
  if (size == 16)
  {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++)
    {
      OLED_WR_Data(ASCII_F8X16[c * 16 + i]);
    }
    OLED_Set_Pos(x, y + 1);
    for (i = 0; i < 8; i++)
    {
      OLED_WR_Data(ASCII_F8X16[c * 16 + i + 8]);
    }
  }
  else
  {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 6; i++)
    {
      OLED_WR_Data(ASCII_F6X8[c][i]);
    }
  }
}
/**
 * 在指定地方显示一个中文字符，中文字符必须在oled_font.h中实现声明
 * @param x 0至127
 * @param y 0至63
 * @param k 中文字符在GB162_F16的索引
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowChineseChar(uint8_t x, uint8_t y, uint8_t k)
{
  uint8_t t = 0;
  if (x > 127)
  {
    x = 0;
    y = y + 2;
  }
  OLED_Set_Pos(x, y);
  for (t = 0; t < 16; t++)
  {
    OLED_WR_Data(GB162_F16[k].Msk[t]);
  }
  OLED_Set_Pos(x, y + 1);
  for (t = 0; t < 16; t++)
  {
    OLED_WR_Data(GB162_F16[k].Msk[t + 16]);
  }
}

uint32_t oled_pow(uint8_t m, uint8_t n)
{
  //m^n函数
  uint32_t result = 1;
  while (n--)
    result *= m;
  return result;
}

/**
 * 在指定地方显示数字
 * @param x 0至127
 * @param y 0至63
 * @param num 数字的位数，负号算一位
 * @param len 字符
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowNum(uint8_t x, uint8_t y, long num, uint8_t len, uint8_t size)
{
  uint32_t t, temp1, temp2, temp3, flag;
  if (num < 0)
  {
    flag = 1;   //负数标志位置1
    num = -num; //数字变正数
  }
  for (t = 0; t < len; t++)
  {
    temp1 = num / oled_pow(10, len - t - 1);
    temp2 = temp1 % 10;
    if (flag == 1)
    {
      temp3 = num / oled_pow(10, len - t - 2);
    }
    if (temp1 != 0)
    {
      OLED_ShowChar(x, y, '0' + temp2, size);
    }
    else
    {
      if (t == len - 1 && temp2 == 0)
      {
        OLED_ShowChar(x, y, '0', size);
      }
      else
      {
        if (flag == 1 && temp3 != 0) //该数是负数
        {
          OLED_ShowChar(x, y, '-', size); //输出负号
          flag = 0;                       //负号已输出，标志位置0
        }
        else
        {
          OLED_ShowChar(x, y, ' ', size);
        }
      }
    }
    x += 8;
  }
}
/**
 * 显示一个汉字串和字符号串(输入\r可以换行，汉字须在oled_font.c中事先录入点阵)
 * @param x 0至127
 * @param y 0至63
 * @param chr 字符串
 * @param size 选择字体大小，8或16（8x6或16x16）
 */
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t size)
{
  uint8_t x0 = x, drawEd = 0;
  uint16_t k;
  while (*chr != '\0')
  {
    if ((*chr) < 128)
    {
      if (*chr == 13)
      {
        x = x0;
        y += size / 8;
      }
      else
      {
        OLED_ShowChar(x, y, *chr, size);
        x += 8;
        if (x > 120)
        {
          x = 0;
          y += size / 8;
        }
      }
      chr++;
    }
    else if (size == 16) //字体是否为16
    {
      drawEd = 0;
      for (k = 0; k < GB162_F16_COUNT; k++) //汉字搜索
      {
        if ((GB162_F16[k].Index[0] == *(chr)) && (GB162_F16[k].Index[1] == *(chr + 1)))
        {
          OLED_ShowChineseChar(x, y, k);
          drawEd = 1;
        }
      }
      if(!drawEd) 
        OLED_ShowChineseChar(x, y, 0);
      if (x > 112)
      {
        x = 0;
        y += 2;
      }
      chr += 2;
      x += 16;
    }
    else //输入错误，忽略汉字
    {
      chr += 2;
    }
  }
}
/**
 * 显示显示BMP图片128×64
 * @param BMP 图片点阵
 */
void OLED_Draw12864BMP(uint8_t BMP[])
{
  //
  uint16_t j = 0;
  uint8_t x, y;
  for (y = 0; y < 8; y++)
  {
    OLED_Set_Pos(0, y);
    for (x = 0; x < 128; x++)
    {
#if DISPLAY_MODE
      OLED_WR_Data(~BMP[j++]);
#else
      OLED_WR_Data(BMP[j++]);
#endif
    }
  }
}
/**
 * 显示显示BMP图片128×64
 * @param x0 起始点坐标x （0～127）
 * @param y0 起始点坐标y （0～7）
 * @param x1 图片点阵宽度
 * @param y1 图片点阵高度
 * @param BMP 图片点阵
 */
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[])
{
  uint16_t j = 0;
  uint8_t x, y;
  for (y = y0; y < y1; y++)
  {
    OLED_Set_Pos(x0, y);
    for (x = x0; x < x1; x++)
      OLED_WR_Data(BMP[j++]);
  }
}

/**
 * OLED设置光标坐标
 * @param x x位置
 * @param y y位置
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
  OLED_WR_Cmd(0xb0 + y);
  OLED_WR_Cmd(((x & 0xf0) >> 4) | 0x10);
  OLED_WR_Cmd((x & 0x0f) | 0x01);
}
