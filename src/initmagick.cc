#if defined(__MINGW32__) || defined(__MINGW64__)
  #ifndef WIN32
  #define WIN32
  #endif
#endif
#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>
#include <string>

static std::string magickcore_coder_path;
static std::string magickcore_filter_path;
static std::string magickcore_library_path;

static void winPathSanitize(std::string &str)
{
  #if defined(__MINGW32__) || defined(__MINGW64__)
  bool tryagain;
  size_t pos;
  do
  {
    tryagain=false;
    pos=str.find("/\\");
    if (pos != std::string::npos)
    {
      str.replace(pos, 2, "\\");
      tryagain=true;
    }
    pos=str.find("\\/");
    if (pos != std::string::npos)
    {
      str.replace(pos, 2, "\\");
      tryagain=true;
    }
    pos=str.find("/");
    if (pos != std::string::npos)
    {
      str.replace(pos, 1, "\\");
      tryagain=true;
    }
  } while (tryagain);
  pos=str.find(":");
  if (pos == std::string::npos)
  {
    std::string prepend="C:\\msys64";
    prepend.append(str);
    str=prepend;
  }
  #endif
}

const char* getMagickCoreCoderPath()
{
  magickcore_coder_path = MAGICKCORE_CODER_PATH;
  winPathSanitize(magickcore_coder_path);
  return magickcore_coder_path.c_str();
} 

const char* getMagickCoreFilterPath()
{
  magickcore_filter_path = MAGICKCORE_FILTER_PATH;
  winPathSanitize(magickcore_filter_path);
  return magickcore_filter_path.c_str();
}

const char* getMagickCoreLibraryPath()
{
  magickcore_library_path = MAGICKCORE_LIBRARY_PATH;
  winPathSanitize(magickcore_library_path);
  return magickcore_library_path.c_str();
}

