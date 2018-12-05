#pragma once
/* 缓冲区最大值 */
#define MAXSIZE 8192
#define MAXMSG 2048

/* 路径长度最大值 */
#define MAXFILEPATH 120

/* 路径数量最大值 */
#define MAXPATHNUM 20

/* server返回信息 */
const char *msgLogin = "Please login!\r\n\
	You can use cmd <USER anonymous> try!\r\n";
const char *msg150 = "150 Opening BINARY mode data connection\r\n";
const char *msg2001 = "200 Type set to I.\r\n";
const char *msg2002 = "200 PORT command successful.\r\n";
const char *msg215 = "215 UNIX Type: L8\r\n";
const char *msg220 = "220 Zhouzl's FTP server ready.Input anything to see what you should do next.\r\n";
const char *msg221 = "221 Goodbye.\r\n";
const char *msg226 = "226 Transfer complete.\r\n";
const char *msg227 = "227 Entering Passive Mode (";
const char *msg230 = "230-\r\n\
230-Anonymous,Welcome to\r\n\
230-Zhouzl's\r\n\
230-simple FTP server\r\n\
230-Guest login ok!\r\n\
230 You can use cmd <HELP> to see what you can do.\r\n";
const char *msg250 = "250 Create file folder, Okay\r\n";
const char *msg2501 = "250 Change the path, Okay.\r\n";
const char *msg2502 = "250 Delete file folder, Okay.\r\n";
const char *msg2503 = "250 get last filename, Okay. Now you can use <RNTO>\r\n";
const char *msg2504 = "250 rename file, Okay.\r\n";
const char *msg2505 = "250 get file_offset, Okay.\r\n";
const char *msg331 = "331 User name ok, input your e-mail address as password.\r\n";
const char *msg425 = "425 No TCP connection was established.\r\n";
const char *msg426 = "426 connection was broken or network failure.\r\n";
const char *msg451 = "451 Error when reading the file from disk.\r\n";
const char *msg500 = "500 error command. Input <HELP/?> to get info.\r\n";
const char *msg5001 = "500 error PORT cmd, lack parameter like <127,0,0,1,125,2>\r\n";
const char *msg5002 = "500 error file cmd, lack parameter <filename>\r\n";
const char *msg5003 = "500 error path cmd, lack parameter <filepath>\r\n";
const char *msg5004 = "500 error path cmd, lack parameter <fileoffset>\r\n";
const char *msg503 = "503 No mode is estabished(PORT/PASV).\r\n";
const char *msg504 = "504 Wrong TYPE command.\r\n";
const char *msg5301 = "530 the command is invalid, try to use <USER your_name>.\r\n";
const char *msg5302 = "530 All users other than anonymous are not supported, please input again.\r\n";
const char *msg5303 = "530 the command is invalid, try to use <PASS your_pwd>.\r\n";
const char *msg5304 = "530 password is invalid, please set your e-mail address as password.\r\n";
const char *msg5305 = "530 Error save file\r\n";
const char *msg5306 = "530 Error transfer file\r\n";
const char *msg5307 = "530 Please use <RNFR> first.\r\n";
const char *msg550 = "550 error.\r\n";
const char *msg5501 = "550 the file has existed.\r\n";
const char *msg5502 = "550 the path does not existed.\r\n";
const char *msg5503 = "550 the file folder is not blank.\r\n";
const char *msg5504 = "550 you can not change the path so many times.\r\n";
const char *msg5505 = "550 last path does not existed.\r\n";
const char *msg551 = "551 No such file or dictionary.\r\n";
const char *msg552 = "552 the server had trouble saving the file to disk.\r\n";
const char *msgHelp = "\r\n\
cmd guide\r\n\
.............................\r\n\
Single cmd:\r\n\
  <SYST> <TYPE> <QUIT/ABOR> <LIST> <PASV> <PWD> <HELP/?>\r\n\
Parse cmd:\r\n\
  <MKD filepath> to create a blank file folder in the specify path.\r\n\
  <CWD filepath> to change current path.\r\n\
  <RMD filepath> to delete the blank file folderin the specify path.\r\n\
  <PORT 166,111,80,233,128,2> to build socket in the ip and port.\r\n\
  <STOR/RETR filename> to upload or download file.\r\n\
  <RNFR/RNTO filename> to rename file.\r\n\
............................\r\n";



/* 
    set_path: server工作路径
    set_port: server端口
*/
char set_path[MAXFILEPATH] = "/tmp"; //默认值
//char last_path[MAXPATHNUM][MAXFILEPATH];
//int path_num = -1;
unsigned int set_port = 21; //默认值

/* 
    server_pasv_socket： PASV模式监听socket 
    server_data_socket： PASV模式accept客户端后建立的数据连接socket
    server_port_socket： PORT模式socket
*/
int server_pasv_socket = -1;
int server_data_socket = -1;
int server_port_socket = -1;

/* 
    listenfd: server最初建立的监听socket
    connfd: server最初accept客户端后建立的连接socket
*/
int listenfd = -1;
int connfd = -1;

/* PORT模式server的通信地址 */
struct sockaddr_in portaddr;

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

/* PASV模式中的随机端口 */
unsigned int rand_port = 30000;

/* 重命名文件路径 */
char last_name_path[MAXFILEPATH]; //默认值
int RN_flag = 0; //默认值

/* 断点下载文件偏移 */
long rest_offset = 0; //默认值

/* 是否准备断点上传 */
int appe_flag = 0;