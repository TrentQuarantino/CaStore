#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void stampa (const char *dato, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}

int main(int argc, char **argv) {
    int len;
    int diff;
    int diff1;
    int diff2;
    char *ret1;
    char *ret2;
    char *ret3;
    char *ret4;
    char *ret5;
    char *ret6;
    char str[] = "GET /?name=pier&email=mao%40mao.it&comments=fanculo HTTP/1.1";
    len = strlen(str);

    ret1 = memchr(str, '=', len) + 1;
    ret2 = memchr(str, '&', len) + 1;
    diff = (ret2 - ret1) -1;
    stampa(ret1, diff);

    ret3 = memchr(ret2, '=', len - (ret2 -str)) +1;
    ret4 = memchr(ret2 +1, '&', len - (ret2 -str)) +1;
    diff1 = (ret4 - ret3) -1;
    stampa(ret3, diff1);

    ret5 = memchr(ret4 + 1, '=', len - (ret4 -str)) + 1;
    ret6 = memchr(ret4 + 1, ' ', len - (ret4 -str)) + 1;
    diff2 = (ret6 -ret5) - 1;
    stampa(ret5, diff2);
}
