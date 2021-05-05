int assert(char *msg, int a, int b) {
  if (a != b) printf("%s: %d != %d\n", msg, a, b);
  return 0;
}
