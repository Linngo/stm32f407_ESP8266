#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
#include "usart.h"
#include "wifiusart.h"

#define send 1
#define rev 0
#define DMA_IRQTX 			1					//0,������;1,����.
//#define DMA_IRQRX

#define SEND_BUF_SIZE 100	//100 byte
#define RECE_BUF_SIZE 1476  //  ģ��һ�ν��յ����ݳ���4k

extern u8 SendBuff[SEND_BUF_SIZE];    //�������ݻ����� 
extern u8 ReceiveBuff[RECE_BUF_SIZE];   //���ջ���

void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr,u8 dir);//����DMAx_CHx
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr);	//ʹ��һ��DMA����		   
void DMA_IRQ_init(void);
#endif






























