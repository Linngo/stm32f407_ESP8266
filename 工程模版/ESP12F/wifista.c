#include "common.h"
#include <stdlib.h>

//ESP8266 连接wifi
//ssid:    wifi名称
//password: wifi密码
//返回值 0：成功
//		1:失败
u8 esp_12F_sta_link_wifi(const u8* ssid,const u8* password)
{
	u8 p[32]={0};
	u8 res=0;
	u8 i=0;
	
//	printf("无线参数为:%s,%s,%s\r\n",(u8*)ssid,(u8*)sta_encryption,(u8*)password);

	while(esp_12F_send_cmd("AT+CWMODE=1","OK",500))
	{i++;if(i>20) {res=1;goto re;}};		//设置WIFI STA模式	
	wifi.Mode=Station;
	
//	if(!chech_ssid((u8*)ssid)) goto re;		//验证ssid是否存在
	sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",ssid,password);//设置无线参数:ssid,密码
	//sprintf((char*)p,"AT+CWJAP_CUR=\"%s\",\"%s\",\"%s\"\r\n",ssid,password,MAC);

	while(esp_12F_send_cmd(p,"WIFI GOT IP",5000))
	{i++;if(i>20) {res=1;goto re;}};			//连接目标路由器,并且获得IP
	wifi.status = Wifi_Connected;
	
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
	u8 p[32]={0};
	u8 res=0;
	
	if(netpro==0)   //UDP
	{
//		printf("WIFI-STA udp mode\r\n"); 
//		printf("正在配置模块,请稍等...");
		sprintf((char*)p,"AT+CIPSTART=\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标UDP服务器
		delay_ms(100);
		esp_12F_send_cmd("AT+CIPMUX=0","OK",20);  //单链接模式
		delay_ms(100);
		while(esp_12F_send_cmd(p,"OK",500));
	}
	else     //TCP
	{ 
//		printf("正在配置模块,请稍等...\r\n");
		if(netpro==1)     //TCP Client    透传模式测试
		{
			printf("WIFI-STA tcp_client mode\r\n");
			esp_12F_send_cmd("AT+CIPMUX=0","OK",20);   //0：单连接
			sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ipbuf,(u8*)portnum);    //配置目标TCP服务器
			while(esp_12F_send_cmd(p,"OK",200))
			{
//				printf("连接TCP失败"); //连接失败	 
			}	
	//		esp_12F_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传			
		}
		else					//TCP Server
		{
//			printf("WIFI-STA tcp_server mode\r\n");
			esp_12F_send_cmd("AT+CIPMUX=1","OK",50);   //1：多连接
			delay_ms(50);
			esp_12F_send_cmd("AT+CIPSERVER=0","OK",100);
			sprintf((char*)p,"AT+CIPSERVER=1,%s",(u8*)portnum);    //开启Server模式(0，关闭；1，打开)，端口号为portnum
			esp_12F_send_cmd(p,"OK",100);    
			sprintf((char*)p,"AT+CIPSTO=%s",(u8*)ipbuf);
			esp_12F_send_cmd(p,"OK",100);     //服务器连接超时 [1,7200]s
		}
	}
	esp_12F_get_staip();//服务器模式,获取ip
//	printf("IP地址:%s 端口:%s\r\n",wifi.MyIP,(u8*)portnum);//显示IP地址和端口
	wifiUSART_RX_STA=0;
	return res;		
}
//接收数据
//返回值: 0 其他模块响应
//		1 接收到一次数据
u8 wifi_callback(void)
{
	static u8 *p=NULL;
	static u8 *p1=NULL;
	if(wifiUSART_RX_STA>>15==1)
	{
		if(esp_12F_check_cmd("CONNECT"))   //收到设备连接信息
		{
			if(esp_12F_check_cmd("DISCONNECT"))
			{	
				wifi.status = Wifi_ConnectionFail;
	//			printf("wifi连接断开\r\n");
				goto RET;
			}
			if(esp_12F_check_cmd("WIFI"))
			{
				wifi.status = Wifi_Connected;
	//			printf("wifi连接\r\n");	
				goto RET;
			}
			wifi.RxIsData=0;
			wifi.TcpIpConnections[wifiUSART_RX_BUF[0]-48].status = TCP_Connected;
	//		printf("客户端%c 连上TCP服务器\r\n",wifiUSART_RX_BUF[0]);		
	//		tcp_send(wifiUSART_RX_BUF[0]-48,"tcp_server_test\r\n",20);
	RET:			
			wifiUSART_RX_STA=0;				//允许新数据
		}
		if(esp_12F_check_cmd("CLOSED"))		//收到设备断开信息
		{
			wifi.RxIsData=0;
			wifi.TcpIpConnections[wifiUSART_RX_BUF[0]-48].status = TCP_Disconnected;
	//		printf("客户端%c 断开TCP服务器\r\n",wifiUSART_RX_BUF[0]);
			wifiUSART_RX_STA=0;				//允许新数据
		}
		if(esp_12F_check_cmd("+IPD,"))		//接收到一次数据了
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
			
	//		printf("收到客户端%d 数据%d字节,内容:\r\n%s\r\n",wifi.LinkId,wifi.RxDataLen,wifi.Rxdata);

			wifiUSART_RX_STA=0;				//允许新数据
			return 1;
		}
		wifiUSART_RX_STA=0;
	}
	return 0;
}
