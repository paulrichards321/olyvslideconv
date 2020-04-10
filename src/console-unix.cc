#include "console-unix.h"

void retractCursor()
{
  std::cout << "\r";
}  

bool platform_mkdir(std::string name, std::string* perror)
{
  // assign permissions user and group read/write/execute, while other just read and execute
  if (mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0)
  {
    return true;
  }
  if (perror)
  {
    *perror = std::strerror(errno);
  }
  return false;
}

