#pragma once

#include "PMTx.h"

namespace pragma_nvm {

  template <uint64_t Size>
  class PMBlockAlloc {
  public:

    struct PMBlock {
      uint64_t prev;
      uint64_t next;
      char data[Size];
    };

    explicit PMBlockAlloc(PMPool *pool)
        :pool(pool),
         freeListHead((PMBlock*)pool->getAllocPool()),
         objListHead((PMBlock*)pool->getAllocPool()+1) {}

    void reinit() {
      void *allocPool = pool->getAllocPool();
      // First three blocks for freeListHead and objListHead;
      uint64_t blockCount = (pool->getAllocPoolSize()-2*sizeof(PMBlock)) / sizeof(PMBlock);
      memset(allocPool, 0, 2*sizeof(PMBlock));

      objListHead->prev = objListHead->next = pool->offset(objListHead);
      freeListHead->prev = freeListHead->next = pool->offset(freeListHead);

      for (int i = 0; i < blockCount; ++i) {
        PMBlock *block = (PMBlock*)allocPool + 2 + i;
        listAttach(freeListHead, block);
      }
    }

    void *allocDirect(PMTx *tx) {
      if (freeListHead->next == freeListHead->prev) {
        return nullptr;
      }

      PMBlock *tailBlock = pool->directAs<PMBlock>(freeListHead->prev);
      listDetachTx(tailBlock, tx);
      listAttachTx(objListHead, tailBlock, tx);
      return tailBlock->data;
    }

    template <typename T>
    T *allocAs(PMTx *tx) {
      return (T*)allocDirect(tx);
    }

    void freeDirect(PMTx *tx, void *data) {
      freeBlock(tx, (PMBlock*)((char*)data-((PMBlock*)nullptr)->data));
    }

    void freeBlock(PMTx *tx, PMBlock *block) {
      if (objListHead == block) {
        return;
      }
      listDetachTx(block, tx);
      listAttachTx(freeListHead, block, tx);
    }

    void listDetachTx(PMBlock *block, PMTx *tx) {
      auto next = pool->directAs<PMBlock>(block->next);
      auto prev = pool->directAs<PMBlock>(block->prev);

      tx->addDirect(&prev->prev, sizeof(prev->prev));
      tx->addDirect(&prev->next, sizeof(prev->next));
      prev->next = block->next;
      next->prev = block->prev;
    }

    void listAttachTx(PMBlock *head, PMBlock *block, PMTx *tx) {
      tx->addDirect(&block->next, sizeof(block->next));
      tx->addDirect(&block->prev, sizeof(block->prev));
      block->next = pool->offset(head);
      block->prev = head->prev;

      auto blockOff = pool->offset(block);
      auto tail = pool->directAs<PMBlock>(head->prev);

      tx->addDirect(&tail->next, sizeof(tail->next));
      tx->addDirect(&head->prev, sizeof(head->prev));
      tail->next = blockOff;
      head->prev = blockOff;
    }
    void listAttach(PMBlock *head, PMBlock *block) {
      block->next = pool->offset(head);
      block->prev = head->prev;

      auto blockOff = pool->offset(block);
      auto tail = pool->directAs<PMBlock>(head->prev);
      tail->next = blockOff;
      head->prev = blockOff;
    }

    PMBlock *getBlockByDataPtr(void *data) {
      return reinterpret_cast<PMBlock*>((char*)data - (uint64_t)((PMBlock*)nullptr)->data);
    }
  private:
    PMPool *pool;
    PMBlock *freeListHead;
    PMBlock *objListHead;
  };

}