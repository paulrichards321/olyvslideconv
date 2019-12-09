#include "zipsupport.h"
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstring>

static uint32_t unix2dostime(time_t *time);

int ZipFile::openArchive(std::string filename, int64_t maxBytes, int append)
{
  int status = 0;
  mOutputFile = filename;
  mMaxBytes = maxBytes;
  mDirNames.clear();
  mZipArchive = zipOpen64(mOutputFile.c_str(), append);
  if (mZipArchive == NULL)
  {
    mErrMsg = strerror(errno); 
    status = -1;
  }
  return status;
}


void ZipFile::setCompression(int method, int flags)
{
  mCompressMethod = method;
  mCompressFlags = flags;
}


int ZipFile::flushArchive()
{
  if (mZipArchive == NULL) return 0;
  int status = zipClose_64(mZipArchive, NULL);
  if (status==ZIP_OK)
  {
    mZipArchive = zipOpen64(mOutputFile.c_str(), APPEND_STATUS_ADDINZIP);
    if (mZipArchive == NULL)
    {
      mErrMsg = strerror(errno); 
      mDirNames.clear();
      status = -1;
    }
  }
  else
  {
    mErrMsg = strerror(errno); 
    mDirNames.clear();
  }
  return status;
}


int ZipFile::closeArchive()
{
  if (mZipArchive == NULL) return 0;
  int status = zipClose_64(mZipArchive, NULL);
  if (status != ZIP_OK)
  {
    mErrMsg = strerror(errno); 
  }    
  mZipArchive = NULL;
  mDirNames.clear();
  return status;
}


int ZipFile::addFile(std::string filename, BYTE* buff, int64_t size)
{
  zip_fileinfo zinfo;
  time_t currentTime;

  if (mZipArchive == NULL || buff == NULL) return 0;

  memset(&zinfo, 0, sizeof(zinfo));
  time(&currentTime);
  zinfo.mz_dos_date = unix2dostime(&currentTime);
  zinfo.internal_fa = 0644;
  zinfo.external_fa = 0644 << 16L;

  int status = zipOpenNewFileInZip_64(mZipArchive, filename.c_str(), &zinfo, 
    NULL, 0, NULL, 0, NULL, mCompressMethod, mCompressFlags, (size > 0xFFFFFFFFLL) ? 1 : 0); 
  if (status == ZIP_OK)
  {
    status = zipWriteInFileInZip(mZipArchive, buff, size);
    if (status == ZIP_OK)
    {
      status = zipCloseFileInZip64(mZipArchive);
      if (status != ZIP_OK)
      {
        mErrMsg = strerror(errno);
      }
    }
    else
    {
      mErrMsg = strerror(errno);
      zipCloseFileInZip64(mZipArchive);
    }
  }
  else
  {
    mErrMsg = strerror(errno);
  }
  return status;
}


int ZipFile::addDir(std::string name)
{
  zip_fileinfo zinfo;
  time_t currentTime;
  
  if (mZipArchive == NULL) return 0;
  
  memset(&zinfo, 0, sizeof(zinfo));
  time(&currentTime);
  zinfo.mz_dos_date = unix2dostime(&currentTime);
  zinfo.internal_fa = 0755;
  zinfo.external_fa = 040755 << 16L;

  std::string nameWithSlash=name;
  std::size_t lastSlashIndex = name.find_last_of(mZipPathSeparator);
  int status = 0;
  if (lastSlashIndex == std::string::npos || lastSlashIndex < name.length())
  {
    nameWithSlash.append(1, mZipPathSeparator);
  }
  if (std::find(mDirNames.begin(), mDirNames.end(), nameWithSlash) != mDirNames.end())
  {
    return ZIP_OK;
  }
  status = zipOpenNewFileInZip_64(mZipArchive, nameWithSlash.c_str(), 
    &zinfo, NULL, 0, NULL, 0, NULL, MZ_COMPRESS_METHOD_STORE, 0, 0); 
  if (status == ZIP_OK)
  {
    mDirNames.push_back(nameWithSlash);
    status = zipCloseFileInZip64(mZipArchive);
  }
  if (status != ZIP_OK)
  {
    mErrMsg = strerror(errno);
  }
  return status;
}


uint32_t unix2dostime(time_t *time)
{
  if (time==NULL) return 0;
  struct tm *ltime = localtime (time);
  int year = ltime->tm_year - 80;
  if (year < 0)
    year = 0;

  return (year << 25
	  | (ltime->tm_mon + 1) << 21
	  | ltime->tm_mday << 16
	  | ltime->tm_hour << 11
	  | ltime->tm_min << 5
	  | ltime->tm_sec >> 1);
}


const char* ZipFile::getErrorMsg()
{
  return mErrMsg.c_str();
}

