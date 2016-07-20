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
    char value[128];
    int v_len;
};

struct hashtable {
    struct node **heads;
    int vect_size;
};

struct hashtable db;

void stampa (const char *dato, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

void hashtable_init (struct hashtable *self, int vect_size) {
    struct node **buf; // vettore di puntatori ai nodi
    buf = (struct node **)malloc(vect_size * sizeof(struct node *));
    int i;
    self->vect_size = vect_size;
    self->heads = buf;
    for (i = 0; i < vect_size; i++){
        self->heads[i] = NULL;
    }
}

int hash (int key) {
    return(key);
}

void hashtable_insert (struct hashtable *self, int key, char *value, int value_length) {
    int vect_index;
    vect_index = hash(key) % self->vect_size;
    struct node *node;
    for (node = self->heads[vect_index]; node != NULL; node = node->next) {
      if (node->key == key) {
        node->v_len = value_length;
        memcpy(node->value, value, value_length);
        return;
      }
    }
    node = (struct node *)malloc(sizeof(struct node));
    node->next = self->heads[vect_index];
    node->key = key;
    node->v_len = value_length;
    memcpy(node->value, value, value_length);
    self->heads[vect_index] = node;
}

char *hashtable_get (struct hashtable *self, int key, int *v_len) {
    struct node *node;
    int v_index;
    v_index = hash(key) % self->vect_size;
    for (node = self->heads[v_index]; node != NULL; node = node->next) {
        if (node->key == key) {
            *v_len = node->v_len;
            return(node->value);
        }
    }
    *v_len = 11;
    return("non trovato");
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
    //leggiamo quello che ha scritto un client
    rd = read(cfd, buffer, sizeof(buffer));
    if (rd <= 0) {
        return(-1);
    }
    if (memcmp(buffer, "scus", 4) == 0) {
        memcpy(buffer, "figur\n", 6);
        rd = 6;
    } else if (memcmp(buffer, "chiudi", 6) == 0) {
        return(-1);
    } else if (memcmp(buffer, "put", 3) == 0) {
        printf("Byte letti %d\n", rd);

        char *key, *value;
        int k_len, v_len;

        key = buffer + 4;
        //printf("key = %p\n", key);

        value = memchr(buffer + 4, ' ',rd - 4) + 1;
        k_len = (value - key) -1;
        v_len = rd - 4 - k_len - 1;
        printf("key = ");
        stampa(key, k_len);
        printf("value = ");
        stampa(value, v_len);
        int i_key;
        i_key = strtol(key, NULL, 10);
        //printf("integer key = %d\n", i_key);

        hashtable_insert(&db, i_key, value, v_len);
        memcpy(buffer, "storato\n", 8);
        rd = 8;

    } else if (memcmp(buffer, "get", 3) == 0) {
        char *g_key, *g_value;
        int ig_key, v_len;
        g_key = buffer + 4;
        ig_key = strtol(g_key, NULL, 10);

        g_value = hashtable_get(&db, ig_key, &v_len);
        memcpy(buffer, g_value, v_len);
        memcpy(buffer + v_len, "\n", 1);
        rd = v_len + 1;
        //todo
    }
    printf(" - letto dal client %d: ", cfd);
    stampa(buffer, rd);
    write(cfd, buffer, rd);
    return(0);
}

int main (int argc, char **argv) {
    hashtable_init(&db, 16);
    int srv_fd;
    srv_fd = create_server(11124);
    server_loop(srv_fd, client_echo);

    close(srv_fd);
    return(0);
}