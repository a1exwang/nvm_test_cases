#pragma once
#include "PMRingBuffer.h"
#include "PMAtomicArray.h"

namespace pragma_nvm {
  enum TxStatus {
    Done,
    UserWorking,
  };

  enum TxResult {
    Ok = 0,
    Failed = 1
  };

  class PMTx {
  public:
    PMTx(void *buf, uint64_t bufSize, void *pmemBase) :undoLog((uint8_t*)buf, bufSize), base(pmemBase) { }
    TxResult start() {
      if (getStatus() != TxStatus::Done) {
        return TxResult::Failed;
      }
      setStatus(TxStatus::UserWorking);

      return TxResult::Ok;
    }

    void add_direct(void *p, uint64_t len) {
      undoLog.enq(direct(p), p, len);
    }

    TxResult commit() {
      if (getStatus() != TxStatus::UserWorking) {
        return TxResult::Failed;
      }

      // just delete all logs
      undoLog.reset();

      return TxResult::Ok;
    }

    uint64_t direct(void *p) {
      uint64_t off = reinterpret_cast<uint64_t>(p) - reinterpret_cast<uint64_t>(base);
      return off;
    }

    TxResult abort() {
      if (getStatus() == TxStatus::Done) {
        return TxResult::Failed;
      }
      recover();
      return TxResult::Ok;
    }

    void recover() {
      if (getStatus() == TxStatus::Done) {
        // Nothing to recover
        return;
      }

      // First recover the ring buffer
      undoLog.recover();
      applyUndoLogs();
      setStatus(TxStatus::Done);
    }

  private:

    void applyUndoLogs() {
      while (!undoLog.isEmpty()) {
        auto *head = undoLog.head();
        applyUndoLog(head);
        undoLog.deq();
      }
    }
    void applyUndoLog(BufEntry *logEntry) {
      uint8_t *dst = (uint8_t*)base + logEntry->offset;
      printf("PMTx::applyUndoLog: offset=0x%lx, len=%ld, target=%p\n",logEntry->offset, logEntry->getDataLen(), (void*)dst);
      memcpy(dst, logEntry->data, logEntry->getDataLen());
      pmem_persist(dst, logEntry->getDataLen());
    }

    TxStatus getStatus() {
      return (TxStatus)status.get(0);
    }
    void setStatus(TxStatus s) {
      status.set(0, s);
    }
  private:
    PMAtomicArray<uint64_t, 1> status;
    PMRingBuffer undoLog;
    void *base;
  };
}