#------------------------------------------------------------------------------
# What is this?
#-----------------------------------------------------------------------------
# An Olympus/Bacus ini jpeg dataset convertor to TIFF/SVS format or 
# Google Maps jpeg dataset format.
# It is used for converting a directory of jpg files with two or more of their
# accompanying  "FinalScan.ini", "FinalCond.ini", "SlideScan.ini", 
# "SlideCond.ini" files to convert into a set of Google Maps jpeg files
# readable by leaflet or the Google Maps javascript API.
# The TIFF/SVS files are readable by Aperio ImageScope and
# the open source library libopenslide.

#-----------------------------------------------------------------------------
# Instructions for compiling olyvslideconv:
#-----------------------------------------------------------------------------
# Go to method 1 right below to use msys2 on Windows to build olyvslideconv.
# Skip to method 2 if you are using Visual Studio.
# Skip to method 3 if you are using RedHat/CentOS/Scientific Linux.
# Skip to usage notes at the bottom of this document if you are interested
# in how to use the command line program.

# Every line in this file that starts with a # (pound symbol) is not meant to
# be typed into a shell prompt (unix or msys2) or command prompt 
# (windows/visual studio)

# ----------------------------------------------------------------------------
# METHOD 1 - Compiling olyvslideconv on Windows 8/10 with msys2
# ----------------------------------------------------------------------------
# Install MSYS2. Download the package from here: 
# http://www.msys2.org/

# 'pacman' is the command line package manager for MSYS2. 
# Update the MSYS2 system, then close your MSYS2 window by typing: pacman -Syu
# Reopen the MSYS2 window, and update all the other packages by typing: pacman -Syu

MSYS=`uname | grep -i MSYS`
REDHAT_DERIVATIVE_RECENT=`which dnf`
REDHAT_DERIVATIVE=`which yum`
DEBIAN_DERIVATIVE=`which apt`

if [ -n "$MSYS" ]
then

# Install Build Environment:
pacman -Ss 
pacman -S mingw64/mingw-w64-x86_64-gcc mingw64/mingw-w64-x86_64-make mingw64/mingw-w64-x86_64-cmake make msys/pkg-config git

# Install Added Dependencies
pacman -S mingw64/mingw-w64-x86_64-libtiff mingw64/mingw-w64-x86_64-minizip2 mingw64/mingw-w64-x86_64-opencv
dependacies):

# If you haven't already downloaded the source code for olyvslideconv
# grab it here:
git clone https://github.com/paulrichards321/olyvslideconv.git

# Compile it 
cd olyvslideconv
mkdir build
cd build
cmake -G 'Unix Makefiles' ..
if [ $? -eq 0 ]
  echo "CMake successfull!"
  echo "Making project..."
else
  echo "CMake failed. Cannot continue building the project!"
  exit 1
fi
make

# Install the olyvslideconv binary by typing
if [ $? -eq 0 ]
then
  make install
else
  echo "Compilation did not succeed due to one or more errors! Cannot install!"
  exit 1
fi

# You should have a executable olyvslideconv.exe. When moving the
# executable to another system or not executing in a MSYS2 or Mingw shell 
# you will need to also supply the MSYS2 dlls for opencv, libjpeg-turbo, 
# and libtiff, and some other generic C/C++ runtime MSYS2 dlls.

fi

# -----------------------------------------------------------------------------
# METHOD 2: Compiling olyvslideconv on Redhat/CentOS/Scientific Linux 8 or 7
# -----------------------------------------------------------------------------

if [ -n "$REDHAT_DERIVATIVE_RECENT" -o -n "$REDHAT_DERIVATE"]
then

# Install git and development tools if you haven't done so already
sudo yum groupinstall 'Development Tools'
sudo yum install git cmake pkgconfig

# Install libjpeg, libtiff, and ncurses:
sudo yum install libjpeg-turbo-devel libtiff-devel ncurses-devel

# If on Redhat/CentOS/Scientific Linux 8 you can easily grab minizip
if [ -n "$REDHAT_DERIVATE_RECENT" ]
then
sudo yum install minizip-devel

else

# Otherwise if you are on version 7 or earlier you will need to build it
# Follow the below block of instructions to build it if needed
# otherwise safe to skip this part
git clone https://github.com/nmoinvaz/minizip
cd minizip
mkdir build
cd build
$CMAKE ..
make
sudo make install

fi

# -- BUILD OPENCV OR DOWNLOAD DEVELOPMENT RPM FOR IT --
# You will need opencv >= 4. You can either find a place to grab or build it using these directions:
sudo yum install libpng-devel jasper-devel openexr-devel libwebp-devel libdc1394-devel libv4l-devel gstreamer-plugins-base-devel tbb-devel eigen3-devel

# Now grab the opencv source:
git clone https://github.com/opencv/opencv.git
cd opencv

# Now specify a stable version of opencv. 4.1.2 works fine:
git checkout 4.3.0
cd ..
mkdir opencv-build
cd opencv-build
$CMAKE -D CMAKE_BUILD_TYPE=RELEASE -D OPENCV_GENERATE_PKGCONFIG ../opencv/
make
sudo make install

# When you build opencv manually, it places it's library files in 
# /usr/local/lib64. You will need /usr/local/lib64 in your LD Path if it 
# isn't already there. You can add this by dropped adding a file in 
# /etc/ld.so.conf called usrlocallib64.conf:
sudo sh -c 'echo "/usr/local/lib64" > /etc/ld.so.conf.d/usrlocallib64.conf'

# Run ldconfig:
sudo ldconfig

# Now go back to your olyvslideconv directory:
cd ..

# -- GRAB THE SOURCE IF YOU HAVEN'T ALREADY AND COMPILE --
git clone https://github.com/paulrichards321/olyvslideconv.git
mkdir olyvslideconv-build
cd olyvslideconv-build
cmake3 ../olyvslideconv
make

# -- INSTALL THE PROGRAM --
sudo make install

fi
