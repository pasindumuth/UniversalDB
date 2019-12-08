FROM ubuntu:bionic
RUN apt-get update && \
	apt-get install -y build-essential git autoconf libtool pkg-config wget

WORKDIR /home

# Install the latest version of CMake (apt is at version 3.10)
RUN wget -O cmake https://cmake.org/files/v3.15/cmake-3.15.2.tar.gz
RUN tar -xzvf cmake

WORKDIR /home/cmake-3.15.2
RUN ./bootstrap
RUN make -j4
RUN make install

WORKDIR /home
RUN rm cmake && rm -rf cmake-3.15.2

# Install latest version of boost (apt is at version 1.55)
RUN wget -O boost https://sourceforge.net/projects/boost/files/boost/1.70.0/boost_1_70_0.tar.gz/download
RUN tar -xzvf boost

WORKDIR /home/boost_1_70_0
RUN apt-get update && \
    apt-get install -y g++ python-dev autotools-dev libicu-dev libbz2-dev libboost-all-dev
RUN ./bootstrap.sh --prefix=/usr
RUN ./b2
RUN ./b2 install

WORKDIR /home
RUN rm boost && rm -rf boost_1_70_0

# Install latest version of protobuf
RUN wget -O protobuf https://github.com/protocolbuffers/protobuf/releases/download/v3.9.0/protobuf-cpp-3.9.0.tar.gz
RUN tar -xzvf protobuf

WORKDIR /home/protobuf-3.9.0
RUN ./configure
RUN make
RUN make install

WORKDIR /home
RUN rm protobuf && rm -rf protobuf-3.9.0

# https://stackoverflow.com/questions/25518701/protobuf-cannot-find-shared-libraries
RUN ldconfig

# Copy sources into the Docker Image
COPY ./src ./src
COPY ./testing ./testing
COPY ./CMakeLists.txt ./CMakeLists.txt

# Copy old builds into this new Image (for faster builds)
COPY --from=universal:latest /home/build build/

# # Build the code
RUN cmake -B build
RUN cmake --build build
