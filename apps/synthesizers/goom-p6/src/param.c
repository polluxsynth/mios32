/*
 * Parameter handler.
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>

#include <math.h>

#include "synth.h"
#include "param.h"

#include <FreeRTOS.h>

#define PARAM_SIZE 4 // # chars each parameter occupies on screen
#define PARAMS_PER_PAGE 6

#define NPAGES 7

#define EEPROM_BLOCKSIZE 64 // size of each patch in EEPROM

/////////////////////////////////////////////////////////////////////////////
// Type and variable definitions
/////////////////////////////////////////////////////////////////////////////

// P6 hardware definitions

// button bits
enum p6_buttons { B_NONE = 0, B_HOME = 1, B_OSC = 2, B_FILT = 4, B_ENV = 8,
                  B_LFO = 16, B_MATRIX = 32, B_ARP = 64, B_GLOBALS = 128,
                  B_LOAD = 256};

// LED bits
enum p6_leds { L_NONE = 0, L_HOME = 1, L_OSC = 2, L_FILT = 4, L_ENV = 8,
               L_LFO = 16, L_MATRIX = 32, L_ARP = 64, L_GLOBALS = 128,
               L_LOAD = 256};

// Parameter display. < 0 are numerical, >= 0 are discrete string values
//

enum param_disp { UNIPOL = -2, BIPOL = -1,
                  OSC_MODE_D, OSC_COMBN_D };

typedef char const *display_set_t[];

// Set of values for each discrete parameter type

static const display_set_t osc_mode_d = { "key", "hi ", "lo " };
static const display_set_t osc_combn_d = { "mix", "FM ", "FM+" };

// Order is same as param_disp, modulo negative values
static const display_set_t const *display_sets[] = 
  { &osc_mode_d, &osc_combn_d };

// Parameter domains: who is the parameter for ?
//

enum param_domain { D_NONE, SYNTH, CHAN, PGM };

// Central structure: definition of parameter

struct param_def {
  u8 param_id; // can be enum param_id or any other set of id's.
  enum param_domain domain;
  enum param_disp display_type;
  u8 min;
  u8 max;
  const char *name;
};

typedef struct param_def pd_t; // makes for shorter lines

struct page {
  enum p6_leds leds;
  const pd_t *params[PARAMS_PER_PAGE];
};

// Parameter management - fetch and store
//

struct param_manager {
  int (*fetch)(const pd_t *param);
  s32 (*store)(const pd_t *param, u8 value);
};

static int PARAM_FetchSynthParamValue(const pd_t *param);
static s32 PARAM_StoreSynthParamValue(const pd_t *param, u8 value);

static int PARAM_FetchChanNum(const pd_t *param);
static s32 PARAM_StoreChanNum(const pd_t *param, u8 value);

static int PARAM_FetchPgmNum(const pd_t *param);
static s32 PARAM_StorePgmNum(const pd_t *param, u8 value);

// Order is same as enum param_domain
static const struct param_manager managers[] =
{
  { NULL, NULL },
  { PARAM_FetchSynthParamValue, PARAM_StoreSynthParamValue },
  { PARAM_FetchChanNum, PARAM_StoreChanNum },
  { PARAM_FetchPgmNum, PARAM_StorePgmNum },
};

// Synth parameter id's (move to synth.h ?)
// correspond to position in ctrl[current_chan][] vector

enum synth_param_id {
  OSC1_DUTY, OSC1_SLOPE, OSC1_COARSE, OSC1_FINE,
  OSC1_ATK, OSC1_DEC, OSC1_LVL, OSC1_MODE,
  F_ATK, F_DEC, F_SUS, F_REL, A_ATK, A_DEC, A_SUS, A_REL,
  OSC0_DUTY, OSC0_SLOPE, OSC_COMBN, FEG_AMT, F_CUTOFF, F_Q, VOL, PAN };

// Synth parameters

static const pd_t osc1_duty = { OSC1_DUTY, SYNTH, UNIPOL, 0, 127, "dut" };
static const pd_t osc1_slope = { OSC1_SLOPE, SYNTH, UNIPOL, 0, 127, "slp" };
static const pd_t osc1_coarse = { OSC1_COARSE, SYNTH, UNIPOL, 0, 127, "det" };
static const pd_t osc1_fine = { OSC1_FINE, SYNTH, UNIPOL, 0, 127, "fin" };
static const pd_t osc1_atk = { OSC1_ATK, SYNTH, UNIPOL, 0, 127, "atk" };
static const pd_t osc1_dec = { OSC1_DEC, SYNTH, UNIPOL, 0, 127, "dcy" };
static const pd_t osc1_lvl =  { OSC1_LVL, SYNTH, UNIPOL, 0, 127, "lvl" };
static const pd_t osc1_mode = { OSC1_MODE, SYNTH, OSC_MODE_D, 0, 2, "OSC1" };
static const pd_t osc1_dummy = { 0, D_NONE, 0, 0, 0, "OSC1" };

static const pd_t feg_dummy = { 0, D_NONE, 0, 0, 0, "FENV" };
static const pd_t f_atk = { F_ATK, SYNTH, UNIPOL, 0, 127, "atk" };
static const pd_t f_dec = { F_DEC, SYNTH, UNIPOL, 0, 127, "dcy" };
static const pd_t f_sus = { F_SUS, SYNTH, UNIPOL, 0, 127, "sus" };
static const pd_t f_rel = { F_REL, SYNTH, UNIPOL, 0, 127, "rel" };

static const pd_t aeg_dummy = { 0, D_NONE, 0, 0, 0, "AENV" };
static const pd_t a_atk = { A_ATK, SYNTH, UNIPOL, 0, 127, "atk" };
static const pd_t a_dec = { A_DEC, SYNTH, UNIPOL, 0, 127, "dcy" };
static const pd_t a_sus = { A_SUS, SYNTH, UNIPOL, 0, 127, "sus" };
static const pd_t a_rel = { A_REL, SYNTH, UNIPOL, 0, 127, "rel" };

static const pd_t osc0_duty = { OSC0_DUTY, SYNTH, UNIPOL, 0, 127, "dut" };
static const pd_t osc0_slope = { OSC0_SLOPE, SYNTH, UNIPOL, 0, 127, "slp" };
static const pd_t osc0_dummy = { 0, D_NONE, 0, 0, 0, "OSC0" };
static const pd_t osc_combn = { OSC_COMBN, SYNTH, OSC_COMBN_D, 0, 2, "cmb" };

static const pd_t f_dummy = { 0, D_NONE, 0, 0, 0, "FILT" };
static const pd_t feg_amt = { FEG_AMT, SYNTH, BIPOL, 0, 127, "env" };
static const pd_t f_cutoff = { F_CUTOFF, SYNTH, UNIPOL, 0, 127, "cut" };
static const pd_t f_q = { F_Q, SYNTH, UNIPOL, 0, 127, "res" };
static const pd_t vol = { VOL, SYNTH, UNIPOL, 0, 127, "vol" };
static const pd_t pan = { PAN, SYNTH, BIPOL, 0, 127, "pan" };

// Misc parameters

// Channel we are editing
static const pd_t chan = { 0, CHAN, UNIPOL, 1, 16, "Channel" };

// Patch management
static const pd_t load_pgm = { 0, PGM, UNIPOL, 0, 127, "Program" };
static const pd_t store_pgm = { 0, PGM, UNIPOL, 0, 127, "Store program" };
static const pd_t pgm_stored = { 0, D_NONE, 0, 0, 0, "Program stored!" };

// Page definitions
//

static const struct page osc0_p =
  { L_OSC, { &osc0_dummy, &osc0_duty, &osc0_slope } };
static const struct page osc1_p1 =
  { L_OSC, { &osc1_mode, &osc1_duty, &osc1_slope, &osc1_atk, &osc1_dec, &osc1_lvl } };
static const struct page osc1_p2 =
  { L_OSC, { &osc1_dummy, &osc1_coarse, &osc1_fine, &osc_combn } };
static const struct page filt_p =
  { L_FILT, { &f_dummy, &f_cutoff, &f_q, &feg_amt, &vol, &pan } };
static const struct page feg_p =
  { L_ENV, { &feg_dummy, &f_atk, &f_dec, &f_sus, &f_rel, NULL } };
static const struct page aeg_p =
  { L_ENV, { &aeg_dummy, &a_atk, &a_dec, &a_sus, &a_rel, NULL } };
static const struct page chan_p =
  { L_HOME, { &chan, NULL, NULL, NULL, NULL, NULL } };
static const struct page load_p =
  { L_LOAD, { &load_pgm, NULL, NULL, NULL, NULL, NULL } };
static const struct page store_p =
  { L_HOME | L_LOAD, { &store_pgm, NULL, NULL, NULL, NULL, NULL } };
static const struct page stored_p =
  { L_HOME | L_LOAD, { &pgm_stored, NULL, NULL, NULL, NULL, NULL } };

struct transition {
  const struct page *page;      // If we're on this page...
  enum p6_buttons newbuttons;   // And this button gets pressed...
  enum p6_buttons downbuttons;  // While these buttons are held...
  const struct page *newpage;   // Then go to this page
  s32 (*action)(const struct page *page); // Call when page reached
};

static s32 do_store_pgm(const struct page *page);

static const struct transition transitions[] = {
  { &store_p, B_LOAD, B_NONE, &stored_p, &do_store_pgm },
  { NULL, B_LOAD, B_HOME, &store_p, NULL },
  { NULL, B_LOAD, B_NONE, &load_p, NULL },
  { NULL, B_HOME, B_NONE, &chan_p, NULL },
  { &osc0_p, B_OSC, B_NONE, &osc1_p1, NULL },
  { &osc1_p1, B_OSC, B_NONE, &osc1_p2, NULL },
  { NULL, B_OSC, B_NONE, &osc0_p, NULL },
  { NULL, B_FILT, B_NONE, &filt_p, NULL },
  { &aeg_p, B_ENV, B_NONE, &feg_p, NULL },
  { NULL, B_ENV, B_NONE, &aeg_p, NULL },
};

// File global variables

static int current_chan;
static int current_pgm;

static u32 current_button_mask; // which buttons are pressed?
static const struct page *current_page; // of menu handler

/////////////////////////////////////////////////////////////////////////////
// Actually store program
/////////////////////////////////////////////////////////////////////////////
static s32 do_store_pgm(const struct page *page)
{
  s32 res;

  res = MIOS32_IIC_BS_Write(0, current_pgm * EEPROM_BLOCKSIZE, SYNTH_GetParamsAddress(current_chan), NPARAM);

#if 0 // debugging
  static u8 buf[EEPROM_BLOCKSIZE];

  MIOS32_MIDI_SendDebugMessage("EEPROM Write: status %d\n", res);

  // Wait 5ms (max write time) before attempting to read back

  MIOS32_DELAY_Wait_uS(5000);
  
  // Verify by reading

  res = MIOS32_IIC_BS_Read(0, current_pgm * EEPROM_BLOCKSIZE, buf, EEPROM_BLOCKSIZE);

  MIOS32_MIDI_SendDebugMessage("EEPROM Read verify: status %d\n", res);
  
  MIOS32_MIDI_SendDebugMessage("Read verify: Data:\n");
  int i;
  for (i = 0; i < EEPROM_BLOCKSIZE; i += 16)
    MIOS32_MIDI_SendDebugMessage("%02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x  %02x %02x %02x %02x\n",
                                 buf[i+0], buf[i+1], buf[i+2], buf[i+3],
                                 buf[i+4], buf[i+5], buf[i+6], buf[i+7],
                                 buf[i+8], buf[i+9], buf[i+10], buf[i+11],
                                 buf[i+12], buf[i+13], buf[i+14], buf[i+15]);
#endif

  return res;
}

/////////////////////////////////////////////////////////////////////////////
// fetch synth parameter value
/////////////////////////////////////////////////////////////////////////////
static int PARAM_FetchSynthParamValue(const pd_t *param)
{
  return SYNTH_FetchParamValue(current_chan, param->param_id);
}

/////////////////////////////////////////////////////////////////////////////
// store synth parameter value
/////////////////////////////////////////////////////////////////////////////
static s32 PARAM_StoreSynthParamValue(const pd_t *param, u8 value)
{
  SYNTH_StoreParamValue(current_chan, param->param_id, value);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// fetch current program number
/////////////////////////////////////////////////////////////////////////////
static int PARAM_FetchPgmNum(const pd_t *param)
{
  return current_pgm;
}

/////////////////////////////////////////////////////////////////////////////
// store program number
/////////////////////////////////////////////////////////////////////////////
static s32 PARAM_StorePgmNum(const pd_t *param, u8 value)
{
  current_pgm = value;
  s32 res = 0;
  
  if (param == &load_pgm) {
MIOS32_MIDI_SendDebugMessage("Loading program %d\n", value);
    res = MIOS32_IIC_BS_Read(0, value * EEPROM_BLOCKSIZE, SYNTH_GetParamsAddress(current_chan), NPARAM);
    if (res == 0) SYNTH_ParamUpdate(current_chan);
  } 

#if 0
  static u8 buf[EEPROM_BLOCKSIZE];
  res = MIOS32_IIC_BS_Read(0, value * EEPROM_BLOCKSIZE, buf, EEPROM_BLOCKSIZE);

  MIOS32_MIDI_SendDebugMessage("EEPROM Read: status %d\n", res);
  
  MIOS32_MIDI_SendDebugMessage("Data:\n");
  int i;
  for (i = 0; i < EEPROM_BLOCKSIZE; i += 16)
    MIOS32_MIDI_SendDebugMessage("%02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x  %02x %02x %02x %02x\n",
                                 buf[i+0], buf[i+1], buf[i+2], buf[i+3],
                                 buf[i+4], buf[i+5], buf[i+6], buf[i+7],
                                 buf[i+8], buf[i+9], buf[i+10], buf[i+11],
                                 buf[i+12], buf[i+13], buf[i+14], buf[i+15]);
#endif

  return res;
}

/////////////////////////////////////////////////////////////////////////////
// fetch program number
/////////////////////////////////////////////////////////////////////////////
static int PARAM_FetchChanNum(const pd_t *param)
{
  return current_chan + 1;
}

/////////////////////////////////////////////////////////////////////////////
// store channel number
/////////////////////////////////////////////////////////////////////////////
static s32 PARAM_StoreChanNum(const pd_t *param, u8 value)
{
  current_chan = value - 1;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// print parameter value
/////////////////////////////////////////////////////////////////////////////
static s32 PARAM_DisplayParam(const pd_t *param, int relative_num, int print_name)
{
  if (!param) return -1; // blank for that parameter

  if (print_name) {
    MIOS32_LCD_CursorSet(relative_num * PARAM_SIZE + !!relative_num, 0);
    MIOS32_LCD_PrintString(param->name);
  }

  if (!managers[param->domain].fetch) return -1; // no fetch defined

  int value = managers[param->domain].fetch(param);

  MIOS32_LCD_CursorSet(relative_num * PARAM_SIZE + 1, 1);
  switch (param->display_type) {
    case BIPOL:
      value -= 64;
      /* fall through */
    case UNIPOL:
      MIOS32_LCD_PrintFormattedString("%3d", value);
      break;
    default: // String, display_type selects which set
      MIOS32_LCD_PrintString((*display_sets[param->display_type])[value]);
      break;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// display page
