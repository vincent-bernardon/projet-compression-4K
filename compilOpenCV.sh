#!/bin/bash
# /!\ Ce script est à exécuter dans le dossier racine du projet /!\    sauf si vous avez modifié les chemins, ou rajouter des options
# Il faut d'abord compiler OpenCV avec les options suivantes: (afin de garder que les modules nécessaires)

# check_and_install() {
#     dpkg -l | grep -qw "$1" || sudo apt-get install -y "$1"
# }

# check_and_install "libgtk2.0-dev"
# check_and_install "pkg-config"

cd src/opencv/opencv-4.11.0
mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=Release \
      -D WITH_GTK=ON \
      -D BUILD_LIST=core,imgproc,imgcodecs,highgui \
      -D BUILD_opencv_video=OFF \
      -D BUILD_opencv_videoio=OFF \
      -D BUILD_opencv_objdetect=OFF \
      -D WITH_VIDEO=OFF \
      -D WITH_V4L=OFF \
      -D WITH_FFMPEG=OFF \
      -D WITH_CUDA=OFF \
      -D BUILD_opencv_python_bindings_generator=OFF \
      -D BUILD_opencv_java=OFF \
      -D BUILD_opencv_js=OFF \
      ..
make -j8