#!/bin/bash
dir=`pwd`

##install dependencies
sudo apt-get install gcc gcc++
#sudo apt-get install libgtest-dev
#cd /usr/src/gtest
#sudo cmake CMakeLists.txt
#sudo make

cd $dir
if [ ! -d $dir/llvm-mirror ]; then
    git clone https://github.com/llvm/llvm-project.git --branch release/11.x llvm-project
fi

cd $dir/llvm-mirror/
if [ ! -d $dir/llvm-mirror/build ]; then
    mkdir build
    cd $dir/llvm-mirror/build
fi

export CC=gcc
export CXX=g++
CC='gcc' CXX='g++' cmake -G "Unix Makefiles" \
-DLLVM_TARGETS_TO_BUILD=host \
-DLLVM_BUILD_EXAMPLES=OFF \
-DCLANG_BUILD_EXAMPLES=OFF \
-DLLVM_ENABLE_OCAMLDOC=OFF \
-DLLVM_BUILD_DOCS=OFF \
-DLLVM_BUILD_TESTS=OFF \
-DCMAKE_BUILD_TYPE=Release ../llvm/

make -j$(nproc)
#sudo make install
