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


constexpr const char *POOL_NAME = "/dev/shm/pmdk-simple-test-pmemobj";
constexpr const char *LAYOUT_NAME = "hello";
const char *DATA_BUF = "111111111111111111111111111111111111111111111111";
#define DATA_COUNT 100

using namespace std;
using namespace pragma_nvm;

BufEntry *newLogEntry(size_t dataSize) {
  auto ret = reinterpret_cast<BufEntry*>(malloc(sizeof(BufEntry) + dataSize));
  return new (ret) BufEntry;
}

template <typename T, typename... Args>
BufEntry *newLogEntryVolatile(Args &&... args) {
  auto len = sizeof(BufEntry) + sizeof(T);
  auto ret = reinterpret_cast<BufEntry*>(malloc(len));
  new (ret) BufEntry();
  new (ret->dataAs<T>()) T(args...);
  ret->len = len;
  return ret;
}

struct Payload {
  int a;
  double b;
  char data[32];
};

int main() {

  uint64_t bufSize = 1048576;
  auto *buf = new uint8_t[bufSize];
  PMRingBuffer ringBuffer(buf, bufSize);


//  nvm_add(nullptr, nullptr, ptr, len);
  for (int i = 0; i < 5; ++i) {
    delete ringBuffer.enq(newLogEntryVolatile<int>(i));
  }
  delete ringBuffer.enq(newLogEntryVolatile<Payload>(Payload{1, 2, {'a', 'b', 'c', '0'}}));

  for (int i = 0; i < 5; ++i) {
    cout << "i = " << i << ", data = " << *ringBuffer.head()->dataAs<int>() << endl;
    ringBuffer.deq();
  }

  cout << "last data: " << ringBuffer.head()->dataAs<Payload>()->data << endl;
  ringBuffer.deq();

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
