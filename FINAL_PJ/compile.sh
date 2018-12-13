#!/bin/bash
git clone --depth=10 --branch=master git://github.com/yanyiwu/cppjieba.git
cd cppjieba
mkdir build
cd build
cmake ..
make -j3
cd ..
cp -r deps/limonp include/cppjieba
cd ..
make
