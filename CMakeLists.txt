cmake_minimum_required(VERSION 3.13)
project(UniversalDB)

set(CMAKE_CXX_STANDARD 17)

# This commands lets us to do `#include <net/Channel.h>` in the source code to use
# the file ./net/Channel.h. Using include_directories for any directory does a similar
# sort of thing with all the files underneath.
include_directories(src)

set(CMAKE_CXX_FLAGS "-fexceptions -fsanitize=address -fsanitize=undefined -pthread")

# Find the library under src/proto, but make it #includable with just <proto/...>
add_subdirectory(src/proto proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package( Boost 1.40 COMPONENTS program_options REQUIRED )
find_package( Boost 1.40 REQUIRED system )
include_directories(${Boost_INCLUDE_DIR}) # Allows us to #include boost headers in our code.

file(GLOB_RECURSE files src/**/*.h src/**/*.cc)
add_library(lib ${files})
target_link_libraries(lib proto protobuf ${Boost_LIBRARIES} ${Protobuf_LIBRARY})

add_executable(Slave src/slave.cc)
target_link_libraries(Slave lib)

add_executable(TestClient src/testclient.cc)
target_link_libraries(TestClient lib)

add_executable(UniversalTest src/testing/test.cc)
target_link_libraries(UniversalTest lib)
