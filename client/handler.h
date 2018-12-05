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

/* �ж��Ƿ�����˶���server��Ϣ����Ϣ����������multireceive */
void Multireceive(int *multireceive, char *sentence);

/* Client�������Է�������Ϣ */
int ClientRecv(int sockfd, char *sentence);

/* Client�������������Ϣ */
int ClientSend(int sockfd, const char *sentence);

/* �ڵ�¼FTP������ǰ�������ж�ȡ���� */
void readCommand(char *sentence);

/* �ڵ�¼FTP��������������ж�ȡ���� */
void readFtpCmd(char *sentence);

/* client����������� */
int clientCommand(int sockfd, char *sentence);

/* client�ر�ָ��socket*/
void closeSpecifySocket(int *specifySocket);

/* client�ر�����socket */
void closeAllSocket();

/* <USER> & <PASS> */
void UserLogin(int sockfd, char *sentence);

/* <PASV>�����ذ�socket���ȴ���server��������*/
int client_PASV(int sockfd, char *sentence);

/* <PORT>�����ذ�socket���������� */
int client_PORT(char *sentence);

/* client�ڽ���PASVģʽʱ��server����connect���� */
int clientConnect();

/* client�ڽ���PORTģʽ��server����connectʱ����accept���� */
int clientAccept();

/* client�˽����ļ� */
int clientReceiveFile(int sock, char *sentence);

/* client�˷����ļ� */
int clientTransferFile(int sock, char *sentence);

/* <REST> �����ļ���С������ƫ���� */
void client_REST(int sockfd, char *sentence);

/* client����PORT/PASVģʽ��,<RETR>,�����ļ����� */
int client_RETR(char *sentence);

/* <APPE> ��ʾ���������ϵ��ϴ��ļ� */
void client_APPEfb(int sockfd, char *sentence);

/* client����PORT/PASVģʽ��,<STOR>,�����ļ��ϴ� */
int client_STOR(char *sentence);

/* client����LIST���� */
int client_LIST(int sockfd, char *cmd, char *sentence);



/*==================================================*/


/* �ж��Ƿ�����˶���server��Ϣ����Ϣ����������multireceive */
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

/* Client�������Է�������Ϣ */
int ClientRecv(int sockfd, char *sentence) {
	memset(sentence, 0, MAXSIZE);
	int len = 0;
	if ((len = read(sockfd, sentence, MAXSIZE)) < 0) {
		printf("Error read() in ClientRecv(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return len;
}

/* Client�������������Ϣ */
int ClientSend(int sockfd, const char *sentence) {
	if (write(sockfd, sentence, strlen(sentence)) < 0) {
		printf("Error write() in ClientSent(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return 0;
}

/* �ڵ�¼FTP������ǰ�������ж�ȡ���� */
void readCommand(char *sentence) {
	memset(sentence, 0, MAXSIZE);
	fgets(sentence, MAXSIZE, stdin);
	return;
}

/* �ڵ�¼FTP��������������ж�ȡ���� */
void readFtpCmd(char *sentence) {
	printf("ftp> ");
	readCommand(sentence);
	return;
}

/* client����������� */
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
		/* �ж��Ƿ�����˶���server��Ϣ����Ϣ����������multireceive */
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

/* client�ر�ָ��socket*/
void closeSpecifySocket(int *specifySocket) {
	close(*specifySocket);
	*specifySocket = -1;
}

/* client�ر�����socket */
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
	/* input�û��� */
	while (1) {
		readCommand(sentence);
		ClientSend(sockfd, sentence);
		ClientRecv(sockfd, sentence);
		printf("%s", sentence);
		if (CommandRecognize(sentence, "331")) {
			break;
		}
	}
	/* input���� */
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

/* <PASV>�����ذ�socket���ȴ���server��������*/
int client_PASV(int sockfd, char *sentence) {
	char ip[20];
	int port = 0;
	int index = 0;
	/* �ͻ��˽�����ȡip��port */
	sentence[strlen(sentence) - 1] = '\0';
	while (sentence[index] != '(') {
		index++;
	}
	get_IP_PORTinParse(sentence, index + 1, ip, &port);
	memset(&pasvaddr, 0, sizeof(pasvaddr));
	/* ���ؽ���socket */
	pasvaddr.sin_family = AF_INET;
	pasvaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &pasvaddr.sin_addr) <= 0) {
		printf("Error inet_pton() in client_PASV(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	/* ��client��PASV���ڴ�״̬����ر� */
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

/* <PORT>�����ذ�socket���������� */
int client_PORT(char *sentence) {
	char ip[20];
	int port = 0;
	struct sockaddr_in portaddr;
	get_IP_PORTinParse(sentence, 5, ip, &port);
	/* PORTģʽ����socket���������� */
	memset(&portaddr, 0, sizeof(portaddr));
	portaddr.sin_family = AF_INET;
	portaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &portaddr.sin_addr) <= 0) {
		printf("Error inet_pton() in client_PORT(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	/* ��client��PORTģʽ���ڴ�״̬����ر� */
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

/* client�ڽ���PASVģʽʱ��server����connect���� */
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

/* client�ڽ���PORTģʽ��server����connectʱ����accept���� */
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

/* client�˽����ļ� */
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

/* client�˷����ļ� */
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
		fseek(dataFile, appe_offset, 0);   //��λ���ļ�ƫ�ƴ�
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
	appe_offset = 0; //�ָ�ƫ����
	fclose(dataFile);
	return 0;
}

/* <REST> �����ļ���С������ƫ���� */
void client_REST(int sockfd, char *sentence) {
	/* δ�������<file_path>ʱ */
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
	fseek(dataFile, 0, SEEK_END); //��λ���ļ�ĩ
	long length = ftell(dataFile);//�ļ�����
	char str[12];
	long2Char(length, str);
	memset(sentence, 0, MAXSIZE);
	strcat(sentence, "REST ");
	strcat(sentence, str);
	strcat(sentence, "\r\n");
	rest_flag = 1;
	return;
}

/* client����PORT/PASVģʽ��,<RETR>,�����ļ����� */
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

/* <APPEfb> ��ȡ�������ļ�ƫ���� */
void client_APPEfb(int sockfd, char *sentence) {
	/* δ�������<file_path>ʱ */
	char pareme[MAXFILEPATH];
	getCmd_parse(sentence, 6, pareme);
	appe_offset = atol(pareme);

	return;
}

/* client����PORT/PASVģʽ��,<STOR>,�����ļ��ϴ� */
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

/* client����LIST���� */
int client_LIST(int sockfd, char *cmd, char *sentence) {
	if (client_PASV(sockfd, sentence) < 0) {
		printf("Error opening socket for data connection\r\n");
		return -1;
	}
	ClientSend(sockfd, cmd);
	ClientRecv(sockfd, sentence);
	printf("%s", sentence);
	/* �ж��Ƿ�����˶���server��Ϣ����Ϣ����������multireceive */
	Multireceive(&multi_receive, sentence);

	while (ClientRecv(client_pasv_socket, sentence) > 0) {
		printf("%s", sentence);
	}
	closeSpecifySocket(&client_pasv_socket);
	return 0;
}

