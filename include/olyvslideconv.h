#ifndef __OLYVSLIDECONV__
#define __OLYVSLIDECONV__
#if defined(_WIN32) | defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
#define stat _stat
#define strcasecmp _stricmp
#else
#include <unistd.h>
#endif
#include "imagesupport.h"
#include "safebmp.h"
#include "blendbkgd.h"
#include "jpgcachesupport.h"
#include "jpgsupport.h"
#include "tiffsupport.h"
#include "composite.h"
#include "zipsupport.h"

#define SLIDE_OPTION_ON  "ON"
#define SLIDE_OPTION_OFF "OFF"

#define SLIDE_DEFAULT_DEBUG          0
#define SLIDE_DEFAULT_GOOGLE         SLIDE_OPTION_ON
#define SLIDE_DEFAULT_TIF            SLIDE_OPTION_OFF
#define SLIDE_DEFAULT_BLEND          SLIDE_OPTION_ON
#define SLIDE_DEFAULT_HIGHLIGHT      SLIDE_OPTION_OFF
#define SLIDE_DEFAULT_LOG            SLIDE_OPTION_OFF 
#define SLIDE_DEFAULT_OPENCV_ALIGN   SLIDE_OPTION_OFF
#define SLIDE_DEFAULT_REGION         SLIDE_OPTION_ON
#define SLIDE_DEFAULT_ZSTACK         SLIDE_OPTION_OFF
#define SLIDE_DEFAULT_MAX_MEM        256 
#define SLIDE_DEFAULT_MAX_JPEG_CACHE 256
#define SLIDE_DEFAULT_QUALITY        85
 
#define CONV_OPENCV_ALIGN     1
#define CONV_BLEND            (1 << 1)
#define CONV_REGION           (1 << 2)
#define CONV_HIGHLIGHT        (1 << 3)
#define CONV_ZSTACK           (1 << 4)
#define CONV_LOG              (1 << 5)
#define CONV_CUSTOM_XOFFSET   (1 << 6)
#define CONV_CUSTOM_YOFFSET   (1 << 7)
#define CONV_CUSTOM_GAMMA     (1 << 8)
#define CONV_ROTATE_180       (1 << 9)
#define CONV_SCANONLY         (1 << 10)
#define CONV_GOOGLE           (1 << 11)
#define CONV_TIF              (1 << 12)

#define LEVEL_TILED 1
#define LEVEL_SCANBKGD (1 << 1)

#define xstringfy(s) xstringfy2(s)
#define xstringfy2(s) #s

extern JpgCache jpgCache;
#endif
