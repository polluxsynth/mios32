// $Id: synth.h 2177 2015-05-24 19:04:58Z tk $
/*
 * ==========================================================================
 * Copyright 2014 Mark Owen
 * http://www.quinapalus.com
 * E-mail: goom@quinapalus.com
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/> or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA  02110-1301, USA.
 * ==========================================================================
 */

#ifndef _SYNTH_H
#define _SYNTH_H


/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

#define NPOLY 16 // polyphony: must be a power of 2
#define NCHAN 16 // number of MIDI channels/patches: must be a power of 2

#define NPARAM 24 // Number of parameters

/////////////////////////////////////////////////////////////////////////////
// Global Types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

s32 SYNTH_Init(u32 mode);

s32 SYNTH_MIDI_NotifyPackage(mios32_midi_port_t port, mios32_midi_package_t midi_package);
s32 SYNTH_Update_1mS(void);
u8 SYNTH_FetchParamValue(u8 chan, u8 param_id);
s32 SYNTH_StoreParamValue(u8 chan, u8 param_id, u8 value);

/////////////////////////////////////////////////////////////////////////////
// Export global variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _SYNTH_H */
