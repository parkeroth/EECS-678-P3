#include "stdlib.h"

char *strrchr (const char *orig, int n) {
  register char *s1 = (char *) orig;

  while (*s1) {
    ++s1;
  }

  while ((s1 != orig) && (*s1 != (char) n)) {
    --s1;
  }

  return (*s1 != (char) n) ? NULL : s1;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %s.\n", strrchr (argv[1], (int) *argv[2]));
}
#endif
