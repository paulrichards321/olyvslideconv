#ifndef SAFE_BMP
#define SAFE_BMP
#ifdef __cplusplus
#include <cstdint>
#else
#include <string.h>
#endif

#ifndef BYTE
#define BYTE uint8_t
#endif 

typedef struct 
{
  int64_t width;
  int64_t height;
  int64_t strideWidth;
  BYTE *data;
  bool freeData;
  bool freePtr;
} safeBmp;

safeBmp* safeBmpAlloc(int64_t width, int64_t height);
uint8_t* safeBmpAlloc2(safeBmp*, int64_t width, int64_t height);
void safeBmpInit(safeBmp *bmp, BYTE *data, int64_t width, int64_t height);
void safeBmpUint32Set(safeBmp *bmp, uint32_t value);
inline void safeBmpByteSet(safeBmp *bmp, int value) { memset(bmp->data, value, bmp->strideWidth * bmp->height); }
void safeBmpFree(safeBmp *bmp);
safeBmp* safeBmpSrc(BYTE* data, int64_t width, int64_t height);
void safeBmpCpy(safeBmp *bmpDest, int64_t x_dest, int64_t y_dest, safeBmp *bmp_src, int64_t x_src, int64_t y_src, int64_t cols, int64_t rows);
inline void safeBmpClear(safeBmp *bmp) { bmp->width = 0; bmp->height = 0; bmp->strideWidth = 0; bmp->data = 0; bmp->freeData = false; bmp->freePtr = false; }
void safeBmpBGRtoRGBCpy(safeBmp *bmpDest, safeBmp *bmpSrc);
#endif

