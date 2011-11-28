#include "stdlib.h"

int strncmp (const char *s1, const char *s2, size_t n) {
  size_t count = 1;

  while (*s1 && *s2 && (*s1 == *s2) && (count < n)) {
    ++s1;
    ++s2;
    ++count;
  }

  return (*s1 - *s2);
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %d.\n", strncmp (argv[1], argv[2], atoi (argv[3])));
}
#endif
