/* File: main-crb.c */

/*
 * Copyright (c) 1997 Ben Harrison, Keith Randall, Peter Ammon, Ron Anderson
 * and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with Macintosh computers running OS X,
 * or OS 8/9 with CarbonLib system extention.
 *
 * To use this file, use an appropriate "Makefile" or "Project File", which
 * should define "MACINTOSH" in case of PEF Carbon compilation on CodeWarrior
 * or MPW, and "MACH_O_CARBON" for OS X Developer CD gcc.  Please note that
 * defining "MACINTOSH" for the latter will result in completely broken
 * binary, mostly because of different pathname conventions.
 *
 * The official compilation uses the CodeWarrior Pro compiler.
 *
 * If you are never going to use "graphics" (especially if you are not
 * compiling support for graphics anyway) then you can delete the "pict"
 * resources with id "1001", "1002", "1003" and "1004" with no dangerous
 * side effects.
 *
 *
 * This file assumes that you will be using a PPC Mac running OS X
 * or OS 8/9 (8.6 or greater) with CarbonLib system extention enabled.
 * In fact, the game will refuse to run unless these features are available.
 *
 * MACH_O_CARBON code pushes the system requirement a bit further, and
 * I don't think it works on System 8, even with CarbonLib, because it uses
 * the Bundle services, but I may be wrong.
 *
 * Note that the "preference" file is now a simple XML text file
 * called "<program name>.plist" in case of PEF Carbon, and "<Java-style
 * program id defined in Info.plist>.plist" for Mach-O Carbon, which contains
 * key-value paris, so it no longer has to check version stamp to validate
 * its contents.
 *
 *
 * Note that "init1.c", "init2.c", "load1.c", "load2.c", and "birth.c"
 * should probably be "unloaded" as soon as they are no longer needed,
 * to save space, but I do not know how to do this.  XXX XXX XXX
 *
 * Stange bug -- The first "ClipRect()" call crashes if the user closes
 * all the windows, switches to another application, switches back, and
 * re-opens the main window, for example, using "command-a".  XXX XXX XXX
 *
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * Initial PowerMac port by Maarten Hazewinkel (mmhazewi@cs.ruu.nl).
 *
 * Most Apple Event code provided by Steve Linberg (slinberg@crocker.com).
 *
 * Most of the graphics code is adapted from an extremely minimal subset of
 * the "Sprite World II" package, an amazing (and free) animation package.
 *
 * Carbon code adapted from works by Peter Ammon and Ron Anderson.
 *
 * (List of changes made by "pelpel" follow)
 * Some API calls are updated to OS 8.x-- ones.
 *
 * Pixmap locking code in Term_pict_mac() follows Carbon Porting Guide
 * by Apple.
 *
 * The idle loop in TERM_XTRA_DELAY is rewritten to sleep on WaitNextEvent
 * for a couple of reasons.
 *
 * CheckEvent now really blocks whenever asked to wait.
 *
 * The unused buffer GWorld is completely removed. It has long been pure waste
 * of memory.
 *
 * The default font-size combination was changed because the old one, Monaco
 * at 12 points causes the redraw artefact problem on OS X.
 *
 * Characters in the ASCII mode are clipped by their bounding rects to reduce
 * redraw artefacts that were quite annoying in certain font-point combos.
 *
 * Transparency effect now avoids double bitblts whenever possible.
 *
 * Old tiles were drawn in a wrong fashion by the USE_TRANSPARENCY code.
 *
 * ASCII and the two graphics modes are now controlled by single graf_mode
 * variable.  arg_* and use_* variables are set when requested mode is
 * successfully initialised.
 *
 * Most of the menus are now loaded from resources.
 *
 * Moved TileWidth and TileHeight menus into Special. There were too many menus.
 *
 * Added support for 32x32 tiles, now for [V] only.
 *
 * Related to the above, globe_init no longer loads tile images twice if
 * a tileset doesn't have corresponding masks.
 *
 * Added support for POSIX-style pathnames, for Mach-O Carbon (gcc, CW >= 7).
 * We can finally live without Pascal strings to handle files this way.
 * 
 * (Mach-O Carbon) Graphics tiles are moved out of the resource fork into
 * bundle-based data fork files.
 *
 * Changed size-related menu code, because they no longer function because
 * some APIs have been changed to return Unicode in some cases.
 *
 * Changed the transparency code again, this time using Ron Anderson's code,
 * which makes more sound assumption about background colour and is more
 * efficient.
 *
 * The old asynchronous sound player could try to lock the same handle more
 * than once, load same sound resource already in use, or unlock and release
 * currently playing sound.
 *
 * hook_quit() now releases memory-related resources dynamically allocated by
 * the graphics and sound code.
 *
 * Added support for yet another required Apple Event, reopen application.
 *
 * Changed the way the main game code is called from the file.  It's now
 * entered from a single point in main().  This makes do_menu_file_new,
 * do_menu_file_open and handle_open_when_ready much less hackish, but
 * slightly complicates main().
 *
 * Introduced some "common sense" practices of Macintosh programs:
 * - the update event handler now respects update regions at last;
 * - removed validating/invalidating that became superfluous; and
 * - the event handlers makes use of Window's refcon to find term_data.
 *
 * Important Resources in the resource file:
 *
 *   FREF 130 = ANGBAND_CREATOR / 'APPL' (application)
 *   FREF 129 = ANGBAND_CREATOR / 'SAVE' (save file)
 *   FREF 130 = ANGBAND_CREATOR / 'TEXT' (bone file, generic text file)
 *   FREF 131 = ANGBAND_CREATOR / 'DATA' (binary image file, score file)
 *
 *   DLOG 128 = "About Angband..."
 *
 *   ALRT 128 = unused (?)
 *   ALRT 129 = "Warning..."
 *
 *   DITL 128 = body for DLOG 128
 *   DITL 129 = body for ALRT 129
 *   DITL 130 = body for ALRT 130
 *
 *   ICON 128 = "warning" icon
 *
 *   MBAR 128 = array of MENU id's (128, 129, 130, 131, 132, 133, 134)
 *   MENU 128 = apple (about, -, ...)
 *   MENU 129 = File (new, open, close, save, -, score, quit)
 *   MENU 130 = Edit (undo, -, cut, copy, paste, clear)
 *   MENU 131 = Font (bold, wide, -)
 *   MENU 132 = Size ()
 *   MENU 133 = Windows ()
 *   MENU 134 = Special (Sound, Graphics, TileWidth, TileHeight, -, Fiddle,
 *                       Wizard)
 *              Graphics have following submenu attached:
 *   MENU 144 = Graphics (None, 8x8, 16x16, 32x32, enlarge tiles)
 *              TileWidth and TileHeight submenus are filled in by this program.
 *   MENU 145 = TileWidth ()
 *   MENU 146 = TileHeight ()
 *
 *   On CFM(PEF) Carbon only:
 *   PICT 1001 = Graphics tile set (8x8)
 *   PICT 1002 = Graphics tile set (16x16 images)
 *   PICT 1004 = Graphics tile set (32x32)
 *
 *   Mach-O Carbon now uses data fork resources: 
 *   8x8.png   = Graphics tile set (8x8)
 *   16x16.png = Graphics tile set (16x16 images)
 *   32x32.png = Graphics tile set (32x32)
 *   These files should go into the Resources subdirectory of an application
 *   bundle.
 *
 *   STR# 128 = "Please select the "lib" folder"
 *
 *   plst 0   can be empty, but required for single binary Carbon apps on OS X
 *            Isn't necessary for Mach-O Carbon.
 *
 *
 * File name patterns:
 *   all 'APEX' files have a filename of the form "*:apex:*" (?)
 *   all 'BONE' files have a filename of the form "*:bone:*" (?)
 *   all 'DATA' files have a filename of the form "*:data:*"
 *   all 'SAVE' files have a filename of the form "*:save:*"
 *   all 'USER' files have a filename of the form "*:user:*" (?)
 *
 * Perhaps we should attempt to set the "_ftype" flag inside this file,
 * to avoid nasty file type information being spread all through the
 * rest of the code.  (?)  This might require adding hooks into the
 * "fd_open()" and "my_fopen()" functions in "util.c".  XXX XXX XXX
 *
 *
 * Reasons for each header file:
 *
 *   angband.h = Angband header file
 *
 *   Types.h = (included anyway)
 *   Gestalt.h = gestalt code
 *   QuickDraw.h = (included anyway)
 *   OSUtils.h = (included anyway)
 *   Files.h = file code
 *   Fonts.h = font code
 *   Menus.h = menu code
 *   Dialogs.h = dialog code
 *   Windows.h = (included anyway)
 *   Palettes.h = palette code
 *   ToolUtils.h = HiWord() / LoWord()
 *   Events.h = event code
 *   Resources.h = resource code
 *   Controls.h = button code
 *   Processes.h = GetProcessInformation(), ExitToShell()
 *   Memory.h = NewPtr(), etc
 *   QDOffscreen.h = GWorld code
 *   Sound.h = Sound code
 *   Navigation.h = save file / lib locating dialogues
 *   CFPreferences.h = Preferences
 *   CFNumber.h = read/write short values from/to preferences
 */

/*
 * Yet another main-xxx.c for Carbon (pelpel) - revision 12
 *
 * Since I'm using CodeWarrior, the traditional header files are
 * #include'd below.
 *
 * I also compiled Angband 3.0.2 successfully with OS X's gcc.
 * Please follow these instructions if you are interested.
 *
 * ---(developer CD gcc + makefile porting notes, for Angband 3.0.2)-------
 * 1. Compiling the binary
 *
 * If you try this on OS X + gcc, please use makefile.std, replacing
 * main.c and main.o with main-crb.c and main-crb.o, removing all main-xxx.c
 * and main-xxx.o from SRCS and OBJS, and, and use these settings:
 *
 * COPTS = -Wall -O1 -g -fpascal-strings
 * INCLUDES =
 * DEFINES = -DMACH_O_CARBON -DANGBAND30X
 * LIBS = -framework CoreFoundation -framework QuickTime -framework Carbon
 *
 * -DANGBAND30X only affects main-crb.c. This is because I'm also compiling
 * a couple of variants, and this arrangement makes my life easier.
 *
 * Never, ever #define MACINTOSH.  It'll wreck havoc in system interface
 * (mostly because of totally different pathname convention).
 *
 * You might wish to disable some SET_UID features for various reasons:
 * to have user folder within the lib folder, savefile names etc.
 *
 * For the best compatibility with the Classic ports and my PEF Carbon
 * ports, my_fopen, fd_make and fd_open [in util.c] should call
 *   (void)fsetfileinfo(buf, _fcreator, _ftype);
 * when a file is successfully opened.  Or you'll see odd icons for some files
 * in the lib folder.  In order to do so, extern.h should contain these lines,
 * within #ifdef MACH_O_CARBON:
 *   extern int fsetfileinfo(char *path, u32b fcreator, u32b ftype);
 *   extern u32b _fcreator;
 *   extern u32b _ftype;
 * And enable the four FILE_TYPE macros in h-config.h for defined(MACH_O_CARBON)
 * in addition to defined(MACINTOSH) && !defined(applec), i.e.
 *  #if defined(MACINTOSH) && !defined(applec) || defined(MACH_O_CARBON)
 *
 * This is a very good way to spot bugs in use of these macros, btw.
 *
 * 2. Installation
 *
 * The "angband" binary must be arranged this way for it to work:
 *
 * lib/ <- the lib folder
 * Angband (OS X).app/
 *   Contents/
 *     MacOS/
 *       angband <- the binary you've just compiled
 *     Info.plist <- to be explained below
 *     Resources/
 *       Angband.icns
 *       Data.icns
 *       Edit.icns
 *       Save.icns
 *       8x8.png <- 8x8 tiles
 *       16x16.png <- 16x16 tiles
 *       angband.rsrc <- see below
 *
 * 3. Preparing Info.plist
 *
 * Info.plist is an XML file describing some attributes of an application,
 * and this is appropriate for Angband:
 *
 * <?xml version="1.0" encoding="UTF-8"?>
 * <plist version="1.0">
 *   <dict>
 *     <key>CFBundleName</key><string>Angband</string>
 *     <key>CFBundleDisplayName</key><string>Angband (OS X)</string>
 *     <key>CFBundleExecutable</key><string>angband</string>
 *     <key>CFBundlePackageType</key><string>APPL</string>
 *     <key>CFBundleSignature</key><string>A271</string>
 *     <key>CFBundleVersion</key><string>3.0.2</string>
 *     <key>CFBundleShortVersionString</key><string>3.0.2</string>
 *     <key>CFBundleIconFile</key><string>Angband</string>
 *     <key>CFBundleIdentifier</key><string>net.thangorodrim.Angband</string>
 *     <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>
 *     <key>CFBundleDocumentTypes</key>
 *       <array>
 *         <dict>
 *           <key>CFBundleTypeExtentions</key><array><string>*</string></array>
 *           <key>CFBundleTypeIconFile</key><string>Save</string>
 *           <key>CFBundleTypeName</key><string>Angband saved game</string>
 *           <key>CFBundleTypeOSTypes</key><array><string>SAVE</string></array>
 *           <key>CFBundleTypeRole</key><string>Editor</string>
 *         </dict>
 *         <dict>
 *           <key>CFBundleTypeExtentions</key><array><string>*</string></array>
 *           <key>CFBundleTypeIconFile</key><string>Edit</string>
 *           <key>CFBundleTypeName</key><string>Angband game data</string>
 *           <key>CFBundleTypeOSTypes</key><array><string>TEXT</string></array>
 *           <key>CFBundleTypeRole</key><string>Editor</string>
 *         </dict>
 *         <dict>
 *           <key>CFBundleTypeExtentions</key><array><string>raw</string></array>
 *           <key>CFBundleTypeIconFile</key><string>Data</string>
 *           <key>CFBundleTypeName</key><string>Angband game data</string>
 *           <key>CFBundleTypeOSTypes</key><array><string>DATA</string></array>
 *           <key>CFBundleTypeRole</key><string>Editor</string>
 *         </dict>
 * 	</array>
 *   </dict>
 * </plist>
 *
 * 4. Menu, diaglogue and gfx resources
 *
 * The binary assumes angband.rsrc should be in the traditional resource
 * mangager format.  Please run this command to create it from its textual
 * description:
 *
 * Rez -i /Developer/Headers/FlatCarbon -d MACH_O -o angband.rsrc Angband.r
 *
 * The command is in /Developer/Tools.  You might wish to include it in your
 * PATH.
 *
 * It's better to comment out the definitions of BNDL and plst resources
 * before you do that.  I think you can DeRez the resulting angband.rsrc and
 * feed it to the Interface Builder to produce a set of compatible .nib files,
 * but this file also needs to be updated to understand .nib...  On the other
 * hand, I really don't like to hardcode UI definitions in C.
 *
 * Graphics resources are moved out of the resource fork and become ordinary
 * PNG files.  Make sure to set its resolution to 72 dpi (<- VERY important)
 * while keeping vertical and horizontal scaling factor to 100% (<- VERY
 * important), when you convert tiles in any formats to PNG.  This means
 * that the real size of an image must shrink or grow when you change it's dpi.
 *
 * Sound resources are a bit more complicated.
 * The easiest way is:
 * 1) Grab recent Mac Angband binary.
 * 2) Run this command:
 *   DeRez -only 'snd ' (Angband binary) > sound.r
 * 3) And specify sound.r files in addition to Angband.r when you run Rez.
 *
 * ---(end of OS X + gcc porting note)--------------------------------------
 *
 * Code adapted from Peter Ammon's work on 2.8.3 and some modifications
 * are made when Apple's Carbon Porting Guide says they are absolutely
 * necessary. Other arbirary changes are mostly because of my hatred
 * of deep nestings and indentations. The code for controlling graphics modes
 * have been thoroughly revised simply because I didn't like it (^ ^;).
 * A bonus of this is that graphics settings can be loaded from Preferences
 * quite easily.
 *
 * I also took Ron Anderson's (minimising the use of local-global coordinate
 * conversions). Some might say his QuickTime multimedia is the most
 * significant achievement... Play your favourite CD instead, if you really
 * miss that (^ ^;) I might consider incorporating it if it makes use of
 * event notification.
 *
 * I replaced some old API calls with new (OS 8.x--) ones, especially
 * when I felt Apple is strongly against their continued usage.
 *
 * Similarly, USE_SFL_CODE (AppleEvent code by Steve Linberg) should be
 * always active, so I removed ifdef's just to prevent accidents, as well as
 * to make the code a bit cleaner.
 *
 * On the contrary, I deliberately left traditional resource interfaces.
 * Whatever Apple might say, I abhor file name extentions. And keeping two
 * different sets of resources for Classic and Carbon is just too much for
 * a personal project XXX
 *
 * Because Carbon forbids the use of 68K code, ANGBAND_LITE_MAC sections
 * are removed.
 *
 * Because the default font-size combination causes redraw artefact problem
 * (some characters, even in monospace fonts, have negative left bearings),
 * I introduced rather crude hack to clip all character drawings within
 * their bounding rects. If you don't like this, please comment out the line
 * #define CLIP_HACK
 * below. The alternative, #define OVERWRITE_HACK, is based on Julian Lighton's
 * brilliant suggestion, but it doesn't work as expected. This is because
 * DrawText can render the same character with _different_ pixel width,
 * depending on relative position of a character to the pen. Fonts do look
 * very nice on the Mac, but too nice I'd say, in case of Angband.
 *
 * The check for return values of AEProcessAppleEvent is removed,
 * because it results in annoying dialogues on OS X, also because
 * Apple says it isn't usually necessary.
 *
 * Because the always_pict code is *so* slow, I changed the graphics
 * mode selection a bit to use higher_pict when a user chooses a fixed
 * width font and doesn't changes tile width / height.
 *
 * Added support for David Gervais' 32x32 tiles.
 *
 * Replaced transparency effect code by Ron Anderson's.
 *
 * Added support for gcc & make compilation. They come free with OS X
 * (on the developer CD).  This means that it can be compiled as a bundle.
 *
 * For Mach-O Carbon binary, moved graphics tiles out of the
 * resource fork and made them plain PNG files, to be stored in the application
 * bundle's "Resources" subdirectory.
 *
 * For Mach-O Carbon binary, provided a compile-time option (USE_QT_SOUND) to
 * move sound effect samples out of the resource fork and use *.wav files in
 * the bundle's "Resources" subdirectory. The "*" part must match the names in
 * angband_sound_name (variable.c) exactly. This doesn't hurt performance
 * a lot in [V] (it's got ~25 sound events), but problematic in [Z]-based
 * ones (they have somewhere around 70 sound events).
 *
 * You still can use the resource file that comes with the ext-mac archive
 * on the Angband FTP server, with these additions:
 * - MENUs 131--134 and 144--146, as described above
 * - MBAR 128: just a array of 128 through 134
 * - plst 0 : can be empty, although Apple recommends us to fill it in.
 * - STR# 128 : something like "Please select your lib folder"
 *
 * Since this involves considerable amount of work, I attached
 * a plain text resource definition (= Rez format) .
 * I heavily commented on the file, hoping it could be easily adapted
 * to future versions of Angband as well as variants.
 * I omitted sound effects and graphic tiles to make it reasonably small.
 * Please copy them from any recent Mac binaries - you can use, say ZAngband
 * ones for Vanilla or [V]-based variants quite safely. T.o.M.E. uses fairly
 * extended 16x16 tileset and it is maintained actively. IIRC Thangorodrim
 * compile page has an intruction explaining how to convert tiles for
 * use on the Mac... It can be tricky, depending on your choice of
 * graphics utility. Remember setting resolution to 72 pixels per inch,
 * while keeping vertical/horizontal scale factor to 100% and dump the
 * result as a PICT from resource.
 *
 * To build Carbonised Angband with CodeWarrior, copy your PPC project
 * and
 * - replace main-mac.c in the project with this file (in the link order tab)
 * - remove InterfaceLib and MathLib
 * - add CarbonLib (found in Carbon SDK or CW's UniversalInterfaces) --
 *   if you have compiler/linker errors, you'll need Carbon SDK 1.1 or greater
 * - replace MSL C.PPC.Lib with MSL C.Carbon.Lib (both found in
 *   MSL:MSL_C:MSL_MacOS:Lib:PPC)
 * - leave MSL RuntimePPC.Lib as it is
 * - don't forget to update resource file, as described above
 * - as in Classic targets, you may have to include <unistd.h> and
 *   <fcntl.h>. The most convinient place for them is the first
 *   #ifdef MACINTOSH in h-system.h
 * - check variant dependent ifdef's explained below, and add
 *   appropriate one(s) in your A-mac-h.pch.
 */


