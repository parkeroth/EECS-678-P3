char *strcat (char *s1, const char *s2) {
  char *orig;

  orig = s1;

  while (*(s1++));
  s1--;

  while (*s2) {
    *(s1++) = *(s2++);
  }
  *s1 = '\0';
  return orig;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("string = %s.\n", strcat (argv[1], argv[2]));
}
#endif
