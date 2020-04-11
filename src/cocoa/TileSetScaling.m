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
- (void)computeDefaultScalingHorizontal:(NSInteger *)pHor
			       vertical:(NSInteger *)pVer
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
    *pHor = newh;
    *pVer = newv;
}

/**
 * Is an internal helper function to update the editable states for the
 * controls based on whether or not using the default scaling.
 *
 * Assumes that the window has been loaded.
 */
- (void)updateEditableStates:(BOOL)usesDefault
{
    if (usesDefault) {
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
    self.horizontalScalingField.delegate = self;
    [self.horizontalScalingField.formatter setAllowsFloats:NO];
    [self.horizontalScalingField.formatter setPartialStringValidationEnabled:YES];
    self.horizontalScalingField.toolTip =
	[NSString stringWithFormat:@"integer between 1 and %ld",
		  (long) TileSetScalingPanelController.scalingMaximum];
    [self.verticalScalingField.formatter setAllowsFloats:NO];
    self.verticalScalingField.delegate = self;
    [self.verticalScalingField.formatter setPartialStringValidationEnabled:YES];
    self.verticalScalingField.toolTip =
	[NSString stringWithFormat:@"integer between 1 and %ld",
		  (long) TileSetScalingPanelController.scalingMaximum];
    self.defaultScalingToggle.state =
	(self.usesDefaultScaling) ? NSOnState : NSOffState;
    [self.horizontalScalingField setIntegerValue:self.horizontalScaling];
    [self.verticalScalingField setIntegerValue:self.verticalScaling];
    [self updateEditableStates:self.usesDefaultScaling];
}

/**
 * Implement the control: textShouldEndEditing: method of the
 * NSTextFieldDelegate (via NSControlTextEditingDelegate) protocol to coerce
 * the value of one of the text field to the accepted range when the insertion
 * point leaves the field.
 */
- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
    if (control.integerValue < 1) {
	control.integerValue = 1;
    } else if (control.integerValue >
	       TileSetScalingPanelController.scalingMaximum) {
	control.integerValue = TileSetScalingPanelController.scalingMaximum;
    }
    return YES;
}

+ (NSInteger)scalingMaximum {
    return 16;
}

+ (NSString *)changeNotificationName {
    return @"TileSetScalingChangeNotification";
}

- (IBAction)respondToDefaultScalingToggle:(id)sender {
    if (self.defaultScalingToggle.state == NSOnState) {
	NSInteger newh, newv;

	[self computeDefaultScalingHorizontal:&newh vertical:&newv];
	[self.horizontalScalingField setIntegerValue:newh];
	[self.verticalScalingField setIntegerValue:newv];
	[self updateEditableStates:YES];
    } else {
	[self updateEditableStates:NO];
    }
}

- (IBAction)respondToHorizontalScaleStep:(id)sender {
    NSInteger change = self.horizontalScalingStepper.integerValue;
    NSInteger result = self.horizontalScalingField.integerValue + change;

    [self.horizontalScalingStepper setIntegerValue:0];

    if (result < 1) {
	result = 1;
    } else if (result > TileSetScalingPanelController.scalingMaximum) {
	result = TileSetScalingPanelController.scalingMaximum;
    }
    self->_horizontalScaling = result;
    [self.horizontalScalingField setIntegerValue:result];
}

- (IBAction)respondToVerticalScaleStep:(id)sender {
    NSInteger change = self.verticalScalingStepper.integerValue;
    NSInteger result = self.verticalScalingField.integerValue + change;

    [self.verticalScalingStepper setIntegerValue:0];

    if (result < 1) {
	result = 1;
    } else if (result > TileSetScalingPanelController.scalingMaximum) {
	result = TileSetScalingPanelController.scalingMaximum;
    }
    self->_verticalScaling = result;
    [self.verticalScalingField setIntegerValue:result];
}

- (IBAction)respondToApply:(id)sender {
    self->_usesDefaultScaling = self.defaultScalingToggle.state == NSOnState;

    /*
     * Coerce the value since editing and clicking directly on a dialog button
     * doesn't invoke the textShouldEndEditing method.
     */
    self->_horizontalScaling = self.horizontalScalingField.integerValue;
    if (self.horizontalScaling < 1) {
	self->_horizontalScaling = 1;
	self.horizontalScalingField.integerValue = self.horizontalScaling;
    } else if (self.horizontalScaling >
	       TileSetScalingPanelController.scalingMaximum) {
	self->_horizontalScaling =
	    TileSetScalingPanelController.scalingMaximum;
	self.horizontalScalingField.integerValue = self.horizontalScaling;
    }

    self->_verticalScaling = self.verticalScalingField.integerValue;
    if (self.verticalScaling < 1) {
	self->_verticalScaling = 1;
	self.verticalScalingField.integerValue = self.verticalScaling;
    } else if (self.verticalScaling >
	       TileSetScalingPanelController.scalingMaximum) {
	self->_verticalScaling = TileSetScalingPanelController.scalingMaximum;
	self.verticalScalingField.integerValue = self.verticalScaling;
    }

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
    }
    self.usesDefaultScaling = NO;
}

- (void)setUsesDefaultScaling:(BOOL)value {
    self->_usesDefaultScaling = value;
    if (self.usesDefaultScaling) {
	NSInteger newh, newv;

	[self computeDefaultScalingHorizontal:&newh vertical:&newv];
	self->_horizontalScaling = newh;
	self->_verticalScaling = newv;
	if (self.windowLoaded) {
	    [self.horizontalScalingField setIntegerValue:newh];
	    [self.verticalScalingField setIntegerValue:newv];
	}
    }
    if (self.windowLoaded) {
	self.defaultScalingToggle.state = (value) ? NSOnState : NSOffState;
	[self updateEditableStates:value];
    }
}

- (void)setDefaultScalingComputer:(id<TileSetDefaultScalingComputing>)delegate {
    self->_defaultScalingComputer = delegate;
    if (self.usesDefaultScaling) {
	NSInteger newh, newv;

	[self computeDefaultScalingHorizontal:&newh vertical:&newv];
	self->_horizontalScaling = newh;
	self->_verticalScaling = newv;
	if (self.windowLoaded) {
	    [self.horizontalScalingField setIntegerValue:newh];
	    [self.verticalScalingField setIntegerValue:newv];
	}
    }
}

@end
