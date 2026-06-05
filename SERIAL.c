#include "SERIAL.h"

// 接收缓冲区（仅本文件可见）
static unsigned char uart_rx_buf[UART_BUF_SIZE];
static unsigned char uart_rx_head = 0;
static unsigned char uart_rx_tail = 0;

/************************ 核心初始化 ************************/
void SERIAL_Init(void)
{
    TMOD |= 0x20;  // 定时器1模式2（8位自动重装）
    
    // 11.0592MHz波特率精准值
    switch(BAUD_RATE)
    {
        case 9600:  TH1 = 0xFD; break;
        case 19200: TH1 = 0xFE; break;
        case 38400: TH1 = 0xFF; break;
        default:    TH1 = 0xFD;
    }
    
    TL1 = TH1;
    TR1 = 1;      // 启动定时器1
    SCON = 0x50;  // 8位数据，允许接收
    ES = 1;       // 开串口中断
    EA = 1;       // 开总中断
}

/************************ 发送函数 ************************/
// 发送单个原始字节（HEX模式核心）
void SERIAL_SendByte(unsigned char dat)
{
    SBUF = dat;
    while(!TI);
    TI = 0;
}

// 发送原始HEX字节数组（电脑选HEX接收）
void SERIAL_SendHexArrayRaw(unsigned char *buf, unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        SERIAL_SendByte(buf[i]);
    }
}

// 发送单个字节的HEX格式字符串（显示0xXX）
void SERIAL_SendHexByte(unsigned char dat)
{
    unsigned char high = dat >> 4;
    unsigned char low = dat & 0x0F;
    
    // 转换为ASCII码（大写）
    high = (high < 10) ? (high + '0') : (high - 10 + 'A');
    low = (low < 10) ? (low + '0') : (low - 10 + 'A');
    
    SERIAL_SendByte('0');
    SERIAL_SendByte('x');
    SERIAL_SendByte(high);
    SERIAL_SendByte(low);
}

// 发送HEX数组格式字符串（带空格分隔）
void SERIAL_SendHexArray(unsigned char *buf, unsigned char len)
{
    unsigned char i;
    for(i=0; i<len; i++)
    {
        SERIAL_SendHexByte(buf[i]);
        if(i != len-1)
        {
            SERIAL_SendByte(' ');  // 字节间加空格
        }
    }
}

// 发送字符串（文本模式）
void SERIAL_SendString(unsigned char *str)
{
    while(*str)
    {
        SERIAL_SendByte(*str++);
    }
}

// 发送字符串+换行
void SERIAL_SendLine(unsigned char *str)
{
    SERIAL_SendString(str);
    SERIAL_SendByte('\r');
    SERIAL_SendByte('\n');
}

/************************ 接收函数（中断式） ************************/
// 串口中断服务函数（自动接收原始HEX字节）
void SERIAL_ISR(void) interrupt 4
{
    if(RI)
    {
        RI = 0;
        // 写入环形缓冲区（满了自动丢弃）
        if(((uart_rx_head + 1) % UART_BUF_SIZE) != uart_rx_tail)
        {
            uart_rx_buf[uart_rx_head] = SBUF;
            uart_rx_head = (uart_rx_head + 1) % UART_BUF_SIZE;
        }
    }
}

// 检查是否有数据
bit SERIAL_Available(void)
{
    return (uart_rx_head != uart_rx_tail);
}

// 读取单个原始HEX字节
unsigned char SERIAL_ReadByte(void)
{
    unsigned char dat = 0xFF;
    if(SERIAL_Available())
    {
        dat = uart_rx_buf[uart_rx_tail];
        uart_rx_tail = (uart_rx_tail + 1) % UART_BUF_SIZE;
    }
    return dat;
}

// 清空缓冲区
void SERIAL_ClearBuffer(void)
{
    uart_rx_head = 0;
    uart_rx_tail = 0;
}