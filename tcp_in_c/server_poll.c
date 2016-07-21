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
#include <poll.h>

#define MAX(a, b)           ((a) > (b) ? (a) : (b))

void stampa (const char *dato, int len) {
  int i;

  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

int main (int argc, char **argv) {
    struct pollfd fds[50];
    int srv_fd, cfd;
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

    fd_count = 1;
    fds[0].fd = srv_fd;
    fds[0].events = POLLIN;

    while (1) {
        printf("WAIT ON SELECT %d\n", fd_count);
        if (poll(fds, fd_count, -1) < 0) {
            perror("poll()");
            continue;
        }

        // qualcuno se connesso
        if (fds[0].revents & POLLIN) {
            int cfd = accept(srv_fd, (struct sockaddr*) NULL, NULL);
            setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &yep, sizeof(int));
            printf("Accettato il client %d\n", cfd);
            fds[fd_count].fd = cfd;
            fds[fd_count].events = POLLIN;
            fd_count++;
        } else {
            char buffer[128];
            ssize_t rd;
            printf("SCAN FOR READ fda=%d\n", fd_count);
            for (int i = 1; i < fd_count; ++i) {
                int cfd = fds[i].fd;
                if (!(fds[i].revents & POLLIN)) continue;

                // leggiamo quello che ha scritto un client
                rd = read(cfd, buffer, sizeof(buffer));
                if (rd <= 0) {
                    memmove(fds + i, fds +i +1, (fd_count - i) * sizeof(struct pollfd));
                    fd_count--;
                    continue;
                }
                printf(" - letto dal client %d: ", cfd);
                stampa(buffer, rd);
                for (int j = 1; j < fd_count; ++j) {
                    write(fds[j].fd, buffer, rd);
                }
            }
        }
    }

    close(srv_fd);
    return(0);
}
