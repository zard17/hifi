//
//  Systime.cpp
//  libraries/shared/src
//
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifdef WIN32

#include "Systime.h"

/**
 * gettimeofday
 * Implementation according to:
 * The Open Group Base Specifications Issue 6
 * IEEE Std 1003.1, 2004 Edition
 */
  
/**
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Contributed by:
 *  Danny Smith <dannysmith@users.sourceforge.net>
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/** Offset between 1/1/1601 and 1/1/1970 in 100 nanosec units */
#define _W32_FT_OFFSET (116444736000000000ULL)

int gettimeofday(timeval* p_tv, timezone* p_tz) {
  
  union {
    unsigned long long ns100; /**time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  }  _now;

  if (p_tv) {
      GetSystemTimeAsFileTime (&_now.ft);
      p_tv->tv_usec=(long)((_now.ns100 / 10ULL) % 1000000ULL );
      p_tv->tv_sec= (long)((_now.ns100 - _W32_FT_OFFSET) / 10000000ULL);
  }
  
  /** Always return 0 as per Open Group Base Specifications Issue 6.
   Do not set errno on error.  */
  return 0;
}

#endif