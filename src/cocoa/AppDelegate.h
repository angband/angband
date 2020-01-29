//
//  AppDelegate.h
//
//  This is a stub set of declarations to be used to rebuild the .nib
//  file with Xcode.  See AngbandAppDelegate.m for more details.
//
//  This work is free software; you can redistribute it and/or modify it
//  under the terms of either:
//
//  a) the GNU General Public License as published by the Free Software
//     Foundation, version 2, or
//
//  b) the "Angband licence":
//     This software may be copied and distributed for educational, research,
//     and not for profit purposes provided that this copyright and statement
//     are included in all such copies.  Other copyrights may also apply.
//

#import <Cocoa/Cocoa.h>

@interface AngbandAppDelegate : NSObject <NSApplicationDelegate> {
    NSMenu *_commandMenu;
    NSDictionary *_commandMenuTagMap;
}
@property (strong, nonatomic, retain) IBOutlet NSMenu *commandMenu;
@property (strong, nonatomic, retain) NSDictionary *commandMenuTagMap;
- (IBAction)newGame:(id)sender;
- (IBAction)editFont:(id)sender;
- (IBAction)openGame:(id)sender;
- (IBAction)saveGame:(id)sender;
- (IBAction)setRefreshRate:(NSMenuItem *)sender;


@end

