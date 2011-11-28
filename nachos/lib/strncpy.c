#include "stdlib.h"

char *strncpy (char *s1, const char *s2, size_t n) {
  char *orig;
  size_t count = 0;

  orig = s1;

  while ((*s2) && (count < n)) {
    *(s1++) = *(s2++);
    ++count;
  }
printf ("n = %d, count = %d\n", n, count);
  if (count < n) {
    *s1 = '\0';
  }
  return orig;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[80];
  printf ("string = %s.\n", strncpy (buffer, argv[1], atoi (argv[2])));
}
#endif
