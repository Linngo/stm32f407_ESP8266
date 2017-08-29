#include "common.h"
#include <stdlib.h>

//ESP8266 ����wifi
//ssid:    wifi����
//password: wifi����
//����ֵ 0���ɹ�
//		1:ʧ��
u8 esp_12F_sta_link_wifi(const u8* ssid,const u8* password)
{
	u8 *p=NULL;
	u8 res=0;
	u8 i=0;
	
	p=mymalloc(SRAMIN,32);
	
	printf("���߲���Ϊ:%s,%s,%s\r\n",(u8*)ssid,(u8*)sta_encryption,(u8*)password);

	while(esp_12F_send_cmd("AT+CWMODE=1","OK",500))
	{i++;if(i>20) {res=1;goto re;}};		//����WIFI STAģʽ
	
	wifi.Mode=Station;
//	if(!chech_ssid((u8*)ssid)) goto re;		//��֤ssid�Ƿ����
	
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",ssid,password);//�������߲���:ssid,����
	while(esp_12F_send_cmd(p,"WIFI GOT IP",5000))
	{i++;if(i>20) {res=1;goto re;}};			//����Ŀ��·����,���һ��IP
	delay_ms(100);
	while(esp_12F_send_cmd("AT+CWAUTOCONN=1","OK",1000))
	{i++;if(i>20) {res=1;goto re;}};			//�ϵ��Զ�����wifi
	
	wifiUSART_RX_STA=0;

	myfree(SRAMIN,p);
re:	
	return res;
}

//ESP8266 ���� udp/tcp/server
//����ֵ:0,����
//    ����,�������
//netpro: udp/tcp/server
//ipbuf:Զ��IP��ַ
//portnum���������˿�
//netpro=serverʱ��ipbufΪTCP��ʱʱ�� [1,7200]s
//								����0 ���������Ͽ�����
u8 esp_12F_setlink_mode(u8 netpro,const u8* ipbuf,const u8* portnum)
{
	u8 *p=NULL;
	u8 res=0;

	p=mymalloc(SRAMIN,32);							//����32�ֽ��ڴ�
	
	if(netpro==0)   //UDP
	{
		printf("WIFI-STA udp mode\r\n"); 
		printf("��������ģ��,���Ե�...");
		sprintf((char*)p,"AT+CIPSTART=\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��UDP������
		delay_ms(200);
		esp_12F_send_cmd("AT+CIPMUX=0","OK",20);  //������ģʽ
		delay_ms(200);
		while(esp_12F_send_cmd(p,"OK",500));
	}
	else     //TCP
	{ 
		printf("��������ģ��,���Ե�...\r\n");
		if(netpro==1)     //TCP Client    ͸��ģʽ����
		{
			printf("WIFI-STA tcp_client mode\r\n");
			esp_12F_send_cmd("AT+CIPMUX=0","OK",20);   //0��������
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //����Ŀ��TCP������
			while(esp_12F_send_cmd(p,"OK",200))
			{
				printf("����TCPʧ��"); //����ʧ��	 
			}	
	//		esp_12F_send_cmd("AT+CIPMODE=1","OK",200);      //����ģʽΪ��͸��			
		}
		else					//TCP Server
		{
			printf("WIFI-STA tcp_server mode\r\n");
			esp_12F_send_cmd("AT+CIPMUX=1","OK",50);   //1��������
			delay_ms(50);
			esp_12F_send_cmd("AT+CIPSERVER=0","OK",100); 
			sprintf((char*)p,"AT+CIPSERVER=1,%s",(u8*)portnum);    //����Serverģʽ(0���رգ�1����)���˿ں�Ϊportnum
			esp_12F_send_cmd(p,"OK",100);    
			
			sprintf((char*)p,"AT+CIPSTO=%s",(u8*)ipbuf);
			esp_12F_send_cmd(p,"OK",100);     //���������ӳ�ʱ [1,7200]s
		}
	}
	esp_12F_get_staip();//������ģʽ,��ȡip
	printf("IP��ַ:%s �˿�:%s\r\n",wifi.MyIP,(u8*)portnum);//��ʾIP��ַ�Ͷ˿�
	
	wifiUSART_RX_STA=0;

	myfree(SRAMIN,p);		//�ͷ��ڴ� 	
	return res;		
}
//��������
//����ֵ: 0 ����
//		1 ���յ�һ������
u8 wifi_callback(void)
{
	static u8 *p=NULL;
	static u8 *p1=NULL;
	
	if(esp_12F_check_cmd("CONNECT"))   //�յ��豸������Ϣ
	{
		if(esp_12F_check_cmd("DISCONNECT"))
		{	
			wifi.status = Wifi_ConnectionFail;
			printf("wifi���ӶϿ�\r\n");
			goto RET;
		}
		if(esp_12F_check_cmd("WIFI"))
		{
			wifi.status = Wifi_Connected;
			printf("wifi����\r\n");	
			goto RET;
		}
		wifi.RxIsData=0;
		wifi.status = TCP_Connected;
		printf("�ͻ���%c ����TCP������\r\n",wifiUSART_RX_BUF[0]);
		
		tcp_send(wifiUSART_RX_BUF[0]-48,"tcp_server_test\r\n",20);
RET:			
		wifiUSART_RX_STA=0;				//����������
	}
	if(esp_12F_check_cmd("CLOSED"))		//�յ��豸�Ͽ���Ϣ
	{
		wifi.RxIsData=0;
		wifi.status = TCP_Disconnected;
		printf("�ͻ���%c �Ͽ�TCP������\r\n",wifiUSART_RX_BUF[0]);
		wifiUSART_RX_STA=0;				//����������
	}
	if(esp_12F_check_cmd("+IPD,"))		//���յ�һ��������
	{
		wifi.RxIsData=1;
		p=(u8 *)strstr((const char*)wifiUSART_RX_BUF,",");
		p[2]=0;
		p1=(u8 *)strstr((const char*)(p+3),":");
		p1[0]=0;
		
		wifi.LinkId = (p+1)[0]-48;
		wifi.RxDataLen = atoi((char*)p+3);
		memcpy(wifi.Rxdata,p1+1,wifi.RxDataLen);
		wifi.Rxdata[wifi.RxDataLen] = 0;
		
		printf("�յ��ͻ���%d ����%d�ֽ�,����:\r\n%s\r\n",wifi.LinkId,wifi.RxDataLen,wifi.Rxdata);

		wifiUSART_RX_STA=0;				//����������
		return 1;
	}
	return 0;
}
