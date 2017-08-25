#include "common.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//�û�������

//���Ӷ˿ں�:8086,���޸�Ϊ�����˿�.
const u8* portnum="8086";		 

//WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,��������Լ���·��������,�����޸�.
u8* sta_ssid="TP-LINK_nyear1";			//·����SSID��
u8* sta_encryption="wpawpa2_aes";	//wpa/wpa2 aes���ܷ�ʽ
u8* sta_password="18691869070"; 	//��������

//WIFI APģʽ,ģ���������߲���,�������޸�.
const u8* ap_ssid="ESP12F";			//����SSID��
const u8 ap_encryption=0;	//���ܷ�ʽ open
const u8* ap_password=""; 		//��������
const u8 ap_channel=6; 		//�ŵ�
const u8 ap_max_conn=1; 		//��������� ��Χ[1,4]

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//4������ģʽ 
const u8 *esp_12F_NETMODE_TBL[3]={"STA","AP","STA+AP"};	//����ģʽ
//4�ֹ���ģʽ
const u8 *esp_12F_WORKMODE_TBL[3]={"UDP_CLIENT","TCP_CLIENT","TCP_SERVER"};	//esp_12F����ģʽ
const u8 *esp_12F_ECN[5]={"open","WEP","WPA_PSK","WPA2_PSK","WPA_WPA2_PSK"};	//����ģʽ
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//esp_12Fģ���ʼ��
//return: 0,��ʼ���ɹ�
//			1,�쳣
u8 esp_12F_init(void)
{
	static	u8 i=0;
 	wifiUSART_init(115200);	//��ʼ������
	
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //ʹ�ܴ��ڵ�DMA����
	MYDMA_Config(DMA1_Stream5,DMA_Channel_4,(u32)&USART2->DR,(u32)ReceiveBuff,RECE_BUF_SIZE,rev);
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);  //ʹ�ܴ��ڵ�DMA���� 
	MYDMA_Config(DMA1_Stream6,DMA_Channel_4,(u32)&USART2->DR,(u32)SendBuff,SEND_BUF_SIZE,send);
	DMA_IRQ_init();
	
	while(esp_12F_send_cmd("AT","OK",40))
	{
		i++;
		delay_ms(10);
		if(i>20)
		{
			printf("wifiģ�鴮��ͨ���쳣\r\n");
			return 1;
		}
	}
	return 0;
} 

