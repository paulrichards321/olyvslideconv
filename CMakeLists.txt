cmake_minimum_required(VERSION 2.8)
project( olyvslideconv LANGUAGES CXX )
find_package( JPEG REQUIRED )
find_package( TIFF REQUIRED )
find_package( OpenCV REQUIRED )

set(ENABLE_EXPORTS, true)
#set (CMAKE_CXX_STANDARD 11)
#set (CMAKE_CXX_EXTENSIONS OFF)
#set (CMAKE_CXX_STANDARD_REQUIRED YES)

if (${JPEG_FOUND} MATCHES false)
  message(FATAL_ERROR "libjpeg not found. Please install libjpeg-turbo development library or similiar. Cannot continue!")
endif()

if (${TIFF_FOUND} MATCHES false)
  message(FATAL_ERROR "libtiff not found. Please install libtiff development libraries. Cannot continue!")
endif()

set( MAIN_SRC
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

set( MAIN_HEADERS
  include/composite.h
  include/imagesupport.h
  include/jpgsupport.h
  include/jpgcachesupport.h
  include/tiffsupport.h
)

set( INFO_SRC
  src/imagesupport.cpp
  src/tiffsupport.cpp
  src/svsinfo.cpp
)

set ( INFO_HEADERS
  include/tiffsupport.h
  include/imagesupport.h
  include/jpgsupport.h
)

if (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND MAIN_HEADERS include/console-mswin.h)
  list(APPEND MAIN_HEADERS include/getopt-mswin.h)
else()
  list(APPEND MAIN_HEADERS include/console-unix.h)
endif()


include_directories( ${OpenCV_INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} ${TIFF_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable( olyvslideconv ${MAIN_SRC} ${MAIN_HEADERS} )
target_link_libraries( olyvslideconv ${OpenCV_LIBS} ${JPEG_LIBRARIES} ${TIFF_LIBRARIES})

add_executable( svsinfo ${INFO_SRC} )
target_link_libraries( svsinfo ${OpenCV_LIBS} ${TIFF_LIBRARIES} ${OpenCV_LIBS} )

install(TARGETS olyvslideconv DESTINATION bin)
install(TARGETS svsinfo DESTINATION bin)

