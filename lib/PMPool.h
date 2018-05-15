#pragma once
#include <stdexcept>
#include "PMAtomicArray.h"

namespace pragma_nvm {
  struct PMTx;

  constexpr const uint64_t PMFileMagic = 0xdeadbeefabcdef01;
  class PMPool {
  public:

    struct PMLayout {
      PMAtomicArray<uint64_t, 2> magicAndSize;
    };

    PMPool(PMLayout *_):_(_) {}

    uint64_t offset(void *p) {
      if (p == nullptr) {
//        throw std::runtime_error("nullptr");
        return 0;
      }
      uint64_t off = reinterpret_cast<uint64_t>(p) - reinterpret_cast<uint64_t>(getBase());
      if (off > getSize()) {
        throw std::runtime_error("out of bounds");
      }
      return off;
    }
    void *direct(uint64_t off) {
      if (off == 0) {
//        throw std::runtime_error("nullptr");
        return nullptr;
      }
      if (off > getSize()) {
        throw std::runtime_error("out of bounds");
      }
      return reinterpret_cast<uint8_t*>(getBase()) + off;
    }

    template <typename T>
    T *directAs(uint64_t off) {
      return reinterpret_cast<T*>(direct(off));
    }

    bool isInitialized() {
      return _->magicAndSize.get(0) == PMFileMagic;
    }

    void setInitialized(uint64_t size) {
      _->magicAndSize.set(1, size);
      _->magicAndSize.set(0, PMFileMagic);
    }
    void *getBase() {
      return &_->magicAndSize;
    }

  private:
    uint64_t getSize() {
      return _->magicAndSize.get(1);
    }
    PMLayout *_;
    uint64_t txMetadataSize;
  };
}