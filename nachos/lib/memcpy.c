#include "stdlib.h"

void *memcpy (void *dest, void *src, unsigned int n) {
  char *d = (char *)dest;
  char *s = (char *)src;
  
  while (n--) {
    d[n] = s[n];
  }

  return (dest);
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[500];
  int i;
  for (i=0; i < 500; i++) {buffer[i] = '\0';}

  printf ("Result = %s.\n", memcpy (buffer, argv[1], atoi (argv[2])));
}
#endif
