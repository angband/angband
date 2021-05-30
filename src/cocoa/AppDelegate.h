/**
 *\file AppDelegate.h
 *\brief Declare the application delegate used by the OS X front end.
 */

#import <Cocoa/Cocoa.h>
#import "TileSetScaling.h"

@interface AngbandAppDelegate : NSObject <NSApplicationDelegate,
					      TileSetDefaultScalingComputing,
					      TileSetScalingChanging> {
    NSMenu *_commandMenu;
    NSDictionary *_commandMenuTagMap;
}
@property (strong, nonatomic, retain) IBOutlet NSMenu *commandMenu;
@property (strong, nonatomic, retain) NSDictionary *commandMenuTagMap;
@property (strong, nonatomic) TileSetScalingPanelController *scalingPanelController;
- (IBAction)newGame:(id)sender;
- (IBAction)editFont:(id)sender;
- (IBAction)openGame:(id)sender;
- (IBAction)saveGame:(id)sender;
- (IBAction)setRefreshRate:(NSMenuItem *)sender;
- (IBAction)showTileSetScalingPanel:(id)sender;

- (void)setGraphicsMode:(NSMenuItem *)sender;
- (void)selectWindow:(id)sender;
- (void)recomputeDefaultTileMultipliersIfNecessary;
- (void)beginGame;

@end

