#include "dma.h"																	   	  
#include "delay.h"
#include <string.h>

u8 SendBuff[SEND_BUF_SIZE];    //发送数据缓冲区 
u8 ReceiveBuff[RECE_BUF_SIZE];   //接收缓冲

//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:外设地址
//mar:存储器地址
//ndtr:数据传输量
//dir:数据方向  1:发送0:接收
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr,u8 dir)
{ 
 
	DMA_InitTypeDef  DMA_InitStructure;

	if((u32)DMA_Streamx>(u32)DMA2)//得到当前stream是属于DMA2还是DMA1
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 	
	}else 
	{
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	}
	DMA_DeInit(DMA_Streamx);

	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}//等待DMA可配置 

	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = chx;  //通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;//DMA 存储器0地址
	if(dir==1)
	{
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设
	}else{
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器
	}
	DMA_InitStructure.DMA_BufferSize = ndtr;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	DMA_Init(DMA_Streamx, &DMA_InitStructure);//初始化DMA Stream
	if(dir==rev)
		DMA_Cmd(DMA_Streamx,ENABLE); //使能接收
} 
//开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
	DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//确保DMA可以被设置  		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //数据传输量  
	DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
}	  
// DMA 发送中断初始化
void DMA_IRQ_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
#ifdef DMA_IRQRX
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;  //接收中断  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    NVIC_Init(&NVIC_InitStructure); 
	DMA_ITConfig(DMA1_Stream5,DMA_IT_TC,ENABLE);        //开完成中断
#endif	
#ifdef DMA_IRQTX
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn; //发送中断   
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;    
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;    
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    
    NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,ENABLE);    
#endif
}

#ifdef DMA_IRQRX
////DMA 接收中断处理
void DMA1_Stream5_IRQHandler(void)  
{  
    //清除标志  
    if(DMA_GetFlagStatus(DMA1_Stream5,DMA_FLAG_TCIF5)!=RESET)//等待DMA传输完成  
	{   
		DMA_Cmd(DMA1_Stream5,DISABLE);
		if((wifiUSART_RX_STA&(1<<15))==0)
			memcpy(wifiUSART_RX_BUF,ReceiveBuff,wifiUSART_MAX_RECV_LEN);
        DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);//清除DMA传输完成标志  
		DMA_SetCurrDataCounter(DMA1_Stream5, RECE_BUF_SIZE);  
        DMA_Cmd(DMA1_Stream5,ENABLE);     //打开DMA,
    }  
} 
#endif

#ifdef DMA_IRQTX
///DMA 发送中断处理
void DMA1_Stream6_IRQHandler(void)  
{  
    //清除标志  
    if(DMA_GetFlagStatus(DMA1_Stream6,DMA_FLAG_TCIF6)!=RESET)//等待DMA传输完成  
    {   
        DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);//清除DMA传输完成标志  
    }  
}
#endif

//***********
//   dma测试
void dma_test(void)
{	
	float pro=0;//进度
	
	MYDMA_Config(DMA1_Stream3,DMA_Channel_4,(u32)&USART3->DR,(u32)SendBuff,SEND_BUF_SIZE,send);
//	MYDMA_Config(DMA1_Stream1,DMA_Channel_4,(u32)&USART3->DR,(u32)ReceiveBuff,RECE_BUF_SIZE,rev);

	memcpy(SendBuff,"test\r\n",6);
	
//	printf("\r\nDMA DATA:\r\n"); 	          
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);  //使能串口的DMA发送  
//		USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);  //使能串口的DMA接收
	MYDMA_Enable(DMA1_Stream3,SEND_BUF_SIZE);     //开始一次DMA传输！	
	while(1)
	{
		if(DMA_GetFlagStatus(DMA1_Stream3,DMA_FLAG_TCIF3)!=RESET)//等待传输完成
		{ 
			DMA_ClearFlag(DMA1_Stream3,DMA_FLAG_TCIF3);//清除传输完成标志
			break; 
		}
		pro=DMA_GetCurrDataCounter(DMA1_Stream3);//得到当前还剩余多少个数据
		pro=1-pro/SEND_BUF_SIZE;    //得到百分比	  
		pro*=100;      			    //扩大100倍
//		printf("%.2f\r\n",pro);
	}
}
