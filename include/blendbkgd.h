#if !defined(BLENDBKGD_H)
#define BLENDBKGD_H

class BlendSection
{
protected:
  int64_t start, free;
  BlendSection *next;
public:
  BlendSection()
  {
    this->start = 0;
    this->free = 0;
    this->next = NULL;
  }
  BlendSection(int64_t value)
  {
    this->start = value;
    this->free = 0;
    this->next = NULL;
  }  
  inline void setStart(int64_t value) { this->start = value; }
  inline void clearFree() { this->free = 0; }
  inline void incrementFree() { this->free++; }
  inline void setNext(BlendSection *value) { this->next = value; }
  inline int64_t getStart() { return this->start; }
  inline int64_t getFree() { return this->free; }
  inline BlendSection* getNext() { return this->next; }
  inline void setFree(int64_t free) { this->free = free; }
};

typedef struct 
{
  safeBmp* pSafeDest;
  safeBmp* pSafeSrcL2;
  double xSrc, ySrc;
  double grabWidthB, grabHeightB;
  double xFactor, yFactor;
  int64_t xMargin, yMargin;
  BlendSection **yFreeMap;
  int64_t ySize;
} BlendArgs;

void blendLevelsFree(BlendSection** yFreeMap, int64_t ySize);
void blendLevels(BlendArgs *args);

#endif

