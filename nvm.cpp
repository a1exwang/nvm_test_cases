#include <cstdio>


#define DATA_COUNT 100

#pragma clang nvm_ptr
int *val;

void nvm_write(int *pnvm) {
  #pragma clang nvm_tx nvm_ptrs(pnvm)
  {
    *pnvm = 0;
    for (int i = 0; i < DATA_COUNT; i++) {
      *pnvm += i;
    }
  }
}

void nvm_read(int *pnvm) {
  for (int i = 0; i < DATA_COUNT; i++) {
    printf("%d\n", pnvm[i]);
  }
}
