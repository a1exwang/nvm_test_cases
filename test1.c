int foo(int *p) {
  #pragma clang nvm
  int *npData = p;
  return 0;
}
