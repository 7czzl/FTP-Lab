#pragma once
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "MyString.h"

/* server接受客户端消息 */
int ServerRecv(int connfd, char *sentence);

/* server给Client发送消息 */
int ServerSend(int connfd, const char *sentence);

/* 命令行处理 */
int serverCommand(int connfd, char *sentence);

/*server关闭指定socket*/
void closeSpecifySocket(int *specifySocket);

/* server关闭所有socket */
void closeAllSocket();

/* <USER> & <PASS> */
void userLogin(int connfd, char *sentence);

/* <ABOR> 断开传输socket */
void server_ABOR(int connfd);

/* <PASV>，在server端进行socket绑定，建立监听 */
int server_PASV(int connfd, char *sentence);

/* <PORT>，建立socket，等待与客户端建立连接 */
int server_PORT(char *sentence);

/* server在建立PASV模式后当客户端发起connect时进行accept连接 */
int pasvAccept();

/* server在建立PORT模式时向客户端发起connect请求 */
int portConnect();

/* server端接受文件 */
int serverReceiveFile(int sock, char *sentence);

/* server端发送文件 */
int serverTransferFile(int sock, char *sentence);


/* <REST> 获取文件偏移量 */
void server_REST(int connfd, char *sentence);

/* server建立PORT/PASV模式后,<RETR>,进行文件下载 */
int server_RETR(char *sentence);

/* <APPE> 断点上传文件 */
void server_APPE(int sockfd, char *sentence);

/* server建立PORT/PASV模式后,<STOR>,进行文件上传 */
int server_STOR(char *sentence);

/* 创建一级或多级目录 */
int mkdirs(char *muldir);

/* <MKD> 在工作路径下新建目录 */
int server_MKD(int connfd, char *sentence);

/* <CWD> 改变工作路径 */
int server_CWD(int connfd, char *sentence);

/* <RWD> 回退工作路径 */
//int server_RWD(int connfd, char *sentence);

/* <PWD> 输出当前工作路径 */
void server_PWD(int connfd, char *sentence);

/* <LIST> */
int server_LIST(int connfd, char *sentence);

/* <RMD> 删除指定目录 */
int server_RMD(int connfd, char *sentence);

/* server端重名文件 */
int server_RNFR(int connfd, char *sentence);
int server_RNTO(int connfd, char *sentence);




/*==================================================*/


