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

#if defined(_WIN32) || defined(_WIN64)
#include "console-mswin.h"
#include "getopt-mswin.h"
#else
#include "console-unix.h"
#include <unistd.h>
#include <getopt.h>
#endif
#include "olyvslideconv.h"

std::string bool2Txt(bool cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
}

std::string boolInt2Txt(int cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
}


void quickEnv(const char* var, const char* value, int optDebug)
{
  char full_char[512];
  std::string full=var;
  full.append("=");
  full.append(value);
  const char * full_const = full.c_str();
  full_char[0] = 0;
  strncpy(full_char, full_const, sizeof(full_char)-1);
  if (optDebug > 0)
  {
    std::cout << "ENV: " << full_char << std::endl;
  }
  putenv(full_char);
}


int getBoolOpt(const char *optarg)
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
    return 1;
  }
  std::string optarg2 = optarg;
  for (int i = 0; i < 14; i++)
  {
    if (optarg2.find(available[i]) != std::string::npos)
    {
      if (i < 7) return 1;
      else return 0;
    }
  }
  return 1;
}


int getBoolOpt(const char *optarg, bool& invalidOpt)
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
    return 1;
  }
  std::string optarg2 = optarg;
  for (int i = 0; i < 14; i++)
  {
    if (optarg2.find(available[i]) != std::string::npos)
    {
      if (i < 7) return 1;
      else return 0;
    }
  }
  return 1;
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


double getDoubleOpt(const char *optarg, bool& invalidOpt)
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
    return atof(optarg2.c_str());
  }
  invalidOpt = true;
  return 0;
}


typedef struct slidelevel_t
{
  bool optTif;
  bool optGoogle;
  int optDebug;
  int optQuality;
  bool optUseGamma;
  double optGamma;
  int64_t optMaxMem;
  bool optLog;
  int optBlend;
  int olympusLevel;
  int readDirection;
  int readZLevel;
  int outLevel;
  bool center;
  bool tiled;
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
  int64_t xOutTile, xTileMap;
  int64_t yOutTile, yTileMap;
  int64_t xEndTile, yEndTile;
  int64_t xDest;
  int64_t yDest;
  int64_t outputLvlTotalWidth;
  int64_t outputLvlTotalHeight;
#ifndef USE_MAGICK
  cv::Mat *pImgScaled;
  cv::Mat *pImgScaledL2Mini;
#else
  safeBmp *pImgScaled;
  safeBmp *pImgScaledL2Mini;
  Magick::MagickWand *magickWand;
  Magick::PixelWand *pixelWand;
#endif
  std::string *pTileName;
  int writeOutputWidth;
  int writeOutputHeight;
  int perc, percOld;
  bool onePercHit;
} SlideLevel;


class SlideConvertor
{
protected:
  bool mValidObject;
  bool mOptTif;
  bool mOptGoogle;
  bool mOptBlend;
  int mOptDebug;
  bool mOptZStack;
  bool mOptLog;
  int64_t mOptMaxMem;
  bool mOptUseGamma;
  double mOptGamma;
  int mOrientation;
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
  int mBaseLevel;
  bool mCenter;
  int mOptQuality;
  int mStep, mZSteps;
  int mLastZLevel, mLastDirection;
  int mTopOutLevel;
  int64_t mMaxSide;
  safeBmp *mpImageL2;
  int64_t mTotalYSections;
  BlendSection **mySubSections;
public:
  #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW__)
  static const char mPathSeparator='\\';
  #else
  static const char mPathSeparator='/';
  #endif
public:
  SlideConvertor();
  ~SlideConvertor() { closeRelated(); }
  void closeRelated();
  std::string getErrMsg() { return errMsg; }
  void setGamma(bool optUseGamma, double optGamma) { mOptUseGamma = optUseGamma; mOptGamma = optGamma; }
  void setDebugLevel(int optDebug) { mOptDebug = optDebug; }
  void setQuality(int optQuality) { mOptQuality = optQuality; }
  void setMaxMem(int64_t optMaxMem) { mOptMaxMem = optMaxMem; }
  int open(std::string inputFile, std::string outputFile, int options, int orientation, int64_t optXOffset, int64_t optYOffset);
  bool my_mkdir(std::string name);
  void calcCenters(int outLevel, int64_t& xCenter, int64_t& yCenter);
  int convert();
  int outputLevel(int olympusLevel, int magnification, int outLevel, int options, int64_t readWidthL2, int64_t readHeightL2, safeBmp *pBitmapL2);
  int checkFullL2(int64_t *pReadWidthL2, int64_t *pReadHeightL2, safeBmp **pFullL2);
  int convert2Tif();
  int convert2Gmap();
  void tileCleanup(SlideLevel &l);
  void blendL2WithSrc(SlideLevel &l);
  void processSrcTile(SlideLevel& l);
  void processGamma(SlideLevel& l);
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
  mOptZStack=getBoolOpt(SLIDE_DEFAULT_ZSTACK);
  mOptQuality=SLIDE_DEFAULT_QUALITY;
  mpImageL2=NULL;
  mTotalYSections = 0;
  mySubSections = NULL;
  mOrientation = 0;
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
    #else
    safeBmpFree(l.pImgScaled);
    #endif
    l.pImgScaled = 0;
  }
  if (l.pImgScaledL2Mini)
  {
    #ifndef USE_MAGICK
    l.pImgScaledL2Mini->release();
    delete l.pImgScaledL2Mini;
    #else
    safeBmpFree(l.pImgScaledL2Mini);
    #endif
    l.pImgScaledL2Mini = 0;
  }
  safeBmpFree(&l.subTileBitmap);
  safeBmpFree(&l.sizedBitmap2);
  safeBmpFree(&l.safeScaledL2Mini);
  safeBmpFree(&l.safeScaledL2Mini2);
}


void SlideConvertor::processGamma(SlideLevel &l)
{
  safeBmp* pSrc = l.pBitmapFinal;
  BYTE* pBmpData = pSrc->data;
  double invGamma = 1.0f/l.optGamma;

  if (pSrc == NULL || pSrc->data == NULL) return;

  int64_t strideWidth = pSrc->width * 3;
  int64_t height = pSrc->height;

  for (int64_t y=0; y < height; y++)
  {
    BYTE * pBmpData2 = pBmpData + (y * strideWidth);
    for (int64_t x=0; x < strideWidth; x++)
    {
      double gamma = (double) *pBmpData2 / (double) 255.0f;
      double powed = pow(gamma, invGamma); 
      int value = (int) round(powed * 255);
      if (value > 255) value=255;
      *pBmpData2 = (BYTE) value;
      pBmpData2++;
    }
  }
}


