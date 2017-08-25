#include "common.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//用户配置区

//连接端口号:8086,可修改为其他端口.
const u8* portnum="8086";		 

//WIFI STA模式,设置要去连接的路由器无线参数,请根据你自己的路由器设置,自行修改.
u8* sta_ssid="TP-LINK_nyear1";			//路由器SSID号
u8* sta_encryption="wpawpa2_aes";	//wpa/wpa2 aes加密方式
u8* sta_password="18691869070"; 	//连接密码

//WIFI AP模式,模块对外的无线参数,可自行修改.
const u8* ap_ssid="ESP12F";			//对外SSID号
const u8 ap_encryption=0;	//加密方式 open
const u8* ap_password=""; 		//连接密码
const u8 ap_channel=6; 		//信道
const u8 ap_max_conn=1; 		//最大连接数 范围[1,4]

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//4个网络模式 
const u8 *esp_12F_NETMODE_TBL[3]={"STA","AP","STA+AP"};	//网络模式
//4种工作模式
const u8 *esp_12F_WORKMODE_TBL[3]={"UDP_CLIENT","TCP_CLIENT","TCP_SERVER"};	//esp_12F工作模式
const u8 *esp_12F_ECN[5]={"open","WEP","WPA_PSK","WPA2_PSK","WPA_WPA2_PSK"};	//加密模式
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//esp_12F模块初始化
//return: 0,初始化成功
//			1,异常
u8 esp_12F_init(void)
{
	static	u8 i=0;
 	wifiUSART_init(115200);	//初始化串口
	
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  //使能串口的DMA接收
	MYDMA_Config(DMA1_Stream5,DMA_Channel_4,(u32)&USART2->DR,(u32)ReceiveBuff,RECE_BUF_SIZE,rev);
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);  //使能串口的DMA发送 
	MYDMA_Config(DMA1_Stream6,DMA_Channel_4,(u32)&USART2->DR,(u32)SendBuff,SEND_BUF_SIZE,send);
	DMA_IRQ_init();
	
	while(esp_12F_send_cmd("AT","OK",40))
	{
		i++;
		delay_ms(10);
		if(i>20)
		{
			printf("wifi模块串口通信异常\r\n");
			return 1;
		}
	}
	return 0;
} 

//将收到的AT指令应答数据返回给电脑串口   打印debug信息
//mode:0,不清零wifiUSART_RX_STA;
//     1,清零wifiUSART_RX_STA;
void esp_12F_at_response(u8 mode)
{
	if(wifiUSART_RX_STA&0X8000)		//接收到一次数据了
	{ 
		wifiUSART_RX_BUF[wifiUSART_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",wifiUSART_RX_BUF);	//发送到串口
		if(mode)wifiUSART_RX_STA=0;
	} 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                       esp12f AT指令操作驱动
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//esp_12F发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* esp_12F_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(wifiUSART_RX_STA&0X8000)		//接收到一次数据了
	{ 
		wifiUSART_RX_BUF[wifiUSART_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)wifiUSART_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}
//向esp_12F发送命令
//cmd:发送的命令字符串(不需要添加回车)
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:超时时间(单位:1ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 esp_12F_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	wifiUSART_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(1);
			if(wifiUSART_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(esp_12F_check_cmd(ack)) 
				{
//					esp_12F_at_response(0);
					break;
				}//得到有效数据 
				wifiUSART_RX_STA=0;
			}
		}
		if(waittime==0)
		{
			printf("%s 响应超时\r\n",cmd);
			res=1;
		} 
	}
	return res;
} 

u8 esp_12F_send_data(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	wifiUSART_RX_STA=0;
	u3_printf("%s",cmd);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(1);
			if(wifiUSART_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(esp_12F_check_cmd(ack)) 
				{
				 //esp_12F_at_response(0);
					break;
				}//得到有效数据 
				wifiUSART_RX_STA=0;
			}
		}
		if(waittime==0)
		{
			printf("%s  超时\r\n",cmd);
			res=1;
		} 
	}
	return res;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////								功能函数
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//恢复出厂设置
//返回值：0 成功
//		1 失败
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

