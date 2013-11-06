batcheck
========
Linux terminal display for laptop battery life and time.
(c) November 2013 R. A. Grant
Released under GPL v. 2

Displays time and laptop battery life in upper right hand corner of the
terminal.

Defaults and Customization
--------------------------
Default time format is [%a %b %d, %X]. To change this, in the source,
search for "strftime (" and change the format there. (You may also need
to search for "x = xy.ws_col" and change the number there to keep the
display properly oriented.)

Default color is cyan. To change this, in the source, search for "int
color =" and change the number according to the following chart. To
change both the foreground and background, you will have to search for
"BREAKDOWN" and change the actual fprintf() underneath. (Change
"\033[1;%d" to "\033[1;x;y", where x and y represent foreground and
background colors. You can also change the intensity by changing the 1 to
a different value (0 = normal, 1 = bold, 2 = faint). For other possible
format values, see http://en.wikipedia.org/wiki/ANSI_escape_code.) Sorry
it's so complicated, but I wrote this for myself; if this project becomes
popular, I may make customization easier.
	30: black	40: black background
	31: red		41: red background
	32: green	42: green background
	33: yellow	43: yellow background
	34: blue	44: blue background
	35: magenta	45: magenta background
	36: cyan	46: cyan background
	37: white	47: white background

The battery display shows the current percentage and either a Z for "AC
plugged in" (it looks kinda like lightning, right?) or an I for "AC
unplugged" (looks kinda like a battery, I guess).

BUGS:
----
	* In Vim, sometimes the foreground and background colors switch.

	* I didn't write any code to clear previously written lines, so
	 when scrolling upwards, extra (unchanging) lines may be left
	 behind.
	
	* When the cursor is on one of the top two lines, backspacing may
	 temporarily cause the display line to disappear.
=======
batcheck
========

Battery life and time in Linux terminal
>>>>>>> 74686b0122e0b320d5e381d2f1a5bc8d0ff51578
