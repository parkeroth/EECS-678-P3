#include "stdlib.h"

char *strncat (char *s1, const char *s2, size_t n) {
  char *orig;
  int count = 0;

  orig = s1;

  while (*(s1++));
  s1--;

  while ((*s2) && (count < n)) {
    *(s1++) = *(s2++);
    ++count;
  }
  *s1 = '\0';
  return orig;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[80];
  strcpy (buffer, argv[1]);
  printf ("string = %s.\n", strncat (buffer, argv[2], atoi (argv[3])));
}
#endif
