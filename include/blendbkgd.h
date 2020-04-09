#if !defined(BLENDBKGD_H)
#define BLENDBKGD_H

class BlendSection
{
protected:
  int64_t start, free;
  BlendSection *previous;
  BlendSection *next;
public:
  BlendSection(int64_t value)
  {
    this->start = value;
    this->free = 0;
    this->previous = NULL;
    this->next = NULL;
  }  
  inline void setStart(int64_t value) { this->start = value; }
  inline void clearFree() { this->free = 0; }
  inline void incrementFree() { this->free++; }
  inline void setNext(BlendSection *value) { this->next = value; }
  inline void setPrevious(BlendSection *value) { this->previous = value; }
  inline int getStart() { return this->start; }
  inline int getFree() { return this->free; }
  inline BlendSection* getNext() { return this->next; }
  inline BlendSection* getPrevious() { return this->previous; }
  inline void setFree(int64_t free) { this->free = free; }
};


typedef struct 
{
  safeBmp* pSafeDest;
  safeBmp* pSafeSrc;
  safeBmp* pSafeSrcL2;
  int64_t x, y;
  double xFactor, yFactor;
  BlendSection **xFreeMap, **yFreeMap;
  int64_t xSize;
  int64_t ySize;
  int xLimit, yLimit;
  BYTE bkgdColor;
  BYTE **rows; 
  int64_t tileWidth, tileHeight;
  int64_t baseWidth;
  int64_t bufferHeight;
  bool bufferFlipped;
  bool bufferFull;
} BlendBkgdArgs;

void blendLevelsScan(BlendBkgdArgs* args);
void blendLevelsByBkgd(BlendBkgdArgs *args);
void blendLevelsFree(BlendSection** xFreeMap, int64_t xSize, BlendSection** yFreeMap, int64_t ySize);
void blendLevelsPrune(BlendSection** xFreeMap, int64_t xSize, int64_t xLimit, BlendSection** yFreeMap, int64_t ySize, int64_t yLimit);

#endif

