#include "stdlib.h"

char *strchr (const char *s, int n) {
  register char *s1 = (char *) s;

  while (*s1 && (*s1 != (char) n)) {
    ++s1;
  }

  return *s1 ? s1 : NULL;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %s.\n", strchr (argv[1], (int) *argv[2]));
}
#endif
