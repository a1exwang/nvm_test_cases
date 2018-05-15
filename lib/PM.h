#pragma once
#include "libpmem.h"
#include <cstdio>

namespace pragma_nvm {
  static void PMPersist(void *ptr, uint64_t size) {
//    printf("persist: ptr=%p: len=%ld\n", ptr, size);

    pmem_msync(ptr, size);
  }
}

