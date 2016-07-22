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
    char key[32];
    char value[128];
    int k_len;
    int v_len;
};

struct hashtable {
    struct node **heads;
    int vect_size;
};

struct hashtable db;
int log_fd;

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

int hash (int k_len) {
    return(k_len);
}

int k_equal (char *node_key, char *key, int node_k_len, int key_length) {
    if (node_k_len != key_length) return(0);
    if (memcmp(node_key, key, key_length) == 0) return(1);
    return(0);
}

void hashtable_insert (struct hashtable *self, char *key, char *value, int key_length, int value_length) {
    int vect_index;
    vect_index = hash(key_length) % self->vect_size;
    struct node *node;
    for (node = self->heads[vect_index]; node != NULL; node = node->next) {
      if (k_equal(node->key, key, node->k_len, key_length)) {
        node->v_len = value_length;
        node->k_len = key_length;
        memcpy(node->key, key, key_length);
        memcpy(node->value, value, value_length);
        return;
      }
    }
    node = (struct node *)malloc(sizeof(struct node));
    node->next = self->heads[vect_index];
    node->k_len = key_length;
    node->v_len = value_length;
    memcpy(node->key, key, key_length);
    memcpy(node->value, value, value_length);
    self->heads[vect_index] = node;
}

char *hashtable_get (struct hashtable *self, char *key, int key_length, int *v_len) {
    struct node *node;
    int v_index;
    v_index = hash(key_length) % self->vect_size;
    for (node = self->heads[v_index]; node != NULL; node = node->next) {
        if (k_equal(node->key, key, node->k_len, key_length)) {
            *v_len = node->v_len;
            return(node->value);
        }
    }
    *v_len = 11;
    return("non trovato");
}

int log_init (const char *filename) {
    int fd;
    fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd < 0) perror("open: ");
    return(fd);
}

void log_replay (int fd, struct hashtable *db) {
    char key[128], value[128];
    int k_len, v_len;
    while (1) {
       if (read(fd, &k_len, sizeof(int)) != sizeof(int)) break;
       if (read(fd, &v_len, sizeof(int)) != sizeof(int)) break;
       if (read(fd, key, k_len) !=k_len) break;
       if (read(fd, value, v_len) != v_len) break;

       hashtable_insert(db, key, value, k_len, v_len);
       printf("replay key\n");
       stampa(key, k_len);
    }
}

void log_insert (int fd, char *key, char *value, int k_len, int v_len) {
    write(fd, &k_len, sizeof(int));
    write(fd, &k_len, sizeof(int));
    write(fd, key, k_len);
    write(fd, value, v_len);
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
        value = memchr(buffer + 4, ' ',rd - 4) + 1;
        k_len = (value - key) -1;
        v_len = rd - 4 - k_len - 1;
        printf("key = ");
        stampa(key, k_len);
        printf("value = ");
        stampa(value, v_len);

        hashtable_insert(&db, key, value, k_len, v_len);
        log_insert(log_fd, key, value, k_len, v_len);
        memcpy(buffer, "storato\n", 8);
        rd = 8;

    } else if (memcmp(buffer, "get", 3) == 0) {
        char *g_key, *g_value;
        int g_key_len, v_len;
        g_key = buffer + 4;
        g_key_len = rd - 4;

        g_value = hashtable_get(&db, g_key, g_key_len, &v_len);
        memcpy(buffer, g_value, v_len);
        memcpy(buffer + v_len, "\n", 1);
        rd = v_len + 1;
    }
    printf(" - letto dal client %d: ", cfd);
    stampa(buffer, rd);
    write(cfd, buffer, rd);
    return(0);
}

int main (int argc, char **argv) {
    hashtable_init(&db, 16);
    log_fd = log_init("CaStore.log");
    log_replay(log_fd, &db);
    int srv_fd;
    srv_fd = create_server(11124);
    server_loop(srv_fd, client_echo);

    close(srv_fd);
    return(0);
}