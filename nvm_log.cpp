#include <cstdio>
#include <cstdint>
#include <libpmemobj/pool.h>
#include <libpmemobj/tx_base.h>
#include <libpmem.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <atomic>
#include <sys/mman.h>
#include <stdexcept>
#include <iostream>
#include "pragma_nvm.h"
#include "lib/PMRingBuffer.h"
#include "lib/PMTx.h"

constexpr const char *POOL_NAME = "/dev/shm/pmdk-simple-test-pmemobj";
constexpr const char *LAYOUT_NAME = "hello";
const char *DATA_BUF = "111111111111111111111111111111111111111111111111";
#define DATA_COUNT 100

using namespace std;
using namespace pragma_nvm;

int main() {

//  #pragma clang nvm nvmptr
//  int *np = pnvm;
//
//  int data = 10;
//  int *non_nvp = &data;
//  printf("np=%p\n", np);
//
//  #pragma clang nvm tx(ppool, _tx) nvmptrs(pnvm)
//  {
//    np[0] = np[0] + 1;
//  }

  return 0;
}
