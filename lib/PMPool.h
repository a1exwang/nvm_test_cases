#pragma once

namespace pragma_nvm {
  class PMPool {
  public:
    PMPool(void *base, uint64_t size)
        :base(base), size(size) {
    }

    uint64_t offset(void *p) {
      if (p == nullptr) {
        return 0;
      }
      uint64_t off = reinterpret_cast<uint64_t>(p) - reinterpret_cast<uint64_t>(base);
      return off;
    }
    void *direct(uint64_t off) {
      if (off == 0) {
        return nullptr;
      }
      return reinterpret_cast<uint8_t*>(base) + off;
    }
    template <typename T>
    T *directAs(uint64_t off) {
      return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(base) + off);
    }
    void *getTxMetadataBuf() {
      return base;
    }
//    uint64_t getTxMetadataSize() {
//      return txMetadataSize;
//    }
    void *getAllocPool() {
      return (uint8_t*)base + txMetadataSize;
    }
    uint64_t getAllocPoolSize() {
      return size - txMetadataSize;
    }
    void *setTxMetadata(uint64_t size) {
      txMetadataSize = size;
      return base;
    }
  private:
    void *base;
    uint64_t size;
    uint64_t txMetadataSize;
  };
}