# BUILD AND RUN
## Project Gen
cmake -B build
cmake --build build

## Build and Run
make -C build
./build/Slave universal0
./build/TestClient

protoc -I=src/proto --cpp_out=src/proto/impl src/proto/*.proto

## Run Notes

## Fast
cmake -B build && cmake --build build
cmake -B build && cmake --build build && ./build/UniversalTest

## Networks
netstat -a



## Docker
docker build --tag=universal .

### Short Run
- Make sure to change the number of servers in the constants.

docker kill universal0; docker rm universal0;
docker kill universal1; docker rm universal1;
docker kill master0; docker rm master0;
docker kill master1; docker rm master1;
docker kill universalclient; docker rm universalclient; 

docker run -it --name=universal0 --ip 172.19.0.10 --network=universal-net universal ./build/Slave 172.19.0.10
docker run -it --name=universal1 --ip 172.19.0.11 --network=universal-net universal ./build/Slave 172.19.0.11 172.19.0.10

docker run -it --name=master0 --ip 172.19.0.20 --network=universal-net universal ./build/Master 172.19.0.20 172.19.0.10 172.19.0.11
docker run -it --name=master1 --ip 172.19.0.21 --network=universal-net universal ./build/Master 172.19.0.21 172.19.0.20 172.19.0.10 172.19.0.11

docker run -it --name=universalclient --network=universal-net universal ./build/TestClient 172.19.0.10 172.19.0.20

### Long Run
docker kill universal0; docker rm universal0;
docker kill universal1; docker rm universal1;
docker kill universal2; docker rm universal2;
docker kill universal3; docker rm universal3;
docker kill universal4; docker rm universal4;
docker kill master0; docker rm master0;
docker kill master1; docker rm master1;
docker kill master2; docker rm master2;
docker kill master3; docker rm master3;
docker kill master4; docker rm master4;
docker kill universalclient; docker rm universalclient; 

docker run -it --name=universal0 --network=universal-net universal ./build/Slave universal0
docker run -it --name=universal1 --network=universal-net universal ./build/Slave universal1 universal0
docker run -it --name=universal2 --network=universal-net universal ./build/Slave universal2 universal0 universal1
docker run -it --name=universal3 --network=universal-net universal ./build/Slave universal3 universal0 universal1 universal2
docker run -it --name=universal4 --network=universal-net universal ./build/Slave universal4 universal0 universal1 universal2 universal3

docker run -it --name=master0 --network=universal-net universal ./build/Master master0 universal0 universal1 universal2 universal3 universal4
docker run -it --name=master1 --network=universal-net universal ./build/Master master1 master0 universal0 universal1 universal2 universal3 universal4
docker run -it --name=master2 --network=universal-net universal ./build/Master master2 master0 master1 universal0 universal1 universal2 universal3 universal4
docker run -it --name=master3 --network=universal-net universal ./build/Master master3 master0 master1 master2 universal0 universal1 universal2 universal3 universal4
docker run -it --name=master4 --network=universal-net universal ./build/Master master4 master0 master1 master2 master3 universal0 universal1 universal2 universal3 universal4


# CMake
cmake --build build  # takes the CMakeLists.txt in the current directory, generates all the Makefiles, and the builds the codebase.
cmake --build build --target Slave # only builds the UniversalDB target

include_directories(./) # This means all headers under this directory, like ./net/Channel.h, can be #included with <net/Channel.h>
add_subdirectory(src/proto proto) # This means find the library under src/proto, but make it #includable with just <proto/...>

# IDE
VSCode can detect the generated protobuf files in both cmake-build-debug, and build. To generate
the protobuf files in cmake-build-debug, run the main file in CLion. This is how CLion
will be able to see the generated protobuf files. 

# Temporary
I'm hesitant about writing a single line of code because I don't know where it fits into the bigger picture. Am I wasting
my time? Will I have to rewrite it anyways? Will it even contribute any value whatsoever? Is there something bigger bang
for buck I could be working on?

I want to know whatever code I write will be traceble through documentation or will be tested at least.


# Questions

So, test.h is actually net/testing/Channel.h, and real.h is actually net/prod/Channel.h. Remember
that my common code (the rest of the codebase) needs to be able to include Channel.h by
doing #include<net/Channel.h>. I want my common code to evaluate #include<net/Channel.h>
to net/testing/Channel.h when I'm building the test target, and I want  #include<net/Channel.h>
to evaluate to net/prod/Channel.h when I'm building the prod target. Is there a way to make
this work, and if there is, how so?

That requires the target_include_directories. Then depending on your project layout either use
target_link_libraries to link the net library or just refer to the two different cc files in your
main binary and test binary add_executable targets respectively

Templates cause a chain reaction of template usage and slows down builds. Having separate files make IDE
development hard and require lots of CMake work and expertise. Inheritance from common interface has the
drawbacks of runtime overhead, advantage of build speed, and drawback that if we want to mock a class
that's already being passed around by value, we're screwed. Type erasure can fix the last point of
inheritance. If performance of this class isn't critical, type erasure is the best option. Well..
I guess we don't really need type erasure to avoid the 3rd problem. We could just allocate
the thing on the heap, dereference the pointer, and assign that a reference. References to
derived classes don't have the slicing problem (I'm not sure how these are efficient though).
In terms of memory safety, allocating everything on the stack, passing the memory address of those
into subsequent constructors, and having those objects hold onto those memory addresses as pointers,
provides good memory safety; no new or delete calls necessary. Using references instead in the same
way allows for some additional semantics (can't reassign, might be more efficient if the compiler knows
the memory address of an object at compile time). We shouldn't store things in the stack in too much
volume, though, since the stack is limited in size. One problem with this is that it only works on a
per-scope basis. So if we want to create many of the same object with a for loop, we need to store the
objects in a vector defined outside of the for loop (it can't be a vanilla array, which don't allow for
non-default constructors). We can't store the objects and pass around references to them, because if
the vector resizes, we get undefined behavior. But we can store them as unique pointers, solving the
problem. The vector, and all objects in it via unique pointers, are owned by the scope. They all get
cleaned up at the end of the scope. In fact, anything that is allocated on the stack can be changed to
the heap if we just use a unique_ptr, maintaining these safety properties (no dangling pointer/corruption
or memory leak). Child objects can pass ownership up to parent objects (transfer ownership of a unique_ptr)
(when they do this, the child must get back the pointer by asking the parent; can't use old pointer).
Children can't delete parent owned objects. Thus, moving more and more memory to be managed upward
for the purpose of sharing results is undeletable memory. Also, we need to wait for memory to be
deleted by having the function exit. If we delete memory early, then might get dangling
pointers/references.

A major benefit over stack allocation over heap allocation is performance. The stack is always hot; the
tip of the stack will always be in the cache (provided that it isn't overwhelmed with data). Although
bulk data (such as what is found in containers) is typically stored in the heap, the more metadata
we can keep in the stack, the better. Doing this also has the advantage of avoiding multiple
indirection with pointers. 

Using the above might be more work than using shared_pointers. Although it can be faster, the main
benefit is a clear understanding of object lifetime.

Have the injection context manage all the singletons and the providers. If all providers just constructed
the objects by passing down singletons, where the provider returns by value, this would work. But what if
the constructor takes in an instance of a class (not singleton)? They must be received by value or unique
pointer (the returned object must own the intermediary somehow). This seems like a sensible way to do
dependency injection without shared pointers.

When we have stack allocated injection, we are restricted in the way we can use the context. The code
has to executed right underneath the code that built the injection context before existing the scope.
The way to have a customizable injection context is to pass down a function that will be called after
the context is built.

Consider a vector of values. We shouldn't take pointers or reference to any elements ever, since they
can become invalidated if the vector resizes. Same with all containers; never take pointers or references
to elements in a container.
