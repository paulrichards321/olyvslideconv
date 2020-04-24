#include <iostream>
#include <cstring>
#include <vector>
#include <assert.h>
#include "composite.h"
#include "blendbkgd.h"

void CompositeSlide::blendLevelsRegionScan(BlendSection** yFreeMap, int64_t ySize) 
{
  if (yFreeMap == NULL) return;
  
  int bottomLevel = 0;
  while (bottomLevel < 4)
  {
    if (checkLevel(bottomLevel)) break;
    bottomLevel++;
  }
  IniConf *pBottomConf = mConf[bottomLevel];
  int64_t bottomTotalWidth = getActualWidth(bottomLevel);

  for (int64_t y=0; y < ySize; y++)
  {
    BlendSection *xTail = new BlendSection(0);
    xTail->setFree(bottomTotalWidth);
    yFreeMap[y] = xTail;
  }
 
  int64_t fileWidth=pBottomConf->mPixelWidth;
  int64_t fileHeight=pBottomConf->mPixelHeight;
  
  for (int64_t tileNum=0; tileNum<pBottomConf->mTotalTiles; tileNum++)
  {
    int64_t x2=pBottomConf->mxyArr[tileNum].mxPixel;
    int64_t y2=pBottomConf->mxyArr[tileNum].myPixel;
    int64_t x3=x2 + fileWidth;
    int64_t y3=y2 + fileHeight;
    // first check if the x y coordinates are within the region of the bitmap
    int64_t y = y2;
    while (y < y3)
    {
      BlendSection *xTail = yFreeMap[y];
      BlendSection *xNext = NULL;
      BlendSection *xPrevious = NULL;
      if (xTail == NULL)
      {
        xTail = new BlendSection(0);
        xTail->setFree(bottomTotalWidth);
        yFreeMap[y] = xTail;
      }
      while (xTail)
      {
        int64_t tailStart = xTail->getStart();
        int64_t tailLen = xTail->getFree();
        int64_t tailEnd = tailStart + tailLen;
        xNext = xTail->getNext();
        if (x2 > tailStart && x2 < tailEnd)
        {
          xTail->setFree(x2 - tailStart);
          if (x3 < tailEnd)
          {
            BlendSection *xNew = new BlendSection(x3);
            xNew->setFree(tailEnd - x3);
            xNew->setNext(xNext);
            xTail->setNext(xNew);
            xNext = xNew;
          }
        }
        else if (x2 < tailStart && x3 > tailStart)
        {
          if (x3 >= tailEnd)
          {
            delete xTail;
            xTail = NULL;
            if (xPrevious)
            {
              xPrevious->setNext(xNext);
              xTail = xPrevious;
            }
            else
            {
              yFreeMap[y] = xNext;
            }
          }
          else if (x3 < tailEnd)
          {
            xTail->setStart(x3);
            xTail->setFree(tailEnd - x3);
          }
        }
        xPrevious = xTail;
        xTail = xNext;
      }
      y++;
    } 
  }
}


void blendLevelsFree(BlendSection** yFreeMap, int64_t ySize)
{
  BlendSection *tail=NULL;
  BlendSection *tailOld=NULL;
  for (int64_t i=0; i < ySize; i++)
  {
    tail = yFreeMap[i];
    while (tail != NULL)
    {
      tailOld = tail;
      tail = tail->getNext();
      delete tailOld;
    }
  }
}


void blendLevels(BlendArgs *args)
{
  double xSrc = args->xSrc;
  double ySrc = args->ySrc;
  double grabWidthB = args->grabWidthB;
  double grabHeightB = args->grabHeightB;

  if (grabWidthB < 0 || grabHeightB < 0) return;

  double xFactor = args->xFactor;
  double yFactor = args->yFactor;
 
  int64_t xStartA = (int64_t) floor(xSrc);
  int64_t yStartA = (int64_t) floor(ySrc);
  int64_t xEndA = (int64_t) ceil(xSrc + grabWidthB);
  int64_t yEndA = (int64_t) ceil(ySrc + grabHeightB);
  if (yEndA > args->ySize) yEndA = args->ySize;
  
  int64_t xStartC = (int64_t) floor(xSrc / xFactor) - 1;
  int64_t yStartC = (int64_t) floor(ySrc / yFactor);
  
  BlendSection** yFreeMap = args->yFreeMap;
  for (int64_t y = yStartA; y < yEndA; y++)
  {
    BlendSection *xTail = yFreeMap[y];
    int64_t yStartD = (int64_t) floor((double) y / yFactor);
//    int64_t yStartD2 = (int64_t) ceil((double) y / yFactor);
    int64_t yDest = (yStartD - yStartC) + args->yMargin - 1;
    bool yIncrement = true;
    if (yDest < 0) 
    {
      yDest = 0;
      yIncrement = false;
    }
    while (xTail != NULL) 
    {
      int64_t xStartB = xTail->getStart();
      int64_t xFreeB = xTail->getFree();
      int64_t xEndB = xStartB + xFreeB;
      if (xStartB > xEndA) break;
      if (((xStartB <= xStartA && xEndB >= xStartA) || 
           (xStartB >= xStartA)))
      {
        if (xStartB < xStartA)
        {
          xStartB = xStartA;
        }
        int64_t xDest = (int64_t) floor(xStartB / xFactor);
//        int64_t xDest2 = (int64_t) ceil(xStartB / xFactor);
        int64_t xDestEnd = (int64_t) ceil(xEndB / xFactor);
        int64_t xCopy = xDestEnd - xDest;
        if (xCopy < 0)
        {
          xTail = xTail->getNext();
          continue;
        }
        xCopy++;
        int64_t yCopy = 2;
        if (yIncrement)
        {
          yCopy++;
        }
        xDest = (xDest - xStartC) + args->xMargin;
        if (xDest > 0)
        {
          xDest--;
          xCopy++;
        }
        safeBmpCpy(args->pSafeDest, xDest, yDest, args->pSafeSrcL2, xDest, yDest, xCopy, yCopy);
      }
      xTail = xTail->getNext();
    }
  }
}
