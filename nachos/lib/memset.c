#include "stdlib.h"

void *memset (void *dest, int c, unsigned int n) {
  char *d = dest;

  while (n--) {
    d[n] = (char)c;
  }

  return (dest);
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[500];
  int i;
  for (i=0; i < 500; i++) {buffer[i] = '\0';}

  printf ("Result = %s.\n", memset (buffer, *argv[1], atoi (argv[2])));
}
#endif
