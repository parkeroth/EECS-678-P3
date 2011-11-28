int atoi (const char *s) {
  int i = 0, neg = 1, done = 0;

  if (*s == '-') {
    neg = -1;
    ++s;
  }
  while (!done && *s) {
    switch (*s) {
    case '0' :
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      i = i * 10;
      i += (int) (*s - '0');
      ++s;
      break;
    default:
      done = 1;
    }
  }

  return (i * neg);
}


#ifdef DEBUG
int main (int argc, char *argv[]) {
  printf ("Result = %d.\n", atoi (argv[1]));
}
#endif
