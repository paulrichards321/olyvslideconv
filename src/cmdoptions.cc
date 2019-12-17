#include "cmdoptions.h"

CmdOptions & CmdOptions::operator << (CmdOptions& out, const char *str)
{
  if (str==NULL) return out;
  std::string text = str;

  size_t newlinePos = str.find("\n");
  size_t tabPos = str.find("\t");
  
  int txtStart = 0;
  for (size_t i = 0; i < text.length(); i++)
  {
    char key = text[i];
    if (key == '\n')
    {
      *out.mpCurrentCol << text.substr(txtStart, i);
      out.mColNumber = 1;
      out.mpCurrentCol = &out.mCol1;
      txtStart = i+1;
    }
    else if (key == '\r')
    {
      txtStart = i+1;
    }
    else if (key == '\t')
    {
      if (i > 0)
      {
        *out.mpCurrentCol << text.substr(txtStart, i-1);
      }
      *out.mpCurrentCol << "\n";
      txtStart = i+1;
      if (out.mColNumber == 1)
      {
        out.mpCurrentCol = &out.mCol2;
        out.mColNumber = 2;
      }
      if (out.mColNumber == 2)
      { 
        out.mpCurrentCol = &out.mCol1;
        out.mColNumber = 1;
      }
    }
  }
}


CmdOptions & CmdOptions::operator << (CmdOptions& out, bool value)
{
  if (value==true)
  {
    *out.mpCurrentCol << "ON";
  }
  else
  {
    *out.mpCurrentCol << "OFF";
  }
  return out;
}

CmdOptions & CmdOptions::operator << (CmdOptions& out, int value)
{
  if (value==0)
  {
    *out.mpCurrentCol << "OFF";
  }
  else if (value==1)
  {
    *out.mpCurrentCol << "ON";
  }
  else
  {
    *out.mpCurrentCol << value;
  }
}


std::string CmdOptions::str() 
{
  std::string fullSyntax;
  size_t maxColWidth1 = maxFirstColWidth();
  
  std::string options = col1.str();
  std::string details = col2.str();
  size_t xLength = details.length();
  int line=0;
  
  int startLine=0;
  do
  {
    for (int i=0; i < maxColWidth1; i++)
    {
      if (i < options.length())
      {
        key = options[i];
        if (key == '\n')
        {
          for (int x=0; x < maxColWidth1 - (i - startLine); x++)
          {
            options.append(" ");
          }
        }
      }
  int startLinePos=0;
  int lineLength=0;
  int lastSpacePos=0;
  
  for (int i=0; i < xLength; i++)
  {
    lineLength++;
    if (details[i]==' ')
    {
      lastSpacePos = i;
    }
    if (lineLength+1 >= mTerminalWidth)
    {
      fullSyntax.append(details.substr(startLinePos, lastSpacePos));
      fullSyntax.append("\n");
      lineLength = 0;
      startLinePos = i+1;
    }
    else if (i+1 >= xLength)
    {
      fullSyntax.append(details.substr(startLinePos));
      fullSyntax.append("\n");
      lineLength = 0;
    }
  }
  break;

  if (strOptions == strOptionsOther)
  {
    strOptions = NULL;
  }
  else
  {
    strOptions = strOptionsOther;
    fullSyntax.append(syntaxOther);
  }
}

