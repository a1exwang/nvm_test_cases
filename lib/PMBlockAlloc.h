#pragma once

#include "PMPool.h"
#include "PMTx.h"
#include "PMAtomicArray.h"
#include <cstdint>

namespace pragma_nvm {
  class PMPool;

  template <uint64_t Size>
  class PMBlockAlloc {
  public:

    struct PMBlock {
      uint64_t prev;
      uint64_t next;
//      PMAtomicArray<uint64_t, 2> _;
//      uint64_t getPrev() { return _.get(0); }
//      uint64_t getNext() { return _.get(1); }
//      uint64_t setPrev(uint64_t prev) { return _.set(0, prev); }
//      uint64_t setNext(uint64_t next) { return _.set(1, next); }
//      void set(uint64_t prev, uint64_t next) { _.set({prev, next}); }

      char data[Size];
    };
    struct PMLayout {
      PMBlock freeListHead;
      PMBlock objListHead;
      char allocPool[0];
    };

    PMBlockAlloc(PMPool *pool, PMLayout *_, uint64_t allocPoolSize)
        :pool(pool), _(_), allocPoolSize(allocPoolSize) { }

    void reinit() {
      // First three blocks for freeListHead and objListHead;
      uint64_t blockCount = (allocPoolSize-2*sizeof(PMBlock)) / sizeof(PMBlock);
      memset(_->allocPool, 0, 2*sizeof(PMBlock));

      _->objListHead.prev = _->objListHead.next = pool->offset(&_->objListHead);
      _->freeListHead.prev = _->freeListHead.next = pool->offset(&_->freeListHead);

      for (int i = 0; i < blockCount; ++i) {
        PMBlock *block = (PMBlock*)_->allocPool + 2 + i;
        listAttach(&_->freeListHead, block);
      }
    }

    void *allocDirect(PMTx *tx) {
      if (!tx->inTx()) {
        throw std::runtime_error("not in tx");
      }

      if (_->freeListHead.next == _->freeListHead.prev) {
        return nullptr;
      }

      PMBlock *targetBlock = pool->template directAs<PMBlock>(_->freeListHead.prev);

      listDetachTx(targetBlock, tx);
      listAttachTx(&_->objListHead, targetBlock, tx);
      return targetBlock->data;
    }

    template <typename T>
    T *allocAs(PMTx *tx) {
      return (T*)allocDirect(tx);
    }

    void freeDirect(PMTx *tx, void *data) {
      freeBlock(tx, (PMBlock*)((char*)data-((PMBlock*)nullptr)->data));
    }

    void freeBlock(PMTx *tx, PMBlock *block) {
      if (!tx->inTx()) {
        throw std::runtime_error("not in tx");
      }
      if (&_->objListHead == block) {
        return;
      }
      listDetachTx(block, tx);
      listAttachTx(&_->freeListHead, block, tx);
    }

    void *getRootDirect() {
      return _->objListHead.data;
    }

  private:
    void listDetachTx(PMBlock *block, PMTx *tx) {
      auto next = pool->template directAs<PMBlock>(block->next);
      auto prev = pool->template directAs<PMBlock>(block->prev);

      tx->addDirect(&prev->next, sizeof(prev->next));
      tx->addDirect(&next->prev, sizeof(next->prev));
      prev->next = block->next;
      next->prev = block->prev;
    }

    void listAttachTx(PMBlock *head, PMBlock *block, PMTx *tx) {
      // NOTE: hack for speed
      tx->addDirect(&block->prev, 2*sizeof(block->prev));
//      tx->addDirect(&block->next, sizeof(block->next));
      block->next = pool->offset(head);
      block->prev = head->prev;

      auto blockOff = pool->offset(block);
      auto tail = pool->template directAs<PMBlock>(head->prev);

      tx->addDirect(&head->prev, sizeof(head->prev));
      tx->addDirect(&tail->next, sizeof(tail->next));
      tail->next = blockOff;
      head->prev = blockOff;
    }
    void listAttach(PMBlock *head, PMBlock *block) {
      block->next =
          pool->offset(head);
      block->prev = head->prev;

      auto blockOff = pool->offset(block);
      auto tail = pool->template directAs<PMBlock>(head->prev);
      tail->next = blockOff;
      head->prev = blockOff;
    }

    PMBlock *getBlockByDataPtr(void *data) {
      return reinterpret_cast<PMBlock*>((char*)data - (uint64_t)((PMBlock*)nullptr)->data);
    }

  private:
    PMPool *pool;
    PMLayout *_;
    uint64_t allocPoolSize;
  };
  constexpr const uint64_t MaxDataSize = 1000-16;
  typedef PMBlockAlloc<MaxDataSize> TheAlloc;

}