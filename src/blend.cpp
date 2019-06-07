#include <iostream>
#include <cstring>
#include <vector>
#include "composite.h"

void CompositeSlide::blendLevelsByRegion(BYTE *pDest, BYTE *pSrc, int64_t x, int64_t y, int64_t width, int64_t height, int tileWidth, int tileHeight, double xScaleOut, double yScaleOut, int srcLevel)
{
  if (checkLevel(srcLevel)==false) return;
  IniConf *pLowerConf=mConf[srcLevel];
  int64_t fileWidth=pLowerConf->mpixelWidth;
  int64_t fileHeight=pLowerConf->mpixelHeight;
  int64_t tileSize = tileWidth * tileHeight * 3;

  for (int64_t tileNum=0; tileNum<pLowerConf->mtotalTiles; tileNum++)
  {
    int64_t xCurrentPos=(int64_t) pLowerConf->mxyArr[tileNum].mxPixel;
    int64_t yCurrentPos=(int64_t) pLowerConf->mxyArr[tileNum].myPixel;
    // first check if the x y coordinates are within the region of the bitmap
    if (((x<xCurrentPos && x+width>xCurrentPos) || (x>=xCurrentPos && x<xCurrentPos+fileWidth)) &&
       ((y<yCurrentPos && y+height>yCurrentPos) || (y>=yCurrentPos && y<yCurrentPos+fileHeight)))
    {
      int64_t y2=yCurrentPos;
      if (y2 < y)
      {
        y2 = y;
      }
      y2 = y2 - y;
      int64_t y3=yCurrentPos + fileHeight;
      if (y3 > y+height)
      {
        y3 = y + height;
      }
      y3 = y3 - y;
      int64_t x2=xCurrentPos;
      if (x2 < x)
      {
        x2 = x;
      }
      x2 = x2 - x;
      int64_t x3=xCurrentPos + fileWidth;
      if (x3 > x+width)
      {
        x3 = x + width;
      }
      x3 = x3 - x;
      int64_t yMax = (int64_t) ceil((double) y3 * yScaleOut);
      if (yMax > tileHeight) 
      {
        yMax=tileHeight;
      }
      int64_t offset = (int64_t) floor((double) x2 * xScaleOut) * 3;
      int64_t rowSize2 = (int64_t) ceil((double) (x3-x2) * xScaleOut) * 3;
      if (offset > tileWidth * 3)
      {
        continue;
      }
      if (offset + rowSize2 > tileWidth*3)
      {
        rowSize2 = (tileWidth * 3) - offset;
      }
      for (int64_t y4 = (int64_t) round(y2 * yScaleOut); y4 < yMax; y4++)
      {
        int64_t offset2=(y4 * tileWidth * 3)+offset;
        if (offset2 + rowSize2 <= tileSize)
        {
          memcpy(&pDest[offset2], &pSrc[offset2], rowSize2);
        }
      }
    }
  }
}


