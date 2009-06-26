/*    Summary/Purpose: define macros to describe this 
 *                       compiler+architecture platform
 */
/* This can use from the global namespace, but it should NOT
   put anything there unless it is prefixed by librock_
   It should not include anything which puts something there.

 */

#define librock_INC_unistd_HAS_unlink 1
#define librock_INC_unistd_HAS_select 1 /* 20040617 */
#define librock_INC_sys_stat_HAS_mkdir 1
#define librock_INC_fcntl_HAS_open 1
#define librock_INC_utime_HAS_utime 1

/*out 20040617 #define librock_INC_sys_socket_HAS_socket 1*/
#define librock_INC_sys_types_AND_sys_socket_HAS_socket 1
#define librock_INC_STYLE1_inet_addr 1 /* 20040617 */
#define librock_INC_netdb_HAS_gethostbyname 1 /* 20040617 */

#define librock_O_BINARY 0
#define librock_O_CREAT O_CREAT
#define librock_O_EXCL O_EXCL
#define librock_O_RDWR O_RDWR

#define librock_OPEN_MODE_RW 0600
