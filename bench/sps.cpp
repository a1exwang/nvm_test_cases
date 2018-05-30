#include "pragma_nvm.h"
#include "lib/PMLayout.h"
#include <chrono>
#include <iostream>
#include <functional>


constexpr const int NODE_COUNT = 100;


void benchmark(const std::string &name, std::function<void(uint64_t&, uint64_t&)> fn) {

  typedef std::chrono::high_resolution_clock Time;
  typedef std::chrono::microseconds us;
  typedef std::chrono::duration<uint64_t> fsec;

  auto t0 = Time::now();
  uint64_t nIo = 0, nBytes = 0;
  fn(nIo, nBytes);
  auto t1 = Time::now();
  us d = std::chrono::duration_cast<us>(t1 - t0);
  std::cout << name + " total: " << d.count() << "us, " << std::endl
            << "  opps: " << d.count()/(double)nIo << "us/op, " << std::endl
            << "  iops: " << nIo / (d.count()/1000.0) << "kop/s, " << std::endl
            << "  bw: " << nBytes/d.count() << "MiB/s" << std::endl;
}

void *alloc(uint64_t n) {
  auto *layout = (pragma_nvm::PMLayout*)nvm_get_layout();
  layout->getTx()->start();
  auto ret = layout->getAlloc()->allocDirect(layout->getTx());
  layout->getTx()->commit();
  return ret;
}

void run() {
  long _tmp;

  unsigned long size = 128 * 1048576;
  nvm_init("/dev/shm/sps", &size);

  long *tmp = &_tmp;

  #pragma clang nvm nvmptr
  long *nodes = (long*)alloc(sizeof(long) * NODE_COUNT);

  void *_tx;
  void *_pool;
  for (int i = 0; i < 1024 * 256; ++i) {
//  for (int i = 0; i < 1; ++i) {
    int i1 = i % NODE_COUNT;
    int i2 = (i+1) % NODE_COUNT;
    #pragma clang nvm tx(_pool, _tx) nvmptrs(nodes)
    {
      *tmp = nodes[i1];
      nodes[i1] = nodes[i2];
      nodes[i2] = *tmp;
    }
  }
}

int main() {
  benchmark("sps", [&](uint64_t &io, uint64_t &bytes) {
    run();
  });
}