#pragma once
#include "PMPool.h"
#include "PMTx.h"
#include "PMBlockAlloc.h"
#include "sys/stat.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "unistd.h"


namespace pragma_nvm {
  struct PMPoolLayout {
    PMPool::PMLayout  poolLayout;
    PMTx::PMLayout    txLayout;
    TheAlloc::PMLayout allocLayout;
  };

  class PMLayout {
  public:
    PMLayout(const std::string &path, uint64_t sizeRequested) {
      struct stat buffer{};
      int status = stat(path.c_str(), &buffer);
      int fd;
      uint64_t totalSize;
      if(status == 0) {
        fd = open(path.c_str(), O_RDWR, 0666);
        if (fd < 0) {
          perror("open");
          throw std::runtime_error("failed to open nvm file");
        }

        totalSize = (uint64_t)buffer.st_size;
      } else {
        fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
          perror("open");
          throw std::runtime_error("failed to open nvm file");
        }

        /* allocate the pmem */
        totalSize = (sizeRequested*2)/4096 * 4096;
        if ((errno = posix_fallocate(fd, 0, totalSize)) != 0) {
          perror("posix_fallocate");
          throw std::runtime_error("failed to alloc nvm file");
        }
      }

      base = (char*)mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      this->size = totalSize;
      if (base == MAP_FAILED) {
        perror("mmap");
        throw std::runtime_error("failed to mmap file");
      }

      if ((errno = close(fd)) != 0) {
        perror("close");
        throw std::runtime_error("failed to close fd");
      }

      pool = new PMPool(&getLayout()->poolLayout);
      tx = new PMTx(pool, &getLayout()->txLayout);
      theAlloc = new TheAlloc(
          pool,
          &getLayout()->allocLayout,
          size - (uint64_t)&((PMPoolLayout*)(nullptr))->allocLayout.allocPool
      );
      if (!pool->isInitialized()) {
        pool->setInitialized(size);
        theAlloc->reinit();
        tx->reinit();
        pool->setInitialized(size);
      }
    }
    ~PMLayout() {
      delete theAlloc;
      delete tx;
      delete pool;
      int status = munmap(base, size);
      if (status < 0) {
        perror("munmap");
        abort();
      }
    }

    TheAlloc *getAlloc() { return theAlloc; }
    PMPool *getPool() { return pool; }
    PMTx *getTx() { return tx; }

    uint64_t getSize() { return size; }
  private:
    PMPoolLayout *getLayout() {
      return (PMPoolLayout*)base;
    }

    void *base;
    uint64_t size;

    PMPool *pool;
    PMTx *tx;
    TheAlloc *theAlloc;
  };

}

