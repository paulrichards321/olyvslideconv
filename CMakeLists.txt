cmake_minimum_required(VERSION 2.8)
project( olyvslideconv LANGUAGES CXX )

find_package(PkgConfig)
find_package(OpenCV)
find_package(ImageMagick COMPONENTS MagickCore MagickWand)
find_package(TIFF)
find_package(JPEG)

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

add_definitions(-Wall)

if ("${USE_MAGICK}" STREQUAL "")
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
  add_definitions(-DMAGICKCORE_QUANTUM_DEPTH=8)
  add_definitions(-DMAGICKCORE_HDRI_ENABLE=0)
  add_definitions(-DMAGICK_CODER_MODULE_PATH="/mingw64/lib/ImageMagick-7.0.8/modules-Q16HDRI/coders")
  add_definitions(-DMAGICK_CODER_FILTER_PATH="/mingw64/lib/ImageMagick-7.0.8/modules-Q16HDRI/filters")
  add_definitions(-DMAGICK_HOME="/mingw64/lib/ImageMagick-7.0.8")
  add_definitions(-DMAGICK_MAP_LIMIT="6GB")
  add_definitions(-DMAGICK_MEMORY_LIMIT="6GB")
  add_definitions(-DMAGICK_DISK_LIMIT="32GB")
  add_definitions(-DMAGICK_AREA_LIMIT="100MPP")
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

if ("${LIBZIP_LIBRARY}" STREQUAL "")
  if (PKG_CONFIG_FOUND)
	  pkg_check_modules(LIBZIP REQUIRED libzip)
  else()
	  find_library(LIBZIP_LIBRARIES NAMES zip libzip libzip.dll zip.dll)
    if (LIBZIP_LIBRARIES_NOTFOUND)
      message(FATAL_ERROR "LibZip not found!")
    endif()
  endif()
endif()

set(ENABLE_EXPORTS, true)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS_DEBUG "-g -pg -O2")

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

if (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND MAIN_HEADERS include/console-mswin.h)
  list(APPEND MAIN_HEADERS include/getopt-mswin.h)
  list(APPEND MAIN_SRC src/console-mswin.cc)
else()
  list(APPEND MAIN_HEADERS include/console-unix.h)
endif()


include_directories( ${MAGICKCORE_INCLUDE_DIRS} ${MAGICKWAND_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${LIBJPEG_INCLUDE_DIRS} ${LIBTIFF_INCLUDE_DIRS} ${LIBZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( olyvslideconv ${MAIN_SRC} ${MAIN_HEADERS} )
target_link_libraries( olyvslideconv ${MAGICKCORE_LIBRARIES} ${MAGICKWAND_LIBRARIES} ${OPENCV_LIBRARIES} ${LIBTIFF_LIBRARIES} ${LIBJPEG_LIBRARIES} ${LIBZIP_LIBRARIES})

add_executable( svsinfo ${INFO_SRC} )
target_link_libraries( svsinfo ${LIBTIFF_LIBRARIES})

install(TARGETS olyvslideconv DESTINATION bin)
install(TARGETS svsinfo DESTINATION bin)

