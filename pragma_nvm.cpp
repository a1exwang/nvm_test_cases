#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "pragma_nvm.h"
#include "PMPool.h"
#include "PMTx.h"
#include "PMBlockAlloc.h"
#include <libpmem.h>
#include <chrono>
#include "PMLayout.h"


using namespace pragma_nvm;
using namespace std;

PMLayout *layout;

void *nvm_get_root() {
  return layout->getAlloc()->getRootDirect();
//  return layout->getAlloc();
}

bool nvm_init(const char *path, uint64_t *size) {
  layout = new PMLayout(path, *size);
  *size = layout->getSize();
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
//  printf("nvm_add: start, ptr=%p, len=%ld\n", ptr, len);
//  printf("nvm_add: end, ptr=%p, len=%ld\n", ptr, len);
  if (!layout->getPool()->isNvmPtr(ptr)) {
    pragma_nvm::PMPersist(ptr, len);
  } else {
    layout->getTx()->addDirect(ptr, len);
  }
//  auto base = layout->getPool()->offset(ptr);
//  using namespace std::chrono;
//  std::chrono::milliseconds ms = duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch());
//  printf("x %ld %ld %ld\n", ms.count(), base, len);
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

