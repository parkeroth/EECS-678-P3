char *itoa (int num, char *buffer) {
  int index = 0, tmp, found = 0;

  if (num < 0) {
    buffer[index++] = '-';
    num = -num;
  }
  if (num == 0) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }
  for (tmp = 1000000000; tmp > 0; tmp /= 10) {
    if (num >= tmp) {
      buffer[index++] = '0' + (num / tmp);
      num -= (num / tmp) * tmp;
      found = 1;
    } else if (found) {
      buffer[index++] = '0';
    }
  }
  buffer[index] = '\0';
  return buffer;
}

#ifdef DEBUG
int main (int argc, char *argv[]) {
  char buffer[80];
  itoa (atoi (argv[1]), buffer);

  printf ("Number: %s\n", buffer);
}
#endif
