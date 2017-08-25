#include "delay.h"
#include "wifiUSART.h"
#include "stdarg.h"	 	 
#include "stdio.h"	 	 
#include "string.h"	  
#include "timer.h"
#include "dma.h"

//串口发送缓存区 	
__align(8) u8 wifiUSART_TX_BUF[wifiUSART_MAX_SEND_LEN]; 	//发送缓冲,最大wifiUSART_MAX_SEND_LEN字节
#ifdef wifiUSART_RX_EN   								//如果使能了接收   	  
//串口接收缓存区 	
u8 wifiUSART_RX_BUF[wifiUSART_MAX_RECV_LEN]; 				//接收缓冲,最大wifiUSART_MAX_RECV_LEN个字节.

//通过判断接收连续2个字符之间的时间差不大于100ms来决定是不是一次连续的数据.
//如果2个字符接收间隔超过100ms,则认为不是1次连续数据.也就是超过100ms没有接收到
//任何数据,则表示此次接收完毕.
//接收到的数据状态
//[15]:0,没有接收到数据;1,接收到了一批数据.
//[14:0]:接收到的数据长度
u16 wifiUSART_RX_STA=0;   	 
void USART2_IRQHandler(void)
{
	u8 clear=clear;
#ifdef DMA_RX
#else
	u8 res;	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART2);		
		if((wifiUSART_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(wifiUSART_RX_STA<wifiUSART_MAX_RECV_LEN)		//还可以接收数据
			{
				wifiUSART_RX_BUF[wifiUSART_RX_STA++]=res;		//记录接收到的值
			}
			else
			{
				wifiUSART_RX_STA|=1<<15;					//强制标记接收完成
			}
		}
	}
#endif	
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //接收完一帧数据
	{
		clear = USART2->SR;
		clear = USART2->DR;
#ifdef DMA_RX		
		DMA_Cmd(DMA1_Stream5,DISABLE);
		wifiUSART_RX_STA = RECE_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Stream5); 
		if((wifiUSART_RX_STA&(1<<15))==0)         //允许接收新数据到wifiUSART_RX_BUF
		{	
			if(wifiUSART_RX_STA>(1460+16)) wifiUSART_RX_STA=1476;
			memcpy(wifiUSART_RX_BUF,ReceiveBuff,wifiUSART_RX_STA);	
		}
		DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);//清除DMA1_Steam5传输完成标志  
		DMA_SetCurrDataCounter(DMA1_Stream5, RECE_BUF_SIZE);  
		DMA_Cmd(DMA1_Stream5,ENABLE);     //打开DMA,
#endif		
		wifiUSART_RX_STA|=1<<15;
	}										 
}  
#endif	
//初始化IO 串口
//bound:波特率	  
void wifiUSART_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART时钟

	USART_DeInit(USART2);  //复位串口

	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); //GPIOB11复用为USART
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); //GPIOB10复用为USART	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; //GPIOB11和GPIOB10初始化
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化GPIOB11，和GPIOB10

	USART_InitStructure.USART_BaudRate = bound;//波特率 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口3

	USART_Cmd(USART2, ENABLE);               //使能串口 

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启空闲中断
#ifdef DMA_RX
#else	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接收中断   
#endif

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	wifiUSART_RX_STA=0;		//清零
}

//串口,printf 函数
//确保一次发送数据不超过wifiUSART_MAX_SEND_LEN字节
void u3_printf(char* fmt,...)  
{  
	u16 i;
//	u16 j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)wifiUSART_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)wifiUSART_TX_BUF);//此次发送数据的长度
#ifdef DMA_RX
	memcpy(SendBuff,wifiUSART_TX_BUF,i);
	MYDMA_Enable(DMA1_Stream6,i);
#else	
	for(j=0;j<i;j++)//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);  //等待上次传输完成 
		USART_SendData(USART2,(uint8_t)wifiUSART_TX_BUF[j]); 	 //发送数据到串口3 
	}
#endif	
}
