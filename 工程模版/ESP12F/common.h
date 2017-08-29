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

typedef	enum					//ģ�鹤��ģʽ
{
	Error                =     0,
	Station				 =     1,
	SoftAp 				 =     2,
	StationAndSoftAp  	 =     3,	
}WifiMode_t;

typedef enum					//wifi����ģʽ softAPʹ��
{
	Open                 =     0,
	WPA_PSK              =     2,
	WPA2_PSK             =     3,
	WPA_WPA2_PSK         =     4,
}WifiEncryptionType_t;

typedef struct						//TCP�������
{
	WifiStatus_t		   status;
	u8                     LinkId;
	char                   Type[4];
	char                   RemoteIp[17];
	u16                    RemotePort;
	u16                    LocalPort;
	u8                   RunAsServer;    
}WifiConnection_t;
//###################################################################################################
typedef struct
{
	//----------------����
	u8                     	 LinkId;				//�������ݵ��豸ID
	u8                       Rxdata[1461];			//���ݻ���
	u16                      RxDataLen;				//���ݳ���
	u8                     	 RxIsData;  
	u8                       GotNewData;
	WifiStatus_t		    status;					//����״̬
	
	//----------------����			
	WifiMode_t               Mode;
	char                     MyIP[16];	
	char                     MyGateWay[16];
	//----------------Station
	u8                     StationDhcp;
	char                     StationIp[16];	
	//----------------TcpIp
	u8                     TcpIpMultiConnection;
	u16                      TcpIpPingAnswer;
	WifiConnection_t         TcpIpConnections[5];        //TCP����5������
	//----------------
}Wifi_t;

u8 esp_12F_init(void);

void esp_12F_at_response(u8 mode);        //��ӡlog��Ϣ
/////ATָ������
u8* esp_12F_check_cmd(u8 *str);			  //at������Ӧ���
u8 esp_12F_send_cmd(u8 *cmd,u8 *ack,u16 waittime);//AT�����

/////���ܺ���
u8 esp_12F_factory(void);                //�ָ���������
u8 esp_12F_reboot(void);				//����ģ��
u8 esp_12F_quit_trans(void);			  //�˳�͸��ģʽ
u8 esp_disconnect_wifi(void);			 //�Ͽ�wifi
u8 esp_12F_apsta_check(void);			  //����״̬��ѯ
u8 esp_12F_consta_check(void);			  //wifi����״̬
void esp_12F_get_staip(void);        //��ȡ����ip

u8 tcp_send(u8 id,u8* data,u8 len);		//TCP������Ϣ

//���wifi ssid�Ƿ����
u8* chech_ssid(u8* ssid);

void esp_12F_ap_config(void);			//����AP
void esp_12F_msg(void);					  //ģ��̼���Ϣ

u8 esp_12F_sta_link_wifi(const u8* ssid,const u8* password);            //����wifi
u8 esp_12F_wifi_disconnect(void);										//�Ͽ�wifi
u8 esp_12F_setlink_mode(u8 netpro,const u8* ipbuf,const u8* portnum);	//��������TCP/udp/server
u8 Wifi_TcpServer_Disable(void);						//�ر�TCP����
u8 Wifi_TcpIp_Close(u8 LinkId);							//�ر�TCP����

//��������
u8 wifi_callback(void);

//������������
u8 wifi_ESP(void);
void set_ESP(void); 	//��ʼ������������
void stop_ESP(void);	//�˳���������

//�û����ò���
extern const u8* portnum;		//���Ӷ˿�
 
extern u8* sta_ssid;		//WIFI STA SSID
extern u8* sta_encryption;	//WIFI STA ���ܷ�ʽ
extern u8* sta_password; 	//WIFI STA ����

extern const u8* ap_ssid;		//WIFI AP SSID
extern const u8  ap_encryption;	//WIFI AP ���ܷ�ʽ
extern const u8* ap_password; 	//WIFI AP ����
extern const u8 ap_channel; 	//�ŵ�
extern const u8 ap_max_conn; 	//��������� ��Χ[1,4]

extern u8* kbd_fn_tbl[2];
extern const u8* esp_12F_NETMODE_TBL[3];
extern const u8* esp_12F_WORKMODE_TBL[3];
extern const u8* esp_12F_ECN[5];

extern Wifi_t wifi;
#endif
