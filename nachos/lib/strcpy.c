char *strcpy (char *s1, const char *s2) {
  char *orig = s1;

  while (*s2) {
    *(s1++) = *(s2++);
  }
  *s1 = '\0';
  return orig;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[80];
  printf ("string = %s.\n", strcpy (buffer, argv[1]));
}
#endif
