CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(nvm_bench)

SET(CMAKE_CXX_COMPILER /home/alexwang/dev/proj/cpp/llvm/llvm-git/cmake-build-x86debug/bin/clang++)
SET(CMAKE_C_COMPILER /home/alexwang/dev/proj/cpp/llvm/llvm-git/cmake-build-x86debug/bin/clang)

include_directories(3rdparty/pmdk/src/include)
link_directories(3rdparty/pmdk/src/debug)

ADD_LIBRARY(pragma_nvm_pmdk SHARED pragma_nvm_pmdk.cpp)
ADD_LIBRARY(pragma_nvm SHARED pragma_nvm.cpp pragma_nvm.h lib/PMAtomicArray.h lib/PMRingBuffer.h)

ADD_EXECUTABLE(nvm nvm.cpp)
TARGET_LINK_LIBRARIES(nvm pmemobj)

ADD_EXECUTABLE(nvm_log nvm_log.cpp)
TARGET_LINK_LIBRARIES(nvm_log pmemobj pmem pragma_nvm)

ADD_SUBDIRECTORY(3rdparty/pmemkv)
