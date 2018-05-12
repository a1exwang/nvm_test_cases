#pragma once
#include "PMRingBuffer.h"
#include "PMAtomicArray.h"
#include "PMPool.h"

namespace pragma_nvm {
  enum TxStatus {
    Done,
    UserWorking,
  };

  enum TxResult {
    Ok = 0,
    Failed = 1
  };

  constexpr const uint64_t LogBufferSize = 1048576; // 1MiB

  class PMTx {
  public:
    explicit PMTx(PMPool *pool)
        :_((decltype(_))pool->setTxMetadata(sizeof(*_))),
         pool(pool),
         undoLog((uint8_t*) _->logBuffer, LogBufferSize) {
      }

    TxResult start() {
      if (getStatus() != TxStatus::Done) {
        return TxResult::Failed;
      }
      setStatus(TxStatus::UserWorking);

      return TxResult::Ok;
    }

    void addDirect(void *p, uint64_t len) {
      undoLog.enq(pool->offset(p), p, len);
    }

    TxResult commit() {
      if (getStatus() != TxStatus::UserWorking) {
        return TxResult::Failed;
      }

      // just delete all logs
      undoLog.reset();

      return TxResult::Ok;
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
      auto *dst = pool->directAs<uint8_t>(logEntry->offset);
      printf("PMTx::applyUndoLog: offset=0x%lx, len=%ld, target=%p\n",logEntry->offset, logEntry->getDataLen(), (void*)dst);
      memcpy(dst, logEntry->data, logEntry->getDataLen());
      pmem_persist(dst, logEntry->getDataLen());
    }

    TxStatus getStatus() {
      return (TxStatus)_->status.get(0);
    }
    void setStatus(TxStatus s) {
      _->status.set(0, s);
    }
  private:
    // PM area
    struct _ano {
      PMAtomicArray<uint64_t, 1> status;
      char logBuffer[LogBufferSize] = {0};
    } *_;
    // VM area
    PMPool *pool;
    PMRingBuffer undoLog;
  };
}