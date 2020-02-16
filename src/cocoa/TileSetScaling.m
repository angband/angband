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

/*
 * Don't use the implicit @synthesize since this property is declared in
 * a superclass, NSWindowController.
 */
@dynamic window;

/**
 * Is an internal helper function to invoke the delegate that computes the
 * default scaling.
 */
- (void)computeDefaultScaling
{
    NSInteger newh, newv;

    if (self.defaultScalingComputer == nil) {
	newh = 1;
	newv = 1;
    } else {
	[self.defaultScalingComputer
	     computeDefaultTileSetScaling:&newh vertical:&newv];
	if (newh < 1 ||
	    newh > TileSetScalingPanelController.scalingMaximum ||
	    newv < 1 ||
	    newv > TileSetScalingPanelController.scalingMaximum) {
	    NSException *exc = [NSException
				   exceptionWithName:@"InvalidTileSetScaling"
				   reason:@"defaultScalingComputer returned an invalid value"
				   userInfo:nil];

	    @throw exc;
	}
    }
    self->_horizontalScaling = newh;
    self->_verticalScaling = newv;
    if (self.windowLoaded) {
	[self.horizontalScalingField setIntegerValue:newh];
	[self.horizontalScalingStepper setIntegerValue:newh];
	[self.verticalScalingField setIntegerValue:newv];
	[self.verticalScalingStepper setIntegerValue:newv];
    }
}

/**
 * Is an internal helper function to update the editable states for the
 * controls based on whether or not using the default scaling.
 *
 * Assumes that the window has been loaded.
 */
- (void)updateEditableStates
{
    if (self.usesDefaultScaling) {
	self.horizontalScalingField.enabled = NO;
	self.horizontalScalingStepper.enabled = NO;
	self.verticalScalingField.enabled = NO;
	self.verticalScalingStepper.enabled = NO;
    } else {
	self.horizontalScalingField.enabled = YES;
	self.horizontalScalingStepper.enabled = YES;
	self.verticalScalingField.enabled = YES;
	self.verticalScalingStepper.enabled = YES;
    }
}

/**
 * Override the NSWindowController property getter to get appropriate nib file.
 */
-(NSString*)windowNibName {
    return @"TileSetScaling";
}

/**
 * Override the NSWindowController function to set formatter properties and
 * current values for the controls.
 */
-(void)windowDidLoad {
    [super windowDidLoad];
    self.window.floatingPanel = YES;
    self.window.becomesKeyOnlyIfNeeded = YES;
    [self.window center];
    self.defaultScalingToggle.toolTip = @"turn off to edit scaling";
    [self.horizontalScalingField.formatter
	 setMinimum:[NSNumber numberWithInt:1]];
    [self.horizontalScalingField.formatter
	 setMaximum:[NSNumber numberWithLong:TileSetScalingPanelController.scalingMaximum]];
    [self.horizontalScalingField.formatter setAllowsFloats:NO];
    [self.horizontalScalingField.formatter setPartialStringValidationEnabled:YES];
    self.horizontalScalingField.toolTip =
	[NSString stringWithFormat:@"integer between 1 and %ld",
		  (long) TileSetScalingPanelController.scalingMaximum];
    self.horizontalScalingStepper.minValue = 1;
    self.horizontalScalingStepper.maxValue =
	TileSetScalingPanelController.scalingMaximum;
    self.horizontalScalingStepper.increment = 1;
    [self.verticalScalingField.formatter
	 setMinimum:[NSNumber numberWithInt:1]];
    [self.verticalScalingField.formatter
	 setMaximum:[NSNumber numberWithLong:TileSetScalingPanelController.scalingMaximum]];
    [self.verticalScalingField.formatter setAllowsFloats:NO];
    [self.verticalScalingField.formatter setPartialStringValidationEnabled:YES];
    self.verticalScalingField.toolTip =
	[NSString stringWithFormat:@"integer between 1 and %ld",
		  (long) TileSetScalingPanelController.scalingMaximum];
    self.verticalScalingStepper.minValue = 1;
    self.verticalScalingStepper.maxValue =
	TileSetScalingPanelController.scalingMaximum;
    self.verticalScalingStepper.increment = 1;
    self.defaultScalingToggle.state =
	(self.usesDefaultScaling) ? NSOnState : NSOffState;
    [self.horizontalScalingField setIntegerValue:self.horizontalScaling];
    [self.horizontalScalingStepper setIntegerValue:self.horizontalScaling];
    [self.verticalScalingField setIntegerValue:self.verticalScaling];
    [self.verticalScalingStepper setIntegerValue:self.verticalScaling];
    [self updateEditableStates];
}

