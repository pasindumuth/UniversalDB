# By having a second CMakeLists.txt file, we can #include the ph.h
# files by doing #include <proto/.pb.h>. If we included these commands
# in the root CMakeLists.txt file, we must #include the generated files
# with #include <.pb.h>.
find_package(Protobuf REQUIRED)
file(GLOB protos *.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protos})
add_library(proto ${PROTO_HDRS} ${PROTO_SRCS})
