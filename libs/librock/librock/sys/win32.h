/*    Summary/Purpose: define macros to describe this 
 *                       compiler+architecture platform
 */
/* This can use from the global namespace, but it should NOT
   put anything there unless it is prefixed by librock_
   It should not include anything which puts something there.

 */

#if 0
#if _MSC_VER == 800
#define librock_COMPAT_short16
#define librock_COMPAT_int16
#define librock_COMPAT_long32
#endif
#if _MSC_VER >= 1000
/* MSVC 4.0 */
#define librock_COMPAT_short16
#define librock_COMPAT_int32
#define librock_COMPAT_long32
#endif
#endif

/* Other windows compatibility */
#define librock_offset_t long

#define librock_INC_direct_HAS_mkdir 1
#define librock_mkdir_1ARG 1
#define librock_INC_io_HAS_open 1
#define librock_INC_sys_utime_HAS_utime 1

#define librock_USESTRICMP 1

#ifndef librock_USESTRICMP
#define librock_USESTRICMP 1
#endif

#define librock_WANT_opendir
#define librock_WANT_opendir_win32
#define librock_WANT_dbiodbc_win32
#define librock_WANT_pthread_win32
#define librock_WANT_socketserve_nonblock_WINDOWS

#define librock_INC_winsock_HAS_socket
#define librock_INC_conio_HAS__getch

#undef librock_EXPORT
#define librock_EXPORT __declspec( dllexport )

#define librock_O_BINARY O_BINARY
#define librock_O_CREAT O_CREAT
#define librock_O_EXCL O_EXCL
#define librock_O_RDWR O_RDWR

#define librock_WANT_FOPEN_MODE_B

#define librock_STARTUP_STDOUT_IS_TEXTMODE 1 /* added 20040719 */

#define librock_OPEN_MODE_RW _S_IREAD|_S_IWRITE

