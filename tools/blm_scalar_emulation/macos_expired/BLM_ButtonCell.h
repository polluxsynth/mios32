// $Id: BLM_ButtonCell.h 1026 2010-05-24 21:32:36Z tk $
//
//  BLM_ButtonCell.h
//  blm_scalar_emulation
//
//  Created by Thorsten Klose on 19.11.09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface BLM_ButtonCell : NSButtonCell {
	NSColor *buttonColor;
	NSInteger LED_State;
}

- (NSColor *)buttonColor;
- (void)setButtonColor:(NSColor *)color;

- (NSInteger)LED_State;
- (void)setLED_State:(NSInteger)state;

@end