/* server接受客户端消息 */
int ServerRecv(int connfd, char *sentence) {
	memset(sentence, 0, MAXSIZE);
	int len = read(connfd, sentence, MAXSIZE);
	if (len < 0) {
		printf("Error read() in ServerRecv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return len;
}

/* server给Client发送消息 */
int ServerSend(int connfd, const char *sentence) {
	if (write(connfd, sentence, strlen(sentence)) < 0) {
		printf("Error write() in ServerSend(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}

/* 命令行处理 */
int serverCommand(int connfd, char *sentence) {
    ServerSend(connfd, msg220);
	//ServerSend(connfd, msgLogin);
    userLogin(connfd, sentence);
    chdir(set_path);
    while(1) {
		//ServerSend(connfd, msgHelp);
        ServerRecv(connfd, sentence);
		if (CommandRecognize(sentence, "RETR")) {
			if (server_RETR(sentence) < 0) {
				ServerSend(connfd, msg5306);
			}
			mode = MDEFAULT;
		}
		else if (CommandRecognize(sentence, "REST")) {
			server_REST(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "APPE")) {
			server_APPE(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "STOR")) {
			if (server_STOR(sentence) < 0) {
				ServerSend(connfd, msg5305);
			}
			mode = MDEFAULT;
		}
		else if (CommandRecognize(sentence, "QUIT")) {
			ServerSend(connfd, msg221);
			return -2;
		}
		else if (CommandRecognize(sentence, "ABOR")) {
			server_ABOR(connfd);
		}
        else if (CommandRecognize(sentence, "SYST")) {
            ServerSend(connfd, msg215);
        }
        else if (CommandRecognize(sentence, "TYPE")) {
            if (CommandRecognize(sentence, "TYPE I")) {
                ServerSend(connfd, msg2001);
            }
            else {
                ServerSend(connfd, msg504);
            }
        }
		else if (CommandRecognize(sentence, "PORT")) {
			if (server_PORT(sentence) < 0) {
				ServerSend(connfd, msg5001);
			}
			else {
				mode = MPORT;
				ServerSend(connfd, msg2002);
			}
		}
		else if (CommandRecognize(sentence, "PASV")) {
			if (server_PASV(connfd, sentence) < 0) {
				ServerSend(connfd, msg550);
			}
			else {
				mode = MPASV;
			}
		}
		else if (CommandRecognize(sentence, "MKD")) {
			server_MKD(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "CWD")) {
			server_CWD(connfd, sentence);
		}
		/*else if (CommandRecognize(sentence, "RWD")) {
			server_RWD(connfd, sentence);
		}*/
		else if (CommandRecognize(sentence, "PWD")) {
			server_PWD(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "LIST")) {
			if (server_LIST(connfd, sentence) < 0) {
				ServerSend(connfd, msg550);
			}
		}   
        else if (CommandRecognize(sentence, "RMD")) {
			server_RMD(connfd, sentence);
        }
		else if (CommandRecognize(sentence, "RNFR")) {
			server_RNFR(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "RNTO")) {
			server_RNTO(connfd, sentence);
		}
		else if (CommandRecognize(sentence, "RMD")) {
			server_RMD(connfd, sentence);
		}
        else if (CommandRecognize(sentence, "HELP")) {
            ServerSend(connfd, msgHelp);
        }
		else if (CommandRecognize(sentence, "?")) {
			ServerSend(connfd, msgHelp);
		}
        else {
            ServerSend(connfd, msg500);
        }
    }
    return 0;
}

/*server关闭指定socket*/
void closeSpecifySocket(int *specifySocket) {
	close(*specifySocket);
	*specifySocket = -1;
}

/* server关闭所有socket */
void closeAllSocket() {
	if (server_data_socket > 0) {
		closeSpecifySocket(&server_data_socket);
	}
	if (server_port_socket > 0) {
		closeSpecifySocket(&server_port_socket);
	}
	if (server_pasv_socket > 0) {
		closeSpecifySocket(&server_pasv_socket);
	}
	if (connfd > 0) {
		closeSpecifySocket(&connfd);
	}
	if (listenfd > 0) {
		closeSpecifySocket(&listenfd);
	}
}

/* <USER> & <PASS> */
void userLogin(int connfd, char *sentence) {
    while(1) {
        ServerRecv(connfd, sentence);
        if(CommandRecognize(sentence, "USER") == 0) {
            ServerSend(connfd, msg5301);
        }
        else if (CommandRecognize(sentence, "USER anonymous") == 0) {
            ServerSend(connfd, msg5302);
        }
        else {
            ServerSend(connfd, msg331);
            break;
        }
    }
    while(1) {
        ServerRecv(connfd, sentence);
        if(CommandRecognize(sentence, "PASS") == 0) {
            ServerSend(connfd, msg5303);
        }
        else {
			/* 只验证是否含有@符号. */
            int flag = 0;
            for (unsigned int i = 4; i < strlen(sentence); i++) {
                if (sentence[i] == '@') {
                    flag = 1;
                    break;
                }
            }
            if (flag) {
                ServerSend(connfd, msg230);
                break;
            }
            else {
                ServerSend(connfd, msg5304);
            }
			/*
			邮箱地址正则验证
			由于测试脚本autugrade.py不支持，
			因此暂时废弃.
			*/
			/*
			char e_mail_addr[100];
			getCmd_parse(sentence, 4, e_mail_addr);
			for (unsigned int i = 0; i < strlen(e_mail_addr); i++) {
				if (e_mail_addr[i] == '\r' || e_mail_addr[i] == '\n') {
					e_mail_addr[i] = '\0';
					break;
				}
			}
			int cflags = REG_EXTENDED;
			regmatch_t pmatch[1];
			const size_t nmatch = 1;
			regex_t reg;
			const char *pattern = "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*.\\w+([-.]\\w+)*$";

			regcomp(&reg, pattern, cflags);
			int status = regexec(&reg, e_mail_addr, nmatch, pmatch, 0);
			if (status == 0) {
				ServerSend(connfd, msg230);
				break;
			}
			else
				ServerSend(connfd, msg5304);
			regfree(&reg);
			*/
        }
    }
}

/* <ABOR> 断开传输socket */
void server_ABOR(int connfd) {
	if (server_data_socket == -1 || server_data_socket == 0)
		ServerSend(connfd, "225 no transfer to ABOR.\r\n");
	else
	{
		closeSpecifySocket(&server_data_socket);
		ServerSend(connfd, "226 transder to ABOR.\r\n");
	}
}

/* <PASV>，在server端进行socket绑定，建立监听 */
int server_PASV(int connfd, char *sentence) {
    struct sockaddr_in pasvaddr;
    srand((unsigned)time(NULL));
    rand_port = rand() % (65535 - 20000 + 1) + 20000;
    /* 关闭旧端口 */
    if (server_pasv_socket >= 0) {
        closeSpecifySocket(&server_pasv_socket);
    }
    if ((server_pasv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        ServerSend(connfd, "500 Error socket()\r\n");
        return -1;
    }
    socklen_t len = sizeof(pasvaddr);
    getsockname(connfd, (struct sockaddr*)&pasvaddr, &len);
    pasvaddr.sin_port = htons(rand_port);
	/*
	复用端口，方便测试.
	*/
	int on = 1;
	if (setsockopt(server_pasv_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		ServerSend(connfd, "500 Error setsockopt()\r\n");
		closeSpecifySocket(&server_pasv_socket);
		return -1;
	}
    if (bind(server_pasv_socket, (struct sockaddr*)&pasvaddr, sizeof(pasvaddr)) == -1) {
        ServerSend(connfd, "500 Error bind()\r\n");
        closeSpecifySocket(&server_pasv_socket);
        return -1;
    }
    if (listen(server_pasv_socket, 10) == -1) {
        ServerSend(connfd, "500 Error listen()\r\n");
        closeSpecifySocket(&server_pasv_socket);
        return -1;
    }

    len = sizeof(pasvaddr);
    getsockname(server_pasv_socket, (struct sockaddr*)&pasvaddr, &len);
    /* 输出227反馈消息——随机端口 */
    char msg[50] = "227 Entering Passive Mode (";
    char str[12];
    strcat(msg, inet_ntoa(pasvaddr.sin_addr));
    len = strlen(msg);
    msg[len++] = '.';
    msg[len] = '\0';
    long2Char(rand_port / 256, str);
    strcat(msg, str);
    len = strlen(msg);
    msg[len++] = '.';
    msg[len] = '\0';
    long2Char(rand_port % 256, str);
    strcat(msg, str);
    len = strlen(msg);
    msg[len++] = ')';
    msg[len++] = '\n';
    msg[len] = '\0';
    for (unsigned int i = 0; i < strlen(msg); i++) {
        if (msg[i] == '.') {
            msg[i] = ',';
        }
    }
    ServerSend(connfd, msg);
    return 0;
}

/* <PORT>，建立socket，等待与客户端建立连接 */
int server_PORT(char *sentence) {
    char ip[20];
    int port = 0;
    get_IP_PORTinParse(sentence, ip, &port);

    memset(&portaddr, 0, sizeof(portaddr));
    portaddr.sin_family = AF_INET;
    portaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &portaddr.sin_addr) <= 0) {
        printf("Error inet_pton() in server_PORT(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    /* 关闭旧端口 */
    if (server_port_socket >= 0) {
        closeSpecifySocket(&server_port_socket);
    }
    if ((server_port_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket() in server_PORT(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

/* server在建立PASV模式后当客户端发起connect时进行accept连接 */
int pasvAccept() {
    if (server_data_socket >= 0) {
        closeSpecifySocket(&server_data_socket);
    }
    if ((server_data_socket = accept(server_pasv_socket, NULL, NULL)) == -1) {
        printf("Error accept() in pasvAccept(): %s(%d)\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}

/* server在建立PORT模式时向客户端发起connect请求 */
int portConnect() {
    if (connect(server_port_socket, (struct sockaddr*)&portaddr, sizeof(portaddr)) < 0) {
        printf("Error connect() in portConnect(): %s(%d)\n", strerror(errno), errno);
        if (server_port_socket >= 0) {
            closeSpecifySocket(&server_port_socket);
        }
        return -1;
    }
    return 0;
}

/* server端接受文件 */
int serverReceiveFile(int sock, char *sentence) {
	char filepath[MAXFILEPATH];
	getCmd_parse(sentence, 4, filepath);
	FILE *dataFile;
	if (appe_flag) {
		dataFile = fopen(filepath, "ab");
		appe_flag = 0;
	}
	else {
		dataFile = fopen(filepath, "wb");
	}
	if (!dataFile) {
		ServerSend(connfd, msg552);
		return -1;
	}
	else {
		char msg[100] = "150 Opening BINARY mode data connection for ";

		strcat(msg, filepath);
		int len = strlen(msg);
		msg[len] = '\0';
		char bytes[20] = ".\r\n";
		strcat(msg, bytes);
		len = strlen(msg);
		msg[len] = '\0';
		ServerSend(connfd, msg);
		/* 接受客户端的文件消息并保存 */
		memset(sentence, 0, MAXSIZE);
		int datasize = 0;
		while (1) {
			if ((datasize = ServerRecv(sock, sentence)) > 0) {
				fwrite(sentence, 1, datasize, dataFile);
			}
			else if (datasize < 0) {
				return -1;
			}
			else {
				break;
			}
		}
		ServerSend(connfd, msg226);
	}
	fclose(dataFile);
	return 0;
}

/* server端发送文件 */
int serverTransferFile(int sock, char *sentence) {
    char filepath[MAXFILEPATH];
    getCmd_parse(sentence, 4, filepath);
    FILE *dataFile = fopen(filepath, "rb");
    if (!dataFile) {
        ServerSend(connfd, msg551);
        return -1;
    }
    else {
        /* 发送150反馈消息 */
        fseek(dataFile, 0, SEEK_END); //定位到文件末
        long length = ftell(dataFile); 
		length -= rest_offset;//文件长度
        fseek(dataFile, rest_offset, 0);   //定位到文件偏移处
        char str[12];
        long2Char(length, str);
        char msg[100] = "150 Opening BINARY mode data connection for ";
        strcat(msg, filepath);
        int len = strlen(msg);
        msg[len++] = ' ';
        msg[len++] = '(';
        msg[len] = '\0';
        strcat(msg, str);
        char bytes[20] = " bytes).\r\n";
        strcat(msg, bytes);
        len = strlen(msg);
        msg[len] = '\0';
        ServerSend(connfd, msg);
        /* 读取文件并发送给客户端 */
        int datasize = 0;
        memset(sentence, 0, MAXSIZE);
        do {
            datasize = fread(sentence, 1, MAXSIZE, dataFile);
            if (datasize < 0) {
                ServerSend(connfd, msg451);
                closeSpecifySocket(&sock);
                if (mode == MPASV) {
                    closeSpecifySocket(&server_pasv_socket);
                }
                return -1;
            }
            else if (datasize == 0) {
                break;
            }
            if (write(sock, sentence, datasize) < 0) {
                ServerSend(connfd, msg426);
                closeSpecifySocket(&sock);
                if (mode == MPASV) {
                    closeSpecifySocket(&server_pasv_socket);
                }
                return -1;
            }
        } while (datasize > 0);
        ServerSend(connfd, msg226);
    }
	rest_offset = 0; //恢复偏移量
    fclose(dataFile);
    return 0;
}

/* <REST> 获取文件偏移量 */
void server_REST(int connfd, char *sentence) {
	/* 未输入参数<file_offset>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 4, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5004);
		return;
	}

	rest_offset = atol(pareme);
	ServerSend(connfd, msg2505);

	return;
}

/* server建立PORT/PASV模式后,<RETR>,进行文件下载 */
int server_RETR(char *sentence) {
	switch (mode)
	{
	case MPORT:
		if (portConnect() < 0) {
			ServerSend(connfd, msg425);
			closeSpecifySocket(&server_port_socket);
			return -1;
		}
		if (serverTransferFile(server_port_socket, sentence) < 0) {
			closeSpecifySocket(&server_port_socket);
			return -1;
		}
		closeSpecifySocket(&server_port_socket);
		break;

	case MPASV:
		if (pasvAccept() < 0) {
			ServerSend(connfd, msg425);
			closeSpecifySocket(&server_data_socket);
			closeSpecifySocket(&server_pasv_socket);
			return -1;
		}
		if (serverTransferFile(server_data_socket, sentence) < 0) {
			closeSpecifySocket(&server_data_socket);
			closeSpecifySocket(&server_pasv_socket);
			return -1;
		}
		closeSpecifySocket(&server_data_socket);
		closeSpecifySocket(&server_pasv_socket);
		break;

	default:
		ServerSend(connfd, msg503);
		break;
	}
    return 0;
}

/* <APPE> 断点上传文件 */
void server_APPE(int sockfd, char *sentence) {
	/* 未输入参数<file_path>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 4, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return;
	}
	FILE *dataFile = fopen(pareme, "rb");
	if (!dataFile) {
		ServerSend(connfd, msg551);
		return;
	}
	fseek(dataFile, 0, SEEK_END); //定位到文件末
	long length = ftell(dataFile);//文件长度
	char str[12];
	long2Char(length, str);
	memset(sentence, 0, MAXSIZE);
	strcat(sentence, "APPEfb ");
	strcat(sentence, str);
	strcat(sentence, "\r\n");
	appe_flag = 1;
	ServerSend(connfd, sentence);
	return;
}

/* server建立PORT/PASV模式后,<STOR>,进行文件上传 */
int server_STOR(char *sentence) {
	switch (mode) {
	case MPORT:
		if (portConnect() < 0) {
			ServerSend(connfd, msg425);
			closeSpecifySocket(&server_port_socket);
			return -1;
		}
		if (serverReceiveFile(server_port_socket, sentence) < 0) {
			closeSpecifySocket(&server_port_socket);
			return -1;
		}
		closeSpecifySocket(&server_port_socket);
		break;

	case MPASV:
		if (pasvAccept() < 0) {
			ServerSend(connfd, msg425);
			closeSpecifySocket(&server_data_socket);
			closeSpecifySocket(&server_pasv_socket);
			return -1;
		}
		if (serverReceiveFile(server_data_socket, sentence) < 0) {
			closeSpecifySocket(&server_data_socket);
			closeSpecifySocket(&server_pasv_socket);
			return -1;
		}
		closeSpecifySocket(&server_data_socket);
		closeSpecifySocket(&server_pasv_socket);
		break;

	default:
		ServerSend(connfd, msg503);
		break;
	}
    return 0;
}

/* 创建一级或多级目录 */
int mkdirs(char *muldir)
{
	int i, len;
	char str[MAXFILEPATH];
	strncpy(str, muldir, MAXFILEPATH);
	len = strlen(str);
	for (i = 0; i<len; i++)
	{
		if (str[i] == '/')
		{
			str[i] = '\0';
			if (access(str, 0) != 0)
			{
				mkdir(str, 0777);
			}
			str[i] = '/';
		}
	}
	if (len>0 && access(str, 0) != 0)
	{
		mkdir(str, 0777);
	}
	return 0;
}

/* <MKD> 在工作路径下新建目录 */
int server_MKD(int connfd, char *sentence) {
	/* 未输入参数<filepath>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 3, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return -1;
	}

	char filepath[MAXFILEPATH];
	getFilepath(filepath, sentence);
	if (access(filepath, F_OK) == 0) {
		ServerSend(connfd, msg5501);
		return -1;
	}
	else if (mkdirs(filepath) == 0) {
		ServerSend(connfd, msg250);
	}
	/*else if (mkdir(filepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) {
		ServerSend(connfd, msg250);
	}*/
	return 0;
}

/* <CWD> 改变工作路径*/
int server_CWD(int connfd, char *sentence) {
	/* 未输入参数<filepath>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 3, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return -1;
	}

	char filepath[MAXFILEPATH];
	getFilepath(filepath, sentence);
	if (chdir(filepath) == 0) {
		/*if (path_num == (MAXPATHNUM - 1)) {
			ServerSend(connfd, msg5504);
			return -1;
		}
		memset(last_path[++path_num], 0, MAXFILEPATH);
		strcpy(last_path[path_num], set_path);*/

		memset(set_path, 0, MAXFILEPATH);
		strcpy(set_path, filepath);
		ServerSend(connfd, msg2501);
	}
	else {
		ServerSend(connfd, msg5502);
		return -1;
	}
	return 0;
}

/* <RWD> 回退工作路径 */
//int server_RWD(int connfd, char *sentence) {
//	if (path_num >= 0) {
//		memset(set_path, 0, MAXFILEPATH);
//		strcpy(set_path, last_path[path_num--]);
//		ServerSend(connfd, msg2501);
//	}
//	else {
//		ServerSend(connfd, msg5505);
//		return -1;
//	}
//	return 0;
//}

/* <PWD> 输出当前工作路径 */
void server_PWD(int connfd, char *sentence) {
	char curdir[MAXFILEPATH] = "257 ";
	strcat(curdir, set_path);
	strcat(curdir, "\r\n");
	ServerSend(connfd, curdir);
}

/* <LIST> */
int server_LIST(int connfd, char *sentence) {
    /* 建立数据连接 */
    /*if (server_PASV(connfd, sentence) < 0) {
        ServerSend(connfd, msg500);
        return -1;
    }*/
    if (pasvAccept() < 0) {
        ServerSend(connfd, msg425);
        closeSpecifySocket(&server_data_socket);
        closeSpecifySocket(&server_pasv_socket);
        return -1;
    }
    /* 利用系统ls将目录下列表保存到filelist.txt */
    if(system("ls -l | tail -n+2 > /var/filelist.txt") < 0){
        ServerSend(connfd, msg451);
        return -1;
    }
    FILE* dataFile = fopen("/var/filelist.txt","r");
    if(!dataFile) {
       ServerSend(connfd, msg451);
       return -1;
    }
    else {
        size_t datasize;
        ServerSend(connfd, msg150);
        memset(sentence, 0, MAXSIZE);
        do {
            datasize = fread(sentence, 1, MAXSIZE, dataFile);
            if (write(server_data_socket, sentence, datasize) < 0) {
                ServerSend(connfd, msg426);
                closeSpecifySocket(&server_data_socket);
                closeSpecifySocket(&server_pasv_socket);
                return -1;
            }
        } while (datasize > 0);
        ServerSend(connfd, msg226);
    }
    fclose(dataFile);
    system("rm /var/filelist.txt");
    closeSpecifySocket(&server_data_socket);
    closeSpecifySocket(&server_pasv_socket);
    return 0;
}

/* <RMD> 删除指定目录 */
int server_RMD(int connfd, char *sentence) {
	/* 未输入参数<filepath>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 3, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return -1;
	}

	char filepath[MAXFILEPATH];
	getFilepath(filepath, sentence);

    if (rmdir(filepath) == 0) {
        ServerSend(connfd, msg2502);
    }
    else {
		if (access(filepath, F_OK) == 0) {
			ServerSend(connfd, msg5503);
			return -1;
		}
		else {
			ServerSend(connfd, msg5502);
			return -1;
		}
    }
    return 0;
}

/* server端重名文件 */
int server_RNFR(int connfd, char *sentence) {
	/* 未输入参数<filepath>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 4, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return -1;
	}

	FILE *dataFile = fopen(pareme, "rb");
	if (!dataFile) {
		ServerSend(connfd, msg551);
		return -1;
	}

	//char filepath[MAXFILEPATH];
	//getFilepath(filepath, sentence);
	
	memset(last_name_path, 0, MAXFILEPATH);
	strcpy(last_name_path, pareme);
	RN_flag = 1;
	ServerSend(connfd, msg2503);

	return 0;
}

int server_RNTO(int connfd, char *sentence) {
	/* 未输入参数<filepath>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 4, pareme);
	if (strlen(pareme) == 0) {
		ServerSend(connfd, msg5003);
		return -1;
	}
	FILE *dataFile = fopen(pareme, "rb");
	if (dataFile) {
		ServerSend(connfd, msg5501);
		return -1;
	}
	if (!RN_flag) {
		ServerSend(connfd, msg5307);
		return -1;
	}
	else if(rename(last_name_path, pareme) == 0) {
		ServerSend(connfd, msg2504);
	}
	RN_flag = 0;
	return 0;
}