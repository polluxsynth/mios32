// $Id: mios32_board.c 2035 2014-08-18 20:16:47Z tk $
//! \defgroup P6_BOARD
//!
//! Development Board specific functions for Audiothingies P6
//!
//! \{
/* ==========================================================================
 *
 *  Copyright (C) 2017 Ricard Wanderlof (polluxsynth@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _p6_board_h_
#define _p6_board_h_
/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>

/////////////////////////////////////////////////////////////////////////////
// Global variables
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// API
/////////////////////////////////////////////////////////////////////////////

s32 MIOS32_BOARD_P6_ScanInit(void);
s32 MIOS32_BOARD_P6_ScanStart(void);
s32 MIOS32_BOARD_P6_DIN_Handler(void *_callback);

#endif // _p6_board_h_

//! \}