/*
 * Force Carbon-compatible APIs
 */
#ifndef MACH_O_CARBON

# ifndef TARGET_API_MAC_CARBON
/* Can be CodeWarrior or MPW */
#  define TARGET_API_MAC_CARBON 1

# endif

#else

/*
 * Must be Mach-O Carbon target with OS X gcc.
 * No need to set TARGET_API_MAC_CARBON to 1 here, but I assume it should
 * be able to make efficient use of BSD functions, hence:
 */
# define USE_MALLOC
/* Not yet */
/* # define USE_NIB */

#endif /* !MACH_O_CARBON */


#include "angband.h"


#if defined(MACINTOSH) || defined(MACH_O_CARBON)

/*
 * Variant-dependent features:
 *
 * #define ALLOW_BIG_SCREEN (V, Ey, O, and Z.  Dr's big screen needs
 * more work.  New S one is too idiosyncratic...)
 * #define ANG281_RESET_VISUALS (Cth, Gum, Z)
 * #define ZANG_AUTO_SAVE (O and Z)
 * #define HAS_SCORE_MENU (V and maybe more)
 * #define ANGBAND_CREATOR four letter code for your variant, if any.
 * or use the default one.
 *
 * For [Z], you also have to say -- #define inkey_flag (p_ptr->inkey_flag)
 * but before that, please, please consider using main-mac-carbon.c in [Z],
 * that has some interesting features.
 */

/* Angband 3.0.x characteristics */
#define USE_DOUBLE_TILES
#define ALLOW_BIG_SCREEN
#define NEW_ZVIRT_HOOKS
/* I can't ditch these, yet, because there are many variants */
#define USE_TRANSPARENCY
#define huge size_t


/* Default creator signature */
#ifndef ANGBAND_CREATOR
# define ANGBAND_CREATOR 'A271'
#endif


/*
 * Use rewritten asynchronous sound player
 */
#define USE_ASYNC_SOUND


/*
 * A rather crude fix to reduce amount of redraw artefacts.
 * Some fixed width fonts (i.e. Monaco) has characters with negative
 * left bearings, so Term_wipe_mac or overwriting cannot completely
 * erase them. This could be introduced to Classic Mac OS ports too,
 * but since I've never heard any complaints and I don't like to
 * make 68K ports even slower, I won't do so there.
 */
#define CLIP_HACK /* */

/*
 * Another redraw artifact killer, based on suggestion by Julian Lighton
 */
/* #define OVERWRITE_HACK */

/* These hacks can co-exist, but I don't like overkill */
#ifdef OVERWRITE_HACK
# ifdef CLIP_HACK
#  undef CLIP_HACK
# endif
#endif


/*
 * In OS X + gcc, use <Carbon/Carbon.h>, <CoreServices/CoreServices.h> and
 * <CoreFoundation/CoreFoundation.h> for ALL of these, including the Apple
 * Event ones.  <QuickTime/QuickTime.h> is used by the tile loading code.
 */
#ifdef MACH_O_CARBON

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>

#else /* MACH_O_CARBON */

#include <MacTypes.h>
#include <Gestalt.h>
#include <QuickDraw.h>
#include <Files.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Windows.h>
#include <Palettes.h>
#include <ToolUtils.h>
#include <Events.h>
#include <Processes.h>
#include <Resources.h>
#include <Controls.h>
#include <MacMemory.h>
#include <QDOffscreen.h>
#include <Sound.h>
#include <Navigation.h>
#include <CFPreferences.h>
#include <CFNumber.h>
#include <AppleEvents.h>
#include <EPPC.h>
#include <Folders.h>

# ifdef MAC_MPW
#  include <CarbonStdCLib.h>
# endif

#endif /* MACH_O_CARBON */



/*
 * Use "malloc()" instead of "NewPtr()"
 */
/* #define USE_MALLOC */


/*
 * Information about each of the 256 available colors
 */
static RGBColor color_info[256];


#if defined(MACH_O_CARBON) || defined(MAC_MPW)

/*
 * Creator signature and file type - Didn't I say that I abhor file name
 * extentions?  Names and metadata are entirely different set of notions.
 */
OSType _fcreator;
OSType _ftype;

#endif /* MACH_O_CARBON || MAC_MPW */


/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term *t;

	Rect r;

	WindowPtr w;


	short padding;

	short pixelDepth;

	/* GWorldPtr theGWorld;	*/

	GDHandle theGDH;

	/* GDHandle mainSWGDH; */

	Str15 title;

	s16b oops;

	s16b keys;

	s16b last;

	s16b mapped;

	s16b rows;
	s16b cols;

	s16b font_id;
	s16b font_size;
	s16b font_face;
	s16b font_mono;

	s16b font_o_x;
	s16b font_o_y;
	s16b font_wid;
	s16b font_hgt;

	s16b tile_o_x;
	s16b tile_o_y;
	s16b tile_wid;
	s16b tile_hgt;

	s16b size_wid;
	s16b size_hgt;

	s16b size_ow1;
	s16b size_oh1;
	s16b size_ow2;
	s16b size_oh2;
};




/*
 * Forward declare -- see below
 */
static bool CheckEvents(int wait);

/*
 * Available values for 'wait'
 */
#define CHECK_EVENTS_DRAIN -1
#define CHECK_EVENTS_NO_WAIT	0
#define CHECK_EVENTS_WAIT 1


#ifndef MACH_O_CARBON

/*
 * Hack -- location of the main directory
 */
static short app_vol;
static long  app_dir;

#endif /* !MACH_O_CARBON */


/*
 * Delay handling of double-clicked savefiles
 */
Boolean open_when_ready = FALSE;

/*
 * Delay handling of pre-emptive "quit" event
 */
Boolean quit_when_ready = FALSE;


/*
 * Aqua automatically supplies the Quit menu.
 */
static Boolean is_aqua = FALSE;

/*
 * Version of Mac OS - for version specific bug workarounds (; ;)
 */
static long mac_os_version;


/*
 * Hack -- game in progress
 */
static Boolean game_in_progress = FALSE;


/*
 * Indicate if the user chooses "new" to start a game
 */
static Boolean new_game = FALSE;


/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;


/*
 * Maximum number of terms
 */
#define MAX_TERM_DATA 8


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;




#ifdef MACH_O_CARBON

/* Carbon File Manager utilities by pelpel */

/*
 * (Carbon)
 * Convert a pathname to a corresponding FSSpec.
 * Returns noErr on success.
 */
static OSErr path_to_spec(const char *path, FSSpec *spec)
{
	OSErr err;
	FSRef ref;

	/* Convert pathname to FSRef ... */
	err = FSPathMakeRef(path, &ref, NULL);
	if (err != noErr) return (err);

	/* ... then FSRef to FSSpec */
	err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);
	
	/* Inform caller of success or failure */
	return (err);
}


/*
 * (Carbon)
 * Convert a FSSpec to a corresponding pathname.
 * Returns noErr on success.
 */
static OSErr spec_to_path(const FSSpec *spec, char *buf, size_t size)
{
	OSErr err;
	FSRef ref;

	/* Convert FSSpec to FSRef ... */
	err = FSpMakeFSRef(spec, &ref);
	if (err != noErr) return (err);

	/* ... then FSRef to pathname */
	err = FSRefMakePath(&ref, buf, size);

	/* Inform caller of success or failure */
	return (err);
}


/*
 * (Carbon) [via path_to_spec]
 * Set creator and filetype of a file specified by POSIX-style pathname.
 * Returns 0 on success, -1 in case of errors.
 */
void fsetfileinfo(cptr pathname, OSType fcreator, OSType ftype)
{
	OSErr err;
	FSSpec spec;
	FInfo info;

	/* Convert pathname to FSSpec */
	if (path_to_spec(pathname, &spec) != noErr) return;

	/* Obtain current finder info of the file */
	if (FSpGetFInfo(&spec, &info) != noErr) return;

	/* Overwrite creator and type */
	info.fdCreator = fcreator;
	info.fdType = ftype;
	err = FSpSetFInfo(&spec, &info);

	/* Done */
	return;
}


#else /* MACH_O_CARBON */


/*
 * Convert a pascal string in place
 *
 * This function may be defined elsewhere, but since it is so
 * small, it is not worth finding the proper function name for
 * all the different platforms.
 */
static void ptocstr(StringPtr src)
{
	int i;

	/* Hack -- pointer */
	char *s = (char*)(src);

	/* Hack -- convert the string */
	for (i = s[0]; i; i--, s++) s[0] = s[1];

	/* Hack -- terminate the string */
	s[0] = '\0';
}



/*
 * Utility routines by Steve Linberg
 *
 * The following three routines (pstrcat, pstrinsert, and PathNameFromDirID)
 * were taken from the Think Reference section called "Getting a Full Pathname"
 * (under the File Manager section).  We need PathNameFromDirID to get the
 * full pathname of the opened savefile, making no assumptions about where it
 * is.
 *
 * I had to hack PathNameFromDirID a little for MetroWerks, but it's awfully
 * nice.
 */
static void pstrcat(StringPtr dst, StringPtr src)
{
	/* copy string in */
	BlockMove(src + 1, dst + *dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}


/*
 * pstrinsert - insert string 'src' at beginning of string 'dst'
 */
static void pstrinsert(StringPtr dst, StringPtr src)
{
	/* make room for new string */
	BlockMove(dst + 1, dst + *src + 1, *dst);

	/* copy new string in */
	BlockMove(src + 1, dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}


static void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName)
{
	CInfoPBRec block;
	Str255 directoryName;
	OSErr err;

	fullPathName[0] = '\0';

	block.dirInfo.ioDrParID = dirID;
	block.dirInfo.ioNamePtr = directoryName;

	while (1)
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		err = PBGetCatInfoSync(&block);
		pstrcat(directoryName, (StringPtr)"\p:");
		pstrinsert(fullPathName, directoryName);
		if (block.dirInfo.ioDrDirID == 2) break;
	}
}


/*
 * Rewritten to use PathNameFromDirID -- pelpel
 *
 * Convert refnum+vrefnum+fname into a full file name
 * Store this filename in 'buf' (make sure it is long enough)
 * Note that 'fname' looks to be a "pascal" string
 */
static void refnum_to_name(char *buf, long refnum, short vrefnum, char *fname)
{
	/* Convert directory & volume reference numbers to an absolute path */
	PathNameFromDirID(refnum, vrefnum, (unsigned char *)buf);

	/* Append file name to the path */
	pstrcat((unsigned char *)buf, (unsigned char *)fname);

	/* Convert the result into a C string */
	ptocstr((unsigned char *)buf);
}

#endif /* MACH_O_CARBON */


#ifdef MAC_MPW

/*
 * Convert pathname to an appropriate format, because MPW's
 * CarbonStdCLib chose to use system's native path format,
 * making our lives harder to create binaries that run on
 * OS 8/9 and OS X :( -- pelpel
 */
void convert_pathname(char* path)
{
	char buf[1024];

	/* Nothing has to be done for CarbonLib on Classic */
	if (mac_os_version >= 0x1000)
	{
		/* Convert to POSIX style */
		ConvertHFSPathToUnixPath(path, buf);

		/* Copy the result back */
		strcpy(path, buf);
	}

	/* Done. */
	return;
}

# ifdef CHECK_MODIFICATION_TIME

/*
 * Although there is no easy way to emulate fstat in the old interface,
 * we still can do stat-like things, because Mac OS is an OS.
 */
static int get_modification_time(cptr path, u32b *mod_time)
{
	CInfoPBRec pb;
	Str255 pathname;
	int i;

	/* Paranoia - make sure the pathname fits in Str255 */
	i = strlen(path);
	if (i > 255) return (-1);

	/* Convert pathname to a Pascal string */
	strncpy((char *)pathname + 1, path, 255);
	pathname[0] = i;

	/* Set up parameter block */
	pb.hFileInfo.ioNamePtr = pathname;
	pb.hFileInfo.ioFDirIndex = 0;
	pb.hFileInfo.ioVRefNum = app_vol;
	pb.hFileInfo.ioDirID = 0;

	/* Get catalog information of the file */
	if (PBGetCatInfoSync(&pb) != noErr) return (-1);

	/* Set modification date and time */
	*mod_time = pb.hFileInfo.ioFlMdDat;

	/* Success */
	return (0);
}


/*
 * A (non-Mach-O) Mac OS version of check_modification_time, for those
 * compilers without good enough POSIX-compatibility libraries XXX XXX
 */
errr check_modification_date(int fd, cptr template_file)
{
#pragma unused(fd)
	u32b txt_stat, raw_stat;
	char *p;
	char fname[32];
	char buf[1024];

	/* Build the file name */
	path_build(buf, sizeof(buf), ANGBAND_DIR_EDIT, template_file);

	/* XXX XXX XXX */
	convert_pathname(buf);

	/* Obtain modification time */
	if (get_modification_time(buf, &txt_stat)) return (-1);

	/* XXX Build filename of the corresponding *.raw file */
	strnfmt(fname, sizeof(fname), "%s", template_file);

	/* Find last '.' */
	p = strrchr(fname, '.');

	/* Can't happen */
	if (p == NULL) return (-1);

	/* Substitute ".raw" for ".txt" */
	strcpy(p, ".raw");

	/* Build the file name of the raw file */
	path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, fname);

	/* XXX XXX XXX */
	convert_pathname(buf);

	/* Obtain modification time */
	if (get_modification_time(buf, &raw_stat)) return (-1);

	/* Ensure the text file is not newer than the raw file */
	if (txt_stat > raw_stat) return (-1);

	/* Keep using the current .raw file */
	return (0);
}

# endif /* CHECK_MODIFICATION_TIME */

#endif /* MAC_MPW */



/*
 * Center a rectangle inside another rectangle
 *
 * Consider using RepositionWindow() whenever possible
 */
static void center_rect(Rect *r, Rect *s)
{
	int centerx = (s->left + s->right)/2;
	int centery = (2*s->top + s->bottom)/3;
	int dx = centerx - (r->right - r->left)/2 - r->left;
	int dy = centery - (r->bottom - r->top)/2 - r->top;
	r->left += dx;
	r->right += dx;
	r->top += dy;
	r->bottom += dy;
}


/*
 * Activate a given window, if necessary
 */
static void activate(WindowPtr w)
{
	/* Activate */
	if (active != w)
	{
		/* Activate */
		if (w) SetPort(GetWindowPort(w));

		/* Remember */
		active = w;
	}
}


/*
 * Display a warning message
 */
static void mac_warning(cptr warning)
{
	Str255 text;
	int len, i;

	/* Limit of 250 chars */
	len = strlen(warning);
	if (len > 250) len = 250;

	/* Make a "Pascal" string */
	text[0] = len;
	for (i=0; i<len; i++) text[i+1] = warning[i];

	/* Prepare the dialog box values */
	ParamText(text, "\p", "\p", "\p");

	/* Display the Alert, wait for Okay */
	Alert(129, 0L);
}


/*
 * Notice fully up-to-date status of the main window
 */
static void validate_main_window(void)
{
	WindowRef w;
	Rect r;

	/* Get the main window */
	w = data[0].w;

	/* Get its rectangle */
	GetPortBounds(GetWindowPort(w), &r);

	/* Validate it */
	ValidWindowRect(w, &r);
}



/*** Some generic functions ***/

/*
 * Update color_info with the current values in angband_color_table
 */
static void update_colour_info(void)
{
	int i;

	/* Update colors */
	for (i = 0; i < 256; i++)
	{
		u16b rv, gv, bv;

		/* Extract the R,G,B data */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Save the actual color */
		color_info[i].red = (rv | (rv << 8));
		color_info[i].green = (gv | (gv << 8));
		color_info[i].blue = (bv | (bv << 8));
	}
}


/*
 * Hack -- activate a color (0 to 255)
 */
static void term_data_color(term_data *td, int a)
{
	/* Activate the color */
	if (td->last != a)
	{
		/* Activate the color */
		RGBForeColor(&color_info[a]);

		/* Memorize color */
		td->last = a;
	}
}


/*
 * Hack -- Apply and Verify the "font" info
 *
 * This should usually be followed by "term_data_check_size()"
 *
 * XXX XXX To force (re)initialisation of td->tile_wid and td->tile_hgt
 * you have to reset them to zero before this function is called.
 * XXX XXX This is automatic when the program starts because the term_data
 * array is WIPE'd by term_data_hack, but isn't in the other cases, i.e.
 * font, font style and size changes.
 */
static void term_data_check_font(term_data *td)
{
	int i;

	FontInfo info;

	WindowPtr old = active;


	/* Activate */
	activate(td->w);

	/* Instantiate font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);

	/* Extract the font info */
	GetFontInfo(&info);

	/* Assume monospaced */
	td->font_mono = TRUE;

	/* Extract the font sizing values XXX XXX XXX */
	td->font_wid = CharWidth('@'); /* info.widMax; */
	td->font_hgt = info.ascent + info.descent;
	td->font_o_x = 0;
	td->font_o_y = info.ascent;

	/* Check important characters */
	for (i = 33; i < 127; i++)
	{
		/* Hack -- notice non-mono-space */
		if (td->font_wid != CharWidth(i)) td->font_mono = FALSE;

		/* Hack -- collect largest width */
		if (td->font_wid < CharWidth(i)) td->font_wid = CharWidth(i);
	}

	/* Set default offsets */
	td->tile_o_x = td->font_o_x;
	td->tile_o_y = td->font_o_y;

	/* Set default tile size */
	if (td->tile_wid == 0) td->tile_wid = td->font_wid;
	if (td->tile_hgt == 0) td->tile_hgt = td->font_hgt;

	/* Re-activate the old window */
	activate(old);
}