void blendLevelsByBkgd(BYTE *pDest, BYTE *pSrc, BYTE *pSrcL2, int64_t x, int64_t y, int tileWidth, int tileHeight, int64_t rowWidth, int16_t xLimit, int16_t yLimit, int16_t *xFreeMap, int64_t totalXMap, int16_t *yFreeMap, int64_t totalYMap, BYTE bkgdColor, bool tiled)
{
  int destTileWidth=tileWidth;
  if (tiled) destTileWidth -= xLimit;
  int destTileHeight=tileHeight;
  if (tiled) destTileHeight -= yLimit;
  if (x+destTileWidth <= 0 || y+destTileHeight <= 0 || x > totalXMap || y > totalYMap || destTileWidth <= 0 || tileWidth <= 0 || destTileHeight <= 0 || tileHeight <= 0 || xLimit < 0 || yLimit < 0) 
  {
    return;
  }
  int64_t totalXMapDiv = totalXMap / 2;
  int srcRowSize = tileWidth * 3;
  int destRowSize = destTileWidth * 3;
//  int srcSize = srcRowSize * tileHeight;
  int destSize = destRowSize * destTileHeight;
//  BYTE *pSrcEnd = pSrc + srcSize;
  BYTE *pDestEnd = pDest + destSize;
  int64_t y2 = 0;
  int16_t yFree = 0;
  while (y+y2 < 0 && y2 < tileHeight)
  {
    y2++;
  }
  while (y2 < tileHeight)
  {
    int64_t x2 = 0;
    BYTE *pSrcB = &pSrc[y2 * srcRowSize];
    BYTE *pDestB = &pDest[y2 * destRowSize];
    BYTE *pSrcL2B = &pSrcL2[y2 * destRowSize];
    int16_t xFree = 0;
    xFree = yFreeMap[y+y2];
    while (x+x2 < 0 && x2 < tileWidth)
    {
      pSrcB += 3;
      pDestB += 3;
      pSrcL2B += 3;
      x2++;
    }
    while (x2 < tileWidth && x+x2 < totalXMapDiv)
    {
      bool setFree = false;
      if (y2 < destTileHeight) 
      {
        yFree = xFreeMap[x+x2];
      }
      else
      {
        yFree = xFreeMap[rowWidth+x+x2];
      }
      if (*pSrcB >= bkgdColor && pSrcB[1] >= bkgdColor && pSrcB[2] >= bkgdColor)
      {
        xFree++;
        if (xFree > tileWidth) 
        {
          xFree = tileWidth;
        }
        yFree++;
        if (yFree > tileHeight) 
        {
          yFree = tileHeight;
        }
        if (x2+1 == tileWidth && y2 < destTileHeight && 
           (xFree >= xLimit || yFree >= yLimit))
        {
          int backX = xFree-1;
          if (x2-backX<0)
          {
            backX = x2;
          }
          int copyX = backX+1;
          if (x2 > destTileWidth)
          {
            copyX = (destTileWidth - (x2 - backX))+1;
          }
          if (copyX<=0)
          {
            backX=0;
            copyX=0;
          }
          BYTE *pDestC = pDestB - (backX * 3);
          BYTE *pDestD = pDestC + (copyX * 3);
          BYTE *pSrcL2C = pSrcL2B - (backX * 3);
          while (pDestC < pDestD && pDestC < pDestEnd)
          {
            *pDestC = *pSrcL2C;
            pDestC[1] = pSrcL2C[1];
            pDestC[2] = pSrcL2C[2];
            pDestC += 3;
            pSrcL2C += 3;
          }
          setFree = true;
        }
        if (y2+1 == tileHeight && x2 < destTileWidth &&
           (xFree >= xLimit || yFree >= yLimit))
        {
          int backY = yFree-1;
          if (y2-backY<0)
          {
            backY = y2;
          }
          int copyY = backY+1;
          if (y2 > destTileHeight)
          {
            copyY = (destTileHeight - (y2 - backY))+1;
          }
          if (copyY<=0)
          {
            backY=0;
            copyY=0;
          }
          BYTE *pDestC = pDestB - (backY * destRowSize);
          BYTE *pDestD = pDestC + (copyY * destRowSize);
          BYTE *pSrcL2C = pSrcL2B - (backY * destRowSize);
          while (pDestC < pDestD && pDestC < pDestEnd)
          {
            *pDestC = *pSrcL2C;
            pDestC[1] = pSrcL2C[1];
            pDestC[2] = pSrcL2C[2];
            pDestC += destRowSize;
            pSrcL2C += destRowSize;
          }
          setFree = true;
        }
      }
      else if (xFree >= xLimit || yFree >= yLimit)
      {
        if (x2-xFree < destTileWidth && y2 < destTileHeight)
        {
          int backX = xFree;
          if (x2-backX<0)
          {
            backX = x2;
          }
          int copyX = backX;
          if (x2 > destTileWidth)
          {
            copyX = destTileWidth - (x2 - backX);
          }
          if (copyX<=0)
          {
            backX=0;
            copyX=0;
          }
          BYTE *pDestC = pDestB - (backX * 3);
          BYTE *pDestD = pDestC + (copyX * 3);
          BYTE *pSrcL2C = pSrcL2B - (backX * 3);
          while (pDestC < pDestD && pDestC < pDestEnd)
          {
            *pDestC = *pSrcL2C;
            pDestC[1] = pSrcL2C[1];
            pDestC[2] = pSrcL2C[2];
            pDestC += 3;
            pSrcL2C += 3;
          }
        }
        if (x2 < destTileWidth && y2-yFree < destTileHeight)
        {
          int backY = yFree;
          if (y2-yFree<0)
          {
            backY = y2;
          }
          int copyY = backY;
          if (y2 > destTileHeight)
          {
            copyY = destTileHeight - (y2 - backY);
          }
          if (copyY<=0)
          {
            backY=0;
            copyY=0;
          }
          BYTE *pDestC = pDestB - (backY * destRowSize);
          BYTE *pDestD = pDestC + (copyY * destRowSize);
          BYTE *pSrcL2C = pSrcL2B - (backY * destRowSize);
          while (pDestC < pDestD && pDestC < pDestEnd)
          {
            *pDestC = *pSrcL2C;
            pDestC[1] = pSrcL2C[1];
            pDestC[2] = pSrcL2C[2];
            pDestC += destRowSize;
            pSrcL2C += destRowSize;
          }
        }
        xFree=0;
        yFree=0;
      }
      else
      {
        xFree=0;
        yFree=0;
      }
      if (setFree==false && x2 < destTileWidth && y2 < destTileHeight && pDestB < pDestEnd)
      {
        /*
        *pDestB = 0;
        pDestB[1] = 0;
        pDestB[2] = 255;
        */
        *pDestB = *pSrcB;
        pDestB[1] = pSrcB[1];
        pDestB[2] = pSrcB[2];
      }
      if (rowWidth+x+x2 < totalXMap)
      {
        if (y2+1 == destTileHeight)
        {
          xFreeMap[rowWidth+x+x2]=yFree;
        }
        if (y2 < destTileHeight) 
        {
          xFreeMap[x+x2]=yFree;
        }
        else
        {
          xFreeMap[rowWidth+x+x2]=yFree;
        }
        if (y+y2 < totalYMap && x2 < destTileWidth)
        {
          yFreeMap[y+y2]=xFree;
        }
      }
      pSrcB += 3;
      pDestB += 3;
      pSrcL2B += 3;
      x2++;
    }
    y2++;
  }
}


