#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS    16

struct container {
    char buf_after[4];
    int buf_after_len;
    char *xstr;
    int xstr_len;
};

void stampa (const char *dato, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        printf("%c", dato[i]);
    }
    printf("\n");
}

int readfile_init (const char *filename) {
    int fd;
    fd = open(filename, O_RDONLY);
    if (fd < 0) perror("open");
    return(fd);
}

int read_linea (int fd, struct container *bag) {
    char buf_picol[4];
    ssize_t letti;

    if (bag->xstr != NULL) {
        free(bag->xstr);
        bag->xstr = NULL;
        bag->xstr_len = 0;
    }
    char *pos;
    int buf_picol_len;
    while(1) {
        if (bag->buf_after_len > 0) {
            memcpy(buf_picol, bag->buf_after, bag->buf_after_len);
            letti = bag->buf_after_len;
            bag->buf_after_len = 0;
        } else {
            if ((letti = read(fd, buf_picol, 4)) == 0) break;
        }

        if ((pos = memchr(buf_picol, '\n', letti)) != NULL) {
            buf_picol_len = pos - buf_picol;
        } else {
            buf_picol_len = letti;
        }
        bag->xstr = (char *)realloc(bag->xstr, bag->xstr_len + buf_picol_len);
        memcpy(bag->xstr + bag->xstr_len, buf_picol, buf_picol_len);
        bag->xstr_len += buf_picol_len;
        if (pos != NULL) {
            bag->buf_after_len = letti - ((pos - buf_picol) + 1);
            memcpy(bag->buf_after, pos + 1, bag->buf_after_len);
            break;
        }
    }
    return(bag->xstr_len > 0);
}

void stampa_csv (const char *str, int rd) {
    const char ch = ',';
    const char *value[MAX_ITEMS];
    int len[MAX_ITEMS];
    int i;
    const char *next;
    next = str;

    for (i = 0; i < MAX_ITEMS; ++i) {
        value[i] = next;
        next = memchr(next, ch, rd);
        if (next == NULL) {
            len[i] = rd;
            break;
        } else {
            next++;
            rd -= (next - value[i]);
            len[i] = (next - value[i]) -1;
        }
    }
    int items = i + 1;
    printf("items scanned %d\n", i);
    for (i = 0; i < items; ++i) {
        stampa(value[i], len[i]);
    }
}

int main (int argc, char **argv) {
    struct container bag;
    int fd;

    memset(&bag, 0, sizeof(struct container));

    fd = readfile_init("ManagersHalf.csv");
    while (read_linea(fd, &bag)) {
        printf("linea -> ");
        stampa(bag.xstr, bag.xstr_len);
        stampa_csv(bag.xstr, bag.xstr_len);
    }
    close(fd);
    return(0);
}

