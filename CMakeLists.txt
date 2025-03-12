cmake_minimum_required(VERSION 3.10)
project(projet)

# Chemin vers le dossier de compilation d'OpenCV (et non le dossier source)
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/src/opencv/opencv-4.11.0/build")

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Ajouter les fichiers sources
add_executable(projet 
    src/main.cpp
    src/ImageUtils.cpp  # Contains stringduplicate function
    src/SLIC.cpp   # Contains SLIC function
    src/Superpixel.cpp # Contains PSNR function
    # Add any other source files here
)
# Inclure les headers d'OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Lier les bibliothèques OpenCV
target_link_libraries(projet ${OpenCV_LIBS})
