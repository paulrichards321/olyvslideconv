#include <cassert>
#include <cstring>
#include <new>
#include <cstdlib>
#include <iostream>
#include "safebmp.h"

safeBmp * safeBmpAlloc(int64_t width, int64_t height)
{
  //int64_t stideWidth = cairoFormatStrideForWidth(CAIROFORMATRGB24, width);
  int64_t strideWidth = width * 3;
  safeBmp *bmp = new safeBmp;
  if (bmp == NULL)
  {
    return NULL;
  }
  bmp->height = height;
  bmp->width = width;
  bmp->strideWidth = strideWidth;
  try 
  {
    bmp->data = new BYTE[strideWidth * height];
  }
  catch (std::bad_alloc &xa)
  {
    bmp->data = NULL;
  }
  if (bmp->data == NULL)
  {
    std::cerr << "Failed to allocate image of memory size=" << ((strideWidth * height) / 1024) << " kb." << std::endl;
    exit(1);
  }
  bmp->freeData = true;
  bmp->freePtr = true;
  return bmp;
}


BYTE* safeBmpAlloc2(safeBmp *bmp, int64_t width, int64_t height)
{
  //int64_t stideWidth = cairoFormatStrideForWidth(CAIROFORMATRGB24, width);
  bmp->data = NULL;
  int64_t strideWidth = width * 3;
  bmp->strideWidth = strideWidth;
  bmp->height = height;
  bmp->width = width;
  try
  {
    bmp->data = new BYTE[strideWidth * height];
  }
  catch (std::bad_alloc &xa)
  {
    bmp->data = NULL;
  }
  if (bmp->data == NULL)
  {
    std::cerr << "Failed to allocate image of memory size=" << ((strideWidth * height) / 1024) << " kb." << std::endl;
    exit(1);
  }
  bmp->freeData = true;
  bmp->freePtr = false;
  return bmp->data;
}


safeBmp* safeBmpSrc(BYTE *data, int64_t width, int64_t height)
{
  safeBmp* bmp=new safeBmp;
  if (!bmp) return NULL;

  bmp->width = width;
  bmp->strideWidth = width * 3;
  bmp->height = height;
  bmp->data = data;
  bmp->freeData = false;
  bmp->freePtr = true;

  return bmp;
}


void safeBmpInit(safeBmp *bmp, BYTE *bmpPtr, int64_t width, int64_t height)
{
  bmp->width = width;
  bmp->strideWidth = width * 3;
  //  bmp.strideWidth = cairoFormatStrideForWidth(CAIROFORMATRGB24, width);
  bmp->height = height;
  bmp->data = bmpPtr;
  bmp->freeData = false;
  bmp->freePtr = false;
}


void safeBmpFree(safeBmp *bmp)
{
  if (!bmp) return;
  if (bmp->freeData && bmp->data)
  {
    delete[] bmp->data;
    bmp->data = NULL;
  }
  if (bmp->freePtr)
  {
    delete bmp;
  }
}


void safeBmpUint32Set(safeBmp *bmp, uint32_t value)
{
  uint64_t size=bmp->width * bmp->height;
  uint32_t *data = (uint32_t*) bmp->data;
  for (uint64_t i=0; i < size; i++) data[i] = value;
}


