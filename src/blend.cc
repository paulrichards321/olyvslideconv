#include <iostream>
#include <cstring>
#include <vector>
#include <assert.h>
#include "composite.h"
#include "blendbkgd.h"

void CompositeSlide::blendLevelsByRegion(safeBmp* pSafeDest, safeBmp* pSafeSrc, int64_t x, int64_t y, double xScaleOut, double yScaleOut, int srcLevel)
{
  if (checkLevel(srcLevel)==false) return;
  IniConf *pLowerConf=mConf[srcLevel];
  int64_t fileWidth=pLowerConf->mPixelWidth;
  int64_t fileHeight=pLowerConf->mPixelHeight;
  int64_t tileWidth=pSafeDest->width;
  int64_t tileHeight=pSafeDest->height;

  if (pSafeSrc->width < tileWidth)
    tileWidth = pSafeSrc->width;
  if (pSafeSrc->height < tileHeight)
    tileHeight = pSafeSrc->height;
    
  for (int64_t tileNum=0; tileNum<pLowerConf->mTotalTiles; tileNum++)
  {
    int64_t xCurrentPos=(int64_t) pLowerConf->mxyArr[tileNum].mxPixel;
    int64_t yCurrentPos=(int64_t) pLowerConf->mxyArr[tileNum].myPixel;
    // first check if the x y coordinates are within the region of the bitmap
    if (((x<xCurrentPos && x+tileWidth>xCurrentPos) || (x>=xCurrentPos && x<xCurrentPos+fileWidth)) &&
       ((y<yCurrentPos && y+tileHeight>yCurrentPos) || (y>=yCurrentPos && y<yCurrentPos+fileHeight)))
    {
      int64_t y2=yCurrentPos;
      if (y2 < y)
      {
        y2 = y;
      }
      y2 = y2 - y;
      int64_t y3=yCurrentPos + fileHeight;
      if (y3 > y+tileHeight)
      {
        y3 = y + tileHeight;
      }
      y3 = y3 - y;
      int64_t x2=xCurrentPos;
      if (x2 < x)
      {
        x2 = x;
      }
      x2 = x2 - x;
      int64_t x3=xCurrentPos + fileWidth;
      if (x3 > x+tileWidth)
      {
        x3 = x + tileWidth;
      }
      x3 = x3 - x;
      int64_t yCopy = (int64_t) ceil((double) y3 * yScaleOut);
      if (yCopy > tileHeight) 
      {
        yCopy=tileHeight;
      }
      int64_t xOffset = (int64_t) floor((double) x2 * xScaleOut);
      int64_t xCopy = (int64_t) ceil((double) (x3-x2) * xScaleOut);
      if (xOffset > tileWidth)
      {
        continue;
      }
      if (xOffset + xCopy > tileWidth)
      {
        xCopy = tileWidth - xOffset;
      }
      int64_t yOffset = (int64_t) round(y2 * yScaleOut);
      safeBmpCpy(pSafeDest, xOffset, yOffset, pSafeSrc, xOffset, yOffset, xCopy, yCopy);

      /*
      for (int64_t y4 = (int64_t) round(y2 * yScaleOut); y4 < yMax; y4++)
      {
        int64_t offset2=(y4 * tileWidth * 3)+offset;
        if (offset2 + rowSize2 <= tileSize)
        {
          memcpy(&pDest[offset2], &pSrc[offset2], rowSize2);
        }
      }*/
    }
  }
}


