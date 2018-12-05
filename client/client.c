#include "handler.h"

int main(int argc, char **argv) {
    /* ./client -ip [ip] -port [port] */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0 && argv[i+1] && strcmp(argv[i+1], "-port") != 0) {
            memset(set_ip, 0, 20);
            strcpy(set_ip, argv[i+1]);
        }
        else if (strcmp(argv[i], "-port") == 0 && argv[i+1] && strcmp(argv[i+1], "-ip") != 0) {
            set_port = atoi(argv[i+1]);
        }
    }

    int sockfd;
    struct sockaddr_in addr;
    char sentence[MAXSIZE];
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;    
    addr.sin_port = htons(set_port);
    if (inet_pton(AF_INET, set_ip, &addr.sin_addr) <= 0) {
        printf("Error inet_pton() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    printf("Waiting to connect with %s:%d...\n", set_ip, set_port);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect() in main(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    clientCommand(sockfd, sentence);
    closeAllSocket();
    return 0;
}