/////////////////////////////////////////////////////////////////////////////
s32 PARAM_DisplayPage(const struct page *page)
{
  u32 leds = MIOS32_BOARD_LED_Get();
  int rpar;

  if (!page) return -1; // No page defined

  MIOS32_BOARD_LED_Set(leds, 0); // Turn all off that are on
  MIOS32_BOARD_LED_Set(page->leds, 0xffff); // Indicate current state

  MIOS32_LCD_Clear();
  for (rpar = 0; rpar < PARAMS_PER_PAGE; rpar++) {
    PARAM_DisplayParam((page->params)[rpar], rpar, 1);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// state transition from button press
/////////////////////////////////////////////////////////////////////////////
s32 PARAM_ButtonHandle(u32 button_mask)
{
  u32 buttons_changed = button_mask ^ current_button_mask;
  u32 new_buttons_down = buttons_changed & button_mask;
  u32 old_button_mask = current_button_mask;
  int t;

  current_button_mask = button_mask;

  MIOS32_MIDI_SendDebugMessage("New button mask: 0x%x, old button mask 0x%x\n", current_button_mask, old_button_mask);

  if (!new_buttons_down) return 0; // never transition on button release

  // Scan transition table for a combination of current page (if any),
  // button pressed and button(s) held (if any), and go to new page if found.
  for (t = 0; t < sizeof(transitions)/ sizeof(struct transition); t++) {
    const struct transition *transition = &transitions[t];
    if (transition->page == NULL || transition->page == current_page) {
      if (transition->newbuttons == new_buttons_down &&
          transition->downbuttons == old_button_mask) {
        // We've found a new page. Transition to it.
        current_page = transition->newpage;
        MIOS32_MIDI_SendDebugMessage("New state 0x%x at line #%d", current_page, t);
        if (transition->action)
          transition->action(current_page);

        PARAM_DisplayPage(current_page);

        return 0;
      }
    }
  }

  return -1; // no valid transition found
}


/////////////////////////////////////////////////////////////////////////////
// called when encoder changes
/////////////////////////////////////////////////////////////////////////////
s32 PARAM_EncoderChange(u32 encoder, u32 delta)
{
  pd_t const *param;
  int value = 0;

  if (!current_page) return -1; // no current page set (e.g. at startup)
  param = current_page->params[encoder];
  if (!param) return -1; // no param at that location

  if (!managers[param->domain].fetch) return -1; // no fetch defined
  value = managers[param->domain].fetch(param);
  int newval = value + delta;

  if (newval > param->max) newval = param->max;
  else if (newval < param->min) newval = param->min;

  if (value == newval) return 0; // no change but no error either

  if (!managers[param->domain].store) return -1;
  managers[param->domain].store(param, newval);

  PARAM_DisplayParam(param, encoder, 0);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// initializes the param handler
/////////////////////////////////////////////////////////////////////////////
s32 PARAM_Init(u32 mode)
{
  current_page = transitions[0].newpage;
  PARAM_DisplayPage(current_page);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Called periodically to update channel controllers
/////////////////////////////////////////////////////////////////////////////
s32 PARAM_Update_1mS(void)
{
  // Do something useful
  return 0; // no error
}
