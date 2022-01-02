********************************************
UTF-8 and Wide-character handling in Angband
********************************************

Background
==========

As of the sequence of commits from 589d1d3 to c91ae22, (plus a few bug-fixes and cleanups since) the Angband source is able to handle UTF-8 characters in its edit files, and has dropped the previous hacky mechanism of generating accented characters (with sequences like ["e] for ë).

There are a number of changes that had to happen for this to work, and this page aims to document them.

Locale
======

Angband now needs to be run within a UTF-8 capable locale, and this is checked in **main.c**:*main()*, as::

	if (setlocale(LC_CTYPE, "")) {
		/* Require UTF-8 */
		if (strcmp(nl_langinfo(CODESET), "UTF-8") != 0)
			quit("Angband requires UTF-8 support");
	}

Files
=====

All the edit files are now expected to be in the UTF-8 encoding, and can have accented characters directly inserted in them.
Output files such as spoilers, character dumps and other text output is now in UTF-8.

User Editing of Strings with Multi-byte Characters
==================================================

Provisions have been made in **ui-input.c**:*askfor_aux_keypress()* and
**ui-birth.c**:*edit_text()* to allow the player to input and edit strings
with multi-byte characters.  If multi-line editing is needed in more places
than just the history editing in **ui-birth.c**, then it would be necessary
to extend **ui-input.c** to provide that.

Internals
=========

Canvas
------

The internal representation of the main (and other terminal) screen(s) is as two arrays, one of "attributes" (``byte attr``) and one of "characters" (``wchar_t char``). The characters to be displayed are stored as unicode, in the native wchar_t representation of a unicode character on the platform, whatever that is. When strings are printed to the screen, they are converted from UTF-8 (as ``char *``) to wide characters (``wchar_t *``) using **z-util.c**:*text_mbstowcs()*. This allows the conversion function to be overloaded if a particular platform needs it.
For the few cases where the reverse conversion, wide characters to UTF-8, is
necessary, there is **z-util.c**:*text_wctomb()* and some support utilities,
**z-util.c**:*text_wcsz()* and **z-util.c**:*text_iswprint()*.  Like
*text_mbstowcs()*, all of those allow overloading for a particular platform.

When they are displayed, the wide chars are put on the screen in different ways, depending on the port (see below). In the case of graphics tiles, things are slightly different. The "character" is still stored as a ``wchar_t``, but only the bottom 7 bits are used, as an index into a large 2-D bitmap, containing the tiles along the x axis, and the attributes ("colour") on the y axis. (Check this bit). A tile used to be indicated by the top bit set in both the attribute and the character, but it is now indicated only by the top bit of the attribute. In the original design, I had hoped to treat tiles as  a special case of a font, and allow all unicode character support, but the tiles are multi-coloured, so this cannot work. 

Textblock
---------

Textblocks (in **z-textblock.c**) also have ``wchar_t`` as the internal representation of the displayed characters. These are then copied directly onto the canvas when the textblock is displayed.

Parsers
-------

In reading the edit files, all strings are maintained in UTF-8 until needed.

Glyphs are read in directly to a ``wchar_t`` type, using the "char" parse type, and the *parser_getchar()* function. This does not apply to preference files, where the symbols are all treated as ``int``.
One exception to that is the lists of symbols used in **ui_entry_renderer.txt**:
those are expected to be encoded as UTF-8 and will be written to preference
files in that encoding as well.

Ports
=====

This section lists port-specific changes and what the individual ports do with the wide-char representation of the display characters to get them onto the display.

SDL
---

Wide chars on the canvas are converted to a UTF-8 string using *wcstombs()* and then rendered to the screen using a pre-computed ``TTF_Font`` using *TTF_RenderUTF8_Shaded()* in the function *sdl_mapFontDraw()*.

X11
---

Wide chars on the canvas are drawn directly to the window using *XwcDrawImageString()* in *Infofnt_test_std()*. Fonts are now rendered using ``XFontSet``, rather than the previous ``XFontStruct``.

GCU
---

Now requires the "wide" version of ncurses (i.e. ncursesw), and will fail to build if this is not present.

Wide characters from the canvas are written directly to the screen using *mvwaddnwstr()* in *Term_text_gcu()*.

Some of the default symbols have changed as follows (note that only the change
to the floor representation remains present in 4.2):

============  ===================  =========================
Feature       From                 To
============  ===================  =========================
Floor         Period '.' (U+002E)  MIDDLE DOT '·' (U+00B7)
Magma         ?? (0x03)            MEDIUM SHADE '▒' (U+2592)
Quartz Vein   ?? (0x03)            LIGHT SHADE '░' (U+2591)
Granite Wall  ?? (0x02)            DARK SHADE '▓' (U+2593)
============  ===================  =========================

This has the added advantage that standard fonts can be used, and it is not necessary to resort to hacking fonts to get "solid walls".

Windows
-------

Windows does not properly support UTF-8 using the standard C library routines for locale, so the *text_mbcs_hook* function is defined to use the Windows-native *MultiByteToWideChar()* function, and the external files are assumed to be in UTF-8. Wide chars from the canvas are written directly to the screen using *ExtTextOutW()* in *Term_text_win()*.
To match up with the UTF-8 to wide character conversion, the Windows front end
also customizes *text_wctomb_hook*, *text_wcsz_hook*, and *text_iswprint_hook*
for handling the reverse conversion.

OSX
---

Commit 80341e8 to master implemented UTF-8 support for Cocoa.  Like the
Windows front end, it uses custom functions for *text_mbcs_hook*,
*text_wctomb_hook*, *text_wcsz_hook*, and *text_iswprint_hook*.

GTK
---

Wide chars on the canvas are converted to UTF-8 using *wcstombs()* and then rendered to the screen with *pango_layout_set_text()* in **cairo-utils.c**:*draw_text()*. Tiles are almost certainly broken.

Android
-------

There were significant problems in adapting an Android port to this change, as the support for wide chars in Android is lacking. ``wchar_t`` is implemented as an 8-bit quantity, and some of the support functions such as *mbstowcs()* are missing or broken. We worked around this problem by storing wide characters in 8-bit char types and using ISO-8859-1 (Latin-1) for the encoding. We implemented the missing functions by using the Java String class to convert between UTF-8 and ISO-8859-1.

To Do
=====

Things still remaining to do:

* Change display editor to allow arbitrary input of unicode chars / avoid
  scrolling display of chars?
* Check preference files, and how they store chars. Check that display changes
  work.
* Improve way that input key is matched to display char. Locale-aware input?
* Change tile-handling, so that the tile index is the look up into the bitmap,
  and not (tile index & 0x7f)

Issues
======

* What's seen by the player will depend on whether the font has glyphs for
  the characters used.  If the input files start using code points outside
  of the 1 - 255 range, may need to consider changing the default fonts used
  in the front ends so that they'll properly render those characters.
* On Windows, wchar_t is 16-bit and represents one piece encoded in UTF-16.
  Because of that and the way the canvas is declared for terminals, only
  code points in the basic multi-lingual plane (which, is still alot) can be
  stored.  Code points which require a surrogate pair when encoded as UTF-16
  can't be passed through for rendering.
* On most platforms, fairly low-level key input is used, and that's necessary
  for being able to respond to single keystrokes.  In the context of
  *askfor_aux_keypress()* or history editing in the birth process, it could be
  useful to have better support for the plaform's method of choice for entering
  characters that aren't represented by a single keystroke and modifiers on the
  current keyboard layout.
