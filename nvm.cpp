#include <cstdio>
#include <cstdint>
#include <libpmemobj/pool.h>
#include <libpmemobj/tx_base.h>
#include <cerrno>
#include <cstdlib>


constexpr const char *POOL_NAME = "/dev/shm/pmdk-simple-test-pmemobj";
constexpr const char *LAYOUT_NAME = "hello";
const char *DATA_BUF = "111111111111111111111111111111111111111111111111";
#define DATA_COUNT 100

using namespace std;

extern "C" {
PMEMobjpool *pool;
int *val;
struct Root {
  int buf[64];
};

int nvm_add(void *ppool, void *tx, void *ptr, uint64_t len) {
  printf("nvm_add: start %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  pmemobj_tx_add_range(pmemobj_oid(ptr), 0, len);
  printf("nvm_add: end %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  return 0;
}

void *nvm_start_tx(void *ppool) {
  printf("nvm_start_tx: let's start, ppool=%p\n", ppool);
  PMEMobjpool *mypool = (PMEMobjpool*)ppool;

  jmp_buf _tx_env;
  enum pobj_tx_stage _stage;
  int _pobj_errno;
  if (_setjmp(_tx_env)) {
    (*__errno_location()) = pmemobj_tx_errno();
  } else {
    _pobj_errno = pmemobj_tx_begin(mypool, _tx_env, TX_PARAM_NONE, TX_PARAM_NONE);
    if (_pobj_errno) (*__errno_location()) = _pobj_errno;
  }


  while ((_stage = pmemobj_tx_stage()) != TX_STAGE_NONE) {
    printf("nvm_start_tx: stage: %d\n", _stage);
    switch (_stage) {
      case TX_STAGE_WORK: {
        printf("nvm_start_tx: tx start ok, ppool=%p\n", ppool);
        return (void*)1;
      }
      default:
        pmemobj_tx_process();
        break;
    }
  }
  _pobj_errno = pmemobj_tx_end();
  if (_pobj_errno)
    (*__errno_location()) = _pobj_errno;

  printf("nvm_start_tx: unexpected tx end! ppool=%p\n", ppool);
  return (void*)nullptr;
}

int nvm_commit(void *ppool, void *tx) {
  // Finish last unfinished process
  pmemobj_tx_process();

  int _pobj_errno;
  enum pobj_tx_stage _stage;
  while ((_stage = pmemobj_tx_stage()) != TX_STAGE_NONE) {
    printf("nvm_commit: stage: %d\n", _stage);
    switch (_stage) {
      case TX_STAGE_WORK: {
        // ERROR:
        fprintf(stderr, "nvm_commit: invalid state: TX_STAGE_WORK");
        abort();
      }
      default:
        pmemobj_tx_process();
        break;
    }
  }
  _pobj_errno = pmemobj_tx_end();
  if (_pobj_errno)
    (*__errno_location()) = _pobj_errno;

  printf("nvm_commit: ppoll=%p, tx=%p, _pobj_errno=%d\n", ppool, tx, _pobj_errno);
  return 0;
}

}

void *_tx;


void nvm_write(void *ppool, int *pnvm) {
//  Root *proot = (Root*)pnvm;

  #pragma clang nvm nvmptr
  int *np = pnvm;

  int data = 10;
  int *non_nvp = &data;
  printf("np=%p\n", np);

  #pragma clang nvm tx(ppool, _tx) nvmptrs(pnvm)
  {
    np[0] = np[0] + 1;
  }
}

int main() {
  // nvm data ptr
  char data[128];
  val = (int*)&data[32];

  PMEMobjpool *pool = pmemobj_create(POOL_NAME, LAYOUT_NAME, 1024 * 1048576, 0666);
  if (pool == nullptr) {
    pool = pmemobj_open(POOL_NAME, LAYOUT_NAME);
  }

  PMEMoid root = pmemobj_root(pool, sizeof (struct Root));
  struct Root *rootp = reinterpret_cast<Root*>(pmemobj_direct_inline(root));

  printf("proot->buf[0]: %d\n", rootp->buf[0]);

  nvm_write(pool, (int*)rootp);
  return 0;
}
