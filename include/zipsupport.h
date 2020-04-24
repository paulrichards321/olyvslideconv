#ifndef __ZIP_SUPPORT_H
#define __ZIP_SUPPORT_H
#include <cstdint>
#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#define OLY_DEFAULT_COMPRESS_METHOD MZ_COMPRESS_METHOD_STORE
#define OLY_DEFAULT_COMPRESS_LEVEL  MZ_COMPRESS_LEVEL_DEFAULT
#define OLY_ZIP_VERSION_MADE_BY     MZ_VERSION_MADEBY 
#define OLY_ZIP_OK                  MZ_OK
#ifdef APPEND_STATUS_CREATE
#define OLY_APPEND_STATUS_CREATE    APPEND_STATUS_CREATE
#else
#define OLY_APPEND_STATUS_CREATE    0
#endif
#else
#include <zlib.h>
#include <zip.h>
#define OLY_DEFAULT_COMPRESS_METHOD Z_NO_COMPRESSION
#define OLY_DEFAULT_COMPRESS_LEVEL  Z_DEFAULT_STRATEGY
#define OLY_ZIP_VERSION_MADE_BY     1
#define OLY_ZIP_OK                  Z_OK
#define OLY_APPEND_STATUS_CREATE    APPEND_STATUS_CREATE
#endif

#if !defined(mz_dos_date) && !defined(OLY_HAVE_MZ_DOS_DATE) 
#ifdef OLY_HAVE_DOS_DATE
#define mz_dos_date dos_date
#else
#define mz_dos_date dosDate
#endif
#endif

#ifndef zipClose_64
#define zipClose_64 zipClose
#endif

#ifndef zipCloseFileInZip64
#define zipCloseFileInZip64 zipCloseFileInZip
#endif

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
