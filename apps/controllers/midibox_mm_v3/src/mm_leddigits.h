// $Id: mm_leddigits.h 1491 2012-07-29 20:41:15Z tk $
/*
 * Header file for LED Digits Handler
 *
 * ==========================================================================
 *
 *  Copyright (C) 2009 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _MM_LEDDIGITS_H
#define _MM_LEDDIGITS_H

/////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////

extern u8 mm_leddigits_status[2];

	
/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

extern s32 MM_LEDDIGITS_Init(u32 mode);
extern s32 MM_LEDDIGITS_Set(u8 number, u8 pattern);

#endif /* _MM_LEDDIGITS_H */
