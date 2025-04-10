#!/bin/bash

mkdir -p build
cd build 
rm ./compresseur-4K
cmake ..
make -j$(nproc)
./compresseur-4K