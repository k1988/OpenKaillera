/******************************************************************************
          .d8888b.   .d8888b.  
         d88P  Y88b d88P  Y88b 
         888    888        888 
88888b.  888    888      .d88P 
888 "88b 888    888  .od888P"  
888  888 888    888 d88P"      
888  888 Y88b  d88P 888"       
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#pragma once

namespace n02 {

#if (!defined(DISABLE_TRACE) || defined(FORCE_GLOBAL_TRACE)) && !defined(DISABLE_ALL_TRACING)
#define DO_TRACE
#else
#ifdef DISABLE_TRACE
#undef DISABLE_TRACE
#endif /* DISABLE_TRACE */
#endif /* (!defined(DISABLE_TRACE) || defined(FORCE_GLOBAL_TRACE)) && !defined(DISABLE_ALL_TRACING) */

    void _n02_trace(const char * file, const char * function, int line);
    void trace_log();

#ifdef DO_TRACE
#define TRACE() n02::_n02_trace(__FILE__, "", __LINE__)
#else
#define TRACE()
#endif /* DO_TRACE */

};
