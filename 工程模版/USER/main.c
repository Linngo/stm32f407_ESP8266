#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "wifiusart.h"
#include "timer.h"
#include "common.h"
#include <stdlib.h>

void toInt(char *str,u16 *number)//字符串转数字
{
	int i;
	*number = 0;//强制初始化
	for (i = 0; str[i] != 0; i++)
		*number = (u16)(str[i] - 48) + *number * 10;//逐一取字符转换为数字，并升权放入number
}

void initled(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_SetBits(GPIOD,GPIO_Pin_13 | GPIO_Pin_14);		
}
//实验伪多线程
unsigned char taskled(void)
{
_SS
	while(1)
		{
		GPIO_ResetBits(GPIOD,GPIO_Pin_13);  
		GPIO_SetBits(GPIOD,GPIO_Pin_14);   
		WaitX(50); //500ms
		GPIO_SetBits(GPIOD,GPIO_Pin_13);	   
		GPIO_ResetBits(GPIOD,GPIO_Pin_14); 
		WaitX(50); //500ms	
		}
;};
}
//测试TCP通信
u8 test_sta(u8 netpro)
{
static u8 *p=NULL;
static u8 *p1=NULL;
static u16 t=0;		//加速第一次获取链接状态
static u8 constate=0;	//连接状态

_SS	
	p=mymalloc(SRAMIN,32);
	p1=mymalloc(SRAMIN,32);
	
	wifi.RxTimes=0;

	printf("%s测试\r\n",esp_12F_WORKMODE_TBL[netpro-1]);
		
	while(1)
	{
		if(wifiUSART_RX_STA>>15==1)
		{
			if(esp_12F_check_cmd("CONNECT"))   //收到设备连接信息
			{
				if(esp_12F_check_cmd("DISCONNECT"))
				{	
					wifi.status = Wifi_ConnectionFail;
					printf("wifi连接断开\r\n");
					goto RET;
				}
				if(esp_12F_check_cmd("WIFI"))
				{
					wifi.status = Wifi_Connected;
					printf("wifi连接\r\n");	
					goto RET;
				}		
				wifi.TcpIpConnections[wifiUSART_RX_BUF[0]-48].status = TCP_Connected;
				printf("客户端%c 连上TCP服务器\r\n",wifiUSART_RX_BUF[0]);
				
				tcp_send(wifiUSART_RX_BUF[0]-48,"tcp_server_test\r\n",20);

				t=0;
	RET:			
				wifiUSART_RX_STA=0;				//允许新数据
			}
			if(esp_12F_check_cmd("CLOSED"))		//收到设备断开信息
			{
				wifi.TcpIpConnections[wifiUSART_RX_BUF[0]-48].status = TCP_Disconnected;
				printf("客户端%c 断开TCP服务器\r\n",wifiUSART_RX_BUF[0]);
				wifiUSART_RX_STA=0;				//允许新数据
			}
			if(esp_12F_check_cmd("+IPD,"))		//接收到一次数据了
			{ 			
				wifi.RxIsData=1;
				wifi.RxTimes++;
				p=(u8 *)strstr((const char*)wifiUSART_RX_BUF,",");
				p[2]=0;
				p1=(u8 *)strstr((const char*)(p+3),":");
				p1[0]=0;
				
				wifi.LinkId = (p+1)[0]-48;
				wifi.RxDataLen = atoi((char*)p+3);
				memcpy(wifi.Rxdata,p1+1,wifi.RxDataLen);
				wifi.Rxdata[wifi.RxDataLen] = 0;
				
//				printf("收到客户端%d 数据%d字节,内容:\r\n%s\r\n",wifi.LinkId,wifi.RxDataLen,wifi.Rxdata);
				printf("%d %d\r\n",wifi.RxTimes,wifi.RxDataLen);
				wifiUSART_RX_STA=0;				//允许新数据
				t=0;
			}
			if(esp_12F_check_cmd("\r\nOK\r\n"))
				wifiUSART_RX_STA=0;
		}
		WaitX(1); //10ms 释放CPU
		if(wifiUSART_RX_STA==0)
		{	if(t<60000)
			{	
				t++;
				if(t%6000==0) wifi.RxTimes=0;
			}else
			{	
				constate=esp_12F_apsta_check();//得到连接状态
				if(constate==Wifi_Connected)
					printf("网络正常,未建立TCP连接\r\n"); 
				if(constate==TCP_Connected)
					printf("10分钟未收到客户端信息\r\n");  
				if(constate==TCP_Disconnected)
					printf("10分钟无TCP连接\r\n"); 
				if(constate==Wifi_ConnectionFail)
					printf("未连接网络\r\n"); 
				t=0;
				break;
			}
		}
	}
 
//_EE
	;}; return 0;
}
//测试AT指令
u8 test_AT(void)
{
static char *a = NULL;
static char *b = NULL;
static char *c = NULL;
static u16 number;
_SS	
while(1){
	if(USART_RX_STA&0x8000)
	{					   			
		USART_RX_STA=0;
		a=strtok((char *)USART_RX_BUF, "/");
		b=strtok(NULL,"/");
		c=strtok(NULL,"/");
		toInt(c,&number);
		if(a[0]=='Q')
			esp_12F_quit_trans();
		else
			esp_12F_send_cmd((u8 *)a,(u8 *)b,number);
		esp_12F_at_response(0);
		wifiUSART_RX_STA=0;
	}
	WaitX(1);
}
;};
}

int main(void)
{ 
	delay_init(168);		  //初始化延时函数
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2	
	  
	initled();
	uart_init(115200);      //调试信息
 	TIM3_Int_Init(100-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数100次为10ms
	
	esp_12F_init();
	
#ifdef esp	
	wifi_ESP();
#else	
	esp_12F_sta_link_wifi(sta_ssid,sta_password);
#endif	
	esp_12F_setlink_mode(server,(const u8*)"240",portnum);
	while(1)
	{	
//		wifi_callback();
		{ if (timers[0]==0) {timers[0]=test_sta(3); continue;} }
		RunTaskA(taskled,1);
		RunTaskA(test_AT,2);
	};
}
