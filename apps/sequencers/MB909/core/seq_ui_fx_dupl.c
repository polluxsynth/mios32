// $Id: seq_ui_fx_limit.c 1142 2011-02-17 23:19:36Z tk $
/*
 * Duplicate Note Fx page
 *
 * ==========================================================================
 *
 *  Copyright (C) 2008 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>
#include "seq_lcd.h"
#include "seq_ui.h"
#include "seq_cc.h"
#include "seq_midi_port.h"
#include <glcd_font.h>

/////////////////////////////////////////////////////////////////////////////
// Local definitions
/////////////////////////////////////////////////////////////////////////////

#define NUM_OF_ITEMS       6
#define ITEM_GXTY          0
#define ITEM_NUM_CHN       1
#define ITEM_FIRST_CHN     2
#define ITEM_PORT          3
#define ITEM_FWD_NON_NOTES 4
#define ITEM_BEH           5


/////////////////////////////////////////////////////////////////////////////
// Local LED handler function
/////////////////////////////////////////////////////////////////////////////
static s32 LED_Handler(u16 *gp_leds)
{
  if( ui_cursor_flash ) // if flashing flag active: no LED flag set
    return 0;

  switch( ui_selected_item ) {
  case ITEM_GXTY:          *gp_leds = 0x0001; break;
  case ITEM_NUM_CHN:       *gp_leds = 0x001e; break;
  case ITEM_FIRST_CHN:     *gp_leds = 0x00e0; break;
  case ITEM_PORT:          *gp_leds = 0x0100; break;
  case ITEM_FWD_NON_NOTES: *gp_leds = 0x0600; break;
  case ITEM_BEH:           *gp_leds = 0xf800; break;
  }

  return 0; // no error
}


/////////////////////////////////////////////////////////////////////////////
// Local encoder callback function
// Should return:
//   1 if value has been changed
//   0 if value hasn't been changed
//  -1 if invalid or unsupported encoder
/////////////////////////////////////////////////////////////////////////////
static s32 Encoder_Handler(seq_ui_encoder_t encoder, s32 incrementer)
{
  u8 visible_track = SEQ_UI_VisibleTrackGet();

  switch( encoder ) {
  case SEQ_UI_ENCODER_GP1:
	if( SEQ_UI_Var8_Inc(&ui_selected_item, 0, NUM_OF_ITEMS-1, incrementer) >= 0 )
		return 1;
	else
		return 0;  
  
  //  ui_selected_item = ITEM_GXTY;
    break;

  case SEQ_UI_ENCODER_GP2:
  case SEQ_UI_ENCODER_GP3:
  case SEQ_UI_ENCODER_GP4:
  case SEQ_UI_ENCODER_GP5:
    ui_selected_item = ITEM_NUM_CHN;
    break;

  case SEQ_UI_ENCODER_GP6:
  case SEQ_UI_ENCODER_GP7:
  case SEQ_UI_ENCODER_GP8:
    ui_selected_item = ITEM_FIRST_CHN;
    break;

  case SEQ_UI_ENCODER_GP9:
    ui_selected_item = ITEM_PORT;
    break;

  case SEQ_UI_ENCODER_GP10:
  case SEQ_UI_ENCODER_GP11:
    ui_selected_item = ITEM_FWD_NON_NOTES;
    break;

  case SEQ_UI_ENCODER_GP12:
  case SEQ_UI_ENCODER_GP13:
  case SEQ_UI_ENCODER_GP14:
  case SEQ_UI_ENCODER_GP15:
  case SEQ_UI_ENCODER_GP16:
    ui_selected_item = ITEM_BEH;
    break;
  }

  // for GP encoders and Datawheel
 if (encoder == SEQ_UI_ENCODER_Datawheel) { 
  switch( ui_selected_item ) {
  case ITEM_GXTY:          return SEQ_UI_GxTyInc(incrementer);
  case ITEM_NUM_CHN:       return SEQ_UI_CC_Inc(SEQ_CC_FX_MIDI_NUM_CHANNELS, 0, 16, incrementer);
  case ITEM_FIRST_CHN:     return SEQ_UI_CC_Inc(SEQ_CC_FX_MIDI_CHANNEL, 0, 15, incrementer);
  case ITEM_PORT: {
    mios32_midi_port_t port = SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_PORT);
    u8 port_ix = SEQ_MIDI_PORT_OutIxGet(port);
    if( SEQ_UI_Var8_Inc(&port_ix, 0, SEQ_MIDI_PORT_OutNumGet()-1-4, incrementer) ) { // -4 so that Bus1..Bus4 can't be selected (not supported)
      mios32_midi_port_t new_port = SEQ_MIDI_PORT_OutPortGet(port_ix);
      SEQ_UI_CC_Set(SEQ_CC_FX_MIDI_PORT, new_port);
      return 1; // value changed
    }
    return 0; // value not changed
  } break;
  case ITEM_FWD_NON_NOTES: {
    if( !incrementer ) // toggle flag
      incrementer = (SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_MODE) & (1<<3)) ? -1 : 1;
    return SEQ_UI_CC_SetFlags(SEQ_CC_FX_MIDI_MODE, (1<<3), (incrementer >= 0) ? (1<<3) : 0);
  }
  case ITEM_BEH: {
    u8 beh = SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_MODE) & 7;
    if( SEQ_UI_Var8_Inc(&beh, 0, 3, incrementer) > 0 ) {
      return SEQ_UI_CC_SetFlags(SEQ_CC_FX_MIDI_MODE, (7<<0), beh);
    }
    return 0;
  }
  }
 }
  return -1; // invalid or unsupported encoder
}


/////////////////////////////////////////////////////////////////////////////
// Local button callback function
// Should return:
//   1 if value has been changed
//   0 if value hasn't been changed
//  -1 if invalid or unsupported button
/////////////////////////////////////////////////////////////////////////////
static s32 Button_Handler(seq_ui_button_t button, s32 depressed)
{
  if( depressed ) return 0; // ignore when button depressed

#if 0
  // leads to: comparison is always true due to limited range of data type
  if( button >= SEQ_UI_BUTTON_GP1 && button <= SEQ_UI_BUTTON_GP16 ) {
#else
  if( button <= SEQ_UI_BUTTON_GP16 ) {
#endif
    // re-use encoder handler - only select UI item, don't increment
    return Encoder_Handler((int)button, 0);
  }

  // remaining buttons:
  switch( button ) {
    case SEQ_UI_BUTTON_Select:
    case SEQ_UI_BUTTON_Right:
      if( ++ui_selected_item >= NUM_OF_ITEMS )
	ui_selected_item = 0;
      return 1; // value always changed

    case SEQ_UI_BUTTON_Left:
      if( ui_selected_item == 0 )
	ui_selected_item = NUM_OF_ITEMS-1;
      else
	--ui_selected_item;
      return 1; // value always changed

    case SEQ_UI_BUTTON_Up:
      return Encoder_Handler(SEQ_UI_ENCODER_Datawheel, 1);

    case SEQ_UI_BUTTON_Down:
      return Encoder_Handler(SEQ_UI_ENCODER_Datawheel, -1);
  }

  return -1; // invalid or unsupported button
}


/////////////////////////////////////////////////////////////////////////////
// Local Display Handler function
// IN: <high_prio>: if set, a high-priority LCD update is requested
/////////////////////////////////////////////////////////////////////////////
static s32 LCD_Handler(u8 high_prio)
{
  if( high_prio )
    return 0; // there are no high-priority updates

  // layout:
  // 00000000001111111111222222222233333333330000000000111111111122222222223333333333
  // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
  // <--------------------------------------><-------------------------------------->
  // Trk. Number of additional  First ChannelPort Non-Notes  Mode                    
  // GxTy     Channels:  3            2      Same Forwarded  Alternate with EchoSynch

  u8 visible_track = SEQ_UI_VisibleTrackGet();

  
  
  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 0);

  if( ui_selected_item == ITEM_GXTY ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintGxTy(ui_selected_group, ui_selected_tracks);
    //SEQ_LCD_PrintSpaces(2);
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintGxTy(ui_selected_group, ui_selected_tracks);
    //SEQ_LCD_PrintSpaces(2);
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  }  
  SEQ_LCD_PrintString("DUPLICATE NOTE Fx");
  
  ///////////////////////////////////////////////////////////////////////////
  //SEQ_LCD_CursorSet(0, 0);
  //SEQ_LCD_PrintString("Trk. Number of additional  First ChannelPort Non-Notes  Mode");
  //SEQ_LCD_PrintSpaces(20);

  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 1);
/*
  if( ui_selected_item == ITEM_GXTY && ui_cursor_flash ) {
    SEQ_LCD_PrintSpaces(4);
  } else {
    SEQ_LCD_PrintGxTy(ui_selected_group, ui_selected_tracks);
  }
  SEQ_LCD_PrintSpaces(5);
*/
  ///////////////////////////////////////////////////////////////////////////
  //SEQ_LCD_PrintString("Extra Chns: ");
  
  if( ui_selected_item == ITEM_NUM_CHN ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintString("Extra Channels: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintString("Extra Channels: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  }  
  SEQ_LCD_PrintFormattedString("%2d", SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_NUM_CHANNELS));
  SEQ_LCD_PrintSpaces(1);
