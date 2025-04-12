cmake_minimum_required(VERSION 3.16)

project(compresseur-4K VERSION 0.1 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable compiler optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
# Enable SIMD instructions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -mavx")

# Enable Link-Time Optimization if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opencv-4.11.0/build")
find_package(OpenCV REQUIRED)

find_package(OpenMP REQUIRED)
if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(compresseur-4K
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        SDGT.cpp
        SDGT.h
        SLIC.cpp
        SLIC.h
        Superpixel.cpp
        Superpixel.h
        ImageUtils.cpp
        ImageUtils.h
        global.h
        timer.h
        slicdialog.h slicdialog.cpp
        sdgtdialog.h sdgtdialog.cpp
        customgraphicsview.h customgraphicsview.cpp
        selectorcolorbutton.h selectorcolorbutton.cpp
        SLICWorker.h SLICWorker.cpp
        SDGTWorker.h SDGTWorker.cpp
)
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET compresseur-4K APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(compresseur-4K SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(compresseur-4K
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(compresseur-4K PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(compresseur-4K PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(compresseur-4K PRIVATE ${OpenCV_LIBS})
target_link_libraries(compresseur-4K PRIVATE ${OpenMP_CXX_LIBRARIES} ${AUTRES_LIBS})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.compresseur-4K)
endif()
set_target_properties(compresseur-4K PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS compresseur-4K
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(compresseur-4K)
endif()
