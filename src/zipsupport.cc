#include "zipsupport.h"
#include <cstring>

int ZipFile::openArchive(std::string filename, int64_t maxBytes, int flags)
{
  int status = 0;
  mOutputFile = filename;
  mMaxBytes = maxBytes;
  int zipErrorNumeric = 0;
  mZipArchive = zip_open(mOutputFile.c_str(), flags, &zipErrorNumeric);
  if (mZipArchive == NULL)
  {
    zip_error_t zipError;
    memset(&zipError, 0, sizeof(zipError));
    zip_error_init_with_code(&zipError, zipErrorNumeric);
    mErrMsg = zip_error_strerror(&zipError); 
    status = 1;
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
  int status = zip_close(mZipArchive);
  if (status == 0)
  {
    int zipErrorNumeric = 0;
    mZipArchive = zip_open(mOutputFile.c_str(), 0, &zipErrorNumeric);
    if (mZipArchive == NULL)
    {
      zip_error_t zipError;
      memset(&zipError, 0, sizeof(zipError));
      zip_error_init_with_code(&zipError, zipErrorNumeric);
      mErrMsg = zip_error_strerror(&zipError); 
      status = 1;
    }
  }
  else
  {
    zip_error_t* pZipError = zip_get_error(mZipArchive);
    mErrMsg = zip_error_strerror(pZipError); 
    mZipArchive = NULL;
  }
  return status;
}

int ZipFile::closeArchive()
{
  if (mZipArchive == NULL) return 0;
  int status = zip_close(mZipArchive);
  if (status != 0)
  {
    zip_error_t* pZipError = zip_get_error(mZipArchive);
    mErrMsg = zip_error_strerror(pZipError); 
  }    
  mZipArchive = NULL;
  return status;
}


int ZipFile::addFile(std::string filename, BYTE* buff, int64_t size)
{
  int zipIndex = -1;
  zip_source_t* zipSrc = NULL;
  int status = 1;
  if (mZipArchive == NULL || buff == NULL) return 0;
  zipSrc = zip_source_buffer(mZipArchive, buff, 0, 0);
  if (zipSrc)
  {
    zipIndex=zip_file_add(mZipArchive, filename.c_str(), zipSrc, ZIP_FL_OVERWRITE);
  }
  if (zipIndex >= 0 &&
      zip_set_file_compression(mZipArchive, zipIndex, mCompressMethod, mCompressFlags) == 0 &&
      zip_source_begin_write(zipSrc) == 0 &&
      zip_source_write(zipSrc, buff, size) == size &&
      zip_source_commit_write(zipSrc) == 0)
  {
    status = 0;
    mBytesProcessed += size;
    if (mBytesProcessed >= mMaxBytes)
    {
      status = flushArchive();
      mBytesProcessed = 0;
    }
  }
  else
  {
    if (mZipArchive)
    {
      zip_error_t* pZipError = zip_get_error(mZipArchive);
      mErrMsg = zip_error_strerror(pZipError); 
    }
    if (zipSrc)
    {
      zip_source_free(zipSrc);
    }
  }
  return status;
}


int ZipFile::addDir(std::string name)
{
  std::string nameWithSlash=name;
  std::size_t lastSlashIndex = name.find_last_of(mZipPathSeparator);
  int status = 0;
  if (mZipArchive == NULL) return 0;
  if (lastSlashIndex == std::string::npos || lastSlashIndex < name.length())
  {
    nameWithSlash.append(1, mZipPathSeparator);
  }
  if (zip_name_locate(mZipArchive, nameWithSlash.c_str(), ZIP_FL_ENC_GUESS) < 0)
  {
    status = zip_dir_add(mZipArchive, name.c_str(), ZIP_FL_ENC_GUESS);
    if (status==-1)
    {
      zip_error_t * zipError = zip_get_error(mZipArchive);
      mErrMsg = zip_error_strerror(zipError);
    }
  }
  return status;
}


const char* ZipFile::getErrorMsg()
{
  return mErrMsg.c_str();
}

