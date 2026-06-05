#ifndef __OLED_H
#define __OLED_H

#include <reg52.h>

// 引脚定义
sbit OLED_SCL = P2^0; // 时钟引脚
sbit OLED_SDA = P2^1; // 数据引脚

// 函数声明
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr);
void OLED_ShowString(unsigned char x, unsigned char y, const unsigned char code *chr);


#endif
