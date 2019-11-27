#ifndef __ZIP_SUPPORT_H
#define __ZIP_SUPPORT_H
#include <zip.h>
#include <cstdint>
#include <string>

#ifndef BYTE
#define BYTE unsigned char
#endif

class ZipFile
{
protected:
  std::string mOutputFile;
  zip_t *mZipArchive;
  std::string mErrMsg;
  int mCompressMethod;
  int mCompressFlags;
  int64_t mBytesProcessed;
  int64_t mMaxBytes;
public:
  ~ZipFile() { closeArchive(); }
  static const char mZipPathSeparator = '/';
  int openArchive(std::string filename, int64_t maxBytes, int flags);
  void setCompression(int method, int flags);
  int flushArchive();
  int closeArchive();
  int addFile(std::string filename, BYTE* buff, int64_t size);
  int addDir(std::string name);
  const char* getErrorMsg();
};

#endif
