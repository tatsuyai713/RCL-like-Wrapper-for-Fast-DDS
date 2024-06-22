#!/bin/bash

fast_dds_version="2.14.2"
foonathan_memory_vendor_version="1.3.1"
googletest_version="1.13.0"
fast_dds_gen_version="3.3.0"

FAST_DDS_WORK_DIR=./dds_build

sudo apt update
sudo apt install -y gcc g++ make cmake automake autoconf unzip git vim openssl gcc make cmake curl tar default-jre default-jdk wget p11-kit

p11-kit list-modules

openssl engine pkcs11 -t

sudo rm -rf ${FAST_DDS_WORK_DIR}

mkdir ${FAST_DDS_WORK_DIR}

cd ${FAST_DDS_WORK_DIR}
git clone https://github.com/eProsima/Fast-DDS.git -b v$fast_dds_version --depth 1
cd Fast-DDS
WORKSPACE=$PWD
git submodule update --init $PWD/thirdparty/asio $PWD/thirdparty/fastcdr $PWD/thirdparty/tinyxml2
cd ${WORKSPACE}
git clone https://github.com/eProsima/foonathan_memory_vendor.git -b v$foonathan_memory_vendor_version
cd ${WORKSPACE}
git clone https://github.com/google/googletest.git -b v$googletest_version --depth 1

INSTALL_PATH=/opt/fast-dds
sudo rm -rf $INSTALL_PATH
sudo mkdir $INSTALL_PATH

cd $WORKSPACE
cd foonathan_memory_vendor && mkdir build && cd build &&\
CXXFLAGS="-O3" cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../ && \
make -j4 && sudo make install

cd $WORKSPACE
cd googletest && mkdir build && cd build &&\
CXXFLAGS="-O3" cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../ && \
make -j4 && sudo make install

# Build and install Fast-CDR (required for FastDDS)
cd $WORKSPACE
cd thirdparty/fastcdr && mkdir build && cd build && \
CXXFLAGS="-O3" cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../ &&
make -j4 && sudo make install


# Build and install TinyXML2 (required for FastDDS)
cd $WORKSPACE
cd thirdparty/tinyxml2 && mkdir build && cd build && \
CXXFLAGS="-O3 -fPIC" cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../ &&
make -j4 && sudo make install


# Build and install ASIO (requited for FastDDS)
cd $WORKSPACE
cd thirdparty/asio/asio && \
./autogen.sh && \
./configure CXXFLAGS="-O3 -g -DASIO_HAS_PTHREADS -D_GLIBCXX_HAS_GTHREADS -std=c++11" --prefix=$INSTALL_PATH  && \
make -j4 && sudo make install


# Build and install Fast-DDS
cd $WORKSPACE
rm -rf ./build && mkdir build && cd build && \
CXXFLAGS="-DASIO_HAS_PTHREADS=1" \
  cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
  -Dfastcdr_DIR=$INSTALL_PATH/lib/cmake/fastcdr/ \
  -Dfoonathan_memory_DIR=$INSTALL_PATH/lib/foonathan_memory/cmake/ \
  -DCMAKE_SYSTEM_PREFIX_PATH=$INSTALL_PATH \
  -DCMAKE_PREFIX_PATH=$INSTALL_PATH \
  .. && \
make -j4 && sudo make install

sed -i -e '/export PATH=$PATH:\/opt\/fast-dds\/bin/d' ~/.bashrc
echo 'export PATH=$PATH:/opt/fast-dds/bin' >> ~/.bashrc

if grep 'export LD_LIBRARY_PATH=/opt/fast-dds/lib:$LD_LIBRARY_PATH' ~/.bashrc >/dev/null; then
  echo "LD_LIBRARY_PATH libs are already added"
else
  echo 'export LD_LIBRARY_PATH=/opt/fast-dds/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
  source ~/.bashrc
fi
sudo ldconfig
