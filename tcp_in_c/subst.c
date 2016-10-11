#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void printa (const char *dato, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    printf("%c", dato[i]);
  }
  printf("\n");
}
struct dati {
  char dato[256];
  int lung;
};

void funz (struct dati *d, const char *src) {
   char substitute[] = "@";
   //char d->dato[256];
   char *ret;
   int len;
   int diff;

   len = strlen(src);
   ret = memchr(src, '%', len);
   diff = ret - src;

   memcpy(d->dato, src, diff);
   printa(d->dato, diff);
   memcpy(d->dato + diff, substitute, 1);
   printa(d->dato, diff +1);
   memcpy(d->dato + diff + 1, ret+3, len-diff-3);
   printa(d->dato, len - 3 +1);
   d->lung = len  - 3 + 1;
}
int main () {
  struct dati d;
   const char src[256] = "batmanandrobin%40yahoo.com";
   funz(&d, src );
   printa(d.dato, d.lung);
   return(0);
}
