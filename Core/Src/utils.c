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


uint16_t genBase = 0;
uint16_t genNoDuplicateInteger(void) {
  if(genBase > 0xffff - 1) genBase = 0;
  return genBase++;
}

//取0a,x]的随机整数
uint16_t genRandomInteger(uint16_t x) {
  return rand() %(x + 1); 
}
//取得(a,b]的随机整数
uint16_t genRandomIntegerRange(uint16_t a, uint16_t b) {
  return rand() %(b-a) + a; 
}


