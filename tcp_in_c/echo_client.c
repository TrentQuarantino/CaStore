//http://www.dis.uniroma1.it/~liberato/struct/typestruct/strutture.shtml
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct libro {
    char titolo[32];
    char autore[32];
    int anno;
};

int main (int argc, char **argv) {
    int sockfd;
    int n;
    char sendline[128];
    char recvline[128];
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(11124);
    inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr));

    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    #if 0
    while(1) {
        bzero(sendline, 128);
        bzero(recvline, 128);
        fgets(sendline, 128, stdin);

        if (write(sockfd, sendline, strlen(sendline) - 1) <= 0) break;
        if (read(sockfd, recvline, 128) <= 0) break;

        printf("%s", recvline);
    }
    #else
    struct libro lassie;
    memset(&lassie, 0, sizeof(struct libro));
    memcpy(lassie.titolo, "lessie titulo", 13);
    memcpy(lassie.autore, "Tito lo lungo", 13);
    lassie.anno = 1950;

    char buffer[128];
    ssize_t buf_len;

    memcpy(buffer,"put culo ", 9);
    memcpy(buffer + 9, &lassie, sizeof(struct libro));


    write(sockfd, buffer, sizeof(struct rubrica) + 9);
    memset(buffer, 0, sizeof(buffer));
    buf_len = read(sockfd, buffer, sizeof(buffer));
    printf("%s", buffer);

    memcpy(buffer, "get culo", 8);
    write(sockfd, buffer, 8);
    buf_len = read(sockfd, buffer, sizeof(buffer));

    memset(&lassie, 0, sizeof(struct libro));
    memcpy(&lassie, buffer, sizeof(struct libro));
    printf("titolo %s\nautore %s\nanno %d\n", lassie.titolo, lassie.autore, lassie.anno);
    #endif

    close(sockfd);
    return(0);
}