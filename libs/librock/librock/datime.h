
#ifndef librock_INC_DATIME_H
#define librock_INC_DATIME_H
/*
$Log: datime.h,v $
Revision 1.4  2002/04/09 03:55:08  forrest@mibsoftware.com rights=#1
  local/local2 bracket.
  extern "C" bracket.

Revision 1.3  2002/02/10 02:35:34  forrest@mibsoftware.com rights=#1
Check for librock_TIME_T and librock_STRUCT_TM before using.
Standardized chg log

Revision 1.2  2002/01/29 04:40:02  forrest@mibsoftware.com rights=#1
   Prep for publish.  API clean up, TAB, space at eol removal

Revision 1.1  2001/01/06 19:36:16  forrest@mibsoftware.com rights=#1
   Initial import to CVS

rights#1 Copyright (c) Forrest J Cavalier III d-b-a Mib Software
rights#1 License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
*/
#include <librock/target/local.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <librock/target/types.c>
#if 0
    int librock_ssdatime(librock_CONST char librock_PTR *sptr,struct tm librock_PTR *tmptr,time_t librock_PTR *ttime);
#endif
#ifdef librock_TIME_T
#ifdef librock_STRUCT_TM
int librock_ssgmtime(librock_CONST char librock_PTR *sptr,librock_STRUCT_TM librock_PTR *tmptr,librock_TIME_T librock_PTR *ttime);
int librock_mkgmtime(const librock_STRUCT_TM *argtmptr,librock_TIME_T *ret);
#endif
int librock_ssnetdatime(librock_CONST char librock_PTR *src,librock_TIME_T librock_PTR *pt);
#endif
int librock_ssmonth(librock_CONST char librock_PTR *sptr,int librock_PTR *iMonth);
    /* names: 3 letter or full, case insensitive */

#ifdef __cplusplus
};
#endif

#include <librock/target/local2.h>

#endif
