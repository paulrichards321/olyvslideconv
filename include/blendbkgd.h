#if !defined(BLENDBKGD_H)
#define BLENDBKGD_H

class BlendSection
{
protected:
  int64_t start, free;
  BlendSection *next;
public:
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
  inline int getStart() { return this->start; }
  inline int getFree() { return this->free; }
  inline BlendSection* getNext() { return this->next; }
  inline void setFree(int64_t free) { this->free = free; }
};

typedef struct 
{
  safeBmp* pSafeDest;
  safeBmp* pSafeSrcL2;
  int64_t x, y;
  double xFactor, yFactor;
  BlendSection **yFreeMap;
  int64_t ySize;
} BlendArgs;

void blendLevelsFree(BlendSection** yFreeMap, int64_t ySize);
void blendLevels(BlendArgs *args);

#endif

