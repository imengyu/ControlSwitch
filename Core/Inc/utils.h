

#ifndef __UTILS_H
#define __UTILS_H

#include "stm32f1xx_hal.h"
#include "stdlib.h"
#include "stdio.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    
 
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  
 
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n) 
 
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  
 
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  
 
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  
 
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  

unsigned char DecToBCD(unsigned char mData);
//生成不重复数字
uint16_t genNoDuplicateInteger(void);
//取0a,x]的随机整数
uint16_t genRandomInteger(uint16_t x);
//取得(a,b]的随机整数
uint16_t genRandomIntegerRange(uint16_t a, uint16_t b);

#define USER_DEBUG

#ifdef USER_DEBUG
#define Debug_Printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define Debug_Info(tag, format, ...) printf("[" tag "] Info:" format "\r\n",##__VA_ARGS__)
#define Debug_Debug(tag, format, ...) printf("[" tag "] Debug:" format "\r\n",##__VA_ARGS__)
#define Debug_Error(tag, format, ...) printf("[" tag "] Error:" format "\r\n",##__VA_ARGS__)
#define Debug_Warning(tag, format, ...) printf("[" tag "] Waining:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(tag, format, ...)
#define Debug_Info(tag, format, ...)
#define Debug_Debug(tag, format, ...)
#define Debug_Error(tag, format, ...)
#define Debug_Warning(tag, format, ...)
#endif

#endif

