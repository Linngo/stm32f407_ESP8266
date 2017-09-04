#ifndef __wifiusart_H
#define __wifiusart_H	 
#include "sys.h"  
	   

#define wifiUSART_MAX_RECV_LEN		2920//1476				//�����ջ����ֽ���  2k byte
#define wifiUSART_MAX_SEND_LEN		100					//����ͻ����ֽ���
#define wifiUSART_RX_EN 			1					//0,������;1,����.
#define DMA_RX 			1                               //ʹ��usarT DMA����

extern u8  wifiUSART_RX_BUF[wifiUSART_MAX_RECV_LEN]; 		//���ջ���,���USART_MAX_RECV_LEN�ֽ�
extern u8  wifiUSART_TX_BUF[wifiUSART_MAX_SEND_LEN]; 		//���ͻ���,���USART_MAX_SEND_LEN�ֽ�
extern u16 wifiUSART_RX_STA;   						//��������״̬

void wifiUSART_init(u32 bound);				//���ڳ�ʼ�� 
void u3_printf(char* fmt, ...);
#endif













