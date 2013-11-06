/*
 * =====================================================================================
 *
 *       Filename:  batcheck.c
 *
 *    Description:  Simple program to display time and battery life in a
 *    terminal window. To use:
 *	Extract c source and makefile.
 *	Compile with $ make.
 *	Put executable wherever you want to keep it.
 *	Add to ~/.bashrc:
 *		if [ -f /path/to/batcheck ]
 *		then
 *			/path/to/batcheck
 *		fi
 *
 *	To change color, find "int color" and change the number.
 *	To change the time format, find "strftime (" and change the time
 *		format there.
 *
 *	Apologies for the over-commented code; I'm a bit of a beginner
 *		and wanted to be sure I could remember what I was doing.
 *
 *        Version:  1.0
 *        Created:  11/05/2013 09:15:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  R. A. Grant (RAG), methermeneus@gmail.com
 *   Organization:  Wholly Crap Productions
 *
 * =====================================================================================
 */
#include <stdio.h> /* for fputs, fprintf, etc */
#include <stdlib.h> /* for malloc () */
#include <unistd.h> /* for sleep() */
#include <sys/ioctl.h> /* for termnal functions */
#include <time.h> /* for time functions */
#include <errno.h> /* Error handling */
#include <string.h> /* needed for strerr() */

/*  Quick way to get battery charge. Linux stores charge as
 *  /sys/class/power_supply/BAT0:
 * 		charge_full = max charge in uWh
 * 		charge_now = current charge in uWh
 * 		similar files for current (uA) and voltage (uV)
 * 		status = charging/discharging *
 * 	percentage is calculated as charge_now / charge_full
 *
 * 	* Easier charge/discharge flag is /sys/class/power_supply/AC/online:
 * 		1/0 = y/n
 */

void check (
	char* output,
	char* max,
	char* read_max,
	char* now,
	char* read_now,
	char* ac
);

int main (int argc, char **argv) {
	if (((argc > 1) && (argv[1][0] == '-')) && (argv[1][1] == 'h')) {
		/* if "batcheck -h", return a help dialogue */
		puts ("Batcheck (c) 2013 R. A. Grant.");
		puts ("Usage: \"batcheck [-option]\"");
		puts ("\tnote that batcheck only takes one option at a time.");
		puts ("options:");
		puts ("\t-h:\tDisplay this help text.\n");
		puts ("Key:\t###:\tbattery percentage remaining (with leading spaces)");
		puts ("\t%:\tpercent sign, of course.");
		puts ("\tZ:\tMnemonic for AC plugged in (looks like lightning)");
		puts ("\tI:\tMnemonic for AC unplugged (looks like battery)");
		puts ("Currently there are no other options for batcheck.");
		puts ("Batcheck now displays time and battery status at");
		puts ("the upper right-hand corner of the terminal.");
		return 0;
	} else if ((argc > 1) && ((argv[1][0] != '-')  || (argv[1][1] != 'h'))) {
		/* if incorrect invocation, return error */
		fputs ("Error, invalid argument.\n", stderr);
		return -1;
	} else {
		/* pid_t is just an int, but since this can be
		 * system-dependent, using pid_t instead is a little
		 * safeguard.
		 *
		 * daemon() basically just calls fork(), then, upon
		 * success, _exit(2). Arguments are int nochdir (if 1,
		 * PWD=$PWD; if 0, PWD=/) and int noclose (if 1, no
		 * change, if 0 redirect stdout and stderr to
		 * /dev/null). All the files called use absolute
		 * referencing, so I don't care about the working
		 * directory, but the whole POINT of this is output, so I
		 * don't want stdout to redirect to /dev/null.
		 * */
		pid_t pid = daemon (0, 1);
		if (pid == -1) {
			fprintf (stderr, "Can't fork process: %s\n",
					strerror (errno));
			return 1;
		}
		if (!pid) {
			/* Let's add time to this. */
			/* Unfortunately, we need a few intermediate
			* steps. */
			time_t nowtime_raw; /* time () will give us a time_t
					(seconds since Unix time) */
			struct tm* nowtime; /* a struct of int values, see below.
					* strftime() requires a tm
					* argument, so we need to convert
					* time_t to this first.
					*/
			char nowtime_str[21]; /* I could format the time myself
					from now.tm_min, now.tm_hour,
					etc, but why bother when strftime()
					will do it all for me?*/
			
			struct winsize xy; /* Window size contains a few
					things, but all that's relevant
					for me is the size of rows and
					columns in characters.*/
			int x;
			/* Color value */
			int color = 36;
			char *max = "/sys/class/power_supply/BAT0/charge_full";
			char *now = "/sys/class/power_supply/BAT0/charge_now";
			char *ac = "/sys/class/power_supply/AC/online";
			char output[6]; /*  output 5 chars (+ \0): 100%Z
					*  Will use leading spaces. Z = charging (looks like
					*  lightning); I = battery (looks like a battery)
					*/
			
			char read_max[8];  /*store number from the file as a string
					File contains a 7-digit (char) number*/
			char read_now[8];
	
			while (1) {
				/* put formatted time into nowtime_str */
				time (&nowtime_raw); /* get time */
				nowtime = localtime (&nowtime_raw); /* store time in
								* struct* tm*/
				strftime (nowtime_str, 21, "%a %b %d, %X", nowtime);
				/* Convert time to string.
				*  Apparently, this doesn't use pointers.
				* %a: 3-char day (eg: Tue)
				* %b: 3-char month (eg: Nov)
				* %d: day of month with leading zeros (eg: 05)
				* %X: 24-hour format hr:min:sec with leading zeros (eg:
				*	16:07:33*/
	
				/* Get info from files and convert it to the
				* output string. */
				check (output, max, read_max, now, read_now, ac);
				/*  Now that it displays on stdout rather than
				*  just on the command line, I don't need to put
				*  it in a file. It'll be there no matter what I
				*  do, so long as I'm in the terminal. That's
				*  good news for Vim's display!
				*/
		
		/********************************************************	
		*		DISPLAY STUFF				*
		*********************************************************/
// Xterm uses TIOCGWINSIZE; tty uses TIOCGSIZE.
// If neither, we've got a problem.
#ifdef TIOCGWINSZ
				ioctl (1, TIOCGWINSZ, &xy); // find terminal size
#elif defined TIOCGSIZE
				ioctl (1, TIOCGSIZE, &xy);
#else
				fputs ("Error, cannot find terminal size.");
#endif
				/* Time string is 21 chars long (including
				* brackets). We want it to display in the upper
				* right hand corner.
				*/
				x = xy.ws_col - 21;
				
				/********************************************************
				*		BREAKDOWN:				*
				* \033 = ESC						*
				* ESC 7 = save cursor info				*
				* ESC [x;ym = format text x; color y			*
				*	1 = bold					*
				*	36 = foreground cyan				*
				* ESC [ y ; x H = move cursor to col x, row y.		*
				*	I think this (col,row where col ~ x and		*
				*	row ~ y) is why ncurses uses (y,x)		*
				*	instead of (x,y) for everything.		*
				* [%s] = "[output]"					*
				* ESC [ 0m = return cursor to original color		*
				*	and format					*
				* ESC 8 = return cursor to condition saved by		*
				*	ESC 7						*
				********************************************************/
				fprintf (stdout, "\0337\033[1;%dm\033[%d;%dH[%s]",
					color, 1, x, nowtime_str);
				/* Next row; adding 8 to the column approximately centers
				* battery output under the time.*/
				fprintf (stdout, "\033[%d;%dH[%s]\033[0m\0338", 2, x + 8, output);
				/*  if we don't flush the buffer, nothing outputs
				*  before we sleep.
				*/
				fflush (stdout);
				/* If we don't sleep, we thrash. There's no
				* reason for the display to update more than
				* once per second anyway. 
				*/
				sleep (1);
			}
			return 0;
		} else {
			return 0;
		}
	}
}
	
