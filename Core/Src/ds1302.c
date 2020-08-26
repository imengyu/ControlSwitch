#include "ds1302.h"
#include "delay.h"

Timedata ReadTime = {0};

/*brust模式，秒、分、时、日、月、星期、年
每次写入一个字节，因此写入的数据要与寄存器的的位置相对应，读数据的时候也是对应的

在非brust模式下，在写入数据的时候，首先要写入相应寄存器的地址，之后在相应的地址上写入数据，
在读取数据的时候，在相应的地址上就可以读到相应的数据
*/
uint8_t time_buf[] = {0x00, 0x25, 0x14, 0x07, 0x07, 0x02, 0x20};  //brust模式
uint8_t val;

void DS1302_Init(void)
{
  DS1302_CS_0;
  DS1302_CLK_0;
}

void DS1302_Write_Byte(uint8_t temp)
{
  uint8_t i = 0;
  DS1302_DAT_Output(); //输出
  DS1302_CLK_0;
  Delay_US(2);
  for (i = 0; i < 8; i++)
  {
    if (temp & 0x01) //低位在前
    {
      DS1302_DAT_OUT = 1;
      Delay_US(2);
    }
    else
    {
      DS1302_DAT_OUT = 0;
      Delay_US(2);
    }

    DS1302_CLK_1; //产生上升沿时钟
    Delay_US(2);
    DS1302_CLK_0; //将时钟拉低，为下次做准备
    Delay_US(2);

    temp = temp >> 1;
  }

  DS1302_DAT_OUT = 1; //释放端口
  Delay_US(2);
}

void DS1302_Write(uint8_t addr, uint8_t data)
{
  DS1302_CS_0;
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_1;
  Delay_US(2);
  addr = addr & 0xfe; //低位为0时是写
  DS1302_Write_Byte(addr);
  Delay_US(2);
  DS1302_Write_Byte(data);
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_0;
  Delay_US(2);
}

uint8_t DS1302_Read_Byte(void)
{
  uint8_t i = 0;
  uint8_t temp;
  DS1302_DAT_Input(); //输入
  for (i = 0; i < 8; i++)
  {
    DS1302_CLK_1;
    Delay_US(2);
    temp >>= 1;

    if (DS1302_DAT_IN)
      temp = temp | 0x80;
    else
      temp = temp & 0x7f;

    DS1302_CLK_0;
    Delay_US(2);
  }

  return temp;
}

uint8_t DS1302_Read(uint8_t addr)
{
  DS1302_CS_0;
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_1;
  Delay_US(2);

  addr = addr | 0x01;

  DS1302_Write_Byte(addr);
  Delay_US(2);
  val = DS1302_Read_Byte();
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_0;
  Delay_US(2);
  DS1302_DAT_OUT = 0; //释放端口
  Delay_US(2);
  DS1302_DAT_OUT = 1; //释放端口
  Delay_US(2);

  return (val / 16 * 10) + (val % 16); //十六进制转换成BCD码
}

/*主要是防止读取的时候，还没读完，数据又变化了，故先将寄存器的数据，
输出到缓冲区内，读取数据时候是去缓冲区进行读取，写也是如此。
指令：
读0xBF
写0xBE
*/
void DS1302_Burst_Write(uint8_t *pdata)
{
  uint8_t i = 0;
  DS1302_CS_0;
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_1;
  Delay_US(2);
  DS1302_Write_Byte(0xbe);
  for (i = 0; i < 8; i++)
  {
    DS1302_Write_Byte(pdata[i]);
  }
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_0;
  Delay_US(2);
}

void DS1302_Brust_Read(uint8_t *pdat)
{
  uint8_t i = 0;
  DS1302_CS_0;
  Delay_US(2);
  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_1;
  Delay_US(2);

  DS1302_Write_Byte(0xbf);

  for (i = 0; i < 8; i++)
  {
    pdat[i] = DS1302_Read_Byte();
  }

  DS1302_CLK_0;
  Delay_US(2);
  DS1302_CS_0;
  Delay_US(2);
}

void DS1302_Burst_Writetime(void)
{
  time_buf[6] = DecToBCD(ReadTime.year);
  time_buf[4] = DecToBCD(ReadTime.month);
  time_buf[3] = DecToBCD(ReadTime.day);
  time_buf[2] = DecToBCD(ReadTime.hour);
  time_buf[1] = DecToBCD(ReadTime.minute);
  time_buf[0] = DecToBCD(ReadTime.second);
  time_buf[5] = DecToBCD(ReadTime.weekday);

  DS1302_Write(ds1302_control_add, 0x80); //关闭写保护
  DS1302_Write(ds1302_sec_add, 0x80);     //暂停时钟
  DS1302_Burst_Write(time_buf);
  DS1302_Write(ds1302_control_add, 0x80); //打开写保护
}

void DS1302_Brust_Readtime(void)
{
  uint8_t pbuf[7];
  DS1302_Brust_Read(pbuf);
  ReadTime.year = ((pbuf[6] / 16 * 10) + (pbuf[6] % 16)); //转换为BCD码
  ReadTime.month = ((pbuf[4] / 16 * 10) + (pbuf[4] % 16));
  ReadTime.day = ((pbuf[3] / 16 * 10) + (pbuf[3] % 16));
  ReadTime.hour = ((pbuf[2] / 16 * 10) + (pbuf[2] % 16));
  ReadTime.minute = ((pbuf[1] / 16 * 10) + (pbuf[1] % 16));
  ReadTime.second = ((pbuf[0] / 16 * 10) + (pbuf[0] % 16));
  ReadTime.weekday = ((pbuf[5] / 16 * 10) + (pbuf[5] % 16));
}

