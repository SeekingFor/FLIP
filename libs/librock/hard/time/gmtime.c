/*  gmtime.c
librock_CHISEL _summary calculate a time_t from a struct tm according to UTC.

Copyright (c) 2001-2002, Forrest J. Cavalier III, doing business as
Mib Software, Saylorsburg Pennsylvania USA

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the author nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************/
#ifdef librock_NOTLIBROCK
/*  ABOUT THIS FILE: GUIDE TO QUICK REUSE

This file has many preprocessor conditional blocks which are
used in publishing http://www.mibsoftware.com/librock/
Here is an easy method to use this file without those features:

  1. At compile time, enable this conditional block by defining
     the preprocessor symbol librock_NOTLIBROCK, either with
     a compiler command line parameter, or as a #define in a
     source file which then #includes this source file.

  2. Define any preprocessor symbols in this block (if any)
     appropriately for your machine and compilation environment.

  3. Copy and use the declarations from this block in your
     source files as appropriate.

*/
#define librock_ISOLATED
#include <time.h>


#define librock_PTR
#define librock_CONST const
#define librock_PRIVATE static
#define librock_uint32_t unsigned long /* Match your target for 32-bit wide*/
#define librock_STRUCT_TM struct tm
#define librock_TIME_T time_t

#endif /* ifdef librock_NOTLIBROCK */
/**************************************************************/

#ifndef librock_ISOLATED
/**************************************************************/
#define librock_IMPLEMENT_gmtime
#include <time.h>
#include <librock/target/types.c>
#include <librock/datime.h>
/**************************************************************/
#endif

#ifdef librock_IMPL_LIDESC
#ifndef librock_NOIMPL_LIDESC_gmtime
/* License description features are documented at
   http://www.mibsoftware.com/librock/                        */
/**************************************************************/
#include <librock/license/librock.lh> /* librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45 */
void *librock_LIDESC_gmtime[] = {
    "\n" __FILE__ librock_LIDESC_librock "\n",
    0
};
/**************************************************************/
#endif
#endif

#ifndef librock_WRAP
/* Function wrapping and tracing features are documented at
   http://www.mibsoftware.com/librock/                        */
/**************************************************************/
#define librock_body_mkgmtime librock_mkgmtime /* For librock_WRAP section */
/**************************************************************/
#endif
/**************************************************************/

#ifndef librock_NOIMPL_monthtoseconds
#define isleap(y) ( !((y) % 400) || (!((y) % 4) && ((y) % 100)) )
librock_PRIVATE time_t monthtoseconds(int isleap,int month)
{
static const long        _secondstomonth[12] = {
        0,
        24L*60*60*31,
        24L*60*60*(31+28),
        24L*60*60*(31+28+31),
        24L*60*60*(31+28+31+30),
        24L*60*60*(31+28+31+30+31),
        24L*60*60*(31+28+31+30+31+30),
        24L*60*60*(31+28+31+30+31+30+31),
        24L*60*60*(31+28+31+30+31+30+31+31),
        24L*60*60*(31+28+31+30+31+30+31+31+30),
        24L*60*60*(31+28+31+30+31+30+31+31+30+31),
        24L*60*60*(31+28+31+30+31+30+31+31+30+31+30)
};
        long ret;
        if ((month > 11)||(month < 0)) {
                return 0;
        }
        ret = _secondstomonth[month];
        if (isleap && (month > 1)) {
                return ret + 24L * 60 * 60;
        }
        return ret;
}
#endif

#ifndef librock_NOIMPL_yeartoseconds
librock_PRIVATE time_t yeartoseconds(int y)
{
        time_t ret;
        if (y < 1970) {
                return 0;
        }
        ret = (y - 1970)*365L*24*60*60;
        /* Add in a day for previous leap years */
        /* We don't bother with the century rules, because
           Y2000 is a leap year, and the time_t will overflow
           by Y2100.
        */
        ret += (((y-1) - 1968) / 4)*24L*60*60;
        return ret;
} /* secondstoyear */
#endif


