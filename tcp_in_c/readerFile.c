#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUF 128

int readfile_init ( const char *filename) {
    int fd;
    fd = open(filename, O_RDONLY);
    if (fd < 0) perror("open");
    return(fd);
}

void readonly (int fd) {
    char html[MAX_BUF];
    int letten;
    int tot = 0;
    while ((letten = read(fd, html, MAX_BUF)) > 0) {
        tot += letten;
        html[letten] = '\0';
        write(1, html, letten);
    }
    printf("tot %d\n", tot);
}

int main(int argc, char **argv) {
    int fd;
    fd = readfile_init("pag_html");
    readonly(fd);
    close(fd);
}
