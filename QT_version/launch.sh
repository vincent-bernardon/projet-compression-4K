#!/bin/bash

mkdir -p build
cd build 
cmake ..
make -j$(nproc)
./compresseur-4K