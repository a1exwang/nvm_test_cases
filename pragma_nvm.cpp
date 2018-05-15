#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "pragma_nvm.h"
#include "PMPool.h"
#include "PMTx.h"
#include "PMBlockAlloc.h"
#include <libpmem.h>
#include "PMLayout.h"


using namespace pragma_nvm;
using namespace std;

PMLayout *layout;

void *nvm_get_root() {
  return layout->getAlloc()->getRootDirect();
//  return layout->getAlloc();
}

bool nvm_init(const char *path, uint64_t size) {
  layout = new PMLayout(path, size);
  return true;
}

void nvm_deinit() {
  delete layout;
}

void *nvm_get_pool() {
  return layout->getPool();
}

void *nvm_get_tx() {
  return layout->getTx();
}

void *nvm_get_alloc() {
  return layout->getAlloc();
}

int nvm_add(void *ppool, void *_tx, void *ptr, uint64_t len) {
//  printf("nvm_add: start %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
//  printf("nvm_add: end %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  layout->getTx()->addDirect(ptr, len);
  return 0;
}

void *nvm_start_tx(void *ppool) {
  layout->getTx()->start();
//  printf("nvm_start_tx\n");
  return (void*)nullptr;
}

int nvm_commit(void *ppool, void *_tx) {
  layout->getTx()->commit();
//  printf("nvm_commit: ppoll=%p, tx=%p\n", ppool, _tx);
  return 0;
}

