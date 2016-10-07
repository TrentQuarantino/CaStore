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
   char first[256];
   char last[256];
   char sub[4];
   char substitute[] = "@";
   char dest[256];
   char *ret;
   int len,len1,len2;
   int diff;

   len = strlen(src);
   printf("len %d\n", len);
   ret = memchr(src, '%', len);
   len1 = strlen(ret);
   printf("ret len %d\n", len1);
   printf("ret = ");
   printa(ret, len1);
   diff = ret - src;
   printf("diff %d\n", diff);
   memcpy(last, ret + 3, len - diff);
   memcpy(first, src, diff);
   memcpy(sub, ret, 3);
   //strcat(last, src);
   len1 = strlen(last);
   printf("last = ");
   printa(last, len-diff);
   memcpy(dest, first, strlen(first));
   printf("dest %s\n", dest);
   memcpy(dest + strlen(first), substitute, 1);
   printf("dest %s\n", dest);
   memcpy(dest + strlen(first) + 1, last, len1+4);
   printf("dest %s\n",dest);
   return(0);
}