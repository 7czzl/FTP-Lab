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


/***************************************************/

/* 判断是否接收了多条server消息，消息数量保存至multireceive */
void Multireceive(int *multireceive, char *sentence);

/* Client接收来自服务器消息 */
int ClientRecv(int sockfd, char *sentence);

/* Client向服务器发送消息 */
int ClientSend(int sockfd, const char *sentence);

/* 在登录FTP服务器前从命令行读取命令 */
void readCommand(char *sentence);

/* 在登录FTP服务器后从命令行读取命令 */
void readFtpCmd(char *sentence);

/* client命令操作汇总 */
int clientCommand(int sockfd, char *sentence);

/* client关闭指定socket*/
void closeSpecifySocket(int *specifySocket);

/* client关闭所有socket */
void closeAllSocket();

/* <USER> & <PASS> */
void UserLogin(int sockfd, char *sentence);

/* <PASV>，本地绑定socket，等待与server建立连接*/
int client_PASV(int sockfd, char *sentence);

/* <PORT>，本地绑定socket并建立监听 */
int client_PORT(char *sentence);

/* client在建立PASV模式时向server发起connect请求 */
int clientConnect();

/* client在建立PORT模式后当server发起connect时进行accept连接 */
int clientAccept();

/* client端接收文件 */
int clientReceiveFile(int sock, char *sentence);

/* client端发送文件 */
int clientTransferFile(int sock, char *sentence);

/* <REST> 计算文件大小并发送偏移量 */
void client_REST(int sockfd, char *sentence);

/* client建立PORT/PASV模式后,<RETR>,进行文件下载 */
int client_RETR(char *sentence);

/* <APPE> 提示服务器将断点上传文件 */
void client_APPEfb(int sockfd, char *sentence);

/* client建立PORT/PASV模式后,<STOR>,进行文件上传 */
int client_STOR(char *sentence);

/* client接收LIST命令 */
int client_LIST(int sockfd, char *cmd, char *sentence);



/*==================================================*/


/* 判断是否接收了多条server消息，消息数量保存至multireceive */
void Multireceive(int *multireceive, char *sentence) {
	*multireceive = -1;
	for (unsigned int i = 0; i < strlen(sentence); i++) {
		if (sentence[i] == '\r' || sentence[i] == '\n') {
			*multireceive += 1;
			if (sentence[i + 1] == '\r' || sentence[i + 1] == '\n') {
				i++;
			}
		}
	}
}

