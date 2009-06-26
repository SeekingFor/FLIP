#ifndef librock_INC_TYPES_C
#define librock_INC_TYPES_C

/*
librock_CHISEL _summary Declare common structures and types so that we can (portably) use them on many platforms.

License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
librock_CHISEL pubwww .h
librock_CHISEL obj .h
*/

/* There are a number of portability and namespace
   challenges addressed by this file.

  The "namespace safety requirements" are:
      1. ensure that anything we put into the global namespace
         at compile time (defines, typedefs, and declarations of
         struct, class, and functions) is prefixed by librock_.

      2. ensure that anything we define in the "extern" namespace
         at link time is prefixed by librock_.

      3. ensure that declarations are exactly the same when compiling
         the library and the clients.

      4. Work properly without assuming anything in the global
         namespace, except the C built-in names and types.


  Point #4 causes a problem when functions accept parameters of
  non-built-in types which interoperate with other libraries,
  such as the standard C library.  (One example is parameters of
  type FILE *, used by the buffered file operations declared by
  <stdio.h>.)

  The client has 3 choices:
  A. Don't #include this file, and librock header files will
     only declare functions with built-in types.

  B. #include this file, which will declare dummy definitions
     of the interoperative types.  The caller must use casts
     when necessary.  (The librock implementation uses casts
     if it passes such parameters to other functions.)

     This disables most type checking for those parameters, but
     will always result in correct code when the parameter passing
     for each type is identical.  It keeps the namespace
     entirely pure.

  C. Decide to #include files to declare the interoperating types.
     (available when the library was compiled to support it. More
     below.)

     This will get the benefit that librock can use these special
     type declarations, but other #include files stick a lot of unprefixed
     declarations into the global namespace.  (Since most programmers
     know what to expect, there is little chance of surprise.)

     Note that because of requirement #3, we don't simply check #ifdef
     _INC_STDIO, for example.  The only way we can satisfy #3 (ensuring
     the client and library compile-time environments are identical)
     is to #include such files ourselves, and to have the controlling
     macros declared in the target/local.h file.

It is because of mode C of operation that this file has a .c extension,
instead of .h.  (The librock convention is that files are .h
only if all the names it declares are prefixed librock_.)

 */

#include <librock/target/local.h>

#ifdef librock_TYPES_INC_TIME
#include <time.h>
#endif

#ifdef librock_TYPES_INC_STDARG
#include <stdarg.h>
#endif

#ifndef librock_VALIST
#ifdef librock_TYPES_INC_STDARG
#define librock_VALIST va_list
#else
struct librock_VALIST_s;
#define librock_VALIST struct librock_VALIST_s *
#endif
#endif

#ifdef librock_TYPES_INC_STDIO
/* NOTE: #defining this macro will #include <stdio.h> from
   librock/target/types.c. which will allow librock_ to
   use the types declared there, but <stdio.h> will populate the
   compile-time namespace with names not prefixed librock_.
 */
#include <stdio.h>
#endif

#ifdef WIN32
#include <librock/sys/win32.h>
#endif

#include <librock/target/bitypes.h>

/**************************************************************/
#ifndef librock_FILE

#ifdef librock_TYPES_INC_STDIO
#define librock_FILE FILE
#else
struct librock_FILE_s;
#define librock_FILE struct librock_FILE_s
#endif

#endif
/**************************************************************/


/**************************************************************/
#ifndef librock_SIZE_T
#ifdef librock_TYPES_INC_STDIO
#define librock_SIZE_T size_t
#else
#define librock_SIZE_T unsigned long
#endif
#endif
/**************************************************************/


/**************************************************************/
#ifndef librock_OFFSET_T
#ifdef librock_TYPES_INC_STDIO
#define librock_OFFSET_T offset_t
#else
#define librock_OFFSET_T long
#endif

#endif
/**************************************************************/

/**************************************************************/
#ifndef librock_TIME_T
#ifdef librock_TYPES_INC_TIME
#define librock_TIME_T time_t
#else
struct librock_TIME_T_s;
#define librock_TIME_T struct librock_TIME_T_s
#endif
#endif

#ifndef librock_STRUCT_TM
#ifdef librock_TYPES_INC_TIME
#define librock_STRUCT_TM struct tm
#else
struct librock_STRUCT_TM_s;
#define librock_STRUCT_TM struct librock_STRUCT_TM_s
#endif
#endif

/**************************************************************/

#define librock_PRIVATE static
#define librock_CONST const

#define librock_FLATMEM  /* all pointers are same size */
#ifndef librock_EXPORT
#define librock_EXPORT
#endif
#define librock_FAR
#define librock_HUGE
#define librock_NEAR
#define librock_PTR

#define librock_PASCAL

#define librock_ALIGN 0

#define librock_NAME_MAX 1024
#include <librock/target/local2.h>
#endif

/*
$Log: types.c,v $
Revision 1.5  2002/04/09 03:47:32  forrest@mibsoftware.com rights=#1
  Portability and other fixes.

Revision 1.4  2002/02/10 02:47:38  forrest@mibsoftware.com rights=#1
Space/tab cleanup

Revision 1.3  2002/02/10 02:45:52  forrest@mibsoftware.com rights=#1
librock_TIME_T,librock_OFFSET_T,librock_STRUCT_TM
Standardized chg log

Revision 1.2  2002/01/29 20:20:23  forrest@mibsoftware.com rights=#1
   CHISEL directions, added librock_O_BINARY

Revision 1.1  2002/01/29 14:24:50  forrest@mibsoftware.com rights=#1
   initial

rights#1 Copyright (c) Forrest J Cavalier III d-b-a Mib Software
rights#1 License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
*/
