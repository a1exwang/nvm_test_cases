
#define TestCase(name) \
extern void Test##name();\
Test##name()

int main() {
  TestCase(PMRingBuffer);
  TestCase(PMTx);
  TestCase(PMAlloc);
  return 0;
}