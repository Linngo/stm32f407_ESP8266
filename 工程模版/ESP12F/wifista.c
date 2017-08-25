#include "common.h"


//ESP8266 连接wifi
//ssid:    wifi名称
//password: wifi密码
//返回值 0：成功
//		1:失败
u8 esp_12F_sta_link_wifi(const u8* ssid,const u8* password)
{
	u8 *p=NULL;
	u8 res=0;
	u8 i=0;

//	printf("无线参数为:%s,%s,%s\r\n",(u8*)ssid,(u8*)sta_encryption,(u8*)password);

	while(esp_12F_send_cmd("AT+CWMODE=1","OK",500))
	{i++;if(i>20) {res=1;goto re;}};		//设置WIFI STA模式
	
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",ssid,password);//设置无线参数:ssid,密码
	while(esp_12F_send_cmd(p,"WIFI GOT IP",5000))
	{i++;if(i>20) {res=1;goto re;}};			//连接目标路由器,并且获得IP
	delay_ms(100);
	while(esp_12F_send_cmd("AT+CWAUTOCONN=1","OK",1000))
	{i++;if(i>20) {res=1;goto re;}};			//上电自动连接wifi
	
	wifiUSART_RX_STA=0;

re:	
	return res;
}

//ESP8266 配置 udp/tcp/server
//返回值:0,正常
//    其他,错误代码
//netpro: udp/tcp/server
//ipbuf:远程IP地址
//portnum：服务器端口
//netpro=server时，ipbuf为TCP超时时间 [1,7200]s
//								设置0 服务器不断开连接
u8 esp_12F_setlink_mode(u8 netpro,const u8* ipbuf,const u8* portnum)
{
	u8 buf[16]={0};
	u8 *p=NULL;
	u8 res=0;
	
	//		printf("正在配置模块,请稍等...");
	if(netpro==0)   //UDP
	{
		sprintf((char*)p,"AT+CIPSTART=\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标UDP服务器
		delay_ms(200);
		esp_12F_send_cmd("AT+CIPMUX=0","OK",20);  //单链接模式
		delay_ms(200);
		while(esp_12F_send_cmd(p,"OK",500));
	}
	else     //TCP
	{ 
		if(netpro==1)     //TCP Client    透传模式测试
		{
			esp_12F_send_cmd("AT+CIPMUX=0","OK",20);   //0：单连接
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器
			while(esp_12F_send_cmd(p,"OK",200))
			{
//				printf("连接TCP失败"); //连接失败	 
			}		
		}
		else			//TCP Server
		{
			esp_12F_send_cmd("AT+CIPMUX=1","OK",50);   //1：多连接
			delay_ms(50);
			sprintf((char*)p,"AT+CIPSERVER=1,%s",(u8*)portnum);    //开启Server模式(0，关闭；1，打开)，端口号为portnum
			esp_12F_send_cmd(p,"OK",100);    
			
			sprintf((char*)p,"AT+CIPSTO=%s",(u8*)ipbuf);
			esp_12F_send_cmd(p,"OK",100);     //服务器连接超时 [1,7200]s
		}
	}
//	esp_12F_get_staip(buf);//服务器模式,获取ip
//	printf("IP地址:%s 端口:%s\r\n",buf,(u8*)portnum);//显示IP地址和端口
	
	wifiUSART_RX_STA=0;

	return res;		
}
