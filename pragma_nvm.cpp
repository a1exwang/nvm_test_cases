#include "pragma_nvm.h"


int nvm_add(void *ppool, void *tx, void *ptr, uint64_t len) {
  printf("nvm_add: start %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  printf("nvm_add: end %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  return 0;
}

void *nvm_start_tx(void *ppool) {
  printf("nvm_start_tx: let's start, ppool=%p\n", ppool);
  return (void*)nullptr;
}

int nvm_commit(void *ppool, void *tx) {
//  printf("nvm_commit: ppoll=%p, tx=%p, _pobj_errno=%d\n", ppool, tx, _pobj_errno);
  return 0;
}

