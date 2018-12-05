#pragma once
/* 缓冲区最大值 */
#define MAXSIZE 8192

/* 路径长度最大值 */
#define MAXFILEPATH 120

/*
set_ip:   client连接的ip地址
set_port: client连接的端口
*/
char set_ip[20] = "127.0.0.1"; //默认值
unsigned int set_port = 21; //默认值

/*
client_port_socket: PORT模式监听socket
client_data_socket: PORT模式接受server的连接请求后建立的数据连接socket
client_pasv_socket: PASV模式socket
sockfd: 客户端最初建立的socket
*/
int client_port_socket = -1;
int client_data_socket = -1;
int client_pasv_socket = -1;
int sockfd = -1;

/* PASV模式client的通信地址 */
struct sockaddr_in pasvaddr;

/*
	MODE模式：默认MDEFAULT
	MDEFAULT：默认模式，最初建立的通信连接
	MPORT：PORT模式
	MPASV：PASV模式
*/
typedef enum { 
	MDEFAULT, 
	MPORT, 
	MPASV 
} MODE;
MODE mode = MDEFAULT;

/* client是否一条recv接收了server多条消息 */
int multi_receive = -1;

/* client是否准备断点下载 */
int rest_flag = 0;

/* 断点上传偏移量 */
long appe_offset = 0; //默认值