// Scale the larger complete L2 image into a tiled smaller 
// mini version
// of it if L2 scaling is requested and the L2 pyramid level was
// read success
void SlideConvertor::blendL2WithSrc(SlideLevel &l)
{
  safeBmp bitmapL2Mini;
  safeBmp *pFinalL2 = &bitmapL2Mini;

  safeBmpClear(&bitmapL2Mini);
  int64_t xSrcStartL2=(int64_t) round(l.xSrc * l.xScaleL2);
  int64_t ySrcStartL2=(int64_t) round(l.ySrc * l.yScaleL2);
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
  if (safeBmpAlloc2(&bitmapL2Mini, (int64_t) l.grabWidthL2, (int64_t) round(l.grabHeightL2))==NULL)
  {
    return;
  }
  safeBmpByteSet(&bitmapL2Mini, l.bkgdColor);
  safeBmpCpy(&bitmapL2Mini, xDestStartL2, yDestStartL2, l.pBitmapL2, xSrcStartL2, ySrcStartL2, (int64_t) l.grabWidthL2, (int64_t) l.grabHeightL2);
  #ifndef USE_MAGICK
  l.pImgScaledL2Mini = new cv::Mat;
  cv::Mat imgSrc((int) l.grabHeightL2, (int) l.grabWidthL2, CV_8UC3, bitmapL2Mini.data);
  cv::Size scaledSize(l.finalOutputWidth, l.finalOutputHeight);
  double xScaleResize = (double) l.inputTileWidth / (double) l.grabWidthL2;
  double yScaleResize = (double) l.inputTileHeight / (double) l.grabHeightL2;
  cv::resize(imgSrc, *l.pImgScaledL2Mini, scaledSize, xScaleResize, yScaleResize, l.scaleMethodL2);
  imgSrc.release();
  safeBmpInit(&l.safeScaledL2Mini, l.pImgScaledL2Mini->data, l.finalOutputWidth, l.finalOutputHeight);
  #else
  Magick::MagickSetCompression(l.magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(l.magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(l.magickWand, 8);
  Magick::MagickSetImageAlphaChannel(l.magickWand, Magick::OffAlphaChannel);
  Magick::MagickNewImage(l.magickWand, l.grabWidthL2, l.grabHeightL2, l.pixelWand);
  Magick::MagickImportImagePixels(l.magickWand, 0, 0, l.grabWidthL2, l.grabHeightL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  //Magick::MagickConstituteImage(l.magickWand, l.grabWidthL2, l.grabHeightL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  Magick::MagickResizeImage(l.magickWand, l.finalOutputWidth, l.finalOutputHeight, l.scaleMethodL2);
  l.pImgScaledL2Mini = safeBmpAlloc(l.finalOutputWidth, l.finalOutputHeight);
  safeBmpInit(&l.safeScaledL2Mini, l.pImgScaledL2Mini->data, l.finalOutputWidth, l.finalOutputHeight);
  Magick::MagickExportImagePixels(l.magickWand, 0, 0, l.finalOutputWidth, l.finalOutputHeight, "RGB", Magick::CharPixel, l.safeScaledL2Mini.data);
  Magick::ClearMagickWand(l.magickWand);
  #endif
  
  pFinalL2 = &l.safeScaledL2Mini;
  if (l.finalOutputWidth != l.finalOutputWidth2 || l.finalOutputHeight != l.finalOutputHeight2)
  {
    safeBmpAlloc2(&l.safeScaledL2Mini2, (int64_t) l.finalOutputWidth2, (int64_t) l.finalOutputHeight2);
    safeBmpByteSet(&l.safeScaledL2Mini2, l.bkgdColor);
    safeBmpCpy(&l.safeScaledL2Mini2, 0, 0, &l.safeScaledL2Mini, 0, 0, l.finalOutputWidth, l.finalOutputHeight);
    pFinalL2 = &l.safeScaledL2Mini2;
  }
  if (l.optDebug > 1)
  {
    std::string errMsg;
    std::string l2TileName;//=*l.pTileName;
    //l2TileName.append(".l2.jpg");
    std::stringstream ss;
    ss << "l2.l" << l.olympusLevel << "x" << l.xSrc << "y" << l.ySrc << ".jpg";
    l2TileName=ss.str();
    bool writeOk=my_jpeg_write(l2TileName, pFinalL2->data, (int) pFinalL2->width, (int) pFinalL2->height, l.optQuality, &errMsg);
    if (!writeOk) 
    {
      std::cout << "Error writing debug file '" << l2TileName << "' errMsg: " << errMsg << std::endl;
    }
  }
  safeBmpCpy(l.pBitmap4, 0, 0, l.pBitmapSrc, 0, 0, l.pBitmap4->width, l.pBitmap4->height);
  xScaleResize = (double) mBaseTotalWidth / (double) l.srcTotalWidth;
  yScaleResize = (double) mBaseTotalHeight / (double) l.srcTotalHeight;
  BlendArgs blendArgs;
  blendArgs.grabWidthB = l.grabWidthB;
  blendArgs.grabHeightB = l.grabHeightB;
  blendArgs.xSrc = l.xSrc;
  blendArgs.ySrc = l.ySrc;
  blendArgs.xMargin = 0;
  blendArgs.yMargin = 0;
  if (l.xSrc < 0)
  {
    blendArgs.grabWidthB += l.xSrc;
    blendArgs.xSrc = 0;
    blendArgs.xMargin = l.xCenter;
  }
  if (l.ySrc < 0)
  {
    blendArgs.grabHeightB += l.ySrc;
    blendArgs.ySrc = 0;
    blendArgs.yMargin = l.yCenter;
  }
  blendArgs.grabWidthB *= xScaleResize;
  blendArgs.grabHeightB *= yScaleResize;
  blendArgs.xSrc *= xScaleResize;
  blendArgs.ySrc *= yScaleResize;
  blendArgs.pSafeDest = l.pBitmap4;
  blendArgs.pSafeSrcL2 = &l.safeScaledL2Mini;
  blendArgs.xFactor = l.xBlendFactor;
  blendArgs.yFactor = l.yBlendFactor;
  blendArgs.yFreeMap = l.ySubSections;
  blendArgs.ySize = l.totalYSections;
  blendLevels(&blendArgs);
  l.pBitmapFinal = l.pBitmap4;
  safeBmpFree(&bitmapL2Mini);
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
    cv::Mat imgSrc((int) l.grabHeightRead, (int) l.grabWidthRead, CV_8UC3, l.pBitmapSrc->data);
    cv::Size scaledSize((int) l.inputSubTileWidthRead, (int) l.inputSubTileHeightRead);
    double xScaleResize = (double) l.inputSubTileWidthRead / (double) l.grabWidthRead;
    double yScaleResize = (double) l.inputSubTileHeightRead / (double) l.grabHeightRead;
    cv::resize(imgSrc, *l.pImgScaled, scaledSize, xScaleResize, yScaleResize, l.scaleMethod);
    imgSrc.release();
    safeBmpInit(&l.safeImgScaled, l.pImgScaled->data, l.inputSubTileWidthRead, l.inputSubTileHeightRead);

    #else
    Magick::MagickSetCompression(l.magickWand, Magick::NoCompression);
    Magick::MagickSetImageType(l.magickWand, Magick::TrueColorType);
    Magick::MagickSetImageDepth(l.magickWand, 8);
    Magick::MagickSetImageAlphaChannel(l.magickWand, Magick::OffAlphaChannel);
    Magick::MagickNewImage(l.magickWand, l.grabWidthRead, l.grabHeightRead, l.pixelWand);
    Magick::MagickImportImagePixels(l.magickWand, 0, 0, l.grabWidthRead, l.grabHeightRead, "RGB", Magick::CharPixel, l.pBitmapSrc->data);
    //Magick::MagickConstituteImage(l.magickWand, l.grabWidthRead, l.grabHeightRead, "RGB", Magick::CharPixel, l.pBitmapSrc->data);
    Magick::MagickResizeImage(l.magickWand, l.inputSubTileWidthRead, l.inputSubTileHeightRead, l.scaleMethod);
    l.pImgScaled = safeBmpAlloc(l.inputSubTileWidthRead, l.inputSubTileHeightRead);
    safeBmpInit(&l.safeImgScaled, l.pImgScaled->data, l.inputSubTileWidthRead, l.inputSubTileHeightRead);
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
    int64_t xSubLoc = (int64_t) round((double) l.xSubTile * ((double) l.inputTileWidth / (double) l.totalSubTiles));
    int64_t ySubLoc = (int64_t) round((double) l.ySubTile * ((double) l.inputTileHeight / (double) l.totalSubTiles));
    xSubLoc += l.xMargin;
    ySubLoc += l.yMargin;
    if (copyWidth > 0 && copyHeight > 0)
    {
      safeBmpCpy(&l.bitmap1, xSubLoc, ySubLoc, l.pBitmapSrc, 0, 0, copyWidth, copyHeight);
    }
    l.pBitmapSrc = &l.bitmap1;
    l.pBitmapFinal = &l.bitmap1;
  }
  if (l.optDebug > 1) 
  {
    std::string preTileName;
    std::string errMsg;
    std::stringstream ss;
    ss << "pre.l" << l.olympusLevel << "x" << l.xSrc << "y" << l.ySrc << ".jpg";
    preTileName=ss.str();
    bool writeOk=my_jpeg_write(preTileName, l.pBitmapSrc->data, (int) l.pBitmapSrc->width, (int) l.pBitmapSrc->height, l.optQuality, &errMsg);
    if (!writeOk)
    {
      std::cout << "Error writing debug file: " << errMsg << std::endl;
    }
  }
}


void SlideConvertor::printPercDone(SlideLevel& l)
{
  l.perc=(int)(((double) l.yDest / (double) l.outputLvlTotalHeight) * 100);
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


int SlideConvertor::outputLevel(int olympusLevel, int magnification, int outLevel, int options, int64_t readWidthL2, int64_t readHeightL2, safeBmp *pBitmapL2)
{
  std::ostringstream output;
  SlideLevel l;

  memset(&l, 0, sizeof(l));
  
  l.optGoogle = mOptGoogle;
  l.optTif = mOptTif;
  l.optBlend = mOptBlend;
  l.optDebug = mOptDebug;
  l.optMaxMem = mOptMaxMem;
  l.optGamma = mOptGamma;
  l.optUseGamma = mOptUseGamma;
  l.optLog = mOptLog;
  l.tiled = (options & LEVEL_TILED) ? true : false;
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
  l.xBlendFactor = l.magnifyX; 
  l.yBlendFactor = l.magnifyY;
  l.fillin = (mOptBlend && l.olympusLevel < 2) ? true : false;

  l.srcTotalWidth = slide->getActualWidth(olympusLevel);
  l.srcTotalHeight = slide->getActualHeight(olympusLevel);
  if (l.fillin && slide->checkLevel(2))
  {
    l.srcTotalWidthL2 = slide->getActualWidth(2);
    l.srcTotalHeightL2 = slide->getActualHeight(2);
  }
  else if (l.fillin && slide->checkLevel(3))
  {
    l.srcTotalWidthL2 = slide->getActualWidth(3);
    l.srcTotalHeightL2 = slide->getActualHeight(3);
  }
  else
  {
    l.fillin = false;
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
  if (l.tiled)
  {
    l.finalOutputWidth=256;
    l.finalOutputHeight=256;
    l.finalOutputWidth2=256;
    l.finalOutputHeight2=256;
    l.inputTileWidth=256;
    l.inputTileHeight=256;
    l.grabWidthA=(double) l.inputTileWidth * l.xScale;
    l.grabHeightA=(double) l.inputTileHeight * l.yScale;
    l.grabWidthB=(double) l.finalOutputWidth * l.xScale;
    l.grabHeightB=(double) l.finalOutputHeight * l.yScale;
    l.grabWidthL2=ceil(256.0 * (double) l.srcTotalWidthL2 / (double) l.destTotalWidth);
    l.grabHeightL2=ceil(256.0 * (double) l.srcTotalHeightL2 / (double) l.destTotalHeight);
  }
  else
  {
    l.finalOutputWidth=(int) l.destTotalWidth;
    l.finalOutputHeight=(int) l.destTotalHeight;
    l.finalOutputWidth2=(int) l.destTotalWidth2;
    l.finalOutputHeight2=(int) l.destTotalHeight2;
    l.inputTileWidth=(int) l.destTotalWidth2;
    l.inputTileHeight=(int) l.destTotalHeight2;
    l.grabWidthA=(double) l.srcTotalWidth;
    l.grabHeightA=(double) l.srcTotalHeight;
    l.grabWidthB=(double) l.srcTotalWidth;
    l.grabHeightB=(double) l.srcTotalHeight;
    l.grabWidthL2=(double) l.srcTotalWidthL2;
    l.grabHeightL2=(double) l.srcTotalHeightL2;
  }
  l.totalSubTiles = 1;
  int64_t totalGrabBytes = (int64_t) round(l.grabWidthB) * (int64_t) round(l.grabHeightB) * 3;
  if (totalGrabBytes > l.optMaxMem)
  {
    do
    {
      l.totalSubTiles *= 2;
      totalGrabBytes = (int64_t) (ceil((double) l.grabWidthB / (double) l.totalSubTiles) * ceil((double) l.grabHeightB / (double) l.totalSubTiles) * 3);
    }
    while (totalGrabBytes > l.optMaxMem);
    std::cout << "Using max memory " << (totalGrabBytes / (1024 * 1024)) << "mb max width=" << round(l.grabWidthB) << " x height=" << round(l.grabHeightB) << " for pixel resizer." << std::endl;
    l.grabWidthA = l.grabWidthB / (double) l.totalSubTiles;
    l.grabHeightA = l.grabHeightB / (double) l.totalSubTiles;
  } 
  if (l.center && l.optGoogle)
  {
    calcCenters(l.outLevel, l.xLevelOffset, l.yLevelOffset);
  
    l.xStartTile = l.xLevelOffset / 256;
    l.xCenter = l.xLevelOffset % 256;
    l.xStartSrc = (double)(-l.xCenter) * l.xScale;
    l.outputLvlTotalWidth = (int64_t) ceil((double) (l.xCenter + l.destTotalWidth) / 256.0) * 256;

    l.yTileMap = l.yLevelOffset / 256;
    l.yCenter = l.yLevelOffset % 256;
    l.yStartSrc = (double)(-l.yCenter) * l.yScale;
    l.outputLvlTotalHeight = (int64_t) ceil((double) (l.yCenter + l.destTotalHeight) / 256.0) * 256;
  }
  else
  {
    l.xStartTile = 0;
    l.xCenter = 0;
    l.xStartSrc = 0.0;
    
    l.yTileMap = 0;
    l.yCenter = 0;
    l.yStartSrc = 0.0;
    if (l.tiled)
    {
      l.outputLvlTotalWidth = (int64_t) ceil((double) l.destTotalWidth / l.inputTileWidth) * l.inputTileWidth;
      l.outputLvlTotalHeight = (int64_t) ceil((double) l.destTotalHeight / l.inputTileHeight) * l.inputTileHeight;
    }
    else
    {
      l.outputLvlTotalWidth = l.destTotalWidth2;
      l.outputLvlTotalHeight = l.destTotalHeight2;
    } 
  }
  if (l.tiled)
  {
    l.xEndTile = (int64_t) ceil((double) l.outputLvlTotalWidth / (double) l.inputTileWidth);
    l.yEndTile = (int64_t) ceil((double) l.outputLvlTotalHeight / (double) l.inputTileHeight);
  }
  else
  {
    l.xEndTile = 1;
    l.yEndTile = 1;
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
  l.optQuality=slide->getQuality(l.olympusLevel);
  if (l.optQuality == 0 || l.optQuality < mOptQuality)
  {
    l.optQuality = mOptQuality;
  }
  if (l.optGoogle)
  {
    output << "Google Maps Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (l.optLog) *logFile << output.str();
    std::cout << output.str();
  }
  else if (l.optTif)
  {
    output << "Tiff Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (l.optLog) *logFile << output.str();
    std::cout << output.str();
   
    int totalMag=slide->getMagnification();
    if (totalMag<=0)
    {
      totalMag=40;
    }
    std::ostringstream oss;
    if (mBaseLevel==olympusLevel || l.tiled==false) 
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
    if (mTif->setAttributes(3, 8, (int) l.destTotalWidth2, (int) l.destTotalHeight2, (l.tiled==true ? l.finalOutputWidth : 0), (l.tiled==true ? l.finalOutputHeight : 0), 1, l.optQuality)==false || mTif->setDescription(strAttributes, (int) mBaseTotalWidth2, (int) mBaseTotalHeight2)==false)
    {
      std::string errMsg;
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to write tif attributes: " << errMsg << std::endl; 
      return 4;
    }
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
    l.pBitmap4 = safeBmpAlloc(l.finalOutputWidth2, l.finalOutputHeight2);
    if (l.readOkL2 && l.optBlend)
    {
      if (mTotalYSections == 0)
      {
        mTotalYSections = mBaseTotalHeight2;
        mySubSections=new BlendSection*[mTotalYSections];
        memset(mySubSections, 0, mTotalYSections*sizeof(BlendSection*));
        slide->blendLevelsRegionScan(mySubSections, mTotalYSections, mOrientation);
      }
      l.totalYSections = mTotalYSections;
      l.ySubSections=mySubSections;
    }
    if (l.optLog)
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
    l.yOutTile = 0;
    while (l.yOutTile < l.yEndTile && round(l.ySrc) < l.srcTotalHeight)
    {
      std::ostringstream yRootStream, yRootStreamZip;
      std::string dirPart1, dirPartZip1;
      std::string dirPart2, dirPartZip2;
      std::string yRoot, yRootZip;
      
      yRootStream << mFileNameOnly;
      dirPart1 = yRootStream.str();
      yRootStream << mPathSeparator << outLevel;
      dirPart2 = yRootStream.str();
      yRootStream << mPathSeparator << l.yTileMap;
      yRoot = yRootStream.str();
      if (l.optDebug > 1)
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
      yRootStreamZip << ZipFile::mZipPathSeparator << l.yTileMap;
      yRootZip = yRootStreamZip.str();
      if (l.optGoogle) 
      {
        // Create the google maps directory structure up to the the y tile
        if (mZip->addDir(dirPartZip1)==-1 || mZip->addDir(dirPartZip2)==-1 || mZip->addDir(yRootZip)==-1)
        {
          std::cerr << "Failed to add zip directory '" << yRoot << "' to archive. Reason: " << mZip->getErrorMsg() << std::endl << "Stopping!" << std::endl;
          error = true;
          break;
        }
      }
      l.ySrc = (l.yOutTile * l.grabHeightB) + l.yStartSrc;
      l.xSrc = l.xStartSrc;
      l.xDest = 0;
      l.xTileMap = l.xStartTile;
      if (round(l.ySrc) + round(l.grabHeightB) < 1.0f)
      {
        std::cerr << "For some reason we have hit this! l.ySrc=" << l.ySrc << " l.yStartSrc=" << l.yStartSrc << " l.grabHeightB=" << l.grabHeightB << std::endl;
        l.yOutTile++;
        continue;
      }
      for (l.xOutTile=0; l.xOutTile < l.xEndTile && round(l.xSrc) < l.srcTotalWidth && error==false; l.xOutTile++) 
      {
        std::ostringstream tileNameStream, tileNameStreamZip;
        std::string errMsg;
        std::string fullTilePath;
        l.xSrc = (l.xOutTile * l.grabWidthB) + l.xStartSrc;
        if (round(l.xSrc) + round(l.grabWidthB) < 1.0f)
        {
          std::cerr << "For some reason we have hit this! l.xSrc=" << l.xSrc << " l.xStartSrc=" << l.xStartSrc << " l.grabWidthB=" << l.grabWidthB << std::endl;
          continue;
        }
        tileNameStream << yRoot << mPathSeparator << l.xTileMap << ".jpg";
        tileNameStreamZip << yRootZip << ZipFile::mZipPathSeparator << l.xTileMap << ".jpg";
        fullTilePath = tileNameStream.str();
        l.pTileName = &fullTilePath;
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
        l.ySubTile=0;
        while (l.ySubTile < l.totalSubTiles && round(l.ySrc) < l.srcTotalHeight)
        {
          l.xSrc = xSrcStart2;
          l.ySrc = ySrcStart2 + (l.ySubTile * l.grabHeightA);
          if (round(l.ySrc) + round(l.grabHeightA) < 1.0f) 
          {
            l.ySubTile++;
            continue;
          }
          for (l.xSubTile=0; l.xSubTile < l.totalSubTiles && round(l.xSrc) < l.srcTotalWidth; l.xSubTile++)
          {
            l.xSrc = xSrcStart2 + (l.xSubTile * l.grabWidthA);
            if (round(l.xSrc) + round(l.grabWidthA) < 1.0f) continue;
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
              l.inputSubTileWidthRead = (int64_t) round(grabWidthReadDec * l.xScaleReverse);
            }
            if (l.ySrcRead + grabHeightReadDec > (double) l.srcTotalHeight)
            {
              grabHeightReadDec = (double) l.srcTotalHeight - l.ySrcRead;
              l.inputSubTileHeightRead = (int64_t) round(grabHeightReadDec * l.yScaleReverse);
            }
            if (l.xSrc < 0.0 || l.ySrc < 0.0)
            {
              l.inputSubTileWidthRead = (int64_t) round(grabWidthReadDec * l.xScaleReverse);
              l.inputSubTileHeightRead = (int64_t) round(grabHeightReadDec * l.yScaleReverse);
            }
            l.grabWidthRead = (int64_t) round(grabWidthReadDec);
            l.grabHeightRead = (int64_t) round(grabHeightReadDec);
            if (l.grabWidthRead <= 0 || l.grabHeightRead <= 0) continue;
            bool allocOk = slide->allocate(&l.subTileBitmap, olympusLevel, (int64_t) round(l.xSrcRead), (int64_t) round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false);
            if (allocOk == false) continue; 
            l.pBitmapSrc = &l.subTileBitmap;
            l.pBitmapFinal = &l.subTileBitmap;
            safeBmpByteSet(&l.subTileBitmap, l.bkgdColor);
            
            l.readWidth=0;
            l.readHeight=0;
            if (l.pBitmapSrc->data == NULL)
            {
              error = true;
              break;
            }
            if (l.optDebug > 2)
            {
              *logFile << " slide->read(x=" << l.xSrcRead << " y=" << l.ySrcRead << " grabWidthA=" << l.grabWidthRead << " grabHeightA=" << l.grabHeightRead << " olympusLevel=" << l.olympusLevel << "); " << std::endl;
            }
            readOkSrc = slide->read(l.pBitmapSrc->data, l.olympusLevel, l.readDirection, l.readZLevel, (int64_t) round(l.xSrcRead), (int64_t) round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false, &l.readWidth, &l.readHeight);
            if (readOkSrc)
            {
              readSubTiles++;
            }
            else
            {
              std::cerr << "Failed to read olympus level " << l.olympusLevel << " jpeg tile @ x=" << l.xSrcRead << " y=" << l.ySrcRead << " width=" << l.grabWidthRead << " height=" << l.grabHeightRead << std::endl;
            }
            if (l.optDebug > 2)
            {
              std::cout << "readWidth: " << l.readWidth << " readHeight: " << l.readHeight<< " grabWidth: " << l.grabWidthRead << " grabHeight: " << l.grabHeightRead << std::endl;
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
              #else
              safeBmpFree(l.pImgScaled);
              #endif
              l.pImgScaled = 0;
              safeBmpFree(&l.subTileBitmap);
            }
            l.xSrc += l.grabWidthA;
          }
          l.ySubTile++;
          l.ySrc = ySrcStart2 + (l.ySubTile * l.grabHeightA);
          printPercDone(l);
        }
        l.xSrc = xSrcStart2;
        l.ySrc = ySrcStart2;
        l.xSrcRead = l.xSrc;
        l.ySrcRead = l.ySrc;
        safeBmpClear(&l.safeScaledL2Mini);
        safeBmpClear(&l.safeScaledL2Mini2);
        l.pImgScaledL2Mini = NULL;
        if (readSubTiles > 0) 
        {
          bool writeOk=false;
          if (l.readOkL2 && l.optBlend)
          {
            blendL2WithSrc(l);  
          }
          if (l.optUseGamma)
          {
            processGamma(l);
          }
          if (l.optGoogle)
          {
            BYTE* pJpegBytes = NULL;
            unsigned long outSize = 0;
            std::string tileName = tileNameStreamZip.str();
            bool compressOk=my_jpeg_compress(&pJpegBytes, l.pBitmapFinal->data, (int) l.pBitmapFinal->width, (int) l.pBitmapFinal->height, l.optQuality, &errMsg, &outSize);
            if (compressOk && mZip->addFile(tileName, pJpegBytes, outSize)==OLY_ZIP_OK)
            {
              writeOk=true;
            }
            else
            {
              error=true;
            }
            my_jpeg_free(&pJpegBytes);
          }
          else if (l.optTif)
          {
            if (l.tiled)
            {
              writeOk=mTif->writeEncodedTile(l.pBitmapFinal->data, (unsigned int) l.xDest, (unsigned int) l.yDest, 1);
            }
            else
            {
              std::stringstream ss;
              ss << "l" << l.olympusLevel << "mag" << l.magnifyX << ".jpg";
              std::string fname = ss.str();
              std::string errMsg;
              my_jpeg_write(fname, l.pBitmapFinal->data, (int) l.pBitmapFinal->width, (int) l.pBitmapFinal->height, 90, &errMsg); 
              writeOk=mTif->writeImage(l.pBitmapFinal->data);
            }
          }
          if (writeOk==false)
          {
            std::string errMsg;
            if (l.optGoogle)
            {
              std::cerr << "Failed to write jpeg tile '" << *l.pTileName << "' reason: " << mZip->getErrorMsg() << std::endl;
            }
            else if (l.optTif && l.tiled)
            {
              mTif->getErrMsg(errMsg);
              std::cerr << "Failed to write tif tile x=" << l.xDest << " y=" << l.yDest << " reason: " << errMsg << std::endl;
            }
            else if (l.optTif && !l.tiled)
            {
              mTif->getErrMsg(errMsg);
              std::cerr << "Failed to write tif image at tif level=" << l.outLevel << " reason: " << errMsg << std::endl;
            }
            error = true;
          }
        }
        tileCleanup(l);
        l.xDest += l.finalOutputWidth2;
        l.xTileMap++;
      }
      l.yDest += l.finalOutputHeight2;
      l.yTileMap++;
      l.yOutTile++;
      l.ySrc = (l.yOutTile * l.grabHeightB) + l.yStartSrc;
      printPercDone(l);
    }
    if (l.optTif)
    {
      bool success = mTif->writeDirectory();
      if (success == false)
      {
        const char *tifDirErrorMsg = "Fatal Error: Failed to write tif directory: ";
        std::string errMsg;
        mTif->getErrMsg(errMsg);
        std::cerr << tifDirErrorMsg << errMsg << std::endl;
        if (l.optLog) *logFile << tifDirErrorMsg << errMsg << std::endl;
        error = true;
      }
    }
  }
  catch (std::bad_alloc& ba)
  {
    const char *msg = "Fatal Error: Failed to get memory. Cannot continue!";
    std::cout << msg << std::endl;
    if (l.optLog) *logFile << msg << std::endl;
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

  int64_t srcTotalWidthL2 = 0;
  int64_t srcTotalHeightL2 = 0;
  if (slide->checkLevel(2))
  {
    srcTotalWidthL2 = slide->getActualWidth(2);
    srcTotalHeightL2 = slide->getActualHeight(2);
  }
  else if (slide->checkLevel(3))
  {
    srcTotalWidthL2 = slide->getActualWidth(3);
    srcTotalHeightL2 = slide->getActualHeight(3);
  }
  else
  {
    return 1;
  }

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
      mOptBlend = false;
      break;
    case 2:
      std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
      return 1;
    case 3:
      std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
      mOptBlend = false;
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
  int options = LEVEL_TILED; 
  while (divisor != maxDivisor && error==0)
  {
    int tiled = 1;
    int olympusLevel = 1;
    if (mOptBlend==false)
    {
      for (olympusLevel = 0; olympusLevel < 4; olympusLevel++)
      {
        if (slide->checkLevel(olympusLevel)) break;
      }
    }
    switch (step)
    {
      case 1:
        divisor = 1;
        if (slide->checkLevel(0))
        {
          olympusLevel = 0;
        }
        break;
      case 2:
        divisor = maxDivisor * 2;
        tiled = 0;
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
    options = LEVEL_TILED * tiled;
    error=outputLevel(olympusLevel, divisor, step, options, readWidthL2, readHeightL2, pFullL2Bitmap);
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
      mOptBlend = false;
      break;
    case 2:
      std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
      return 1;
    case 3:
      std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
      mOptBlend = false;
      break;
  }
  //****************************************************************
  // Output each level, each level is 2^level size 
  //****************************************************************
  int divisor = 1 << mTopOutLevel;
  int outLevel = 0;

  while (outLevel <= mTopOutLevel && error==0)
  {
    int olympusLevel;
    if (slide->checkLevel(0) && slide->checkLevel(1) && mOptBlend)
    {
      if (divisor < 4)
      {
        olympusLevel = 0;
      }
      else
      {
        olympusLevel = 1;
      }
    }
    else 
    {
      for (olympusLevel = 0; olympusLevel < 4; olympusLevel++)
      {
        if (slide->checkLevel(olympusLevel)) break;
      }
    }
    error=outputLevel(olympusLevel, divisor, outLevel, LEVEL_TILED, readWidthL2, readHeightL2, pFullL2Bitmap);
    divisor /= 2;
    outLevel++;
  }
  if (error==0 && outLevel > mTopOutLevel && outLevel > 0)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  return error;
}


int SlideConvertor::open(std::string inputFile, std::string outputFile, int options, int orientation, int64_t optXOffset, int64_t optYOffset)
{
  mValidObject = false;
  closeRelated();
  logFile = new std::ofstream;
  mOptTif = options & CONV_TIF;
  mOptGoogle = options & CONV_GOOGLE;
  mOrientation = orientation;
  mOptLog = options & CONV_LOG;
  mOptBlend = options & CONV_BLEND;
  mOptZStack = options & CONV_ZSTACK;
  if (mOptLog)
  {
    logFile->open("olyvslideconv.log");
  }
  slide = new CompositeSlide();
  errMsg="";
  if (slide->open(inputFile.c_str(), options, orientation, mOptDebug, optXOffset, optYOffset, &mpImageL2)==false)
  {
    return 1;
  }
  mOutputFile = outputFile;
  mOutputDir = outputFile;
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

  if (mOptTif)
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
  else if (mOptGoogle)
  {
    mZip = new ZipFile();
    if (mZip->openArchive(outputFile.c_str(), OLY_APPEND_STATUS_CREATE) != 0)
    {
      std::cerr << "Failed to create zip file '" << outputFile << "'. Reason: " << mZip->getErrorMsg() << std::endl;
      return 2;
    }
    mZip->setCompression(OLY_DEFAULT_COMPRESS_METHOD, OLY_DEFAULT_COMPRESS_LEVEL);

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
      mBaseTotalWidth2 = (int64_t) ceil((double) mBaseTotalWidth / (double) 256.0) * 256;
      mBaseTotalHeight = slide->getActualHeight(mBaseLevel);
      mBaseTotalHeight2 = (int64_t) ceil((double) mBaseTotalHeight / (double) 256.0) * 256;
      break;
    }
  }
  mStep=0;
  mLastZLevel=-1;
  mMaxSide = 0;
  mTopOutLevel = 0;
  
  std::cout << "baseTotalWidth=" << mBaseTotalWidth << " baseTotalHeight=" << mBaseTotalHeight << std::endl;
  if (mBaseTotalWidth > 0 && mBaseTotalHeight > 0)
  {
    mValidObject = true;
    if (mOptGoogle)
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
  if (mySubSections)
  {
    blendLevelsFree(mySubSections, mTotalYSections);
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


int main(int argc, char** argv)
{
  SlideConvertor slideConv;
  int error=0;
  std::string infile, outfile;
  
  int optGoogle = getBoolOpt(SLIDE_DEFAULT_GOOGLE);
  int optTif = getBoolOpt(SLIDE_DEFAULT_TIF);
  int optBlend = getBoolOpt(SLIDE_DEFAULT_BLEND);
  int optHighlight = getBoolOpt(SLIDE_DEFAULT_HIGHLIGHT);
  int optLog = getBoolOpt(SLIDE_DEFAULT_LOG); 
  int optOpenCVAlign = getBoolOpt(SLIDE_DEFAULT_OPENCV_ALIGN);
  int optZStack = getBoolOpt(SLIDE_DEFAULT_ZSTACK);
  int optQuality = SLIDE_DEFAULT_QUALITY;
  int optDebug = SLIDE_DEFAULT_DEBUG;
  int optMaxJpegCache = SLIDE_DEFAULT_MAX_JPEG_CACHE;
  int64_t optMaxMem = SLIDE_DEFAULT_MAX_MEM; 
  double optGamma = 1.0f;
  int optRotate = 0;
  int optUseGamma = 0;
  int optXOffset = 0;
  int optYOffset = 0;
  int optUseXOffset = 0;
  int optUseYOffset = 0;
  int allOptions = 0;
  
  const char fullSyntax[] =
"Usage: olyvslideconv [OPTION] <inputfolder> <outputfile>\n"
"Main Output Flags:\n"
"  -g, --google               Output to google maps format zipped. Default " 
SLIDE_DEFAULT_GOOGLE ".\n"
"  -t, --tif                  Output to tif file instread. Default " 
SLIDE_DEFAULT_TIF ".\n\n"
"Other Optional Flags:\n"
"  -a, --gamma                Alter gamma. Gamma less than one is darker, \n"
"                             greater than one is brighter. 1.5 is 50% percent\n"
"                             brighter 0.5 is 50% darker. Default no extra\n"
"                             gamma processing is done.\n"
"  -b, --blend                Blend the top level with the middle level. \n"
"                             Default " 
SLIDE_DEFAULT_BLEND ".\n"
"  -d, --debug=x              Debug mode, output debugging info and files. The\n"
"                             higher the more debugging output. Sets logging\n"
"                             on as well if greater than 1. Default " 
xstringfy(SLIDE_DEFAULT_DEBUG) ".\n"
"  -h, --highlight            Highlight visible lower areas with a black\n"
"                             border. Default " 
SLIDE_DEFAULT_HIGHLIGHT ".\n"
"  -j, --max-jpeg-cache=x     Specify max jpeg cache size in megabytes.Don't\n"
"                             specify this unless you run out of memory or\n"
"                             looking to decrease the time to output.\n"
"                             Default " 
xstringfy(SLIDE_DEFAULT_MAX_JPEG_CACHE) "mb.\n"
"  -l, --log                  Log general information about the slide. \n"
"                             Default " 
SLIDE_DEFAULT_LOG ".\n"
"  -m, --max-mem=x            Specify max memory size of pixel rescaler or\n"
"                             resizer in megabytes. Don't specify this unless\n"
"                             you run out of memory or want to increase\n"
"                             performance. Default " 
xstringfy(SLIDE_DEFAULT_MAX_MEM) "mb.\n"
"  -o, --opencv-align         Use opencv (computer vision) to calculate the \n"
"                             upper alignment offset for the upper and lower\n"
"                             levels (almost never required). Default " 
SLIDE_DEFAULT_OPENCV_ALIGN ".\n"
"  -q, --quality=x            Set minimal jpeg quality percentage. Default " 
xstringfy(SLIDE_DEFAULT_QUALITY) "%.\n"
"  -r, --rotate=x             Set orientation of slide or rotate the entire\n"
"                             by x degrees. Currently only -90, 90, 180, or\n"
"                             270 degree rotation is supported.\n"
"  -x, --xoffset=x            Manually set X alignment offset of top pyramid\n"
"                             level with the bottom. Use this if the default\n"
"                             calculation or opencv computer vision method\n"
"                             does not align the top and bottom levels\n"
"                             properly.\n"
"  -y, --yoffset=y            Manually set Y alignment offset of top pyramid\n"
"                             level with the bottom. Use this if the default\n"
"                             calculation or opencv computer vision method\n"
"                             does not align the top and bottom levels\n"
"                             properly.\n"
"  -z, --zstack               Process Z-stack if the image has one.\n"
"                             Experimental and only works for tif files.\n"
"                             Default " SLIDE_DEFAULT_ZSTACK ".\n\n";
  if (argc < 3)
  {
    std::cerr << fullSyntax;
    return 1;
  }
  int opt;
  int optIndex = 0;
  bool invalidOpt = false;
  char emptyString[] = "";
  static struct option longOptions[] =
    {
      // Main output arguments
      {"google",            no_argument,        0,             'g'},
      {"tiff",              no_argument,        0,             't'},
      
      // Optional output arguments
      {"gamma",             required_argument,  0,             'a'},
      {"blend",             no_argument,        0,             'b'},
      {"debug",             required_argument,  0,             'd'},
      {"highlight",         no_argument,        0,             'h'},
      {"max-jpeg-cache",    required_argument,  0,             'j'},
      {"log",               no_argument,        0,             'l'},
      {"max-mem",           required_argument,  0,             'm'},   
      {"opencv-align",      no_argument,        0,             'o'},
      {"quality",           required_argument,  0,             'q'},
      {"rotate",            required_argument,  0,             'r'},
      {"xoffset",           required_argument,  0,             'x'},
      {"yoffset",           required_argument,  0,             'y'},
      {"zstack",            no_argument,        0,             'z'},
      {0,                   0,                  0,             0 }
    };
  
  while((opt = getopt_long(argc, argv, "gta:bd:hj:lm:oq:r:x:y:z", longOptions, &optIndex)) != -1)
  {
    if (optarg == NULL) optarg = emptyString;
    switch (opt)
    {
      case 'a':
        optGamma = getDoubleOpt(optarg, invalidOpt);
        optUseGamma = 1;
        break;
      case 'b':
        optBlend = getBoolOpt(optarg, invalidOpt);
        break;
      case 'd':
        optDebug = getIntOpt(optarg, invalidOpt);
        break;
      case 'g':
        optGoogle = getBoolOpt(optarg, invalidOpt);
        if (optGoogle) optTif=0;
        break;
      case 'h':
        optHighlight = getBoolOpt(optarg, invalidOpt);
        break;
      case 'j':
        optMaxJpegCache = getIntOpt(optarg, invalidOpt);
        break;
      case 'l':
        optLog = getBoolOpt(optarg, invalidOpt);
        break;
      case 'm':
        optMaxMem = getIntOpt(optarg, invalidOpt);
        break;
      case 'o':
        #ifndef USE_MAGICK
        optOpenCVAlign = getBoolOpt(optarg, invalidOpt);
        #endif
        break;
      case 'q':
        optQuality = getIntOpt(optarg, invalidOpt);
        break;
      case 'r':
        optRotate = getIntOpt(optarg, invalidOpt);
        break;
      case 't':
        optTif = getBoolOpt(optarg, invalidOpt);
        if (optTif) optGoogle=0;
        break;
      case 'x':
        optXOffset = getIntOpt(optarg, invalidOpt);
        optUseXOffset = 1;
        break;
      case 'y':
        optYOffset = getIntOpt(optarg, invalidOpt);
        optUseYOffset = 1;
        break;
      case 'z':
        optZStack = getBoolOpt(optarg, invalidOpt);
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
      std::cerr << fullSyntax;
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
  if (optDebug > 1) optLog = 1;
  
  if (infile.length() == 0 || outfile.length() == 0)
  {
    std::cerr << fullSyntax;
    return 1;
  }
  allOptions = (optOpenCVAlign * CONV_OPENCV_ALIGN) | 
            (optBlend * CONV_BLEND) |
            (optHighlight * CONV_HIGHLIGHT) |
            (optZStack * CONV_ZSTACK) |
            (optLog * CONV_LOG) |
            (optUseXOffset * CONV_CUSTOM_XOFFSET) |
            (optUseYOffset * CONV_CUSTOM_YOFFSET) |
            (optUseGamma * CONV_CUSTOM_GAMMA) |
            (optGoogle * CONV_GOOGLE) |
            (optTif * CONV_TIF);

  if ((allOptions & (CONV_TIF | CONV_GOOGLE))==0)
  {
    std::cerr << "No output format specified. Please use -g or --google for google maps output or -t or --tiff for tif output." << std::endl;
    return 1;
  }
  if ((allOptions & CONV_TIF) && (allOptions & CONV_GOOGLE))
  {
    std::cerr << "Please specify either -g or --google for google maps output or -t or --tiff for tif output." << std::endl;
    return 1;
  }
  if (optMaxMem < 32)
  {
    std::cerr << "Max resize memory must be a size greater than or equal to 32 megabytes." << std::endl;
    return 1;
  }
  if (optMaxJpegCache == 0)
  {
    optMaxJpegCache = 1;
  }

  if (allOptions & CONV_GOOGLE)
  {
    std::cout << "Output format: Google Maps Zipped" << std::endl;
  }
  if (allOptions & CONV_TIF)
  {
    std::cout << "Output format: TIFF/SVS" << std::endl;
  }
  if (optRotate != 0)
  {
    std::cout << "Slide Orientation: " << optRotate << " degrees" << std::endl;
  }  

  if (optDebug > 0)
  {
    if (optRotate == 0)
    {
      std::cout << "Slide Orientation: Normal" << std::endl;
    }
    std::cout << "Set logging: " << boolInt2Txt(allOptions & CONV_LOG) << std::endl;
    std::cout << "Set debug level: " << optDebug << std::endl;
    std::cout << "Set minimum quality: " << optQuality << std::endl;
    std::cout << "Use OpenCV/computer vision for XY alignment: " << boolInt2Txt(allOptions & CONV_OPENCV_ALIGN) << std::endl;
    std::cout << "Set border highlight: " << boolInt2Txt(allOptions & CONV_HIGHLIGHT) << std::endl;
    std::cout << "Set blend levels: " << boolInt2Txt(allOptions & CONV_BLEND) << std::endl;
    std::cout << "Set blend by region: " << boolInt2Txt(allOptions & CONV_REGION) << std::endl;
    if (optUseGamma)
    {
      std::cout << "Set gamma: " << optGamma << std::endl;
    }
    else
    {
      std::cout << "Set gamma: default" << std::endl;
    }
    if (optUseXOffset) 
    {
      std::cout << "Set option X offset: " << optXOffset << std::endl;
    }
    else
    {
      std::cout << "Set option X offset: default" << std::endl;
    }
    if (optUseYOffset)
    {
      std::cout << "Set option Y offset: " << optYOffset << std::endl;
    }
    else
    {
      std::cout << "Set option Y offset: default" << std::endl;
    }
    std::cout << "Set maximum resize/scale memory: " << optMaxMem << "mb " << std::endl;
    std::cout << "Set maximum jpeg cache memory: " << optMaxJpegCache << "mb " << std::endl;
  }

  jpgCache.setMaxOpen(optMaxJpegCache);

  #ifdef USE_MAGICK
  #if defined(__MINGW32__) || defined(__MINGW64__)
  quickEnv("MAGICK_CODER_MODULE_PATH", getMagickCoreCoderPath(), optDebug);
  quickEnv("MAGICK_CODER_FILTER_PATH", getMagickCoreFilterPath(), optDebug);
  #endif
  quickEnv("MAGICK_MAP_LIMIT", MAGICK_MAP_LIMIT, optDebug);
  quickEnv("MAGICK_MEMORY_LIMIT", MAGICK_MEMORY_LIMIT, optDebug);
  quickEnv("MAGICK_DISK_LIMIT", MAGICK_DISK_LIMIT, optDebug);
  quickEnv("MAGICK_AREA_LIMIT", MAGICK_AREA_LIMIT, optDebug);
  Magick::MagickWandGenesis();
  #endif
 
  slideConv.setDebugLevel(optDebug);
  error=slideConv.open(infile.c_str(), outfile.c_str(), allOptions, optRotate, optXOffset, optYOffset);
  slideConv.setGamma(allOptions & CONV_CUSTOM_GAMMA, optGamma);
  slideConv.setQuality(optQuality);
  slideConv.setMaxMem(optMaxMem * 1024 * 1024);

  if (error==0)
  {
    if (allOptions & CONV_TIF)
    {
      error = slideConv.convert2Tif();
    }
    else if (allOptions & CONV_GOOGLE)
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

