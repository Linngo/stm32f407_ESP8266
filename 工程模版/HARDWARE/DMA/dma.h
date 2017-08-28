#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
#include "usart.h"
#include "wifiusart.h"

#define send 1
#define rev 0
#define DMA_IRQTX 			1					//0,不接收;1,接收.
//#define DMA_IRQRX

#define SEND_BUF_SIZE 100	//100 byte
#define RECE_BUF_SIZE 1476  //  模块一次接收的数据长度4k

extern u8 SendBuff[SEND_BUF_SIZE];    //发送数据缓冲区 
extern u8 ReceiveBuff[RECE_BUF_SIZE];   //接收缓冲

void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr,u8 dir);//配置DMAx_CHx
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr);	//使能一次DMA传输		   
void DMA_IRQ_init(void);
#endif






