void blendLevelsScan(BlendBkgdArgs* args)
{
  BYTE* pSrc = args->pSafeSrc->data;
  int64_t tileWidth = args->pSafeSrc->width;
  int64_t tileHeight = args->pSafeSrc->height;
  int64_t x = args->x;
  int64_t y = args->y;
  int64_t ySrc = 0;
  int64_t xSrc = 0;
  if (y < 0 && abs(y) > tileHeight)
  {
    return;
  }
  else if (y < 0)
  {
    ySrc = abs(y);
  }
  if (ySrc + y > args->ySize) return;

  if (x < 0 && abs(x) > tileWidth)
  {
    return;
  }
  else if (x < 0)
  {
    xSrc = abs(x);
  }
  if (xSrc + x > args->xSize) return;
  
  int64_t srcRowSize = args->pSafeSrc->strideWidth; 
  unsigned char bkgdColor = args->bkgdColor;
  int xLimit = args->xLimit;
  int yLimit = args->yLimit;
  BlendSection **xFreeMap = args->xFreeMap;
  BlendSection **yFreeMap = args->yFreeMap;
  int64_t ySize = args->ySize;
  int64_t xSize = args->xSize;
  while (ySrc < tileHeight && y+ySrc < ySize)
  {
    xSrc = 0;
    if (x < 0)
    {
      xSrc = abs(x);
    }
    BYTE *pSrcB = &pSrc[(ySrc * srcRowSize) + (xSrc * 3)];
    BlendSection *xTail = yFreeMap[y+ySrc];
    if (xTail == NULL)
    {
      xTail = new BlendSection(0);
      yFreeMap[y+ySrc] = xTail;
    }
    while (xSrc < tileWidth && x+xSrc < xSize)
    {
      BlendSection *yTail = xFreeMap[x+xSrc];
      if (yTail == NULL)
      {
        yTail = new BlendSection(0);
        xFreeMap[x+xSrc] = yTail;
      }
      if (*pSrcB >= bkgdColor && pSrcB[1] >= bkgdColor && pSrcB[2] >= bkgdColor)
      {
        if (xTail->getFree() == 0)
        {
          xTail->setStart(x+xSrc);
        }
        xTail->incrementFree();
        if (yTail->getFree() == 0)
        {
          yTail->setStart(y+ySrc);
        }
        yTail->incrementFree();
      }
      else
      {
        if (xTail->getFree() >= xLimit)
        {
          BlendSection* xLast = xTail;
          xTail = new BlendSection(xSrc + x);
          xTail->setPrevious(xLast);
          xLast->setNext(xTail);
          yFreeMap[y+ySrc] = xTail;
        }
        else
        {
          xTail->clearFree();
        }
        if (yTail->getFree() >= yLimit)
        {
          BlendSection* yLast = yTail;
          yTail = new BlendSection(ySrc + y);
          yTail->setPrevious(yLast);
          yLast->setNext(yTail);
          xFreeMap[x+xSrc] = yTail;
        }
        else
        {
          yTail->clearFree();
        }
      }
      xSrc++;
      pSrcB += 3;
    }
    ySrc++;
  }
}


void blendLevelsFree(BlendSection** xFreeMap, int64_t xSize, BlendSection** yFreeMap, int64_t ySize)
{
  BlendSection *tail=NULL;
  BlendSection *tailOld=NULL;
  // free all the y and x pointers
  for (int i=0; i < xSize; i++)
  {
    tail = xFreeMap[i];
    while (tail != NULL)
    {
      tailOld = tail;
      tail = tail->getPrevious();
      delete tailOld;
    }
  }
  for (int i=0; i < ySize; i++)
  {
    tail = yFreeMap[i];
    while (tail != NULL)
    {
      tailOld = tail;
      tail = tail->getPrevious();
      delete tailOld;
    }
  }
}


