/**
 *\file TileSetScaling.h
 *\brief Declare interface to tile set scaling panel used by OS X front end.
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

#import <Cocoa/Cocoa.h>

/**
 * Declare a protocol to encapsulate computing the default scaling.
 */
@protocol TileSetDefaultScalingComputing
- (void)computeDefaultTileSetScaling:(NSInteger *)pHoriz vertical:(NSInteger *)pVert;
@end

/**
 * Declare a protocol to encapsulate responding to a user-requested change in
 * the scaling.
 */
@protocol TileSetScalingChanging
- (void) changeTileSetScaling:(NSInteger)h vertical:(NSInteger)v isDefault:(BOOL)flag;
@end

/**
 * Declare a NSWindowController subclass to load the panel from the nib file.
 */
@interface TileSetScalingPanelController : NSWindowController

/**
 * Is the maximum allowed scale factor for either the horizontal or vertical
 * direction.
 */
@property(class, readonly) NSInteger scalingMaximum;

/**
 * Is the name used for the notifications broadcast when the user accepts a
 * change to the tile set scaling.  The notification will include the
 * controller object for the panel and, for the user info, a NSDictionary with
 * these keys: @"horizontalScaling" whose value will be a long integer as a
 * NSNumber, @"verticalScaling" whose value will be a long integer as a
 * NSNumber, and @"usesDefaultScaling" whose value will be a boolean as a
 * NSNumber.
 */
@property(class, readonly) NSString *changeNotificationName;

/**
 * Is the value for the horizontal scaling currently displayed in the panel.
 * Setting this property implicitly sets usesDefaultScaling to NO.  Setting
 * this property to a value less than one or greater than
 * TileSetScalingPanelController.scalingMaximum will throw an exception.
 */
@property(nonatomic) NSInteger horizontalScaling;

/**
 * Is the value for the vertical scaling currently displayed in the panel.
 * Setting this property implicitly sets usesDefaultScaling to NO.  Setting
 * this property to a value less than one or greater than
 * TileSetScalingPanelController.scalingMaximum will throw an exception.
 */
@property(nonatomic) NSInteger verticalScaling;

/**
 * If YES, the scaling is recomputed in response to changes in the font and
 * selected tile set.  Otherwise, the scaling remains at the values selected
 * by the user until the user changes them.  Setting this property to YES
 * may change the values for horizontalScaling and verticalScaling.
 */
@property(nonatomic) BOOL usesDefaultScaling;

/**
 * Is the delegate that computes what the default scaling factors are.
 * If that delegate returns scaling factors that are not positive and less
 * than or equal to scalingMaximum, an exception will be raised.
 * Changing this to a value other than nil while usesDefaultTileSetScaling is
 * true will immediately send a computeDefaultTileSetScaling message to the
 * new value.
 */
@property(weak,nonatomic) id<TileSetDefaultScalingComputing> defaultScalingComputer;

/**
 * Is the delegate that responds when the user accepts a change to scaling
 * parameters.
 */
@property(weak,nonatomic) id<TileSetScalingChanging> scalingChangeHandler;

/* These are implementation details. */
@property (strong) IBOutlet NSPanel *window;
@property (strong) IBOutlet NSButton *defaultScalingToggle;
@property (strong) IBOutlet NSTextField *horizontalScalingField;
@property (strong) IBOutlet NSStepper *horizontalScalingStepper;
@property (strong) IBOutlet NSTextField *verticalScalingField;
@property (strong) IBOutlet NSStepper *verticalScalingStepper;

@end
