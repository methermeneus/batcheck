/*********************************************************************
 *                                                                   *
 * laptop.c: Laptop version of batcheck                              *
 * (c) 2017 R. A. Grant (methermeneus@gmail.com)                     *
 *     Adapted from code written in 2008                             *
 * This code is freely available for use and adaptation.             *
 * Filename:  laptop.c                                               *
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
 * Version 2.0: Massively cleaned up. Part of batcheck V 2.0.        *
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

#define TEMPFILE    "/sys/bus/acpi/devices/LNXTHERM:00/thermal_zone/temp"
#define TOTALFILE   "/sys/class/power_supply/BAT1/charge_full"
#define CURRENTFILE "/sys/class/power_supply/BAT1/charge_now"
#define ACFILE      "/sys/class/power_supply/ACAD/online"
#define TIMELEN     21 // Current format, time should be 20 chars.
#ifdef TIOCGWINSZ
#define TERMSIZE    TIOCGWINSZ
#elif defined TIOCGSZ
#define TERMSIZE    TIOCGSZ
#endif

/*********************************************************************
 * ACPI files: /sys/class/power_supply/BAT1/                         *
 *    charge_full = max charge in uWh                                *
 *    charge_now = current charge in uWh                             *
 *    similar files exist for current in uA and voltage in uV        *
 *    status = charging/discharging, but...                          *
 * /sys/class/power_supply/ACAD/online = 1 or 0 for plugged in or    *
 *    not                                                            *
 ********************************************************************/

int main (int argc, char *argv[], char *env[]) {
	// If we can't fork the process, there's not much point.
	pid_t pid = daemon (0, 1);
	if (pid) {
		fprintf (stderr, "Can't fork process: %s\n", strerror (errno));
		return (errno);
	}
	// Else, lots of variables.
	struct tm      *currTime;
	struct winsize xy;
	       char    timeStr[TIMELEN];
		   char    tempStr[8]; // @Ambiguous: This means temporary, not temperature.
	const  char    *tempPath       = TEMPFILE;
	const  char    *maxChargePath  = TOTALFILE;
	const  char    *currChargePath = CURRENTFILE;
	const  char    *acPath = ACFILE;
		   char    acFlag          = '\0';
		   char    suffix          = '\0';
	       float   tempC           = 0.0;
	       float   tempF           = 0.0;
	       float   maxCharge       = 0.0;
	       float   currCharge      = 0.0;
	       int     x               = 0;
	       int     percent         = 0;
	       int     textColor       = 36;
	       int     textStyle       = 1;
	       FILE    *infoFile;
	       time_t  raw;

	while (1) {
		time (&raw);
		currTime = localtime (&raw);
		strftime (timeStr, 21, "%a %b %d, %X", currTime);

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
		
		// This SHOULD be constant, but I've had crappy laptops where
		// it wasn't, so we want to know where we are in relation to
		// the current maximum.
		infoFile = fopen (maxChargePath, "r");
		if (infoFile == NULL) {
			perror ("Error opening max charge file:");
			return (errno);
		}
		if (fgets (tempStr, 8, infoFile) == NULL) {
			perror ("Error retrieving max charge:");
			return (errno);
		}
		fclose (infoFile);
		maxCharge = atof (tempStr);

		infoFile = fopen (currChargePath, "r");
		if (infoFile == NULL) {
			perror ("Error opening current charge file:");
			return (errno);
		}
		if (fgets (tempStr, 8, infoFile) == NULL) {
			perror ("Error retrieving current charge:");
			return (errno);
		}
		fclose (infoFile);
		currCharge = atof (tempStr);
		// In case we somehow have an overcharge:
		if (currCharge >= maxCharge) percent = 100;
		else percent = (int) ((currCharge / maxCharge) * 100);

		infoFile = fopen (acPath, "r");
		if (infoFile == NULL) {
			perror ("Error opening AC status file:");
			return (errno);
		}
		acFlag = fgetc (infoFile);
		if (acFlag == EOF) {
			perror ("Error retrieving AC status:");
			return (errno);
		}
		fclose (infoFile);
		acFlag -= 48; // make 1 or 0 instead of '1' or '0'
		// Mnemonic: I looks like a battery, Z like a lightning bolt.
		suffix = (acFlag ? 'Z' : 'I');

#ifndef TERMSIZE
		fprintf (stderr, "Error, could not retrieve terminal size.\n");
		return (EXIT_FAILURE);
#endif
		ioctl (1, TERMSIZE, &xy);
		x = xy.ws_col - TIMELEN;

		printf ("\0337\033[%d;%dm\033[1;%dH[%s]", textStyle, textColor, x, timeStr);
		// \033 = escape sequence
		// 7 = save cursor location
		// [x;ym = text-style x, color y
		// [y;xH = position (x,y) (in fixed-width character units, from
		//		origin at top left of screen)
		// text format 1, color 36 (cyan), first line( from top), xth column (from right), string
		x += 4; // center smaller text
		printf ("\033[2;%dH[%2.1fC] [%2.1fF]", x, tempC, tempF);
		// [y;xH again, down one row
		x += 4; // smaller text again
		printf ("\033[3;%dH[%3d%%%c]\033[0m\0338", x, percent, suffix);
		// [y;xH one more row down
		// [0m = default color and text style
		// 8 = return to saved cursor location
		fflush (stdout);
		usleep (250);
	}
	// If we get here, there's a problem.
	return (EXIT_FAILURE);
}

