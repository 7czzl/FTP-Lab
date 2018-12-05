#pragma once
/* ���������ֵ */
#define MAXSIZE 8192

/* ·���������ֵ */
#define MAXFILEPATH 120

/*
set_ip:   client���ӵ�ip��ַ
set_port: client���ӵĶ˿�
*/
char set_ip[20] = "127.0.0.1"; //Ĭ��ֵ
unsigned int set_port = 21; //Ĭ��ֵ

/*
client_port_socket: PORTģʽ����socket
client_data_socket: PORTģʽ����server�����������������������socket
client_pasv_socket: PASVģʽsocket
sockfd: �ͻ������������socket
*/
int client_port_socket = -1;
int client_data_socket = -1;
int client_pasv_socket = -1;
int sockfd = -1;

/* PASVģʽclient��ͨ�ŵ�ַ */
struct sockaddr_in pasvaddr;

/*
	MODEģʽ��Ĭ��MDEFAULT
	MDEFAULT��Ĭ��ģʽ�����������ͨ������
	MPORT��PORTģʽ
	MPASV��PASVģʽ
*/
typedef enum { 
	MDEFAULT, 
	MPORT, 
	MPASV 
} MODE;
MODE mode = MDEFAULT;

/* client�Ƿ�һ��recv������server������Ϣ */
int multi_receive = -1;

/* client�Ƿ�׼���ϵ����� */
int rest_flag = 0;

/* �ϵ��ϴ�ƫ���� */
long appe_offset = 0; //Ĭ��ֵ