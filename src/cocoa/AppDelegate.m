/**
 * \file AppDelegate.m
 * \brief This is a minimal implementation of the OS X front end.
 *
 * Use this file to rebuild the .nib file with Xcode without having to pull
 * in all of the Angband source.  This is the procedure with Xcode 11.3:
 *
 * 1) Create a new Xcode project for a macOS App.
 * 2) You can set the "Product Name", "Team", "Organization Name",
 *    "Organization Identifier" as you wish.  Setting the product name to
 *    "angband" and the "Organization Identifier" to "org.rephial" will match
 *    the bundle identifier used in the full builds for Angband.  Set
 *    "Language" to "Objective-C" and "User Interface" to "XIB".  Leave
 *    "Create Document-Based Application" and "Use Core Data" off.
 *    The settings for "Include Unit Tests" and "Include UI Tests" don't
 *    matter; you can turn them off to avoid extra clutter.
 * 3) In the Angband project settings, set "Main Interface" to "MainMenu".
 *    Set the deployment target to what's used in Angband's Makefile.osx.
 *    When this was written, that was 10.9.
 * 4) Copy src/AppDelegate.h and src/cocoa/AppDelegate.m from the Angband
 *    source files to the directory in the project with main.m.  Copy
 *    src/cocoa/MainMenu.xib to the Base.lproj subdirectory of that directory.
 * 5) If you modify MainMenu.xib after copying it over, you may want to
 *    set it so that it can open in older versions of Xcode.  Select it in
 *    Xcode, and select one of the things, like "File's Owner" from it.  In
 *    the file information panel for it, there will be a section labeled
 *    "Document Editing" with an option menu for "Opens in".  Choosing one of
 *    the options other than "Latest Xcode" will close the file and save it
 *    with the appropriate flags.  Note that reopening that MainMenu.xib in
 *    Xcode will cause the version to revert to the latest Xcode.
 * 6) Use Xcode's Product->Build For->Running menu entry to build the project.
 * 7) The generated .nib file will be
 *    Contents/Resources/Base.lproj/MainMenu.nib in the product directory which
 *    is something like
 *    ~/Library/Developer/Xcode/DerivedData/<product_name>-<some_string>/Build/Products/Debug/<product_name>.app
 *    You can use that to replace the src/cocoa/en.lproj/MainMenu.nib in the
 *    Angband source files (in older versions of Angband, MainMenu.nib is a
 *    directory; you'll have to remove it and replace it with the flat file
 *    that is generated by the above procedure and adjust the installation
 *    rules for it in Makefile.osx).
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#import "AppDelegate.h"

@implementation AngbandAppDelegate

@synthesize commandMenu=_commandMenu;
@synthesize commandMenuTagMap=_comandMenuTagMap;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (IBAction)newGame:(id)sender {
}

- (IBAction)editFont:(id)sender {
}

- (IBAction)openGame:(id)sender {
}

- (IBAction)saveGame:(id)sender {
}

- (IBAction)setRefreshRate:(NSMenuItem *)sender {
}

- (void)setGraphicsMode:(NSMenuItem *)sender {
}

- (void)selectWindow:(id)sender {
}

@end
