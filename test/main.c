int fib(int n) {
  if (n <= 0) return 0;
  if (n == 1) return 1;
  return fib(n - 2) + fib(n - 1);
}

int main() {
  int i;
  for (i = 0; i <= 10; i = i + 1) {
    printf("fib(%d) = %d\n", i, fib(i));
  }
  return 0;
}
