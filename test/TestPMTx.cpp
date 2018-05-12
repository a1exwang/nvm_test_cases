#include "../lib/PMTx.h"
#include <iostream>


using namespace pragma_nvm;
using namespace std;

void TestPMTx() {

  // Must > 1MiB
  uint64_t bufSize = 1048576 * 4;
  char *pmem = new char[bufSize];
  memset(pmem, 0, bufSize);

  PMPool pool(pmem, bufSize);
  PMTx tx(&pool);

  void *allocPool = pool.getAllocPool();
  tx.recover();
  tx.start();
  tx.addDirect(allocPool, 32);

  tx.abort();
//  tx.commit();
  delete [] pmem;
}