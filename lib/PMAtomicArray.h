#pragma once

#include <cstdint>
#include <libpmem.h>
#include <stdexcept>

namespace pragma_nvm {

template <typename T, uint64_t N>
class PMAtomicArray {
public:
  PMAtomicArray() :currentBuffer(0) {}
  T get(uint64_t idx) {
    return d[currentBuffer][idx];
  }

  void set(int idx, T item) {
    d[currentBuffer][idx] = item;
    pmem_persist(&d[currentBuffer][idx], sizeof(T));
  }
  void set(T *data, uint64_t n) {
    // First copy data to the other buffer. It may not be atomic.
    uint64_t nextBuffer = currentBuffer == 0 ? 1 : 0;
    for (int i = 0; i < n; ++i) {
      this->d[nextBuffer][i] = data[i];
    }
    pmem_persist(this->d[nextBuffer], sizeof(T) * n);

    // Then swap buffer. This one is atomic
    this->currentBuffer = nextBuffer;
    pmem_persist(&currentBuffer, sizeof(currentBuffer));
  }

  template <size_t M>
  void set(T data[M]) {
    if (M > N) {
      throw std::runtime_error("PMAtomicArray, data too large");
    }
    set(data, M);
  }
private:
  T d[2][N];
  uint64_t currentBuffer;
};
}