/*
 * Hack -- Apply and Verify the "size" info
 */
static void term_data_check_size(term_data *td)
{
	if (td == &data[0])
	{
#ifndef ALLOW_BIG_SCREEN

		/* Forbid resizing of the Angband window */
		td->cols = 80;
		td->rows = 24;

#else

		/* Enforce minimal size */
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;

#endif /* !ALLOW_BIG_SCREEN */
	}

	/* Information windows can be much smaller */
	else
	{
		if (td->cols < 1) td->cols = 1;
		if (td->rows < 1) td->rows = 1;
	}

	/* Enforce maximal sizes */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;

	/* Minimal tile size */
	if (td->tile_wid < td->font_wid) td->tile_wid = td->font_wid;
	if (td->tile_hgt < td->font_hgt) td->tile_hgt = td->font_hgt;

	/* Default tile offsets */
	td->tile_o_x = (td->tile_wid - td->font_wid) / 2;
	td->tile_o_y = (td->tile_hgt - td->font_hgt) / 2;

	/* Minimal tile offsets */
	if (td->tile_o_x < 0) td->tile_o_x = 0;
	if (td->tile_o_y < 0) td->tile_o_y = 0;

	/* Apply font offsets */
	td->tile_o_x += td->font_o_x;
	td->tile_o_y += td->font_o_y;

	/* Calculate full window size */
	td->size_wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	td->size_hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	{
		BitMap tScreen;

		/* Get current screen */
		(void)GetQDGlobalsScreenBits(&tScreen);

		/* Verify the bottom */
		if (td->r.top > tScreen.bounds.bottom - td->size_hgt)
		{
			td->r.top = tScreen.bounds.bottom - td->size_hgt;
		}

		/* Verify the top */
		if (td->r.top < tScreen.bounds.top + GetMBarHeight())
		{
			td->r.top = tScreen.bounds.top + GetMBarHeight();
		}

		/* Verify the right */
		if (td->r.left > tScreen.bounds.right - td->size_wid)
		{
			td->r.left = tScreen.bounds.right - td->size_wid;
		}

		/* Verify the left */
		if (td->r.left < tScreen.bounds.left)
		{
			td->r.left = tScreen.bounds.left;
		}
	}

	/* Calculate bottom right corner */
	td->r.right = td->r.left + td->size_wid;
	td->r.bottom = td->r.top + td->size_hgt;

	/* Assume no graphics */
	td->t->higher_pict = FALSE;
	td->t->always_pict = FALSE;


	/* Handle graphics */
	if (use_graphics)
	{
		/* Use higher pict whenever possible */
		if (td->font_mono) td->t->higher_pict = TRUE;

		/* Use always_pict only when necessary */
		else td->t->always_pict = TRUE;
	}

	/* Fake mono-space */
	if (!td->font_mono ||
	    (td->font_wid != td->tile_wid) ||
		(td->font_hgt != td->tile_hgt))
	{
		/*
		 * Handle fake monospace
		 *
		 * pelpel: This is SLOW. Couldn't we use CharExtra
		 * and SpaceExtra for monospaced fonts? 
		 */
		if (td->t->higher_pict) td->t->higher_pict = FALSE;
		td->t->always_pict = TRUE;
	}
}


/*
 * Hack -- resize a term_data
 *
 * This should normally be followed by "term_data_redraw()"
 */
static void term_data_resize(term_data *td)
{
	/*
	 * Actually resize the window
	 * 
	 * ResizeWindow is the preferred API call, but it cannot
	 * be used here.
	 */
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);
}



/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;

	/* Activate the term */
	Term_activate(td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();

	/* Restore the old term */
	Term_activate(old);
}


/*
 * Graphics support
 */

/*
 * PICT id / file name of image tiles
 */
#ifdef MACH_O_CARBON
static CFStringRef pict_id = NULL;
#else
static int pict_id = 0;
#endif /* MACH_O_CARBON */

/*
 * Width and height of a tile in pixels
 */
static int graf_width = 0;
static int graf_height = 0;

/*
 * Numbers of rows and columns in a tileset,
 * calculated by the PICT/PNG loading code
 */
static int pict_cols = 0;
static int pict_rows = 0;

/*
 * Available graphics modes
 */
#define GRAF_MODE_NONE	0	/* plain ASCII */
#define GRAF_MODE_8X8	1	/* 8x8 tiles */
#define GRAF_MODE_16X16	2	/* 16x16 tiles */
#define GRAF_MODE_32X32	3	/* 32x32 tiles */

/*
 * Current and requested graphics modes
 */
static int graf_mode = GRAF_MODE_NONE;
static int graf_mode_req = GRAF_MODE_NONE;

/*
 * Available transparency effect modes:
 *   TR_NONE - no transparency effects
 *   TR_OVER - Overwriting with transparent black pixels
 * TR_OVER only works with 256 colour (8-bit) images if this file
 * is compiled to produce a PEF Carbon binary, while on Mach-O Carbon
 * it can handle much deeper pixels (verified with 16-bit and
 * 24-bit ones).
 */
#define TR_NONE	0
#define TR_OVER 1

/*
 * Current transparency effect mode
 */
static int transparency_mode = TR_NONE;


/*
 * Forward Declare
 */
typedef struct FrameRec FrameRec;

/*
 * Frame
 *
 *	- GWorld for the frame image
 *	- Handle to pix map (saved for unlocking/locking)
 *	- Pointer to color pix map (valid only while locked)
 */
struct FrameRec
{
	GWorldPtr 		framePort;
	PixMapHandle 	framePixHndl;
	PixMapPtr 		framePix;
};


/*
 * The global picture data
 */
static FrameRec *frameP = NULL;


/*
 * Lock a frame
 */
static void BenSWLockFrame(FrameRec *srcFrameP)
{
	PixMapHandle pixMapH;

	pixMapH = GetGWorldPixMap(srcFrameP->framePort);
	(void)LockPixels(pixMapH);
	HLockHi((Handle)pixMapH);
	srcFrameP->framePixHndl = pixMapH;
	srcFrameP->framePix = (PixMapPtr)(*(Handle)pixMapH);
}


/*
 * Unlock a frame
 */
static void BenSWUnlockFrame(FrameRec *srcFrameP)
{
	if (srcFrameP->framePort != NULL)
	{
		HUnlock((Handle)srcFrameP->framePixHndl);
		UnlockPixels(srcFrameP->framePixHndl);
	}

	srcFrameP->framePix = NULL;
}



#ifdef MACH_O_CARBON

/*
 * Moving graphics resources into data fork -- pelpel
 *
 * (Carbon, Bundle)
 * Given base and type names of a resource, find a file in the
 * current application bundle and return its FSSpec in the third argument.
 * Returns true on success, false otherwise.
 * e.g. get_resource_spec(CFSTR("8x8"), CFSTR("png"), &spec);
 */
static Boolean get_resource_spec(
	CFStringRef base_name, CFStringRef type_name, FSSpec *spec)
{
	CFURLRef res_url;
	FSRef ref;

	/* Find resource (=file) in the current bundle */
	res_url = CFBundleCopyResourceURL(
		CFBundleGetMainBundle(), base_name, type_name, NULL);

	/* Oops */
	if (res_url == NULL) return (false);

	/* Convert CFURL to FSRef */
	(void)CFURLGetFSRef(res_url, &ref);

	/* Convert FSRef to FSSpec */
	(void)FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);

	/* Free allocated CF data */
	CFRelease(res_url);

	/* Success */
	return (true);
}


/*
 * (QuickTime)
 * Create an off-screen GWorld from contents of a file specified by a FSSpec.
 * Based on BenSWCreateGWorldFromPict.
 *
 * Globals referenced: data[0], graf_height, graf_width
 * Globals updated: pict_rows, pict_cols.
 */
static OSErr create_gworld_from_spec(
	GWorldPtr *tile_gw, FSSpec *tile_spec)
{
	OSErr err;
	GraphicsImportComponent gi;
	GWorldPtr gw, tmp_gw;
	GDHandle gdh, tmp_gdh;
	Rect r;
	SInt16 depth;

	/* See if QuickTime understands the file format */
	err = GetGraphicsImporterForFile(tile_spec, &gi);

	/* Oops */
	if (err != noErr) return (err);

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	gdh = data[0].theGDH;

	/* Retrieve the rect of the image */
	err = GraphicsImportGetNaturalBounds(gi, &r);

	/* Adjust it, so that the upper left corner becomes (0, 0) */
	OffsetRect(&r, -r.left, -r.top);

	/* Calculate and set numbers of rows and columns */
	pict_rows = r.bottom / graf_height;
	pict_cols = r.right / graf_width;

	/* Create a GWorld */
	err = NewGWorld(&gw, depth, &r, NULL, gdh, noNewDevice);

	/* Oops */
	if (err != noErr) return (err);

	/* Save the pointer to the GWorld */
	*tile_gw = gw;

	/* Save the current GWorld */
	GetGWorld(&tmp_gw, &tmp_gdh);

	/* Activate the newly created GWorld */
	(void)GraphicsImportSetGWorld(gi, gw, NULL);

	/* Prevent pixmap from moving while drawing */
	(void)LockPixels(GetGWorldPixMap(gw));

	/* Clear the pixels */
	EraseRect(&r);

	/* Draw the image into it */
	(void)GraphicsImportDraw(gi);

	/* Release the lock*/
	UnlockPixels(GetGWorldPixMap(gw));

	/* Restore GWorld */
	SetGWorld(tmp_gw, tmp_gdh);

	/* Close the image importer */
	CloseComponent(gi);

	/* Success */
	return (noErr);
}

#else /* MACH_O_CARBON */

static OSErr BenSWCreateGWorldFromPict(
	GWorldPtr *pictGWorld, PicHandle pictH)
{
	OSErr err;
	GWorldPtr saveGWorld;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	Rect pictRect;
	short depth;
	GDHandle theGDH;

	tempGWorld = NULL;

	/* Reset */
	*pictGWorld = NULL;

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	theGDH = data[0].theGDH;

	/* Obtain size rectangle */
	pictRect = (**pictH).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);

	/* Calculate and set numbers of rows and columns */
	pict_rows = pictRect.bottom / graf_height;
	pict_cols = pictRect.right / graf_width;

	/* Create a GWorld */
	err = NewGWorld(&tempGWorld, depth, &pictRect, nil, theGDH, noNewDevice);

	/* Oops */
	if (err != noErr) return (err);

	/* Save pointer */
	*pictGWorld = tempGWorld;

	/* Save GWorld */
	GetGWorld(&saveGWorld, &saveGDevice);

	/* Activate */
	SetGWorld(tempGWorld, nil);

	/* Dump the pict into the GWorld */
	(void)LockPixels(GetGWorldPixMap(tempGWorld));
	EraseRect(&pictRect);
	DrawPicture(pictH, &pictRect);
	UnlockPixels(GetGWorldPixMap(tempGWorld));

	/* Restore GWorld */
	SetGWorld(saveGWorld, saveGDevice);

	/* Success */
	return (0);
}

#endif /* MACH_O_CARBON */


/*
 * Init the global "frameP"
 */
static errr globe_init(void)
{
	OSErr err;

	GWorldPtr tempPictGWorldP;

#ifdef MACH_O_CARBON
	FSSpec pict_spec;
#else
	PicHandle newPictH;
#endif /* MACH_O_CARBON */


	/* Use window XXX XXX XXX */
	SetPort(GetWindowPort(data[0].w));


#ifdef MACH_O_CARBON

	/* Get the tile resources */
	if (!get_resource_spec(pict_id, CFSTR("png"), &pict_spec)) return (-1);

	/* Create GWorld */
	err = create_gworld_from_spec(&tempPictGWorldP, &pict_spec);

#else /* MACH_O_CARBON */

	/* Get the pict resource */
	if ((newPictH = GetPicture(pict_id)) == 0) return (-1);

	/* Create GWorld */
	err = BenSWCreateGWorldFromPict(&tempPictGWorldP, newPictH);

	/* Release resource */
	ReleaseResource((Handle)newPictH);

#endif /* MACH_O_CARBON */

	/* Error */
	if (err != noErr) return (err);

	/* Create the frame */
	frameP = (FrameRec*)NewPtrClear((Size)sizeof(FrameRec));

	/* Analyze result */
	if (frameP == NULL)
	{
		/* Dispose of image GWorld */
		DisposeGWorld(tempPictGWorldP);

		/* Fake error code */
		return (-1);
	}

	/* Save GWorld */
	frameP->framePort = tempPictGWorldP;

	/* Lock it */
	BenSWLockFrame(frameP);

	/* Success */
	return (noErr);
}


/*
 * Nuke the global "frameP"
 */
static errr globe_nuke(void)
{
	/* Dispose */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);

		/* Forget */
		frameP = NULL;
	}

	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Success */
	return (0);
}


#ifdef USE_ASYNC_SOUND

/*
 * Asynchronous sound player revised
 */
#if defined(USE_QT_SOUND) && !defined(MACH_O_CARBON)
# undef USE_QT_SOUND
#endif /* USE_QT_SOUND && !MACH_O_CARBON */

/*
 * How many sound channels will be pooled
 *
 * Was: 20, but I don't think we need 20 sound effects playing
 * simultaneously :) -- pelpel
 */
#define MAX_CHANNELS		8

/*
 * A pool of sound channels
 */
static SndChannelPtr channels[MAX_CHANNELS];

/*
 * Status of the channel pool
 */
static Boolean channel_initialised = FALSE;

/*
 * Data handles containing sound samples
 */
static SndListHandle samples[MSG_MAX];

/*
 * Reference counts of sound samples
 */
static SInt16 sample_refs[MSG_MAX];

#define SOUND_VOLUME_MIN	0	/* Default minimum sound volume */
#define SOUND_VOLUME_MAX	255	/* Default maximum sound volume */
#define VOLUME_MIN			0	/* Minimum sound volume in % */
#define VOLUME_MAX			100	/* Maximum sound volume in % */
#define VOLUME_INC			5	/* Increment sound volume in % */

/*
 * I'm just too lazy to write a panel for this XXX XXX
 */
static SInt16 sound_volume = SOUND_VOLUME_MAX;


#ifdef USE_QT_SOUND

/*
 * QuickTime sound, by Ron Anderson
 *
 * I didn't choose to use Windows-style .ini files (Ron wrote a parser
 * for it, but...), nor did I use lib/xtra directory, hoping someone
 * would code plist-based configuration code in the future -- pelpel
 */

/*
 * (QuickTime)
 * Load sound effects from data-fork resources.  They are wav files
 * with the same names as angband_sound_name[] (variable.c)
 *
 * Globals referenced: angband_sound_name[]
 * Globals updated: samples[] (they can be *huge*)
 */
static void load_sounds(void)
{
	OSErr err;
	int i;

	/* Start QuickTime */
	err = EnterMovies();

	/* Error */
	if (err != noErr) return;

	/*
	 * This loop may take a while depending on the count and size of samples
	 * to load.
	 *
	 * We should use a progress dialog for this.
	 */
	for (i = 1; i < MSG_MAX; i++)
	{
		/* Apple APIs always give me headacke :( */
		CFStringRef name;
		FSSpec spec;
		SInt16 file_id;
		SInt16 res_id;
		Str255 movie_name;
		Movie movie;
		Track track;
		Handle h;
		Boolean res;

		/* Allocate CFString with the name of sound event to be processed */
		name = CFStringCreateWithCString(NULL, angband_sound_name[i],
			kTextEncodingUS_ASCII);

		/* Error */
		if (name == NULL) continue;

		/* Find sound sample resource with the same name */
		res = get_resource_spec(name, CFSTR("wav"), &spec);

		/* Free the reference to CFString */
		CFRelease(name);

		/* Error */
		if (!res) continue;

		/* Open the sound file */
		err = OpenMovieFile(&spec, &file_id, fsRdPerm);

		/* Error */
		if (err != noErr) continue;

		/* Create Movie from the file */
		err = NewMovieFromFile(&movie, file_id, &res_id, movie_name,
			newMovieActive, NULL);

		/* Error */
		if (err != noErr) goto close_file;

		/* Get the first track of the movie */
		track = GetMovieIndTrackType(movie, 1, AudioMediaCharacteristic,
			movieTrackCharacteristic | movieTrackEnabledOnly );

		/* Error */
		if (track == NULL) goto close_movie;

		/* Allocate a handle to store sample */
		h = NewHandle(0);

		/* Error */
		if (h == NULL) goto close_track;

		/* Dump the sample into the handle */
		err = PutMovieIntoTypedHandle(movie, track, soundListRsrc, h, 0,
			GetTrackDuration(track), 0L, NULL);

		/* Success */
		if (err == noErr)
		{
			/* Store the handle in the sample list */
			samples[i] = (SndListHandle)h;
		}

		/* Failure */
		else
		{
			/* Free unused handle */
			DisposeHandle(h);
		}

		/* Free the track */
close_track: DisposeMovieTrack(track);

		/* Free the movie */
close_movie: DisposeMovie(movie);

		/* Close the movie file */
close_file: CloseMovieFile(file_id);
	}

	/* Stop QuickTime */
	ExitMovies();
}

#else /* USE_QT_SOUND */

/*
 * Return a handle of 'snd ' resource given Angband sound event number,
 * or NULL if it isn't found.
 *
 * Globals referenced: angband_sound_name[] (variable.c)
 */
static SndListHandle find_sound(int num)
{
	Str255 sound;

	/* Get the proper sound name */
	strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[num]);
	sound[0] = strlen((char*)sound + 1);

	/* Obtain resource XXX XXX XXX */
	return ((SndListHandle)GetNamedResource('snd ', sound));
}

#endif /* USE_QT_SOUND */


/*
 * Clean up sound support - to be called when the game exits.
 *
 * Globals referenced: channels[], samples[], sample_refs[].
 */
static void cleanup_sound(void)
{
	int i;

	/* No need to clean it up */
	if (!channel_initialised) return;

	/* Dispose channels */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		/* Drain sound commands and free the channel */
		SndDisposeChannel(channels[i], TRUE);
	}

	/* Free sound data */
	for (i = 1; i < MSG_MAX; i++)
	{
		/* Still locked */
		if ((sample_refs[i] > 0) && (samples[i] != NULL))
		{
			/* Unlock it */
			HUnlock((Handle)samples[i]);
		}

#ifndef USE_QT_SOUND

		/* Release it */
		if (samples[i]) ReleaseResource((Handle)samples[i]);

#else
		/* Free handle */
		if (samples[i]) DisposeHandle((Handle)samples[i]);

#endif /* !USE_QT_SOUND */
	}
}


