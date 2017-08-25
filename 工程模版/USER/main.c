#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "wifiusart.h"
#include "timer.h"
#include "common.h"

void toInt(char *str,u16 *number)//�ַ���ת����
{
	int i;
	*number = 0;//ǿ�Ƴ�ʼ��
	for (i = 0; str[i] != 0; i++)
		*number = (u16)(str[i] - 48) + *number * 10;//��һȡ�ַ�ת��Ϊ���֣�����Ȩ����number
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
//ʵ��α���߳�
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
//����TCP��udpͨ��
u8 test_sta(u8 netpro)
{
static u8 *p=NULL;
static u8 *p1=NULL;
static u16 t=0;		//���ٵ�һ�λ�ȡ����״̬
static u8 constate=0;	//����״̬
static u8 overtime=0;

_SS	
	p=mymalloc(SRAMIN,32);
	p1=mymalloc(SRAMIN,32);
	
	printf("%s����\r\n",esp_12F_WORKMODE_TBL[netpro-1]);
		
	while(1)
	{
		if(esp_12F_check_cmd("CONNECT"))   //�յ��豸������Ϣ
		{
			if(esp_12F_check_cmd("DISCONNECT"))
			{printf("wifi���ӶϿ�\r\n");goto RET;}
			if(esp_12F_check_cmd("WIFI CONNECTED"))
			{printf("wifi����\r\n");	goto RET;}
			
			printf("�ͻ���%c ����TCP������\r\n",wifiUSART_RX_BUF[0]);
			sprintf((char*)p,"AT+CIPSENDEX=%c,20",wifiUSART_RX_BUF[0]);
			esp_12F_send_cmd(p,"\r\n> ",1000);	//��������
			wifiUSART_RX_STA=0;
			u3_printf("%s����\r\n\\0",esp_12F_WORKMODE_TBL[netpro-1]);//��������
			while(!esp_12F_check_cmd("SEND OK"))					//�ȴ��������
			{
				wifiUSART_RX_STA=0;
				delay_ms(10);
				overtime++;
				if(overtime>20){overtime=0;break;}
			}
			t=0;
RET:			
			wifiUSART_RX_STA=0;				//����������
		}
		if(esp_12F_check_cmd("CLOSED"))		//�յ��豸�Ͽ���Ϣ
		{
			printf("�ͻ���%c �Ͽ�TCP������\r\n",wifiUSART_RX_BUF[0]);
			wifiUSART_RX_STA=0;				//����������
		}
		WaitX(1);
//		delay_ms(10);
		if(esp_12F_check_cmd("+IPD,"))		//���յ�һ��������
		{ 			
			p=(u8 *)strstr((const char*)wifiUSART_RX_BUF,",");
			p[2]=0;
			p1=(u8 *)strstr((const char*)(p+3),":");
			p1[0]=0;
			printf("�յ��ͻ���%s ����%s�ֽ�,����:\r\n%s\r\n",p+1,p+3,p1+1);
//			printf("�յ��ͻ���%s ����%s�ֽ�\r\n",p+1,p+3);
			wifiUSART_RX_STA=0;				//����������
			t=0;
		}
		if(wifiUSART_RX_STA==0)
		{	if(t<60000)
			{	t++;
			}else
			{	
				constate=esp_12F_consta_check();//�õ�����״̬
				if(constate==0)printf("��������\r\n10�������豸��Ϣ\r\n�رշ���\r\n");  //����״̬
				else printf("��������ʧ��\r\n�رշ���\r\n"); 	
//				esp_12F_send_cmd("AT+CIPSERVER=0","OK",50);
				t=0;
				break;
			}
		}
	}
	myfree(SRAMIN,p);		//�ͷ��ڴ� 
	myfree(SRAMIN,p1);	
//_EE
	;}; return 0;
}
//����ATָ��
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
	delay_init(168);		  //��ʼ����ʱ����
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2	
	  
	initled();
	uart_init(115200);      //������Ϣ
 	TIM3_Int_Init(100-1,8400-1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����100��Ϊ10ms
	
	esp_12F_init();
	
#ifdef esp	
	wifi_ESP();
#else	
	esp_12F_sta_link_wifi(sta_ssid,sta_password);
#endif	
	esp_12F_setlink_mode(server,(const u8*)"240",portnum);
	while(1)
	{	
		//RunTaskA(test_sta(3),0);
		{ if (timers[0]==0) {timers[0]=test_sta(3); continue;} }
		RunTaskA(taskled,1);
		RunTaskA(test_AT,2);
	};
}
