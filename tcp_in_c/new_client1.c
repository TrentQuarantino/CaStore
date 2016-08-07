#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_client (int port, char *hostis) {
    int cfd;
    struct sockaddr_in servaddr;

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, hostis, &(servaddr.sin_addr));

     if (connect(cfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
     	perror("ERROR connecting");
     	exit(1);
     } 
     printf("connesso\n");
     return(cfd);
}

int create_client_loop (int cfd) {
    char sendline[128];
    char recvline[128];

    while(1) {
        bzero(sendline, 128);
        bzero(recvline, 128);
        fgets(sendline, 128, stdin);

        if (write(cfd, sendline, strlen(sendline) - 1) <= 0) break;
        if (read(cfd, recvline, 128) <= 0) break;

        printf("%s", recvline);
    }
    return(0);
}

int main (int argc, char **argv) {
    int cfd;
    char hostis[] = "127.0.0.1"; //host
    int port = 11124;

    cfd = create_client(port, hostis);
    create_client_loop(cfd);

    close(cfd);
    return(0);
}