#include <REG52.H>

// 简单的毫秒级延时函数 (基于12MHz晶振)
void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = ms; i > 0; i--)
        for (j = 110; j > 0; j--);
}

unsigned char MatrixKey()
{
	unsigned char KeyNumber=0;

	P1=0xF7;
	// ??:?????? P1.7 ??,?? P1_7(????)
	if((P1 & 0x80)==0){Delay_ms(20);while((P1 & 0x80)==0);Delay_ms(20);KeyNumber=1;}
	if((P1 & 0x40)==0){Delay_ms(20);while((P1 & 0x40)==0);Delay_ms(20);KeyNumber=5;}
	if((P1 & 0x20)==0){Delay_ms(20);while((P1 & 0x20)==0);Delay_ms(20);KeyNumber=9;}
	if((P1 & 0x10)==0){Delay_ms(20);while((P1 & 0x10)==0);Delay_ms(20);KeyNumber=13;}
	
	P1=0xFB;
	if((P1 & 0x80)==0){Delay_ms(20);while((P1 & 0x80)==0);Delay_ms(20);KeyNumber=2;}
	if((P1 & 0x40)==0){Delay_ms(20);while((P1 & 0x40)==0);Delay_ms(20);KeyNumber=6;}
	if((P1 & 0x20)==0){Delay_ms(20);while((P1 & 0x20)==0);Delay_ms(20);KeyNumber=10;}
	if((P1 & 0x10)==0){Delay_ms(20);while((P1 & 0x10)==0);Delay_ms(20);KeyNumber=14;}
	
	P1=0xFD;
	if((P1 & 0x80)==0){Delay_ms(20);while((P1 & 0x80)==0);Delay_ms(20);KeyNumber=3;}
	if((P1 & 0x40)==0){Delay_ms(20);while((P1 & 0x40)==0);Delay_ms(20);KeyNumber=7;}
	if((P1 & 0x20)==0){Delay_ms(20);while((P1 & 0x20)==0);Delay_ms(20);KeyNumber=11;}
	if((P1 & 0x10)==0){Delay_ms(20);while((P1 & 0x10)==0);Delay_ms(20);KeyNumber=15;}
	
	P1=0xFE;
	if((P1 & 0x80)==0){Delay_ms(20);while((P1 & 0x80)==0);Delay_ms(20);KeyNumber=4;}
	if((P1 & 0x40)==0){Delay_ms(20);while((P1 & 0x40)==0);Delay_ms(20);KeyNumber=8;}
	if((P1 & 0x20)==0){Delay_ms(20);while((P1 & 0x20)==0);Delay_ms(20);KeyNumber=12;}
	if((P1 & 0x10)==0){Delay_ms(20);while((P1 & 0x10)==0);Delay_ms(20);KeyNumber=16;}
	
	return KeyNumber;
}