/*  
  if( ui_selected_item == ITEM_NUM_CHN && ui_cursor_flash ) {
    SEQ_LCD_PrintSpaces(2);
  } else {
    SEQ_LCD_PrintFormattedString("%2d", SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_NUM_CHANNELS));
  }
  SEQ_LCD_PrintSpaces(11);
*/
  
  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 2);  
  if( ui_selected_item == ITEM_FIRST_CHN ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintString("First Channel : ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintString("First Channel : ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } 
  SEQ_LCD_PrintFormattedString("%2d", SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_CHANNEL) + 1);
  
/*  
  if( ui_selected_item == ITEM_FIRST_CHN && ui_cursor_flash ) {
    SEQ_LCD_PrintSpaces(2);
  } else {
    SEQ_LCD_PrintFormattedString("%2d", SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_CHANNEL) + 1);
  }
*/
  SEQ_LCD_PrintSpaces(1);

  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 3);
  if( ui_selected_item == ITEM_PORT ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintString("Port     : ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintString("Port: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  }
  
  mios32_midi_port_t port = SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_PORT);
  if( ui_selected_item == ITEM_PORT && ui_cursor_flash ) {
    SEQ_LCD_PrintSpaces(5);
  } else {
    if( port == DEFAULT ) {
      SEQ_LCD_PrintString("Same ");
    } else {
      SEQ_LCD_PrintMIDIOutPort(port);
      SEQ_LCD_PrintChar(SEQ_MIDI_PORT_OutCheckAvailable(port) ? ' ' : '*');
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 4);  
  if( ui_selected_item == ITEM_FWD_NON_NOTES ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintString("Non-Notes: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintString("Non-Notes: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  }  
  seq_core_fx_midi_mode_t fx_midi_mode;
  fx_midi_mode.ALL = SEQ_CC_Get(visible_track, SEQ_CC_FX_MIDI_MODE);
//  if( ui_selected_item == ITEM_FWD_NON_NOTES && ui_cursor_flash ) {
//    SEQ_LCD_PrintSpaces(9);
//  } else {
    if( fx_midi_mode.fwd_non_notes ) {
      SEQ_LCD_PrintString("Forwarded");
    } else {
      SEQ_LCD_PrintString("Filtered ");
    }
//  }
  SEQ_LCD_PrintSpaces(2);

  ///////////////////////////////////////////////////////////////////////////
  SEQ_LCD_CursorSet(0, 5);
  if( ui_selected_item == ITEM_BEH ) {
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL_INV);
    SEQ_LCD_PrintString("Mode     : ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  } else {
    SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
	SEQ_LCD_PrintString("Mode: ");
	SEQ_LCD_FontInit((u8 *)GLCD_FONT_NORMAL);
  }

  SEQ_LCD_CursorSet(0, 6);  
//  if( ui_selected_item == ITEM_BEH && ui_cursor_flash ) {
//    SEQ_LCD_PrintSpaces(21);
//  } else {
    const char *mode_str[4] = {
      "Forward to all Chanls",
      "Alternate Channels   ",
      "Alterna with EchoSync",
      "Random Chanl Selectn ",
    };
    SEQ_LCD_PrintStringPadded((char *)mode_str[(fx_midi_mode.beh < 4) ? fx_midi_mode.beh : 3], 21);
  //}


  ///////////////////////////////////////////////////////////////////////////

  return 0; // no error
}


/////////////////////////////////////////////////////////////////////////////
// Initialisation
/////////////////////////////////////////////////////////////////////////////
s32 SEQ_UI_FX_DUPL_Init(u32 mode)
{
  // install callback routines
  SEQ_UI_InstallButtonCallback(Button_Handler);
  SEQ_UI_InstallEncoderCallback(Encoder_Handler);
  SEQ_UI_InstallLEDCallback(LED_Handler);
  SEQ_UI_InstallLCDCallback(LCD_Handler);

  return 0; // no error
}
