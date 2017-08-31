#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"		
#include "delay.h"	  
#include "malloc.h"
#include "string.h"    
#include "wifiusart.h"
#include "usart.h"
#include "timer.h"
#include "dma.h"

#define udp 0
#define tcp 1
#define server 2

typedef enum
{
	Wifi_Error			=     0, 
	Wifi_Connected      =     2,
	TCP_Connected  		=     3,
	TCP_Disconnected	=     4,
	Wifi_ConnectionFail	=     5,
}WifiStatus_t;

typedef	enum					//模块工作模式
{
	Error                =     0,
	Station				 =     1,
	SoftAp 				 =     2,
	StationAndSoftAp  	 =     3,	
}WifiMode_t;

typedef enum					//wifi加密模式 softAP使用
{
	Open                 =     0,
	WPA_PSK              =     2,
	WPA2_PSK             =     3,
	WPA_WPA2_PSK         =     4,
}WifiEncryptionType_t;

typedef struct
{
	WifiStatus_t		   status;
	char                   RemoteIp[17];
	u16                    RemotePort;
	u16                    LocalPort;  
}WifiConnection_t;
//###################################################################################################
typedef struct
{
	//----------------变量
	u8                     	 LinkId;				//发送数据的设备ID
	u8                       Rxdata[1461];			//数据缓冲
	u16                      RxDataLen;				//数据长度
	u8                     	 RxIsData;  
	u8											 RxTimes;
	u8                       GotNewData;
	WifiStatus_t		     status;				//网络状态	
	//----------------参数			
	WifiMode_t               Mode;
	char                     MyIP[16];	
	char                     MyGateWay[16];
	//----------------Tcp设备
	WifiConnection_t         TcpIpConnections[5];    //5个连接TCP客户端信息
	//----------------
}Wifi_t;

u8 esp_12F_init(void);

void esp_12F_at_response(u8 mode);        //打印log信息
/////AT指令驱动
u8* esp_12F_check_cmd(u8 *str);			  //at命令响应检测
u8 esp_12F_send_cmd(u8 *cmd,u8 *ack,u16 waittime);//AT命令发送

/////功能函数
u8 esp_12F_factory(void);                //恢复出厂设置
u8 esp_12F_reboot(void);				//重启模块
u8 esp_12F_quit_trans(void);			  //退出透传模式
u8 esp_disconnect_wifi(void);			 //断开wifi
u8 esp_12F_apsta_check(void);			  //网络状态查询
u8 esp_12F_consta_check(void);			  //wifi连接状态
void esp_12F_get_staip(void);        //获取本地ip

u8 tcp_send(u8 id,u8* data,u8 len);		//TCP发送信息

//检查wifi ssid是否存在
u8* chech_ssid(u8* ssid);

void esp_12F_ap_config(void);			//配置AP
void esp_12F_msg(void);					  //模块固件信息

u8 esp_12F_sta_link_wifi(const u8* ssid,const u8* password);            //连接wifi
u8 esp_12F_wifi_disconnect(void);										//断开wifi
u8 esp_12F_setlink_mode(u8 netpro,const u8* ipbuf,const u8* portnum);	//设置连接TCP/udp/server
u8 Wifi_TcpServer_Disable(void);						//关闭TCP服务
u8 Wifi_TcpIp_Close(u8 LinkId);							//关闭TCP连接

//接收数据
u8 wifi_callback(void);

//智能配网功能
u8 wifi_ESP(void);
void set_ESP(void); 	//初始化智能配网功
void stop_ESP(void);	//退出智能配网

//用户配置参数
extern const u8* portnum;		//连接端口
 
extern u8* sta_ssid;		//WIFI STA SSID
extern u8* sta_encryption;	//WIFI STA 加密方式
extern u8* sta_password; 	//WIFI STA 密码

extern const u8* ap_ssid;		//WIFI AP SSID
extern const u8  ap_encryption;	//WIFI AP 加密方式
extern const u8* ap_password; 	//WIFI AP 密码
extern const u8 ap_channel; 	//信道
extern const u8 ap_max_conn; 	//最大连接数 范围[1,4]

extern u8* kbd_fn_tbl[2];
extern const u8* esp_12F_NETMODE_TBL[3];
extern const u8* esp_12F_WORKMODE_TBL[3];
extern const u8* esp_12F_ECN[5];

extern Wifi_t wifi;
#endif
