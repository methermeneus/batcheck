/********************************************************************
 *                                                                  *
 * desktop.c: Desktop version of batcheck                           *
 * (c) 2013 R. A. Grant (methermeneus@gmail.com)                    *
 * This code is freely available for use and adaptation.            *
 *                                                                  *
 * Dependencies: This program requires access to information        *
 *    by the ACPI tool. To get ACPI on most Linux platforms:        *
 *       $ apt-get install acpi                                     *
 *       $ yum install acpi                                         *
 *       or find the appropriate package for your flavor of Linux   *
 *                                                                  *
 * @TODO: Get info directly from the system's ACPI table, instead   *
 *    of making the ACPI package do all the hard work.              *
 *                                                                  *
 * Purpose: Displays time and core temperature in the upper-right   *
 *    corner of a tty or terminal emulator. Useful for people who,  *
 *    like me, spend a lot of time with a terminal blocking the     *
 *    desktop clock.                                                *
 *                                                                  *
 * Version 1.1: Cleaned up a little. Part of batcheck V 2.0.        *
 * Currently Linux-only. Might work on POSIX-standard systems, but  *
 *    currently untested.                                           *
 *                                                                  *
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
#ifdef TIOCGWINSZ
#define TERMSIZE TIOCGWINSZ
#elif defined TIOCGSZ
#define TERMSIZE TIOCGSZ
#endif

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
	       char    currTimeStr[TIMELEN];
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
		strftime (currTimeStr, TIMELEN, "%a %b %d, %X", currTime);

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
		tempF = (tempC + 32) * (9.0 / 5.0);
		
#ifndef TERMSIZE
		fprintf (stderr, "Error, could not retrieve terminal size.\n");
		return (EXIT_FAILURE);
#endif
		ioctl (1, TERMSIZE, &xy);
		x = xy.ws_col - 21;

		printf ("\0337\033[%d;%dm\033[1;%dH[%s]", textStyle, textColor, x, currTimeStr);
		// \033 = escape sequence
		// 7 = save cursor location
		// [x;ym = text-style x, color y
		// [y;xH = position (x,y) (in fixed-width character units, from
		//		origin at top left of screen)
		x += 4;
		printf ("\033[2;%dH[%2.1fC] [%2.1F]\033[0m\0338", x, tempC, tempF);
		// [y;xH again, down one row
		// [0m = default color and text style
		// 8 = return to saved cursor location
		fflush (stdout);
		usleep (250);
	}
	// If we get here, something went wrong
	fprintf (stderr, "Error, unknown error in info display.\n");
	return (EXIT_FAILURE);
}
