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

struct node {
    struct node *next;
    char key[20];//int key;
    char value[128];
    int v_len;
    int k_len;
};

struct hashtable {
    struct node **heads;
    int nodi_size;
    pthread_mutex_t mutex;
};

struct hashtable db;
int log_fd;

struct client {
  int fd;
};

char html[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>asino"
              "<img src='http://www.w3schools.com/images/lamp.jpg'></body></html>";

void hashtable_init (struct hashtable *self, int vect_size) {
    pthread_mutex_init(&(self->mutex), NULL);
    struct node **buf;//vettore di puntatori ai nodi
    buf = (struct node **) malloc(vect_size *  sizeof(struct node *));
    int i;
    self->nodi_size = vect_size;
    self->heads = buf;
    for (i = 0; i < vect_size; i++) {
        self->heads[i] = NULL;
    }
}

int hash (int vect_size) {
    return(vect_size);
}

int k_equal (char *node_key, char *key, int node_k_len, int key_length) {
    if (node_k_len != key_length) return (0);
    if (memcmp(node_key, key, key_length) == 0) return (1);
    return (0);
}

void hashtable_insert (struct hashtable *self, char *key, char *value, int key_length, int value_length) {
    int v_index;
    v_index = hash(key_length) % self->nodi_size;
    pthread_mutex_lock(&(self->mutex));
    struct node *node;
    for (node = self->heads[v_index]; node != NULL; node = node->next) {
      if (k_equal(node->key, key, node->k_len, key_length)) {
        node->v_len = value_length;
        node->k_len = key_length;
        memcpy(node->key, key, key_length);
        memcpy(node->value, value, value_length);
        pthread_mutex_unlock(&(self->mutex));
        return;
      }

    }
    node = (struct node *) malloc( sizeof(struct node));
    node->next = self->heads[v_index];
    memcpy(node->key, key,key_length);
    node->v_len = value_length;
    node->k_len = key_length;
    //node->value = value; col pointer
    memcpy(node->value, value, value_length);
    self->heads[v_index] = node;
    pthread_mutex_unlock(&(self->mutex));
}

char *hashtable_get (struct hashtable *self, char *key, int key_length, int *v_len) {
    int v_index;
    v_index = hash(key_length) % self->nodi_size;
    pthread_mutex_lock(&(self->mutex));
    struct node *node;
    for (node = self->heads[v_index]; node != NULL; node = node->next) {

        if (k_equal(node->key, key, node->k_len, key_length)) {
            *v_len = node->v_len;
            pthread_mutex_unlock(&(self->mutex));
            return(node->value);
        }
    }
    *v_len = 11;
    pthread_mutex_unlock(&(self->mutex));
    return("non trovato");
}

void client_loop (int cfd);
void *client_thread (void *arg);
void stampa (const char *dato, int len);
int log_init ( const char *filename);
void log_replay (int fd, struct hashtable *db);
void log_insert (int log_fd, char *key, char *value, int k_len, int v_len);

int main (int argc, char **argv) {

  int srv_fd, cfd;


  hashtable_init(&db, 16);
  log_fd = log_init("CaStore.log");
  log_replay(log_fd, &db);

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
    while (1) {
        cfd = accept(srv_fd, (struct sockaddr*) NULL, NULL);
        setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &yep, sizeof(int));
        printf("Accettato il client %d\n", cfd);

        struct client *cl =(struct client *)malloc(sizeof(struct client));
        pthread_t tred_id;
        cl->fd = cfd;
        pthread_create(&tred_id, NULL, client_thread, cl);
    }
  close(srv_fd);
  return(0);
}

void stampa (const char *dato, int len) {
  int i;

  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

void client_loop (int cfd) {
    char buffer[128];

    while (1) {
        ssize_t rd;
        if ((rd = read(cfd, buffer, 128)) <= 0) {
        perror("read()");
        break;
        }
        if (memcmp(buffer, "scus", 4) == 0) {
            memcpy(buffer, "figur\n", 6);
            rd = 6;
        } else if (memcmp(buffer, "chiudi", 6) == 0) {
            break;
        } else if (memcmp(buffer, "put", 3) == 0) {
            printf("Byte letti %d\n", rd);

            char *key, *value;
            int k_len, v_len;

            key = buffer + 4;
            printf("key = %p\n",key);

            value = memchr(buffer + 4, ' ', rd - 4) +1;
            k_len =(value - key) - 1;
            v_len = rd - 4 - k_len - 1;
            printf("Key = ");
            stampa(key, k_len);
            printf("Value = ");
            stampa(value, v_len);
          /*int i_key;
          i_key = strtol(key, NULL, 10);
          printf("integer key = %d\n", i_key);*/
            hashtable_insert(&db, key, value,k_len, v_len);
            log_insert(log_fd, key, value,k_len, v_len);
            memcpy(buffer, "storato\n", 8);
            rd = 8;
        } else if (memcmp(buffer, "get", 3) == 0) {
            char *g_key, *g_value;
            int  g_key_len, v_len;
            g_key = buffer + 4;
            g_key_len = rd - 4;
          //ig_key = strtol(g_key, NULL, 10);

            g_value = hashtable_get(&db, g_key, g_key_len, &v_len);
            memcpy(buffer, g_value, v_len);
            memcpy(buffer + v_len, "\n", 1);
            rd = v_len + 1;
        } else if (memcmp(buffer, "GET", 3) == 0) {
            char xbuffer[4096];
            read(cfd, xbuffer, sizeof(xbuffer));
            memcpy(buffer, html, strlen(html));
            rd = strlen(html);
        } else {
            memcpy(buffer + rd, "culo\n", 5);
            printf("il buffer %s\n", buffer);
            rd += 5;
        }
        if (write(cfd, buffer, rd) != rd) {
            perror("write()");
            break;
        }
    }
    close(cfd);
}

void *client_thread (void *arg) {
  struct client *cl = (struct client *)arg;
  client_loop(cl->fd);
  free(cl);
  return(NULL);
}

int log_init ( const char *filename) {
  int fd;
  fd = open(filename, O_CREAT | O_RDWR, 0644);
  if (fd < 0) perror("open");
  return(fd);
}

void log_replay (int fd, struct hashtable *db) {
  char key[128], value[128];
  int k_len,v_len;
  while(1) {
    if (read(fd, &k_len, sizeof(int)) != sizeof(int)) break;
    if (read(fd, &v_len, sizeof(int)) != sizeof(int)) break;
    if (read(fd, key, k_len) != k_len) break;
    if (read(fd, value, v_len) != v_len) break;

    hashtable_insert(db, key, value, k_len, v_len);
    printf("replay key\n ");
    stampa(key, k_len);
  }
}

void log_insert (int fd, char *key, char *value, int k_len, int v_len) {
  write(fd, &k_len, sizeof(int));
  write(fd, &v_len, sizeof(int));
  write(fd, key, k_len);
  write(fd, value, v_len);
}

