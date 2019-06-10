AC_INIT([olyvslideconv], 1.0.0, [paulrichards321@gmail.com])
AC_PREREQ(2.59)
AC_LANG([C++])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT
AC_CONFIG_HEADER([conf.h])
AC_CONFIG_SRCDIR([src])
AC_PROG_CXX
AC_CONFIG_MACRO_DIRS([m4])
AC_MSG_CHECKING([pkg-config m4 macros])
if test m4_ifdef([PKG_CHECK_MODULES], [yes], [no]) == yes; then
    AC_MSG_RESULT([yes]);
else
    AC_MSG_RESULT([no]);
    AC_MSG_ERROR([
pkg-config is required.
See pkg-config.freedesktop.org])
fi

AC_CANONICAL_HOST

build_windows=no

# Detect the target system on Windows
case "${host_os}" in
  cygwin*|mingw*)
    build_windows=yes
    ;;
esac

AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
my_jpeg_cflags=""
my_jpeg_libs="none"
AC_DEFINE([MY_JPEG_CFLAGS], [], [C Flags for jpeg library])
AC_DEFINE([MY_JPEG_LIBS], [], [LD Flags for jpeg library])
AC_SEARCH_LIBS([jpeg_CreateDecompress], [jpeg], [
  my_jpeg_libs="jpeg"
])
PKG_CHECK_MODULES([LIBJPEG], [libjpeg >= 1.0.0], [
  my_jpeg_cflags="$LIBJPEG_CFLAGS"
  my_jpeg_libs="$LIBJPEG_LIBS"
])
AC_SEARCH_LIBS([jpeg_CreateDecompress], [jpegturbo], [
  my_jpeg_libs="jpegturbo" 
]) 
PKG_CHECK_MODULES([LIBTURBOJPEG], [libturbojpeg >= 1.0.0], [ 
  my_jpeg_cflags="$LIBTURBOJPEG_CFLAGS"
  my_jpeg_libs="$LIBTURBOJPEG_LIBS"
])

AM_CONDITIONAL([SHELL_WRAPPER], [test "$my_jpeg_libs" = "none" ])
AM_COND_IF([SHELL_WRAPPER], [AC_MSG_FAILURE([Fatal Error: cannot find jpeg library.])])
AC_DEFINE([MY_JPEG_CFLAGS], ["$my_jpeg_cflags"])
AC_DEFINE([MY_JPEG_LIBS], ["$my_jpeg_libs"])
PKG_CHECK_MODULES([OPENCV4], [opencv4 >= 4.0.0], [HAVE_OPENCV=true], [AC_MSG_FAILURE([Fatal Error: Cannot find opencv4 development libraries.])]) 
PKG_CHECK_MODULES([LIBTIFF], [libtiff-4 >= 4.0], [], [AC_MSG_FAILURE([Fatal Error: Cannot find libtiff4 development libraries.])])

AC_LANG_PUSH([C++])
AX_CHECK_COMPILE_FLAG([-Wall], [CXXFLAGS="$CXXFLAGS -Wall"])
AX_CXX_COMPILE_STDCXX_11([ext],[mandatory])
#AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS="$CXXFLAGS -std=c++11 -Wall"])
AC_LANG_POP([C++])

#AC_CHECK_HEADERS([])

INCLUDES="$INCLUDES -I\$(top_srcdir)\include"

AC_CONFIG_FILES([Makefile
  src/Makefile])
AC_OUTPUT

