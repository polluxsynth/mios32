/*
 * ==========================================================================
 * Copyright 2018 Ricard Wanderlof <ricard2015@butoba.net>
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

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>

#include <math.h>

#include "proc.h"

#include <FreeRTOS.h>
#include <portmacro.h>


/////////////////////////////////////////////////////////////////////////////
// Local definitions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Local Variables
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Local Prototypes
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////
s32 PROC_Init(u32 mode)
{
}

/////////////////////////////////////////////////////////////////////////////
// Handle incoming MIDI data
/////////////////////////////////////////////////////////////////////////////
s32 PROC_MIDI_NotifyPackage(mios32_midi_port_t port, mios32_midi_package_t midi_package)
{
  switch (midi_package.type) {
  case NoteOff:
    midi_package.velocity = 0; // set velocity to 0 and fall through
  case NoteOn:
    // blabla midi_package.note
    if (midi_package.velocity == 0) { // note off
    } else {
    }

#if 0
    MIOS32_IRQ_Disable();
    MIOS32_IRQ_Enable();
#endif
    break;

  case CC:
    if (midi_package.cc_number == 120 || midi_package.cc_number >= 123) {
      // all notes off
      break;
    }
    if (midi_package.cc_number == 64) {
      // sustain pedal
      break;
    }
    // blabla midi_package.cc_number and midi_package.value
    break;
  case PitchBend:
    // blabla (midi_package.evnt2 << 7) + midi_package.evnt1 - 0x2000;
    break;
  default:
    break;
  }

  return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// Called periodically to update channel controllers
/////////////////////////////////////////////////////////////////////////////
s32 PROC_Update_1mS(void)
{
  // Do something
  return 0; // no error
}
