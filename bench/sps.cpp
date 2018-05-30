#include "pragma_nvm.h"
#include <random>
#include "mallocBench.h"
#include <chrono>
#include <iostream>


constexpr const int NODE_COUNT = 100;

static long _nodes[NODE_COUNT];

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

int main() {
  unsigned long size = 128 * 1048576;
  nvm_init("/dev/shm/sps", &size);

  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0, NODE_COUNT);

  int pos_a = distribution(generator);
  int pos_b = distribution(generator);
//  int pos_a, pos_b;


  #pragma clang nvm nvmptr
  long *nodes = _nodes;
  long tmp = 0;

  benchmark("sps", [&](uint64_t &io, uint64_t &bytes) {
    void *_tx;
    void *_pool;
    for (int i = 0; i < 1048576; ++i) {
      #pragma clang nvm tx(_pool, _tx) nvmptrs(nodes)
      {
        tmp = nodes[pos_a];
        nodes[pos_a] = nodes[pos_b];
        nodes[pos_a] = nodes[pos_b];
//        nodes[pos_b] = tmp;
      }
    }
  });

}