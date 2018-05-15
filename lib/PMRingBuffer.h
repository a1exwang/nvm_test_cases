#pragma once

#include <cassert>
#include <cstring>
#include "PMAtomicArray.h"
namespace pragma_nvm{

  #pragma pack(push, 8)
  struct BufEntry {
    uint64_t len;
    uint64_t offset;
    char data[0];

    char *getDataPtr() { return data; }
    uint64_t getDataLen() const { return len - (uint64_t)&((BufEntry*)nullptr)->data; }
    template <typename T>
    T *dataAs() {
      return reinterpret_cast<T*>(data);
    }
  };
  #pragma pack(pop)

  enum RingBufferStatus {
    Idle,
    WritingLogEntry,
    MovingLogTailOffset,
  };

  class BufMeta {
  public:
    void set(uint64_t head, uint64_t tail, uint64_t st) {
      uint64_t _a[3] = {head, tail, st};
      data.set<3>(_a);
    }
    uint64_t getHeadOff() { return data.get(0); }
    uint64_t getTailOff() { return data.get(1); }
    uint64_t getStatus() { return data.get(2); }
    void setHead(uint64_t val) { return data.set(0, val); }
    void setStatus(uint64_t val) { return data.set(2, val); }

  private:
    PMAtomicArray<uint64_t, 3> data;
  };

  class PMRingBuffer {
  public:
    PMRingBuffer(uint8_t *buf, uint64_t bufSize)
        :rawBuf(buf),
         bufSize(bufSize),
         buf((uint8_t*)rawBuf + sizeof(BufMeta)),
         meta(new (rawBuf) BufMeta()) {
    }
    BufEntry *head() {
      return (BufEntry*)(buf+meta->getHeadOff());
    }
    BufEntry *tail() {
      return (BufEntry*)(buf+meta->getTailOff());
    }

    void recover() {
      if (meta->getStatus() == Idle) {
        // do nothing
        return;
      }
      assert(
          meta->getStatus() == WritingLogEntry ||
          meta->getStatus() == MovingLogTailOffset
      );

      if (meta->getStatus() == WritingLogEntry) {
        // log entry partly written, just do nothing, the log is reverted
        meta->setStatus(Idle);
      } else if (meta->getStatus() == MovingLogTailOffset) {
        auto *curEntry = (BufEntry *) (buf + meta->getTailOff());
        // After setting the correct offset, the log is successfully appended
        meta->set(meta->getHeadOff(), meta->getTailOff() + curEntry->len, Idle);
      } else {
        assert(0);
      }
    }

    void deq() {
      // This is atomic
      meta->setHead(meta->getHeadOff() + head()->len);
    }

    const BufEntry *enq(const BufEntry *log) {
      enq(log->offset, log->data, log->getDataLen());
      return log;
    }
    void enq(uint64_t offset, const void *ptr, uint64_t dataLen) {
//      printf("PMRingBuffer::enq: offset=0x%lx, ptr=%p, len=%ld\n", offset, ptr, dataLen);
      if (meta->getStatus() != Idle) {
        recover();
      }
      assert(meta->getStatus() == Idle);

      // None -> WritingLogEntry
      meta->setStatus(WritingLogEntry);

      // Maybe non atomic, but it's ok
      auto *curLog = reinterpret_cast<BufEntry*>(buf + meta->getTailOff());
      curLog->offset = offset;
      curLog->len = sizeof(BufEntry) + dataLen;
      memcpy(curLog->data, ptr, dataLen);
      pmem_persist(curLog, curLog->len);

      // WritingLogEntry -> MovingLogTailOffset
      meta->setStatus(MovingLogTailOffset);

      // MovingLogTailOffset -> Idle
      recover();
    }

    bool isEmpty() const {
      return meta->getHeadOff() == meta->getTailOff();
    }

    void reset() {
      meta->set(0, 0, RingBufferStatus::Idle);
    }
  private:
    uint8_t *rawBuf;
    uint64_t bufSize;
    uint8_t *buf;
    BufMeta *meta;
  };
}


