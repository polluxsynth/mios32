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

struct keyinfo {
  unsigned int note:7; // original note number
  unsigned int chn:4; // original channel number
  unsigned int valid:1;
};

#define SPLIT_MAX 127

/////////////////////////////////////////////////////////////////////////////
// Local Variables
/////////////////////////////////////////////////////////////////////////////

enum proc_sm State;
int Trans_left = 0, Trans_right = 0;
int Splitpoint = SPLIT_MAX;
struct keyinfo keystatus[128]; // TODO: This should really be per port

/////////////////////////////////////////////////////////////////////////////
// Local Prototypes
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////
s32 PROC_Init(u32 mode)
{
  int i;

  for (i = 0; i < 128; i++) {
    keystatus[i].valid = 0;
  }
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
  } else if (number >= 2 && number <= 4) {
    Trans_left = (number - 3) * 12;
    MIOS32_MIDI_SendDebugMessage("SM LEFT_XPOSE %d\n", Trans_left);
    State = IDLE;
  } else if (number >= 5 && number <= 7) {
    Trans_right = (number - 6) * 12;
    MIOS32_MIDI_SendDebugMessage("SM RIGHT_XPOSE %d\n", Trans_right);
    State = IDLE;
  } else if (State == ARMED) {
    if (number == 1) {
      State = SPLIT_SET;
      MIOS32_MIDI_SendDebugMessage("SM SPLIT_SET\n");
    } else {
      MIOS32_MIDI_SendDebugMessage("Unkown combo: ->SM IDLE\n");
      State = IDLE;
    }
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
#if 0 // original split algo
    if (midi.note > Splitpoint) midi.chn++;
#endif
    if (keystatus[midi.note].valid) {
      // Grab original note and channel number from note on time
      midi.note = keystatus[midi.note].note;
      midi.chn = keystatus[midi.chn].chn;
      // keystatus[midi.note].valid = 0 // TODO: ?needed
    }
    // Else we don't have a mapping and just echo it through.

    MIOS32_MIDI_SendNoteOff(port, midi.chn, midi.note, midi.velocity);
    break;

  case NoteOn:
    if (State == SPLIT_SET) {
      MIOS32_MIDI_SendDebugMessage("Set split to %d\n", midi.note);
      Splitpoint = midi.note;
      State = IDLE;
      break; // Don't echo note
    }

    if (midi.velocity) { // true note on
      int *transp;

      // Do the split
      if (midi.note > Splitpoint) {
        midi.chn++;
        transp = &Trans_right;
      } else {
        transp = &Trans_left;
      }

      // Do transpose
      u8 orig_note = midi.note; // save it
      int new_note = (int) midi.note + *transp; // try it

      if (new_note <= 127 && new_note >= 0)
        midi.note += *transp;;

      // Save status for subsequnt note off
      keystatus[orig_note].note = midi.note;
      keystatus[orig_note].chn = midi.chn;
      keystatus[orig_note].valid = 1;
    } else if (keystatus[midi.note].valid) {
      // It's really a note off so treat it that way.
      // Grab original note and channel number from note on time
      // keystatus[midi.note].valid = 0 // TODO: ?needed
      midi.chn = keystatus[midi.note].chn;
      midi.note = keystatus[midi.note].note; // Needs to be last
    }
    // Else we don't have a mapping and just echo it through.

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