//重启模块
//返回值：0 成功
//		1 失败
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

//esp_12F退出透传模式
//返回值:0,退出成功;
//       1,退出失败
u8 esp_12F_quit_trans(void)
{
	u3_printf("+++");
	delay_ms(500);					//等待500ms
	return esp_12F_send_cmd("AT","OK",20);//退出透传判断.
}

//esp_12F 断开wifi
//返回值:0,退出成功;
//       1,退出失败
u8 esp_disconnect_wifi(void)
{
	return esp_12F_send_cmd("AT+CWQAP","OK",100);
}

//esp_12F模块的连接状态
//返回值:1，未连接;
//		2,已连接wifi并获得ip
//      3,已建立TCP\UDP连接
//		4,无TCP\UDP连接
//      5,未连接wifi
u8 esp_12F_apsta_check(void)
{
	u8 *p=NULL;
//	if(esp_12F_quit_trans())return 0;			//退出透传 
	esp_12F_send_cmd("AT+CIPSTATUS",":",100);	//查询连接状态
	p=esp_12F_check_cmd("STATUS:");
	wifiUSART_RX_STA=0;
	if(p)return p[7];
	else return 1;
}
/*{
	u8 *p; 
	esp_12F_send_cmd("AT+CIPSTATUS","OK",20);	//获取WIFI STA连接状态
	p=esp_12F_check_cmd("+CIPSTATUS:");
	return p[12];
}*/
//获取esp_12F模块的连接状态
//返回值:0,wifi连接正常;
//      1,wifi未连接.
u8 esp_12F_consta_check(void)
{
	u8 res=0;
	u8 *p=NULL;
	esp_12F_send_cmd("AT+CWJAP?","OK",50);		//查询连接状态 无连接NO AP,其他：+ CWJAP:<ssid>,<bssid>,<channel>,<rssi> 
	p=esp_12F_check_cmd("+CWJAP:"); 
	wifiUSART_RX_STA=0;
	if(p[0]!='+') res=1; 
	return res;
}

