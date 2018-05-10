#include "../lib/PMTx.h"
#include <iostream>


using namespace pragma_nvm;
using namespace std;

void TestPMTx() {

  uint64_t bufSize = 1048576;
  char *pmem = new char[bufSize*2];
  char *allocBase = pmem + bufSize;

  PMTx tx(pmem+bufSize, bufSize, pmem);
  tx.recover();
  tx.start();
  tx.add_direct(allocBase, 32);

  tx.abort();
//  tx.commit();
  delete [] pmem;
}