ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
bin_PROGRAMS = olyvslideconv svsinfo

olyvslideconv_SOURCES = blend.cc composite.cc composite-read.cc drawing.cc imagesupport.cc jpgcachesupport.cc jpgsupport.cc olyvslideconv.cc tiffsupport.cc
if WINDOWS
  olyvslideconv_SOURCES += console-mswin.cc
endif
olyvslideconv_CXXFLAGS = -I$(top_srcdir)/include $(MY_JPEG_CFLAGS) $(LIBTIFF_CFLAGS) $(OPENCV4_CFLAGS)
olyvslideconv_LDADD = $(MY_JPEG_LIBS) $(OPENCV4_LIBS) $(LIBTIFF_LIBS)

svsinfo_SOURCES = imagesupport.cc svsinfo.cc tiffsupport.cc
svsinfo_CXXFLAGS = -I$(top_srcdir)/include $(LIBTIFF_CFLAGS)
svsinfo_LDADD = $(LIBTIFF_LIBS)

bin_PROGRAMS += olyvslideconv
bin_PROGRAMS += svsinfo

