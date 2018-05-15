#pragma once
#include "PMRingBuffer.h"
#include "PMPool.h"
#include "PM.h"

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

  class PMPool;
  class PMTx {
  public:
    // PM area
    struct PMLayout {
      PMAtomicArray<uint64_t, 1> status;
      char logBuffer[LogBufferSize];
      char _pad[951400 + 97152]; // allign to 2MiB
    };
  public:
    explicit PMTx(PMPool *pool, PMLayout *_)
        :_(_), pool(pool), undoLog((uint8_t*)_->logBuffer, sizeof(PMLayout)) {}

    TxResult start() {
      recover();
      if (getStatus() != TxStatus::Done) {
        return TxResult::Failed;
      }
      setStatus(TxStatus::UserWorking);

      return TxResult::Ok;
    }

    void addDirect(void *p, uint64_t len) {
      if (!inTx()) {
        throw std::runtime_error("not in tx");
      }
      undoLog.enq(pool->offset(p), p, len);
    }
    void tryAddDirect(void *p, uint64_t len) {
      if (!inTx()) {
        return;
      }
      if (p == nullptr)
        return;
      undoLog.enq(pool->offset(p), p, len);
    }
    void add(uint64_t off, uint64_t len) {
      if (!inTx()) {
        throw std::runtime_error("not in tx");
      }
      undoLog.enq(off, pool->direct(off), len);
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
      undoLog.recover(pool->getBase());
      applyUndoLogs();
      setStatus(TxStatus::Done);
    }

    bool inTx() {
      return getStatus() == TxStatus::UserWorking;
    }

    void reinit() {
      setStatus(TxStatus::Done);
      undoLog.reset();
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
      memcpy(dst, logEntry->data, logEntry->getDataLen());
      PMPersist(dst, logEntry->getDataLen());
    }

    TxStatus getStatus() {
      return (TxStatus)_->status.get(0);
    }
    void setStatus(TxStatus s) {
      _->status.set(0, s);
    }

  private:
    // VM area
    PMLayout *_;
    PMRingBuffer undoLog;
    PMPool *pool;
  };
}