#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <functional>


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
