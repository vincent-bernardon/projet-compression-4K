#!/bin/bash

# Définir le chemin vers OpenCV
export OpenCV_DIR=$(pwd)/src/opencv/opencv-4.11.0/build

mkdir -p build
cd build
cmake -DOpenCV_DIR=$OpenCV_DIR ..
make -j8
./projet




