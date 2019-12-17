#include <iostream>
#include <cstring>
#include <vector>
#include "imagesupport.h"
#include "jpgcachesupport.h"
#include "composite.h"

extern JpgCache jpgCache;

bool CompositeSlide::read(BYTE *pBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, bool setGrayScale, int64_t *pReadWidth, int64_t *pReadHeight)
{
  *pReadWidth = 0;
  *pReadHeight = 0;
  if (checkZLevel(level, direction, zLevel)==false || checkLevel(level)==false)
  {
    return false;
  }
  int64_t actualWidth = mConf[level]->mtotalWidth;
  int64_t actualHeight = mConf[level]->mtotalHeight;
  if (x>actualWidth || y>actualHeight)
  {
    std::cerr << "Warning: in CompositeSlide::read: x or y out of bounds: x=" << x << " y=" << y << std::endl;
    return true;
  }
  if (width <= 0 || height <= 0)
  {
    std::cerr << "Warning: in CompositeSlide::read: width or height out of bounds: width=" << width << " height=" << height << std::endl;
    return true;
  } 
  if (x+width < 1 || y+width < 1)
  {
    return true;
  }
  int samplesPerPixel = 3;
  if (setGrayScale || mGrayScale)
  {
    samplesPerPixel = 1;
    setGrayScale = true;
  }  
  int64_t maxWidth=width;
  int64_t maxHeight=height;
  if (x+width>actualWidth)
  {
    maxWidth=actualWidth-x;
  }
  if (y+height>actualHeight)
  {
    maxHeight=actualHeight-y;
  }
 
  int64_t bmpSize=maxWidth*maxHeight*samplesPerPixel;
  IniConf* pConf=mConf[level];
  int64_t fileWidth=pConf->mpixelWidth;
  int64_t fileHeight=pConf->mpixelHeight;
  int64_t widthGrab=0, heightGrab=0;
  int64_t totalTilesRead=0;
  for (int64_t tileNum=0; tileNum<pConf->mtotalTiles; tileNum++)
  {
    if (zLevel > 0 && direction > 0 && pConf->mxyArr[tileNum].mzStack[direction-1][zLevel] == false) continue;
    int64_t xFilePos=pConf->mxyArr[tileNum].mxPixel;
    int64_t yFilePos=pConf->mxyArr[tileNum].myPixel;
    if (((x<xFilePos && x+maxWidth>xFilePos) || (x>=xFilePos && x<xFilePos+fileWidth)) &&
        ((y<yFilePos && y+maxHeight>yFilePos) || (y>=yFilePos && y<yFilePos+fileHeight)))
    {
      Jpg *pjpg;
      int64_t xRead=0;
      int64_t xWrite=xFilePos-x;
      widthGrab=(x+maxWidth)-xFilePos;
      if (xWrite<0)
      {
        xWrite=0;
        xRead=x-xFilePos;
        widthGrab=fileWidth-xRead;
        if (widthGrab>maxWidth)
        {
          widthGrab=maxWidth;
        }
      }
      int64_t yRead=0;
      int64_t yWrite=yFilePos-y;
      heightGrab=(y+maxHeight)-yFilePos;
      if (yWrite<0)
      {
        yWrite=0;
        yRead=y-yFilePos;
        heightGrab=fileHeight-yRead;
        if (heightGrab>maxHeight)
        {
          heightGrab=maxHeight;
        }
      }
      if (yRead+heightGrab>fileHeight)
      {
        heightGrab=fileHeight-yRead;
      }
      if (xRead+widthGrab>fileWidth)
      {
        widthGrab=fileWidth-xRead;
      }
      /*
      if (level==2 && y>= 1000)
      {
        std::cout << "Filename to open: " << pConf->mxyArr[tileNum].mBaseFileName << " xFilePos: " << xFilePos << " yFilePos: " << yFilePos << " widthGrab: " << widthGrab << " heightGrab: " << heightGrab << " xRead, xWrite: " << xRead << ", " << xWrite << " yRead, yWrite: " << yRead << ", " << yWrite << std::endl;
      }
      */
      std::string& fileName=(direction > 0 ? pConf->mxyArr[tileNum].mFileName[direction-1][zLevel] : pConf->mxyArr[tileNum].mBaseFileName);
      pjpg=jpgCache.open(fileName, setGrayScale);
      if (pjpg->isValidObject() && pjpg->read(xRead, yRead, widthGrab, heightGrab))
      {
        int64_t jpgCX=pjpg->getReadWidth();
        int64_t jpgCY=pjpg->getReadHeight();
        int jpgSamplesPerPixel=pjpg->getSamplesPerPixel();
        BYTE *jpgBitmap=pjpg->bitmapPointer();
        for (int64_t row=0; row<jpgCY; row++)
        {
      //    std::cout << "read bytes: " << parsedHeight+parsedWidth+(row*maxWidth*3) << std::endl;
      //    std::cout << "jpgBitmap=" << (long long) jpgBitmap << std::endl;
          int64_t desti=(yWrite*maxWidth*samplesPerPixel)+(xWrite*samplesPerPixel)+(row*maxWidth*samplesPerPixel);
          if (desti+(jpgCX*samplesPerPixel) > bmpSize)
          {
            std::cerr << "In CompositeSlide::read, pointer out of bounds: bmpSize=" << bmpSize << " desti=" << desti << std::endl;
          }
          else if (samplesPerPixel == jpgSamplesPerPixel && desti >= 0)
          {
            memcpy(&pBmp[desti], &jpgBitmap[row*jpgCX*jpgSamplesPerPixel], jpgCX*jpgSamplesPerPixel);
          }
          else if (samplesPerPixel == 3 && jpgSamplesPerPixel == 1)
          {
            // TODO Convert grayscale jpg to color bitmap if color bitmap was requested
            std::cerr << "Conversion from grayscale to color bitmap not supported yet!" << std::endl;

          }
        }
        totalTilesRead++;
        if (level==2 && mOptBorder)
        {
          drawBorder(pBmp, samplesPerPixel, x, y, maxWidth, maxHeight, level); 
        }
      }
      else
      {
        std::string errMsg;
        pjpg->getErrMsg(errMsg);
        std::cerr << "Warning: failed to read " << fileName << ": " << errMsg << std::endl;
        jpgCache.release(pjpg);
      }
    }
  }
  *pReadWidth=maxWidth;
  *pReadHeight=maxHeight;
  return true;
}



