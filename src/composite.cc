/*************************************************************************
Initial Author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017
https://github.com/paulrichards321/jpg2svs

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <new>
#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#include "olyvslideconv.h"

const char* CompositeSlide::miniNames[4][4] = 
{ 
  {
    "FinalScan.ini", "FinalCond.ini", 
    "SlideScan.ini", "SlideCond.ini" 
  },
  {
    "finalscan.ini", "finalcond.ini", 
    "slidescan.ini", "slidecond.ini" 
  },
  {
    "finalScan.ini", "finalCond.ini", 
    "slideScan.ini", "slideCond.ini" 
  },
  {
    "Finalscan.ini", "Finalcond.ini", 
    "Slidescan.ini", "Slidecond.ini" 
  }
};


CompositeSlide::CompositeSlide()
{
  initialize();
  mValidObject = false;
  mbkgColor = 255;
}


IniConf::IniConf()
{
  mname="";
  mFound=false;
  mxMin=0;
  mxMax=0;
  mxDiffMin=0;
  myMin=0;
  myMax=0;
  myDiffMin=0;
  mxAdj=0.0;
  myAdj=0.0;
  mTotalTiles=0;
  mxAxis=0;
  myAxis=0;
  mPixelWidth=0;
  mPixelHeight=0;
  mTotalWidth=0;
  mTotalHeight=0;
  myStepSize=0;
  mxStepSize=0;
  mIsPreviewSlide=false;
  mQuality=0;
  for (int zSplit=0; zSplit < 2; zSplit++)
  {
    for (int zLevel=0; zLevel < 4; zLevel++)
    {
      mzStackExists[zSplit][zLevel] = false;
    }
  }
}


bool CompositeSlide::isPreviewSlide(size_t level)
{
  if (level < mConf.size() && mConf[level]->mFound) 
  {
    return mConf[level]->mIsPreviewSlide; 
  }
  return false;
}


bool CompositeSlide::checkZLevel(int level, int direction, int zLevel)
{
  return (mValidObject == true && level >= 0 && level < (int) mConf.size() && direction >= 0 && direction < 3 && mConf[level]->mFound && (direction==0 || mConf[level]->mzStackExists[direction-1][zLevel])) ? true : false; 
}


void CompositeSlide::initialize()
{
  mValidObject = false;
  
  //std::cout << "Ini Conf size: " << mConf.size();
  if (mConf.size()>0)
  {
    for (size_t i = 0; i < mConf.size(); i++)
    {
      if (mConf[i])
      {
        delete mConf[i];
        mConf[i] = NULL;
      }
    }
    mConf.clear();
  }
  for (int i=0; i<4; i++)
  {
    IniConf *mConfLocal = new IniConf;
    mConfLocal->mname = miniNames[0][i];
    mConf.push_back(mConfLocal);
  }
  mxMin=0;
  mxMax=0;
  myMin=0;
  myMax=0;
  mmagnification = 0;
  mTotalZLevels = 0;
  mTotalBottomZLevels = 0;
  mTotalTopZLevels = 0;
  mGrayScale = false;
} 


void CompositeSlide::close()
{
  mValidObject = false;
  if (mConf.size()>0)
  {
    for (size_t i = 0; i < mConf.size(); i++)
    {
      if (mConf[i])
      {
        delete mConf[i];
        mConf[i] = NULL;
      }
    }
    mConf.clear();
  }
}


bool CompositeSlide::checkLevel(int level)
{
  if (mValidObject == false || level < 0 || level > (int) mConf.size() || mConf[level]->mFound == false || mConf[level]->mKnowStepSizes == false) 
  {
    return false;
  }
  return true;
}


bool CompositeSlide::open(const std::string& srcFileName, int options, int orientation, int optDebug, int64_t bestXOffset, int64_t bestYOffset, safeBmp **ptpImageL2)
{
  JpgFileXY jpgxy;
  JpgFileXY jpgxyzstack;                  
  bool nameFound=false;
  bool xFound=false, yFound=false;
  bool header=false;
  std::string iImageWidth = "iImageWidth";
  std::string iImageHeight = "iImageHeight";
  std::string lXStageRef = "lXStageRef";
  std::string lYStageRef = "lYStageRef";
  std::string lXStepSize = "lXStepSize";
  std::string lYStepSize = "lYStepSize";
  std::string lXOffset = "lXOffset";
  std::string lYOffset = "lYOffset";
  std::string headerStr = "header";
  std::string dMagnification = "dMagnification";
  std::string ImageQuality = "ImageQuality";
  std::string inputDir = srcFileName;
  mGrayScale = false;

  initialize();

  mOptDebug = optDebug;
  mOrientation = orientation;
  mOptBorder=options & CONV_HIGHLIGHT; 
  int optOpenCVAlign=options & CONV_OPENCV_ALIGN;
  int optUseCustomOffset = options & (CONV_CUSTOM_XOFFSET | CONV_CUSTOM_YOFFSET);
  mBestXOffset = bestXOffset;
  mBestYOffset = bestYOffset;

  for (int i=0; i<4; i++)
  {
    for (int cases=0; cases < 4; cases++)
    {
      size_t namePos=inputDir.find(miniNames[cases][i]);
      if (namePos != std::string::npos)
      {
        inputDir = srcFileName.substr(0, namePos-1);
        break;
      }
    }
  }
  if (inputDir.length()>0)
  {
    char lastKey=inputDir[inputDir.length()];
    if (lastKey=='\\' || lastKey=='/')
    {
      inputDir=inputDir.substr(0, inputDir.length()-1);
    }
  }
  
  std::fstream logFile;
  if (optDebug > 1)
  {
    logFile.open("SlideScan.openimage.log", std::ios::out);
  }
  for (int fileNum = 0; fileNum < 4; fileNum++)
  {
    IniConf* pConf = mConf[fileNum];
    std::string inputName;
    std::ifstream iniFile;
    bool foundFile = false;

    for (int cases=0; cases < 4 && foundFile==false; cases++)
    {
      inputName = inputDir;
      inputName += separator();
      inputName += miniNames[cases][fileNum];
    
      iniFile.open(inputName.c_str());
      if (iniFile.good())
      {
        std::cout << "Found: '" << inputName << "'" << std::endl;
        foundFile = true;
      }
    }
    if (foundFile==false)
    {
      inputName = inputDir;
      inputName += separator();
      inputName += miniNames[0][fileNum];
      std::cout << "Warning: Failed to open: '" << inputName << "'!" << std::endl;
    }
    xFound = false;
    yFound = false;
    nameFound = false;
    header = false;

    int c = 0;
    while (iniFile.good() && iniFile.eof()==false)
    {
      std::string line="";
      do
      {
        c = iniFile.get();
        if (c == 0x0A || c == EOF) break;
        if (c != 0x0D && c != ' ' && c != '\t') line += (char) c;
      } while (iniFile.eof()==false && iniFile.good());

      if (line.length()>=3)
      {
        size_t rpos = line.find(']');
        if (line[0]=='[' && rpos != std::string::npos)
        {
          if (xFound && yFound && nameFound)
          {
            pConf->mxyArr.push_back(jpgxy);
          }
          std::string chunkName=line.substr(1, rpos-1);
          if (strcasecmp(headerStr.c_str(), chunkName.c_str())==0)
          {
            jpgxy.mBaseFileName.clear();
            nameFound = false;
            header = true;
          }
          else
          {
            jpgxy.mBaseFileName = inputDir;
            jpgxy.mBaseFileName += separator();
            jpgxy.mBaseFileName += chunkName;
            char location[2][4] = { "_u", "_d" };
            struct stat fileStat;
            for (int zSplit=0; zSplit < 2; zSplit++)
            {
              for (int zLevel=0; zLevel<4; zLevel++)
              {
                std::ostringstream nameStream;
                nameStream << jpgxy.mBaseFileName << location[zSplit] << (zLevel+1) << ".jpg";
                jpgxy.mFileName[zSplit][zLevel] = nameStream.str();
                if (stat(jpgxy.mFileName[zSplit][zLevel].c_str(), &fileStat)==0)
                {
                  pConf->mzStackExists[zSplit][zLevel] = true;
                  jpgxy.mzStack[zSplit][zLevel] = true;
                }
                else
                {
                  jpgxy.mzStack[zSplit][zLevel] = false;
                }
              }
            }
            jpgxy.mBaseFileName += ".jpg";
            nameFound = true;
            header = false;
          }
          //g_object_unref(foldedHeaderName);
          xFound = false;
          yFound = false;
        }
        if (header)
        {
          size_t widthPos=line.find(iImageWidth);
          if (widthPos != std::string::npos && widthPos+iImageWidth.length()+1 < line.length())
          {
            std::string width = line.substr(widthPos+iImageWidth.length()+1);
            pConf->mPixelWidth=atoi(width.c_str());
          }
          size_t heightPos=line.find(iImageHeight);
          if (heightPos != std::string::npos && heightPos+iImageHeight.length()+1 < line.length())
          {
            std::string height = line.substr(heightPos+iImageHeight.length()+1);
            pConf->mPixelHeight=atoi(height.c_str());
          }
          size_t xStagePos=line.find(lXStageRef);
          if (xStagePos != std::string::npos && xStagePos+lXStageRef.length()+1<line.length())
          {
            std::string xStageSubStr = line.substr(xStagePos+lXStageRef.length()+1);
            pConf->mxAxis=atoi(xStageSubStr.c_str());
          }
          size_t yStagePos=line.find(lYStageRef);
          if (yStagePos != std::string::npos && yStagePos+lYStageRef.length()+1<line.length())
          {
            std::string yStageSubStr = line.substr(yStagePos+lYStageRef.length()+1);
            pConf->myAxis=atoi(yStageSubStr.c_str());
          }
          size_t yStepPos = line.find(lYStepSize);
          if (yStepPos != std::string::npos && yStepPos+lYStepSize.length()+1<line.length())
          {
            std::string yStepSubStr = line.substr(yStepPos+lYStepSize.length()+1);
            pConf->myStepSize = atoi(yStepSubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Exact y step measurements found for level " << fileNum << std::endl;
            }
          }
          size_t xStepPos = line.find(lXStepSize);
          if (xStepPos != std::string::npos && xStepPos+lXStepSize.length()+1<line.length())
          {
            std::string xStepSubStr = line.substr(xStepPos+lXStepSize.length()+1);
            pConf->mxStepSize = atoi(xStepSubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Exact x step measurements found for level " << fileNum << std::endl;
            }
          }
          size_t xOffsetPos = line.find(lXOffset);
          if (xOffsetPos != std::string::npos && xOffsetPos+lXOffset.length()+1<line.length())
          {
            //std::string xOffsetSubStr = line.substr(xOffsetPos+lXOffset.length()+1);
            //xOffset = atoi(xOffsetSubStr.c_str());
          }
          size_t yOffsetPos = line.find(lYOffset);
          if (yOffsetPos != std::string::npos && yOffsetPos+lYOffset.length()+1<line.length())
          {
            //std::string yOffsetSubStr = line.substr(yOffsetPos+lYOffset.length()+1);
            //yOffset = atoi(yOffsetSubStr.c_str());
          }
          size_t dMagPos = line.find(dMagnification);
          if (dMagPos != std::string::npos && dMagPos+dMagnification.length()+1<line.length())
          {
            std::string dMagSubStr = line.substr(dMagPos+dMagnification.length()+1);
            mmagnification = atoi(dMagSubStr.c_str());
          }
          size_t qualityPos = line.find(ImageQuality);
          if (qualityPos != std::string::npos && qualityPos+ImageQuality.length()+1<line.length())
          {
            std::string qualitySubStr = line.substr(qualityPos+ImageQuality.length()+1);
            pConf->mQuality = atoi(qualitySubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Jpeg quality read from ini file: " << pConf->mQuality << std::endl;
            }
            if (optDebug > 1)
            {
              logFile << "Jpeg quality read from ini file: " << pConf->mQuality << std::endl;
            }
          }
        }
        std::string line2=line.substr(0, 2);
        if (line2=="x=")
        {
          std::string somenum=line.substr(2);
          jpgxy.mx=atoi(somenum.c_str());
          if (header) 
          {
            pConf->mxAxis = jpgxy.mx;
            jpgxy.mx=0;
          }
          else
          {
            xFound=true;
          }
        }
        if (line2=="y=")
        {
          std::string somenum=line.substr(2);
          jpgxy.my=atoi(somenum.c_str());
          if (header) 
          {
            pConf->myAxis = jpgxy.my;
            jpgxy.my=0;
          }
          else
          {
            yFound=true;
          }
        }
      }
    }
    if (xFound && yFound && nameFound)
    {
      pConf->mxyArr.push_back(jpgxy);
    }
    iniFile.close();
  }
  
  
  myMin=0, myMax=0, mxMin=0, mxMax=0;
  bool yMinSet=false, xMaxSet=false, xMinSet=false, yMaxSet=false;
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    IniConf* pConf=mConf[fileNum];
    if (pConf->mxyArr.size()==0) continue;

    pConf->mTotalTiles = pConf->mxyArr.size();
    if (pConf->mPixelWidth<=0 || pConf->mPixelHeight<=0)
    {
      Jpg jpg;
      jpg.setUnfilledColor(mbkgColor);
      if (jpg.open(pConf->mxyArr[0].mBaseFileName))
      {
        pConf->mPixelWidth=jpg.getActualWidth();
        pConf->mPixelHeight=jpg.getActualHeight();
        jpg.close();
      }
      else
      {
        std::string errMsg;
        jpg.getErrMsg(errMsg);
        std::cerr << "Error: failed to open " << pConf->mxyArr[0].mBaseFileName << " do not have pixel width and height for source jpgs." << std::endl;
        std::cerr << "Returned error: " << errMsg << std::endl;
        if (optDebug > 1) 
        {
          logFile << "Error: failed to open " << pConf->mxyArr[0].mBaseFileName << " do not have pixel width and height for source jpgs." << std::endl;
          logFile << "Returned error: " << errMsg << std::endl;
          logFile.close();
        }
        return false;
      }
    }
    if (optDebug > 1) logFile << "fileName=" << pConf->mname << " jpgWidth=" << pConf->mPixelWidth << " jpgHeight=" << pConf->mPixelHeight << std::endl;
    pConf->mFound = true;
    
    //************************************************************************
    // Get the xmin and xmax values
    //************************************************************************
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end(), JpgFileXYSortForX());
    pConf->mxMin = pConf->mxyArr[0].mx;
    pConf->mxMax = pConf->mxyArr[pConf->mTotalTiles-1].mx;
    for (int64_t i=0; i+1 < pConf->mTotalTiles; i++)
    {
      if (pConf->mxyArr[i+1].mx==pConf->mxyArr[i].mx)
      {
        int64_t diff=pConf->mxyArr[i+1].my - pConf->mxyArr[i].my;
        if ((diff>0 && diff<pConf->myDiffMin) || (diff>0 && pConf->myDiffMin<1))
        {
          pConf->myDiffMin=diff;
        }
      }
    }

    //************************************************************************
    // Get the ymin and ymax values
    //************************************************************************
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end(), JpgFileXYSortForY());
    pConf->myMin=pConf->mxyArr[0].my;
    pConf->myMax=pConf->mxyArr[pConf->mTotalTiles-1].my; // + pConf->yDiffMin;

    for (int64_t i=0; i+1 < pConf->mTotalTiles; i++)
    {
      if (pConf->mxyArr[i+1].my==pConf->mxyArr[i].my)
      {
        int64_t diff=pConf->mxyArr[i+1].mx - pConf->mxyArr[i].mx;
        if ((diff>0 && diff<pConf->mxDiffMin) || (diff>0 && pConf->mxDiffMin<1)) 
        {
          pConf->mxDiffMin=diff;
        }
      }
    }
    if (pConf->mxStepSize>0)
    {
      if (optDebug > 1) logFile << "fileName=" << pConf->mname << " xAdj calculation exact=";
      pConf->mxKnowStepSize = true;
    }
    else
    {
      if (fileNum>0 && mConf[fileNum-1]->mFound && mConf[fileNum-1]->mxStepSize>0)
      {
        pConf->mxStepSize = mConf[fileNum-1]->mxStepSize*4;
        pConf->mxKnowStepSize = true;
      }
      else
      {
        if (pConf->mxDiffMin > 0)
        {
          pConf->mxStepSize = pConf->mxDiffMin;
          pConf->mxKnowStepSize = true;
        }
        else
        {
          pConf->mxStepSize = abs(pConf->mxMax);
          pConf->mxKnowStepSize = false;
        }
      }
      if (optDebug > 1) logFile << "fileName=" << pConf->mname << " Guessing xAdj=";
    }
   	//pConf->mxMin -= pConf->mxStepSize;
    if (pConf->mPixelWidth>0 && pConf->mxStepSize>0)
    {
      pConf->mxAdj = (double) pConf->mxStepSize / (double) pConf->mPixelWidth;
      if (optDebug > 1) logFile << pConf->mxAdj << std::endl;
    }
   
    if (pConf->myStepSize>0)
    {
      if (optDebug > 1) logFile << "fileName=" << pConf->mname << " yAdj calculation exact=";
      pConf->myKnowStepSize = true;
    }
    else
    {
      if (fileNum>0 && mConf[fileNum-1]->mFound && mConf[fileNum-1]->myStepSize>0)
      {
        pConf->myStepSize = (int64_t) (mConf[fileNum-1]->myStepSize*4);
        pConf->myKnowStepSize = true;
      }
      else
      {
        if (pConf->myDiffMin > 0)
        {
          pConf->myStepSize = pConf->myDiffMin;
          pConf->myKnowStepSize = true;
        }
        else
        {
          pConf->myStepSize = abs(pConf->myMax);
          pConf->myKnowStepSize = false;
        }
      }
      if (optDebug > 1) logFile << "fileName=" << pConf->mname << " Guessing yAdj=";
    }
    //pConf->myMin -= pConf->myStepSize;
    pConf->mKnowStepSizes = (pConf->mxKnowStepSize && pConf->myKnowStepSize) ? true : false;
    if (pConf->mPixelHeight>0 && pConf->myStepSize>0)
    {
      pConf->myAdj = (double) pConf->myStepSize / (double) pConf->mPixelHeight;
      if (optDebug > 1) logFile << pConf->myAdj << std::endl;
    }
  
    if (optDebug > 1) 
    {
      logFile << "fileName=" << pConf->mname << " xDiffMin=" << pConf->mxDiffMin << " xStepSize=" << pConf->mxStepSize << " xMin=" << pConf->mxMin << " xMax=" << pConf->mxMax << " xAxis=" << pConf->mxAxis << std::endl;
      logFile << "fileName=" << pConf->mname << " yDiffMin=" << pConf->myDiffMin << " yStepSize=" << pConf->myStepSize << " yMin=" << pConf->myMin << " yMax=" << pConf->myMax << " yAxis=" << pConf->myAxis << std::endl;
    }
    pConf->mOrgDetailedWidth = (int64_t) floor((pConf->mxMax - (pConf->mxMin - pConf->mxStepSize)) / pConf->mxAdj);
    pConf->mDetailedWidth = pConf->mOrgDetailedWidth;

    pConf->mOrgDetailedHeight = (int64_t) floor((pConf->myMax - (pConf->myMin - pConf->myStepSize)) / pConf->myAdj);
    pConf->mDetailedHeight = pConf->mOrgDetailedHeight;

    if ((yMinSet==false || pConf->myMin < myMin) && fileNum < 3)
    {
      myMin=pConf->myMin;
      yMinSet = true;
    }
    if ((yMaxSet==false || pConf->myMax > myMax) && fileNum < 3)
    {
      myMax=pConf->myMax;
      yMaxSet = true;
    }
    if ((xMinSet==false || pConf->mxMin < mxMin) && fileNum < 3)
    { 
      mxMin=pConf->mxMin;
      xMinSet = true;
    }
    if ((xMaxSet==false || pConf->mxMax > mxMax) && fileNum < 3)
    {
      mxMax=pConf->mxMax;
      xMaxSet = true;
    }
    if (fileNum==0)
    {
      for (int zLevel=0; zLevel<4; zLevel++)
      {
        if (pConf->mzStackExists[0][zLevel])
        {
          mTotalBottomZLevels++;
          mTotalZLevels++;
        }
      }
      for (int zLevel=0; zLevel<4; zLevel++)
      {
        if (pConf->mzStackExists[1][zLevel])
        {
          mTotalTopZLevels++;
          mTotalZLevels++;
        }
      }
    }
  }

  bool iniSortByAdj=true;
  for (int i=0; i<4; i++)
  {
    if (mConf[i]->mFound==false || mConf[i]->mxAdj==0 || mConf[i]->myAdj==0)
    {
      iniSortByAdj=false;
    }
  }
  if (iniSortByAdj)
  {
    std::sort(mConf.begin(), mConf.end(), JpgFileXYSortForXAdj());
  }
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    if (mConf[fileNum]->mxAxis==0 || mConf[fileNum]->myAxis==0)
    {
      int targetFileNum;
      if (fileNum==0 && mConf[2]->mxAxis > 0)
      {
        targetFileNum = 2;
      }
      else if (fileNum==0 && mConf[1]->mxAxis > 0)
      {
        targetFileNum = 1;
      }
      else if (fileNum==2 && mConf[0]->mxAxis > 0)
      {
        targetFileNum = 0;
      }
      else if (fileNum==2 && mConf[1]->mxAxis > 0)
      {
        targetFileNum = 1;
      }
      else if (fileNum==1 || fileNum==3)
      {
        targetFileNum = fileNum - 1;
      }
      else 
      {
        targetFileNum = -1;
      }
      if (targetFileNum >= 0)
      {
        mConf[fileNum]->mxAxis = mConf[targetFileNum]->mxAxis;
        mConf[fileNum]->myAxis = mConf[targetFileNum]->myAxis;
      }
      else
      {
        mConf[fileNum]->mxAxis=278000;
        mConf[fileNum]->myAxis=142500;
      }
    }
    if (mConf[fileNum]->mQuality == 0)
    {
      if (fileNum==1)
      {
        mConf[fileNum]->mQuality = 90;
      }
      else if (fileNum>1)
      {
        mConf[fileNum]->mQuality = 95;
      }
      else
      {
        mConf[fileNum]->mQuality = 85;
      }
    }
  } 
  
  //*******************************************************************
  // Find the pyramid level lowest zoom and set that as current image
  //*******************************************************************
  int level=-1;
  for (int min=3; min>=0; min--)
  {
    if (mConf[min]->mFound==true && mConf[min]->mKnowStepSizes==true)
    {
      level=min;
      break;
    }
  }
  if (level==-1)
  {
    if (optDebug > 1) logFile << "File has no readable levels." << std::endl;
    mValidObject = false;
    if (optDebug > 1) logFile.close();
    return false;
  }
  
  //****************************************************************
  // Guess the total image width and height for each pyramid level
  //****************************************************************
  double multiX[3] = { 1.0, 1.0, 1.0 };
  double multiY[3] = { 1.0, 1.0, 1.0 };
  
  if (mConf[3]->mFound && mConf[2]->mFound)
  {
    multiX[2] = mConf[2]->mxAdj / mConf[3]->mxAdj;
    multiY[2] = mConf[2]->myAdj / mConf[3]->myAdj;
  }
  if (mConf[2]->mFound && mConf[1]->mFound)
  {
    multiX[1] = mConf[2]->mxAdj / mConf[1]->mxAdj;
    multiY[1] = mConf[2]->myAdj / mConf[1]->myAdj;
  }
  else if (mConf[3]->mFound && mConf[1]->mFound)
  {
    multiX[1] = mConf[3]->mxAdj / mConf[1]->mxAdj;
    multiY[1] = mConf[3]->myAdj / mConf[1]->myAdj;
  }
  if (mConf[0]->mFound)
  {
    if (mConf[2]->mFound)
    {
      multiX[0] = mConf[2]->mxAdj / mConf[0]->mxAdj;
      multiY[0] = mConf[2]->myAdj / mConf[0]->myAdj;
    }
    else if (mConf[3]->mFound)
    {
      multiX[0] = mConf[3]->mxAdj / mConf[0]->mxAdj;
      multiY[0] = mConf[3]->myAdj / mConf[0]->myAdj;
    }
    else if (mConf[1]->mFound)
    {
      multiX[0] = mConf[1]->mxAdj / mConf[0]->mxAdj;
      multiY[0] = mConf[1]->myAdj / mConf[0]->myAdj;
    }
  }
  
  if (mConf[2]->mFound && mConf[2]->mKnowStepSizes)
  {
    mConf[2]->mTotalWidth = (int64_t)floor((double)(mConf[2]->mxMax - (mConf[2]->mxMin - mConf[2]->mxStepSize)) / (double) mConf[2]->mxAdj);
    mConf[2]->mTotalHeight = (int64_t)floor((double)(mConf[2]->myMax - (mConf[2]->myMin - mConf[2]->myStepSize)) / (double) mConf[2]->myAdj);

    mConf[3]->mTotalWidth = (int64_t) floor(mConf[2]->mTotalWidth * multiX[2]);
    mConf[3]->mTotalHeight = (int64_t) floor(mConf[2]->mTotalHeight * multiY[2]);

    mConf[1]->mTotalWidth = (int64_t) floor(mConf[2]->mTotalWidth * multiX[1]);
    mConf[1]->mTotalHeight = (int64_t) floor(mConf[2]->mTotalHeight * multiY[1]);

    mConf[0]->mTotalWidth = (int64_t) floor(mConf[2]->mTotalWidth * multiX[0]);
    mConf[0]->mTotalHeight = (int64_t) floor(mConf[2]->mTotalHeight * multiY[0]);
  }
  else if (mConf[3]->mFound && mConf[3]->mKnowStepSizes)
  {
    mConf[3]->mTotalWidth = (int64_t) floor((double)(mConf[3]->mxMax - (mConf[3]->mxMin - mConf[3]->mxStepSize)) / (double) mConf[3]->mxAdj);
    mConf[3]->mTotalHeight = (int64_t) floor((double)(mConf[3]->myMax - (mConf[3]->myMin - mConf[3]->myStepSize)) / (double) mConf[3]->myAdj);

    mConf[1]->mTotalWidth = (int64_t) floor(mConf[3]->mTotalWidth * multiX[1]);
    mConf[1]->mTotalHeight = (int64_t) floor(mConf[3]->mTotalHeight * multiY[1]);
    mConf[0]->mTotalWidth = (int64_t) floor(mConf[3]->mTotalWidth * multiX[0]);
    mConf[0]->mTotalHeight = (int64_t) floor(mConf[3]->mTotalHeight * multiY[0]);
  }
  else if (mConf[1]->mFound && mConf[1]->mKnowStepSizes)
  {
    mConf[1]->mTotalWidth = (int64_t) floor((double)(mConf[1]->mxMax - (mConf[1]->mxMin - mConf[1]->mxStepSize)) / (double) mConf[1]->mxAdj);
    mConf[1]->mTotalHeight = (int64_t) floor((double)(mConf[1]->myMax - (mConf[1]->myMin - mConf[1]->myStepSize)) / (double) mConf[1]->myAdj);

    mConf[0]->mTotalWidth = (int64_t) floor(mConf[1]->mTotalWidth * multiX[0]);
    mConf[0]->mTotalHeight = (int64_t) floor(mConf[1]->mTotalHeight * multiY[0]);
  }
  else
  {
    for (int fileNum=0; fileNum < 4; fileNum++)
    {
      mConf[fileNum]->mTotalWidth = (int64_t) floor((double)(mConf[fileNum]->mxMax - (mConf[fileNum]->mxMin - mConf[fileNum]->mxStepSize)) / (double) mConf[fileNum]->mxAdj);
      mConf[fileNum]->mTotalHeight = (int64_t) floor((double)(mConf[fileNum]->myMax - (mConf[fileNum]->myMin - mConf[fileNum]->myStepSize)) / (double) mConf[fileNum]->myAdj);
    }
  }

  // log file width and height
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    if (optDebug > 1) logFile << "fileName=" << mConf[fileNum]->mname << " totalWidth in pixels=" << mConf[fileNum]->mTotalWidth << " totalHeight in pixels=" << mConf[fileNum]->mTotalHeight << std::endl;
  }

  IniConf* pHigherConf = NULL;
  IniConf* pLowerConf = NULL;
  int higherLevel = -1;
  int lowerLevel = -1;
  bool higherLevelFound = false, lowerLevelFound = false;
  if (mConf[2]->mFound && mConf[2]->mKnowStepSizes)
  {
    pHigherConf = mConf[2];
    higherLevelFound = true;
    higherLevel = 2;
  }
  else if (mConf[3]->mFound && mConf[3]->mKnowStepSizes)
  {
    pHigherConf = mConf[3];
    higherLevelFound = true;
    higherLevel = 3;
  }
  if (mConf[0]->mFound)
  {
    pLowerConf = mConf[0];
    lowerLevel = 0;
    lowerLevelFound = true;
  }
  if (mConf[1]->mFound)
  {
    pLowerConf = mConf[1];
    lowerLevel = 1;
    lowerLevelFound = true;
  }
  //*****************************************************************
  // Calculate the x and y coordinate of higherLevels 
  //*****************************************************************
  for (int fileNum=2; fileNum<4; fileNum++)
  {
    IniConf* pConf=mConf[fileNum];
    if (pConf->mFound==false) continue;
     
    for (int64_t i=0; i<pConf->mTotalTiles; i++)
    {
      // missing lines problem occurs in rounding here
      double xPixel=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
      int64_t xPixelInt=(int64_t) round(xPixel);
      //if (xPixelInt>0) xPixelInt--;
      pConf->mxyArr[i].mxPixel=xPixelInt; // previous use lround here
      
      double yPixel=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
      int64_t yPixelInt=(int64_t) round(yPixel);
      //if (yPixelInt>0) yPixelInt--;
      pConf->mxyArr[i].myPixel=yPixelInt; // previous use lround here
      
      if (optDebug > 1) logFile << "filename=" << pConf->mxyArr[i].mBaseFileName << " x=" << xPixelInt << " y=" << yPixelInt << std::endl;

      for (int zSplit=0; zSplit < 2; zSplit++)
      {
        for (int zLevel=0; zLevel<4; zLevel++)
        {
          if (pConf->mxyArr[i].mzStack[zSplit][zLevel] && optDebug > 1)
          {
            logFile << "filename=" << pConf->mxyArr[i].mFileName[zSplit][zLevel] << " x=" << xPixelInt << " y=" << yPixelInt << std::endl;
          }
        }
      }
    }
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end());
  }
  mValidObject = true;

  int64_t bestXOffsetL0=0, bestXOffsetL1=0;
  int64_t bestYOffsetL0=0, bestYOffsetL1=0;
  #ifndef USE_MAGICK
  if (lowerLevelFound && higherLevelFound && optOpenCVAlign)
  {
    findXYOffset(lowerLevel, higherLevel, &bestXOffsetL0, &bestYOffsetL0, &bestXOffsetL1, &bestYOffsetL1, optUseCustomOffset, optDebug, logFile);
  }
  #endif
  if (lowerLevelFound && higherLevelFound && optOpenCVAlign==false)
  {
    double higherRatioX = (double) pHigherConf->mPixelWidth / (double) pHigherConf->mxStepSize; 
    double higherRatioY = (double) pHigherConf->mPixelHeight / (double) pHigherConf->myStepSize;

    double higherMinBaseX = (double) (pLowerConf->mxAxis - pHigherConf->mxMax);
    double higherMinBaseY = (double) (pLowerConf->myAxis - pHigherConf->myMax);

    if (mConf[0]->mFound)
    {
      double ratioAL0X = (double) mConf[0]->mPixelWidth / (double) mConf[0]->mxStepSize;
      double ratioAL0Y = (double) mConf[0]->mPixelHeight / (double) mConf[0]->myStepSize;
      double ratioBL0X = (double) pHigherConf->mxStepSize / (double) mConf[0]->mxStepSize;
      double ratioBL0Y = (double) pHigherConf->myStepSize / (double) mConf[0]->myStepSize;

      double stageBaseL0X = (double) mConf[0]->mxAxis + ((double) pHigherConf->mxStepSize / 2);
      stageBaseL0X -= (double) mConf[0]->mxStepSize / 2;

      double stageBaseL0Y = (double) mConf[0]->myAxis + ((double) pHigherConf->myStepSize / 2);
      stageBaseL0Y -= (double) mConf[0]->myStepSize / 2;

      double lowerMinBaseL0X = stageBaseL0X - mConf[0]->mxMax;
      double lowerMinBaseL0Y = stageBaseL0Y - mConf[0]->myMax;
   
      double minusL0X = higherMinBaseX * higherRatioX * ratioBL0X;
      double minusL0Y = higherMinBaseY * higherRatioY * ratioBL0Y;

      bestXOffsetL0 = (int64_t) ceil(lowerMinBaseL0X * ratioAL0X - minusL0X);
      bestYOffsetL0 = (int64_t) ceil(lowerMinBaseL0Y * ratioAL0Y - minusL0Y);
    }
    if (mConf[1]->mFound)
    {
      double ratioAL1X = (double) mConf[1]->mPixelWidth / (double) mConf[1]->mxStepSize;
      double ratioAL1Y = (double) mConf[1]->mPixelHeight / (double) mConf[1]->myStepSize;
      double ratioBL1X = (double) pHigherConf->mxStepSize / (double) mConf[1]->mxStepSize;
      double ratioBL1Y = (double) pHigherConf->myStepSize / (double) mConf[1]->myStepSize;

      double stageBaseL1X = (double) mConf[1]->mxAxis + ((double) pHigherConf->mxStepSize / 2);
      stageBaseL1X -= (double) mConf[1]->mxStepSize / 8;

      double stageBaseL1Y = (double) mConf[1]->myAxis + ((double) pHigherConf->myStepSize / 2);
      stageBaseL1Y -= (double) mConf[1]->myStepSize / 8;

      double lowerMinBaseL1X = stageBaseL1X - mConf[1]->mxMax;
      double lowerMinBaseL1Y = stageBaseL1Y - mConf[1]->myMax;
   
      double minusL1X = higherMinBaseX * higherRatioX * ratioBL1X;
      double minusL1Y = higherMinBaseY * higherRatioY * ratioBL1Y;

      bestXOffsetL1 = (int64_t) ceil(lowerMinBaseL1X * ratioAL1X - minusL1X);
      bestYOffsetL1 = (int64_t) ceil(lowerMinBaseL1Y * ratioAL1Y - minusL1Y);
    }
  }
  if (optDebug > 0)
  {
    std::cout << "Best X Offset Level0=" << bestXOffsetL0 << std::endl;
    std::cout << "Best Y Offset Level0=" << bestYOffsetL0 << std::endl;
    std::cout << "Best X Offset Level1=" << bestXOffsetL1 << std::endl;
    std::cout << "Best Y Offset Level1=" << bestYOffsetL1 << std::endl;
  }
  //*****************************************************************
  // Calculate the x and y coordinate of each file starting pixels
  //*****************************************************************
  for (int fileNum=0; fileNum<2; fileNum++)
  {
    IniConf* pConf=mConf[fileNum];
    if (pConf->mFound==false) continue;
    pConf->mxSortedArr.resize(pConf->mxyArr.size());

    for (int64_t i=0; i<pConf->mTotalTiles; i++)
    {
      double xPixel;
      xPixel=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
      double yPixel;
      yPixel=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
      if (higherLevelFound && fileNum==0)
      {
        xPixel += bestXOffsetL0;
        yPixel += bestYOffsetL0;
      }  
      else if (higherLevelFound && fileNum==1)
      {
        xPixel += bestXOffsetL1;
        yPixel += bestYOffsetL1;
      }
      pConf->mxyArr[i].mxPixel=(int64_t)round(xPixel);
      pConf->mxyArr[i].myPixel=(int64_t)round(yPixel);
      pConf->mxSortedArr[i].mxPixel=(int64_t)round(xPixel);
      pConf->mxSortedArr[i].myPixel=(int64_t)round(yPixel);
      
      if (optDebug > 1) logFile << "filename=" << pConf->mxyArr[i].mBaseFileName << " x=" << xPixel << " y=" << yPixel << std::endl;
    }
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end());
    std::sort(pConf->mxSortedArr.begin(), pConf->mxSortedArr.end(), JpgXYSortForX());
    for (int64_t tileNum=0; tileNum< (int64_t) pConf->mxyArr.size(); tileNum++)
    {
      for (int64_t tileNum2=0; tileNum2< (int64_t) pConf->mxyArr.size(); tileNum2++)
      {
        if (pConf->mxSortedArr[tileNum].mxPixel==pConf->mxyArr[tileNum2].mxPixel && pConf->mxyArr[tileNum2].myPixel==pConf->mxSortedArr[tileNum].myPixel)
        {
          pConf->mxyArr[tileNum2].mxSortedIndex = tileNum;
          break;
        }
      }
    }
  }
  //*****************************************************************
  // If orientation different, recalculate x and y coordinates
  // based on already existing ones
  //*****************************************************************
  if (orientation != 0) 
  {
    setOrientation(orientation, logFile);
  }
  
  if (higherLevelFound)
  {
    loadFullImage(higherLevel, ptpImageL2, NULL, orientation, 1.0, 1.0, false, optDebug, logFile);
  }
  mbaseWidth = mConf[0]->mTotalWidth;
  mbaseHeight = mConf[0]->mTotalHeight;
  std::string previewFileName = inputDir;
  previewFileName += separator();
  previewFileName += "PreviewSlide.jpg";
  Jpg previewJpg;
  if (previewJpg.open(previewFileName))
  {
    IniConf *previewConf = new IniConf;
    previewConf->mPixelWidth = previewConf->mTotalWidth = previewJpg.getActualWidth();
    previewConf->mPixelHeight = previewConf->mTotalHeight = previewJpg.getActualHeight();
    previewConf->mTotalTiles = 1;
    if (optDebug > 1) logFile << " PreviewSlide.jpg found. Width=" << previewConf->mPixelWidth << " Height=" << previewConf->mPixelHeight << std::endl;
    jpgxy.mBaseFileName = previewFileName;
    jpgxy.mxPixel=0;
    jpgxy.myPixel=0;
    jpgxy.mx=0;
    jpgxy.my=0;
    previewConf->mFound = true;
    previewConf->mxyArr.push_back(jpgxy);
    previewConf->mIsPreviewSlide = true;
    mConf.push_back(previewConf);
    previewJpg.close();
  }
  else
  {
    std::cerr << "Warning: PreviewSlide.jpg not found." << std::endl;
  }
  if (optDebug > 1) logFile.close();
  return true;
}

#ifndef USE_MAGICK
bool CompositeSlide::findXYOffset(int lowerLevel, int higherLevel, int64_t *bestXOffset0, int64_t *bestYOffset0, int64_t *bestXOffset1, int64_t *bestYOffset1, int optUseCustomOffset, int optDebug, std::fstream& logFile)
{
  double xMulti0 = mConf[2]->mxAdj / mConf[0]->mxAdj;
  double yMulti0 = mConf[2]->myAdj / mConf[0]->myAdj;
  double xMulti1 = mConf[2]->mxAdj / mConf[1]->mxAdj;
  double yMulti1 = mConf[2]->myAdj / mConf[1]->myAdj;
  IniConf *pLowerConf = mConf[lowerLevel];
  IniConf *pHigherConf = mConf[higherLevel];
  if (pLowerConf->mFound == false || pHigherConf->mFound == false)
  {
    return false;
  }
  double xZoomOut = pHigherConf->mxAdj / pLowerConf->mxAdj;
  double yZoomOut = pHigherConf->myAdj / pLowerConf->myAdj;

  cv::Mat *pImgComplete1 = NULL;
  cv::Mat *pImgComplete2 = NULL;
  if (optDebug > 1) 
  {
    logFile << "Reading Olympus level " << lowerLevel << " and scaling..." << std::endl;
  }
  bool success = loadFullImage(lowerLevel, NULL, &pImgComplete1, 0, xZoomOut, yZoomOut, true, optDebug, logFile);
  if (success == false)
  {
    std::cerr << "Warning! Failed to load lower Olympus level " << lowerLevel << ". Cannot use image matching to determine X and Y Offsets between the higher and lower levels!" << std::endl;
    return false;
  }

  if (optDebug > 1) 
  {
    logFile << "Reading Olympus level " << higherLevel << "..." << std::endl;
  }
  success = loadFullImage(higherLevel, NULL, &pImgComplete2, 0, 1.0, 1.0, false, optDebug, logFile);
  if (success == false)
  {   
    std::cerr << "Warning! Failed to load higher Olympus level " << lowerLevel << ". Cannot use image matching to determine X and Y Offsets between the higher and lower levels!" << std::endl;
    if (pImgComplete1)
    {
      if (pImgComplete1->data)
      {
        pImgComplete1->release();
      }
      delete pImgComplete1;
    }
    return false;
  }
  //------------------------------------------------------------------------
  // AKAZE
  //------------------------------------------------------------------------
  std::cout << "Finding unique image descriptors in lower and higher levels ..." << std::endl;
  cv::Ptr<cv::AKAZE> akaze_detector = cv::AKAZE::create();
  std::vector<cv::KeyPoint> keypoints1, keypoints2;
  cv::Mat descriptors1, descriptors2;

  akaze_detector->detectAndCompute(*pImgComplete1, cv::noArray(), keypoints1, descriptors1);
  akaze_detector->detectAndCompute(*pImgComplete2, cv::noArray(), keypoints2, descriptors2);

  std::cout << "Running descriptor matcher..." << std::endl;
  cv::BFMatcher bfMatcher(cv::NORM_HAMMING);
  std::vector<cv::DMatch> matches;
  bfMatcher.match(descriptors1, descriptors2, matches);

  if (pImgComplete1)
  {
    if (pImgComplete1->data)
    {
      pImgComplete1->release();
    }
    delete pImgComplete1;
    pImgComplete1 = NULL;
  }
  if (pImgComplete2)
  {
    if (pImgComplete2->data)
    {
      pImgComplete2->release();
    }
    delete pImgComplete2;
    pImgComplete2 = NULL;
  }
 
  std::vector<double> diffXs;
  std::vector<double> diffYs;
  std::sort(matches.begin(), matches.end(), CVMatchCompare());
  for (size_t i = 0; i < matches.size(); i++)
  { 
    double diffX = keypoints2[matches[i].trainIdx].pt.x - keypoints1[matches[i].queryIdx].pt.x;
    double diffY = keypoints2[matches[i].trainIdx].pt.y - keypoints1[matches[i].queryIdx].pt.y;
    diffXs.push_back(diffX);
    diffYs.push_back(diffY);
  }
  int64_t bestXOffset, bestYOffset;
  if (optUseCustomOffset)
  {
    bestXOffset = mBestXOffset;
    bestYOffset = mBestYOffset;
  }
  else if (diffXs.size()>0)
  {
    bestXOffset = (int64_t) lround(diffXs[0]);
    bestYOffset = (int64_t) lround(diffYs[0]);
  }
  else
  {
    bestXOffset = 0;
    bestYOffset = 0;
  }
  if (optDebug > 1)
  {
    logFile << "Diff X Vector Size: " << diffXs.size() << std::endl;
    logFile << "Best (Usually first in sorted arrays unless you set custom offsets) X, Y alignment: " << bestXOffset << " " << bestYOffset << std::endl;
    logFile << "Alignment array: " << std::endl;
    for (uint64_t i=0; i<diffXs.size(); i++)
    {
      logFile << " {" << diffXs[i] << "," << diffYs[i] << "} ";
    }
    logFile << std::endl;
  }

  *bestXOffset0 = (int64_t) floor((double)(bestXOffset * xMulti0) + ((mConf[1]->mxMax - mConf[0]->mxMax) / mConf[0]->mxAdj));
  *bestYOffset0 = (int64_t) floor((double)(bestYOffset * yMulti0) + ((mConf[1]->myMax - mConf[0]->myMax) / mConf[0]->myAdj));
  *bestXOffset1 = (int64_t) floor(bestXOffset * xMulti1);
  *bestYOffset1 = (int64_t) floor(bestYOffset * yMulti1);
 
  return true;
}
#endif


bool CompositeSlide::setOrientation(int orientation, std::fstream& logFile)
{
  mOrientation = 0;
  
  switch (orientation)
  {
    case 0:
      return true;
    case -90:
    case 90:
    case 180:
    case 270:
      break;
    default:
      return false;
  }
  mOrientation = orientation;

  //*****************************************************************
  // Recalculate the x and y coordinate of each file starting pixels
  // based on orientation
  //*****************************************************************
  for (int fileNum=0; fileNum<4; fileNum++)
  {
    IniConf* pConf=mConf[fileNum];
    if (pConf->mFound==false) continue;

    int64_t totalTiles = pConf->mTotalTiles;
    int64_t totalWidth = pConf->mTotalWidth;
    int64_t totalHeight = pConf->mTotalHeight;
    int64_t detailedWidth = pConf->mDetailedWidth;
    int64_t detailedHeight = pConf->mDetailedHeight;
    int64_t pixelWidth = pConf->mPixelWidth;
    int64_t pixelHeight = pConf->mPixelHeight;
    for (int64_t i=0; i < totalTiles; i++)
    {
      int64_t xPixel=pConf->mxyArr[i].mxPixel;
      int64_t yPixel=pConf->mxyArr[i].myPixel;
      int64_t xPixelNew = xPixel;
      int64_t yPixelNew = yPixel;
      if (orientation == 0)
      {
        // place holder
      }  
      else if (orientation == 90)
      {
        xPixelNew = (totalHeight - yPixel) - pixelHeight;
        yPixelNew = xPixel;
      }
      else if (orientation == -90 || orientation == 270)
      {
        xPixelNew = yPixel;
        yPixelNew = (totalWidth - xPixel) - pixelWidth;
      }
      else if (orientation == 180)
      {
        xPixelNew = (totalWidth - xPixel) - pixelWidth;
        yPixelNew = (totalHeight - yPixel) - pixelHeight;
      }
      pConf->mxyArr[i].mxPixel=xPixelNew;
      pConf->mxyArr[i].myPixel=yPixelNew;
      if (fileNum < 2)
      {
        pConf->mxSortedArr[i].mxPixel=xPixelNew;
        pConf->mxSortedArr[i].myPixel=yPixelNew;
      }
      if (mOptDebug > 1) logFile << "filename=" << pConf->mxyArr[i].mBaseFileName << " x=" << xPixel << " y=" << yPixel << std::endl;
    }
    if (orientation == 90 || orientation == -90 || orientation == 270)
    {
      pConf->mTotalWidth = totalHeight;
      pConf->mTotalHeight = totalWidth;
      pConf->mDetailedWidth = detailedHeight;
      pConf->mDetailedHeight = detailedWidth;
    }
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end());
    if (fileNum < 2)
    {
      std::sort(pConf->mxSortedArr.begin(), pConf->mxSortedArr.end(), JpgXYSortForX());
      for (int64_t tileNum=0; tileNum< (int64_t) pConf->mxyArr.size(); tileNum++)
      {
        for (int64_t tileNum2=0; tileNum2< (int64_t) pConf->mxyArr.size(); tileNum2++)
        {
          if (pConf->mxSortedArr[tileNum].mxPixel==pConf->mxyArr[tileNum2].mxPixel && pConf->mxyArr[tileNum2].myPixel==pConf->mxSortedArr[tileNum].myPixel)
          {
            pConf->mxyArr[tileNum2].mxSortedIndex = tileNum;
            break;
          }
        }
      }
    }
  }
  return true;
}

#ifndef USE_MAGICK
bool CompositeSlide::loadFullImage(int level, safeBmp **ptpFullImage, cv::Mat **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile)
{
  IniConf *pConf = mConf[level];
  
  if (ptpFullImage == NULL && ptpMatImage == NULL) 
  {
    return false;
  }
  if (pConf->mFound == false)
  {
    return false;
  }

  if (ptpFullImage)
  {
    *ptpFullImage = NULL;
  }
  if (ptpMatImage)
  {
    *ptpMatImage = NULL;
  }

  int64_t orgDetailedWidth = pConf->mOrgDetailedWidth;
  int64_t orgDetailedHeight = pConf->mOrgDetailedHeight;
  int64_t simulatedWidth = orgDetailedWidth; 
  int64_t simulatedHeight = orgDetailedHeight;
  if (useZoom)
  {
    simulatedWidth = (int64_t) lround((double) orgDetailedWidth / xZoomOut);
    simulatedHeight = (int64_t) lround((double) orgDetailedHeight / yZoomOut);
  }
  if (orientation == 90 || orientation == -90 || orientation == 270)
  {
    int64_t simulatedHeightOld = simulatedHeight;
    simulatedHeight = simulatedWidth;
    simulatedWidth = simulatedHeightOld;
  }
 
  cv::Mat *pImgComplete = new cv::Mat((int) simulatedHeight, (int) simulatedWidth, CV_8UC3, cv::Scalar(255,255,255));

  for (int64_t i=0; i < pConf->mTotalTiles; i++)
  {
    cv::Mat imgPart1 = cv::imread(pConf->mxyArr[i].mBaseFileName, cv::IMREAD_COLOR); 
    cv::Mat *imgPart = &imgPart1;
    int64_t orgCols = imgPart1.cols;
    int64_t orgRows = imgPart1.rows;
    cv::Mat imgPart2, imgPart3;
    cv::Mat* pImgScaled = NULL;
    switch (orientation)
    {
      case 0:
        break;
      case 90:
        cv::transpose(imgPart1, imgPart2);
        imgPart1.release();
        cv::flip(imgPart2, imgPart3, 1);
        imgPart2.release();
        imgPart = &imgPart3;
        break;
      case -90:
      case 270:
        cv::transpose(imgPart1, imgPart2);
        imgPart1.release();
        cv::flip(imgPart2, imgPart3, 0);
        imgPart2.release();
        imgPart = &imgPart3;
        break;
      case 180:
        cv::flip(imgPart1, imgPart2, -1);
        imgPart1.release();
        imgPart = &imgPart2;
        break;
    }
    if (useZoom)
    {
      cv::Size scaledSize((int64_t)lround(imgPart->cols / xZoomOut), (int64_t)lround(imgPart->rows / yZoomOut));
      pImgScaled = new cv::Mat((int64_t)scaledSize.width, (int64_t)scaledSize.height, CV_8UC3, cv::Scalar(255,255,255));
      cv::resize(*imgPart, *pImgScaled, scaledSize);
      imgPart->release();
      imgPart = pImgScaled;
    }
    double xPixelDbl=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
    double yPixelDbl=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
    int64_t xPixel = (int64_t) round(xPixelDbl);
    int64_t yPixel = (int64_t) round(yPixelDbl);
    int64_t cols = imgPart->cols;
    int64_t rows = imgPart->rows;
    int64_t xPixelNew = xPixel;
    int64_t yPixelNew = yPixel;
    if (orientation == 0)
    {
      // place holder, do nothing
    }  
    else if (orientation == 90)
    {
      xPixelNew = (orgDetailedHeight - yPixel) - orgRows;
      yPixelNew = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      xPixelNew = yPixel;
      yPixelNew = (orgDetailedWidth - xPixel) - orgCols;
    }
    else if (orientation == 180)
    {
      xPixelNew = (orgDetailedWidth - xPixel) - orgCols;
      yPixelNew = (orgDetailedHeight - yPixel) - orgRows;
    }
    if (useZoom)
    {
      xPixelNew = (int64_t) lround(xPixelNew / xZoomOut);
      yPixelNew = (int64_t) lround(yPixelNew / yZoomOut);
    }
    if (xPixelNew < 0)
    {
      cols += xPixelNew;
      xPixelNew = 0;
    }
    if (yPixelNew < 0)
    {
      rows += yPixelNew;
      yPixelNew = 0;
    }
    if (xPixelNew + cols > pImgComplete->cols)
    {
      cols -= (xPixelNew + cols) - pImgComplete->cols;
    }
    if (yPixelNew + rows > pImgComplete->rows)
    {
      rows -= (yPixelNew + rows) - pImgComplete->rows;
    }
    if (cols > 0 && rows > 0 && xPixelNew < pImgComplete->cols && yPixelNew < pImgComplete->rows)
    {
      cv::Rect roi(0, 0, (int) cols, (int) rows);
      cv::Mat srcRoi(*imgPart, roi);
      cv::Rect roi2((int) xPixelNew, (int) yPixelNew, (int) cols, (int) rows);
      cv::Mat destRoi(*pImgComplete, roi2); 
      srcRoi.copyTo(destRoi);
      srcRoi.release();
      destRoi.release();
    }
    else
    {
      std::cerr << "Warning: ROI outside of image boundaries: xPixelNew=" << xPixelNew << " cols=" << cols << " total width of image=" << pImgComplete->cols;
      std::cerr << " yPixelNew=" << yPixelNew << " rows=" << rows << " total height of image=" << pImgComplete->rows << std::endl;
    }
    imgPart->release();
    if (pImgScaled)
    {
      delete pImgScaled;
      pImgScaled = NULL;
    }
  }
  if (optDebug > 1 && pImgComplete && pImgComplete->data)
  {
    std::stringstream ss;
    ss << "imgComplete" << level;
    if (orientation != 0)
    {
      ss << "_" << orientation;    
    }
    ss << ".jpg";

    std::string levelFName = ss.str();
    cv::imwrite(levelFName.c_str(), *pImgComplete);
  }
  if (ptpFullImage && pImgComplete && pImgComplete->data)
  {
    safeBmp *pImageL2 = safeBmpAlloc(pConf->mDetailedWidth, pConf->mDetailedHeight);
    *ptpFullImage = pImageL2;
    safeBmp safeImgComplete2Ref;
    safeBmpInit(&safeImgComplete2Ref, pImgComplete->data, pConf->mDetailedWidth, pConf->mDetailedHeight);
    safeBmpBGRtoRGBCpy(pImageL2, &safeImgComplete2Ref);
    safeBmpFree(&safeImgComplete2Ref);
  }
  if (ptpMatImage == NULL && pImgComplete)
  {
    pImgComplete->release();
    delete pImgComplete;
    pImgComplete = NULL;
  }
  else
  {
    *ptpMatImage = pImgComplete;
  }
  return true;
}

#else

// TODO: Zoom is not enabled on this function yet

bool CompositeSlide::loadFullImage(int level, safeBmp **ptpImageL2, void **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile)
{
  IniConf *pConf = mConf[level];
  if (pConf->mFound == false)
  {
    return false;
  }
  if (ptpImageL2 == NULL)
  {
    return false;
  }

  Magick::MagickWand *magickWand = Magick::NewMagickWand();
  Magick::PixelWand *pixelWand = Magick::NewPixelWand();
  Magick::PixelSetColor(pixelWand, "#ffffff");
  Magick::MagickSetImageType(magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(magickWand, 8);
  Magick::MagickSetImageAlphaChannel(magickWand, Magick::OffAlphaChannel);
  Magick::MagickSetCompression(magickWand, Magick::NoCompression);
  Magick::MagickNewImage(magickWand, pConf->mDetailedWidth, pConf->mDetailedHeight, pixelWand);
  Magick::MagickWand *magickWand2 = Magick::NewMagickWand(); 

  int64_t orgDetailedWidth = pConf->mOrgDetailedWidth;
  int64_t orgDetailedHeight = pConf->mOrgDetailedHeight;

  if (optDebug > 1) 
  {
    logFile << "Reading level " << level << "." << std::endl;
  }
  for (int64_t i=0; i<pConf->mTotalTiles; i++)
  {
    Magick::MagickSetImageType(magickWand2, Magick::TrueColorType);
    Magick::MagickSetImageDepth(magickWand2, 8);
    Magick::MagickSetImageAlphaChannel(magickWand2, Magick::OffAlphaChannel);
    if (Magick::MagickReadImage(magickWand2, pConf->mxyArr[i].mBaseFileName.c_str())==Magick::MagickFalse)
    {
      Magick::ExceptionType exType;
      std::cerr << "Failed to open '" << pConf->mxyArr[i].mBaseFileName << "'. Reason: " << Magick::MagickGetException(magickWand2, &exType) << std::endl;
      continue;
    }  
    int64_t orgCols = (int64_t) Magick::MagickGetImageWidth(magickWand2);
    int64_t orgRows = (int64_t) Magick::MagickGetImageHeight(magickWand2);

    switch (orientation)
    {
      case 0:
        break;
      case 90:
      case -90:
      case 270:
      case 180:
        Magick::MagickRotateImage(magickWand2, pixelWand, (double) orientation);
        break;
    }
    double xPixelDbl=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
    double yPixelDbl=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
    int64_t xPixel = (int64_t) round(xPixelDbl);
    int64_t yPixel = (int64_t) round(yPixelDbl);
    int64_t xPixelNew = xPixel;
    int64_t yPixelNew = yPixel;
    if (orientation == 0)
    {
      // place holder, do nothing
    }  
    else if (orientation == 90)
    {
      xPixelNew = (orgDetailedHeight - yPixel) - orgRows;
      yPixelNew = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      xPixelNew = yPixel;
      yPixelNew = (orgDetailedWidth - xPixel) - orgCols;
    }
    else if (orientation == 180)
    {
      xPixelNew = (orgDetailedWidth - xPixel) - orgCols;
      yPixelNew = (orgDetailedHeight - yPixel) - orgRows;
    }
    Magick::MagickCompositeImage(magickWand, magickWand2, Magick::OverCompositeOp, Magick::MagickTrue, xPixelNew, yPixelNew); 
    Magick::ClearMagickWand(magickWand2);
  }
  if (magickWand2) Magick::DestroyMagickWand(magickWand2);
  if (optDebug > 1) 
  {
    Magick::Image* pImgComplete2 = Magick::GetImageFromMagickWand(magickWand);
    magickWand2 = Magick::NewMagickWandFromImage(pImgComplete2);
    Magick::MagickSetImageType(magickWand2, Magick::TrueColorType);
    Magick::MagickSetImageDepth(magickWand2, 8);
    Magick::MagickSetImageAlphaChannel(magickWand2, Magick::OffAlphaChannel);
    MagickSetImageCompressionQuality(magickWand2, 90);
    std::stringstream ss;
    ss << "imgComplete" << level << ".jpg";
    std::string levelFName = ss.str();
    Magick::MagickWriteImage(magickWand2, levelFName.c_str());
    Magick::DestroyMagickWand(magickWand2);
  }
  safeBmp *pImageL2 = safeBmpAlloc(pConf->mDetailedWidth, pConf->mDetailedHeight);
  *ptpImageL2 = pImageL2;
  Magick::MagickExportImagePixels(magickWand, 0, 0, pConf->mDetailedWidth, pConf->mDetailedHeight, "RGB", Magick::CharPixel, pImageL2->data);
  Magick::DestroyPixelWand(pixelWand);
  Magick::DestroyMagickWand(magickWand);
  return true;
}
#endif

bool CompositeSlide::testHeader(BYTE* fileHeader, int64_t length)
{
  std::string headerStr = (const char*) fileHeader;
  std::string header = "header";
  if (length >= 8 && headerStr.length() >= 8)
  {
    std::string chunk = headerStr.substr(1, 6);
//    std::cout << chunk << std::endl;
//    gchar* foldedChunkName = g_utf8_casefold((gchar*)chunk.c_str(), chunk.size());
//    gchar* foldedHeaderName = g_utf8_casefold((gchar*)header.c_str(), header.length());
    if (strcasecmp(chunk.c_str(), header.c_str())==0)
    {
      return true;
    }
  }
  return false;
}