/*
 * Play sound effects asynchronously -- pelpel
 *
 * I don't believe those who first started using the previous implementations
 * imagined this is *much* more complicated as it may seem.  Anyway, 
 * introduced round-robin scheduling of channels and made it much more
 * paranoid about HLock/HUnlock.
 *
 * XXX XXX de-refcounting, HUnlock and ReleaseResource should be done
 * using channel's callback procedures, which set global flags, and
 * a procedure hooked into CheckEvents does housekeeping.  On the other
 * hand, this lazy reclaiming strategy keeps things simple (no interrupt
 * time code) and provides a sort of cache for sound data.
 *
 * Globals referenced: channel_initialised, channels[], samples[],
 *   sample_refs[].
 * Globals updated: channel_initialised, channels[], sample_refs[].
 *   Only in !USE_QT_SOUND, samples[].
 */
static void play_sound(int num, SInt16 vol)
{
	OSErr err;
	int i;
	int prev_num;
	SndListHandle h;
	SndChannelPtr chan;
	SCStatus status;

	static int next_chan;
	static SInt16 channel_occupants[MAX_CHANNELS];
	static SndCommand volume_cmd, quiet_cmd;


	/* Initialise sound channels */
	if (!channel_initialised)
	{
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			/* Paranoia - Clear occupant table */
			/* channel_occupants[i] = 0; */

			/* Create sound channel for all sounds to play from */
			err = SndNewChannel(&channels[i], sampledSynth, initMono, NULL);

			/* Error */
			if (err != noErr)
			{
				/* Free channels */
				while (--i >= 0)
				{
					SndDisposeChannel(channels[i], TRUE);
				}

				/* Notify error */
				plog("Cannot initialise sound channels!");

				/* Cancel request */
				use_sound = arg_sound = FALSE;

				/* Failure */
				return;
			}
		}

		/* First channel to use */
		next_chan = 0;

		/* Prepare volume command */
		volume_cmd.cmd = volumeCmd;
		volume_cmd.param1 = 0;
		volume_cmd.param2 = 0;

		/* Prepare quiet command */
		quiet_cmd.cmd = quietCmd;
		quiet_cmd.param1 = 0;
		quiet_cmd.param2 = 0;

		/* Initialisation complete */
		channel_initialised = TRUE;
	}

	/* Paranoia */
	if ((num <= 0) || (num >= MSG_MAX)) return;

	/* Prepare volume command */
	volume_cmd.param2 = ((SInt32)vol << 16) | vol;

	/* Channel to use (round robin) */
	chan = channels[next_chan];

	/* See if the resource is already in use */
	if (sample_refs[num] > 0)
	{
		/* Resource in use */
		h = samples[num];

		/* Increase the refcount */
		sample_refs[num]++;
	}

	/* Sound is not currently in use */
	else
	{
		/* Get handle for the sound */
#ifdef USE_QT_SOUND
		h = samples[num];
#else
		h = find_sound(num);
#endif /* USE_QT_SOUND */

		/* Sample not available */
		if (h == NULL) return;

#ifndef USE_QT_SOUND

		/* Load resource */
		LoadResource((Handle)h);

		/* Remember it */
		samples[num] = h;

#endif /* !USE_QT_SOUND */

		/* Lock the handle */
		HLockHi((Handle)h);

		/* Initialise refcount */
		sample_refs[num] = 1;
	}

	/* Poll the channel */
	err = SndChannelStatus(chan, sizeof(SCStatus), &status);

	/* It isn't available */
	if ((err != noErr) || status.scChannelBusy)
	{
		/* Shut it down */
		SndDoImmediate(chan, &quiet_cmd);
	}

	/* Previously played sound on this channel */
	prev_num = channel_occupants[next_chan];

	/* Process previously played sound */
	if (prev_num != 0)
	{
		/* Decrease refcount */
		sample_refs[prev_num]--;

		/* We can free it now */
		if (sample_refs[prev_num] <= 0)
		{
			/* Unlock */
			HUnlock((Handle)samples[prev_num]);

#ifndef USE_QT_SOUND

			/* Release */
			ReleaseResource((Handle)samples[prev_num]);

			/* Forget handle */
			samples[prev_num] = NULL;

#endif /* !USE_QT_SOUND */

			/* Paranoia */
			sample_refs[prev_num] = 0;
		}
	}

	/* Remember this sound as the current occupant of the channel */
	channel_occupants[next_chan] = num;

	/* Set up volume for channel */
	SndDoImmediate(chan, &volume_cmd);

	/* Play new sound asynchronously */
	SndPlay(chan, h, TRUE);

	/* Schedule next channel (round robin) */
	next_chan++;
	if (next_chan >= MAX_CHANNELS) next_chan = 0;
}

#else /* USE_ASYNC_SOUND */

/*
 * Synchronous sound effect player
 *
 * This may not be your choice, but much safer and much less
 * resource hungry.
 */
static void play_sound(int num, SInt16 vol)
{
#pragma unused(vol)
	Handle handle;
	Str255 sound;

	/* Get the proper sound name */
	strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[num]);
	sound[0] = strlen((char*)sound + 1);

	/* Obtain resource XXX XXX XXX */
	handle = GetNamedResource('snd ', sound);

	/* Oops -- it is a failure, but we return 0 anyway */
	if (handle == NULL) return;

	/* Load and Lock */
	LoadResource(handle);
	HLockHi(handle);

	/* Play sound (wait for completion) */
	SndPlay(NULL, (SndListHandle)handle, FALSE);

	/* Unlock and release */
	HUnlock(handle);
	ReleaseResource(handle);
}

#endif /* USE_ASYNC_SOUND */




/*** Support for the "z-term.c" package ***/


/*
 * Initialize a new Term
 *
 * Note also the "window type" called "noGrowDocProc", which might be more
 * appropriate for the main "screen" window.
 *
 * Note the use of "srcCopy" mode for optimized screen writes.
 */
static void Term_init_mac(term *t)
{
	term_data *td = (term_data*)(t->data);
	WindowAttributes wattrs;
	OSStatus err;

	static RGBColor black = {0x0000,0x0000,0x0000};
	static RGBColor white = {0xFFFF,0xFFFF,0xFFFF};

#ifndef ALLOW_BIG_SCREEN

	/* Every window has close and collapse boxes */
	wattrs = kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;

	/* Information windows are resizable */
	if (td != &data[0]) wattrs |= kWindowResizableAttribute;

#else

	/* Big screen - every window has close, collapse and resize boxes */
	wattrs = kWindowCloseBoxAttribute |
		kWindowCollapseBoxAttribute |
		kWindowResizableAttribute;

#endif /* !ALLOW_BIG_SCREEN */

	/* Make the window  */
	err = CreateNewWindow(
			kDocumentWindowClass,
			wattrs,
			&td->r,
			&td->w);

	/* Fatal error */
	if (err != noErr) ExitToShell();

	/* Set refcon */
	SetWRefCon(td->w, (long)td);

	/* Set window title */
	SetWTitle(td->w, td->title);

	/* Activate the window */
	activate(td->w);

	/* Erase behind words */
	TextMode(srcCopy);

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);

	/* Resize the window */
	term_data_resize(td);


	/* Prepare the colors (real colors) */
	RGBBackColor(&black);
	RGBForeColor(&white);

	/* Block */
	{
		Rect globalRect;
		GDHandle mainGDH;
		GDHandle currentGDH;
		GWorldPtr windowGWorld;
		PixMapHandle basePixMap;

		/* Obtain the global rect */
		GetWindowBounds((WindowRef)td->w, kWindowContentRgn, &globalRect);

		/* Obtain the proper GDH */
		mainGDH = GetMaxDevice(&globalRect);

		/* Extract GWorld and GDH */
		GetGWorld(&windowGWorld, &currentGDH);

		/* Obtain base pixmap */
		basePixMap = (**mainGDH).gdPMap;

		/* Save pixel depth */
		td->pixelDepth = (**basePixMap).pixelSize;

		/* Save Window GWorld - unused */
		/* td->theGWorld = windowGWorld; */

		/* Save Window GDH */
		td->theGDH = currentGDH;

		/* Save main GDH - unused */
		/* td->mainSWGDH = mainGDH; */
	}

	{
		Rect portRect;

		/* Get current Rect */
		GetPortBounds(GetWindowPort(td->w), &portRect);

		/* Clip to the window */
		ClipRect(&portRect);

		/* Erase the window */
		EraseRect(&portRect);
	}

	/*
	 * A certain release of OS X fails to display windows at proper
	 * locations (_ _#)
	 */
	if ((mac_os_version >= 0x1000) && (mac_os_version < 0x1010))
	{
		/* Hack - Make sure the window is displayed at (r.left,r.top) */
		MoveWindow(td->w, td->r.left, td->r.top, 1);
	}

	/* Display the window if needed */
	if (td->mapped)
	{
		TransitionWindow(td->w,
			kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);
	}

	/* Hack -- set "mapped" flag */
	t->mapped_flag = td->mapped;

	/* Forget color */
	td->last = -1;
}



/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{
#pragma unused(t)
	/* XXX */
}



/*
 * Unused
 */
static errr Term_user_mac(int n)
{
#pragma unused(n)
	/* Success */
	return (0);
}



/*
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	term_data *td = (term_data*)(Term->data);


	/* Reset color */
	td->last = -1;

	/* Update colors */
	update_colour_info();


	/* Handle sound */
	if (use_sound != arg_sound)
	{
		/* Apply request */
		use_sound = arg_sound;
	}

	/* Don't actually switch graphics until the game is running */
	if (!initialized || !game_in_progress) return (-1);

	/* Handle graphics */
	if (graf_mode_req != graf_mode)
	{
		/* dispose old GWorld's if present */
		globe_nuke();

		/*
		 * Setup parameters according to request
		 */
		switch (graf_mode_req)
		{
			/* ASCII - no graphics whatsoever */
			case GRAF_MODE_NONE:
			{
				use_graphics = arg_graphics = GRAPHICS_NONE;
				transparency_mode = TR_NONE;
				break;
			}

			/*
			 * 8x8 tiles (PICT id 1001)
			 * no transparency effect
			 * "old" graphics definitions
			 */
			case GRAF_MODE_8X8:
			{
				use_graphics = arg_graphics = GRAPHICS_ORIGINAL;
				ANGBAND_GRAF = "old";
				transparency_mode = TR_NONE;
#ifdef MACH_O_CARBON
				pict_id = CFSTR("8x8");
#else
				pict_id = 1001;
#endif /* MACH_O_CARBON */
				graf_width = graf_height = 8;
				break;
			}

			/*
			 * 16x16 tiles (images: PICT id 1002, masks: PICT id 1003)
			 * with transparency effect
			 * "new" graphics definitions
			 */
			case GRAF_MODE_16X16:
			{
				use_graphics = arg_graphics = GRAPHICS_ADAM_BOLT;
				ANGBAND_GRAF = "new";
				transparency_mode = TR_OVER;
#ifdef MACH_O_CARBON
				pict_id = CFSTR("16x16");
#else
				pict_id = 1002;
#endif /* MACH_O_CARBON */
				graf_width = graf_height = 16;
				break;
			}

			/*
			 * 32x32 tiles (images: PICT id 1004)
			 * with transparency effect
			 * "david" graphics definitions
			 * Vanilla-specific
			 */
			case GRAF_MODE_32X32:
			{
				use_graphics = arg_graphics = GRAPHICS_DAVID_GERVAIS;
				ANGBAND_GRAF = "david";
				transparency_mode = TR_OVER;
#ifdef MACH_O_CARBON
				pict_id = CFSTR("32x32");
#else
				pict_id = 1004;
#endif /* MACH_O_CARBON */
				graf_width = graf_height = 32;
				break;
			}
		}

		/* load tiles and setup GWorlds if tiles are requested */
		if ((graf_mode_req != GRAF_MODE_NONE) && (globe_init() != 0))
		{
			/* Oops */
			plog("Cannot initialize graphics!");

			/* reject request */
			graf_mode_req = GRAF_MODE_NONE;

			/* reset graphics flags */
			use_graphics = arg_graphics = GRAPHICS_NONE;

			/* reset transparency mode */
			transparency_mode = TR_NONE;
		}

		/* update current graphics mode */
		graf_mode = graf_mode_req;

		/* Apply and Verify */
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		/* Reset visuals */
		if (initialized && game_in_progress)
		{
#ifndef ANG281_RESET_VISUALS
			reset_visuals(TRUE);
#else
			reset_visuals();
#endif /* !ANG281_RESET_VISUALS */
		}
	}

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	Rect r;

	/* Analyze */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		{
			/* Make a noise */
			SysBeep(1);

			/* Success */
			return (0);
		}

		/* Make a sound */
		case TERM_XTRA_SOUND:
		{
			/* Play sound */
			play_sound(v, sound_volume);

			/* Success */
			return (0);
		}

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			/* Process an event */
			(void)CheckEvents(CHECK_EVENTS_NO_WAIT);

			/* Success */
			return (0);
		}

		/* Process pending events */
		case TERM_XTRA_EVENT:
		{
			/* Process an event */
			(void)CheckEvents(v);

			/* Success */
			return (0);
		}

		/* Flush all pending events (if any) */
		case TERM_XTRA_FLUSH:
		{
			/* Hack -- flush all events */
			while (CheckEvents(CHECK_EVENTS_DRAIN)) /* loop */;

			/* Success */
			return (0);
		}

		/* Hack -- Change the "soft level" */
		case TERM_XTRA_LEVEL:
		{
			/* Activate if requested */
			if (v) activate(td->w);

			/* Success */
			return (0);
		}

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			Rect portRect;

			/* Get current Rect */
			GetPortBounds(GetWindowPort(td->w), &portRect);

			/* No clipping XXX XXX XXX */
			ClipRect(&portRect);

			/* Erase the window */
			EraseRect(&portRect);

			/* Set the color */
			term_data_color(td, TERM_WHITE);

			/* Frame the window in white */
			MoveTo(0, 0);
			LineTo(0, td->size_hgt-1);
			LineTo(td->size_wid-1, td->size_hgt-1);
			LineTo(td->size_wid-1, 0);

			/* Clip to the new size */
			r.left = portRect.left + td->size_ow1;
			r.top = portRect.top + td->size_oh1;
			r.right = portRect.right - td->size_ow2;
			r.bottom = portRect.bottom - td->size_oh2;
			ClipRect(&r);

			/* Success */
			return (0);
		}

		/* React to changes */
		case TERM_XTRA_REACT:
		{
			/* React to changes */
			return (Term_xtra_mac_react());
		}

		/* Delay (milliseconds) */
		case TERM_XTRA_DELAY:
		{
			/*
			 * WaitNextEvent relinquishes CPU as well as
			 * induces a screen refresh on OS X
			 */

			/* If needed */
			if (v > 0)
			{
				EventRecord tmp;
				UInt32 ticks;

				/* Convert millisecs to ticks */
				ticks = (v * 60L) / 1000;

				/*
				 * Hack? - Put the programme into sleep.
				 * No events match ~everyEvent, so nothing
				 * should be lost in Angband's event queue.
				 * Even if ticks are 0, it's worth calling for
				 * the above mentioned reasons.
				 */
				WaitNextEvent((EventMask)~everyEvent, &tmp, ticks, nil);
			}

			/* Success */
			return (0);
		}
	}

	/* Oops */
	return (1);
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_what()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_curs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	FrameRect(&r);

	/* Success */
	return (0);
}


#ifdef USE_DOUBLE_TILES

/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box", twice the width of
 * the current font.
 * We are allowed to use "Term_what()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_bigcurs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + 2 * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	FrameRect(&r);

	/* Success */
	return (0);
}

#endif /* USE_DOUBLE_TILES */


#ifdef OVERWRITE_HACK

/*
 * Low level graphics helper (Assumes valid input)
 *
 * Based on suggestion by Julian Lighton
 *
 * Overwrite "n" old characters starting at	(x,y)
 * with the same ones in the background colour
 */
static void Term_wipe_mac_aux(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	int xp, yp;

	const char *cp;

	static RGBColor black = {0x0000,0x0000,0x0000};


	/* Hack - Black, blacker, blackest-- */
	if (td->last != -2) RGBForeColor(&black);

	/* Hack - force later RGBForeColor switching */
	td->last = -2;

	/* Mega-Hack - use old screen image kept inside the term package */
	cp = &(Term->old->c[y][x]);

	/* Starting pixel */
	xp = x * td->tile_wid + td->tile_o_x + td->size_ow1;
	yp = y * td->tile_hgt + td->tile_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*cp);

	/* Draw the string */
	else DrawText(cp, 0, n);
}

#endif /* OVERWRITE_HACK */


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);


#ifdef OVERWRITE_HACK

	/*
	 * Hack - overstrike the leftmost character with
	 * the background colour. This doesn't interfere with
	 * the graphics modes, because they set always_pict.
	 */
	Term_wipe_mac_aux(x, y, 1);

#endif /* OVERWRITE_HACK */

	/* Erase the block of characters */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + n * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	EraseRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_mac(int x, int y, int n, byte a, const char *cp)
{
	int xp, yp;

#ifdef CLIP_HACK
	Rect r;
#endif /* CLIP_HACK */

	term_data *td = (term_data*)(Term->data);


#ifdef OVERWRITE_HACK
	/* Hack - overstrike with background colour. Is 1 enough? */
	Term_wipe_mac_aux(x, y, n);
#endif /* OVERWRITE_HACK */

	/* Set the color */
	term_data_color(td, a);

#ifdef CLIP_HACK
	/* Hack - only draw within the bounding rect */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + n * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	ClipRect(&r);

	/* Hack - clear the content of the bounding rect */
	EraseRect(&r);
#endif /* CLIP_HACK */

	/* Starting pixel */
	xp = x * td->tile_wid + td->tile_o_x + td->size_ow1;
	yp = y * td->tile_hgt + td->tile_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*cp);

	/* Draw the string */
	else DrawText(cp, 0, n);

#ifdef CLIP_HACK
	/* Obtain current window's rect */
	GetPortBounds(GetWindowPort(td->w), &r);

	/* Clip to the window again */
	ClipRect(&r);
#endif /* CLIP_HACK */

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
#ifdef USE_TRANSPARENCY
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp,
			  const byte *tap, const char *tcp)