bool CompositeSlide::read(int64_t x, int64_t y, int64_t width, int64_t height, bool setGrayScale)
{
  return false;
}


bool CompositeSlide::allocate(safeBmp* pBmp, int level, int64_t x, int64_t y, int64_t width, int64_t height, bool setGrayScale)
{
  if (mValidObject==false || level<0 || level > (int64_t) mConf.size() || mConf[level]->mfound==false)
  {
    return false;
  }
  int64_t actualWidth=mConf[level]->mtotalWidth;
  int64_t actualHeight=mConf[level]->mtotalHeight;
  if (x>actualWidth || y>actualHeight)
  {
    std::cerr << "x or y out of bounds: x=" << x << " y=" << y;
    return false;
  }
  if (width <= 0 || height <= 0)
  {
    std::cerr << "width or height out of bounds: width=" << width << " height=" << height;
    return false;
  } 
  int samplesPerPixel = 3;
  if (setGrayScale || mGrayScale)
  {
    samplesPerPixel = 1;
  }  
  int64_t maxWidth=width;
  int64_t maxHeight=height;
  if (x+width>actualWidth)
  {
    maxWidth=actualWidth-x;
  }
  if (y+height>actualHeight)
  {
    maxHeight=actualHeight-y;
  }
 
  int64_t bmpSize=maxWidth*maxHeight*samplesPerPixel;
  if (bmpSize > 512 * 1024 * 1024)
  {
    std::cout << "allocating " << (bmpSize / (1024 * 1024)) << " megabytes in memory." << std::endl;
  }
  BYTE* data = safeBmpAlloc2(pBmp, maxWidth, maxHeight);
  return (data ? true : false);
}
 
