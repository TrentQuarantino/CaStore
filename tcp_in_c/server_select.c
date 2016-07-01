#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>

#define MAX(a, b)           ((a) > (b) ? (a) : (b))

void stampa (const char *dato, int len) {
  int i;

  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

int main (int argc, char **argv) {
    int srv_fd, cfd;
    int fd_list[50];
    int fd_count;

    struct sockaddr_in servaddr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    int yep = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &yep, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(11124);

    bind(srv_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(srv_fd, 10);

    fd_count = 0;
    while (1) {
        fd_set read_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_SET(srv_fd, &read_fds);
        nfds = srv_fd + 1;
        for (int i = 0; i < fd_count; ++i) {
          FD_SET(fd_list[i], &read_fds);
          nfds = MAX(fd_list[i] + 1, nfds);
        }

        printf("WAIT ON SELECT %d\n", nfds);
        if (select(nfds, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select()");
            continue;
        }

        // qualcuno se' connesso
        if (FD_ISSET(srv_fd, &read_fds)) {
            int cfd = accept(srv_fd, (struct sockaddr*) NULL, NULL);
            setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &yep, sizeof(int));
            printf("Accettato il client %d\n", cfd);
            fd_list[fd_count++] = cfd;
        } else {
            char buffer[128];
            ssize_t rd;
            printf("SCAN FOR READ max_fd=%d\n", nfds);
            for (int i = 0; i < fd_count; ++i) {
                int cfd = fd_list[i];
                if (!FD_ISSET(cfd, &read_fds)) continue;

                // leggiamo quello che ha scritto un client
                rd = read(cfd, buffer, sizeof(buffer));
                if (rd <= 0) {
                    //FD_CLR(cfd, &read_fds);
                    memmove(fd_list + i, fd_list +i +1, (fd_count - i) * sizeof(int));
                    fd_count--;
                    continue;
                }
                printf(" - letto dal client %d: ", cfd);
                stampa(buffer, rd);
                for (int j = 0; j < fd_count; ++j) {
                //write(cfd, buffer, rd);
                    write(fd_list[j], buffer, rd);
                }
            }
        }
    }

    close(srv_fd);
    return(0);
}