//获取本地IP地址
//ipbuf:ip地址
void esp_12F_get_staip(u8* ipbuf)
{
	u8 *p=NULL,*p1=NULL;
	if(esp_12F_send_cmd("AT+CIPSTA?","OK",50))//获取IP地址失败
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

//配置启用ap   启用同时wifi会关闭
void esp_12F_ap_config(void)
{
	u8 *p=NULL;
	
	//p=mymalloc(SRAMIN,32);							//申请32字节内存
	
	printf("ssid:%s passwd:%s ch:%d ecn:%s max_link:%d\r\n",ap_ssid,ap_password,ap_channel,esp_12F_ECN[ap_encryption],ap_max_conn);
	
	while(esp_12F_send_cmd("AT+CWMODE=2","OK",100));		//设置WIFI AP模式

	esp_12F_send_cmd("AT+CWDHCP_DEF=0,1","OK",50);		//设置AP使用DHCP开
	esp_12F_send_cmd("AT+CWDHCPS_DEF=0","OK",50);		//DHCP服务器DNS设置，默认DNS:192.168.4.1
	
	//WIFI AP模式模块对外的WIFI网络名称/密码/信道/加密模式/允许连接数
	sprintf((char*)p,"AT+CWSAP_DEF=\"%s\",\"%s\",%d,%d,%d",ap_ssid,ap_password,ap_channel,ap_encryption,ap_max_conn);
	esp_12F_send_cmd(p,"OK",100);					//配置AP参数
	
	wifiUSART_RX_STA=0;
	
	//myfree(SRAMIN,p);		//释放内存 
}

//esp_12F模块信息
void esp_12F_msg(void)
{
	u8 *p=NULL,*p1=NULL,*p2=NULL;
	//p=mymalloc(SRAMIN,32);							//申请32字节内存
	//p1=mymalloc(SRAMIN,32);							//申请32字节内存
	//p2=mymalloc(SRAMIN,32);							//申请32字节内存

	esp_12F_send_cmd("AT+GMR","OK",20);		//获取固件版本号
	p=esp_12F_check_cmd("SDK version:");    
	p1=(u8*)strstr((const char*)(p+1),"\r\n");
	*p1=0;
	printf("固件版本:%s\r\n",p);
	esp_12F_send_cmd("AT+CWMODE?","+CWMODE:",20);	//获取网络模式
	p=esp_12F_check_cmd(":");
	printf("网络模式:%s\r\n",(u8*)esp_12F_NETMODE_TBL[*(p+1)-'1']);
	esp_12F_send_cmd("AT+CWSAP?","+CWSAP:",100);	//获取ap配置
	p=esp_12F_check_cmd("\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	p2=p1;
	*p1=0;
	printf("AP_SSID号:%s\r\n",p+1);
	p=(u8*)strstr((const char*)(p2+1),"\"");
	p1=(u8*)strstr((const char*)(p+1),"\"");
	p2=p1;
	*p1=0;
	delay_ms(1);
	printf("AP_密码:%s\r\n",p+1);
	p=(u8*)strstr((const char*)(p2+1),",");
	p1=(u8*)strstr((const char*)(p+1),",");
	*p1=0;
	printf("通道号:%s\r\n",p+1);
	printf("加密方式:%s\r\n",(u8*)esp_12F_ECN[*(p1+1)-'0']);
	wifiUSART_RX_STA=0;
	
	//myfree(SRAMIN,p);		//释放内存 
	//myfree(SRAMIN,p1);		//释放内存 
	//myfree(SRAMIN,p2);		//释放内存 
}

//初始化智能配网功能
void set_ESP(void)
{
	esp_12F_send_cmd("AT+CWMODE=1","OK",100);   //配置WiFi模组工作模式为单STA模式
	esp_12F_send_cmd("AT+CWAUTOCONN=1","OK",100);   //使能上电自动连接AP
	delay_ms(100);
	esp_12F_send_cmd("AT+CWSTARTSMART=3","OK",100); //支持ESP-Touch和Airkiss智能配网
}
//退出智能配网
void stop_ESP(void)
{
	esp_12F_send_cmd("AT+CWSTOPSMART","OK",100);
	wifiUSART_RX_STA=0;
}
//智能配网功能 10分钟不成功则失败
//返回值：0 成功
//		1 失败
u8 wifi_ESP(void)
{
	u16 i=0;
ESP:	
	set_ESP();
	printf("等待smartconfig 配置wifi账号密码\r\n");
	while(!(esp_12F_check_cmd("WIFI CONNECT")
			||esp_12F_check_cmd("WIFI GOT IP")))
	{
		wifiUSART_RX_STA=0;
		delay_ms(10);
		i++;
		if(i%6000==0)
		{
			printf("等待smartconfig 配置wifi账号密码\r\n");
			stop_ESP();
			goto ESP;
		}
		if(i>60000)
		{
			printf("smartconfig失败\r\n");
			stop_ESP();
			i=0;
			return 1;
		}
	}
	printf("smartconfig成功\r\n");
	stop_ESP();
	return 0;
}

//更改tcp服务器端口
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

//检查wifi ssid是否存在
//返回值:0,wifi存在
//    其他,没有这个wifi
u8* chech_ssid(u8* ssid)
{
	esp_12F_send_cmd("AT+CWLAP","+CWLAP:",5000);
	while(esp_12F_check_cmd("OK")){delay_ms(10);wifiUSART_RX_STA=0;};
//	esp_12F_at_response(1);  //输出扫描到的信息wifi
	return esp_12F_check_cmd(ssid);
}
