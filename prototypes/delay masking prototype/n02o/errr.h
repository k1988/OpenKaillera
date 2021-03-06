/******************************************************************************
***********   .d8888b.   .d8888b    *******************************************
**********   d88P  Y88b d88P  Y88b   ******************************************
*            888    888        888   ******************************************
*    8888b.  888    888      .d88P   ******************************************
*   888 "88b 888    888  .od888P"  ********************************************
*   888  888 888    888 d88P"    **********************************************
*   888  888 Y88b  d88P 888"        *******************************************
*   888  888  "Y8888P"  888888888            Open Kaillera: http://okai.sf.net
******************************************************************************/
#pragma once

#include <windows.h>

int n02ExceptionFilterFunction(_EXCEPTION_POINTERS *ExceptionInfo);

#ifdef TRACE


void _n02_TRACE(char * file, int line);

#define n02_TRACE() _n02_TRACE(__FILE__, __LINE__)

#else

#define n02_TRACE()

#endif

