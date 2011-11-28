#include "stdlib.h"

char *strstr (const char *s1, const char *s2) {
  char *start = NULL;

  do {
    while (*s1 && *s2 && (*s1 != *s2)) {
      ++s1; 
    }
    start = (char *) s1;

    while (*s2 && (*s1 == *s2)) {
      ++s1;
      ++s2;
    }

    if (*s2) {
      start = NULL;
    }
  } while (*s1 && !start);

  return start;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %s.\n", strstr (argv[1], argv[2]));
}
#endif