//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���   ��ӡdebug��Ϣ
//mode:0,������wifiUSART_RX_STA;
//     1,����wifiUSART_RX_STA;
void esp_12F_at_response(u8 mode)
{
	if(wifiUSART_RX_STA&0X8000)		//���յ�һ��������
	{ 
		wifiUSART_RX_BUF[wifiUSART_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",wifiUSART_RX_BUF);	//���͵�����
		if(mode)wifiUSART_RX_STA=0;
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                       esp12f ATָ���������
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//esp_12F���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* esp_12F_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(wifiUSART_RX_STA&0X8000)		//���յ�һ��������
	{ 
		wifiUSART_RX_BUF[wifiUSART_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)wifiUSART_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//��esp_12F��������
//cmd:���͵������ַ���(����Ҫ��ӻس�)
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:��ʱʱ��(��λ:1ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 esp_12F_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	wifiUSART_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(1);
			if(wifiUSART_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(esp_12F_check_cmd(ack)) 
				{
//					esp_12F_at_response(0);
					break;
				}//�õ���Ч���� 
				wifiUSART_RX_STA=0;
			}
		}
		if(waittime==0)
		{
			printf("%s ��Ӧ��ʱ\r\n",cmd);
			res=1;
		} 
	}
	return res;
} 

u8 esp_12F_send_data(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	wifiUSART_RX_STA=0;
	u3_printf("%s",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(1);
			if(wifiUSART_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(esp_12F_check_cmd(ack)) 
				{
				 //esp_12F_at_response(0);
					break;
				}//�õ���Ч���� 
				wifiUSART_RX_STA=0;
			}
		}
		if(waittime==0)
		{
			printf("%s  ��ʱ\r\n",cmd);
			res=1;
		} 
	}
	return res;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////								���ܺ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�ָ���������
//����ֵ��0 �ɹ�
//		1 ʧ��
u8 esp_12F_factory(void)
{
	u8 i=0;
	u8 res=0;
	esp_12F_send_cmd("AT+RESTORE","OK",100);
	while(!esp_12F_check_cmd("ready"))
	{
		wifiUSART_RX_STA=0;
		i++;
		delay_ms(10);
		if(i>200) 
		{res=1;break;}
	}
	return res;
}

//����ģ��
//����ֵ��0 �ɹ�
//		1 ʧ��
u8 esp_12F_reboot(void)
{
	u8 i=0;
	u8 res=0;
	esp_12F_send_cmd("AT+CWAUTOCONN=0","OK",100);
	esp_12F_send_cmd("AT+RST","OK",100);
	while(!esp_12F_check_cmd("ready"))
	{
		wifiUSART_RX_STA=0;
		i++;
		delay_ms(10);
		if(i>200) 
		{res=1;break;}
	}
	return res;
}

//esp_12F�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
u8 esp_12F_quit_trans(void)
{
	u3_printf("+++");
	delay_ms(500);					//�ȴ�500ms
	return esp_12F_send_cmd("AT","OK",20);//�˳�͸���ж�.
}

//esp_12F �Ͽ�wifi
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
u8 esp_disconnect_wifi(void)
{
	return esp_12F_send_cmd("AT+CWQAP","OK",100);
}

//esp_12Fģ�������״̬
//����ֵ:1��δ����;
//		2,������wifi�����ip
//      3,�ѽ���TCP\UDP����
//		4,��TCP\UDP����
//      5,δ����wifi
u8 esp_12F_apsta_check(void)
{
	u8 *p=NULL;
//	if(esp_12F_quit_trans())return 0;			//�˳�͸�� 
	esp_12F_send_cmd("AT+CIPSTATUS",":",100);	//��ѯ����״̬
	p=esp_12F_check_cmd("STATUS:");
	wifiUSART_RX_STA=0;
	if(p)return p[7];
	else return 1;
}
/*{
	u8 *p; 
	esp_12F_send_cmd("AT+CIPSTATUS","OK",20);	//��ȡWIFI STA����״̬
	p=esp_12F_check_cmd("+CIPSTATUS:");
	return p[12];
}*/
//��ȡesp_12Fģ�������״̬
//����ֵ:0,wifi��������;
//      1,wifiδ����.
u8 esp_12F_consta_check(void)
{
	u8 res=0;
	u8 *p=NULL;
	esp_12F_send_cmd("AT+CWJAP?","OK",50);		//��ѯ����״̬ ������NO AP,������+ CWJAP:<ssid>,<bssid>,<channel>,<rssi> 
	p=esp_12F_check_cmd("+CWJAP:"); 
	wifiUSART_RX_STA=0;
	if(p[0]!='+') res=1; 
	return res;
}

//��ȡ����IP��ַ
//ipbuf:ip��ַ
void esp_12F_get_staip(u8* ipbuf)
{
	u8 *p=NULL,*p1=NULL;
	if(esp_12F_send_cmd("AT+CIPSTA?","OK",50))//��ȡIP��ַʧ��
	{
		ipbuf[0]=0;
		return;
	}		
	p=esp_12F_check_cmd("\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	*p1=0;
	sprintf((char*)ipbuf,"%s",p+1);		
	wifiUSART_RX_STA=0;
}

//��������ap   ����ͬʱwifi��ر�
void esp_12F_ap_config(void)
{
	u8 *p=NULL;
	
	//p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	
	printf("ssid:%s passwd:%s ch:%d ecn:%s max_link:%d\r\n",ap_ssid,ap_password,ap_channel,esp_12F_ECN[ap_encryption],ap_max_conn);
	
	while(esp_12F_send_cmd("AT+CWMODE=2","OK",100));		//����WIFI APģʽ

	esp_12F_send_cmd("AT+CWDHCP_DEF=0,1","OK",50);		//����APʹ��DHCP��
	esp_12F_send_cmd("AT+CWDHCPS_DEF=0","OK",50);		//DHCP������DNS���ã�Ĭ��DNS:192.168.4.1
	
	//WIFI APģʽģ������WIFI��������/����/�ŵ�/����ģʽ/����������
	sprintf((char*)p,"AT+CWSAP_DEF=\"%s\",\"%s\",%d,%d,%d",ap_ssid,ap_password,ap_channel,ap_encryption,ap_max_conn);
	esp_12F_send_cmd(p,"OK",100);					//����AP����
	
	wifiUSART_RX_STA=0;
	
	//myfree(SRAMIN,p);		//�ͷ��ڴ� 
}

//esp_12Fģ����Ϣ
void esp_12F_msg(void)
{
	u8 *p=NULL,*p1=NULL,*p2=NULL;
	//p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	//p1=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	//p2=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�

	esp_12F_send_cmd("AT+GMR","OK",20);		//��ȡ�̼��汾��
	p=esp_12F_check_cmd("SDK version:");    
	p1=(u8*)strstr((const char*)(p+1),"\r\n");
	*p1=0;
	printf("�̼��汾:%s\r\n",p);
	esp_12F_send_cmd("AT+CWMODE?","+CWMODE:",20);	//��ȡ����ģʽ
	p=esp_12F_check_cmd(":");
	printf("����ģʽ:%s\r\n",(u8*)esp_12F_NETMODE_TBL[*(p+1)-'1']);
	esp_12F_send_cmd("AT+CWSAP?","+CWSAP:",100);	//��ȡap����
	p=esp_12F_check_cmd("\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	p2=p1;
	*p1=0;
	printf("AP_SSID��:%s\r\n",p+1);
	p=(u8*)strstr((const char*)(p2+1),"\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	p2=p1;
	*p1=0;
	delay_ms(1);
	printf("AP_����:%s\r\n",p+1);
	p=(u8*)strstr((const char*)(p2+1),",");
	p1=(u8*)strstr((const char*)(p+1),",");
	*p1=0;
	printf("ͨ����:%s\r\n",p+1);
	printf("���ܷ�ʽ:%s\r\n",(u8*)esp_12F_ECN[*(p1+1)-'0']);
	wifiUSART_RX_STA=0;
	
	//myfree(SRAMIN,p);		//�ͷ��ڴ� 
	//myfree(SRAMIN,p1);		//�ͷ��ڴ� 
	//myfree(SRAMIN,p2);		//�ͷ��ڴ� 
}

//��ʼ��������������
void set_ESP(void)
{
	esp_12F_send_cmd("AT+CWMODE=1","OK",100);   //����WiFiģ�鹤��ģʽΪ��STAģʽ
	esp_12F_send_cmd("AT+CWAUTOCONN=1","OK",100);   //ʹ���ϵ��Զ�����AP
	delay_ms(100);
	esp_12F_send_cmd("AT+CWSTARTSMART=3","OK",100); //֧��ESP-Touch��Airkiss��������
}
//�˳���������
void stop_ESP(void)
{
	esp_12F_send_cmd("AT+CWSTOPSMART","OK",100);
	wifiUSART_RX_STA=0;
}
//������������ 10���Ӳ��ɹ���ʧ��
//����ֵ��0 �ɹ�
//		1 ʧ��
u8 wifi_ESP(void)
{
	u16 i=0;
ESP:	
	set_ESP();
	printf("�ȴ�smartconfig ����wifi�˺�����\r\n");
	while(!(esp_12F_check_cmd("WIFI CONNECT")
			||esp_12F_check_cmd("WIFI GOT IP")))
	{
		wifiUSART_RX_STA=0;
		delay_ms(10);
		i++;
		if(i%6000==0)
		{
			printf("�ȴ�smartconfig ����wifi�˺�����\r\n");
			stop_ESP();
			goto ESP;
		}
		if(i>60000)
		{
			printf("smartconfigʧ��\r\n");
			stop_ESP();
			i=0;
			return 1;
		}
	}
	printf("smartconfig�ɹ�\r\n");
	stop_ESP();
	return 0;
}

//����tcp�������˿�
u8 change_port(u8* portnum)
{
	u8* p=NULL;
	u8 res=0;
	esp_12F_send_cmd("AT+CIPMUX=1","OK",100);
	esp_12F_send_cmd("AT+CIPSERVER=0","OK",100);
	sprintf((char*)p,"AT+CIPSERVER=1,%s",(u8*)portnum);
	if(esp_12F_send_cmd(p,"OK",100)) res=1;
	return res; 
}

//���wifi ssid�Ƿ����
//����ֵ:0,wifi����
//    ����,û�����wifi
u8* chech_ssid(u8* ssid)
{
	esp_12F_send_cmd("AT+CWLAP","+CWLAP:",5000);
	while(esp_12F_check_cmd("OK")){delay_ms(10);wifiUSART_RX_STA=0;};
//	esp_12F_at_response(1);  //���ɨ�赽����Ϣwifi
	return esp_12F_check_cmd(ssid);
}
