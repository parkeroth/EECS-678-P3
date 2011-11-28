int strcmp (const char *s1, const char *s2) {
  while (*s1 && *s2 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }

  return (*s1 - *s2);
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %d.\n", strcmp (argv[1], argv[2]));
}
#endif
