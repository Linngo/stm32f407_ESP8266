#include "delay.h"
#include "wifiUSART.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	  
#include "timer.h"
#include "dma.h"

//���ڷ��ͻ����� 	
__align(8) u8 wifiUSART_TX_BUF[wifiUSART_MAX_SEND_LEN]; 	//���ͻ���,���wifiUSART_MAX_SEND_LEN�ֽ�
#ifdef wifiUSART_RX_EN   								//���ʹ���˽���   	  
//���ڽ��ջ����� 	
u8 wifiUSART_RX_BUF[wifiUSART_MAX_RECV_LEN]; 				//���ջ���,���wifiUSART_MAX_RECV_LEN���ֽ�.

//ͨ���жϽ�������2���ַ�֮���ʱ������100ms�������ǲ���һ������������.
//���2���ַ����ռ������100ms,����Ϊ����1����������.Ҳ���ǳ���100msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
u16 wifiUSART_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	u8 clear=clear;
#ifdef DMA_RX
#else
	u8 res;	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART2);		
		if((wifiUSART_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(wifiUSART_RX_STA<wifiUSART_MAX_RECV_LEN)		//�����Խ�������
			{
				wifiUSART_RX_BUF[wifiUSART_RX_STA++]=res;		//��¼���յ���ֵ
			}
			else
			{
				wifiUSART_RX_STA|=1<<15;					//ǿ�Ʊ�ǽ������
			}
		}
	}
#endif	
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //������һ֡����
	{
		clear = USART2->SR;
		clear = USART2->DR;
#ifdef DMA_RX		
		DMA_Cmd(DMA1_Stream5,DISABLE);
		wifiUSART_RX_STA = RECE_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream5); 
		if((wifiUSART_RX_STA&(1<<15))==0)         //������������ݵ�wifiUSART_RX_BUF
		{	
			if(wifiUSART_RX_STA>(1460+16)) wifiUSART_RX_STA=1476;
			memcpy(wifiUSART_RX_BUF,ReceiveBuff,wifiUSART_RX_STA);	
		}
		DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);//���DMA1_Steam5������ɱ�־  
		DMA_SetCurrDataCounter(DMA1_Stream5, RECE_BUF_SIZE);  
		DMA_Cmd(DMA1_Stream5,ENABLE);     //��DMA,
#endif		
		wifiUSART_RX_STA|=1<<15;
	}										 
}  
#endif	
//��ʼ��IO ����
//bound:������	  
void wifiUSART_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //ʹ��GPIODʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USARTʱ��

	USART_DeInit(USART2);  //��λ����

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); //GPIOB11����ΪUSART
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); //GPIOB10����ΪUSART	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; //GPIOB11��GPIOB10��ʼ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOD,&GPIO_InitStructure); //��ʼ��GPIOB11����GPIOB10

	USART_InitStructure.USART_BaudRate = bound;//������ 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART2, &USART_InitStructure); //��ʼ������3

	USART_Cmd(USART2, ENABLE);               //ʹ�ܴ��� 

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//���������ж�
#ifdef DMA_RX
#else	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���������ж�   
#endif

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	wifiUSART_RX_STA=0;		//����
}

//����,printf ����
//ȷ��һ�η������ݲ�����wifiUSART_MAX_SEND_LEN�ֽ�
void u3_printf(char* fmt,...)  
{  
	u16 i;
//	u16 j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)wifiUSART_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)wifiUSART_TX_BUF);//�˴η������ݵĳ���
#ifdef DMA_RX
	memcpy(SendBuff,wifiUSART_TX_BUF,i);
	MYDMA_Enable(DMA1_Stream6,i);
#else	
	for(j=0;j<i;j++)//ѭ����������
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);  //�ȴ��ϴδ������ 
		USART_SendData(USART2,(uint8_t)wifiUSART_TX_BUF[j]); 	 //�������ݵ�����3 
	}
#endif	
}