#else /* USE_TRANSPARENCY */
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp)
#endif /* USE_TRANSPARENCY */
{
	int i;
	Rect dst_r;
	GrafPtr port;
	PixMapHandle pixmap_h;

#ifdef CLIP_HACK
	Rect portRect;
#endif /* CLIP_HACK */

	term_data *td = (term_data*)(Term->data);

	static RGBColor black = {0x0000,0x0000,0x0000};
	static RGBColor white = {0xFFFF,0xFFFF,0xFFFF};


#ifdef CLIP_HACK
	/* Remember current window's rect */
	GetPortBounds(GetWindowPort(td->w), &portRect);
#endif /* CLIP_HACK */

	/* Destination rectangle */
	dst_r.left = x * td->tile_wid + td->size_ow1;
#ifndef USE_DOUBLE_TILES
	dst_r.right = dst_r.left + td->tile_wid;
#endif /* !USE_DOUBLE_TILES */
	dst_r.top = y * td->tile_hgt + td->size_oh1;
	dst_r.bottom = dst_r.top + td->tile_hgt;

	/* Scan the input */
	for (i = 0; i < n; i++)
	{
		byte a = *ap++;
		char c = *cp++;

#ifdef USE_TRANSPARENCY
		byte ta = *tap++;
		char tc = *tcp++;
#endif


#ifdef USE_DOUBLE_TILES

		/* Hack -- a filler for double-width tile */
		if (use_bigtile && (a == 255))
		{
			/* Advance */
			dst_r.left += td->tile_wid;

			/* Ignore */
			continue;
		}

		/* Prepare right side of rectagle now */
		dst_r.right = dst_r.left + td->tile_wid;

#endif /* USE_DOUBLE_TILES */

		/* Graphics -- if Available and Needed */
		if (use_graphics && ((byte)a & 0x80) && ((byte)c & 0x80))
		{
			int col, row;
			Rect src_r;
#ifdef USE_TRANSPARENCY
			int t_col, t_row;
			Rect terrain_r;
#endif /* USE_TRANSPARENCY */

			/* Row and Col */
			row = ((byte)a & 0x7F) % pict_rows;
			col = ((byte)c & 0x7F) % pict_cols;

			/* Source rectangle */
			src_r.left = col * graf_width;
			src_r.top = row * graf_height;
			src_r.right = src_r.left + graf_width;
			src_r.bottom = src_r.top + graf_height;

#ifdef USE_TRANSPARENCY
			/* Row and Col */
			t_row = ((byte)ta & 0x7F) % pict_rows;
			t_col = ((byte)tc & 0x7F) % pict_cols;

			/* Source rectangle */
			terrain_r.left = t_col * graf_width;
			terrain_r.top = t_row * graf_height;
			terrain_r.right = terrain_r.left + graf_width;
			terrain_r.bottom = terrain_r.top + graf_height;
#endif /* USE_TRANSPARENCY */

			/* Hardwire CopyBits */
			RGBBackColor(&white);
			RGBForeColor(&black);

#ifdef USE_DOUBLE_TILES

			/* Double width tiles */
			if (use_bigtile) dst_r.right += td->tile_wid;

#endif /* USE_DOUBLE_TILES */

			/*
			 * OS X requires locking and unlocking of window port
			 * when we draw directly to its pixmap.
			 * The Lock/Unlock protocol is described in the Carbon
			 * Porting Guide.
			 */

			/* Obtain current window's graphic port */
			port = GetWindowPort(td->w);

			/* Lock pixels, so we can use handle safely */
			LockPortBits(port);

			/* Get Pixmap handle */
			pixmap_h = GetPortPixMap(port);

#ifdef USE_TRANSPARENCY

			/* Transparency effect */
			switch (transparency_mode)
			{
				/* No transparency effects */
				case TR_NONE:
				default:
				{
					/* Draw the picture */
					CopyBits((BitMap*)frameP->framePix,
						(BitMap*)*pixmap_h,
						&src_r, &dst_r, srcCopy, NULL);

					break;
				}

				/* Overwriting with transparent black pixels */
				case TR_OVER:
				{
					/* Draw the terrain */
					CopyBits((BitMap*)frameP->framePix,
						 (BitMap*)*pixmap_h,
						 &terrain_r, &dst_r, srcCopy, NULL);

					/* There's something on the terrain */
					if ((row != t_row) || (col != t_col))
					{
						/* Make black pixels transparent */
						RGBBackColor(&black);

						/* Draw monster/object/player */
						CopyBits((BitMap*)frameP->framePix,
							(BitMap*)*pixmap_h,
							&src_r, &dst_r, transparent, NULL);
					}

					break;
				}
			}

#else /* USE_TRANSPARENCY */

			/* Draw the picture */
			CopyBits((BitMap*)frameP->framePix,
				(BitMap*)*pixmap_h,
				&src_r, &dst_r, srcCopy, NULL);

#endif /* USE_TRANSPARENCY */

			/* Release the lock and dispose the PixMap handle */
			UnlockPortBits(port);

			/* Restore colors */
			RGBBackColor(&black);
			RGBForeColor(&white);

			/* Forget color */
			td->last = -1;
		}

		/*
		 * Deal with these cases:
		 * (1) the player changed tile width / height, or
		 * (2) fake fixed-width for proportional font
		 */
		else
		{
			int xp, yp;

#ifdef OVERWRITE_HACK
			/*
			 * Hack - overstrike with the background colour
			 * Utterly useless in the graphics mode, but it only affects
			 * performance slightly...
			 */
			Term_wipe_mac_aux(x+i, y, 1);
#endif /* OVERWRITE_HACK */

#ifdef CLIP_HACK
			/* Hack - avoid writing outside of dst_r */
			ClipRect(&dst_r);
			/* Some characters do not match dst_r, therefore we have to... */
#endif /* CLIP_HACK */

			/* Erase */
			EraseRect(&dst_r);

			/* Set the color */
			term_data_color(td, a);

			/* Starting pixel */
			xp = dst_r.left + td->tile_o_x;
			yp = dst_r.top + td->tile_o_y;

			/* Move to the correct location */
			MoveTo(xp, yp);

			/* Draw the character */
			DrawChar(c);

#ifdef CLIP_HACK			
			/* Clip to the window - inefficient (; ;) XXX XXX */
			ClipRect(&portRect);
#endif /* CLIP_HACK */
		}

		/* Advance */
		dst_r.left += td->tile_wid;
#ifndef USE_DOUBLE_TILES
		dst_r.right += td->tile_wid;
#endif /* !USE_DOUBLE_TILES */
	}

	/* Success */
	return (0);
}





/*
 * Create and initialize window number "i"
 */
static void term_data_link(int i)
{
	term *old = Term;

	term_data *td = &data[i];

	/* Only once */
	if (td->t) return;

	/* Require mapped */
	if (!td->mapped) return;

	/* Allocate */
	MAKE(td->t, term);

	/* Initialize the term */
	term_init(td->t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	td->t->soft_cursor = TRUE;

	/*
	 * HACK - We have an "icky" lower right corner, since
	 * the window resize control is placed there
	 */
	td->t->icky_corner = TRUE;

	/* Erase with "white space" */
	td->t->attr_blank = TERM_WHITE;
	td->t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	td->t->init_hook = Term_init_mac;
	td->t->nuke_hook = Term_nuke_mac;

	/* Prepare the function hooks */
	td->t->user_hook = Term_user_mac;
	td->t->xtra_hook = Term_xtra_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->curs_hook = Term_curs_mac;
#ifdef USE_DOUBLE_TILES
	td->t->bigcurs_hook = Term_bigcurs_mac;
#endif /* USE_DOUBLE_TILES */
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;

#if 0

	/* Doesn't make big difference? */
	td->t->never_bored = TRUE;

#endif

	/* Link the local structure */
	td->t->data = (void *)(td);

	/* Activate it */
	Term_activate(td->t);

	/* Global pointer */
	angband_term[i] = td->t;

	/* Activate old */
	Term_activate(old);
}




#ifdef MACH_O_CARBON

/*
 * (Carbon, Bundle)
 * Return a POSIX pathname of the lib directory, or NULL if it can't be
 * located.  Caller must supply a buffer along with its size in bytes,
 * where returned pathname will be stored.
 */
static char *locate_lib(char *buf, size_t size)
{
	CFBundleRef main_bundle;
	CFURLRef main_url;
	bool success;

	/* Get the application bundle (Angband.app) */
	main_bundle = CFBundleGetMainBundle();

	/* Oops */
	if (!main_bundle) return (NULL);

	/* Obtain the URL of the main bundle */
	main_url = CFBundleCopyBundleURL(main_bundle);

	/* Oops */
	if (!main_url) return (NULL);

	/* Get the URL in the file system's native string representation */
	success = CFURLGetFileSystemRepresentation(main_url, TRUE, buf, size);

	/* Free the url */
	CFRelease(main_url);

	/* Oops */
	if (!success) return (NULL);	

	/* Append "/Contents/Resources/lib/" */
	my_strcat(buf, "/Contents/Resources/lib/", size);

	return (buf);
}


#else /* MACH_O_CARBON */

/*
 * Set the "current working directory" (also known as the "default"
 * volume/directory) to the location of the current application.
 *
 * Original code by: Maarten Hazewinkel (mmhazewi@cs.ruu.nl)
 *
 * Completely rewritten to use Carbon Process Manager.  It retrieves the
 * volume and directory of the current application and simply stores it
 * in the (static) global variables app_vol and app_dir, but doesn't
 * mess with the "current working directory", because it has long been
 * an obsolete (and arcane!) feature.
 */
static void SetupAppDir(void)
{
	OSErr err;
	ProcessSerialNumber curPSN;
	ProcessInfoRec procInfo;
	FSSpec cwdSpec;

	/* Initialise PSN info for the current process */
	curPSN.highLongOfPSN = 0;
	curPSN.lowLongOfPSN = kCurrentProcess;

	/* Fill in mandatory fields */
	procInfo.processInfoLength = sizeof(ProcessInfoRec);
	procInfo.processName = nil;
	procInfo.processAppSpec = &cwdSpec;

	/* Obtain current process information */
	err = GetProcessInformation(&curPSN, &procInfo);

	/* Oops */
	if (err != noErr)
	{
		mac_warning("Unable to get process information");

		/* Quit without writing anything */
		ExitToShell();
	}

	/* Extract and save the Vol and Dir */
	app_vol = cwdSpec.vRefNum;
	app_dir = cwdSpec.parID;
}

#endif /* MACH_O_CARBON */




/*
 * Using Core Foundation's Preferences services -- pelpel
 *
 * Requires OS 8.6 or greater with CarbonLib 1.1 or greater. Or OS X,
 * of course.
 *
 * Without this, we can support older versions of OS 8 as well
 * (with CarbonLib 1.0.4).
 *
 * Frequent allocation/deallocation of small chunks of data is
 * far from my liking, but since this is only called at the
 * beginning and the end of a session, I hope this hardly matters.
 */


/*
 * Store "value" as the value for preferences item name
 * pointed by key
 */
static void save_pref_short(const char *key, short value)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* allocate and initialise the value */
	cf_value = CFNumberCreate(NULL, kCFNumberShortType, &value);

	if ((cf_key != NULL) && (cf_value != NULL))
	{
		/* Store the key-value pair in the applications preferences */
		CFPreferencesSetAppValue(
			cf_key,
			cf_value,
			kCFPreferencesCurrentApplication);
	}

	/*
	 * Free CF data - the reverse order is a vain attempt to
	 * minimise memory fragmentation.
	 */
	if (cf_value) CFRelease(cf_value);
	if (cf_key) CFRelease(cf_key);
}


/*
 * Load preference value for key, returns TRUE if it succeeds with
 * vptr updated appropriately, FALSE otherwise.
 */
static bool query_load_pref_short(const char *key, short *vptr)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* Oops */
	if (cf_key == NULL) return (FALSE);

	/* Retrieve value for the key */
	cf_value = CFPreferencesCopyAppValue(
		cf_key,
		kCFPreferencesCurrentApplication);

	/* Value not found */
	if (cf_value == NULL)
	{
		CFRelease(cf_key);
		return (FALSE);
	}

	/* Convert the value to short */
	CFNumberGetValue(
		cf_value,
		kCFNumberShortType,
		vptr);

	/* Free CF data */
	CFRelease(cf_value);
	CFRelease(cf_key);

	/* Success */
	return (TRUE);
}


/*
 * Update short data pointed by vptr only if preferences
 * value for key is located.
 */
static void load_pref_short(const char *key, short *vptr)
{
	short tmp;

	if (query_load_pref_short(key, &tmp)) *vptr = tmp;
}


/*
 * Save preferences to preferences file for current host+current user+
 * current application.
 */
static void cf_save_prefs()
{
	int i;

	/* Version stamp */
	save_pref_short("version.major", VERSION_MAJOR);
	save_pref_short("version.minor", VERSION_MINOR);
	save_pref_short("version.patch", VERSION_PATCH);
	save_pref_short("version.extra", VERSION_EXTRA);

	/* Gfx settings */
	save_pref_short("arg.arg_sound", arg_sound);
	save_pref_short("arg.graf_mode", graf_mode);
#ifdef USE_DOUBLE_TILES
	save_pref_short("arg.big_tile", use_bigtile);
#endif /* USE_DOUBLE_TILES */

	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		save_pref_short(format("term%d.mapped", i), td->mapped);

		save_pref_short(format("term%d.font_id", i), td->font_id);
		save_pref_short(format("term%d.font_size", i), td->font_size);
		save_pref_short(format("term%d.font_face", i), td->font_face);

		save_pref_short(format("term%d.tile_wid", i), td->tile_wid);
		save_pref_short(format("term%d.tile_hgt", i), td->tile_hgt);

		save_pref_short(format("term%d.cols", i), td->cols);
		save_pref_short(format("term%d.rows", i), td->rows);
		save_pref_short(format("term%d.left", i), td->r.left);
		save_pref_short(format("term%d.top", i), td->r.top);
	}

	/*
	 * Make sure preferences are persistent
	 */
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}


/*
 * Load preferences from preferences file for current host+current user+
 * current application.
 */
static void cf_load_prefs()
{
	bool ok;
	short pref_major, pref_minor, pref_patch, pref_extra;
	short valid;
	int i;

	/* Assume nothing is wrong, yet */
	ok = TRUE;

	/* Load version information */
	ok &= query_load_pref_short("version.major", &pref_major);
	ok &= query_load_pref_short("version.minor", &pref_minor);
	ok &= query_load_pref_short("version.patch", &pref_patch);
	ok &= query_load_pref_short("version.extra", &pref_extra);

	/* Any of the above failed */
	if (!ok)
	{
#if 0
		/* This may be the first run */
		mac_warning("Preferences are not found.");
#endif /* 0 */

		/* Ignore the rest */
		return;
	}

#if 0

	/* Check version */
	if ((pref_major != VERSION_MAJOR) ||
		(pref_minor != VERSION_MINOR) ||
		(pref_patch != VERSION_PATCH) ||
		(pref_extra != VERSION_EXTRA))
	{
		/* Message */
		mac_warning(
			format("Ignoring %d.%d.%d.%d preferences.",
				pref_major, pref_minor, pref_patch, pref_extra));

		/* Ignore */
		return;
	}

#endif

	/* HACK - Check for broken preferences */
	load_pref_short("term0.mapped", &valid);

	/* Ignore broken preferences */
	if (!valid)
	{
		mac_warning("Ignoring broken preferences.");

		/* Ignore */
		return;
	}

	/* Gfx settings */
	{
		short pref_tmp;

		/* sound */
		if (query_load_pref_short("arg.arg_sound", &pref_tmp))
			arg_sound = pref_tmp;

		/* graphics */
		if (query_load_pref_short("arg.graf_mode", &pref_tmp))
			graf_mode_req = pref_tmp;

#ifdef USE_DOUBLE_TILES

		/* double-width tiles */
		if (query_load_pref_short("arg.big_tile", &pref_tmp))
		{
			use_bigtile = pref_tmp;
		}

#endif /* USE_DOUBLE_TILES */

	}

	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		load_pref_short(format("term%d.mapped", i), &td->mapped);

		load_pref_short(format("term%d.font_id", i), &td->font_id);
		load_pref_short(format("term%d.font_size", i), &td->font_size);
		load_pref_short(format("term%d.font_face", i), &td->font_face);

		load_pref_short(format("term%d.tile_wid", i), &td->tile_wid);
		load_pref_short(format("term%d.tile_hgt", i), &td->tile_hgt);

		load_pref_short(format("term%d.cols", i), &td->cols);
		load_pref_short(format("term%d.rows", i), &td->rows);
		load_pref_short(format("term%d.left", i), &td->r.left);
		load_pref_short(format("term%d.top", i), &td->r.top);
	}
}




/*
 * Hack -- default data for a window
 */
static void term_data_hack(term_data *td)
{
	short fid;

	/* Default to Monaco font */
	GetFNum("\pmonaco", &fid);

	/* Wipe it */
	WIPE(td, term_data);

	/* No color */
	td->last = -1;

	/* Default borders */
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh2 = 2;

	/* Start hidden */
	td->mapped = FALSE;

	/* Default font */
	td->font_id = fid;

	/* Default font size - was 12 */
	td->font_size = 14;

	/* Default font face */
	td->font_face = 0;

	/* Default size */
	td->rows = 24;
	td->cols = 80;

	/* Default position */
	td->r.left = 10;
	td->r.top = 40;

	/* Minimal keys */
	td->keys = 16;
}


/*
 * Read the preference file, Create the windows.
 *
 * We attempt to use "FindFolder()" to track down the preference file.
 */
static void init_windows(void)
{
	int i, b = 0;

	term_data *td;


	/*** Default values ***/

	/* Initialize (backwards) */
	for (i = MAX_TERM_DATA; i-- > 0; )
	{
		int n;

		cptr s;

		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Obtain title */
		s = angband_term_name[i];

		/* Get length */
		n = strlen(s);

		/* Maximal length */
		if (n > 15) n = 15;

		/* Copy the title */
		strncpy((char*)(td->title) + 1, s, n);

		/* Save the length */
		td->title[0] = n;

		/* Tile the windows */
		td->r.left += (b * 30);
		td->r.top += (b * 30);

		/* Tile */
		b++;
	}


	/*** Load preferences ***/

	cf_load_prefs();


	/*** Instantiate ***/

	/* Main window */
	td = &data[0];

	/* Many keys */
	td->keys = 1024;

	/* Start visible */
	td->mapped = TRUE;

	/* Link (backwards, for stacking order) */
	for (i = MAX_TERM_DATA; i-- > 0; )
	{
		term_data_link(i);
	}

	/* Main window */
	td = &data[0];

	/* Main window */
	Term_activate(td->t);
}


/*
 * Save preferences
 */
static void save_pref_file(void)
{
	cf_save_prefs();
}




/*
 * Prepare savefile dialogue and set the variable
 * savefile accordingly. Returns true if it succeeds, false (or
 * aborts) otherwise. If all is false, only allow files whose type
 * is 'SAVE'.
 * Originally written by Peter Ammon
 */
