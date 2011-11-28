#include "stdlib.h"

size_t strlen (const char *ptr) {
  size_t cnt = 0;

  while (*(ptr++)) {
    ++cnt;
  }
  return cnt;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("strlen=%d\n", strlen (argv[1]));
}
#endif
