CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(nvm_bench)

SET(CMAKE_CXX_COMPILER /home/alexwang/dev/proj/cpp/llvm/llvm-git/cmake-build-x86debug/bin/clang++)
SET(CMAKE_C_COMPILER /home/alexwang/dev/proj/cpp/llvm/llvm-git/cmake-build-x86debug/bin/clang)

include_directories(3rdparty/pmdk/src/include)
include_directories(lib)
include_directories(.)
link_directories(3rdparty/pmdk/src/nondebug)

ADD_LIBRARY(pragma_nvm_pmdk SHARED pragma_nvm_pmdk.cpp)
ADD_LIBRARY(pragma_nvm SHARED pragma_nvm.cpp pragma_nvm.h lib/PMPool.cpp
        lib/PMAtomicArray.h lib/PMRingBuffer.h lib/PMTx.h lib/PMBlockAlloc.h lib/PMPool.h lib/PMLayout.h lib/PM.h)

ADD_EXECUTABLE(nvm nvm.cpp)
TARGET_LINK_LIBRARIES(nvm pmemobj)

ADD_EXECUTABLE(nvm_log nvm_log.cpp)
TARGET_LINK_LIBRARIES(nvm_log pmemobj pmem pragma_nvm)

ADD_EXECUTABLE(testPm test/TestPMRingBuffer.cpp test/TestPMTx.cpp test/TestPMAlloc.cpp test/main.cpp)
TARGET_LINK_LIBRARIES(testPm pmem)

ADD_SUBDIRECTORY(3rdparty/pmemkv)

ADD_EXECUTABLE(benchMalloc bench/mallocBench.cpp bench/mallocBench.h)
TARGET_LINK_LIBRARIES(benchMalloc pmemobj pmem)

ADD_EXECUTABLE(benchSPS bench/sps.cpp)
TARGET_LINK_LIBRARIES(benchSPS pmemobj pmem pragma_nvm)

#set_source_files_properties(bench/sps.cpp PROPERTIES COMPILE_FLAGS "-Xclang -pragma-nvm-enable-opt")
set_source_files_properties(bench/sps.cpp PROPERTIES COMPILE_FLAGS "${PRAGMA_NVM_OPT}")
