#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ���server���ܵ����� */
int CommandRecognize(char *source, const char *target);

/* longתΪchar���� */
void long2Char(long num, char *str);

/* ��ȡ����Ĳ������� */
void getCmd_parse(char *sentence, int len, char *cmd);

/* ��ȡ�����е��ļ�·�� */
void getFilepath(char *filepath, char *sentence);

/* ��ȡPORT��������е�ip��port */
void get_IP_PORTinParse(char *sentence, char *ip, int *port);

/*==============================================================================*/

/* 
���server���ܵ�����
ʵ�������ж�source�Ƿ���0-len��Χ�ڵ���target
*/
int CommandRecognize(char *source, const char *target) {
	int len = strlen(target);
	int flag = 1;
	for (int i = 0; i < len; i++) {
		if (source[i] != target[i]) {
			flag = 0;
		}
	}
	return flag;
}

/* longתΪchar���� */
void long2Char(long num, char *str) {
	int index = 0;
	char temp[12];
	while (num) {
		temp[index++] = num % 10 + '0';
		num /= 10;
	}
	for (int i = 0; i < index; i++) {
		str[i] = temp[index - 1 - i];
	}
	str[index] = '\0';
}

/* ��ȡ����Ĳ������� */
void getCmd_parse(char *sentence, int len, char *cmd) {
	unsigned int index = len;
	unsigned int cmdLength = strlen(sentence);
	while (sentence[index] == ' ' && index < cmdLength) {
		index++;
	}
	if (index == cmdLength) {
		return;
	}
	int i = 0;
	for (; index < cmdLength; index++) {
		cmd[i++] = sentence[index];
	}
	cmd[i] = '\0';
	/* ȥ��"\r\n" */
	for (unsigned int j = 0; j < strlen(cmd); j++) {
		if (cmd[j] == '\r' || cmd[j] == '\n') {
			cmd[j] = '\0';
			break;
		}
	}
}

/* ��ȡ�����е��ļ�·�� */
void getFilepath(char *filepath, char *sentence) {
	getCmd_parse(sentence, 3, filepath);
	/* ���·��ת��Ϊ����·�� */
	if (CommandRecognize(filepath, "/") == 0) {
		char temp[MAXFILEPATH];
		strcpy(temp, set_path);
		int len = strlen(temp);
		temp[len++] = '/';
		temp[len] = '\0';
		strcat(temp, filepath);
		strcpy(filepath, temp);
	}
}

/* ��ȡPORT��������е�ip��port */
void get_IP_PORTinParse(char *sentence, char *ip, int *port) {
	int digit = 0;
	unsigned int index = 5;
	int symbol = 0;
	int len = strlen(sentence);
	//����ip
	for (int i = 0; index < len; index++) {
		if (sentence[index] >= '0' && sentence[index] <= '9') {
			ip[i++] = sentence[index];
			digit = digit * 10 + (sentence[index] - '0');
		}
		else if (sentence[index] == ',') {
			symbol++;
			if (symbol == 4) {
				ip[i++] = '\0';
				break;
			}
			ip[i++] = '.';
			if (digit < 0 || digit > 255) {
				printf("ip address is invalid.\r\n");
				return;
			}
			digit = 0;

		}
	}
	//����port
	digit = 0;
	for (; index < len; index++) {
		if (sentence[index] >= '0' && sentence[index] <= '9') {
			*port = *port * 10 + (sentence[index] - '0');
		}
		else if (sentence[index] == ',') {
			digit = *port;
			*port = 0;
		}
	}
	*port += digit * 256;
}