static bool select_savefile(bool all)
{
	OSErr err;
	FSSpec theFolderSpec;
	FSSpec savedGameSpec;
	NavDialogOptions dialogOptions;
	NavReplyRecord reply;
	/* Used only when 'all' is true */
	NavTypeList types = {ANGBAND_CREATOR, 1, 1, {'SAVE'}};
	NavTypeListHandle myTypeList;
	AEDesc defaultLocation;

#ifdef MACH_O_CARBON

	short foundVRefNum;
	long foundDirID;

	/* Find the preferences folder */
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kDontCreateFolder,
	                 &foundVRefNum, &foundDirID);

	if (err != noErr) quit("Couldn't find the preferences folder!");

	/* Look for the "Angband/save/" sub-folder */
	err = FSMakeFSSpec(foundVRefNum, foundDirID, "\p:Angband:save:", &theFolderSpec);

	/* Oops */
	if (err != noErr) quit_fmt("Unable to find the savefile folder! (Error %d)", err);

#else

	/* Find :lib:save: folder */
	err = FSMakeFSSpec(app_vol, app_dir, "\p:lib:save:", &theFolderSpec);


	/* Oops */
	if (err != noErr) quit("Unable to find the folder :lib:save:");

#endif

	/* Get default Navigator dialog options */
	err = NavGetDefaultDialogOptions(&dialogOptions);

	/* Clear preview option */
	dialogOptions.dialogOptionFlags &= ~kNavAllowPreviews;

	/* Disable multiple file selection */
	dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;

	/* Make descriptor for default location */
	err = AECreateDesc(typeFSS, &theFolderSpec, sizeof(FSSpec),
		&defaultLocation);

	/* Oops */
	if (err != noErr) quit("Unable to allocate descriptor");

	/* We are indifferent to signature and file types */
	if (all)
	{
		myTypeList = (NavTypeListHandle)nil;
	}

	/* Set up type handle */
	else
	{
		err = PtrToHand(&types, (Handle *)&myTypeList, sizeof(NavTypeList));

		/* Oops */
		if (err != noErr) quit("Error in PtrToHand. Try enlarging heap");

	}

	/* Call NavGetFile() with the types list */
	err = NavChooseFile(&defaultLocation, &reply, &dialogOptions, NULL,
		NULL, NULL, myTypeList, NULL);

	/* Free type list */
	if (!all) DisposeHandle((Handle)myTypeList);

	/* Error */
	if (err != noErr)
	{
		/* Nothing */
	}

	/* Invalid response -- allow the user to cancel */
	else if (!reply.validRecord)
	{
		/* Hack -- Fake error */
		err = -1;
	}

	/* Retrieve FSSpec from the reply */
	else
	{
		AEKeyword theKeyword;
		DescType actualType;
		Size actualSize;

		/* Get a pointer to selected file */
		(void)AEGetNthPtr(&reply.selection, 1, typeFSS, &theKeyword,
			&actualType, &savedGameSpec, sizeof(FSSpec), &actualSize);

		/* Dispose NavReplyRecord, resources and descriptors */
		(void)NavDisposeReply(&reply);
	}

	/* Dispose location info */
	AEDisposeDesc(&defaultLocation);

	/* Error */
	if (err != noErr) return (FALSE);

#ifdef MACH_O_CARBON

	/* Convert FSSpec to pathname and store it in variable savefile */
	(void)spec_to_path(&savedGameSpec, savefile, sizeof(savefile));

#else

	/* Convert FSSpec to pathname and store it in variable savefile */
	refnum_to_name(
		savefile,
		savedGameSpec.parID,
		savedGameSpec.vRefNum,
		(char *)savedGameSpec.name);

#endif

	/* Success */
	return (TRUE);
}


/*
 * Handle menu: "File" + "New"
 */
static void do_menu_file_new(void)
{
	/* Game is in progress */
	game_in_progress = TRUE;

	/* Start a new game */
	new_game = TRUE;
}


/*
 * Handle menu: "File" + "Open" /  "Import"
 */
static void do_menu_file_open(bool all)
{
	/* Let the player to choose savefile */
	if (!select_savefile(all)) return;

	/* Game is in progress */
	game_in_progress = TRUE;

	/* Use an existing savefile */
	new_game = FALSE;
}


/*
 * Handle the "open_when_ready" flag
 */
static void handle_open_when_ready(void)
{
	/* Check the flag XXX XXX XXX make a function for this */
	if (open_when_ready && initialized && !game_in_progress)
	{
		/* Forget */
		open_when_ready = FALSE;

		/* Game is in progress */
		game_in_progress = TRUE;

		/* Use an existing savefile */
		new_game = FALSE;

		/* Wait for a keypress */
		pause_line(Term->hgt - 1);
	}
}




/*
 * Menus
 *
 * The standard menus are:
 *
 *   Apple (128) =   { About, -, ... }
 *   File (129) =    { New,Open,Import,Close,Save,-,Score,Quit }
 *   Edit (130) =    { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) =    { Bold, Extend, -, Monaco, ..., -, ... }
 *   Size (132) =    { ... }
 *   Window (133) =  { Angband, Term-1/Mirror, Term-2/Recall, Term-3/Choice,
 *                     Term-4, Term-5, Term-6, Term-7 }
 *   Special (134) = { Sound, Graphics, TileWidth, TileHeight, -,
 *                     Fiddle, Wizard }
 */

/* Apple menu */
#define MENU_APPLE	128
#define ITEM_ABOUT	1

/* File menu */
#define MENU_FILE	129
# define ITEM_NEW	1
# define ITEM_OPEN	2
# define ITEM_IMPORT	3
# define ITEM_CLOSE	4
# define ITEM_SAVE	5
# ifdef HAS_SCORE_MENU
#  define ITEM_SCORE 7
#  define ITEM_QUIT	8
# else
#  define ITEM_QUIT	7
# endif /* HAS_SCORE_MENU */

/* Edit menu */
#define MENU_EDIT	130
# define ITEM_UNDO	1
# define ITEM_CUT	3
# define ITEM_COPY	4
# define ITEM_PASTE	5
# define ITEM_CLEAR	6

/* Font menu */
#define MENU_FONT	131
# define ITEM_BOLD	1
# define ITEM_WIDE	2

/* Size menu */
#define MENU_SIZE	132

/* Windows menu */
#define MENU_WINDOWS	133

/* Special menu */
#define MENU_SPECIAL	134
# define ITEM_SOUND	1
# define ITEM_GRAPH	2
# define SUBMENU_GRAPH	144
#  define ITEM_NONE	1
#  define ITEM_8X8	2
#  define ITEM_16X16	3
#  define ITEM_32X32	4
#  define ITEM_BIGTILE 6
# define ITEM_TILEWIDTH 3
# define SUBMENU_TILEWIDTH 145
# define ITEM_TILEHEIGHT 4
# define SUBMENU_TILEHEIGHT 146
# define ITEM_FIDDLE	6
# define ITEM_WIZARD	7


/*
 * I HATE UNICODE!  We've never wanted it.  Some multi-national companies
 * made it up as their internationalisation "solution".  So I won't use
 * any such API's -- pelpel
 */
#define NSIZES 32
static byte menu_size_values[NSIZES];
static byte menu_tilewidth_values[NSIZES];
static byte menu_tileheight_values[NSIZES];

/*
 * Initialize the menus
 *
 * Fixed top level menus are now loaded all at once by GetNewMBar().
 * Although this simplifies the function a bit, we have to make sure
 * that resources have all the expected entries defined XXX XXX
 */
static void init_menubar(void)
{
	int i, n;

	Rect r;

	WindowPtr tmpw;

	MenuRef m;

#ifdef USE_NIB

	/* The new way - loading main menu using Interface Builder services */
	{
		IBNibRef nib;
		OSStatus err;

		/* Create a nib reference to the main nib file */
		err = CreateNibReference(CFSTR("main"), &nib);

		/* Fatal error - missing Main.nib */
		if (err != noErr) quit("Cannot find Main.nib in the bundle!");

		/* Unarchive the menu bar and make it ready to use */
		err = SetMenuBarFromNib(nib, CFSTR("MainMenu"));

		/* Fatal error - couldn't insert menu bar */
		if (err != noErr) quit("Cannot prepare menu bar!");

		/* Dispose of the nib reference because we don't need it any longer */
		DisposeNibReference(nib);
	}

#else /* USE_NIB */

	/* The old way - loading main menu from Resource Manager resource */
	{
		Handle mbar;

		/* Load menubar from resources */
		mbar = GetNewMBar(128);

		/* Whoops! */
		if (mbar == nil) quit("Cannot find menubar('MBAR') id 128!");

		/* Insert them into the current menu list */
		SetMenuBar(mbar);

		/* Free handle */
		DisposeHandle(mbar);
	}

#endif /* USE_NIB */


	/* Apple menu (id 128) - we don't have to do anything */


#ifndef USE_NIB

	/* File menu (id 129) - Aqua provides Quit menu for us */
	if (is_aqua)
	{
		/* Get a handle to the file menu */
		m = GetMenuHandle(MENU_FILE);

		/* Nuke the quit menu since Aqua does that for us */
		DeleteMenuItem(m, ITEM_QUIT);

#ifndef HAS_SCORE_MENU

		/* Hack - because the above leaves a separator as the last item */
		DeleteMenuItem(m, ITEM_QUIT - 1);

#endif /* !HAS_SCORE_MENU */
	}

#endif /* !USE_NIB */


	/* Edit menu (id 130) - we don't have to do anything */


	/*
	 * Font menu (id 131) - append names of mono-spaced fonts
	 * followed by all available ones
	 */
	m = GetMenuHandle(MENU_FONT);

	/* Fake window */
	r.left = r.right = r.top = r.bottom = 0;

	/* Make the fake window so that we can retrieve font info */
	(void)CreateNewWindow(
		kDocumentWindowClass,
		kWindowNoAttributes,
		&r,
		&tmpw);

	/* Activate the "fake" window */
	SetPort(GetWindowPort(tmpw));

	/* Default mode */
	TextMode(0);

	/* Default size */
	TextSize(12);

	/* Add the fonts to the menu */
	AppendResMenu(m, 'FONT');

	/* Size of menu */
	n = CountMenuItems(m);

	/* Scan the menu */
	for (i = n; i >= 4; i--)
	{
		Str255 tmpName;
		short fontNum;

		/* Acquire the font name */
		GetMenuItemText(m, i, tmpName);

		/* Acquire the font index */
		GetFNum(tmpName, &fontNum);

		/* Apply the font index */
		TextFont(fontNum);

		/* Remove non-mono-spaced fonts */
		if ((CharWidth('i') != CharWidth('W')) || (CharWidth('W') == 0))
		{
			/* Delete the menu item */
			DeleteMenuItem(m, i);
		}
	}

	/* Destroy the fake window */
	DisposeWindow(tmpw);

	/* Add a separator */
	AppendMenu(m, "\p-");

	/* Add the fonts to the menu */
	AppendResMenu(m, 'FONT');


#ifndef USE_NIB

	/* Size menu (id 132) */
	m = GetMenuHandle(MENU_SIZE);

	/* Add some sizes (stagger choices) */
	for (i = 8, n = 1; i <= 32; i += ((i / 16) + 1), n++)
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);

		/* Remember its value, for we can't be sure it's in ASCII */
		menu_size_values[n] = i;
	}

#endif /* !USE_NIB */


	/* Windows menu (id 133) */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Default choices */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		Str15 buf;

		/* Describe the item */
		strnfmt((char*)buf + 1, 15, "%.15s", angband_term_name[i]);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);

		/* Command-Key shortcuts */
		if (i < 8) SetItemCmd(m, i + 1, I2D(i));
	}


#ifndef USE_NIB

# ifndef MAC_MPW

	/* CW or gcc -- Use recommended interface for hierarchical menus */

	/* Special menu (id 134) */
	m = GetMenuHandle(MENU_SPECIAL);

	/* Insert Graphics submenu (id 144) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_GRAPH);

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_GRAPH, submenu);
	}

	/* Insert TileWidth submenu (id 145) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_TILEWIDTH);

		/* Add some sizes */
		for (i = 4, n = 1; i <= 32; i++, n++)
		{
			Str15 buf;

			/* Textual size */
			strnfmt((char*)buf + 1, 15, "%d", i);
			buf[0] = strlen((char*)buf + 1);

			/* Append item */
			AppendMenu(submenu, buf);

			/* Remember its value, for we can't be sure it's in ASCII */
			menu_tilewidth_values[n] = i;
		}

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_TILEWIDTH, submenu);
	}

	/* Insert TileHeight submenu (id 146) */
	{
		MenuHandle submenu;

		/* Get the submenu */
		submenu = GetMenu(SUBMENU_TILEHEIGHT);


		/* Add some sizes */
		for (i = 4, n = 1; i <= 32; i++, n++)
		{
			Str15 buf;

			/* Textual size */
			strnfmt((char*)buf + 1, 15, "%d", i);
			buf[0] = strlen((char*)buf + 1);

			/* Append item */
			AppendMenu(submenu, buf);

			/* Remember its value, for we can't be sure it's in ASCII */
			menu_tileheight_values[n] = i;
		}

		/* Insert it */
		SetMenuItemHierarchicalMenu(m, ITEM_TILEHEIGHT, submenu);
	}

# else /* !MAC_MPW */

	/* XXX XXX */

	/* MPW's Universal Interface doesn't understand some newer Carbon APIs */

	/* Special menu (id 134) */

	/* Get graphics (sub)menu (id 144) */
	m = GetMenu(SUBMENU_GRAPH);

	/* Insert it as a submenu */		
	InsertMenu(m, hierMenu);


	/* Get TileWidth (sub)menu (id 145) */
	m = GetMenu(SUBMENU_TILEWIDTH);

	/* Add some sizes */
	for (i = 4, n = 1; i <= 32; i++, n++)
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);

		/* Remember its value, for we can't be sure it's in ASCII */
		menu_tilewidth_values[n] = i;
	}

	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);

	/* Get TileHeight (sub)menu (id 146) */
	m = GetMenu(SUBMENU_TILEHEIGHT);

	/* Add some sizes */
	for (i = 4, n = 1; i <= 32; i++, n++)
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);

		/* Remember its value, for we can't be sure it's in ASCII */
		menu_tileheight_values[n] = i;
	}

	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);

# endif /* MAC_MPW */

#endif /* !USE_NIB */


	/* Update the menu bar */
	DrawMenuBar();
}


/*
 * Prepare the menus
 *
 * It is very important that the player not be allowed to "save" the game
 * unless the "inkey_flag" variable is set, indicating that the game is
 * waiting for a new command.  XXX XXX XXX
 */

static void setup_menus(void)
{
	int i, n;

	short value;

	Str255 s;

	MenuHandle m;

	WindowRef w;

	term_data *td = NULL;


	/* Find window */
	w = FrontWindow();

	/* Relevant "term_data" */
	if (w != NULL) td = (term_data *)GetWRefCon(w);


	/* File menu */
	m = GetMenuHandle(MENU_FILE);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
	}

	/* Enable "new"/"open..."/"import..." */
	if (initialized && !game_in_progress)
	{
		EnableMenuItem(m, ITEM_NEW);
		EnableMenuItem(m, ITEM_OPEN);
		EnableMenuItem(m, ITEM_IMPORT);
	}

	/* Enable "close" */
	if (initialized)
	{
		EnableMenuItem(m, ITEM_CLOSE);
	}

	/* Enable "save" */
	if (initialized && character_generated && inkey_flag)
	{
		EnableMenuItem(m, ITEM_SAVE);
	}

#ifdef HAS_SCORE_MENU

	/* Enable "score" */
	if (initialized && character_generated && !character_icky)
	{
		EnableMenuItem(m, ITEM_SCORE);
	}

#endif /* HAS_SCORE_MENU */

	/* Enable "quit" */
	if (!is_aqua)
	{
		if (!initialized || !character_generated || inkey_flag)
		{
			EnableMenuItem(m, ITEM_QUIT);
		}
	}


	/* Edit menu */
	m = GetMenuHandle(MENU_EDIT);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
	}

	/* Enable "edit" options if "needed" */
	if (!td)
	{
		EnableMenuItem(m, ITEM_UNDO);
		EnableMenuItem(m, ITEM_CUT);
		EnableMenuItem(m, ITEM_COPY);
		EnableMenuItem(m, ITEM_PASTE);
		EnableMenuItem(m, ITEM_CLEAR);
	}


	/* Font menu */
	m = GetMenuHandle(MENU_FONT);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
	}

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, ITEM_BOLD, bold); */

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, ITEM_WIDE, extend); */

	/* Active window */
	if (initialized && td)
	{
		/* Enable "bold" */
		EnableMenuItem(m, ITEM_BOLD);

		/* Enable "extend" */
		EnableMenuItem(m, ITEM_WIDE);

		/* Check the appropriate "bold-ness" */
		if (td->font_face & bold) CheckMenuItem(m, ITEM_BOLD, TRUE);

		/* Check the appropriate "wide-ness" */
		if (td->font_face & extend) CheckMenuItem(m, ITEM_WIDE, TRUE);

		/* Analyze fonts */
		for (i = 4; i <= n; i++)
		{
			/* Enable it */
			EnableMenuItem(m, i);

			/* Analyze font */
			GetMenuItemText(m, i, s);
			GetFNum(s, &value);

			/* Check active font */
			if (td->font_id == value) CheckMenuItem(m, i, TRUE);
		}
	}


	/* Size menu */
	m = GetMenuHandle(MENU_SIZE);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableMenuItem(m, i);
		CheckMenuItem(m, i, FALSE);
	}

	/* Active window */
	if (initialized && td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			value = menu_size_values[i];

			/* Enable the "real" sizes */
			if (RealFont(td->font_id, value)) EnableMenuItem(m, i);

			/* Check the current size */
			if (td->font_size == value) CheckMenuItem(m, i, TRUE);
		}
	}


	/* Windows menu */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Check active windows */
	for (i = 1; i <= n; i++)
	{
		/* Check if needed */
		CheckMenuItem(m, i, data[i-1].mapped);
	}


	/* Special menu */
	m = GetMenuHandle(MENU_SPECIAL);

	/* Get menu size */
	n = CountMenuItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableMenuItem(m, i);

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* XXX Oh no, this removes submenu... */
		if ((i != ITEM_GRAPH) &&
		    (i != ITEM_TILEWIDTH) &&
		    (i != ITEM_TILEHEIGHT)) CheckMenuItem(m, i, FALSE);

#else

		CheckMenuItem(m, i, FALSE);
#endif
	}

	/* Item "arg_sound" */
	EnableMenuItem(m, ITEM_SOUND);
	CheckMenuItem(m, ITEM_SOUND, arg_sound);

	/* Item "Graphics" */
	EnableMenuItem(m, ITEM_GRAPH);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* Graphics submenu */
		submenu = GetMenuHandle(SUBMENU_GRAPH);

