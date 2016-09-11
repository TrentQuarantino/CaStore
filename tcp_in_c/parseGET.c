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
    char *ret1;
    char *ret2;
    char str[] = "GET /?name=pier&email=mao%40mao.it&comments=fanculo HTTP/1.1";
    len = strlen(str);
    printf("len %d\n", len -1);
    ret1 = memchr(str, '=', len);
    ret2 = memchr(str, '&', len);
    diff = ret2 - ret1;
    stampa(ret1, diff);
}
