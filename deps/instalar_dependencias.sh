#!/bin/bash
dir=`pwd`

#install dependencies
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make

cd $dir
if [ ! -d $dir/llvm-mirror ]; then
    mkdir $dir/llvm-mirror
fi
cd $dir/llvm-mirror
if [ -d $dir/llvm-mirror/llvm ]; then
    cd $dir/llvm-mirror/llvm
    git pull
else
    git clone --recursive https://github.com/llvm-mirror/llvm
fi
if [ -d $dir/llvm-mirror/llvm/tools/clang ]; then
    cd $dir/llvm-mirror/llvm/tools/clang
    git pull
else
    cd $dir/llvm-mirror/llvm/tools
    git clone --recursive https://github.com/llvm-mirror/clang
fi
if [ -d $dir/llvm-mirror/llvm/tools/lld ]; then
    cd $dir/llvm-mirror/llvm/tools/lld
    git pull
else
    cd $dir/llvm-mirror/llvm/tools
    git clone --recursive https://github.com/llvm-mirror/lld
fi
if [ -d $dir/llvm-mirror/llvm/projects/compiler-rt ]; then
    cd $dir/llvm-mirror/llvm/projects/compiler-rt
    git pull
else
    cd $dir/llvm-mirror/llvm/projects/
    git clone --recursive https://github.com/llvm-mirror/compiler-rt
fi
if [ -d $dir/llvm-mirror/llvm/projects/libcxx ]; then
    cd $dir/llvm-mirror/llvm/projects/libcxx
    git pull
else
    cd $dir/llvm-mirror/llvm/projects/
    git clone --recursive https://github.com/llvm-mirror/libcxx
fi
if [ -d $dir/llvm-mirror/llvm/projects/libcxxabi ]; then
    cd $dir/llvm-mirror/llvm/projects/libcxxabi
    git pull
else
    cd $dir/llvm-mirror/llvm/projects/
    git clone --recursive https://github.com/llvm-mirror/libcxxabi
fi
if [ -d $dir/llvm-mirror/llvm/tools/clang/tools/extra ]; then
    cd $dir/llvm-mirror/llvm/tools/clang/tools/extra
    git pull
else
    cd $dir/llvm-mirror/llvm/tools/clang/tools
    git clone --recursive https://github.com/llvm-mirror/clang-tools-extra extra
fi

cd $dir/llvm-mirror/
if [ ! -d $dir/llvm-mirror/build ]; then
    mkdir build
    cd $dir/llvm-mirror/build
else
    cd $dir/llvm-mirror/build
    rm CMakeCache.txt
fi
CC='gcc' CXX='gcc++' cmake -G "Unix Makefiles" \
-DLLVM_TARGETS_TO_BUILD=host \
-DLLVM_BUILD_EXAMPLES=ON \
-DCLANG_BUILD_EXAMPLES=ON \
-DLLVM_ENABLE_OCAMLDOC=OFF \
-DLLVM_BUILD_DOCS=OFF \
-DLLVM_BUILD_TESTS=ON \
-DCMAKE_BUILD_TYPE=Release ../llvm/

make -j$(nproc)
sudo make install