/*********************************************************************
 *                                                                   *
 * desktop.c: Desktop version of batcheck                            *
 * (c) 2013 R. A. Grant (methermeneus@gmail.com)                     *
 * This code is freely available for use and adaptation.             *
 *                                                                   *
 * Dependencies: This program requires access to information         *
 *    by the ACPI tool. To get ACPI on most Linux platforms:         *
 *       $ apt-get install acpi                                      *
 *       $ yum install acpi                                          *
 *       or find the appropriate package for your flavor of Linux    *
 *                                                                   *
 * @TODO: Get info directly from the system's ACPI table, instead    *
 *    of making the ACPI package do all the hard work.               *
 *                                                                   *
 * Purpose: Displays time and core temperature in the upper-right    *
 *    corner of a tty or terminal emulator. Useful for people who,   *
 *    like me, spend a lot of time with a terminal blocking the      *
 *    desktop clock.                                                 *
 * Works best when called by bashrc.                                 *
 *                                                                   *
 * Version 1.1: Cleaned up a little. Part of batcheck V 2.0.         *
 * Currently Linux-only. Might work on POSIX-standard systems, but   *
 *    currently untested.                                            *
 *                                                                   *
 ********************************************************************/

#include <stdio.h> // FILE, [f]printf, fgets, fgetc, perror, fflush
#include <stdlib.h> // atof, NULL, EXIT_FAILURE
#include <errno.h> // errno
#include <sys/ioctl.h> // ioctl, struct winsize
#include <unistd.h> // usleep, daemon, pid_t
#include <string.h> // strerror; Yes, I know strerror isn't threadsafe.
                    // Do you really think this dinky program is going to
                    // implicitly fork threads?
#include <time.h> // struct tm, time_t, localtime, strftime

#define TEMPFILE "/sys/bus/acpi/devices/LNXTHERM:00/thermal_zone/temp"
#define TIMELEN 21 // Current format, time should be 20 chars.

// @TODO: Multi-platform? This thing's so small, I won't need to do much
// to make it work for Windows or Mac. I'll need to include different
// build files, though. And find a different way to get the info from the
// computer.

int main (int argc, char *argv[], char *env[]) {
    // If we can't fork the process, there's not much point.
    pid_t pid = daemon (1,1);
    if (pid) {
        fprintf (stderr, "Error, cannot fork process: %s\n", strerror (errno));
        return (errno);
    }
    // Else, lots of variables.
    struct tm      *currTime;
    struct winsize xy;
           char    timeStr[TIMELEN];
           char    tempStr[6];
    const  char    *tempPath = TEMPFILE;
           float   tempC     = 0.0;
           float   tempF     = 0.0;
           int     x         = 0;
           int     textColor = 36;
           int     textStyle = 1;
           FILE    *infoFile;
           time_t  raw;
	
    while (1) {
        time (&raw);
        currTime = localtime (&raw);
        strftime (timeStr, TIMELEN, "%a %b %d, %X", currTime);

        infoFile = fopen (tempPath, "r");
        if (infoFile == NULL) {
            perror ("Error opening temperature file:");
            return (errno);
        }
        if (fgets (tempStr, 6, infoFile) == NULL) {
            perror ("Error retrieving temperature:");
            return (errno);
        }
        fclose (infoFile);
        tempC = atof (tempStr) / 1000.0;
        tempF = (tempC + 32.0) * (9.0 / 5.0);

// Putting the function inside the ifdef helps with terminal window
// resizing
#ifdef TIOCGWINSZ
        ioctl (1, TIOCGWINSZ, &xy);
#elif defined TIOCGSZ
        ioctl (1, TIOCGSZ, &xy);
#else
        fprintf (stderr, "Error, could not retrieve terminal size.\n");
        return (EXIT_FAILURE);
#endif
        x = xy.ws_col - TIMELEN;
		printf ("\033[?25l\0337\033[1;%dH\033[K\033[B\033[K", x);
        // \033 = escape sequence
		// [?25l = hide cursor
        // 7 = save cursor location
        // [y;xH = position (x,y) (in fixed-width character units, from
        //        origin at top left of screen)
		// [nK: clear in line; 0 (default): cursor to end of line
		// [nB: move down n lines (default 1)
        printf ("\033[%d;%dm\033[1;%dH[%s]", textStyle, textColor, x, timeStr);
        // [x;ym = text-style x, color y
		printf ("\033[B\033[%dD[%2.1fC] [%3.1fF]\033[0m\0338\033[?25h", TIMELEN - 2, tempC, tempF);
		// [nD: move cursor back n spaces (default 1)
        //printf ("\033[2;%dH[%2.1fC] [%3.1F]\033[0m\0338", x, tempC, tempF);
        // [0m = default color and text style
        // 8 = return to saved cursor location
		// [?25h = restore cursor
        fflush (stdout);
        usleep (5000);
    }
    // If we get here, something went wrong
    fprintf (stderr, "Error, unknown error in info display.\n");
    return (EXIT_FAILURE);
}
