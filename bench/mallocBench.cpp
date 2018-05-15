#include <iostream>
#include "mallocBench.h"
#include <functional>
#include <chrono>

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

void benchMalloc(const std::string &name, MallocBench *bench) {
  int n = 10000;
  char **ptrs = new char*[n];
  uint64_t blockSize = 392;

  benchmark(name + " alloc", [&](uint64_t &nio, uint64_t &nbytes) -> void {
    for (int i = 0; i < n; ++i) {
      ptrs[i] = (char*)bench->alloc(blockSize);
    }
    nio = n;
    nbytes = blockSize * n;
  });

  benchmark(name + " free", [&](uint64_t &nio, uint64_t &nbytes) -> void {
    for (int i = 0; i < n; ++i) {
      bench->free(ptrs[i]);
    }
    nio = n;
    nbytes = blockSize * n;
  });
  delete []ptrs;
}


int main() {
  benchMalloc("pragma_nvm", new MallocBenchPragmaNvm);
  benchMalloc("pmdk", new MallocBenchPmdk);
  return 0;
}