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
 * Declare a NSWindowController subclass to load the panel from the nib file.
 */
@interface TileSetScalingPanelController : NSWindowController<NSTextFieldDelegate>

/**
 * Is the maximum allowed scale factor for either the horizontal or vertical
 * direction.
 */
@property(class, readonly) NSInteger scalingMaximum;

@end
