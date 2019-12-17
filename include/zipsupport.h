#ifndef __ZIP_SUPPORT_H
#define __ZIP_SUPPORT_H
#include <cstdint>
#include <minizip/zip.h>
#include <vector>
#include <string>

#ifndef BYTE
#define BYTE unsigned char
#endif

class ZipFile
{
protected:
  std::string mOutputFile;
  void *mZipArchive;
  std::string mErrMsg;
  int mCompressMethod;
  int mCompressFlags;
  int64_t mCheckProcessed;
  int64_t mTotalProcessed;
  int64_t mMaxBytes;
  std::vector<std::string> mDirNames;
public:
  ~ZipFile() { closeArchive(); }
  static const char mZipPathSeparator = '/';
  int openArchive(std::string filename, int flags);
  void setCompression(int method, int flags);
  int flushArchive();
  int closeArchive();
  int addFile(std::string filename, BYTE* buff, int64_t size);
  int addDir(std::string name);
  const char* getErrorMsg();
};

#endif
