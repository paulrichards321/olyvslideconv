/*************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017 
https://github.com/paulrichards321/oly2gmap

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
#include "console-mswin.h"
#include "getopt-mswin.h"
#else
#include "console-unix.h"
#include <unistd.h>
#include <getopt.h>
#endif
#include "jpgcachesupport.h"
#include "jpgsupport.h"
#include "tiffsupport.h"
#include "zipsupport.h"
#include "composite.h"
#include "safebmp.h"
#include "blendbkgd.h"

#define MAX_RESIZE_MEMORY 512
#define MAX_JPG_CACHE_MEMORY 256

#define OLYVSLIDE_SCANONLY 3
#define OLYVSLIDE_GOOGLE   2
#define OLYVSLIDE_TIF      1

extern JpgCache jpgCache;

std::string bool2txt(bool cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
}

void quickEnv(const char* var, const char* value, int debugLevel)
{
  char full_char[512];
  std::string full=var;
  full.append("=");
  full.append(value);
  const char * full_const = full.c_str();
  full_char[0] = 0;
  strncpy(full_char, full_const, sizeof(full_char)-1);
  if (debugLevel > 0)
  {
    std::cout << "ENV: " << full_char << std::endl;
  }
  putenv(full_char);
}

class SlideLevel
{
protected:
  friend class SlideConvertor;
  int debugLevel;
  int outputType;
  int olympusLevel;
  int readDirection;
  int readZLevel;
  int outLevel;
  bool center;
  bool tiled;
  bool scanBkgd;
  int64_t maxMemory;
  int64_t srcTotalWidth;
  int64_t srcTotalHeight;
  int64_t srcTotalWidthL2;
  int64_t srcTotalHeightL2;
  int64_t L2Size;
  int64_t destTotalWidth, destTotalWidth2;
  int64_t destTotalHeight, destTotalHeight2;
  int finalOutputWidth, finalOutputWidth2;
  int finalOutputHeight, finalOutputHeight2;
  int inputTileWidth;
  int inputTileHeight;
  int quality;
  int totalSubTiles;
  int xSubTile, ySubTile;
  int64_t readWidthL2;
  int64_t readHeightL2;
  int64_t readWidth;
  int64_t readHeight;
  int64_t grabWidthRead;
  int64_t grabHeightRead;
  double magnifyX;
  double magnifyY;
  double destTotalWidthDec;
  double destTotalHeightDec;
  double xScale, yScale;
  double xScaleL2, yScaleL2;
  double xScaleReverse, yScaleReverse;
  double xScaleResize, yScaleResize;
  double xBlendFactor;
  double yBlendFactor;
  int xBkgdLimit;
  int yBkgdLimit;
  double grabWidthA, grabWidthB;
  double grabHeightA, grabHeightB;
  double grabWidthL2, grabHeightL2;
  double xSrcRead;
  double ySrcRead;
  double xSrc;
  double ySrc;
  bool readOkL2;
  int64_t inputTileWidthRead;
  int64_t inputTileHeightRead;
  int64_t inputSubTileWidthRead;
  int64_t inputSubTileHeightRead;
  int64_t xMargin;
  int64_t yMargin;
  #ifndef USE_MAGICK
  int scaleMethod;
  int scaleMethodL2;
  #else
  Magick::FilterType scaleMethod;
  Magick::FilterType scaleMethodL2;
  #endif
  int64_t totalXSections, totalYSections;
  unsigned char bkgdColor;
  BlendSection **xSubSections;
  BlendSection **ySubSections;
  safeBmp* pBitmapSrc;
  safeBmp* pBitmapL2;
  safeBmp* pBitmap4;
  safeBmp* pBitmapFinal;
  safeBmp bitmap1;
  safeBmp subTileBitmap;
  safeBmp safeImgScaled;
  safeBmp sizedBitmap;
  safeBmp sizedBitmap2;
  safeBmp safeScaledL2Mini;
  safeBmp safeScaledL2Mini2;
  safeBmp bitmapBlended;
  int64_t bitmap4Size;
  bool fillin;
  int64_t xLevelOffset;
  int64_t yLevelOffset;
  int64_t xStartTile;
  int xCenter;
  int yCenter;
  double xStartSrc;
  double yStartSrc;
  int64_t xTile;
  int64_t yTile;
  int64_t xDest;
  int64_t yDest;
  int64_t outputLvlTotalWidth;
  int64_t outputLvlTotalHeight;
#ifndef USE_MAGICK
  cv::Mat *pImgScaled;
  cv::Mat *pImgScaledL2Mini;
#else
  Magick::Image *pImgScaled;
  Magick::Image *pImgScaledL2Mini;
  Magick::MagickWand *magickWand;
  Magick::PixelWand *pixelWand;
#endif
  std::string tileName;
  int writeOutputWidth;
  int writeOutputHeight;
  int perc, percOld;
  bool onePercHit;
public:
  SlideLevel()
  {
    debugLevel=0;
    outputType=0;
    olympusLevel=0;
    readDirection=0;
    readZLevel=0;
    outLevel=0;
    center=false;
    tiled=false;
    scanBkgd=false;
    srcTotalWidth=0;
    srcTotalHeight=0;
    srcTotalWidthL2=0;
    srcTotalHeightL2=0;
    L2Size=0;
    destTotalWidth=0;
    destTotalWidth2=0;
    destTotalHeight=0;
    destTotalHeight2=0;
    finalOutputWidth=0;
    finalOutputHeight=0;
    inputTileWidth=0;
    inputTileHeight=0;
    quality=0;
    readWidthL2=0;
    readHeightL2=0;
    readWidth=0;
    readHeight=0;
    grabWidthRead=0;
    grabHeightRead=0;
    magnifyX=0;
    magnifyY=0;
    destTotalWidthDec=0.0;
    destTotalHeightDec=0.0;
    xScale=0.0, yScale=0.0;
    xScaleL2=0.0, yScaleL2=0.0;
    xScaleReverse=0.0, yScaleReverse=0.0;
    xScaleResize=0.0, yScaleResize=0.0;
    xBlendFactor = 1.0;
    yBlendFactor = 1.0;
    xBkgdLimit=0;
    yBkgdLimit=0;
    grabWidthA=0, grabWidthB=0;
    grabHeightA=0, grabHeightB=0;
    grabWidthL2=0, grabHeightL2=0;
    xSrcRead=0.0;
    ySrcRead=0.0;
    xSrc=0.0;
    ySrc=0.0;
    readOkL2=false;
    inputTileWidthRead=0;
    inputTileHeightRead=0;
    xMargin=0;
    yMargin=0;
    #ifndef USE_MAGICK
    scaleMethod=0;
    scaleMethodL2=0;
    #else
    scaleMethod=Magick::MitchellFilter;
    scaleMethodL2=Magick::MitchellFilter;
    #endif
    totalXSections=0, totalYSections=0;
    bkgdColor=0;
    xSubSections=NULL;
    ySubSections=NULL;
    bitmap1.data=NULL;
    sizedBitmap.data=NULL;
    sizedBitmap2.data=NULL;
    pBitmapSrc=NULL;
    pBitmapL2=NULL;
    pBitmap4=NULL;
    pBitmapFinal=NULL;
    pImgScaled=NULL;
    pImgScaledL2Mini=NULL;
    bitmap4Size=0;
    fillin=false;
    xLevelOffset=0;
    yLevelOffset=0;
    xStartTile=0;
    xCenter=0;
    yCenter=0;
    xStartSrc=0.0;
    yStartSrc=0.0;
    xTile=0;
    yTile=0;
    xDest=0;
    yDest=0;
    outputLvlTotalWidth=0;
    outputLvlTotalHeight=0;
    writeOutputWidth=0;
    writeOutputHeight=0;
  }
};


class SlideConvertor
{
protected:
  CompositeSlide *slide;
  Tiff *mTif;
  ZipFile *mZip;
  std::ofstream *logFile;
  std::string errMsg;
  std::string mOutputFile;
  std::string mOutputDir;
  std::string mFileNameOnly;
  int64_t mBaseTotalWidth, mBaseTotalHeight;
  int64_t mBaseTotalWidth2, mBaseTotalHeight2;
  bool mValidObject;
  int mOutputType;
  bool mBlendTopLevel, mBlendByRegion;
  bool mCreateLog;
  int mBaseLevel;
  bool mIncludeZStack;
  bool mCenter;
  int mQuality;
  int mStep, mZSteps;
  int mLastZLevel, mLastDirection;
  int mTopOutLevel;
  int64_t mMaxSide;
  int mMaxMemory;
  int mDebugLevel;
  safeBmp *mpImageL2;
  int64_t mTotalXSections, mTotalYSections;
  BlendSection **mxSubSections;
  BlendSection **mySubSections;
public:
  #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  static const char mPathSeparator='\\';
  #else
  static const char mPathSeparator='/';
  #endif
  static const int64_t mMaxZipBufferBytes=1024000000LL;
public:
  SlideConvertor();
  ~SlideConvertor() { closeRelated(); }
  void closeRelated();
  std::string getErrMsg() { return errMsg; }
  int open(std::string inputFile, std::string outputFile, bool useOpenCV, bool blendTopLevel, bool blendByRegion, bool markOutline, bool includeZStack, bool createLog, int quality, int64_t bestXOffset, int64_t bestYOffset, int maxMemory, int outputType, int debugLevel);
  bool my_mkdir(std::string name);
  void calcCenters(int outLevel, int64_t& xCenter, int64_t& yCenter);
  int convert();
  int outputLevel(int olympusLevel, int magnification, int outLevel, bool tiled, bool scanBkgd, bool flushArchive, int64_t readWidthL2, int64_t readHeightL2, safeBmp *pBitmapL2);
  int checkFullL2(int64_t *pReadWidthL2, int64_t *pReadHeightL2, safeBmp **pFullL2);
  int convert2Tif();
  int convert2Gmap();
  void tileCleanup(SlideLevel &l);
  void blendL2WithSrc(SlideLevel &l);
  void scanSrcTileBkgd(SlideLevel& l);
  void processSrcTile(SlideLevel& l);
  void printPercDone(SlideLevel& l);
};


bool SlideConvertor::my_mkdir(std::string name)
{
  struct stat info;
  std::string errorMkdir;
  if (stat(name.c_str(), &info) != 0)
  {
    if (!platform_mkdir(name, &errorMkdir))
    {
      std::cout << "Fatal Error: Cannot create directory: " << name << ": " << errorMkdir << " Quiting!";
      return false;
    }
  }
  else if (!(info.st_mode & S_IFDIR))
  {
    std::cout << "Fatal Error: " << name << " exists already, and is not a directory! Quiting!";
    return false;
  }
  return true;
}


SlideConvertor::SlideConvertor()
{
  slide = 0;
  mTif = 0;
  mZip = 0;
  logFile = 0;
  mOutputType = 0;
  mCenter = false;
  mValidObject = false;
  mStep=0;
  mLastZLevel=-1;
  mLastDirection=-1;
  mZSteps=0;
  mBaseTotalWidth=0;
  mBaseTotalWidth2=0;
  mBaseTotalHeight=0;
  mBaseTotalHeight2=0;
  mBaseLevel=0;
  mIncludeZStack=true;
  mQuality=90;
  mpImageL2=NULL;
  mTotalXSections = 0;
  mTotalYSections = 0;
  mxSubSections = NULL;
  mySubSections = NULL;
}


void SlideConvertor::calcCenters(int outLevel, int64_t &xCenter, int64_t &yCenter)
{
  int64_t baseWidth = slide->getActualWidth(mBaseLevel);
  int64_t baseHeight = slide->getActualHeight(mBaseLevel);
  int64_t gmapMaxSide = (1 << mTopOutLevel) * 256;
  int64_t xBaseCenter = (gmapMaxSide - baseWidth) / 2;
  int64_t yBaseCenter = (gmapMaxSide - baseHeight) / 2;
  
  int64_t xTopCenter = xBaseCenter >> mTopOutLevel;
  xCenter = xTopCenter << outLevel;

  int64_t yTopCenter = yBaseCenter >> mTopOutLevel;
  yCenter = yTopCenter << outLevel;
}



void SlideConvertor::tileCleanup(SlideLevel &l)
{
  if (l.pImgScaled)
  {
    #ifndef USE_MAGICK
    l.pImgScaled->release();
    delete l.pImgScaled;
    #endif
    l.pImgScaled = 0;
  }
  if (l.pImgScaledL2Mini)
  {
    #ifndef USE_MAGICK
    l.pImgScaledL2Mini->release();
    delete l.pImgScaledL2Mini;
    #endif
    l.pImgScaledL2Mini = 0;
  }
  safeBmpFree(&l.subTileBitmap);
  safeBmpFree(&l.sizedBitmap2);
  safeBmpFree(&l.safeScaledL2Mini);
  safeBmpFree(&l.safeScaledL2Mini2);
}

// Scale the larger complete L2 image into a tiled smaller 
// mini version
// of it if L2 scaling is requested and the L2 pyramid level was
// read success
void SlideConvertor::blendL2WithSrc(SlideLevel &l)
{
  safeBmp bitmapL2Mini;
  BlendBkgdArgs blendArgs;

  safeBmpClear(&bitmapL2Mini);
  int64_t xSrcStartL2=round(l.xSrc * l.xScaleL2);
  int64_t ySrcStartL2=round(l.ySrc * l.yScaleL2);
  int64_t xDestStartL2=0, yDestStartL2=0;

  if (xSrcStartL2 < 0)
  {
    xDestStartL2 = abs(xSrcStartL2);
    xSrcStartL2 = 0;
  }
  if (ySrcStartL2 < 0)
  {
    yDestStartL2 = abs(ySrcStartL2);
    ySrcStartL2 = 0;
  }
  if (safeBmpAlloc2(&bitmapL2Mini, (int64_t) l.grabWidthL2, (int64_t) l.grabHeightL2)==NULL)
  {
    return;
  }
  safeBmpByteSet(&bitmapL2Mini, l.bkgdColor);
  safeBmpCpy(&bitmapL2Mini, xDestStartL2, yDestStartL2, l.pBitmapL2, xSrcStartL2, ySrcStartL2, (int64_t) l.grabWidthL2, (int64_t) l.grabHeightL2);
  #ifndef USE_MAGICK
  l.pImgScaledL2Mini = new cv::Mat;
  cv::Mat imgSrc(l.grabHeightL2, l.grabWidthL2, CV_8UC3, bitmapL2Mini.data);
  cv::Size scaledSize(l.finalOutputWidth, l.finalOutputHeight);
  cv::resize(imgSrc, *l.pImgScaledL2Mini, scaledSize, l.xScaleResize, l.yScaleResize, l.scaleMethodL2);
  imgSrc.release();
  safeBmpInit(&l.safeScaledL2Mini, l.pImgScaledL2Mini->data, l.finalOutputWidth, l.finalOutputHeight);
  #else
  Magick::MagickSetCompression(l.magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(l.magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(l.magickWand, 8);
  Magick::MagickSetImageAlphaChannel(l.magickWand, Magick::OffAlphaChannel);
  //Magick::MagickNewImage(l.magickWand, l.grabWidthL2, l.grabHeightL2, l.pixelWand);
  //Magick::MagickImportImagePixels(l.magickWand, 0, 0, l.grabWidthL2, l.grabHeightL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  Magick::MagickConstituteImage(l.magickWand, l.grabWidthL2, l.grabHeightL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  Magick::MagickResizeImage(l.magickWand, l.finalOutputWidth, l.finalOutputHeight, l.scaleMethodL2);
  safeBmpAlloc2(&l.safeScaledL2Mini, l.finalOutputWidth, l.finalOutputHeight);
  Magick::MagickExportImagePixels(l.magickWand, 0, 0, l.finalOutputWidth, l.finalOutputHeight, "RGB", Magick::CharPixel, l.safeScaledL2Mini.data);
  Magick::ClearMagickWand(l.magickWand);
  #endif
  
  if (l.finalOutputWidth != l.finalOutputWidth2 || l.finalOutputHeight != l.finalOutputHeight2)
  {
    safeBmpAlloc2(&l.safeScaledL2Mini2, (int64_t) l.finalOutputWidth2, (int64_t) l.finalOutputHeight2);
    safeBmpByteSet(&l.safeScaledL2Mini2, l.bkgdColor);
    safeBmpCpy(&l.safeScaledL2Mini2, 0, 0, &l.safeScaledL2Mini, 0, 0, l.finalOutputWidth, l.finalOutputHeight);
  }
  if (l.debugLevel > 1)
  {
    std::string errMsg;
    std::string l2TileName=l.tileName;
    l2TileName.append(".l2.jpg");
    bool writeOk=my_jpeg_write(l2TileName, l.safeScaledL2Mini.data, l.finalOutputWidth2, l.finalOutputHeight2, l.quality, &errMsg);
    if (!writeOk) 
    {
      std::cout << "Error writing debug file '" << l2TileName << "' errMsg: " << errMsg << std::endl;
    }
  }
  if (mBlendByRegion)
  {
    // slide->blendLevelsByRegion(l.safeScaledL2Mini.data, l.pBitmapSrc, round(l.xSrc), round(l.ySrc), round(l.grabWidthA), round(l.grabHeightA), l.inputTileWidth, l.inputTileHeight, l.xScaleReverse, l.yScaleReverse, l.olympusLevel); 
    slide->blendLevelsByRegion(&l.safeScaledL2Mini, l.pBitmapSrc, round(l.xSrc), round(l.ySrc), l.xScaleReverse, l.yScaleReverse, l.olympusLevel); 
    safeBmpInit(&l.bitmapBlended, l.safeScaledL2Mini.data, l.finalOutputWidth2, l.finalOutputHeight2);
    l.pBitmapFinal = &l.bitmapBlended;
  }
  else
  {
    safeBmpCpy(l.pBitmap4, 0, 0, l.pBitmapSrc, 0, 0, l.pBitmap4->width, l.pBitmap4->height);
    blendArgs.x = round(l.xDest) - l.xCenter;
    blendArgs.y = round(l.yDest) - l.yCenter;
    blendArgs.pSafeDest = l.pBitmap4;
    blendArgs.pSafeSrc = l.pBitmapSrc;
    blendArgs.pSafeSrcL2 = &l.safeScaledL2Mini;
    blendArgs.xFactor = l.xBlendFactor;
    blendArgs.yFactor = l.yBlendFactor;
    blendArgs.xFreeMap = l.xSubSections;
    blendArgs.yFreeMap = l.ySubSections;
    blendArgs.xSize = l.totalXSections;
    blendArgs.ySize = l.totalYSections;
    blendArgs.xLimit = l.xBkgdLimit;
    blendArgs.yLimit = l.yBkgdLimit;
    blendArgs.bkgdColor = l.bkgdColor;
    blendLevelsByBkgd(&blendArgs);
    l.pBitmapFinal = l.pBitmap4;
  }
  safeBmpFree(&bitmapL2Mini);
}


void SlideConvertor::scanSrcTileBkgd(SlideLevel& l)
{
  BlendBkgdArgs blendArgs;
  blendArgs.pSafeSrc = l.pBitmapSrc;
  blendArgs.pSafeDest = NULL;
  blendArgs.pSafeSrcL2 = NULL;
  blendArgs.x = round(l.xSrcRead);
  blendArgs.y = round(l.ySrcRead);
  blendArgs.xFactor = l.xBlendFactor;
  blendArgs.yFactor = l.yBlendFactor;
  blendArgs.xFreeMap = l.xSubSections;
  blendArgs.yFreeMap = l.ySubSections;
  blendArgs.xSize = l.totalXSections;
  blendArgs.ySize = l.totalYSections;
  blendArgs.xLimit = l.xBkgdLimit;
  blendArgs.yLimit = l.yBkgdLimit;
  blendArgs.bkgdColor = l.bkgdColor;
  blendLevelsScan(&blendArgs);
}


void SlideConvertor::processSrcTile(SlideLevel& l)
{
  // Check if read height and width returned from composite read
  // came back smaller than expected, resize the bitmap if so
  if (l.readWidth != l.grabWidthRead || l.readHeight != l.grabHeightRead)
  {
    safeBmpAlloc2(&l.sizedBitmap, l.grabWidthRead, l.grabHeightRead);
    safeBmpByteSet(&l.sizedBitmap, l.bkgdColor);

    int64_t copyWidth=l.readWidth;
    if (copyWidth > l.grabWidthRead) copyWidth=l.grabWidthRead;
    int64_t copyHeight=l.readHeight;
    if (copyHeight > l.grabHeightRead) copyHeight=l.grabHeightRead;
    safeBmpCpy(&l.sizedBitmap, 0, 0, l.pBitmapSrc, 0, 0, copyWidth, copyHeight);
    l.pBitmapSrc = &l.sizedBitmap;
    l.pBitmapFinal = &l.sizedBitmap;
  }
  // Check if the grabbed data needs to be scaled in or out
  // If we are at the very bottom of the image pyramid, this part will
  // be skipped because no scaling will be done
  if (l.grabWidthRead!=l.inputSubTileWidthRead || l.grabHeightRead!=l.inputSubTileHeightRead)
  {
    #ifndef USE_MAGICK
    l.pImgScaled = new cv::Mat;
    cv::Mat imgSrc(l.grabHeightRead, l.grabWidthRead, CV_8UC3, l.pBitmapSrc->data);
    cv::Size scaledSize(l.inputSubTileWidthRead, l.inputSubTileHeightRead);
    cv::resize(imgSrc, *l.pImgScaled, scaledSize, l.xScaleReverse, l.yScaleReverse, l.scaleMethod);
    imgSrc.release();
    safeBmpInit(&l.safeImgScaled, l.pImgScaled->data, l.inputSubTileWidthRead, l.inputSubTileHeightRead);
    #else
    Magick::MagickSetCompression(l.magickWand, Magick::NoCompression);
    Magick::MagickSetImageType(l.magickWand, Magick::TrueColorType);
    Magick::MagickSetImageDepth(l.magickWand, 8);
    Magick::MagickSetImageAlphaChannel(l.magickWand, Magick::OffAlphaChannel);
    //Magick::MagickNewImage(l.magickWand, l.grabWidthRead, l.grabHeightRead, l.pixelWand);
    //Magick::MagickImportImagePixels(l.magickWand, 0, 0, l.grabWidthRead, l.grabHeightRead, "RGB", Magick::CharPixel, l.pBitmapSrc->data);
    Magick::MagickConstituteImage(l.magickWand, l.grabWidthRead, l.grabHeightRead, "RGB", Magick::CharPixel, l.pBitmapSrc->data);
    Magick::MagickResizeImage(l.magickWand, l.inputSubTileWidthRead, l.inputSubTileHeightRead, l.scaleMethod);
    safeBmpAlloc2(&l.safeImgScaled, l.inputSubTileWidthRead, l.inputSubTileHeightRead);
    Magick::MagickExportImagePixels(l.magickWand, 0, 0, l.inputSubTileWidthRead, l.inputSubTileHeightRead, "RGB", Magick::CharPixel, l.safeImgScaled.data);
    //safeBmpInit(&l.safeImgScaled, (BYTE*) QueueAuthenticPixels(l.pImgScaled, 0, 0, l.inputTileWidthRead, l.inputTileHeightRead, NULL), l.inputTileWidthRead, l.inputTileHeightRead);  
    Magick::ClearMagickWand(l.magickWand);
    #endif
    l.pBitmapSrc = &l.safeImgScaled;
    l.pBitmapFinal = &l.safeImgScaled;
  }
  // Check if the current input tile is smaller than what is needed
  // to process the L2 background with it. This can occur
  // if the grabbed original composite source image has different
  // dimensions (cx,cy) than what is need because of a margin
  // that is need to center the image the side of the grabbed image
  if (l.totalSubTiles==1 && (l.inputSubTileWidthRead!=l.inputTileWidth || l.inputSubTileHeightRead!=l.inputTileHeight))
  {
    safeBmpAlloc2(&l.sizedBitmap2, l.inputTileWidth, l.inputTileHeight);
    safeBmpByteSet(&l.sizedBitmap2, l.bkgdColor);
    int64_t copyWidth = l.inputSubTileWidthRead;
    if (l.xMargin + copyWidth > l.inputTileWidth)
    {
      copyWidth -= (l.xMargin + copyWidth) - l.inputTileWidth;
    }
    int64_t copyHeight = l.inputSubTileHeightRead;
    if (l.yMargin + copyHeight > l.inputTileHeight)
    {
      copyHeight -= (l.yMargin + copyHeight) - l.inputTileHeight;
    }
    if (copyWidth > 0 && copyHeight > 0)
    {
      safeBmpCpy(&l.sizedBitmap2, l.xMargin, l.yMargin, l.pBitmapSrc, 0, 0, copyWidth, copyHeight);
    }
    l.pBitmapSrc = &l.sizedBitmap2;
    l.pBitmapFinal = &l.sizedBitmap2;
  }          
  else if (l.totalSubTiles > 1)
  {
    int64_t copyWidth = l.inputSubTileWidthRead;
    if (l.xMargin + copyWidth > l.inputTileWidth)
    {
      copyWidth -= (l.xMargin + copyWidth) - l.inputTileWidth;
    }
    int64_t copyHeight = l.inputSubTileHeightRead;
    if (l.yMargin + copyHeight > l.inputTileHeight)
    {
      copyHeight -= (l.yMargin + copyHeight) - l.inputTileHeight;
    }
    int64_t xSubLoc = l.xSubTile * (int64_t) round((double) l.inputTileWidth / (double) l.totalSubTiles);
    int64_t ySubLoc = l.ySubTile * (int64_t) round((double) l.inputTileHeight / (double) l.totalSubTiles);
    xSubLoc += l.xMargin;
    ySubLoc += l.yMargin;
    if (copyWidth > 0 && copyHeight > 0)
    {
      safeBmpCpy(&l.bitmap1, xSubLoc, ySubLoc, l.pBitmapSrc, 0, 0, copyWidth, copyHeight);
    }
    l.pBitmapSrc = &l.bitmap1;
    l.pBitmapFinal = &l.bitmap1;
  }
  if (l.debugLevel > 1)
  {
    std::string preTileName = l.tileName;
    std::string errMsg;
    preTileName.append(".pre.jpg");
    bool writeOk=my_jpeg_write(preTileName, l.pBitmapSrc->data, l.pBitmapSrc->width, l.pBitmapSrc->height, l.quality, &errMsg);
    if (!writeOk)
    {
      std::cout << "Error writing debug file: " << errMsg << std::endl;
    }
  }
}


void SlideConvertor::printPercDone(SlideLevel& l)
{
  if (l.outputType == OLYVSLIDE_SCANONLY)
  {
    l.perc=(int)(((double) l.ySrc / (double) l.srcTotalHeight) * 100);
  }
  else
  {
    l.perc=(int)(((double) l.yDest / (double) l.outputLvlTotalHeight) * 100);
  }
  if (l.perc>100)
  {
    l.perc=100;
  }
  if (l.perc>l.percOld)
  {
    l.percOld=l.perc;
    retractCursor();
    std::cout << l.perc << "% done...    " << std::flush;
  }
  else if (l.onePercHit==false)
  {
    retractCursor();
    std::cout << l.perc << "% done...    " << std::flush;
    l.onePercHit = true;
  }
}


int SlideConvertor::outputLevel(int olympusLevel, int magnification, int outLevel, bool tiled, bool scanBkgd, bool flushArchive, int64_t readWidthL2, int64_t readHeightL2, safeBmp *pBitmapL2)
{
  std::ostringstream output;
  SlideLevel l;

  l.debugLevel = mDebugLevel;
  l.maxMemory = mMaxMemory;
  l.outputType = mOutputType;
  if (scanBkgd == true)
  {
    l.outputType = OLYVSLIDE_SCANONLY;
  }
  l.tiled = tiled;
  l.center = mCenter;
  l.olympusLevel = olympusLevel;
  l.magnifyX=magnification;
  l.magnifyY=magnification;
  l.outLevel=outLevel;
  l.readWidthL2 = readWidthL2;
  l.readHeightL2 = readHeightL2;
  l.pBitmapL2 = pBitmapL2;
  l.bkgdColor=255;
  #ifndef USE_MAGICK
  l.scaleMethod=cv::INTER_CUBIC;
  l.scaleMethodL2=cv::INTER_CUBIC;
  #else
  l.scaleMethod=Magick::MitchellFilter;
  l.scaleMethodL2=Magick::MitchellFilter;
  #endif
  l.scanBkgd = scanBkgd;
  l.fillin = (mBlendTopLevel && scanBkgd==false && l.olympusLevel < 2 && slide->checkLevel(2)) ? true : false;

  l.srcTotalWidth = slide->getActualWidth(olympusLevel);
  l.srcTotalHeight = slide->getActualHeight(olympusLevel);
  if (l.fillin)
  {
    l.srcTotalWidthL2 = slide->getActualWidth(2);
    l.srcTotalHeightL2 = slide->getActualHeight(2);
  }
  l.destTotalWidthDec = (double) mBaseTotalWidth / (double) l.magnifyX;
  l.destTotalHeightDec = (double) mBaseTotalHeight / (double) l.magnifyY;
  l.destTotalWidth = (int64_t) round(l.destTotalWidthDec);
  l.destTotalHeight = (int64_t) round(l.destTotalHeightDec);
  l.destTotalWidth2 = (int64_t) round(mBaseTotalWidth2 / l.magnifyX);
  l.destTotalHeight2 = (int64_t) round(mBaseTotalHeight2 / l.magnifyY);
  l.xScale=(double) l.srcTotalWidth / (double) l.destTotalWidthDec;
  l.yScale=(double) l.srcTotalHeight / (double) l.destTotalHeightDec;
  l.xScaleReverse=(double) l.destTotalWidthDec / (double) l.srcTotalWidth;
  l.yScaleReverse=(double) l.destTotalHeightDec / (double) l.srcTotalHeight;

  l.L2Size=l.readWidthL2 * l.readHeightL2 * 3;
  l.readOkL2=false;
  if (l.fillin && l.L2Size > 0 && l.pBitmapL2)
  {
    l.readOkL2=true;
    l.xScaleResize=(double) l.destTotalWidth / (double) l.srcTotalWidthL2;
    l.yScaleResize=(double) l.destTotalHeight / (double) l.srcTotalHeightL2;
    if (l.xScaleResize < 1.0 || l.yScaleResize < 1.0)
    {
      #ifndef USE_MAGICK
      l.scaleMethodL2=cv::INTER_AREA;
      #else
      l.scaleMethodL2=Magick::LanczosFilter;
      #endif
    }
  }
  l.xScaleL2=(double) l.srcTotalWidthL2 / (double) l.srcTotalWidth;
  l.yScaleL2=(double) l.srcTotalHeightL2 / (double) l.srcTotalHeight;
  if (tiled)
  {
    l.finalOutputWidth=256;
    l.finalOutputHeight=256;
    l.finalOutputWidth2=256;
    l.finalOutputHeight2=256;
    l.inputTileWidth=256;
    l.inputTileHeight=256;
    l.grabWidthA=round((double) l.inputTileWidth * l.xScale);
    l.grabHeightA=round((double) l.inputTileHeight * l.yScale);
    l.grabWidthB=round((double) l.finalOutputWidth * l.xScale);
    l.grabHeightB=round((double) l.finalOutputHeight * l.yScale);

    l.grabWidthL2=round(256.0 * (double) l.srcTotalWidthL2 / (double) l.destTotalWidth);
    l.grabHeightL2=round(256.0 * (double) l.srcTotalHeightL2 / (double) l.destTotalHeight);
  }
  else
  {
    l.finalOutputWidth=l.destTotalWidth;
    l.finalOutputHeight=l.destTotalHeight;
    l.finalOutputWidth2=l.destTotalWidth2;
    l.finalOutputHeight2=l.destTotalHeight2;
    l.inputTileWidth=l.destTotalWidth;
    l.inputTileHeight=l.destTotalHeight;
    l.grabWidthA=l.srcTotalWidth;
    l.grabHeightA=l.srcTotalHeight;
    l.grabWidthB=l.srcTotalWidth;
    l.grabHeightB=l.srcTotalHeight;
    l.grabWidthL2=l.srcTotalWidthL2;
    l.grabHeightL2=l.srcTotalHeightL2;
  }
  l.totalSubTiles = 1;
  int64_t totalGrabBytes = l.grabWidthB * l.grabHeightB * 3;
  if (totalGrabBytes > l.maxMemory && l.outputType != OLYVSLIDE_SCANONLY)
  {
    do
    {
      l.totalSubTiles *= 2;
      totalGrabBytes = ceil((double) l.grabWidthB / (double) l.totalSubTiles) * ceil((double) l.grabHeightB / (double) l.totalSubTiles) * 3;
    }
    while (totalGrabBytes > l.maxMemory);
    std::cout << "Using max memory " << (totalGrabBytes / (1024 * 1024)) << "mb for pixel resizer." << std::endl;
    l.grabWidthA = round(l.grabWidthB / (double) l.totalSubTiles);
    l.grabHeightA = round(l.grabHeightB / (double) l.totalSubTiles);
  } 

  if ((scanBkgd || l.readOkL2) && mBlendByRegion==false)
  {
    l.xBlendFactor = l.magnifyX; 
    l.yBlendFactor = l.magnifyY;
    l.xBkgdLimit = 752;
    l.yBkgdLimit = 480;
    if (scanBkgd)
    {
      l.inputTileWidth = 752;
      l.inputTileHeight = 480;
      l.grabWidthA = 752;
      l.grabHeightA = 480;
      l.grabWidthB = 752;
      l.grabHeightB = 480;
      l.finalOutputWidth = 752;
      l.finalOutputHeight = 480;
      l.finalOutputWidth2 = 752;
      l.finalOutputHeight2 = 480;
    }
  }
  else
  {
    l.xBkgdLimit = 0;
    l.yBkgdLimit = 0;
  }
  if (l.center && l.outputType == OLYVSLIDE_GOOGLE)
  {
    calcCenters(l.outLevel, l.xLevelOffset, l.yLevelOffset);
  
    l.xStartTile = l.xLevelOffset / 256;
    l.xCenter = l.xLevelOffset % 256;
    l.xStartSrc = (double)(-l.xCenter) * l.xScale;
    l.outputLvlTotalWidth = ceil((double) (l.xCenter + l.destTotalWidth) / 256.0) * 256;

    l.yTile = l.yLevelOffset / 256;
    l.yCenter = l.yLevelOffset % 256;
    l.yStartSrc = (double)(-l.yCenter) * l.yScale;
    l.outputLvlTotalHeight = ceil((double) (l.yCenter + l.destTotalHeight) / 256.0) * 256;
  }
  else
  {
    l.xStartTile = 0;
    l.xCenter = 0;
    l.xStartSrc = 0.0;
    
    l.yTile = 0;
    l.yCenter = 0;
    l.yStartSrc = 0.0;
    if (l.tiled)
    {
      l.outputLvlTotalWidth = ceil((double) l.destTotalWidth / 256.0) * 256;
      l.outputLvlTotalHeight = ceil((double) l.destTotalHeight / 256.0) * 256;
    }
    else
    {
      l.outputLvlTotalWidth = l.destTotalWidth2;
      l.outputLvlTotalHeight = l.destTotalHeight2;
    } 
  }
  if (l.xScaleReverse < 1.0 || l.yScaleReverse < 1.0)
  {
    #ifndef USE_MAGICK
    l.scaleMethod=cv::INTER_AREA;
    #else
    l.scaleMethod=Magick::LanczosFilter;
    #endif
  }
  // Get the quality from the composite level (this does work as long as
  // the ini file specifies it (some ini files do, some don't)
  // Make sure the quality is at minimum the quality specific on the
  // command line
  l.quality=slide->getQuality(l.olympusLevel);
  if (l.quality == 0 || l.quality < mQuality)
  {
    l.quality = mQuality;
  }
  if (l.outputType == OLYVSLIDE_GOOGLE)
  {
    output << "Google Maps Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (mCreateLog) *logFile << output.str();
    std::cout << output.str();
  }
  else if (l.outputType == OLYVSLIDE_TIF)
  {
    output << "Tiff Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (mCreateLog) *logFile << output.str();
    std::cout << output.str();
   
    int totalMag=slide->getMagnification();
    if (totalMag<=0)
    {
      totalMag=40;
    }
    std::ostringstream oss;
    if (mBaseLevel==olympusLevel || tiled==false)
    {
      oss << "|AppMag=" << totalMag;
      if (slide->getTotalZLevels() > 1 && mZSteps == 1) 
      {
        oss << "|TotalDepth = " << slide->getTotalZLevels() << "\0";
      }
      else if (slide->getTotalZLevels() > 1 && mZSteps > 1)
      {
        oss << "|OffsetZ = " << (mZSteps-1) << "\0";
      }
    }
    std::string strAttributes=oss.str();
    if (mTif->setAttributes(3, 8, l.destTotalWidth2, l.destTotalHeight2, (l.tiled==true ? l.finalOutputWidth : 0), (l.tiled==true ? l.finalOutputHeight : 0), 1, l.quality)==false || mTif->setDescription(strAttributes, mBaseTotalWidth2, mBaseTotalHeight2)==false)
    {
      std::string errMsg;
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to write tif attributes: " << errMsg << std::endl; 
      return 4;
    }
  }
  else if (l.outputType == OLYVSLIDE_SCANONLY)
  {
    output << "Scanning Olympus Level=" << l.olympusLevel << " for Background Blending Information" << std::endl;
    
    if (mCreateLog) *logFile << output.str();
    std::cout << output.str();
  }

  bool error=false;
  time_t timeStart=0, timeLast=0;
  #ifdef USE_MAGICK
  l.magickWand = Magick::NewMagickWand();
  Magick::MagickSetCompression(l.magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(l.magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(l.magickWand, 8);
  Magick::MagickSetImageAlphaChannel(l.magickWand, Magick::OffAlphaChannel);
  l.pixelWand = Magick::NewPixelWand();
  Magick::PixelSetColor(l.pixelWand, "#ffffff");
  #endif
  try
  {
    safeBmpClear(&l.bitmap1);
    if (l.totalSubTiles > 1)
    {
      safeBmpAlloc2(&l.bitmap1, l.inputTileWidth, l.inputTileHeight);
    }
    if ((l.readOkL2 || scanBkgd) && mBlendByRegion==false)
    {
      l.pBitmap4 = safeBmpAlloc(l.finalOutputWidth2, l.finalOutputHeight2);
      if (mTotalXSections == 0 || mTotalYSections == 0)
      {
        mTotalXSections = l.outputLvlTotalWidth;
        mTotalYSections = l.outputLvlTotalHeight;
        mxSubSections=new BlendSection*[mTotalXSections];
        mySubSections=new BlendSection*[mTotalYSections];
        memset(mxSubSections, 0, mTotalXSections*sizeof(BlendSection*));
        memset(mySubSections, 0, mTotalYSections*sizeof(BlendSection*));
      }
      l.totalXSections = mTotalXSections; 
      l.totalYSections = mTotalYSections;
      l.xSubSections=mxSubSections;
      l.ySubSections=mySubSections;
    }
    if (mCreateLog)
    {
      *logFile << " xScale=" << l.xScale << " yScale=" << l.yScale;
      *logFile << " srcTotalWidth=" << l.srcTotalWidth << " srcTotalHeight=" << l.srcTotalHeight;
      *logFile << " destTotalWidth=" << l.destTotalWidth << " destTotalHeight=" << l.destTotalHeight;
      *logFile << std::endl;
    }
    
    l.perc=0, l.percOld=0;
    l.onePercHit=false;
    bool error=false;
    timeStart = time(NULL);
    l.ySrc=l.yStartSrc;
    
    retractCursor();
    std::cout << "0% done...    " << std::flush;

    // Keep looping until the current composite pyramid level is read
    // or the resulting output pyramid is done or on error
    while (round(l.ySrc) < l.srcTotalHeight && l.yDest < l.outputLvlTotalHeight && error==false)
    {
      std::ostringstream yRootStream, yRootStreamZip;
      std::string dirPart1, dirPartZip1;
      std::string dirPart2, dirPartZip2;
      std::string yRoot, yRootZip;
      
      yRootStream << mFileNameOnly;
      dirPart1 = yRootStream.str();
      yRootStream << mPathSeparator << outLevel;
      dirPart2 = yRootStream.str();
      yRootStream << mPathSeparator << l.yTile;
      yRoot = yRootStream.str();
      if (l.debugLevel > 1 && l.outputType != OLYVSLIDE_SCANONLY)
      {
        // Create the google maps directory structure up to the the y tile
        if (!my_mkdir(dirPart1) || !my_mkdir(dirPart2) || !my_mkdir(yRoot))
        {
          std::cerr << "Failed to add create directory '" << yRoot << "'. Stopping!" << std::endl;
          error = true;
          break;
        }
      }
      yRootStreamZip << mFileNameOnly;
      dirPartZip1 = yRootStreamZip.str();
      yRootStreamZip << ZipFile::mZipPathSeparator << outLevel;
      dirPartZip2 = yRootStreamZip.str();
      yRootStreamZip << ZipFile::mZipPathSeparator << l.yTile;
      yRootZip = yRootStreamZip.str();
      if (l.outputType == OLYVSLIDE_GOOGLE) 
      {
        // Create the google maps directory structure up to the the y tile
        if (mZip->addDir(dirPartZip1)==-1 || mZip->addDir(dirPartZip2)==-1 || mZip->addDir(yRootZip)==-1)
        {
          std::cerr << "Failed to add zip directory '" << yRoot << "' to archive. Reason: " << mZip->getErrorMsg() << std::endl << "Stopping!" << std::endl;
          error = true;
          break;
        }
      }
      l.xDest = 0;
      l.xTile = l.xStartTile;
      // Loop until this cx block is complete or on error
      for (l.xSrc=l.xStartSrc; round(l.xSrc) < l.srcTotalWidth && l.xDest<l.outputLvlTotalWidth && error==false; l.xSrc += l.grabWidthB) 
      {
        std::ostringstream tileNameStream, tileNameStreamZip;
        std::string errMsg;
        if (l.xSrc + l.grabWidthB < 1.0 || l.ySrc + l.grabHeightB < 1.0) continue;
        tileNameStream << yRoot << mPathSeparator << l.xTile << ".jpg";
        tileNameStreamZip << yRootZip << ZipFile::mZipPathSeparator << l.xTile << ".jpg";
        l.tileName = tileNameStream.str();

        l.inputSubTileWidthRead = (int64_t) round((double) l.inputTileWidth / (double) l.totalSubTiles);
        l.inputSubTileHeightRead = (int64_t) round((double) l.inputTileHeight / (double) l.totalSubTiles);
        bool readOkSrc=false;
        int readSubTiles=0;
        if (l.totalSubTiles > 1)
        {
          safeBmpByteSet(&l.bitmap1, l.bkgdColor);
        }
        l.pBitmapSrc = NULL;
        double xSrcStart2=l.xSrc;
        double ySrcStart2=l.ySrc;

        safeBmpClear(&l.subTileBitmap);
        safeBmpClear(&l.safeImgScaled);
        safeBmpClear(&l.sizedBitmap);
        safeBmpClear(&l.sizedBitmap2);
        l.pImgScaled = NULL;
        // this tile needs to be the same with and length as final output width and length
        for (l.ySubTile=0; l.ySubTile < l.totalSubTiles && l.ySrc < l.srcTotalHeight; l.ySubTile++)
        {
          l.xSrc=xSrcStart2;
          for (l.xSubTile=0; l.xSubTile < l.totalSubTiles && l.xSrc < l.srcTotalWidth; l.xSubTile++)
          {
            if (l.xSrc + l.grabWidthA < 1.0 || l.ySrc + l.grabHeightA < 1.0)
            {
              l.xSrc += l.grabWidthA;
              continue;
            }
            safeBmpClear(&l.subTileBitmap);
            safeBmpClear(&l.safeImgScaled);
            safeBmpClear(&l.sizedBitmap);
            safeBmpClear(&l.sizedBitmap2);
            l.pImgScaled = NULL;
            
            l.xSrcRead = l.xSrc;
            l.ySrcRead = l.ySrc;
            double grabWidthReadDec = l.grabWidthA;
            double grabHeightReadDec = l.grabHeightA;
            l.inputSubTileWidthRead = (int64_t) round((double) l.inputTileWidth / (double) l.totalSubTiles);
            l.inputSubTileHeightRead = (int64_t) round((double) l.inputTileHeight / (double) l.totalSubTiles);
            l.xMargin = 0;
            l.yMargin = 0;
            if (l.xSrc < 0.0)
            {
              grabWidthReadDec=l.grabWidthA + l.xSrc;
              l.xSrcRead = 0.0;
              l.xMargin = l.xCenter - (l.xSubTile * l.inputSubTileWidthRead);
            }
            if (l.ySrc < 0.0)
            {
              grabHeightReadDec=l.grabHeightA + l.ySrc;
              l.ySrcRead = 0.0;
              l.yMargin = l.yCenter - (l.ySubTile * l.inputSubTileHeightRead);
            }
            if (l.xSrcRead + grabWidthReadDec > (double) l.srcTotalWidth)
            {
              grabWidthReadDec = (double) l.srcTotalWidth - l.xSrcRead;
              l.inputSubTileWidthRead = round(grabWidthReadDec * l.xScaleReverse);
            }
            if (l.ySrcRead + grabHeightReadDec > (double) l.srcTotalHeight)
            {
              grabHeightReadDec = (double) l.srcTotalHeight - l.ySrcRead;
              l.inputSubTileHeightRead = round(grabHeightReadDec * l.yScaleReverse);
            }
            if (l.xSrc < 0.0 || l.ySrc < 0.0)
            {
              l.inputSubTileWidthRead = round(grabWidthReadDec * l.xScaleReverse);
              l.inputSubTileHeightRead = round(grabHeightReadDec * l.yScaleReverse);
            }
            l.grabWidthRead = round(grabWidthReadDec);
            l.grabHeightRead = round(grabHeightReadDec);
            slide->allocate(&l.subTileBitmap, olympusLevel, round(l.xSrcRead), round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false);
            l.pBitmapSrc = &l.subTileBitmap;
            safeBmpByteSet(&l.subTileBitmap, l.bkgdColor);
            
            l.readWidth=0;
            l.readHeight=0;
            if (l.pBitmapSrc->data == NULL)
            {
              error = true;
              break;
            }
            if (l.debugLevel > 2)
            {
              *logFile << " slide->read(x=" << l.xSrcRead << " y=" << l.ySrcRead << " grabWidthA=" << l.grabWidthRead << " grabHeightA=" << l.grabHeightRead << " olympusLevel=" << l.olympusLevel << "); " << std::endl;
            }
            readOkSrc = slide->read(l.pBitmapSrc->data, l.olympusLevel, l.readDirection, l.readZLevel, round(l.xSrcRead), round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false, &l.readWidth, &l.readHeight);
            if (readOkSrc)
            {
              readSubTiles++;
            }
            else
            {
              std::cerr << "Failed to read olympus level " << l.olympusLevel << " jpeg tile @ x=" << l.xSrcRead << " y=" << l.ySrcRead << " width=" << l.grabWidthRead << " height=" << l.grabHeightRead << std::endl;
            }
            if (l.debugLevel > 2)
            {
              std::cout << "readWidth: " << l.readWidth << " readHeight: " << l.readHeight<< " grabWidth: " << l.grabWidthRead << " grabHeight: " << l.grabHeightRead << std::endl;
            }
            if (readOkSrc && l.outputType == OLYVSLIDE_SCANONLY)
            {
              scanSrcTileBkgd(l);
            }
            else if (readOkSrc)
            {
              processSrcTile(l);
            }
            safeBmpFree(&l.safeImgScaled);
            safeBmpFree(&l.sizedBitmap);
            if (l.pImgScaled && l.totalSubTiles > 1)
            {
              #ifndef USE_MAGICK
              l.pImgScaled->release();
              delete l.pImgScaled;
              #endif
              l.pImgScaled = 0;
              safeBmpFree(&l.subTileBitmap);
            }
            l.xSrc += l.grabWidthA;
            printPercDone(l);
          }
          l.ySrc += l.grabHeightA;
        }
        l.xSrc = xSrcStart2;
        l.ySrc = ySrcStart2;
        l.xSrcRead = l.xSrc;
        l.ySrcRead = l.ySrc;
        safeBmpClear(&l.safeScaledL2Mini);
        safeBmpClear(&l.safeScaledL2Mini2);
        l.pImgScaledL2Mini = NULL;
        if (readSubTiles > 0 && l.outputType != OLYVSLIDE_SCANONLY) 
        {
          bool writeOk=false;
          if (l.readOkL2)
          {
            blendL2WithSrc(l);  
          }
          if (l.outputType == OLYVSLIDE_GOOGLE)
          {
            BYTE* pJpegBytes = NULL;
            unsigned long outSize = 0;
            std::string tileName = tileNameStreamZip.str();
            bool compressOk=my_jpeg_compress(&pJpegBytes, l.pBitmapFinal->data, l.pBitmapFinal->width, l.pBitmapFinal->height, l.quality, &errMsg, &outSize);
            if (compressOk && mZip->addFile(tileName, pJpegBytes, outSize)==ZIP_OK)
            {
              writeOk=true;
            }
            else
            {
              error=true;
            }
            my_jpeg_free(&pJpegBytes);
          }
          else if (l.outputType == OLYVSLIDE_TIF)
          {
            if (l.tiled)
            {
              writeOk=mTif->writeEncodedTile(l.pBitmapFinal->data, l.xDest, l.yDest, 1);
            }
            else
            {
              writeOk=mTif->writeImage(l.pBitmapFinal->data);
            }
          }
          if (writeOk==false)
          {
            std::string errMsg;
            if (l.outputType == OLYVSLIDE_GOOGLE)
            {
              std::cerr << "Failed to write jpeg tile '" << l.tileName << "' reason: " << mZip->getErrorMsg() << std::endl;
            }
            else if (l.outputType == OLYVSLIDE_TIF && tiled)
            {
              mTif->getErrMsg(errMsg);
              std::cerr << "Failed to write tif tile x=" << l.xDest << " y=" << l.yDest << " reason: " << errMsg << std::endl;
            }
            else if (l.outputType == OLYVSLIDE_TIF && !tiled)
            {
              mTif->getErrMsg(errMsg);
              std::cerr << "Failed to write tif image at tif level=" << l.outLevel << " reason: " << errMsg << std::endl;
            }
            error = true;
          }
        }
        tileCleanup(l);
        l.xDest += l.finalOutputWidth2;
        l.xTile++;
      }
      l.yDest += l.finalOutputHeight2;
      l.ySrc += l.grabHeightB;
      l.yTile++;
      printPercDone(l);
    }
    if (l.outputType == OLYVSLIDE_TIF)
    {
      bool success = mTif->writeDirectory();
      if (success == false)
      {
        const char *tifDirErrorMsg = "Fatal Error: Failed to write tif directory: ";
        std::string errMsg;
        mTif->getErrMsg(errMsg);
        std::cerr << tifDirErrorMsg << errMsg << std::endl;
        if (mCreateLog) *logFile << tifDirErrorMsg << errMsg << std::endl;
        error = true;
      }
    }
  }
  catch (std::bad_alloc& ba)
  {
    const char *msg = "Fatal Error: Failed to get memory. Cannot continue!";
    std::cout << msg << std::endl;
    if (mCreateLog) *logFile << msg << std::endl;
    error = true;
    exit(1);
  }
  safeBmpFree(l.pBitmap4);
  safeBmpFree(&l.bitmap1);
  #ifdef USE_MAGICK
  if (l.magickWand) 
  {
    Magick::DestroyMagickWand(l.magickWand);
    l.magickWand = NULL;
  }
  if (l.pixelWand)
  {
    Magick::DestroyPixelWand(l.pixelWand);
    l.pixelWand = NULL;
  }
  #endif
  l.pBitmap4 = NULL;
  timeLast = time(NULL);
  if (error==false)
  {
    std::cout << "Took " << timeLast - timeStart << " seconds for this level." << std::endl;
  }
  return (error==true ? 1 : 0); 
}


int SlideConvertor::checkFullL2(int64_t *pReadWidthL2, int64_t *pReadHeightL2, safeBmp **pFullL2)
{
  *pFullL2 = NULL;
  *pReadWidthL2=0;
  *pReadHeightL2=0;
  if (!slide->checkLevel(2)) return 1;

  int64_t srcTotalWidthL2 = slide->getActualWidth(2);
  int64_t srcTotalHeightL2 = slide->getActualHeight(2);
  if (srcTotalWidthL2 <= 0 || srcTotalHeightL2 <= 0) return 1;
  if (!mpImageL2) return 2;
  if (mpImageL2->width > 0 && mpImageL2->height > 0 && mpImageL2->data) 
  {
    *pReadWidthL2 = mpImageL2->width;
    *pReadHeightL2 = mpImageL2->height;
    *pFullL2 = mpImageL2;
    return 0;
  }
  return 1;
}


int SlideConvertor::convert2Tif()
{
  int error = 0;
  safeBmp* pFullL2Bitmap = 0; 
  int64_t readWidthL2 = 0;
  int64_t readHeightL2 = 0;
 
  if (mValidObject == false) return 1;
 
  int status = checkFullL2(&readWidthL2, &readHeightL2, &pFullL2Bitmap);
  switch (status)
  {
    case 0:
      break;
    case 1:
      std::cout << "Slide missing level 2 pyramid, continuing without one." << std::endl;
      mBlendTopLevel = false;
      break;
    case 2:
      std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
      return 1;
    case 3:
      std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
      mBlendTopLevel = false;
      break;
  }
  int maxDivisor=128;
  while (maxDivisor > 16 && (mBaseTotalWidth / maxDivisor < 2000 && mBaseTotalHeight / maxDivisor < 2000)) 
  {
    maxDivisor /= 2;
  }; 
  //****************************************************************
  // Output the base level, thumbnail, 4x, 16x, and 32x levels 
  //****************************************************************
  int divisor = 1;
  int step = 1;
  
  error=outputLevel(mBaseLevel, divisor, step, true, true, false, readWidthL2, readHeightL2, pFullL2Bitmap);

  while (divisor != maxDivisor && error==0)
  {
    bool tiled = true;
    int olympusLevel = 1;
    switch (step)
    {
      case 1:
        divisor = 1;
        olympusLevel = 0; 
        break;
      case 2:
        divisor = maxDivisor * 2;
        tiled = false;
        break;
      case 3:
        divisor = 4;
        break;
      case 4:
        divisor = 16;
        break;
      case 5:
        if (maxDivisor / 2 >= 64)
        {
          divisor = maxDivisor / 2;
        }
        else
        {
          divisor = maxDivisor;
        }
        break;
      case 6:
        divisor = maxDivisor;
        break;
    }
    error=outputLevel(olympusLevel, divisor, step, tiled, false, false, readWidthL2, readHeightL2, pFullL2Bitmap);
    step++;
  }
  if (error == 0 && step > 1)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  return error;
}



int SlideConvertor::convert2Gmap()
{
  int error = 0;
  safeBmp* pFullL2Bitmap = 0; 
  int64_t readWidthL2 = 0;
  int64_t readHeightL2 = 0;
  
  if (mValidObject == false) return 4;

  int status = checkFullL2(&readWidthL2, &readHeightL2, &pFullL2Bitmap);
  switch (status)
  {
    case 0:
      break;
    case 1:
      std::cout << "Slide missing level 2 pyramid, continuing without one." << std::endl;
      mBlendTopLevel = false;
      break;
    case 2:
      std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
      return 1;
    case 3:
      std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
      mBlendTopLevel = false;
      break;
  }
  //****************************************************************
  // Output each level, each level is 2^level size 
  //****************************************************************
  int64_t divisor = 1 << mTopOutLevel;
  int outLevel = 0;

  error=outputLevel(mBaseLevel, 1, 0, true, true, false, readWidthL2, readHeightL2, pFullL2Bitmap);
  while (outLevel <= mTopOutLevel && error==0)
  {
    int olympusLevel;
    if (divisor < 4)
    {
      olympusLevel = 0;
    }
    else
    {
      olympusLevel = 1;
    }
    bool flushArchive = (outLevel == mTopOutLevel ? true : false);
    error=outputLevel(olympusLevel, divisor, outLevel, true, false, flushArchive, readWidthL2, readHeightL2, pFullL2Bitmap);
    divisor /= 2;
    outLevel++;
  }
  if (error==0 && outLevel > mTopOutLevel && outLevel > 0)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  return error;
}


int SlideConvertor::open(std::string inputFile, std::string outputFile, bool useOpenCV, bool blendTopLevel, bool blendByRegion, bool markOutline, bool includeZStack, bool createLog, int quality, int64_t bestXOffset, int64_t bestYOffset, int maxMemory, int outputType, int debugLevel)
{
  mValidObject = false;
  mDebugLevel = debugLevel;
  closeRelated();
  logFile = new std::ofstream;
  mCreateLog = createLog;
  if (mCreateLog)
  {
    logFile->open("olyvslideconv.log");
  }
  slide = new CompositeSlide();
  errMsg="";
  if (slide->open(inputFile.c_str(), useOpenCV, markOutline, debugLevel, bestXOffset, bestYOffset, &mpImageL2)==false)
  {
    return 1;
  }
  mMaxMemory = maxMemory;
  mOutputFile = outputFile;
  mOutputDir = outputFile;
  mOutputType = outputType;
  std::size_t lastSlashIndex = mOutputFile.find_last_of("\\/");
  if (lastSlashIndex != std::string::npos)
  {
    if (lastSlashIndex+1 < mOutputFile.length())
    {
      mFileNameOnly = mOutputFile.substr(lastSlashIndex+1);
    }
    else
    {
      std::cerr << "Error: provided output '" << mOutputFile << "' must include destination zip file name." << std::endl;
      return 2;
    }
  }
  else
  {
    mFileNameOnly = mOutputFile;
  }
  std::size_t dot_index = mFileNameOnly.find_last_of(".");
  if (dot_index != std::string::npos)
  {
    mFileNameOnly.erase(dot_index);
  }

  if (mOutputType == OLYVSLIDE_TIF)
  {
    mOutputDir.append("_debug_jpgs");
    mCenter = false;
    mTif = new Tiff();
    if (mTif->createFile(outputFile)==false)
    {
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to create tiff file '" << outputFile << "'. Reason: " << errMsg << std::endl;
      return 2;
    }
  }
  else if (mOutputType == OLYVSLIDE_GOOGLE)
  {
    mZip = new ZipFile();
    if (mZip->openArchive(outputFile.c_str(), mMaxZipBufferBytes, APPEND_STATUS_CREATE) != 0)
    {
      std::cerr << "Failed to create zip file '" << outputFile << "'. Reason: " << mZip->getErrorMsg() << std::endl;
      return 2;
    }
    mZip->setCompression(MZ_COMPRESS_METHOD_STORE, MZ_COMPRESS_LEVEL_DEFAULT);
    mCenter = true;
  }
  else
  {
    return 1;
  }
  for (mBaseLevel=0; mBaseLevel<4; mBaseLevel++)
  {
    if (slide->checkLevel(mBaseLevel))
    {
      mBaseTotalWidth = slide->getActualWidth(mBaseLevel);
      mBaseTotalWidth2 = ceil((double) mBaseTotalWidth / (double) 256.0) * 256;
      mBaseTotalHeight = slide->getActualHeight(mBaseLevel);
      mBaseTotalHeight2 = ceil((double) mBaseTotalHeight / (double) 256.0) * 256;
      break;
    }
  }
  mStep=0;
  mLastZLevel=-1;
  mBlendTopLevel = blendTopLevel;
  mBlendByRegion = blendByRegion;
  mIncludeZStack = includeZStack;
  mQuality = quality;
  mMaxSide = 0;
  mTopOutLevel = 0;
  
  std::cout << "baseTotalWidth=" << mBaseTotalWidth << " baseTotalHeight=" << mBaseTotalHeight << std::endl;
  if (mBaseTotalWidth > 0 && mBaseTotalHeight > 0)
  {
    mValidObject = true;
    if (mOutputType == OLYVSLIDE_GOOGLE)
    {
      for (mTopOutLevel = 0; mTopOutLevel < 20; mTopOutLevel++)
      {
        mMaxSide = (1 << mTopOutLevel) * 256;
        if (mMaxSide >= mBaseTotalWidth && mMaxSide >= mBaseTotalHeight)
        {
          std::cout << "Total Google Maps Levels: " << mTopOutLevel << " (because 2^" << mTopOutLevel << "*256=" << mMaxSide << ") " << std::endl;
          break;
        }
      }
    }
  }
  return (mValidObject == true) ? 0 : 3;
}


void SlideConvertor::closeRelated()
{
  if (mpImageL2)
  {
    safeBmpFree(mpImageL2);
    mpImageL2 = NULL;
  }
  if (mTif)
  {
    mTif->close();
    delete mTif;
    mTif = NULL;
  }
  if (mZip)
  {
    mZip->closeArchive();
    delete mZip;
    mZip = NULL;
  }
  if (logFile)
  {
    logFile->close();
    delete logFile;
    logFile = NULL;
  }
  if (slide)
  {
    slide->close();
    delete slide;
    slide = NULL;
  }
  if (mxSubSections && mySubSections)
  {
    blendLevelsFree(mxSubSections, mTotalXSections, mySubSections, mTotalYSections);
  }
  if (mxSubSections)
  {
    delete[] mxSubSections;
    mxSubSections = NULL;
    mTotalXSections = 0;
  }
  if (mySubSections)
  {
    delete[] mySubSections;
    mySubSections = NULL;
    mTotalYSections = 0;
  }
  mStep=0;
  mZSteps=0;
  mLastDirection=-1;
  mLastZLevel=-1;
  mValidObject=false;
  mBaseTotalWidth=0;
  mBaseTotalHeight=0;
}


bool getBoolOpt(const char *optarg, bool& invalidOpt)
{
  const char *available[14] = { 
    "1", 
    "true", "TRUE", 
    "enable", "ENABLE", 
    "on", "ON", 
    "0", 
    "false", "FALSE", 
    "disable", "DISABLE", 
    "off", "OFF" 
  };
  if (optarg == NULL) 
  {
    invalidOpt = true;
    return false;
  }
  std::string optarg2 = optarg;
  for (int i = 0; i < 14; i++)
  {
    if (optarg2.find(available[i]) != std::string::npos)
    {
      if (i < 7) return true;
      else return false;
    }
  }
  invalidOpt = true;
  return false;
}


int getIntOpt(const char *optarg, bool& invalidOpt)
{
  unsigned int i=0;
  if (optarg==NULL)
  {
    invalidOpt = true;
    return 0;
  }
  std::string optarg2 = optarg;
  while (i < optarg2.length() && (optarg2[i]==' ' || optarg2[i]==':' || optarg2[i]=='=' || optarg2[i]=='\t')) i++;
  optarg2 = optarg2.substr(i);
  if (optarg2.length() > 0 && isdigit(optarg2[0]))
  {
    return atoi(optarg2.c_str());
  }
  invalidOpt = true;
  return 0;
}


int main(int argc, char** argv)
{
  SlideConvertor slideConv;
  int error=0;
  std::string infile, outfile;
  int64_t bestXOffset = -1, bestYOffset = -1;
  bool xOffsetSet = false, yOffsetSet = false;
  bool useOpenCV = false;
  bool blendTopLevel = true;
  bool blendByRegion = false;
  bool doBorderHighlight = false;
  bool includeZStack = false;
  bool createLog = false;
  static int outputType = 0;
  int quality = 85;
  int debugLevel = 0;
  int maxResizeMemory = MAX_RESIZE_MEMORY;
  int maxJpgCacheMemory = MAX_JPG_CACHE_MEMORY;
  char syntax[] = "syntax: olyvslideconv -b=[on,off] -l=[on,off] -o=[on,off] -d=[0-3] -h=[on,off] -r=[on,off] -x=[bestXOffset] -y=[bestYOffset] -o=[on,off] -z=[on,off] [--tiff, --google] <inputfolder> <outputfile> \nFlags:\t--tiff output to tif file.\n\t--google output to google maps format.\n\t-b blend the top level with the middle level. Default on.\n\t-l log general information about the slide.\n\t-d Debug mode, output debugging info and files. Default 0. The higher the more debugging output. Sets logging on as well if greater than 1.\n\t-m Specify max memory size of pixel rescaler/resizer in megabytes, default 512mb.\n\t-j Specify max jpg cache size in megabytes, default 256mb.\n\t-r Blend the top level with the middle level by region, not by empty background, default off.\n\t-h highlight visible areas with a black border. Default off.\n\t-q Set minimal jpeg quality percentage. Default 85 for level 0, 90 for level 1, and 95 for level 2 and above.\n\t-o Use opencv (computer vision) to calculate the offset for the upper and higher level (almost never required).\n\t-x and -y Optional: set best X, Y offset of image if upper and lower pyramid levels are not aligned.\n\t-z Process Z-stack if the image has one. Default off.\n";

  if (argc < 3)
  {
    std::cerr << syntax;
    return 1;
  }
  int opt;
  int optIndex = 0;
  bool invalidOpt = false;
  char emptyString[] = "";
  static struct option longOptions[] =
    {
      {"google",            no_argument,        &outputType,  OLYVSLIDE_GOOGLE},
      {"tiff",              no_argument,        &outputType,  OLYVSLIDE_TIF},
      {"blend",             required_argument,  0,             'b'},
      {"debug",             required_argument,  0,             'd'},
      {"log",               required_argument,  0,             'l'},
      {"highlight",         required_argument,  0,             'h'},
      {"opencv",            required_argument,  0,             'o'},
      {"region",            required_argument,  0,             'r'},
      {"quality",           required_argument,  0,             'q'},
      {"xoffset",           required_argument,  0,             'x'},
      {"yoffset",           required_argument,  0,             'y'},
      {"zstack",            required_argument,  0,             'z'},
      {"max-resize-memory", required_argument,  0,             'm'},   
      {"max-jpg-cache",     required_argument,  0,             'j'},
      {0,                   0,                  0,             0 }
    };
  
  while((opt = getopt_long(argc, argv, "b:d:h:r:q:x:y:z:", longOptions, &optIndex)) != -1)
  {
    if (optarg == NULL) optarg = emptyString;
    switch (opt)
    {
      case 'b':
        blendTopLevel = getBoolOpt(optarg, invalidOpt);
        break;
      case 'd':
        debugLevel = getIntOpt(optarg, invalidOpt);
        break;
      case 'h':
        doBorderHighlight = getBoolOpt(optarg, invalidOpt);
        break;
      case 'l':
        createLog = getBoolOpt(optarg, invalidOpt);
        break;
      case 'r':
        blendByRegion = getBoolOpt(optarg, invalidOpt);
        break;
      case 'q':
        quality = getIntOpt(optarg, invalidOpt);
        break;
      case 'o':
        #ifndef USE_MAGICK
        useOpenCV = getBoolOpt(optarg, invalidOpt);
        #endif
        break;
      case 'm':
        maxResizeMemory = getIntOpt(optarg, invalidOpt);
        break;
      case 'j':
        maxJpgCacheMemory = getIntOpt(optarg, invalidOpt);
        break;
      case 'x':
        bestXOffset = getIntOpt(optarg, invalidOpt);
        xOffsetSet = true;
        break;
      case 'y':
        bestYOffset = getIntOpt(optarg, invalidOpt);
        yOffsetSet = true;
        break;
      case 'z':
        includeZStack = getBoolOpt(optarg, invalidOpt);
        break;
      case '?':
        if (infile.length() == 0)
        {
          infile=optarg;
        }
        else if (outfile.length() == 0)
        {
          outfile=optarg;
        }
        else
        {
          invalidOpt = true;
        }
        break;
    }
    if (invalidOpt)
    {
      std::cerr << syntax;
      return 1;
    } 
  }
  for (; optind < argc; optind++)
  {
    if (infile.length() == 0)
    {
      infile=argv[optind];
    }
    else if (outfile.length() == 0)
    {
      outfile=argv[optind];
    }
  }
  if (debugLevel > 1) createLog = true;
  
  if (infile.length() == 0 || outfile.length() == 0)
  {
    std::cerr << syntax;
    return 1;
  }
  if (outputType==0)
  {
    std::cerr << "No output format specified. Please use --tiff or --google to specify tiff file or google maps zip output." << std::endl;
    std::cerr << syntax;
    return 1;
  }
  if (maxResizeMemory < 32)
  {
    std::cerr << "Max resize memory must be a size greater than or equal to 32mb." << std::endl;
    return 1;
  }
  if (maxJpgCacheMemory == 0)
  {
    maxJpgCacheMemory = 1;
  }

  if (debugLevel > 0)
  {
    if (outputType==OLYVSLIDE_GOOGLE)
    {
      std::cout << "Output format: Zipped Google Maps" << std::endl;
    }
    else if (outputType==OLYVSLIDE_TIF)
    {
      std::cout << "Output format: TIFF/SVS" << std::endl;
    }
    std::cout << "Set logging: " << bool2txt(createLog) << std::endl;
    std::cout << "Set debug level: " << debugLevel << std::endl;
    std::cout << "Set minimum quality: " << quality << std::endl;
    std::cout << "Use OpenCV/computer vision: " << bool2txt(useOpenCV) << std::endl;
    std::cout << "Set blend top level: " << bool2txt(blendTopLevel) << std::endl;
    std::cout << "Set border highlight: " << bool2txt(doBorderHighlight) << std::endl;
    std::cout << "Set blend by region: " << bool2txt(blendByRegion) << std::endl;
    if (xOffsetSet) 
    {
      std::cout << "Set bestXOffset: " << bestXOffset << std::endl;
    }
    else
    {
      std::cout << "Set bestXOffset: default" << std::endl;
    }
    if (yOffsetSet)
    {
      std::cout << "Set bestYOffset: " << bestYOffset << std::endl;
    }
    else
    {
      std::cout << "Set bestYOffset: default" << std::endl;
    }
    std::cout << "Set maximum resize memory: " << maxResizeMemory << "mb " << std::endl;
    std::cout << "Set maximum jpg cache memory: " << maxJpgCacheMemory << "mb " << std::endl;
  }

  maxResizeMemory *= 1024 * 1024;
  jpgCache.setMaxOpen(maxJpgCacheMemory);

  #ifdef USE_MAGICK
  #if defined(__MINGW32__) || defined(__MINGW64__)
  quickEnv("MAGICK_CODER_MODULE_PATH", getMagickCoreCoderPath(), debugLevel);
  quickEnv("MAGICK_CODER_FILTER_PATH", getMagickCoreFilterPath(), debugLevel);
  #endif
  quickEnv("MAGICK_MAP_LIMIT", MAGICK_MAP_LIMIT, debugLevel);
  quickEnv("MAGICK_MEMORY_LIMIT", MAGICK_MEMORY_LIMIT, debugLevel);
  quickEnv("MAGICK_DISK_LIMIT", MAGICK_DISK_LIMIT, debugLevel);
  quickEnv("MAGICK_AREA_LIMIT", MAGICK_AREA_LIMIT, debugLevel);
  Magick::MagickWandGenesis();
  #endif
 
  error=slideConv.open(infile.c_str(), outfile.c_str(), useOpenCV, blendTopLevel, blendByRegion, doBorderHighlight, includeZStack, createLog, quality, bestXOffset, bestYOffset, maxResizeMemory, outputType, debugLevel);
  if (error==0)
  {
    if (outputType == OLYVSLIDE_TIF)
    {
      error = slideConv.convert2Tif();
    }
    else if (outputType == OLYVSLIDE_GOOGLE)
    {
      error = slideConv.convert2Gmap();
    }
  }
  else if (error>0) 
  {
    if (error==1)
    {
      std::cerr << "Failed to open " << infile << std::endl;
    }
    else if (error==2)
    {
      // failed to create tiff file, error already outputted
    }
    else if (error==3)
    {
      std::cerr << "No valid levels found." << std::endl;
    }
    error++;
  }
  slideConv.closeRelated();
  
  #ifdef USE_MAGICK
  Magick::MagickWandTerminus();
  #endif
  return error;
}

