#pragma once

#include <string>


class PragmaNvmPool {
public:
  PragmaNvmPool(const std::string &path);
  ~PragmaNvmPool();
  PragmaNvmPool(const PragmaNvmPool&) = delete;
  PragmaNvmPool &operator=(const PragmaNvmPool&) = delete;

  void *getRoot();
private:

};

extern "C" {
int nvm_add(void *ppool, void *tx, void *ptr, uint64_t len);
void *nvm_start_tx(void *ppool);
int nvm_commit(void *ppool, void *tx);
}
