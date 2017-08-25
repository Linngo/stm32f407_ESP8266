#include "dma.h"																	   	  
#include "delay.h"
#include <string.h>

u8 SendBuff[SEND_BUF_SIZE];    //�������ݻ����� 
u8 ReceiveBuff[RECE_BUF_SIZE];   //���ջ���

//DMAx�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//8λ���ݿ��/�洢������ģʽ
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMAͨ��ѡ��,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:�����ַ
//mar:�洢����ַ
//ndtr:���ݴ�����
//dir:���ݷ���  1:����0:����
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr,u8 dir)
{ 
 
	DMA_InitTypeDef  DMA_InitStructure;

	if((u32)DMA_Streamx>(u32)DMA2)//�õ���ǰstream������DMA2����DMA1
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2ʱ��ʹ�� 	
	}else 
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1ʱ��ʹ�� 
	}
	DMA_DeInit(DMA_Streamx);

	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//�ȴ�DMA������ 

	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel = chx;  //ͨ��ѡ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;//DMA�����ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;//DMA �洢��0��ַ
	if(dir==1)
	{
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//�洢��������
	}else{
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//���赽�洢��
	}
	DMA_InitStructure.DMA_BufferSize = ndtr;//���ݴ����� 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�洢�����ݳ���:8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//�е����ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
	DMA_Init(DMA_Streamx, &DMA_InitStructure);//��ʼ��DMA Stream
	if(dir==rev)
		DMA_Cmd(DMA_Streamx,ENABLE); //ʹ�ܽ���
} 
//����һ��DMA����
//DMA_Streamx:DMA������,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:���ݴ�����  
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
	DMA_Cmd(DMA_Streamx, DISABLE);                      //�ر�DMA���� 	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//ȷ��DMA���Ա�����  		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //���ݴ�����  
	DMA_Cmd(DMA_Streamx, ENABLE);                      //����DMA���� 
}	  
// DMA �����жϳ�ʼ��
void DMA_IRQ_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
#ifdef DMA_IRQRX
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;  //�����ж�  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    NVIC_Init(&NVIC_InitStructure); 
	DMA_ITConfig(DMA1_Stream5,DMA_IT_TC,ENABLE);        //������ж�
#endif	
#ifdef DMA_IRQTX
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn; //�����ж�   
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,ENABLE);    
#endif
}

#ifdef DMA_IRQRX
////DMA �����жϴ���
void DMA1_Stream5_IRQHandler(void)  
{  
    //�����־  
    if(DMA_GetFlagStatus(DMA1_Stream5,DMA_FLAG_TCIF5)!=RESET)//�ȴ�DMA�������  
	{   
		DMA_Cmd(DMA1_Stream5,DISABLE);
		if((wifiUSART_RX_STA&(1<<15))==0)
			memcpy(wifiUSART_RX_BUF,ReceiveBuff,wifiUSART_MAX_RECV_LEN);
        DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);//���DMA������ɱ�־  
		DMA_SetCurrDataCounter(DMA1_Stream5, RECE_BUF_SIZE);  
        DMA_Cmd(DMA1_Stream5,ENABLE);     //��DMA,
    }  
} 
#endif

#ifdef DMA_IRQTX
///DMA �����жϴ���
void DMA1_Stream6_IRQHandler(void)  
{  
    //�����־  
    if(DMA_GetFlagStatus(DMA1_Stream6,DMA_FLAG_TCIF6)!=RESET)//�ȴ�DMA�������  
    {   
        DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);//���DMA������ɱ�־  
    }  
}
#endif

//***********
//   dma����
void dma_test(void)
{	
	float pro=0;//����
	
	MYDMA_Config(DMA1_Stream3,DMA_Channel_4,(u32)&USART3->DR,(u32)SendBuff,SEND_BUF_SIZE,send);
//	MYDMA_Config(DMA1_Stream1,DMA_Channel_4,(u32)&USART3->DR,(u32)ReceiveBuff,RECE_BUF_SIZE,rev);

	memcpy(SendBuff,"test\r\n",6);
	
//	printf("\r\nDMA DATA:\r\n"); 	          
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);  //ʹ�ܴ��ڵ�DMA����  
//		USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);  //ʹ�ܴ��ڵ�DMA����
	MYDMA_Enable(DMA1_Stream3,SEND_BUF_SIZE);     //��ʼһ��DMA���䣡	
	while(1)
	{
		if(DMA_GetFlagStatus(DMA1_Stream3,DMA_FLAG_TCIF3)!=RESET)//�ȴ��������
		{ 
			DMA_ClearFlag(DMA1_Stream3,DMA_FLAG_TCIF3);//���������ɱ�־
			break; 
		}
		pro=DMA_GetCurrDataCounter(DMA1_Stream3);//�õ���ǰ��ʣ����ٸ�����
		pro=1-pro/SEND_BUF_SIZE;    //�õ��ٷֱ�	  
		pro*=100;      			    //����100��
//		printf("%.2f\r\n",pro);
	}
}
