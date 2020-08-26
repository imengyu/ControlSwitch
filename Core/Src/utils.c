#include "utils.h"
#include "stdio.h"

unsigned char DecToBCD(unsigned char mData)
{
  unsigned char BCD = 0;

  while (mData >= 10)
  {
    mData -= 10;
    BCD++;
  }
  BCD <<= 4;
  BCD |= mData;
  return BCD;
}

