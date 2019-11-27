cmake_minimum_required(VERSION 2.8)
project( olyvslideconv LANGUAGES CXX )

find_package(PkgConfig)
find_package(OpenCV)
find_package(TIFF)
find_package(JPEG)

if (TIFF_FOUND)
	set (LIBTIFF_LIBRARIES ${TIFF_LIBRARIES})
	set (LIBTIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
endif()

if (DEFINED OpenCV_LIBS)
	set (OPENCV_LIBRARIES ${OpenCV_LIBS})
	set (OPENCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
endif()

if (NOT JPEG_FOUND AND PKG_CONFIG_FOUND)
	pkg_check_modules(LIBJPEG REQUIRED libjpeg)
endif()

if (JPEG_FOUND)
	set (LIBJPEG_LIBRARIES ${JPEG_LIBRARIES})
	set (LIBJPEG_INCLUDE ${JPEG_INCLUDE_DIRS})
endif()

if (PKG_CONFIG_FOUND)
	if (NOT TIFF_FOUND)
		pkg_check_modules(LIBTIFF REQUIRED libtiff-4)
	endif()
	pkg_check_modules(LIBZIP libzip)
	if (NOT DEFINED OPENCV_LIBRARIES)
		pkg_check_modules(OPENCV REQUIRED opencv4)
	endif()
endif()

if (NOT DEFINED LIBZIP_LIBRARIES)
	set(LIBZIP_LIBRARIES zip)
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


include_directories( ${OPENCV_INCLUDE_DIRS} ${LIBJPEG_INCLUDE_DIRS} ${LIBTIFF_INCLUDE_DIRS} ${LIBZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( olyvslideconv ${MAIN_SRC} ${MAIN_HEADERS} )
target_link_libraries( olyvslideconv ${OPENCV_LIBRARIES} ${LIBTIFF_LIBRARIES} ${LIBJPEG_LIBRARIES} ${LIBZIP_LIBRARIES})

add_executable( svsinfo ${INFO_SRC} )
target_link_libraries( svsinfo ${LIBTIFF_LIBRARIES})

install(TARGETS olyvslideconv DESTINATION bin)
install(TARGETS svsinfo DESTINATION bin)

