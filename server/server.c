#include "handler.h"

int main(int argc, char **argv) {
    /* ./server -root [path] -port [port] */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-root") == 0 && argv[i+1] && strcmp(argv[i+1], "-port") != 0) {
            memset(set_path, 0, 80);
            strcpy(set_path, argv[i+1]);
        }
        else if (strcmp(argv[i], "-port") == 0 && argv[i+1] && strcmp(argv[i+1], "-root") != 0) {
            set_port = atoi(argv[i+1]);
        }
    }

    struct sockaddr_in addr;
    char sentence[MAXSIZE];
    pid_t pid;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(set_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*
	复用端口，方便测试.
	*/
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) { 
        printf("Error setsockopt() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    } 

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    if (listen(listenfd, 10) == -1) {
        printf("Error listen() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    while (1) {
		// 连接成功
		int res = 0;
        if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
            printf("Error accept() in main(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
		pid = fork();
		if (pid == -1) {
			printf("Error fork() in main(): %s(%d)\n", strerror(errno), errno);
		}
		else if (pid == 0) {
			res = serverCommand(connfd, sentence);
			if (res == -1) {
				closeAllSocket();
				exit(-1);
			}
			else if (res == -2) {
				closeAllSocket();
				exit(0);
			}
		}
        closeSpecifySocket(&connfd);
    }
    closeAllSocket();
    return 0;
}