+ (NSInteger)scalingMaximum {
    return 16;
}

+ (NSString *)changeNotificationName {
    return @"TileSetScalingChangeNotification";
}

- (IBAction)respondToDefaultScalingToggle:(id)sender {
    self.usesDefaultScaling = self.defaultScalingToggle.state == NSOnState;
}

- (IBAction)respondToHorizontalScaleEdit:(id)sender {
    self->_horizontalScaling = self.horizontalScalingField.integerValue;
    [self.horizontalScalingStepper setIntegerValue:self.horizontalScaling];
}

- (IBAction)respondToHorizontalScaleStep:(id)sender {
    self->_horizontalScaling = self.horizontalScalingStepper.integerValue;
    [self.horizontalScalingField setIntegerValue:self.horizontalScaling];
}

- (IBAction)respondToVerticalScaleEdit:(id)sender {
    self->_verticalScaling = self.verticalScalingField.integerValue;
    [self.verticalScalingStepper setIntegerValue:self.verticalScaling];
}

- (IBAction)respondToVerticalScaleStep:(id)sender {
    self->_verticalScaling = self.verticalScalingStepper.integerValue;
    [self.verticalScalingField setIntegerValue:self.verticalScaling];
}

- (IBAction)respondToApply:(id)sender {
    if (self.scalingChangeHandler != nil) {
	[self.scalingChangeHandler
	     changeTileSetScaling:self.horizontalScaling
	     vertical:self.verticalScaling
	     isDefault:self.usesDefaultScaling];
    }
    NSDictionary *extraInfo =
	[NSDictionary dictionaryWithObjectsAndKeys:
			  [NSNumber numberWithLong:self.horizontalScaling],
		      @"horizontalScaling",
		      [NSNumber numberWithLong:self.verticalScaling],
		      @"verticalScaling",
		      [NSNumber numberWithBool:self.usesDefaultScaling],
		      @"usesDefaultScaling",
		      nil];
    [[NSNotificationCenter defaultCenter]
	postNotificationName:TileSetScalingPanelController.changeNotificationName
	object:self
	userInfo:extraInfo];
}

- (IBAction)respondToCancel:(id)sender {
    [self.window close];
}

- (IBAction)respondToOK:(id)sender {
    [self respondToApply:sender];
    [self.window close];
}

- (void)setHorizontalScaling:(NSInteger)value {
    if (value < 1 || value > TileSetScalingPanelController.scalingMaximum) {
	NSException *exc = [NSException
			       exceptionWithName:@"InvalidTileSetScaling"
			       reason:@"setHorizontalScaling called with invalid value"
			       userInfo:nil];

	@throw exc;
    }
    self->_horizontalScaling = value;
    if (self.windowLoaded) {
	[self.horizontalScalingField setIntegerValue:value];
	[self.horizontalScalingStepper setIntegerValue:value];
    }
    self.usesDefaultScaling = NO;
}

- (void)setVerticalScaling:(NSInteger)value {
    if (value < 1 || value > TileSetScalingPanelController.scalingMaximum) {
	NSException *exc = [NSException
			       exceptionWithName:@"InvalidTileSetScaling"
			       reason:@"setVerticalScaling called with invalid value"
			       userInfo:nil];

	@throw exc;
    }
    self->_verticalScaling = value;
    if (self.windowLoaded) {
	[self.verticalScalingField setIntegerValue:value];
	[self.verticalScalingStepper setIntegerValue:value];
    }
    self.usesDefaultScaling = NO;
}

- (void)setUsesDefaultScaling:(BOOL)value {
    self->_usesDefaultScaling = value;
    if (self.usesDefaultScaling) {
	[self computeDefaultScaling];
    }
    if (self.windowLoaded) {
	self.defaultScalingToggle.state = (value) ? NSOnState : NSOffState;
	[self updateEditableStates];
    }
}

- (void)setDefaultScalingComputer:(id<TileSetDefaultScalingComputing>)delegate {
    self->_defaultScalingComputer = delegate;
    if (self.usesDefaultScaling) {
	[self computeDefaultScaling];
    }
}

@end
