cmake_minimum_required(VERSION 2.8)
project( olyvslideconv )
find_package( OpenCV REQUIRED )
find_package( JPEG REQUIRED )
find_package( TIFF REQUIRED )

macro(my_use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(my_use_cxx11)

my_use_cxx11()

if (${JPEG_FOUND} MATCHES false)
  message(FATAL_ERROR "libjpeg not found. Please install libjpeg-turbo development library or similiar. Cannot continue!")
endif()

if (${TIFF_FOUND} MATCHES false)
  message(FATAL_ERROR "libtiff not found. Please install libtiff development libraries. Cannot continue!")
endif()

set( NAME_SRC
  src/olyvslideconv.cpp
  src/jpgcachesupport.cpp
  src/imagesupport.cpp
  src/jpgsupport.cpp
  src/tiffsupport.cpp
  src/composite.cpp
  src/composite-read.cpp
  src/drawing.cpp
  src/blend.cpp
)

set( NAME_HEADERS
  include/composite.h
  include/imagesupport.h
  include/jpgsupport.h
  include/jpgcachesupport.h
  include/tiffsupport.h
)

if (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND NAME_HEADERS include/console-mswin.h)
  list(APPEND NAME_HEADERS include/getopt-mswin.h)
else()
  list(APPEND NAME_HEADERS include/console-unix.h)
endif()


include_directories( ${OpenCV_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${TIFF_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( olyvslideconv ${NAME_SRC} ${NAME_HEADERS} )
target_link_libraries( olyvslideconv ${OpenCV_LIBS} ${JPEG_LIBRARIES} ${TIFF_LIBRARIES})

install(TARGETS olyvslideconv DESTINATION bin)