#else

		/* Graphics submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_GRAPH, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Item "None" */
		EnableMenuItem(submenu, ITEM_NONE);
		CheckMenuItem(submenu, ITEM_NONE, (graf_mode_req == GRAF_MODE_NONE));

		/* Item "8x8" */
		EnableMenuItem(submenu, ITEM_8X8);
		CheckMenuItem(submenu, ITEM_8X8, (graf_mode_req == GRAF_MODE_8X8));

		/* Item "16x16" */
		EnableMenuItem(submenu, ITEM_16X16);
		CheckMenuItem(submenu, ITEM_16X16, (graf_mode_req == GRAF_MODE_16X16));

		/* Item "32x32" */
		EnableMenuItem(submenu, ITEM_32X32);
		CheckMenuItem(submenu, ITEM_32X32, (graf_mode_req == GRAF_MODE_32X32));

#ifdef USE_DOUBLE_TILES

		/* Item "Big tiles" */
		if (inkey_flag) EnableMenuItem(submenu, ITEM_BIGTILE);
		CheckMenuItem(submenu, ITEM_BIGTILE, use_bigtile);

#endif /* USE_DOUBLE_TILES */

	}

	/* Item "TileWidth" */
	EnableMenuItem(m, ITEM_TILEWIDTH);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* TIleWidth submenu */
		submenu = GetMenuHandle(SUBMENU_TILEWIDTH);

#else

		/* TileWidth submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_TILEWIDTH, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Active window */
		if (initialized && td)
		{
			/* Analyze sizes */
			for (i = 1; i <= n; i++)
			{
				/* Analyze size */
				value = menu_tilewidth_values[i];

				/* Enable */
				if (value >= td->font_wid) EnableMenuItem(submenu, i);

				/* Check the current size */
				if (td->tile_wid == value) CheckMenuItem(submenu, i, TRUE);
			}
		}
	}

	/* Item "TileHeight" */
	EnableMenuItem(m, ITEM_TILEHEIGHT);
	{
		MenuRef submenu;

#ifdef MAC_MPW

		/* MPW's Universal Interface is a bit out of date */

		/* TileHeight submenu */
		submenu = GetMenuHandle(SUBMENU_TILEHEIGHT);

#else

		/* TileWidth submenu */
		(void)GetMenuItemHierarchicalMenu(m, ITEM_TILEHEIGHT, &submenu);

#endif

		/* Get menu size */
		n = CountMenuItems(submenu);

		/* Reset menu */
		for (i = 1; i <= n; i++)
		{
			/* Reset */
			DisableMenuItem(submenu, i);
			CheckMenuItem(submenu, i, FALSE);
		}

		/* Active window */
		if (initialized && td)
		{
			/* Analyze sizes */
			for (i = 1; i <= n; i++)
			{
				/* Analyze size */
				value = menu_tileheight_values[i];

				/* Enable */
				if (value >= td->font_hgt) EnableMenuItem(submenu, i);

				/* Check the current size */
				if (td->tile_hgt == value) CheckMenuItem(submenu, i, TRUE);
			}
		}
	}

	/* Item "arg_fiddle" */
	EnableMenuItem(m, ITEM_FIDDLE);
	CheckMenuItem(m, ITEM_FIDDLE, arg_fiddle);

	/* Item "arg_wizard" */
	EnableMenuItem(m, ITEM_WIZARD);
	CheckMenuItem(m, ITEM_WIZARD, arg_wizard);
}


/*
 * Process a menu selection (see above)
 *
 * Hack -- assume that invalid menu selections are disabled above,
 * which I have been informed may not be reliable.  XXX XXX XXX
 */