/* Client接收来自服务器消息 */
int ClientRecv(int sockfd, char *sentence) {
	memset(sentence, 0, MAXSIZE);
	int len = 0;
	if ((len = read(sockfd, sentence, MAXSIZE)) < 0) {
		printf("Error read() in ClientRecv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return len;
}

/* Client向服务器发送消息 */
int ClientSend(int sockfd, const char *sentence) {
	if (write(sockfd, sentence, strlen(sentence)) < 0) {
		printf("Error write() in ClientSent(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}

/* 在登录FTP服务器前从命令行读取命令 */
void readCommand(char *sentence) {
	memset(sentence, 0, MAXSIZE);
	fgets(sentence, MAXSIZE, stdin);
	return;
}

/* 在登录FTP服务器后从命令行读取命令 */
void readFtpCmd(char *sentence) {
	printf("ftp> ");
	readCommand(sentence);
	return;
}

/* client命令操作汇总 */
int clientCommand(int sockfd, char *sentence) {
	char cmd[MAXSIZE];
	ClientRecv(sockfd, sentence);
	if (CommandRecognize(sentence, "220") == 0) {
		printf("fail to connect.\n");
		closeAllSocket();
		exit(1);
	}
	printf("%s", sentence);
	UserLogin(sockfd, sentence);
	while (1) {
		multi_receive = -1;
		readFtpCmd(sentence);
		memset(cmd, 0, MAXSIZE);
		strcpy(cmd, sentence);
		if (CommandRecognize(sentence, "LIST")) {
			char str[5] = "PASV";
			for (int i = 0; i < 4; i++) {
				sentence[i] = str[i];
			}
		}
		if (CommandRecognize(sentence, "REST")) {
			client_REST(sockfd, sentence);
		}
		if (CommandRecognize(sentence, "APPE")) {
			char appe_file[MAXFILEPATH];
			getCmd_parse(sentence, 4, appe_file);
			FILE *dataFile = fopen(appe_file, "rb");
			if (!dataFile) {
				printf("no such file.\r\n");
				continue;
			}
		}
		if (ClientSend(sockfd, sentence) < 0) {
			closeAllSocket();
			exit(1);
		}
		if (ClientRecv(sockfd, sentence) < 0) {
			closeAllSocket();
			exit(1);
		}
		printf("%s", sentence);
		/* 判断是否接收了多条server消息，消息数量保存至multireceive */
		Multireceive(&multi_receive, sentence);

		if (CommandRecognize(sentence, "200") && CommandRecognize(cmd, "PORT")) {
			if (client_PORT(cmd) < 0) {
				printf("Error opening socket for data connection\r\n");
			}
			else {
				mode = MPORT;
			}
		}
		else if (CommandRecognize(sentence, "227") && CommandRecognize(cmd, "PASV")) {
			if (client_PASV(sockfd, sentence) < 0) {
				printf("Error opening socket for data connection\r\n");
			}
			else {
				mode = MPASV;
			}
		}
		else if (CommandRecognize(sentence, "150") && CommandRecognize(cmd, "RETR")) {
			if (client_RETR(cmd) < 0) {
				printf("Error download file\r\n");
			}
			if (multi_receive == 0) {
				if (ClientRecv(sockfd, sentence) < 0) {
					closeAllSocket();
					exit(1);
				}
				printf("%s", sentence);
			}
			mode = MDEFAULT;
		}
		else if (CommandRecognize(sentence, "150") && CommandRecognize(cmd, "STOR")) {

			if (client_STOR(cmd) < 0) {
				printf("Error upload file\r\n");
			}
			if (multi_receive == 0) {
				if (ClientRecv(sockfd, sentence) < 0) {
					closeAllSocket();
					exit(1);
				}
				printf("%s", sentence);
			}
			mode = MDEFAULT;
		}
		else if (CommandRecognize(sentence, "227") && CommandRecognize(cmd, "LIST")) {
			if (client_LIST(sockfd, cmd, sentence) < 0) {
				printf("fail to get list.\n");
			}
			if (multi_receive == 0) {
				if (ClientRecv(sockfd, sentence) < 0) {
					closeAllSocket();
					exit(1);
				}
				printf("%s", sentence);
			}
		}
		else if (CommandRecognize(sentence, "APPEfb") && CommandRecognize(cmd, "APPE")) {
			client_APPEfb(sockfd, sentence);
		}
		else if (CommandRecognize(sentence, "221") && CommandRecognize(cmd, "QUIT")) {
			break;
		}
		else if (CommandRecognize(sentence, "221") && CommandRecognize(cmd, "ABOR")) {
			break;
		}
	}
	return 0;
}

/* client关闭指定socket*/
void closeSpecifySocket(int *specifySocket) {
	close(*specifySocket);
	*specifySocket = -1;
}

/* client关闭所有socket */
void closeAllSocket() {
	if (client_data_socket > 0) {
		closeSpecifySocket(&client_data_socket);
	}
	if (client_pasv_socket > 0) {
		closeSpecifySocket(&client_pasv_socket);
	}
	if (client_port_socket > 0) {
		closeSpecifySocket(&client_port_socket);
	}
	closeSpecifySocket(&sockfd);
}

/* <USER> & <PASS> */
void UserLogin(int sockfd, char *sentence) {
	/* input用户名 */
	while (1) {
		readCommand(sentence);
		ClientSend(sockfd, sentence);
		ClientRecv(sockfd, sentence);
		printf("%s", sentence);
		if (CommandRecognize(sentence, "331")) {
			break;
		}
	}
	/* input密码 */
	while (1) {
		readCommand(sentence);
		ClientSend(sockfd, sentence);
		ClientRecv(sockfd, sentence);
		printf("%s", sentence);
		if (CommandRecognize(sentence, "230")) {
			break;
		}
	}
}

/* <PASV>，本地绑定socket，等待与server建立连接*/
int client_PASV(int sockfd, char *sentence) {
	char ip[20];
	int port = 0;
	int index = 0;
	/* 客户端解析获取ip和port */
	sentence[strlen(sentence) - 1] = '\0';
	while (sentence[index] != '(') {
		index++;
	}
	get_IP_PORTinParse(sentence, index + 1, ip, &port);
	memset(&pasvaddr, 0, sizeof(pasvaddr));
	/* 本地建立socket */
	pasvaddr.sin_family = AF_INET;
	pasvaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &pasvaddr.sin_addr) <= 0) {
		printf("Error inet_pton() in client_PASV(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	/* 若client的PASV处于打开状态，则关闭 */
	if (client_pasv_socket >= 0) {
		closeSpecifySocket(&client_pasv_socket);
	}
	if ((client_pasv_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket() in client_PASV(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	if (clientConnect() < 0) {
		printf("Error connect() in client_PASV(): %s(%d)\n", strerror(errno), errno);
		closeSpecifySocket(&client_pasv_socket);
		return -1;
	}
	return 0;
}

/* <PORT>，本地绑定socket并建立监听 */
int client_PORT(char *sentence) {
	char ip[20];
	int port = 0;
	struct sockaddr_in portaddr;
	get_IP_PORTinParse(sentence, 5, ip, &port);
	/* PORT模式建立socket并设立监听 */
	memset(&portaddr, 0, sizeof(portaddr));
	portaddr.sin_family = AF_INET;
	portaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &portaddr.sin_addr) <= 0) {
		printf("Error inet_pton() in client_PORT(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	/* 若client的PORT模式处于打开状态，则关闭 */
	if (client_port_socket >= 0) {
		closeSpecifySocket(&client_port_socket);
	}
	if ((client_port_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket() in client_PORT(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	if (bind(client_port_socket, (struct sockaddr*)&portaddr, sizeof(portaddr)) == -1) {
		printf("Error bind() in client_PORT(): %s(%d)\n", strerror(errno), errno);
		if (client_port_socket >= 0) {
			closeSpecifySocket(&client_port_socket);
		}
		return -1;
	}
	if (listen(client_port_socket, 10) == -1) {
		printf("Error listen() in client_PORT(): %s(%d)\n", strerror(errno), errno);
		if (client_port_socket >= 0) {
			closeSpecifySocket(&client_port_socket);
		}
		return -1;
	}
	return 0;
}

/* client在建立PASV模式时向server发起connect请求 */
int clientConnect() {
	if (connect(client_pasv_socket, (struct sockaddr*)&pasvaddr, sizeof(pasvaddr)) < 0) {
		printf("Error connect() in clientConnect(): %s(%d)\n", strerror(errno), errno);
		if (client_pasv_socket >= 0) {
			closeSpecifySocket(&client_pasv_socket);
		}
		return -1;
	}
	return 0;
}

/* client在建立PORT模式后当server发起connect时进行accept连接 */
int clientAccept() {
	if (client_data_socket >= 0) {
		closeSpecifySocket(&client_data_socket);
	}
	if ((client_data_socket = accept(client_port_socket, NULL, NULL)) == -1) {
		printf("Error accept() in clientAccept(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}

/* client端接收文件 */
int clientReceiveFile(int sock, char *sentence) {
	char filepath[MAXFILEPATH];
	getCmd_parse(sentence, 4, filepath);
	FILE *dataFile;
	if (rest_flag) {
		dataFile = fopen(filepath, "ab");
		rest_flag = 0;
	}
	else {
		dataFile = fopen(filepath, "wb");
	}
	if (!dataFile) {
		ClientSend(sockfd, "No such file or dictionary.");
		return -1;
	}
	else {
		int datasize = 0;
		while (1) {
			datasize = ClientRecv(sock, sentence);
			if (datasize > 0) {
				fwrite(sentence, 1, datasize, dataFile);
			}
			else if (datasize == 0) {
				break;
			}
			else {
				fclose(dataFile);
				return -1;
			}
		}
	}
	fclose(dataFile);
	return 0;
}

/* client端发送文件 */
int clientTransferFile(int sock, char *sentence) {
	char filepath[MAXFILEPATH];
	getCmd_parse(sentence, 4, filepath);
	FILE *dataFile = fopen(filepath, "rb");
	if (!dataFile) {
		printf("%s", "No such file or dictionary.\r\n");
		return -1;
	}
	else {
		int datasize = 0;
		memset(sentence, 0, MAXSIZE);
		fseek(dataFile, appe_offset, 0);   //定位到文件偏移处
		do {
			if ((datasize = fread(sentence, 1, MAXSIZE, dataFile)) < 0) {
				printf("%s", "Error when reading the file.\r\n");
				closeSpecifySocket(&sock);
				if (mode == MPORT) {
					closeSpecifySocket(&client_port_socket);
				}
				return -1;
			}
			else if (datasize == 0) {
				break;
			}
			if (write(sock, sentence, datasize) < 0) {
				printf("%s", "connection was broken or network failure.\r\n");
				closeSpecifySocket(&sock);
				if (mode == MPORT) {
					closeSpecifySocket(&client_port_socket);
				}
				return -1;
			}
		} while (datasize > 0);
	}
	appe_offset = 0; //恢复偏移量
	fclose(dataFile);
	return 0;
}

/* <REST> 计算文件大小并发送偏移量 */
void client_REST(int sockfd, char *sentence) {
	/* 未输入参数<file_path>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 4, pareme);
	if (strlen(pareme) == 0) {
		return;
	}
	FILE *dataFile = fopen(pareme, "rb");
	if (!dataFile) {
		printf("open file failed in client_REST()");
		return;
	}
	fseek(dataFile, 0, SEEK_END); //定位到文件末
	long length = ftell(dataFile);//文件长度
	char str[12];
	long2Char(length, str);
	memset(sentence, 0, MAXSIZE);
	strcat(sentence, "REST ");
	strcat(sentence, str);
	strcat(sentence, "\r\n");
	rest_flag = 1;
	return;
}

/* client建立PORT/PASV模式后,<RETR>,进行文件下载 */
int client_RETR(char *sentence) {
	switch (mode)
	{
	case MPORT:
		if (clientAccept() < 0) {
			printf("Error accept() in client_RETR(): %s(%d)\n", strerror(errno), errno);
			closeSpecifySocket(&client_data_socket);
			closeSpecifySocket(&client_port_socket);
			return -1;
		}
		if (clientReceiveFile(client_data_socket, sentence) < 0) {
			return -1;
		}
		closeSpecifySocket(&client_data_socket);
		closeSpecifySocket(&client_port_socket);
		break;

	case MPASV:
		if (clientReceiveFile(client_pasv_socket, sentence) < 0) {
			return -1;
		}
		closeSpecifySocket(&client_pasv_socket);
		break;

	default:
		break;
	}
	return 0;
}

/* <APPEfb> 获取服务器文件偏移量 */
void client_APPEfb(int sockfd, char *sentence) {
	/* 未输入参数<file_path>时 */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 6, pareme);
	appe_offset = atol(pareme);

	return;
}

/* client建立PORT/PASV模式后,<STOR>,进行文件上传 */
int client_STOR(char *sentence) {
	switch (mode)
	{
	case MPORT:
		if (clientAccept() < 0) {
			printf("Error accept() in client_STOR(): %s(%d)\n", strerror(errno), errno);
			closeSpecifySocket(&client_data_socket);
			closeSpecifySocket(&client_port_socket);
			return -1;
		}
		if (clientTransferFile(client_data_socket, sentence) < 0) {
			closeSpecifySocket(&client_data_socket);
			closeSpecifySocket(&client_port_socket);
			return -1;
		}
		closeSpecifySocket(&client_data_socket);
		closeSpecifySocket(&client_port_socket);
		break;

	case MPASV:
		if (clientTransferFile(client_pasv_socket, sentence) < 0) {
			closeSpecifySocket(&client_pasv_socket);
			return -1;
		}
		closeSpecifySocket(&client_pasv_socket);
		break;

	default:
		break;
	}
	return 0;
}

/* client接收LIST命令 */
int client_LIST(int sockfd, char *cmd, char *sentence) {
	if (client_PASV(sockfd, sentence) < 0) {
		printf("Error opening socket for data connection\r\n");
		return -1;
	}
	ClientSend(sockfd, cmd);
	ClientRecv(sockfd, sentence);
	printf("%s", sentence);
	/* 判断是否接收了多条server消息，消息数量保存至multireceive */
	Multireceive(&multi_receive, sentence);

	while (ClientRecv(client_pasv_socket, sentence) > 0) {
		printf("%s", sentence);
	}
	closeSpecifySocket(&client_pasv_socket);
	return 0;
}