void blendLevelsByBkgd(BlendBkgdArgs *args)
{
  int64_t tileWidth = args->pSafeSrc->width;
  int64_t tileHeight = args->pSafeSrc->height;
  int64_t xSize = args->xSize;
  int64_t ySize = args->ySize;

  int64_t x = args->x;
  int64_t y = args->y;
  int64_t xSrc=0;
  int64_t ySrc=0;
  
  if (y < 0 && abs(y) > tileHeight)
  {
    return;
  }
  else if (y < 0)
  {
    ySrc = abs(y);
  }
  if (ySrc + y > ySize) return;

  if (x < 0 && abs(x) > tileWidth)
  {
    return;
  }
  else if (x < 0)
  {
    xSrc = abs(x);
  }
  if (xSrc + x > xSize) return;

  double xFactor = args->xFactor;
  double yFactor = args->yFactor;
 
  int xLimit = args->xLimit;
  int yLimit = args->yLimit;
  int64_t xEndA = round((x + tileWidth) * xFactor);
  if (xEndA > xSize) xEndA = xSize;
  int64_t yEndA = round((y + tileHeight) * yFactor);
  if (yEndA > ySize) yEndA = ySize;
  int64_t xStartA = round((x + xSrc) * xFactor);
  int64_t yStartA = round((y + ySrc) * yFactor);
  int64_t xStartB, xFreeB, xEndB;
  BlendSection** yFreeMap = args->yFreeMap;
  BlendSection** xFreeMap = args->xFreeMap;
  while (yStartA < yEndA)
  {
    BlendSection *xTail = yFreeMap[yStartA];
    ySrc = floor(yStartA / yFactor) - y;
    while (xTail != NULL) 
    {
      xStartB = xTail->getStart();
      xFreeB = xTail->getFree();
      xEndB = xStartB + xFreeB;
      if (xFreeB >= xLimit && 
         ((xStartB <= xStartA && xEndB >= xStartA) || 
          (xStartB >= xStartA && xEndA >= xStartB)
         ))
      {
        int64_t xStartC = floor(xStartB / xFactor);
        if (xStartC < x)
        {
          xStartC = x;
        }
        else if (xStartC > x+tileWidth)
        {
          xStartC = x+tileWidth;
        }
        int64_t xEndC = ceil(xEndB / xFactor);
        if (xEndC > x+tileWidth)
        {
          xEndC = x+tileWidth;
        }
        int64_t xCopy = xEndC - xStartC;
        if (xCopy == 0 && xStartC < x+tileWidth && floor(xEndB / xFactor) >= x) xCopy = 1;
        xSrc = xStartC - x;
        safeBmpCpy(args->pSafeDest, xSrc, ySrc, args->pSafeSrcL2, xSrc, ySrc, xCopy, 1); // copy just one row
      }
      xTail = xTail->getPrevious();
    }
    yStartA++;
  }

  ySrc = 0;
  if (y < 0)
  {
    ySrc = abs(y);
  }
  xSrc = 0;
  if (x < 0)
  {
    xSrc = abs(x);
  }
  yStartA = round((y + ySrc) * yFactor);
  xStartA = round((x + xSrc) * xFactor);
  int64_t yStartB, yFreeB, yEndB;
  while (xStartA < xEndA)
  {
    BlendSection *yTail = xFreeMap[xStartA];
    xSrc = floor(xStartA / xFactor) - x;
    while (yTail != NULL) 
    {
      yStartB = yTail->getStart();
      yFreeB = yTail->getFree();
      yEndB = yStartB + yFreeB;
      if (yFreeB >= yLimit && 
         ((yStartB <= yStartA && yEndB >= yStartA) || 
          (yStartB >= yStartA && yEndA >= yStartB)))
      {
        int64_t yStartC = floor(yStartB / yFactor);
        if (yStartC < y)
        {
          yStartC = y;
        }
        else if (yStartC > y+tileHeight)
        {
          yStartC = y+tileHeight;
        }
        int64_t yEndC = ceil(yEndB / yFactor);
        if (yEndC > y+tileHeight)
        {
          yEndC = y+tileHeight;
        }
        int64_t yCopy = yEndC - yStartC;
        if (yCopy == 0 && yStartC < y+tileHeight && floor(yEndB / yFactor) >= y) yCopy = 1;
        ySrc = yStartC - y;
        safeBmpCpy(args->pSafeDest, xSrc, ySrc, args->pSafeSrcL2, xSrc, ySrc, 1, yCopy); // copy just one vertical row
      }
      yTail = yTail->getPrevious();
    }
    xStartA++;
  }
}

