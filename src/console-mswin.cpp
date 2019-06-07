#include <cerrno>
#include <cstring>
#include <windows.h>
#include <direct.h>
#include <iostream>

void retractCursor()
{
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  BOOL completed=FALSE;
  if (hStdout && GetConsoleScreenBufferInfo(hStdout, &csbi))
  {
    COORD coord;
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y;
    completed=SetConsoleCursorPosition(hStdout, coord);
  }
  
  if (completed==FALSE) 
  {
    std::cout << "\r";
  }
}

bool platform_mkdir(std::string name, std::string* perror)
{
  if (_mkdir(name.c_str())==0)
  {
    return true;
  }
  if (perror)
  {
    *perror = std::strerror(errno);
  }  
  return false;
}

