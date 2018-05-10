#include "../lib/PMRingBuffer.h"
#include <iostream>


using namespace pragma_nvm;
using namespace std;

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

void TestPMRingBuffer() {
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

  delete [] buf;
}
