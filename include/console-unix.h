#ifndef __CONSOLE_UNIX__
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

void retractCursor();

bool platform_mkdir(std::string name, std::string* perror);
#endif