void check (
	char* output,
	char* max,
	char* read_max,
	char* now,
	char* read_now,
	char* ac)
{
	/* These characters are the same no matter what. */
	*(output + 3) = '%';
	*(output + 5) = '\0';

	FILE *max_charge; /* read from /sys/class/power_supply/BAT0/charge_full */
	float max_flt; /* store number converted from string to float */

	/* Same thing with current charge instead of max. */
	FILE *current_charge;
	float now_flt;

	FILE *adapter;
	char ac_flag; /* only one char, so convert directly to int flag */

	int percent;

	/* Read in relevant strings from the files. */
	max_charge = fopen (max, "r");
	fgets (read_max, 8, max_charge);
	fclose (max_charge);
	max_flt = atof (read_max);

	current_charge = fopen (now, "r");
	fgets (read_now, 8, current_charge);
	fclose (current_charge);
	now_flt = atof (read_now);

	/* now_flt shouldn't ever be greater than max_flt, but I believe
	 * in covering all my bases. Regardless, if this is the case,
	 * we're at 100%, and it's cheaper to assign 100 than to do any
	 * calculations.*/
	if (now_flt >= max_flt) {
		percent = 100;
	} else {
		/* Otherwise, calculate the percentage, then convert to
		 * an int. */
		percent = (int) ((now_flt / max_flt) * 100);
	}

	/* Easiest way I could think of to do leading spaces was this if
	 * statement. Direct assignment is cheaper and easier than
	 * calculating, so that's what I did for 100. Else, I took
	 * advantage of integer division to directly convert higher
	 * digits. All the builtin stuff including itoc() seems to
	 * convert values directly (ie: int 0 = char 0 =  char '\0' when
	 * I want int 0 to become char '0'), so I took the liberty of
	 * directly adding 48 to the values to get to the right place in
	 * the ASCII chart.*/
	if (percent == 100) {
		*output = '1';
		*(output + 1) = '0';
		*(output + 2) = '0';
	} else if (percent >= 10) {
		*output  = ' ';
		*(output + 1) = (char) ((percent / 10) + 48);
		*(output + 2) = (char) ((percent - ((percent / 10 * 10))) + 48);
	} else {
		*output = ' ';
		*(output + 1) = ' ';
		*(output + 2) = (char) (percent + 48);
	}
	
	/* Is the computer plugged in? */
	adapter = fopen (ac, "r");
	ac_flag = fgetc (adapter) - 48;
	fclose (adapter);

	if (ac_flag == 1) {
		*(output + 4) = 'Z';
	} else {
		*(output + 4) = 'I';
	}
}
