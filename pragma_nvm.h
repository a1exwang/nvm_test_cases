#pragma once

#include <string>

extern "C" {
bool nvm_init(const char *path, uint64_t size);
void nvm_deinit();
void *nvm_get_pool();
void *nvm_get_tx();
void *nvm_get_root();
void *nvm_get_alloc();
int nvm_add(void *ppool, void *tx, void *ptr, uint64_t len);
void *nvm_start_tx(void *ppool);
int nvm_commit(void *ppool, void *tx);
}
