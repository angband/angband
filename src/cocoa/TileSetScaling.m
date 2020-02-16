/**
 *\file TileSetScaling.m
 *\brief Define tile set scaling panel used by OS X front end.
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

#import "TileSetScaling.h"

@implementation TileSetScalingPanelController

+ (NSInteger)scalingMaximum {
    return 16;
}

@end
