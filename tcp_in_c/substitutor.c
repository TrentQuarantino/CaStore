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
int main () {
   const char src[256] = "mao%40mao.it";
   char substitute[] = "@";
   char dest[256];
   char *ret;
   int len;
   int diff;

   len = strlen(src);
   ret = memchr(src, '%', len);
   diff = ret - src;

   memcpy(dest, src, diff);
   printa(dest, diff);
   memcpy(dest + diff, substitute, 1);
   printa(dest, diff +1);
   memcpy(dest + diff + 1, ret+3, len-diff-3);
   printa(dest, len - diff +1);
   return(0);
}