#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b)       ((a) > (b) ? (a) : (b))

typedef int (*client_func_t)(int cfd);

struct node {
    struct node *next;
    int key;
    char *value;
};

struct hashtable {
    struct node **heads;
    int vect_size;
};

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
    ssize_t = rd;
    //leggiamo quello che ha scritto un client
    rd = read(cfd, buffer, sizeof(buffer));
    if (rd <= 0) {
        return(-1);
    }
    if (mecmp(buffer, "scus", 4) == 0) {
        memcpy(buffer, "figur\n", 6);
        rd = 6;
    } else if (memcmp(buffer, "chiudi", 6) == 0) {
        return(-1);
    } else if (memcmp(buffer, "put", 3) == 0) {
        //todo
    } else if (memcmp(buffer, "get", 3) == 0) {
        //todo
    }
    printf(" - letto dal client %d: ", cfd);
    stampa(buffer, rd);
    write(cfd, buffer, rd);
    return(0);
}

void stampa (const char *dato, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

int hash (int key) {
    return(key);
}

void hashtale_insert (struct hashtable *self, int key, char *value, int value_length) {
    struct node *node;
    int vect_index;
    vect_index = hash(key) % self->vect_size;
    node = (struct node *)malloc(sizeof(struct node));
    node->key = key;
    node->next = self->heads[vect_index];
    memcpy(node->value, value, value_length);
    self->heads[vect_index] = node;
}

char *hashtable_get (struct hashtable *self, int key) {
    struct node *node;
    int v_index;
    v_index = hash(key) % self->vect_size;
    for (node = self->heads[v_index]; node != NULL; node = node->next) {
        if (node->key == key) {
            return(node->value);
        }
    }
    return("non trovato");
}





int main (int argc, char **argv) {
    int srv_fd;
    srv_fd = create_server(11124);
    server_loop(srv_fd, client_echo);

    close(srv_fd);
    return(0);
}