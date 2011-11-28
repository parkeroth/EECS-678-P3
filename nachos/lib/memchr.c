#include "stdlib.h"

void *memchr (void *src, int c, int n) {
  char *s = (char *)src;
  int i = 0;

  while ((i < n) && (s[i] != (char) c)) {
    i++;
  }

  if (i < n) {
    return (&s[i]);
  }
  return (NULL);
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %s.\n", memchr (argv[1], (int) *argv[2], atoi (argv[3])));
}
#endif
