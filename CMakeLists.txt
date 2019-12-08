cmake_minimum_required(VERSION 3.13)
project(UniversalDB)

set(CMAKE_CXX_STANDARD 17)

# This commands lets us to do `#include <net/Channel.h>` in the
# source code to use the file ./net/Channel.h. Using include_directories
# for any directory does a similar sort of thing with all the files
# underneath. Otherwise, we would have to use `#include "net/Channel.h"`.
# include_directories(src)

set(CMAKE_CXX_FLAGS "-fexceptions -fsanitize=address -fsanitize=undefined -pthread")

# Find the library under src/proto, but make it #includable with just <proto/...>
add_subdirectory(src/proto proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package( Boost 1.40 COMPONENTS program_options REQUIRED )
find_package( Boost 1.40 REQUIRED system )
include_directories(${Boost_INCLUDE_DIR}) # Allows us to #include boost headers in our code.

file(GLOB_RECURSE files src/**/*.cc)
add_library(lib ${files})
target_include_directories(lib PUBLIC src)
target_link_libraries(lib proto protobuf ${Boost_LIBRARIES} ${Protobuf_LIBRARY})

# Create Slave executable
add_executable(Slave src/slave.cc)
target_link_libraries(Slave lib)

# Create Master executable
add_executable(Master src/master.cc)
target_link_libraries(Master lib)

# Create TestClient executable
add_executable(TestClient src/testclient.cc)
target_link_libraries(TestClient lib)

# Setup gtest
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.8.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# Create create UniversalTest library and executable
file(GLOB_RECURSE testfiles testing/**/*.cc)
add_library(test ${testfiles})
target_include_directories(test PUBLIC testing)
target_link_libraries(test gtest_main lib proto protobuf ${Boost_LIBRARIES} ${Protobuf_LIBRARY})

add_executable(UniversalTest
  testing/test.cc
  testing/unit/slave/KVStoreTest.cc
)

target_link_libraries(UniversalTest test)
