#include <libpmemobj++/pool.hpp>
#include "libpmemobj++/detail/pexceptions.hpp"
#include "libpmemobj/tx_base.h"

extern "C" {

int nvm_add(void *ppool, void *tx, void *ptr, uint64_t len) {
  pmemobj_tx_add_range(pmemobj_oid(ptr), 0, len);
//  printf("nvm_add: %p, tx=%p, ptr=%p, len=%ld\n", ppool, tx, ptr, len);
  return 0;
}

void *nvm_start_tx(void *ppool) {
  auto pool = (pmem::obj::pool_base*)ppool;
  if (pmemobj_tx_begin(pool->get_handle(), nullptr, TX_PARAM_NONE) != 0) {
    printf("nvm_start_tx: failed ppoll=%p\n", ppool);
    return nullptr;
  }
//    throw pmem::transaction_error("failed to start transaction");

//  printf("nvm_start_tx: ok ppoll=%p\n", ppool);
  return nullptr;
}

int nvm_commit(void *ppool, void *tx) {
  auto stage = pmemobj_tx_stage();

  if (stage == TX_STAGE_WORK) {
    pmemobj_tx_commit();
  } else if (stage == TX_STAGE_ONABORT) {
    (void)pmemobj_tx_end();
//    throw pmem::transaction_error("transaction aborted");
    return 0;
  } else if (stage == TX_STAGE_NONE) {
//    throw pmem::transaction_error("transaction ended prematurely");
    return 0;
  }

  pmemobj_tx_end();

//  printf("nvm_commit: ppoll=%p, tx=%p\n", ppool, tx);
  return 0;
}
}
