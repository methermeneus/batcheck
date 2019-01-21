ARCHIVED 01/20/2019
===================
I haven't used this in a year, and it's not exactly an example of my best work, but I don't want to delete it, because it's my first actually _useful_ program. Take this as an example of how far I've come since 2008, especially since it did what I needed it to, so I still used it as-is for almost a decade.


batcheck
========
Linux terminal display for laptop battery life and time.
(c) November 2008 R. A. Grant
uploaded to GitHub November 2013
Revised in February 2017
Released under GPL v. 2

Displays time and laptop battery life in upper right hand corner of the
terminal.

Defaults and Customization
--------------------------
Default time format is [%a %b %d, %X]. To change this, in the source,
search for "strftime (" and change the format there. (You may also need
to search for "TIMELEN" and change the number there to keep the display properly
oriented.)

Default color is cyan. To change this, in the source, search for "int
color =" and change the number according to the following chart. To
change both the foreground and background, you will have to search for
"BREAKDOWN" and change the actual fprintf() underneath. (Change
"\033[1;%d" to "\033[1;x;y", where x and y represent foreground and
background colors. You can also change the intensity by changing the 1 to
a different value (0 = normal, 1 = bold, 2 = faint). For other possible
format values, see http://en.wikipedia.org/wiki/ANSI_escape_code) Sorry
it's so complicated, but I wrote this for myself; if this project becomes
popular, I may make customization easier.
<table>
	<tr>
		<td>30</td><td>black</td><td>40</td><td>black background</td>
	</tr>
	<tr>
		<td>31</td><td>red</td><td>41</td><td>red background</td>
	</tr>
	<tr>
		<td>32</td><td>green</td><td>42</td><td>green background</td>
	</tr>
	<tr>
		<td>33</td><td>yellow</td><td>43</td><td>yellow background</td>
	</tr>
	<tr>
		<td>34</td><td>blue</td><td>44</td><td>blue background</td>
	</tr>
	<tr>
		<td>35</td><td>magenta</td><td>45</td><td>magenta background</td>
	</tr>
	<tr>
		<td>36</td><td>cyan</td><td>46</td><td>cyan background</td>
	</tr>
	<tr>
		<td>37</td><td>white</td><td>47</td><td>white background</td>
	</tr>
</table>

The battery display shows the current percentage and either a Z for "AC
plugged in" (it looks kinda like lightning, right?) or an I for "AC
unplugged" (looks kinda like a battery, I guess).

BUGS/TODO:
----
- [ ] In Vim, sometimes the foreground and background colors switch.

- [ ] I didn't write any code to clear previously written lines, so
	 when scrolling upwards, extra (unchanging) lines may be left
	 behind.
	
- [ ] When the cursor is on one of the top two lines, backspacing may
	 temporarily cause the display line to disappear.

- [x] ~~Refactor so there aren't two separate laptop and desktop files.~~

- [ ] Parse ACPI table so we don't depend on the ACPI package.

- [ ] Cross-platform? The display part should be easy, but what about getting the info?

- [x] I went on a bit of a delete-spree! ~~Man, do I have a lot of comments, and
	badly-formatted ones at that. I still actually use this program, but it's weird to see
	how little I knew about C and code formatting back then. This needs fixing, even if it
	has nothing to do with the program itself.~~

```
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1

iQIcBAABAgAGBQJYm+mSAAoJECzE45aCeCKZtvQP/AgcBOSra58hDhYiNpOUvDN8
sVADViGSX37Z2xh+NfnIpA8uYPHpO8GN84o8Y4xRqqc7JQPWDC6wm7JG1mwRIFCo
3SGfE8q+zLCj9hZXNCjPUe0ICQ/8pklRY4DYfsKoES59cg94IO9LSKgAkpbh2kxw
YRF9V3SZT+yRER/TSLFvPoi6z0pyHX/FYHtQrKgeh+k9S3y8ScKMmWJI6NFtIIWB
Z+D5GI9KPnTqmLN2OwWAqWq0DKO9es+lGw2BLuKYnds09iBLn9ASdOAIy7FweuEW
qFW51TESuKZ9j0QMOnwgoConEIjHL2lLon0x1OMOaIw3AmvfjR+VEVCHYbNhNG5t
cd8j3ln2nQVWBo2BCwa80KFgqSgsT1hH2PhrSPCQdR0QM6ZCKr0ooJaSMbNcYhV4
vSLKFQiy1us91FwHO/WBkEFyrpOWh+LrtAu5Vt8Tza1y0uVzX2J+kVZdA0Rd8WGn
Zo95GwlUqVAscGJYYsfrs/Hk2VvJhJSV2FGCNgOwizGSXSMAbpaVvwhMwZEu4DxO
II3aWtsMvKwuk1i9UyCtK0UaTILmTmByEHxjfLZPENTsFvpxD4UfB20dfKUU3+1i
JCwfCb/NnT0Q1f9q1voAKaXxVIGIX/ATLvgSOvPJOEU2dFYz7vQfEwCRd46ocoOo
EJfCFufxOyRBZsY+esra
=axyy
-----END PGP SIGNATURE-----
```
