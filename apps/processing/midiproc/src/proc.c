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

enum proc_sm { IDLE, ARMED, SPLIT_SET };

#define SPLIT_MAX 127

/////////////////////////////////////////////////////////////////////////////
// Local Variables
/////////////////////////////////////////////////////////////////////////////

enum proc_sm State;
int Splitpoint = SPLIT_MAX;

/////////////////////////////////////////////////////////////////////////////
// Local Prototypes
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////
s32 PROC_Init(u32 mode)
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////
s32 PROC_sm(u8 program_change)
{
  u8 number = program_change & 7;
//  u8 bank = (program_change >> 3) & 7;
//  u8 group = program_change >> 6;

  if (number == 0) {
    State = ARMED;
    MIOS32_MIDI_SendDebugMessage("SM ARMED\n");
  } else if (State == ARMED && number == 1) {
    State = SPLIT_SET;
    MIOS32_MIDI_SendDebugMessage("SM SPLIT_SET\n");
  } else {
    MIOS32_MIDI_SendDebugMessage("Unkown combo: ->SM IDLE\n");
    State = IDLE;
  }

  return State;
}

/////////////////////////////////////////////////////////////////////////////
// Handle incoming MIDI data
/////////////////////////////////////////////////////////////////////////////
s32 PROC_MIDI_NotifyPackage(mios32_midi_port_t port, mios32_midi_package_t midi)
{
  uint bend_val;

MIOS32_MIDI_SendDebugMessage("Received midi from port %d\n", port);

  switch (midi.type) {
  case NoteOff:
    if (midi.note > Splitpoint) midi.chn++;
    MIOS32_MIDI_SendNoteOn(port, midi.chn, midi.note, midi.velocity);
    break;

  case NoteOn:
    if (State == SPLIT_SET) {
      MIOS32_MIDI_SendDebugMessage("Set split to %d\n", midi.note);
      Splitpoint = midi.note;
      State = IDLE;
      break; // Don't echo note
    }

    if (midi.note > Splitpoint) midi.chn++;
    MIOS32_MIDI_SendDebugMessage("Note on: port %d, chan %d, note %d, vel %d\n",
                                 port, midi.chn, midi.note, midi.velocity);
    MIOS32_MIDI_SendNoteOn(port, midi.chn, midi.note, midi.velocity);

    break;

  case CC:
    MIOS32_MIDI_SendCC(port, midi.chn, midi.cc_number, midi.value);
    if (Splitpoint < SPLIT_MAX)
      MIOS32_MIDI_SendCC(port, midi.chn + 1, midi.cc_number, midi.value);
#if 0
    if (midi.cc_number == 120 || midi.cc_number >= 123) {
      // all notes off
      break;
    }
    if (midi.cc_number == 64) {
      // sustain pedal
      break;
    }
#endif
    break;

  case PitchBend:
    bend_val = (midi.evnt2 << 7) + midi.evnt1; //  - 0x2000;
    MIOS32_MIDI_SendPitchBend(port, midi.chn, bend_val);
    if (Splitpoint < SPLIT_MAX)
      MIOS32_MIDI_SendPitchBend(port, midi.chn + 1, bend_val);
    break;

  case ProgramChange:
    MIOS32_MIDI_SendDebugMessage("Program change: port %d, chan %d, value %d\n",
                                 port, midi.chn, midi.program_change);
    PROC_sm(midi.program_change);
    // And don't send it on.
    break;

  case Aftertouch:
    MIOS32_MIDI_SendAftertouch(port, midi.chn, midi.evnt1);
    if (Splitpoint < SPLIT_MAX)
      MIOS32_MIDI_SendAftertouch(port, midi.chn + 1, midi.evnt1);
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