static void menu(long mc)
{
	int i;

	int menuid, selection;

	static unsigned char s[1000];

	short fid;

	term_data *td = NULL;

	WindowPtr old_win;


	/* Analyze the menu command */
	menuid = HiWord(mc);
	selection = LoWord(mc);


	/* Find the window XXX XXX Declare another variable */
	old_win = FrontWindow();

	/* Relevant "term_data" */
	if (old_win) td = (term_data *)GetWRefCon(old_win);


	/* Branch on the menu */
	switch (menuid)
	{
		/* Apple Menu */
		case MENU_APPLE:
		{
			/* About Angband... */
			if (selection == ITEM_ABOUT)
			{
				DialogPtr dialog;
				short item_hit;

				/* Get the about dialogue */
				dialog = GetNewDialog(128, 0, (WindowPtr)-1);

				/* Move it to the middle of the screen */
				RepositionWindow(
					GetDialogWindow(dialog),
					NULL,
					kWindowCenterOnMainScreen);

				/* Show the dialog */
				TransitionWindow(GetDialogWindow(dialog),
					kWindowZoomTransitionEffect,
					kWindowShowTransitionAction,
					NULL);

				/* Wait for user to click on it */
				ModalDialog(0, &item_hit);

				/* Free the dialogue */
				DisposeDialog(dialog);
				break;
			}

			break;
		}

		/* File Menu */
		case MENU_FILE:
		{
			switch (selection)
			{
				/* New */
				case ITEM_NEW:
				{
					do_menu_file_new();
					break;
				}

				/* Open... */
				case ITEM_OPEN:
				{
					do_menu_file_open(FALSE);
					break;
				}

				/* Import... */
				case ITEM_IMPORT:
				{
					do_menu_file_open(TRUE);
					break;
				}

				/* Close */
				case ITEM_CLOSE:
				{
					/* No window */
					if (!td) break;

					/* Not Mapped */
					td->mapped = FALSE;

					/* Not Mapped */
					td->t->mapped_flag = FALSE;

					/* Hide the window */
					TransitionWindow(td->w,
						kWindowZoomTransitionEffect,
						kWindowHideTransitionAction,
						NULL);

					break;
				}

				/* Save */
				case ITEM_SAVE:
				{
					/* Hack -- Forget messages */
					msg_flag = FALSE;

					/* Hack -- Save the game */
#ifndef ZANG_AUTO_SAVE
					do_cmd_save_game();
#else
					do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */

					break;
				}

#ifdef HAS_SCORE_MENU

				/* Show score */
				case ITEM_SCORE:
				{
					char buf[1024];

					/* Paranoia */
					if (!initialized || character_icky ||
					    !game_in_progress || !character_generated)
					{
						/* Can't happen but just in case */
						plog("You may not do that right now.");

						break;
					}

					/* Build the pathname of the score file */
					path_build(buf, sizeof(buf), ANGBAND_DIR_APEX,
						"scores.raw");

					/* Hack - open the score file for reading */
					highscore_fd = fd_open(buf, O_RDONLY);

					/* Paranoia - No score file */
					if (highscore_fd < 0)
					{
						msg_print("Score file is not available.");

						break;
					}

					/* Mega-Hack - prevent various functions XXX XXX XXX */
					initialized = FALSE;

					/* Save screen */
					screen_save();

					/* Clear screen */
					Term_clear();

					/* Prepare scores */
					if (game_in_progress && character_generated)
					{
						predict_score();
					}

#if 0 /* I don't like this - pelpel */

					/* Mega-Hack - No current player XXX XXX XXX XXX */
					else
					{
						display_scores_aux(0, MAX_HISCORES, -1, NULL);
					}

#endif

					/* Close the high score file */
					(void)fd_close(highscore_fd);

					/* Forget the fd */
					highscore_fd = -1;

					/* Restore screen */
					screen_load();

					/* Hack - Flush it */
					Term_fresh();

					/* Mega-Hack - We are ready again */
					initialized = TRUE;

					/* Done */
					break;
				}

#endif /* HAS_SCORE_MENU */

				/* Quit (with save) */
				case ITEM_QUIT:
				{
					/* Save the game (if necessary) */
					if (game_in_progress && character_generated)
					{
						/* Hack -- Forget messages */
						msg_flag = FALSE;

						/* Save the game */
#ifndef ZANG_AUTO_SAVE
						do_cmd_save_game();
#else
						do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */
					}

					/* Quit */
					quit(NULL);
					break;
				}
			}
			break;
		}

		/* Edit menu */
		case MENU_EDIT:
		{
			/* Unused */
			break;
		}

		/* Font menu */
		case MENU_FONT:
		{
			/* Require a window */
			if (!td) break;

			/* Memorize old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Toggle the "bold" setting */
			if (selection == ITEM_BOLD)
			{
				/* Toggle the setting */
				if (td->font_face & bold)
				{
					td->font_face &= ~bold;
				}
				else
				{
					td->font_face |= bold;
				}

				/* Hack - clear tile size info XXX XXX */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Toggle the "wide" setting */
			if (selection == ITEM_WIDE)
			{
				/* Toggle the setting */
				if (td->font_face & extend)
				{
					td->font_face &= ~extend;
				}
				else
				{
					td->font_face |= extend;
				}

				/* Hack - clear tile size info XXX XXX */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Get a new font name */
			GetMenuItemText(GetMenuHandle(MENU_FONT), selection, s);
			GetFNum(s, &fid);

			/* Save the new font id */
			td->font_id = fid;

			/* Current size is bad for new font */
			if (!RealFont(td->font_id, td->font_size))
			{
				/* Find similar size */
				for (i = 1; i <= 32; i++)
				{
					/* Adjust smaller */
					if (td->font_size - i >= 8)
					{
						if (RealFont(td->font_id, td->font_size - i))
						{
							td->font_size -= i;
							break;
						}
					}

					/* Adjust larger */
					if (td->font_size + i <= 128)
					{
						if (RealFont(td->font_id, td->font_size + i))
						{
							td->font_size += i;
							break;
						}
					}
				}
			}

			/* Hack - clear tile size info XXX XXX */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore the window */
			activate(old_win);

			break;
		}

		/* Size menu */
		case MENU_SIZE:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			td->font_size = menu_size_values[selection];

			/* Hack - clear tile size info XXX XXX */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* Window menu */
		case MENU_WINDOWS:
		{
			/* Parse */
			i = selection - 1;

			/* Check legality of choice */
			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			/* Obtain the window */
			td = &data[i];

			/* Mapped */
			td->mapped = TRUE;

			/* Link */
			term_data_link(i);

			/* Mapped (?) */
			td->t->mapped_flag = TRUE;

			/* Show the window */
			TransitionWindow(td->w,
				kWindowZoomTransitionEffect,
				kWindowShowTransitionAction,
				NULL);

			/* Bring to the front */
			SelectWindow(td->w);

			break;
		}

		/* Special menu */
		case MENU_SPECIAL:
		{
			switch (selection)
			{
				case ITEM_SOUND:
				{
					/* Toggle arg_sound */
					arg_sound = !arg_sound;

					/* React to changes */
					Term_xtra(TERM_XTRA_REACT, 0);

					break;
				}

				case ITEM_FIDDLE:
				{
					arg_fiddle = !arg_fiddle;

					break;
				}

				case ITEM_WIZARD:
				{
					arg_wizard = !arg_wizard;

					break;
				}
			}

			break;
		}

		/* Graphics submenu */
		case SUBMENU_GRAPH:
		{
			switch (selection)
			{
				case ITEM_NONE:
				{
					graf_mode_req = GRAF_MODE_NONE;

					break;
				}

				case ITEM_8X8:
				{
					graf_mode_req = GRAF_MODE_8X8;

					break;
				}

				case ITEM_16X16:
				{
					graf_mode_req = GRAF_MODE_16X16;

					break;
				}

				case ITEM_32X32:
				{
					graf_mode_req = GRAF_MODE_32X32;

					break;
				}

#ifdef USE_DOUBLE_TILES

				case ITEM_BIGTILE:
				{
					term *old = Term;
					term_data *td = &data[0];

					/* Toggle "use_bigtile" */
					use_bigtile = !use_bigtile;

					/* Activate */
					Term_activate(td->t);

					/* Resize the term */
					Term_resize(td->cols, td->rows);

					/* Activate old */
					Term_activate(old);

					break;
				}

#endif /* USE_DOUBLE_TILES */

			}

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

		/* TileWidth menu */
		case SUBMENU_TILEWIDTH:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Analyse value */
			td->tile_wid = menu_tilewidth_values[selection];

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* TileHeight menu */
		case SUBMENU_TILEHEIGHT:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Analyse value */
			td->tile_hgt = menu_tileheight_values[selection];

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}
	}


	/* Clean the menu */
	HiliteMenu(0);
}


/*
 * Check for extra required parameters -- From "Maarten Hazewinkel"
 */
static OSErr CheckRequiredAEParams(const AppleEvent *theAppleEvent)
{
	OSErr aeError;
	DescType returnedType;
	Size actualSize;

	aeError = AEGetAttributePtr(
		theAppleEvent, keyMissedKeywordAttr, typeWildCard,
		&returnedType, NULL, 0, &actualSize);

	if (aeError == errAEDescNotFound) return (noErr);

	if (aeError == noErr) return (errAEParamMissed);

	return (aeError);
}


/*
 * Apple Event Handler -- Open Application
 */
static pascal OSErr AEH_Start(const AppleEvent *theAppleEvent, AppleEvent *reply,
	SInt32 handlerRefCon)
{
#pragma unused(reply)
#pragma unused(handlerRefCon)

	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Quit Application
 */
static pascal OSErr AEH_Quit(const AppleEvent *theAppleEvent, AppleEvent *reply,
	SInt32 handlerRefCon)
{
#pragma unused(reply)
#pragma unused(handlerRefCon)

	/* Quit later */
	quit_when_ready = TRUE;

	/* Check arguments */
	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Print Documents
 */
static pascal OSErr AEH_Print(const AppleEvent *theAppleEvent, AppleEvent *reply,
	SInt32 handlerRefCon)
{
#pragma unused(theAppleEvent)
#pragma unused(reply)
#pragma unused(handlerRefCon)

	return (errAEEventNotHandled);
}


/*
 * Apple Event Handler by Steve Linberg (slinberg@crocker.com).
 *
 * The old method of opening savefiles from the finder does not work
 * on the Power Macintosh, because CountAppFiles and GetAppFiles,
 * used to return information about the selected document files when
 * an application is launched, are part of the Segment Loader, which
 * is not present in the RISC OS due to the new memory architecture.
 *
 * The "correct" way to do this is with AppleEvents.  The following
 * code is modeled on the "Getting Files Selected from the Finder"
 * snippet from Think Reference 2.0.  (The prior sentence could read
 * "shamelessly swiped & hacked")
 */
static pascal OSErr AEH_Open(const AppleEvent *theAppleEvent, AppleEvent* reply,
	SInt32 handlerRefCon)
{
	FSSpec myFSS;
	AEDescList docList;
	OSErr err;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	char msg[128];
	FInfo myFileInfo;

#pragma unused(reply)
#pragma unused(handlerRefCon)

	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(
		theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;

	/*
	 * We ignore the validity check, because we trust the FInder, and we only
	 * allow one savefile to be opened, so we ignore the depth of the list.
	 */
	err = AEGetNthPtr(
		&docList, 1L, typeFSS, &keywd, &returnedType,
		(Ptr) &myFSS, sizeof(myFSS), &actualSize);
	if (err) return err;

	/* Only needed to check savefile type below */
	err = FSpGetFInfo(&myFSS, &myFileInfo);
	if (err)
	{
		strnfmt(msg, sizeof(msg), "Argh!  FSpGetFInfo failed with code %d", err);
		mac_warning(msg);
		return err;
	}

	/* Ignore non 'SAVE' files */
	if (myFileInfo.fdType != 'SAVE') return noErr;

#ifdef MACH_O_CARBON

	/* Extract a file name */
	(void)spec_to_path(&myFSS, savefile, sizeof(savefile));

#else

	/* XXX XXX XXX Extract a file name */
	PathNameFromDirID(myFSS.parID, myFSS.vRefNum, (StringPtr)savefile);
	pstrcat((StringPtr)savefile, (StringPtr)&myFSS.name);

	/* Convert the string */
	ptocstr((StringPtr)savefile);

#endif /* MACH_O_CARBON */

	/* Delay actual open */
	open_when_ready = TRUE;

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}


/*
 * Apple Event Handler -- Re-open Application
 *
 * If no windows are currently open, show the Angband window.
 * This required AppleEvent was introduced by System 8 -- pelpel
 */
static pascal OSErr AEH_Reopen(const AppleEvent *theAppleEvent,
			     AppleEvent* reply, long handlerRefCon)
{
#pragma unused(theAppleEvent, reply, handlerRefCon)

	term_data *td = NULL;

	/* No open windows */
	if (NULL == FrontWindow())
	{
		/* Obtain the Angband window */
		td = &data[0];

		/* Mapped */
		td->mapped = TRUE;

		/* Link */
		term_data_link(0);

		/* Mapped (?) */
		td->t->mapped_flag = TRUE;

		/* Show the window */
		ShowWindow(td->w);

		/* Bring to the front */
		SelectWindow(td->w);

		/* Make it active */
		activate(td->w);
	}

	/* Event handled */
	return (noErr);
}


/*
 * Handle quit_when_ready, by Peter Ammon,
 * slightly modified to check inkey_flag.
 */
static void quit_calmly(void)
{
	/* Quit immediately if game's not started */
	if (!game_in_progress || !character_generated) quit(NULL);

	/* Save the game and Quit (if it's safe) */
	if (inkey_flag)
	{
		/* Hack -- Forget messages */
		msg_flag = FALSE;

		/* Save the game */
#ifndef ZANG_AUTO_SAVE
		do_cmd_save_game();
#else
		do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */

		/* Quit */
		quit(NULL);
	}

	/* Wait until inkey_flag is set */
}


/*
 * Macintosh modifiers (event.modifier & ccc):
 *   cmdKey, optionKey, shiftKey, alphaLock, controlKey
 *
 *
 * Macintosh Keycodes (0-63 normal, 64-95 keypad, 96-127 extra):
 *
 * Return:36
 * Delete:51
 *
 * Period:65
 * Star:67
 * Plus:69
 * Clear:71
 * Slash:75
 * Enter:76
 * Minus:78
 * Equal:81
 * 0-7:82-89
 * 8-9:91-92
 *
 * backslash/vertical bar (Japanese keyboard):93
 *
 * F5: 96
 * F6: 97
 * F7: 98
 * F3:99
 * F8:100
 * F10:101
 * F11:103
 * F13:105
 * F14:107
 * F9:109
 * F12:111
 * F15:113
 * Help:114
 * Home:115
 * PgUp:116
 * Del:117
 * F4: 118
 * End:119
 * F2:120
 * PgDn:121
 * F1:122
 * Lt:123
 * Rt:124
 * Dn:125
 * Up:126
 */


/*
 * Optimize non-blocking calls to "CheckEvents()"
 * Idea from "Maarten Hazewinkel <mmhazewi@cs.ruu.nl>"
 *
 * WAS: 6. The value of one (~ 60 FPS) seems to work better with the Borg,
 * and so should be for other CPU-intensive features like the autoroller.
 */
#define EVENT_TICKS 1


/*
 * Check for Events, return TRUE if we process any
 */
static bool CheckEvents(int wait)
{
	EventRecord event;

	WindowPtr w;

	Rect r;

	UInt32 sleep_ticks;

	int ch, ck;

	int mc, ms, mo, mx;

	term_data *td = NULL;

	UInt32 curTicks;

	static UInt32 lastTicks = 0L;


	/* Access the clock */
	curTicks = TickCount();

	/* Hack -- Allow efficient checking for non-pending events */
	if ((wait == CHECK_EVENTS_NO_WAIT) &&
		(curTicks < lastTicks + EVENT_TICKS)) return (FALSE);

	/* Timestamp last check */
	lastTicks = curTicks;


	/* Handles the quit_when_ready flag */
	if (quit_when_ready) quit_calmly();

	/* Blocking call to WaitNextEvent - should use MAX_INT XXX XXX */
	if (wait == CHECK_EVENTS_WAIT) sleep_ticks = 0x7FFFFFFFL;

	/* Non-blocking */
	else sleep_ticks = 0L;

	/* Get an event (or null)  */
	WaitNextEvent(everyEvent, &event, sleep_ticks, nil);

	/* Hack -- Nothing is ready yet */
	if (event.what == nullEvent) return (FALSE);


	/* Analyze the event */
	switch (event.what)
	{

#if 0

		case activateEvt:
		{
			w = (WindowPtr)event.message;

			activate(w);

			break;
		}

#endif

		case updateEvt:
		{
			/* Extract the window */
			w = (WindowPtr)event.message;

			/* Relevant "term_data" */
			td = (term_data *)GetWRefCon(w);

			/* Clear window's update region and clip drawings with it */
			BeginUpdate(w);

			/* Redraw the window */
			if (td) term_data_redraw(td);

			/* Restore window's clipping region */
			EndUpdate(w);

			break;
		}

		case keyDown:
		case autoKey:
		{
			/* Extract some modifiers */
			mc = (event.modifiers & controlKey) ? TRUE : FALSE;
			ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
			mo = (event.modifiers & optionKey) ? TRUE : FALSE;
			mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

			/* Keypress: (only "valid" if ck < 96) */
			ch = (event.message & charCodeMask) & 255;

			/* Keycode: see table above */
			ck = ((event.message & keyCodeMask) >> 8) & 255;

			/* Command + "normal key" -> menu action */
			if (mx && (ck < 64))
			{
				/* Hack -- Prepare the menus */
				setup_menus();

				/* Run the Menu-Handler */
				menu(MenuKey(ch));

				/* Turn off the menus */
				HiliteMenu(0);

				/* Done */
				break;
			}


			/* Hide the mouse pointer */
			ObscureCursor();

			/* Normal key -> simple keypress */
			if ((ck < 64) || (ck == 93))
			{
				/* Enqueue the keypress */
				Term_keypress(ch);
			}

			/* Keypad keys -> trigger plus simple keypress */
			else if (!mc && !ms && !mo && !mx && (ck < 96))
			{
				/* Hack -- "enter" is confused */
				if (ck == 76) ch = '\n';

				/* Begin special trigger */
				Term_keypress(31);

				/* Send the "keypad" modifier */
				Term_keypress('K');

				/* Terminate the trigger */
				Term_keypress(13);

				/* Send the "ascii" keypress */
				Term_keypress(ch);
			}

			/* Bizarre key -> encoded keypress */
			else if (ck <= 127)
			{
				/* Begin special trigger */
				Term_keypress(31);

				/* Send some modifier keys */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (mo) Term_keypress('O');
				if (mx) Term_keypress('X');

				/* Downshift and encode the keycode */
				Term_keypress(I2D((ck - 64) / 10));
				Term_keypress(I2D((ck - 64) % 10));

				/* Terminate the trigger */
				Term_keypress(13);
			}

			break;
		}

		case mouseDown:
		{
			int code;

			/* Analyze click location */
			code = FindWindow(event.where, &w);

			/* Relevant "term_data" */
			td = (term_data *)GetWRefCon(w);

			/* Analyze */
			switch (code)
			{
				case inMenuBar:
				{
					setup_menus();
					menu(MenuSelect(event.where));
					HiliteMenu(0);
					break;
				}

				case inDrag:
				{
					WindowPtr old_win;
					BitMap tBitMap;
					Rect pRect;

					r = GetQDGlobalsScreenBits(&tBitMap)->bounds;
					r.top += 20; /* GetMBarHeight() XXX XXX XXX */
					InsetRect(&r, 4, 4);
					DragWindow(w, event.where, &r);

					/* Oops */
					if (!td) break;

					/* Save */
					old_win = active;

					/* Activate */
					activate(td->w);

					/* Analyze */
					GetWindowBounds(
						(WindowRef)td->w,
						kWindowContentRgn,
						&pRect);
					td->r.left = pRect.left;
					td->r.top = pRect.top;

					/* Apply and Verify */
					term_data_check_size(td);

					/* Restore */
					activate(old_win);

					break;
				}

				case inGoAway:
				{
					/* Oops */
					if (!td) break;

					/* Track the go-away box */
					if (TrackGoAway(w, event.where))
					{
						/* Not Mapped */
						td->mapped = FALSE;

						/* Not Mapped */
						td->t->mapped_flag = FALSE;

						/* Hide the window */
						TransitionWindow(td->w,
							kWindowZoomTransitionEffect,
							kWindowHideTransitionAction,
							NULL);
					}

					break;
				}

				case inGrow:
				{
					int x, y;

					Rect nr;

					term *old = Term;

					/* Oops */
					if (!td) break;

#ifndef ALLOW_BIG_SCREEN

					/* Minimum and maximum sizes */
					r.left = 20 * td->tile_wid + td->size_ow1;
					r.right = 80 * td->tile_wid + td->size_ow1 + td->size_ow2 + 1;
					r.top = 1 * td->tile_hgt + td->size_oh1;
					r.bottom = 24 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

					/* Grow the rectangle */
					if (!ResizeWindow(w, event.where, &r, NULL)) break;
#else

					/* Grow the rectangle */
					if (!ResizeWindow(w, event.where, NULL, NULL)) break;

#endif /* !ALLOW_BIG_SCREEN */


					/* Obtain geometry of resized window */
					GetWindowBounds(w, kWindowContentRgn, &nr);

					/* Extract the new size in pixels */
					y = nr.bottom - nr.top - td->size_oh1 - td->size_oh2;
					x = nr.right - nr.left - td->size_ow1 - td->size_ow2;

					/* Extract a "close" approximation */
					td->rows = y / td->tile_hgt;
					td->cols = x / td->tile_wid;

					/* Apply and Verify */
					term_data_check_size(td);

					/* Activate */
					Term_activate(td->t);

					/* Hack -- Resize the term */
					Term_resize(td->cols, td->rows);

					/* Resize and Redraw */
					term_data_resize(td);
					term_data_redraw(td);

					/* Restore */
					Term_activate(old);

					break;
				}

				case inContent:
				{
					SelectWindow(w);

					break;
				}
			}

			break;
		}

		/* OS Event -- From "Maarten Hazewinkel" */
		case osEvt:
		{
			switch ((event.message >> 24) & 0x000000FF)
			{
				case suspendResumeMessage:

				/* Resuming: activate the front window */
				if (event.message & resumeFlag)
				{
					Cursor tempCursor;
					WindowRef w;

					/* Find the window */
					w = FrontWindow();
					
					if (w != NULL)
					{
						/* Relevant "term_data" */
						td = (term_data *)GetWRefCon(FrontWindow());

						/* Activate the window */
						SetPort(GetWindowPort(td->w));

						/* Mega-Hack -- Synchronise 'active' */
						active = td->w;

						SetCursor(GetQDGlobalsArrow(&tempCursor));

						/* Synchronise term */
						Term_activate(td->t);
					}
				}

				/* Suspend: deactivate the front window */
				else
				{
					/* Nothing */
				}

				break;
			}

			break;
		}

		/* From "Steve Linberg" and "Maarten Hazewinkel" */
		case kHighLevelEvent:
		{
			/* Process apple events */
			(void)AEProcessAppleEvent(&event);

			/* Handle "quit_when_ready" */
			if (quit_when_ready)
			{
#if 0 /* Doesn't work with Aqua well */
				/* Forget */
				quit_when_ready = FALSE;

				/* Do the menu key */
				menu(MenuKey('q'));
#endif
				/* Turn off the menus */
				HiliteMenu(0);
			}

			/* Handle "open_when_ready" */
			else if (open_when_ready)
			{
				handle_open_when_ready();
			}

			break;
		}

	}


	/* Something happened */
	return (TRUE);
}


/*** Some Hooks for various routines ***/


/*
 * Mega-Hack -- emergency lifeboat
 */
static void *lifeboat = NULL;


/*
 * Hook to "release" memory
 */
#ifdef NEW_ZVIRT_HOOKS /* [V] removed the unused 'size' argument. */
static void *hook_rnfree(void *v)
#else
static void *hook_rnfree(void *v, huge size)
#endif /* NEW_ZVIRT_HOOKS */
{

#ifdef USE_MALLOC

	/* Alternative method */
	free(v);

#else

	/* Dispose */
	DisposePtr(v);

#endif

	/* Success */
	return (NULL);
}

/*
 * Hook to "allocate" memory
 */
static void *hook_ralloc(huge size)
{

#ifdef USE_MALLOC

	/* Make a new pointer */
	return (malloc(size));

#else

	/* Make a new pointer */
	return (NewPtr(size));

#endif

}

/*
 * Hook to handle "out of memory" errors
 */
static void *hook_rpanic(huge size)
{
#pragma unused(size)

	/* Free the lifeboat */
	if (lifeboat)
	{
		/* Free the lifeboat */
		DisposePtr(lifeboat);

		/* Forget the lifeboat */
		lifeboat = NULL;

		/* Mega-Hack -- Warning */
		mac_warning("Running out of Memory!\rAbort this process now!");

		/* Mega-Hack -- Never leave this function */
		while (TRUE) CheckEvents(CHECK_EVENTS_WAIT);
	}

	/* Mega-Hack -- Crash */
	return (NULL);
}


/*
 * Hook to tell the user something important
 */
static void hook_plog(cptr str)
{
	/* Warning message */
	mac_warning(str);
}


/*
 * Hook to tell the user something, and then quit
 */
static void hook_quit(cptr str)
{
	/* Warning if needed */
	if (str) mac_warning(str);

#ifdef USE_ASYNC_SOUND

	/* Clean up sound support */
	cleanup_sound();

#endif /* USE_ASYNC_SOUND */

	/* Dispose of graphic tiles */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);
	}

	/* Write a preference file */
	if (initialized) save_pref_file();

	/* All done */
	ExitToShell();
}


/*
 * Hook to tell the user something, and then crash
 */
static void hook_core(cptr str)
{
	/* XXX Use the debugger */
	/* DebugStr(str); */

	/* Warning */
	if (str) mac_warning(str);

	/* Warn, then save player */
	mac_warning("Fatal error.\rI will now attempt to save and quit.");

	/* Attempt to save */
	if (!save_player()) mac_warning("Warning -- save failed!");

	/* Quit */
	quit(NULL);
}



/*** Main program ***/


/*
 * Init some stuff
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "Macintosh Save Bug" by using "absolute" path names, since on
 * System 7 machines anyway, the "current working directory" often
 * "changes" due to background processes, invalidating any "relative"
 * path names.  Note that the Macintosh is limited to 255 character
 * path names, so be careful about deeply embedded directories...
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "missing lib folder bug" by allowing the user to help find the
 * "lib" folder by hand if the "application folder" code fails...
 *
 *
 * The problem description above no longer applies, but I left it here,
 * modified for Carbon, to allow the game proceeds when a user doesn't
 * placed the Angband binary and the lib folder in the same place for
 * whatever reasons. -- pelpel
 */
static void init_stuff(void)
{
	Rect r;
	BitMap tBitMap;
	Rect screenRect;
	Point topleft;

	char path[1024];

	OSErr err = noErr;
	NavDialogOptions dialogOptions;
	FSSpec theFolderSpec;
	NavReplyRecord theReply;


	/* Fake rectangle */
	r.left = 0;
	r.top = 0;
	r.right = 344;
	r.bottom = 188;

	/* Center it */
	screenRect = GetQDGlobalsScreenBits(&tBitMap)->bounds;
	center_rect(&r, &screenRect);

	/* Extract corner */
	topleft.v = r.top;
	topleft.h = r.left;


	/* Default to the "lib" folder with the application */
#ifdef MACH_O_CARBON
	if (locate_lib(path, sizeof(path)) == NULL) quit(NULL);
#else
	/* Metrowerks uses colon-separated path */
	refnum_to_name(path, app_dir, app_vol, (char*)("\plib:"));
#endif


	/* Check until done */
	while (1)
	{
		/* Create directories for the users files */
		create_user_dirs();

		/* Prepare the paths */
		init_file_paths(path);

		/* Build the filename */
		path_build(path, sizeof(path), ANGBAND_DIR_FILE, "news.txt");

		/* Attempt to open and close that file */
		if (0 == fd_close(fd_open(path, O_RDONLY))) break;

		/* Warning */
		plog_fmt("Unable to open the '%s' file.", path);

		/* Warning */
		plog("The Angband 'lib' folder is probably missing or misplaced.");

		/* Ask the user to choose the lib folder */
		err = NavGetDefaultDialogOptions(&dialogOptions);

		/* Paranoia */
		if (err != noErr) quit(NULL);

		/* Set default location option */
		dialogOptions.dialogOptionFlags |= kNavSelectDefaultLocation;

		/* Clear preview option */
		dialogOptions.dialogOptionFlags &= ~(kNavAllowPreviews);

		/* Forbit selection of multiple files */
		dialogOptions.dialogOptionFlags &= ~(kNavAllowMultipleFiles);

		/* Display location */
		dialogOptions.location = topleft;

#if 0

		/* Load the message for the missing folder from the resource fork */
		/* GetIndString(dialogOptions.message, 128, 1); */

#else

		/* Set the message for the missing folder XXX XXX */
		strcpy((char *)dialogOptions.message + 1,
			"Please select the \"lib\" folder");
		dialogOptions.message[0] = strlen((char *)dialogOptions.message + 1);

#endif

		/* Wait for the user to choose a folder */
		err = NavChooseFolder(
			nil, &theReply, &dialogOptions, nil, nil, nil);

		/* Assume the player doesn't want to go on */
		if ((err != noErr) || !theReply.validRecord) quit(NULL);

		/* Retrieve FSSpec from the reply */
		{
			AEKeyword theKeyword;
			DescType actualType;
			Size actualSize;

			/* Get a pointer to selected folder */
			err = AEGetNthPtr(
				&(theReply.selection), 1, typeFSS, &theKeyword,
				&actualType, &theFolderSpec, sizeof(FSSpec), &actualSize);

			/* Paranoia */
			if (err != noErr) quit(NULL);
		}

		/* Free navitagor reply */
		err = NavDisposeReply(&theReply);

		/* Paranoia */
		if (err != noErr) quit(NULL);

#ifdef MACH_O_CARBON

		/* Extract textual file name for given file */
		if (spec_to_path(&theFolderSpec, path, sizeof(path)) != noErr)
		{
			quit(NULL);
		}

#else /* MACH_O_CARBON */

		/* Extract textual file name for given file */
		refnum_to_name(
			path,
			theFolderSpec.parID,
			theFolderSpec.vRefNum,
			(char *)theFolderSpec.name);

#endif /* MACH_O_CARBON */

	}
}


/*
 * Macintosh Main loop
 */
int main(void)
{
	long response;
	OSStatus err;
	UInt32 numberOfMasters = 10;


	/* Get more Masters -- it is not recommended by Apple, should go away */
	MoreMasterPointers(numberOfMasters);

	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Initialise the cursor and turn it into an "arrow" */
	InitCursor();


	/* Check for existence of Carbon */
	err = Gestalt(gestaltCarbonVersion, &response);

	if (err != noErr) quit("This program requires Carbon API");

	/* See if we are running on Aqua */
	err = Gestalt(gestaltMenuMgrAttr, &response);

	/* Cache the result */
	if ((err == noErr) &&
	    (response & gestaltMenuMgrAquaLayoutMask)) is_aqua = TRUE;

	/* 
	 * Remember Mac OS version, in case we have to cope with version-specific
	 * problems
	 */
	(void)Gestalt(gestaltSystemVersion, &mac_os_version);


	/* Install the start event hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEOpenApplication,
		NewAEEventHandlerUPP(AEH_Start),
		0L,
		FALSE);

	/* Install the quit event hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEQuitApplication,
		NewAEEventHandlerUPP(AEH_Quit),
		0L,
		FALSE);

	/* Install the print event hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEPrintDocuments,
		NewAEEventHandlerUPP(AEH_Print),
		0L,
		FALSE);

	/* Install the open event hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEOpenDocuments,
		NewAEEventHandlerUPP(AEH_Open),
		0L,
		FALSE);

	/* Install the Re-open event hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEReopenApplication,
		NewAEEventHandlerUPP(AEH_Reopen),
		0L,
		FALSE);


#ifndef MACH_O_CARBON

	/* Find the current application */
	SetupAppDir();

#endif /* !MACH_O_CARBON */


	/* Mark ourself as the file creator */
	_fcreator = ANGBAND_CREATOR;

	/* Default to saving a "text" file */
	_ftype = 'TEXT';


	/* Hook in some "z-virt.c" hooks */
	rnfree_aux = hook_rnfree;
	ralloc_aux = hook_ralloc;
	rpanic_aux = hook_rpanic;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;


	/* Initialize colors */
	update_colour_info();


	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));

	/* Prepare the menubar */
	init_menubar();

	/* Prepare the windows */
	init_windows();

	/* Hack -- process all events */
	while (CheckEvents(CHECK_EVENTS_DRAIN)) /* loop */;

	/* Reset the cursor */
	{
		Cursor tempCursor;

		SetCursor(GetQDGlobalsArrow(&tempCursor));
	}


	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = NewPtr(16384);

#ifdef USE_QT_SOUND

	/* Load sound effect resources */
	load_sounds();

#endif /* USE_QT_SOUND */

	/* Note the "system" */
	ANGBAND_SYS = "mac";


	/* Initialize */
	init_stuff();

	/* Initialize */
	init_angband();

	/* Validate the contents of the main window */
	validate_main_window();

	/* Hack -- process all events */
	while (CheckEvents(CHECK_EVENTS_DRAIN)) /* loop */;


	/* We are now initialized */
	initialized = TRUE;


	/* Handle "open_when_ready" */
	handle_open_when_ready();

	/* Let the player choose a savefile or start a new game */
	if (!game_in_progress)
	{
		/* Prompt the user - You may have to change this for some variants */
		prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);

		/* Flush the prompt */
		Term_fresh();

		/* Hack -- Process Events until "new" or "open" is selected */
		while (!game_in_progress) CheckEvents(CHECK_EVENTS_WAIT);
	}

	/* Handle pending events (most notably update) and flush input */
	Term_flush();

	/*
	 * Play a game -- "new_game" is set by "new", "open" or the open document
	 * even handler as appropriate
	 */
	play_game(new_game);

	/* Quit */
	quit(NULL);

	/* Since it's an int function */
	return (0);
}

#endif /* MACINTOSH || MACH_O_CARBON */
