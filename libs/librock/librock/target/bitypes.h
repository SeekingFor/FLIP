#ifndef librock_INC_BITYPES_H
#define librock_INC_BITYPES_H
/* This MUST NOT POLLUTE THE NAMESPACE BY INCLUDING ANYTHING!
   What should happen is that local.h should set up librock_COMPAT_
   macros, and this should use them.

  If you have to #include something from the system include
  directories, then you must not do it here.  Do it in
  librock/target/types.c.
 */
/*
License text in <librock/license/librock.txt> librock_LIDESC_HC=12440211096131f5976d36be0cddca4cd9152e45
*/

#include <librock/target/local.h>

#ifndef librock_bool
typedef unsigned int librock_bool;
#endif

/* Integral type sizes */
#if defined(librock_COMPAT_short16)
#ifndef librock_uint16_t
typedef unsigned short librock_uint16_t;
#endif
#ifndef librock_int16_t
typedef short librock_int16_t;
#define librock_uint16_t_defined
#endif
#endif

#if defined(librock_COMPAT_long32)
#ifndef librock_uint32_t
typedef unsigned long librock_uint32_t;
#define librock_uint32_t_defined
#endif

#ifndef librock_int32_t
typedef long librock_int32_t;
#endif

#else
#if defined(librock_COMPAT_int32)

#ifndef librock_uint32_t
typedef unsigned int librock_uint32_t;
#define librock_uint32_t_defined
#endif

#ifndef librock_int32_t
typedef int librock_int32_t;
#endif
#endif

#endif

#ifndef librock_uint32_t_defined
#error Did not define librock_uint32_t
#endif

#include <librock/target/local2.h>

#endif
