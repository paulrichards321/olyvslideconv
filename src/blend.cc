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
            BlendSection *xNew = new BlendSection(x3+1);
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
            xTail->setStart(x3+1);
            xTail->setFree((tailEnd - x3) - 1);
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
  int64_t tileWidth = args->pSafeSrcL2->width;
  int64_t tileHeight = args->pSafeSrcL2->height;
  int64_t ySize = args->ySize;

  int64_t x = args->x;
  int64_t y = args->y;
  int64_t xSrc=0;
  int64_t ySrc=0;
  
  if (y < 0 && y+ySize < 0)
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

  double xFactor = args->xFactor;
  double yFactor = args->yFactor;
 
  int64_t xEndA = ceil((x + tileWidth) * xFactor);
  int64_t yEndA = ceil((y + tileHeight) * yFactor);
  if (yEndA > ySize) yEndA = ySize;
  int64_t xStartA = floor((x + xSrc) * xFactor);
  int64_t yStartA = floor((y + ySrc) * yFactor);
  int64_t xStartB, xFreeB, xEndB;
  BlendSection** yFreeMap = args->yFreeMap;
  while (yStartA < yEndA)
  {
    BlendSection *xTail = yFreeMap[yStartA];
    ySrc = floor(yStartA / yFactor) - y;
    while (xTail != NULL) 
    {
      xStartB = xTail->getStart();
      xFreeB = xTail->getFree();
      xEndB = xStartB + xFreeB;
      if (xStartB > xEndA) break;
      if (((xStartB <= xStartA && xEndB >= xStartA) || 
           (xStartB >= xStartA)))
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
        if (xCopy == 0 && xStartC < x+tileWidth && ceil(xEndB / xFactor) >= x) xCopy = 1;
        xSrc = xStartC - x;
        safeBmpCpy(args->pSafeDest, xSrc, ySrc, args->pSafeSrcL2, xSrc, ySrc, xCopy, 1); // copy just one row
      }
      xTail = xTail->getNext();
    }
    yStartA++;
  }
}

