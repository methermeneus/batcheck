/*********************************************************************
 *                                                                   *
 * batcheck.c: Time, core temperature, and battery life terminal     *
 *     display                                                       *
 * (c) 2017 R. A. Grant (methermeneus@gmail.com)                     *
 *     Adapted from code written in 2008                             *
 * This code is freely available for use and adaptation.             *
 * Filename:  batcheck.c                                             *
 *                                                                   *
 * Dependencies: This program requires access to information         *
 *    by the ACPI tool. To get ACPI on most Linux platforms:         *
 *       $ apt-get install acpi                                      *
 *       $ yum install acpi                                          *
 *       or find the appropriate package for your flavor of Linux    *
 *                                                                   *
 * @TODO: Get info directly from the system's ACPI table, instead    *
 *    of making the ACPI package do all the hard work.               *
 * @TODO: Check if the above is relevant, seeing as we're using stuff*
 *    Linux defines for any info taken from ACPI drivers...          *
 *    Possibly, reinstall Linux on the laptop and see if this works  *
 *    without apt-get install acpi?                                  *
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
 *
 *
 *                                                                   *
 ********************************************************************/
#include <stdio.h> // FILE, [f]printf, fgets, fgetc, perror, fflush
#include <stdlib.h> // atof, NULL, EXIT_FAILURE
#include <errno.h> // errno
#include <sys/ioctl.h> // ioctl, struct winsize
#include <unistd.h> // usleep, daemon, pid_t, access
#include <string.h> // strerror; Yes, I know strerror isn't threadsafe.
                    // Do you really think this dinky program is going to
                    // implicitly fork threads?
#include <time.h> // struct tm, time_t, localtime, strftime

#define TIMELEN     21 // Current format, time should be 20 chars.
#define TEMPFILE    "/sys/bus/acpi/devices/LNXTHERM:00/thermal_zone/temp"
// @CLEANUP: Move stuff around so there aren't so many #ifdef LAPTOPs
// lying around. We should only need... two, I think? We'll see.
#ifdef LAPTOP // Only a laptop is gonna need battery info.
#define TOTALFILE   "/sys/class/power_supply/BAT1/charge_full"
#define TOTALFILE_OLD   "/sys/class/power_supply/BAT0/charge_full"
#define CURRENTFILE "/sys/class/power_supply/BAT1/charge_now"
#define CURRENTFILE_OLD "/sys/class/power_supply/BAT0/charge_now"
#define ACFILE      "/sys/class/power_supply/ACAD/online"
#endif // LAPTOP

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
	// The path to the battery charge files has had several changes in
	// the last few Linux kernel versions; Checking is a lot easier if
	// the variables aren't const.  I'm only checking the two latest
	// paths here; may do more later if it ever comes up. -RAG 20170224
	// Again, battery stuff is only relevant for laptops.
#ifdef LAPTOP
           char    *maxChargePath  = TOTALFILE;
           char    *currChargePath = CURRENTFILE;
    const  char    *acPath = ACFILE;
           char    acFlag          = '\0';
           char    suffix          = '\0';
           float   maxCharge       = 0.0;
           float   currCharge      = 0.0;
           int     percent         = 0;
#endif // LAPTOP
           float   tempC           = 0.0;
           float   tempF           = 0.0;
           int     x               = 0;
           int     textColor       = 36;
           int     textStyle       = 1;
           FILE    *infoFile;
           time_t  raw;
#ifdef LAPTOP
	// Access returns 0 if ok, else -1 and set errno
	// @TODO: Maybe use preprocessor to query Linux version to set this? -RAG 20170224
	//        kernel < 2.6.24: /proc/acpi/battery/BAT0/
	//        2.6.24 <= kernel < 3.19: /sys/class/power_supply/BAT0/
	//        kernel >= 3.19: /sys/class/power_supply/BAT1/
	if (access (maxChargePath, R_OK)) {
		maxChargePath = TOTALFILE_OLD;
		if (access (maxChargePath, R_OK)) {
			perror ("Error finding total charge file.\nThis program may not be compatible with your Linux kernel: ");
			return (errno);
		}
	}
	if (access (currChargePath, R_OK)) {
		currChargePath = CURRENTFILE_OLD;
		if (access (currChargePath, R_OK)) {
			perror ("Error finding current charge file.\nThis program may not be compatible with your Linux kernel: ");
			return (errno);
		}
	}
#endif // LAPTOP
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

#ifdef LAPTOP
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
#endif // LAPTOP
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
        // [y;xH again, down one row
		printf ("\033[%d;%dm\033[1;%dH[%s]", textStyle, textColor, x, timeStr);
		// [x;ym = text-style x, color y
		printf ("\033[B\033[%dD[%2.1fC] [%3.1fF]", TIMELEN - (tempF >= 100.0 ? 2 : 3), tempC, tempF);
		// [nD: move cursor back n spaces (default 1)
#ifdef LAPTOP
        printf ("\033[B\033[%dD[%3d%%%c]\033[0m\0338\033[?25h", TIMELEN - 10, percent, suffix);
        // [0m = default color and text style
        // 8 = return to saved cursor location
		// [?25h = restore cursor
#else
		// It feels right to include these in the final printf() call,
		// but laptop and desktop have different final calls, so tack
		// this onto the desktop version.
		printf ("\033[0m\0338\033[?25h");
#endif // LAPTOP
        fflush (stdout);
        usleep (5000);
    }
    // If we get here, there's a problem.
    return (EXIT_FAILURE);
}

