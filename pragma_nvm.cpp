#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "pragma_nvm.h"
#include "PMPool.h"
#include "PMTx.h"
#include "PMBlockAlloc.h"
#include <libpmem.h>


using namespace pragma_nvm;
using namespace std;

char *base;
uint64_t totalSize;
PMPool *pool;
PMTx *tx;
TheAlloc *theAlloc;

void *nvm_get_root() {
  return theAlloc->getRootDirect();
}

bool nvm_init(const char *path, uint64_t size) {
  struct stat buffer;
  int status = stat(path, &buffer);
  int fd;
  if(status == 0) {
    fd = open(path, O_RDWR, 0666);
    if (fd < 0) {
      perror("open");
      return false;
    }

    totalSize = (uint64_t)buffer.st_size;
  } else {
    fd = open(path, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
      perror("open");
      return false;
    }

    /* allocate the pmem */
    totalSize = (size*4)/4096 * 4096;
    if ((errno = posix_fallocate(fd, 0, totalSize)) != 0) {
      perror("posix_fallocate");
      return false;
    }
  }

  base = (char*)mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (base == MAP_FAILED) {
    perror("mmap");
    return false;
  }

  if ((errno = close(fd)) != 0) {
    perror("close");
    return false;
  }

  pool = new PMPool(base, totalSize);
  tx = new PMTx(pool);
  theAlloc = new TheAlloc(pool);

  if (!tx->isInitialized()) {
    theAlloc->reinit();
    tx->setInitialized();
  }

  return true;
}

void nvm_deinit() {
  delete theAlloc;
  delete tx;
  delete pool;

  munmap(base, totalSize);
}

void *nvm_get_pool() {
  return pool;
}

void *nvm_get_tx() {
  return tx;
}

void *nvm_get_alloc() {
  return theAlloc;
}

int nvm_add(void *ppool, void *_tx, void *ptr, uint64_t len) {
//  printf("nvm_add: start %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
//  printf("nvm_add: end %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  tx->addDirect(ptr, len);
  return 0;
}

void *nvm_start_tx(void *ppool) {
  tx->start();
  return (void*)nullptr;
}

int nvm_commit(void *ppool, void *_tx) {
  tx->commit();
//  printf("nvm_commit: ppoll=%p, tx=%p\n", ppool, tx);
  return 0;
}

