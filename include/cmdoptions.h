#ifndef __CMDOPTIONS_H__
#define __CMDOPTIONS_H__
#include <iostream>

class CmdOptions
{
protected:
  ostringstream col1, col2;
  std::string insertText;
public:
  CmdOptions & operator << (CmdOptions &out, const char* str)
  {
    insertText = str;
  }
}

#endif
