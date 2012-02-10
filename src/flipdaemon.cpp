#include "flipdaemon.h"

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#ifndef _WIN32
	#include <unistd.h>
	#include <sys/stat.h>
#else
	#include <windows.h>
#endif

/*
	modified from http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
*/
void Daemonize()
{
#ifndef _WIN32
    pid_t pid, sid;

    /* already a daemon */
    if ( getppid() == 1 ) return;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* At this point we are executing as the child process */

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Redirect standard files to /dev/null */
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);
#else
	HWND hWnd = GetConsoleWindow();
	ShowWindow( hWnd, SW_HIDE );
#endif
}

void Undaemonize()
{
#ifdef _WIN32
	HWND hWnd = GetConsoleWindow();
	ShowWindow( hWnd, SW_SHOWNORMAL );
#endif
}
