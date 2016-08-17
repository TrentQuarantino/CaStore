//struct libro lassie
//struct libro libri[N_LIBRI];
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  char sendline[100];
  char buffer [100];

  int c = 0;
  int d = 0;
  int i = 0;
  memset(buffer, 0, sizeof(buffer));
  while(i <= 3) {
    fgets(sendline, 100, stdin);
    d = strlen(sendline) - 1;

    printf("%d\n", d);
    if (d <= 0) break;

    memcpy (buffer + c , sendline, d);
    c += d;
    printf("%d\n",c);
    i++;
  }
  printf ("buffer = %s\n", buffer);
  char cmd_key[32];
  memcpy(cmd_key, "put ", 4);
  fgets(cmd_key + 4, 32, stdin);
  printf("cmd & key = %s\n", cmd_key);
  return(0);
}