#ifndef librock_NOIMPL_mkgmtime
/**************************************************************/
int librock_body_mkgmtime(const librock_STRUCT_TM librock_PTR *argtmptr,librock_TIME_T librock_PTR *ret)
{/* Copyright 1998-2002, Forrest J. Cavalier III d-b-a Mib Software
  Licensed under BSD-ish license, NO WARRANTY. Copies must retain this block.
  License, originals, details: http://www.mibsoftware.com/librock/
*/
#ifdef librock_MANUAL_mkgmtime
/*<NAME>
librock_mkgmtime - calculate a time_t from a struct tm according to UTC.
*/
/*<SYNOPSIS>*/
#include <librock/target/types.c>
#include <librock/datime.h>

librock_TIME_T
librock_body_mkgmtime(
    const librock_STRUCT_TM *tmptr,
    librock_TIME_T *ret
);

/*<DESCRIPTION>
librock_mkgmtime() takes a pointer to a struct tm (cast to a
librock_STRUCT_TM) with values in UTC, and returns a time_t,
cast to librock_TIME_T.  (The casts are part of the portability
provided by librock/target/types.c, and normally they will
be simply struct tm and time_t.)

This operates as ANSI mktime(), but interprets the members of
struct tm according to UTC.
 tm_year is the year-1900.
 tm_mon is the month 0-11
 tm_mday is the day of month 1-31
 tm_hour is the hour 0-23
 tm_min is the minute 0-59
 tm_sec is the second 0-59.
 tm_isdst is ignored.

Typical use is to convert stored dates and times in UTC to a time_t.
For something that scans strings of GMT dates and times, see
librock_ssgmtime().

*/
#ifdef librock_TYPICAL_USE_mkgmtime
    #include <time.h>
    struct tm tmbuf;
    time_t t;
    tmbuf.tm_year = 2002-1900;
    tmbuf.tm_mon = 1;
    tmbuf.tm_mday = 2;
    tmbuf.tm_hour = 2;
    tmbuf.tm_min = 2;
    tmbuf.tm_sec = 2;
    tmbuf.tm_isdst = 0; /* Ignored */
    librock_mkgmtime(&tmbuf,&t);
    printf("%ld\n",(long) t);
#endif

/*<USES>
 //No external calls.
*/

/*<LICENSE>  Copyright 1998-2002 Forrest J. Cavalier III, http://www.mibsoftware.com
  Licensed under BSD-ish license, NO WARRANTY. Copies must retain this block.
  License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
*/
#endif /* MANUAL */

/*<IMPLEMENTATION>-------------------------------------------*/
/*mktime() converts from local time, and it is not always possible to
figure out the timezone offset portably and reliably.*/
    const struct tm librock_PTR *ptmbuf = (struct tm librock_PTR *)argtmptr;
    time_t t;
    int year = ptmbuf->tm_year + ptmbuf->tm_mon / 12;
    t = yeartoseconds(year+1900);
    t += monthtoseconds(isleap(year+1900),
                                                ptmbuf->tm_mon % 12);
    t += (ptmbuf->tm_mday - 1) * 3600L * 24;
    t += ptmbuf->tm_hour * 3600L + ptmbuf->tm_min * 60L + ptmbuf->tm_sec;
    * (time_t librock_PTR *)ret = t;
    return 0;
}
/**************************************************************/
#endif /* NOIMP section */
/*
$Log: gmtime.c,v $
Revision 1.6  2002/08/01 20:24:31  forrest@mibsoftware.com rights=#1
  Updated TYPICAL_USE section.
  Added NOTLIBROCK section.
  Moved CVS log to end.
  Changed LIDESC MD5 to HC.

Revision 1.5  2002/04/09 03:39:35  forrest@mibsoftware.com rights=#1
  Added FNTYPEs.
  Updated LICENSE in manual pages.

Revision 1.4  2002/03/18 19:25:25  forrest@mibsoftware.com rights=#1
 ifdef bracketing.
 manual page.

Revision 1.3  2002/02/11 14:38:29  forrest@mibsoftware.com rights=#1
Docs

Revision 1.2  2002/02/11 14:35:40  forrest@mibsoftware.com rights=#1
[doc] Fix chg log

Revision 1.1  2002/02/11 14:34:09  forrest@mibsoftware.com rights=#1
Initial

rights#1 Copyright (c) Forrest J Cavalier III d-b-a Mib Software
rights#1 License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
*/
