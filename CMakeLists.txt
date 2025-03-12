cmake_minimum_required(VERSION 3.10)
project(projet)

# Set default build type to Release for better performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Use ccache if available for faster rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# Enable compiler optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
# Enable SIMD instructions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mavx")


# Chemin vers le dossier de compilation d'OpenCV (et non le dossier source)
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/src/opencv/opencv-4.11.0/build")

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Disable OpenCV debug code in Release mode
add_definitions(-DCV_DISABLE_OPTIMIZATION=0)
add_definitions(-DNDEBUG)

# Enable Link-Time Optimization if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Ajouter les fichiers sources
add_executable(projet 
    src/main.cpp
    src/ImageUtils.cpp  # Contains stringduplicate function
    src/SLIC.cpp   # Contains SLIC function
    src/Superpixel.cpp # Contains PSNR function
    # Add any other source files here
)

# Use precompiled headers for faster builds
target_precompile_headers(projet PRIVATE
    <opencv2/opencv.hpp>
    <vector>
    <iostream>
)

# Inclure les headers d'OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Lier les bibliothèques OpenCV
target_link_libraries(projet ${OpenCV_LIBS})
