#ifdef DEBUG
#define ConsoleInput  0  
#define Read read
#endif

#include "syscall.h"
#include "stdlib.h"

char *gets (char *s) {
  int i = 0;
  int ret;

  do {
    ret = Read (ConsoleInput, &(s[i]), 1); 
  } 
  while ((ret == 1) && (s[i++] != '\n'));

  if (ret == 1) {
    s[i-1] = '\0';
  } else {
    s[i] = '\0';
  }
  
  return s;
}

#ifdef DEBUG 
int main (int argc, char *argv[]) { 
  char buffer[80];
  printf ("Result = %s.\n", gets (&buffer));
} 
#endif 
