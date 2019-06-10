/**************************************************************************
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
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include "console-mswin.h"
#include "getopt-mswin.h"
#else
#include "console-unix.h"
#include <unistd.h>
#include <getopt.h>
#endif
#include "jpgsupport.h"
#include "tiffsupport.h"
#include "composite.h"

#define OLYVSLIDE_GOOGLE 2
#define OLYVSLIDE_TIF    1

std::string bool2txt(bool cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
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
  int64_t srcTotalWidth;
  int64_t srcTotalHeight;
  int64_t srcTotalWidthL2;
  int64_t srcTotalHeightL2;
  int64_t L2Size;
  int64_t destTotalWidth;
  int64_t destTotalHeight;
  int finalOutputWidth;
  int finalOutputHeight;
  int inputTileWidth;
  int inputTileHeight;
  int quality;
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
  int64_t xMargin;
  int64_t yMargin;
  int scaleMethod;
  int scaleMethodL2;
  int64_t totalXSections, totalYSections;
  unsigned char bkgColor;
  int16_t *xSubSections;
  int16_t *ySubSections;
  BYTE* pBitmap1;
  BYTE* pBitmapSrc;
  BYTE* pBitmapL2;
  BYTE* pSizedBitmap;
  BYTE* pSizedBitmap2;
  BYTE* pBitmap4;
  BYTE* pBitmapFinal;
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
  cv::Mat *pImgScaled;
  cv::Mat *pImgScaledL2Mini;
  std::string tileName;
  int writeOutputWidth;
  int writeOutputHeight;
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
    srcTotalWidth=0;
    srcTotalHeight=0;
    srcTotalWidthL2=0;
    srcTotalHeightL2=0;
    L2Size=0;
    destTotalWidth=0;
    destTotalHeight=0;
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
    scaleMethod=0;
    scaleMethodL2=0;
    totalXSections=0, totalYSections=0;
    bkgColor=0;
    xSubSections=NULL;
    ySubSections=NULL;
    pBitmap1=NULL;
    pBitmapSrc=NULL;
    pBitmapL2=NULL;
    pSizedBitmap=NULL;
    pSizedBitmap2=NULL;
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
  std::ofstream *logFile;
  std::string errMsg;
  std::string mOutputFile;
  std::string mOutputDir;
  int64_t mBaseTotalWidth, mBaseTotalHeight;
  bool mValidObject;
  int mOutputType;
  bool mBlendTopLevel, mBlendByRegion;
  int mBaseLevel;
  bool mIncludeZStack;
  bool mCenter;
  int mQuality;
  int mStep, mZSteps;
  int mLastZLevel, mLastDirection;
  int mTopOutLevel;
  int64_t mMaxSide;
  int mDebugLevel;
  cv::Mat *mpImageL2;
public:
  #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  static const char mPathSeparator='\\';
  #else
  static const char mPathSeparator='/';
  #endif
public:
  SlideConvertor();
  ~SlideConvertor() { closeRelated(); }
  void closeRelated();
  std::string getErrMsg() { return errMsg; }
  int open(std::string inputFile, std::string outputFile, bool blendTopLevel, bool blendByRegion, bool markOutline, bool includeZStack, int quality, int64_t bestXOffset = -1, int64_t bestYOffset = -1, int outputType = 0, int debugLevel = 0);
  bool my_mkdir(std::string name);
  void calcCenters(int outLevel, int64_t& xCenter, int64_t& yCenter);
  int convert();
  int outputLevel(int olympusLevel, int magnification, int outLevel, bool tiled, int64_t readWidthL2, int64_t readHeightL2, BYTE *pBitmapL2);
  int checkFullL2(int64_t *pReadWidthL2, int64_t *pReadHeightL2, BYTE **pFullL2);
  int convert2Tif();
  int convert2Gmap();
  void tileCleanup(SlideLevel &l);
  void blendL2WithSrc(SlideLevel &l);
  void processSrcTile(SlideLevel& l);
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
  logFile = 0;
  mOutputType = 0;
  mCenter = false;
  mValidObject = false;
  mStep=0;
  mLastZLevel=-1;
  mLastDirection=-1;
  mZSteps=0;
  mBaseTotalWidth=0;
  mBaseTotalHeight=0;
  mBaseLevel=0;
  mIncludeZStack=true;
  mQuality=90;
  mpImageL2=NULL;
}


void SlideConvertor::calcCenters(int outLevel, int64_t &xCenter, int64_t &yCenter)
{
  int64_t baseWidth = slide->getActualWidth(mBaseLevel);
  int64_t baseHeight = slide->getActualHeight(mBaseLevel);
  int64_t gmapMaxSide = (1 << mTopOutLevel) * 256;
  //int64_t gmapMaxSide = pow(2.0, mTopOutLevel) * 256;
  int64_t xBaseCenter = (gmapMaxSide - baseWidth) / 2;
  int64_t yBaseCenter = (gmapMaxSide - baseHeight) / 2;
  
  int64_t xTopCenter = xBaseCenter >> mTopOutLevel;
  xCenter = xTopCenter << outLevel;

  int64_t yTopCenter = yBaseCenter >> mTopOutLevel;
  yCenter = yTopCenter << outLevel;
}



void SlideConvertor::tileCleanup(SlideLevel &l)
{
  if (l.pBitmap1)
  {
    delete[] l.pBitmap1;
    l.pBitmap1 = 0;
  }
  if (l.pImgScaled)
  {
    l.pImgScaled->release();
    delete l.pImgScaled;
    l.pImgScaled = 0;
  }
  if (l.pImgScaledL2Mini)
  {
    l.pImgScaledL2Mini->release();
    delete l.pImgScaledL2Mini;
    l.pImgScaledL2Mini = 0;
  }
  if (l.pSizedBitmap)
  {
    delete[] l.pSizedBitmap;
    l.pSizedBitmap = 0;
  }
  if (l.pSizedBitmap2)
  {
    delete[] l.pSizedBitmap2;
    l.pSizedBitmap2 = 0;
  }
}

// Scale the larger complete L2 image into a tiled smaller 
// mini version
// of it if L2 scaling is requested and the L2 pyramid level was
// read success
void SlideConvertor::blendL2WithSrc(SlideLevel &l)
{
  double xSrcStartL2=l.xSrc * l.xScaleL2;
  double xDestStartL2=0.0;
  double xSrcEndL2=xSrcStartL2 + l.grabWidthL2;
  if (xSrcStartL2 < 0.0)
  {
    xDestStartL2 = abs(xSrcStartL2);
    xSrcStartL2 = 0.0;
  }
  if (xSrcEndL2 > l.readWidthL2)
  {
    xSrcEndL2=l.readWidthL2;
  }
  double grabWidth2=xSrcEndL2 - xSrcStartL2;
  if (grabWidth2 < 0.0) grabWidth2 = 0.0;
  
  double ySrcStartL2=l.ySrc * l.yScaleL2;
  double yDestStartL2=0.0;
  double ySrcEndL2=ySrcStartL2 + l.grabHeightL2;
  if (ySrcStartL2 < 0.0)
  {
    yDestStartL2 = abs(ySrcStartL2);
    ySrcStartL2 = 0.0;
  }
  if (ySrcEndL2 > l.readHeightL2) 
  {
    ySrcEndL2=l.readHeightL2;
  }
  double grabHeight2=ySrcEndL2 - ySrcStartL2;
  if (grabHeight2 < 0.0) grabHeight2 = 0.0;
  
  int64_t rowSize = round(l.grabWidthL2) * 3;
  int64_t copySize = round(grabWidth2) * 3;
  int64_t tileSize = rowSize * round(l.grabHeightL2);
  BYTE *pBitmapL2Mini = new BYTE[tileSize];
  memset(pBitmapL2Mini, l.bkgColor, tileSize);
  int64_t xSrcStartL2Int = round(xSrcStartL2);
  int64_t ySrcStartL2Int = round(ySrcStartL2);
  int64_t xDestStartL2Int = round(xDestStartL2);
  int64_t yDestStartL2Int = round(yDestStartL2);
  int64_t grabHeightL2Int = round(l.grabHeightL2);
  if (floor(xSrcEndL2) > 0 && floor(ySrcEndL2) > 0)
  {
    for (int64_t row=0; ySrcStartL2Int+row < l.readHeightL2 && row+yDestStartL2Int < grabHeightL2Int; row++)
    {
      int64_t offsetL2x = (ySrcStartL2Int+row)*(l.readWidthL2*3) + (xSrcStartL2Int*3);
      int64_t offset3 = ((yDestStartL2Int+row) * rowSize) + (xDestStartL2Int*3);
      if (offset3 >= 0 && offset3 + copySize <= tileSize && offsetL2x >= 0 && offsetL2x < l.L2Size)
      {
        memcpy(&pBitmapL2Mini[offset3], &l.pBitmapL2[offsetL2x], copySize);
      }
    }
  }
  l.pImgScaledL2Mini = new cv::Mat;
  cv::Mat imgSrc(round(l.grabHeightL2), round(l.grabWidthL2), CV_8UC3, pBitmapL2Mini);
  cv::Size scaledSize(l.finalOutputWidth, l.finalOutputHeight);
  cv::resize(imgSrc, *l.pImgScaledL2Mini, scaledSize, l.xScaleResize, l.yScaleResize, l.scaleMethodL2);
  imgSrc.release();
  if (l.debugLevel > 0)
  {
    std::string errMsg;
    std::string l2TileName=l.tileName;
    l2TileName.append(".l2.jpg");
    bool writeOk=my_jpeg_write(l2TileName, l.pImgScaledL2Mini->data, l.finalOutputWidth, l.finalOutputHeight, l.quality, &errMsg);
    if (!writeOk) 
    {
      std::cout << "Error writing debug file '" << l2TileName << "' errMsg: " << errMsg << std::endl;
    }
  }
  if (mBlendByRegion)
  {
    slide->blendLevelsByRegion(l.pImgScaledL2Mini->data, l.pBitmapSrc, round(l.xSrc), round(l.ySrc), round(l.grabWidthA), round(l.grabHeightA), l.inputTileWidth, l.inputTileHeight, l.xScaleReverse, l.yScaleReverse, l.olympusLevel); 
    l.pBitmapFinal=l.pImgScaledL2Mini->data;
    l.writeOutputWidth = l.finalOutputWidth;
    l.writeOutputHeight = l.finalOutputHeight;
  }
  else
  {
    memset(l.pBitmap4, l.bkgColor, l.bitmap4Size);
    blendLevelsByBkgd(l.pBitmap4, l.pBitmapSrc, l.pImgScaledL2Mini->data, l.xDest, l.yDest, l.inputTileWidth, l.inputTileHeight, l.totalXSections / 2, l.xBkgdLimit, l.yBkgdLimit, l.xSubSections, l.totalXSections, l.ySubSections, l.totalYSections, l.bkgColor, true);
    l.pBitmapFinal = l.pBitmap4;
    l.writeOutputWidth = l.finalOutputWidth;
    l.writeOutputHeight = l.finalOutputHeight;
  }
  delete[] pBitmapL2Mini;
}


void SlideConvertor::processSrcTile(SlideLevel& l)
{
  l.pBitmapFinal = l.pBitmapSrc;
  l.writeOutputWidth = l.readWidth;
  l.writeOutputHeight = l.readHeight;
  // Check if read height and width returned from composite read
  // came back smaller than expected, resize the bitmap if so
  if (l.readWidth != l.grabWidthRead || l.readHeight != l.grabHeightRead)
  {
    int64_t sizedBitmapLength = l.grabWidthRead*l.grabHeightRead*3;
    l.pSizedBitmap = new BYTE[sizedBitmapLength];
    memset(l.pSizedBitmap, l.bkgColor, sizedBitmapLength);
    int copyWidth=l.readWidth;
    if (copyWidth > l.grabWidthRead) copyWidth=l.grabWidthRead;
    copyWidth *= 3;
    for (int64_t row=0; row < l.readHeight && row < l.grabHeightRead; row++)
    {
      memcpy(&l.pSizedBitmap[row*l.grabWidthRead*3], &l.pBitmapSrc[row*l.readWidth*3], copyWidth);
    }
    l.pBitmapSrc = l.pSizedBitmap;
    l.pBitmapFinal = l.pSizedBitmap;
    l.writeOutputWidth = l.grabWidthRead;
    l.writeOutputHeight = l.grabHeightRead;
  }
  // Check if the grabbed data needs to be scaled in or out
  // If we are at the very bottom of the image pyramid, this part will
  // be skipped because no scaling will be done
  if (l.grabWidthRead!=l.inputTileWidthRead || l.grabHeightRead!=l.inputTileHeightRead)
  {
    l.pImgScaled = new cv::Mat;
    cv::Mat imgSrc(l.grabHeightRead, l.grabWidthRead, CV_8UC3, l.pBitmapSrc);
    cv::Size scaledSize(l.inputTileWidthRead, l.inputTileHeightRead);
    cv::resize(imgSrc, *l.pImgScaled, scaledSize, l.xScaleReverse, l.yScaleReverse, l.scaleMethod);
    imgSrc.release();
    l.pBitmapSrc = l.pImgScaled->data;  
    l.pBitmapFinal = l.pImgScaled->data;
    l.writeOutputWidth = l.inputTileWidthRead;
    l.writeOutputHeight = l.inputTileHeightRead;
  }
  // Check if the current input tile is smaller than what is needed
  // to process the L2 background with it. This can occur
  // if the grabbed original composite source image has different
  // dimensions (cx,cy) than what is need because of a margin
  // that is need to center the image the side of the grabbed image
  if (l.inputTileWidthRead!=l.inputTileWidth || l.inputTileHeightRead!=l.inputTileHeight)
  {
    int64_t sizedBitmapLength2 = l.inputTileWidth*l.inputTileHeight*3;
    l.pSizedBitmap2 = new BYTE[sizedBitmapLength2];
    memset(l.pSizedBitmap2, l.bkgColor, sizedBitmapLength2);
    int copyWidth = l.inputTileWidthRead;
    int sizedRowLength = l.inputTileWidth * 3;
    int orgRowLength = l.inputTileWidthRead * 3;
    if (l.xMargin + copyWidth > l.inputTileWidth)
    {
      copyWidth -= (l.xMargin + copyWidth) -l.inputTileWidth;
    }
    copyWidth *= 3;
    int centeredOffset = l.xMargin * 3;
    for (int row=0; l.yMargin+row < l.inputTileHeight && row < l.inputTileHeightRead; row++)
    {
      memcpy(&l.pSizedBitmap2[((l.yMargin+row)*sizedRowLength)+centeredOffset], &l.pBitmapSrc[row * orgRowLength], copyWidth);
    }
    l.pBitmapSrc = l.pSizedBitmap2;
    l.pBitmapFinal = l.pSizedBitmap2;
    l.writeOutputWidth = l.inputTileWidth;
    l.writeOutputHeight = l.inputTileHeight;
  }          
  if (l.debugLevel > 0)
  {
    std::string preTileName = l.tileName;
    std::string errMsg;
    preTileName.append(".pre.jpg");
    bool writeOk=my_jpeg_write(preTileName, l.pBitmapSrc, l.writeOutputWidth, l.writeOutputHeight, l.quality, &errMsg);
    if (!writeOk)
    {
      std::cout << "Error writing debug file: " << errMsg << std::endl;
    }
  }
}


int SlideConvertor::outputLevel(int olympusLevel, int magnification, int outLevel, bool tiled, int64_t readWidthL2, int64_t readHeightL2, BYTE *pBitmapL2)
{
  std::ostringstream output;
  SlideLevel l;

  l.debugLevel = mDebugLevel;
  l.outputType = mOutputType;
  l.tiled = tiled;
  l.center = mCenter;
  l.olympusLevel = olympusLevel;
  l.magnifyX=magnification;
  l.magnifyY=magnification;
  l.outLevel=outLevel;
  l.readWidthL2 = readWidthL2;
  l.readHeightL2 = readHeightL2;
  l.pBitmapL2 = pBitmapL2;
  l.bkgColor=255;
  l.scaleMethod=cv::INTER_CUBIC;
  l.scaleMethodL2=cv::INTER_CUBIC;
  l.fillin = (mBlendTopLevel && l.olympusLevel < 2 && slide->checkLevel(2)) ? true : false;

  l.srcTotalWidth = slide->getActualWidth(olympusLevel);
  l.srcTotalHeight = slide->getActualHeight(olympusLevel);
  if (l.fillin)
  {
    l.srcTotalWidthL2 = slide->getActualWidth(2);
    l.srcTotalHeightL2 = slide->getActualHeight(2);
  }
  l.destTotalWidthDec = mBaseTotalWidth / l.magnifyX;
  l.destTotalHeightDec = mBaseTotalHeight / l.magnifyY;
  l.destTotalWidth = (int64_t) round(l.destTotalWidthDec);
  l.destTotalHeight = (int64_t) round(l.destTotalHeightDec);
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
      l.scaleMethodL2=cv::INTER_AREA;
    }
  }
  l.xScaleL2=(double) l.srcTotalWidthL2 / (double) l.srcTotalWidth;
  l.yScaleL2=(double) l.srcTotalHeightL2 / (double) l.srcTotalHeight;
  if (tiled)
  {
    l.finalOutputWidth=256;
    l.finalOutputHeight=256;
    l.inputTileWidth=256;
    l.inputTileHeight=256;
    l.grabWidthA=(double) l.inputTileWidth * l.xScale;
    l.grabHeightA=(double) l.inputTileHeight * l.yScale;
    l.grabWidthB=(double) l.finalOutputWidth * l.xScale;
    l.grabHeightB=(double) l.finalOutputHeight * l.yScale;
    l.grabWidthL2=256.0 * (double) l.srcTotalWidthL2 / (double) l.destTotalWidth;
    l.grabHeightL2=256.0 * (double) l.srcTotalHeightL2 / (double) l.destTotalHeight;
  }
  else
  {
    l.finalOutputWidth=l.destTotalWidth;
    l.finalOutputHeight=l.destTotalHeight;
    l.inputTileWidth=l.destTotalWidth;
    l.inputTileHeight=l.destTotalHeight;
    l.grabWidthA=l.srcTotalWidth;
    l.grabHeightA=l.srcTotalHeight;
    l.grabWidthB=l.srcTotalWidth;
    l.grabHeightB=l.srcTotalHeight;
    l.grabWidthL2=l.srcTotalWidthL2;
    l.grabHeightL2=l.srcTotalHeightL2;
  }
  if (l.readOkL2 && tiled && mBlendByRegion==false)
  {
    l.xBkgdLimit = (int) ceil((double) 752 / (double) l.xScale);
    l.yBkgdLimit = (int) ceil((double) 480 / (double) l.yScale);
    l.inputTileWidth=256+l.xBkgdLimit;
    l.inputTileHeight=256+l.yBkgdLimit;
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
      l.outputLvlTotalWidth = l.destTotalWidth;
      l.outputLvlTotalHeight = l.destTotalHeight;
    } 
  }
  if (l.xScaleReverse < 1.0 || l.yScaleReverse < 1.0)
  {
    l.scaleMethod=cv::INTER_AREA;
  }
  // Get the quality from the composite level (this does work as long as
  // the ini file specifies it (some ini files do, some don't)
  // Make sure the quality is at minimum the quality specific on the
  // command line
  l.quality=slide->getQuality(l.olympusLevel);
  if (l.quality<mQuality || l.quality<=0)
  {
    l.quality=mQuality;
  }

  if (l.outputType == OLYVSLIDE_GOOGLE)
  {
    output << "Google Maps Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    *logFile << output.str();
    std::cout << output.str();
  }
  else if (l.outputType == OLYVSLIDE_TIF)
  {
    output << "Tiff Level=" << l.outLevel << " Olympus Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    *logFile << output.str();
    std::cout << output.str();
   
    int totalMag=slide->getMagnification();
    if (totalMag<=0)
    {
      totalMag=40;
    }
    std::ostringstream oss;
    oss << "Aperio Image|AppMag=" << totalMag;
    if (slide->getTotalZLevels() > 1 && mZSteps == 1) 
    {
      oss << "|TotalDepth = " << slide->getTotalZLevels() << "\0";
    }
    else if (slide->getTotalZLevels() > 1 && mZSteps > 1)
    {
      oss << "|OffsetZ = " << (mZSteps-1) << "\0";
    }
    std::string strAttributes=oss.str();
    if (mTif->setAttributes(3, 8, l.destTotalWidth, l.destTotalHeight, (l.tiled==true ? l.finalOutputWidth : 0), (l.tiled==true ? l.finalOutputHeight : 0), 1, l.quality)==false || mTif->setDescription(strAttributes, mBaseTotalWidth, mBaseTotalHeight)==false)
    {
      std::string errMsg;
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to write tif attributes: " << errMsg << std::endl; 
      return 4;
    }
  }

  bool error=false;
  time_t timeStart=0, timeLast=0;
  try
  {
    if (l.readOkL2 && mBlendByRegion==false)
    {
      l.bitmap4Size = l.finalOutputWidth * l.finalOutputHeight * 3;
      l.pBitmap4 = new BYTE[l.bitmap4Size];
      l.totalXSections = (int64_t) (ceil((double) l.outputLvlTotalWidth / (double) l.finalOutputWidth)*l.finalOutputWidth) * 2;
      l.totalYSections = (int64_t) ceil((double) l.outputLvlTotalHeight / (double) l.finalOutputHeight)*l.finalOutputHeight;
      l.xSubSections=new int16_t[l.totalXSections];
      l.ySubSections=new int16_t[l.totalYSections];
      memset(l.xSubSections, 0, l.totalXSections*sizeof(int16_t));
      memset(l.ySubSections, 0, l.totalYSections*sizeof(int16_t));
    }
    *logFile << " xScale=" << l.xScale << " yScale=" << l.yScale;
    *logFile << " srcTotalWidth=" << l.srcTotalWidth << " srcTotalHeight=" << l.srcTotalHeight;
    *logFile << " destTotalWidth=" << l.destTotalWidth << " destTotalHeight=" << l.destTotalHeight;
    *logFile << std::endl;
    
    int perc=0, percOld=0;
    bool onePercHit=false;
    bool error=false;
    timeStart = time(NULL);
    l.ySrc=l.yStartSrc;
    
    retractCursor();
    std::cout << "0% done...    " << std::flush;

    // Keep looping until the current composite pyramid level is read
    // or the resulting output pyramid is done or on error
    while (round(l.ySrc) < l.srcTotalHeight && l.yDest < l.outputLvlTotalHeight && error==false)
    {
      std::ostringstream yRootStream;
      std::string dirPart1;
      std::string dirPart2;
      std::string yRoot;
      
      yRootStream << mOutputDir;
      dirPart1 = yRootStream.str();
      yRootStream << mPathSeparator << outLevel;
      dirPart2 = yRootStream.str();
      yRootStream << mPathSeparator << l.yTile;
      yRoot = yRootStream.str();
      if (l.outputType == OLYVSLIDE_GOOGLE || l.debugLevel > 0)
      {
        // Create the google maps directory structure up to the the y tile
        if (!my_mkdir(dirPart1) || !my_mkdir(dirPart2) || !my_mkdir(yRoot))
        {
          error = true;
          break;
        }
      }
      l.xDest = 0;
      l.xTile = l.xStartTile;
      // Loop until this cx block is complete or on error
      for (l.xSrc=l.xStartSrc; round(l.xSrc)<l.srcTotalWidth && l.xDest<l.outputLvlTotalWidth && error==false; l.xSrc += l.grabWidthB) 
      {
        bool writeOk=false;
        std::ostringstream tileNameStream;
        std::string errMsg;
        if (l.xSrc + l.grabWidthA < 1.0 || l.ySrc + l.grabHeightA < 1.0) continue;
        tileNameStream << yRoot << mPathSeparator << l.xTile << ".jpg";
        l.tileName = tileNameStream.str();
        l.xSrcRead = l.xSrc;
        l.ySrcRead = l.ySrc;
        double grabWidthReadDec = l.grabWidthA;
        double grabHeightReadDec = l.grabHeightA;
        l.inputTileWidthRead = l.inputTileWidth;
        l.inputTileHeightRead = l.inputTileHeight;
        l.xMargin = 0;
        l.yMargin = 0;
        if (l.xSrc < 0.0)
        {
          grabWidthReadDec=l.grabWidthA + l.xSrc;
          l.xSrcRead = 0.0;
          l.xMargin = l.xCenter;
        }
        if (l.ySrc < 0.0)
        {
          grabHeightReadDec=l.grabHeightA + l.ySrc;
          l.ySrcRead = 0.0;
          l.yMargin = l.yCenter;
        }
        if (l.xSrcRead + grabWidthReadDec > (double) l.srcTotalWidth)
        {
          grabWidthReadDec = (double) l.srcTotalWidth - l.xSrcRead;
          l.inputTileWidthRead = round(grabWidthReadDec * l.xScaleReverse);
        }
        if (l.ySrcRead + grabHeightReadDec > (double) l.srcTotalHeight)
        {
          grabHeightReadDec = (double) l.srcTotalHeight - l.ySrcRead;
          l.inputTileHeightRead = round(grabHeightReadDec * l.yScaleReverse);
        }
        if (l.xSrc < 0.0 || l.ySrc < 0.0)
        {
          l.inputTileWidthRead = round(grabWidthReadDec * l.xScaleReverse);
          l.inputTileHeightRead = round(grabHeightReadDec * l.yScaleReverse);
        }
        l.grabWidthRead = round(grabWidthReadDec);
        l.grabHeightRead = round(grabHeightReadDec);
        l.pBitmap1 = slide->allocate(l.olympusLevel, round(l.xSrcRead), round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false);
        l.pBitmapSrc = l.pBitmap1;
        
        l.pSizedBitmap = NULL;
        l.pSizedBitmap2 = NULL;
        l.pImgScaled = NULL;
        l.pImgScaledL2Mini = NULL;
        
        l.readWidth=0;
        l.readHeight=0;
        bool readOkSrc=false;
        if (l.pBitmapSrc)
        {
          if (l.debugLevel > 1)
          {
            *logFile << " slide->read(x=" << l.xSrcRead << " y=" << l.ySrcRead << " grabWidthA=" << l.grabWidthRead << " grabHeightA=" << l.grabHeightRead << " olympusLevel=" << l.olympusLevel << "); " << std::endl;
          }
          readOkSrc=slide->read(l.pBitmapSrc, l.olympusLevel, l.readDirection, l.readZLevel, round(l.xSrcRead), round(l.ySrcRead), l.grabWidthRead, l.grabHeightRead, false, &l.readWidth, &l.readHeight);
          if (l.debugLevel > 2)
          {
            std::cout << "readWidth: " << l.readWidth << " readHeight: " << l.readHeight<< " grabWidth: " << l.grabWidthRead << " grabHeight: " << l.grabHeightRead << std::endl;
          }
        }
        if (readOkSrc)
        {
          std::string errMsg;
          processSrcTile(l);
          if (l.readOkL2)
          {
            blendL2WithSrc(l);  
          }
          if (l.outputType == OLYVSLIDE_GOOGLE)
          {
            writeOk=my_jpeg_write(l.tileName, l.pBitmapFinal, l.writeOutputWidth, l.writeOutputHeight, l.quality, &errMsg);
          }
          else if (l.outputType == OLYVSLIDE_TIF)
          {
            if (l.tiled)
            {
              writeOk=mTif->writeEncodedTile(l.pBitmapFinal, l.xDest, l.yDest, 1);
            }
            else
            {
              writeOk=mTif->writeImage(l.pBitmapFinal);
            }
          }
          if (writeOk==false)
          {
            std::string errMsg;
            if (l.outputType == OLYVSLIDE_GOOGLE)
            {
              std::cerr << "Failed to write jpeg tile '" << l.tileName << "' reason: " << errMsg << std::endl;
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
        else
        {
          std::cerr << "Failed to read olympus level " << l.olympusLevel << " jpeg tile @ x=" << l.xSrcRead << " y=" << l.ySrcRead << " width=" << l.grabWidthRead << " height=" << l.grabHeightRead << std::endl;
        }
        tileCleanup(l);
        l.xDest += l.finalOutputWidth;
        l.xTile++;
      }
      l.yDest += l.finalOutputHeight;
      l.ySrc += l.grabHeightB;
      l.yTile++;
      perc=(int)(((double) l.yDest / (double) l.outputLvlTotalHeight) * 100);
      if (perc>100)
      {
        perc=100;
      }
      if (perc>percOld)
      {
        percOld=perc;
        retractCursor();
        std::cout << perc << "% done...    " << std::flush;
      }
      else if (onePercHit==false)
      {
        retractCursor();
        std::cout << perc << "% done...    " << std::flush;
        onePercHit = true;
      }
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
        *logFile << tifDirErrorMsg << errMsg << std::endl;
        error = true;
      }
    }
  }
  catch (std::bad_alloc& ba)
  {
    const char *msg = "Fatal Error: Failed to get memory. Cannot continue!";
    std::cout << msg << std::endl;
    *logFile << msg << std::endl;
    error = true;
  }
  if (l.pBitmap4)
  {
    delete[] l.pBitmap4;
    l.pBitmap4 = NULL;
  }
  timeLast = time(NULL);
  if (error==false)
  {
    std::cout << "Took " << timeLast - timeStart << " seconds for this level." << std::endl;
  }
  return (error==true ? 1 : 0); 
}


int SlideConvertor::checkFullL2(int64_t *pReadWidthL2, int64_t *pReadHeightL2, BYTE **pFullL2)
{
  *pFullL2 = NULL;
  *pReadWidthL2=0;
  *pReadHeightL2=0;
  if (!slide->checkLevel(2)) return 1;

  int64_t srcTotalWidthL2 = slide->getActualWidth(2);
  int64_t srcTotalHeightL2 = slide->getActualHeight(2);
  if (srcTotalWidthL2 <= 0 || srcTotalHeightL2 <= 0) return 1;
  if (!mpImageL2) return 2;
  if (mpImageL2->rows > 0 && mpImageL2->cols > 0 && mpImageL2->data) 
  {
    *pReadWidthL2 = mpImageL2->cols;
    *pReadHeightL2 = mpImageL2->rows;
    *pFullL2 = mpImageL2->data;
    return 0;
  }
  return 1;
}


int SlideConvertor::convert2Tif()
{
  int error = 0;
  BYTE* pFullL2Bitmap = 0; 
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
  for (; divisor != maxDivisor && error==0; step++)
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
        divisor = maxDivisor;
        break;
    }
    error=outputLevel(olympusLevel, divisor, step, tiled, readWidthL2, readHeightL2, pFullL2Bitmap);
  }
  if (error == 0 && step > 1)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  if (mpImageL2)
  {
    mpImageL2->release();
    delete mpImageL2;
    mpImageL2 = NULL;
  }
  return error;
}



int SlideConvertor::convert2Gmap()
{
  int error = 0;
  BYTE* pFullL2Bitmap = 0; 
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
  for (; outLevel <= mTopOutLevel && error==0; outLevel++)
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
    error=outputLevel(olympusLevel, divisor, outLevel, true, readWidthL2, readHeightL2, pFullL2Bitmap);
    divisor /= 2;
  }
  if (error==0 && outLevel > mTopOutLevel && outLevel > 0)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  if (mpImageL2)
  {
    mpImageL2->release();
    delete mpImageL2;
    mpImageL2 = NULL;
  }
  return error;
}


int SlideConvertor::open(std::string inputFile, std::string outputFile, bool blendTopLevel, bool blendByRegion, bool markOutline, bool includeZStack, int quality, int64_t bestXOffset, int64_t bestYOffset, int outputType, int debugLevel)
{
  mValidObject = false;
  mDebugLevel = debugLevel;
  closeRelated();
  logFile = new std::ofstream("olyvslideconv.log");
  slide = new CompositeSlide();
  errMsg="";
  if (slide->open(inputFile.c_str(), false, markOutline, bestXOffset, bestYOffset, &mpImageL2)==false)
  {
    return 1;
  }
  mOutputFile = outputFile;
  mOutputDir = outputFile;
  mOutputType = outputType;
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
      mBaseTotalHeight = slide->getActualHeight(mBaseLevel);
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
    mpImageL2->release();
    delete mpImageL2;
    mpImageL2 = NULL;
  }
  if (mTif)
  {
    mTif->close();
    delete mTif;
    mTif = NULL;
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
  bool blendTopLevel = true;
  bool blendByRegion = true;
  bool doBorderHighlight = false;
  bool includeZStack = false;
  static int outputType = 0;
  int quality = 90;
  int debugLevel = 0;
  char syntax[] = "syntax: olyvslideconv -b=[on,off] -d=[0-3] -h=[on,off] -r=[on,off] -x=[bestXOffset] -y=[bestYOffset] -z=[on,off] [--tiff, --google] <inputfolder> <outputfile> \nFlags:\t--tiff output to tif file.\n\t--google output to google maps format.\n\t-b blend the top level with the middle level. Default on.\n\t-d Debug mode, output debugging info and files. Default 0. The higher the more debugging output.\n\t-r Blend the top level with the middle level by region, not by empty background, default on.\n\t-h highlight visible areas with a black border. Default off.\n\t-q Set minimal jpeg quality percentage. Default 90.\n\t-x and -y Optional: set best X, Y offset of image if upper and lower pyramid levels are not aligned.\n\t-z Process Z-stack if the image has one. Default off.\n";

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
      {"google",      no_argument,        &outputType,  OLYVSLIDE_GOOGLE},
      {"tiff",        no_argument,        &outputType,  OLYVSLIDE_TIF},
      {"blend",       required_argument,  0,             'b'},
      {"debug",       required_argument,  0,             'd'},
      {"highlight",   required_argument,  0,             'h'},
      {"region",      required_argument,  0,             'r'},
      {"quality",     required_argument,  0,             'q'},
      {"xoffset",     required_argument,  0,             'x'},
      {"yoffset",     required_argument,  0,             'y'},
      {"zstack",      required_argument,  0,             'z'},
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
      case 'r':
        blendByRegion = getBoolOpt(optarg, invalidOpt);
        break;
      case 'q':
        quality = getIntOpt(optarg, invalidOpt);
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
  if (infile.length() == 0 || outfile.length() == 0)
  {
    std::cerr << syntax;
    return 1;
  }
  if (outputType==OLYVSLIDE_GOOGLE)
  {
    std::cout << "Output format: Google Maps" << std::endl;
  }
  else if (outputType==OLYVSLIDE_TIF)
  {
    std::cout << "Output format: TIFF/SVS" << std::endl;
  }
  else
  {
    std::cerr << "No output format specified. Please use --tiff or --google to specify tiff file or google maps directory output." << std::endl;
    std::cerr << syntax;
    return 1;
  }
  
  std::cout << "Set debug level: " << debugLevel << std::endl;
  std::cout << "Set quality: " << quality << std::endl;
  std::cout << "Set blend top level: " << bool2txt(blendTopLevel) << std::endl;
  std::cout << "Set border highlight: " << bool2txt(doBorderHighlight) << std::endl;
  std::cout << "Set blend by region: " << bool2txt(blendByRegion) << std::endl;
  if (xOffsetSet) 
  {
    std::cout << "Set bestXOffset: " << bestXOffset << std::endl;
  }
  else
  {
    std::cout << "Set bestXOffset: default (calculated with opencv)" << std::endl;
  }
  if (yOffsetSet)
  {
    std::cout << "Set bestYOffset: " << bestYOffset << std::endl;
  }
  else
  {
    std::cout << "Set bestYOffset: default (calculated with opencv)" << std::endl;
  }

  error=slideConv.open(infile.c_str(), outfile.c_str(), blendTopLevel, blendByRegion, doBorderHighlight, includeZStack, quality, bestXOffset, bestYOffset, outputType, debugLevel);
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
  return error;
}