void safeBmpCpy(safeBmp *bmpDest, int64_t xDest, int64_t yDest, safeBmp *bmpSrc, int64_t xSrc, int64_t ySrc, int64_t cols, int64_t rows)
{
  //assert(bmpSrc->data);
  //assert(bmpDest->data);
  int64_t xEnd = xSrc + cols;
  if (xSrc < 0)
  {
    cols += xSrc;
    xSrc = 0;
  }
  if (xEnd < 0 || xSrc > bmpSrc->width || cols <= 0 ||
      xDest > bmpDest->width) 
  {
    return;
  }
  if (xEnd > bmpSrc->width)
  {
    cols = bmpSrc->width - xSrc;
    if (cols < 0) return;
  }
  int64_t yEnd = ySrc + rows;
  if (ySrc < 0)
  {
    rows += ySrc;
    ySrc = 0;
  }
  if (yEnd < 0 || ySrc > bmpSrc->height || rows <= 0 ||
      yDest > bmpDest->height) 
  {
    return;
  }
  if (yEnd > bmpSrc->height)
  {
    rows = bmpSrc->height - ySrc;
  }
  if (xDest < 0)
  {
    cols += xDest;
    xDest = 0;
  }
  if (xDest + cols > bmpDest->width)
  {
    cols = bmpDest->width - xDest;
  }
  if (yDest < 0)
  {
    rows += yDest;
    yDest = 0;
  }
  if (yDest + rows > bmpDest->height)
  {
    rows = bmpDest->height - yDest;
  }
  if (rows < 0 || cols <= 0) return;
  int64_t srcRowWidth = bmpSrc->strideWidth;
  int64_t destRowWidth = bmpDest->strideWidth;
  int64_t xSrcOffset = xSrc * 3;
  int64_t xDestOffset = xDest * 3;
  int64_t colCopySize=cols*3;
  BYTE *destData = (BYTE*) bmpDest->data;
  BYTE *srcData = (BYTE*) bmpSrc->data;
  for (int64_t y = 0; y < rows; y++)
  {
    int64_t src = ((y+ySrc) * srcRowWidth) + xSrcOffset;
    int64_t dest = ((y+yDest) * destRowWidth) + xDestOffset;
    memcpy(&destData[dest], &srcData[src], colCopySize);
  }
}


void safeBmpBGRtoRGBCpy(safeBmp *bmpDest, safeBmp *bmpSrc)
{
  int64_t xSrc = 0;
  int64_t ySrc = 0;
  int64_t xDest = 0;
  int64_t yDest = 0;
  int64_t cols = bmpSrc->width;
  int64_t rows = bmpSrc->height;

  int64_t xEnd = xSrc + cols;
  if (xSrc < 0)
  {
    cols += xSrc;
    xSrc = 0;
  }
  if (xEnd < 0 || xSrc > bmpSrc->width || cols <= 0 ||
      xDest > bmpDest->width) 
  {
    return;
  }
  if (xEnd > bmpSrc->width)
  {
    cols = bmpSrc->width - xSrc;
    if (cols < 0) return;
  }
  int64_t yEnd = ySrc + rows;
  if (ySrc < 0)
  {
    rows += ySrc;
    ySrc = 0;
  }
  if (yEnd < 0 || ySrc > bmpSrc->height || rows <= 0 ||
      yDest > bmpDest->height) 
  {
    return;
  }
  if (yEnd > bmpSrc->height)
  {
    rows = bmpSrc->height - ySrc;
  }
  if (xDest < 0)
  {
    cols += xDest;
    xDest = 0;
  }
  if (xDest + cols > bmpDest->width)
  {
    cols = bmpDest->width - xDest;
  }
  if (yDest < 0)
  {
    rows += yDest;
    yDest = 0;
  }
  if (yDest + rows > bmpDest->height)
  {
    rows = bmpDest->height - yDest;
  }
  if (rows < 0 || cols <= 0) return;
  int64_t srcRowWidth = bmpSrc->strideWidth;
  int64_t destRowWidth = bmpDest->strideWidth;
  int64_t xSrcOffset = xSrc * 3;
  int64_t xDestOffset = xDest * 3;
  int64_t colCopySize=cols*3;
  BYTE *destData = (BYTE*) bmpDest->data;
  BYTE *srcData = (BYTE*) bmpSrc->data;
  for (int64_t y = 0; y < rows; y++)
  {
    int64_t src = ((y+ySrc) * srcRowWidth) + xSrcOffset;
    int64_t dest = ((y+yDest) * destRowWidth) + xDestOffset;
    //memcpy(&destData[dest], &srcData[src], colCopySize);
    int64_t end = src + colCopySize;
    while (src < end)
    {
      // BGR to RGB
      destData[dest+2] = srcData[src];
      destData[dest+1] = srcData[src+1];
      destData[dest] = srcData[src+2];
      dest += 3;
      src += 3;
    }
  }
}

