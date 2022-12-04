/**
 *\file AppDelegate.h
 *\brief Declare the application delegate used by the OS X front end.
 */

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

- (void)setGraphicsMode:(NSMenuItem *)sender;
- (void)setTileFraction:(NSMenuItem *)sender;
- (void)selectWindow:(id)sender;
- (void)beginGame;

@end

