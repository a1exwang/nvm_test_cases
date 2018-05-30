#pragma once

#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <pragma_nvm.h>
#include <PMBlockAlloc.h>

namespace pragma_nvm {
  template<typename T>
  class p {
  public:
    p() = default;

    T &get_rw() {
      nvm_add(nvm_get_pool(), nvm_get_tx(), &obj, sizeof(T));
//        if (pmemobj_tx_stage() == TX_STAGE_WORK) {
//          if (pmemobj_pool_by_ptr(&obj)) {
//            pmemobj_tx_add_range_direct(&obj, sizeof(T));
//          }
//        }

      return this->obj;
    }

    const T &get_ro() const { return this->obj; }

  private:
    T obj;
  };

  template<typename T>
  class persistent_ptr {
  public:
    persistent_ptr() : offset(0) {}

    persistent_ptr(T *ptr) : offset(((pragma_nvm::PMPool *) nvm_get_pool())->offset(ptr)) {}

    persistent_ptr(uint64_t offset) : offset(offset) {}

//      persistent_ptr(PMEMoid oid) :ptr(ptr) { }
    persistent_ptr(const persistent_ptr<T> &rhs) : offset(rhs.offset) {}

    operator uint64_t () {
      return this->offset;
    }
    bool null() const {
      return this->offset == 0;
    }

    persistent_ptr<T> &operator=(const persistent_ptr<T> &rhs) {
      this->offset = rhs.offset;
      return *this;
    }

    T &operator*() {
      return *get();
    }

    T *operator->() {
      nvm_add(nullptr, nullptr, get(), sizeof(T));
      return get();
    }

    T *get() {
      return ((pragma_nvm::PMPool *) nvm_get_pool())->directAs<T>(offset);
    }

    const T *get() const {
      return ((pragma_nvm::PMPool *) nvm_get_pool())->directAs<T>(offset);
    }

    operator bool() const {
      return offset != 0;
    }

    bool operator!=(const persistent_ptr<T> &rhs) {
//        return this->oid.pool_uuid_lo == rhs.oid.pool_uuid_lo && this->oid.off == rhs.oid.off;
      return this->offset != rhs.offset;
    }

  private:
    uint64_t offset;
  };

  static int a = 0;

  template<typename T>
  persistent_ptr<T> make_persistent() {
    if (sizeof(T) >= pragma_nvm::MaxDataSize) {
      abort();
    }
//      a++;
//      printf("a = %d\n", a);
//      return persistent_ptr<T>((T*)malloc(sizeof(T)));
    return persistent_ptr<T>(
        ((pragma_nvm::TheAlloc *) nvm_get_alloc())->allocAs<T>((pragma_nvm::PMTx *) nvm_get_tx())
    );
  }

  template<typename T>
  persistent_ptr<T> make_persistent(size_t n) {
    if (n * sizeof(T) >= pragma_nvm::MaxDataSize) {
      abort();
    }

//      a++;
//      printf("a = %d\n", a);
    return persistent_ptr<T>(
        ((pragma_nvm::TheAlloc *) nvm_get_alloc())->allocAs<T>((pragma_nvm::PMTx *) nvm_get_tx())
    );
//      return persistent_ptr<T>((T*)malloc(sizeof(T)*n));
  }

//    template <typename T>
//    void delete_persistent(persistent_ptr<T> ptr) {
//      pmem::obj::delete_persistent<T>(ptr);
//    }
  template<typename T>
  void delete_persistent_arr(persistent_ptr<T> ptr, size_t len) {
//      pmem::obj::delete_persistent<T[]>(pmem::obj::persistent_ptr<T[]>(ptr.get()), len);
//      printf("delete_persistent_arr: ptr=%p, len=%ld\n", ptr.get(), len);
    ((pragma_nvm::TheAlloc *) nvm_get_alloc())->freeDirect((pragma_nvm::PMTx *) nvm_get_tx(), ptr.get());
  }
}
