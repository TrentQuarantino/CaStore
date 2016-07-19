//https://en.wikipedia.org/wiki/Hash_table
//http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_hashtable.aspx
//http://www.ateneonline.it/hyperbook/c_book/C1208.htm
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <poll.h>

#define MAX(a, b)           ((a) > (b) ? (a) : (b))

typedef int (*client_func_t)(int cfd);

void stampa (const char *dato, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

int create_server (int port) {
    int srv_fd;
    int yep = 1;
    struct sockaddr_in servaddr;
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &yep, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);

    bind(srv_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(srv_fd, 10);
    return(srv_fd);
}

void server_loop (int srv_fd, client_func_t c_func) {
    struct pollfd fds[50];
    int cfd;
    int fd_count;
    fd_count = 1;
    fds[0].fd = srv_fd;
    fds[0].events = POLLIN;

    while (1) {
        printf("WAIT ON SELECT %d\n", fd_count);
        if (poll(fds, fd_count, -1) < 0) {
            perror("poll()");
            continue;
        }

        // qualcuno si e' connesso
        if (fds[0].revents & POLLIN) {
            int cfd = accept(srv_fd, (struct sockaddr*) NULL, NULL);
            int yep = 1;
            setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &yep, sizeof(int));
            printf("Accettato il client %d\n", cfd);
            fds[fd_count].fd = cfd;
            fds[fd_count].events = POLLIN;
            fd_count++;
        } else {

            printf("SCAN FOR READ fda=%d\n", fd_count);
            for (int i = 1; i < fd_count; ++i) {
                int cfd = fds[i].fd;
                if (!(fds[i].revents & POLLIN)) continue;

                if (c_func(cfd) < 0) {
                    memmove(fds + i, fds +i +1, (fd_count - i) * sizeof(struct pollfd));
                    fd_count--;
                    continue;
                }
            }
        }
    }
}

int client_echo (int cfd) {
    char buffer[128];
    ssize_t rd;
    // leggiamo quello che ha scritto un client
                rd = read(cfd, buffer, sizeof(buffer));
                if (rd <= 0) {
                    return(-1);
                }
                if (memcmp(buffer, "scus", 4) == 0) {
                    memcpy(buffer, "figur\n",6);
                    rd = 6;
                } else if (memcmp(buffer, "chiudi", 6) == 0) {
                    return(-1);
                }
                printf(" - letto dal client %d: ", cfd);
                stampa(buffer, rd);
                /*memcpy(buffer + rd, "ciao\n",5);
                printf("il buffer %s", buffer);
                rd += 5;*/
                write(cfd, buffer, rd);
                return(0);
}

int main (int argc, char **argv) {
    int srv_fd;
    srv_fd = create_server(11124);
    server_loop(srv_fd, client_echo);

    close(srv_fd);
    return(0);
}
