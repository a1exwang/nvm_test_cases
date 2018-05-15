#pragma once
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/transaction.hpp>
#include <unistd.h>
#include <sys/stat.h>
#include "lib/PMLayout.h"

class MallocBench {
public:
  virtual void *alloc(uint64_t n) = 0;
  virtual void free(void *ptr) = 0;
};

class MallocBenchPmdk :public MallocBench{
public:
  MallocBenchPmdk() {
    std::string path = "/dev/shm/mallocBench";
    struct stat buffer;
    if (stat (path.c_str(), &buffer) != 0) {
      pool = pmem::obj::pool<uint64_t>::create(path, path, 1048576 * 100);
    } else {
      pool = pmem::obj::pool<uint64_t>::open(path, path);
    }
  }

  void *alloc(uint64_t n) override {
    pmem::obj::transaction::exec_tx(pool, [n]() {
      auto p = pmem::obj::make_persistent<char[]>(n);
      return p.get();
    });
    return nullptr;
  }
  void free(void *ptr) override {
    pmem::obj::transaction::exec_tx(pool, [ptr]() {
      pmem::obj::persistent_ptr<char> p((char*)ptr);
      pmem::obj::delete_persistent<char>(p);
    });
  }
private:
  pmem::obj::pool<uint64_t> pool;
};

class MallocBenchPragmaNvm :public MallocBench {
public:
  MallocBenchPragmaNvm() {
    std::string path = "/dev/shm/pragmaNvmBench";
    layout = new pragma_nvm::PMLayout(path, 1048576*100);
  }

  void *alloc(uint64_t n) override {
    layout->getTx()->start();
    auto ret = layout->getAlloc()->allocDirect(layout->getTx());
    layout->getTx()->commit();
    return ret;
  }
  void free(void *ptr) override {
    layout->getTx()->start();
    layout->getAlloc()->freeDirect(layout->getTx(), ptr);
    layout->getTx()->commit();
  }
private:
  pragma_nvm::PMLayout *layout;
};
