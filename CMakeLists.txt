cmake_minimum_required(VERSION 2.8)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()
project( olyvslideconv LANGUAGES C CXX )

if (CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
endif()

include(CheckStructHasMember)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif()

option(USE_MAGICK "Use ImageMagick instead of OpenCV for processing." OFF)
option(USE_OPENCV "Use OpenCV for image processing instead of ImageMagick." ON)

if (NOT MSVC AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  find_package(PkgConfig)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -O0 -Wall")
  set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -Wall")
endif()

find_package(JPEG)
find_package(TIFF)
find_package(Threads)
if ("\"${CMAKE_CXX_COMPILER_ID}\"" MATCHES "MSVC")
  find_package(minizip 1)
  find_path(MINIZIP_INCLUDE_DIRS NAMES zip.h PATH_SUFFIXES minizip)
  set(MINIZIP_LIBRARIES minizip::minizip)
endif()

if (USE_MAGICK)
	find_package(ImageMagick COMPONENTS MagickCore MagickWand)
	set (USE_OPENCV OFF)
endif()

if (USE_OPENCV)
	find_package(OpenCV)
endif()

if (JPEG_FOUND)
  set (LIBJPEG_LIBRARIES ${JPEG_LIBRARY})
  set (LIBJPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
elseif (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBJPEG REQUIRED libjpeg)
else()
  message(FATAL_ERROR "Jpeg library not found!")
endif()

if (TIFF_FOUND)
  set (LIBTIFF_LIBRARIES ${TIFF_LIBRARY})
  set (LIBTIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
elseif (PKG_CONFIG_FOUND)
  pkg_check_modules(LIBTIFF REQUIRED libtiff-4)
else()
  message(FATAL_ERROR "LibTiff not found!")
endif()

if (${USE_MAGICK} STREQUAL "OFF")
  add_definitions(-DUSE_OPENCV=1)
  if (OpenCV_FOUND)
 	  set (OPENCV_LIBRARIES ${OpenCV_LIBS})
    set (OPENCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR})
  elseif (PKG_CONFIG_FOUND)
    pkg_check_modules(OPENCV REQUIRED opencv4)
  else()
    message(FATAL_ERROR "OpenCV Libraries not found!")
  endif()
else()
  add_definitions(-DUSE_MAGICK=1)
  add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=16)
  add_definitions(-DMAGICKCORE_HDRI_ENABLE=1)
  add_definitions(-DMAGICK_MAP_LIMIT="256MB")
  add_definitions(-DMAGICK_MEMORY_LIMIT="256MB")
  add_definitions(-DMAGICK_DISK_LIMIT="16GB")
  add_definitions(-DMAGICK_AREA_LIMIT="50MPP")
  if (ImageMagick_MagickWand_FOUND AND ImageMagick_MagickCore_FOUND)
    set (MAGICKWAND_LIBRARIES ${ImageMagick_MagickWand_LIBRARY})
    set (MAGICKWAND_INCLUDE_DIRS ${ImageMagick_MagickWand_INCLUDE_DIR})
    set (MAGICKCORE_LIBRARIES ${ImageMagick_MagickCore_LIBRARY})
    set (MAGICKCORE_INCLUDE_DIRS ${ImageMagick_MagickCore_INCLUDE_DIR})
  elseif (PKG_CONFIG_FOUND)
    pkg_check_modules(MAGICKWAND REQUIRED MagickWand)
    pkg_check_modules(MAGICKCORE REQUIRED MagickCore)
  else()
    message(FATAL_ERROR "ImageMagick Wand and Core Libraries not found!")
  endif()
endif()

if (NOT MINIZIP_LIBRARIES AND PKG_CONFIG_FOUND)
  pkg_check_modules(MINIZIP minizip)
  if (NOT MINIZIP_LIBRARIES)
    pkg_check_modules(MINIZIP minizip2 REQUIRED)
    add_definitions(-DOLY_USE_MZ_VER2=1)
  endif()
endif()

if (NOT MINIZIP_LIBRARIES)
	message(FATAL_ERROR "Minzip library version 1 or 2 not found!")
endif()

set(ENABLE_EXPORTS, true)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

set( MAIN_SRC
  src/safebmp.cc
  src/olyvslideconv.cc
  src/jpgcachesupport.cc
  src/imagesupport.cc
  src/jpgsupport.cc
  src/tiffsupport.cc
  src/zipsupport.cc
  src/composite.cc
  src/composite-read.cc
  src/drawing.cc
  src/blend.cc
)

set( MAIN_HEADERS
  include/olyvslideconv.h
  include/composite.h
  include/imagesupport.h
  include/jpgsupport.h
  include/jpgcachesupport.h
  include/tiffsupport.h
  include/zipsupport.h
  include/blendbkgd.h
)

set( INFO_SRC
  src/imagesupport.cc
  src/tiffsupport.cc
  src/svsinfo.cc
)

set ( INFO_HEADERS
  include/tiffsupport.h
  include/imagesupport.h
  include/jpgsupport.h
)

if (WIN32 OR WIN64 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND MAIN_HEADERS include/console-mswin.h)
  list(APPEND MAIN_HEADERS include/getopt-mswin.h)
  list(APPEND MAIN_SRC src/console-mswin.cc)
  if (USE_MAGICK)
    list(APPEND MAIN_SRC src/initmagick.cc)
  endif()
else()
  list(APPEND MAIN_HEADERS include/console-unix.h)
  list(APPEND MAIN_SRC src/console-unix.cc)
endif()

include_directories( ${MAGICKCORE_INCLUDE_DIRS} ${MAGICKWAND_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${LIBJPEG_INCLUDE_DIRS} ${LIBTIFF_INCLUDE_DIRS} ${MINIZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( olyvslideconv ${MAIN_SRC} ${MAIN_HEADERS} )
target_link_libraries( olyvslideconv ${MAGICKWAND_LIBRARIES} ${MAGICKCORE_LIBRARIES} ${OPENCV_LIBRARIES} ${LIBTIFF_LIBRARIES} ${LIBJPEG_LIBRARIES} ${MINIZIP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable( svsinfo ${INFO_SRC} )
target_link_libraries( svsinfo ${LIBTIFF_LIBRARIES})

install(TARGETS olyvslideconv DESTINATION bin)
install(TARGETS svsinfo DESTINATION bin)

