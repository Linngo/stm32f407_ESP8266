#ifndef __wifiusart_H
#define __wifiusart_H	 
#include "sys.h"  
	   

#define wifiUSART_MAX_RECV_LEN		2920//1476				//最大接收缓存字节数  2k byte
#define wifiUSART_MAX_SEND_LEN		100					//最大发送缓存字节数
#define wifiUSART_RX_EN 			1					//0,不接收;1,接收.
#define DMA_RX 			1                               //使能usarT DMA接收

extern u8  wifiUSART_RX_BUF[wifiUSART_MAX_RECV_LEN]; 		//接收缓冲,最大USART_MAX_RECV_LEN字节
extern u8  wifiUSART_TX_BUF[wifiUSART_MAX_SEND_LEN]; 		//发送缓冲,最大USART_MAX_SEND_LEN字节
extern u16 wifiUSART_RX_STA;   						//接收数据状态

void wifiUSART_init(u32 bound);				//串口初始化 
void u3_printf(char* fmt, ...);
#endif













