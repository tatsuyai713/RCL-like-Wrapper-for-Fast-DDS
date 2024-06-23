#!/bin/bash

OPT=$1
OPT_NUM=$#

# clean
if [ ! $OPT_NUM -ne 1 ]; then
  if [ "clean" = $OPT ]; then
    sudo rm -rf ./libraries/build
    mkdir -p ./libraries/build
    exit
  fi
fi

cd libraries
mkdir build
cd build
DDS_PATH=/opt/fast-dds
INSTALL_PATH=/opt/fast-dds-libs
sudo mkdir -p $INSTALL_PATH

sudo rm $INSTALL_PATH/include/lwrcl.hpp
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DDS_PATH/lib

cmake ..  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_SYSTEM_PREFIX_PATH=$DDS_PATH \
  -DCMAKE_PREFIX_PATH=$DDS_PATH \
  -Dfastcdr_DIR=$DDS_PATH/lib/cmake/fastcdr/ \
  -Dfastrtps_DIR=$DDS_PATH/share/fastrtps/cmake/ \
  -Dfoonathan_memory_DIR=$DDS_PATH/lib/foonathan_memory/cmake/ \
  -Dtinyxml2_DIR=$DDS_PATH/lib/cmake/tinyxml2/ \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH \
  -DCMAKE_PREFIX_PATH=$DDS_PATH \
  -DYAML_BUILD_SHARED_LIBS=ON \
  -DYAML_CPP_INSTALL=ON

make -j4

if [ ! $OPT_NUM -ne 1 ]; then
	if [ "install" = $OPT ]; then
    sudo make install
    sudo cp ../src/lwrcl/fastdds.xml $DDS_PATH/
	fi

fi
