// $Id: mbcv_rgb.h 2439 2016-11-05 01:19:21Z tk $
/*
 * Header file for MIDIbox CV V2 RGB LED functions
 *
 * ==========================================================================
 *
 *  Copyright (C) 2016 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _MBCV_RGB_H
#define _MBCV_RGB_H

/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

// number of RGB LEDs
#define MBCV_RGB_LED_NUM WS2812_NUM_LEDS // defined in mios32_config.h


/////////////////////////////////////////////////////////////////////////////
// Global Types
/////////////////////////////////////////////////////////////////////////////

typedef enum {
  MBCV_RGB_MODE_DISABLED = 0,
  MBCV_RGB_MODE_CHANNEL_HUE = 1,
} mbcv_rgb_mode_t;


/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

extern s32 MBCV_RGB_Init(u32 mode);

extern s32 MBCV_RGB_UpdateLed(u32 led);

extern s32 MBCV_RGB_UpdateAllLeds(void);


/////////////////////////////////////////////////////////////////////////////
// Export global variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _MBCV_RGB_H */
