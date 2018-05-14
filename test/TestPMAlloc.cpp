#include "PMBlockAlloc.h"
#include "PMTx.h"

using namespace pragma_nvm;
using namespace std;

struct Wtf {
  char data[64];
};

const char *newData = "hello, world!";
const char *oldData = "old data";

void TestPMAlloc() {

  // Must > 1MiB
  uint64_t bufSize = 1048576 * 4;
  char *pmem = new char[bufSize];
  memset(pmem, 0, bufSize);

  PMPool pool(pmem, bufSize);
  PMTx tx(&pool);

  PMBlockAlloc<sizeof(Wtf)> alloc(&pool);
  alloc.reinit();


  // tx 1
  tx.start();

  Wtf *data = alloc.allocAs<Wtf>(&tx);
  auto off = pool.offset(data);
  tx.addDirect(data, strlen(oldData));
  strcpy(data->data, oldData);

  tx.commit();

  printf("data, off=%ld: '%s'\n", off, data->data);

  // tx 2
  tx.start();

  tx.addDirect(data, strlen(newData));
  strcpy(data->data, newData);
  printf("data, off=%ld: '%s'\n", off, data->data);

  tx.abort();
//  tx.commit();

  printf("data, off=%ld: '%s'\n", off, data->data);

  delete [] pmem;
}