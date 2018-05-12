#pragma once

#include "PMTx.h"

namespace pragma_nvm {

//  struct PMemBlock {
//    uint64_t prev;
//    uint64_t next;
//  };

  template <uint64_t Size>
  class PMAlloc {
  public:
    PMAlloc(PMPool *pool) :pool(pool) {

    }
  private:
//    PMemBlock *freeListHead;
//    PMemBlock *objListHead;
    PMPool *pool;
  };

}