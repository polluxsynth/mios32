// $Id: app_lcd.c 2467 2016-12-30 09:40:35Z tk $
/*
 * Four-bit LCD driver for character displays with 4-bit interface and
 * no read capability, e.g. as used in the Audiothingies P6.
 *
 * See mios32_lcd_type_t in mios32_lcd.h for list of LCDs
 *
 * The LCD type will be fetched from the Bootloader Info section after reset.
 * It can be changed with the bootloader update application.
 *
 * Optionally it's also possible to change the type during runtime with
 * MIOS32_LCD_TypeSet (e.g. after it has been loaded from a config file from
 * SD Card)
 *
 * This driver has to be selected with MIOS32_LCD environment variable set
 * to "fourbit"
 *
 * Please only add drivers which are not resource hungry (e.g. they shouldn't
 * allocate RAM for buffering) - otherwise all applications would be affected!
 *
 * ==========================================================================
 *
 *  Copyright (C) 2008 Thorsten Klose (tk@midibox.org)
 *  Copyright (C) 2017 Ricard Wanderlof (polluxsynth@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>
#include <glcd_font.h>
#include <string.h>

#include "app_lcd.h"


/////////////////////////////////////////////////////////////////////////////
// Local defines
/////////////////////////////////////////////////////////////////////////////

// optional debug messages
#define DEBUG_VERBOSE_LEVEL 0

// Pin definitions for P6 LCD

#define DATA_PORT    GPIOB
#define D4_PIN       GPIO_Pin_10
#define D5_PIN       GPIO_Pin_14
#define D6_PIN       GPIO_Pin_11
#define D7_PIN       GPIO_Pin_15

#define CTRL_PORT    GPIOC
#define E_PIN        GPIO_Pin_1
#define RS_PIN       GPIO_Pin_0
// Note there is no WR pin, WR is tied to ground => can't read from LCD

// Values for RS
#define CMD_REG      0
#define DATA_REG     1     

/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

static u8 lcd_testmode = 0;

/////////////////////////////////////////////////////////////////////////////
// Write four-bit data (bits0..3) and pulse E, then delay
/////////////////////////////////////////////////////////////////////////////
static s32 APP_LCD_DataWrite4(u8 data, u8 long_delay)
{
  // Assume E is low
  MIOS32_SYS_STM_PINSET(DATA_PORT, D4_PIN, data & 1)
  MIOS32_SYS_STM_PINSET(DATA_PORT, D5_PIN, data & 2)
  MIOS32_SYS_STM_PINSET(DATA_PORT, D6_PIN, data & 4)
  MIOS32_SYS_STM_PINSET(DATA_PORT, D7_PIN, data & 8)
  MIOS32_SYS_STM_PINSET(CTRL_PORT, E_PIN, 1);
  MIOS32_DELAY_Wait_uS(1);
  MIOS32_SYS_STM_PINSET(CTRL_PORT, E_PIN, 0);
  MIOS32_DELAY_Wait_uS(long_delay ? 2000 : 40); // 2ms vs 40us

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Set RS line
/////////////////////////////////////////////////////////////////////////////
static s32 APP_LCD_RS(u8 reg)
{
  MIOS32_SYS_STM_PINSET(CTRL_PORT, RS_PIN, reg);

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Initializes application specific LCD driver
// IN: <mode>: optional configuration
// OUT: returns < 0 if initialisation failed
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_Init(u32 mode)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  // currently only mode 0 supported
  if( mode != 0 )
    return -1; // unsupported mode

  switch( mios32_lcd_parameters.lcd_type ) {

  case MIOS32_LCD_TYPE_CLCD:
  case MIOS32_LCD_TYPE_CLCD_DOG:
  case MIOS32_LCD_TYPE_CLCD_PP:
  default: {

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    // configure pins
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; // weak driver to reduce transients

    // 4 data lines
    GPIO_InitStructure.GPIO_Pin = D4_PIN | D5_PIN | D6_PIN | D7_PIN;
    GPIO_Init(DATA_PORT, &GPIO_InitStructure);

    // control lines. Note that RW is tied low in hardware.
    GPIO_InitStructure.GPIO_Pin = RS_PIN | E_PIN;
    GPIO_Init(CTRL_PORT, &GPIO_InitStructure);
    MIOS32_SYS_STM_PINSET(CTRL_PORT, E_PIN, 0);

    // initialize LCD
   
    // Get interface into 4-bit mode
    APP_LCD_RS(CMD_REG);
    APP_LCD_DataWrite4(0x3, 1); // long delay
    APP_LCD_DataWrite4(0x3, 0); // short delay
    APP_LCD_DataWrite4(0x3, 0); // short delay
    APP_LCD_DataWrite4(0x2, 0); // short delay

    APP_LCD_Cmd(0x28); // Func set: 4 bit mode, 2 lines, 5x8 font (#0)

    APP_LCD_Cmd(0x08); // Display Off

    APP_LCD_Cmd(0x0c); // Display On
    APP_LCD_Cmd(0x06); // Entry Mode
    APP_LCD_Cmd(0x01); // Clear Display

#if 0 // shouldn't be needed
    APP_LCD_Cmd(0x28); // experience from PIC based MIOS: without these lines
    APP_LCD_Cmd(0x0c); // the LCD won't work correctly after a second APP_LCD_Init
#endif
  }
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////
// Sends data byte to LCD
// IN: data byte in <data>
// OUT: returns < 0 if display not available or timed out
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_Data(u8 data)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  switch( mios32_lcd_parameters.lcd_type ) {

  case MIOS32_LCD_TYPE_CLCD:
  case MIOS32_LCD_TYPE_CLCD_PP:
  default: {

    APP_LCD_RS(DATA_REG);
    APP_LCD_DataWrite4(data >> 4, 0); // high nybble
    APP_LCD_DataWrite4(data, 0); // low nybble

    return 0; // no error
  }
  }

  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Sends command byte to LCD
// IN: command byte in <cmd>
// OUT: returns < 0 if display not available or timed out
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_Cmd(u8 cmd)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  switch( mios32_lcd_parameters.lcd_type ) {

  case MIOS32_LCD_TYPE_CLCD:
  case MIOS32_LCD_TYPE_CLCD_PP:
  default: {

    APP_LCD_RS(CMD_REG);
    APP_LCD_DataWrite4(cmd >> 4, 0); // high nybble
    APP_LCD_DataWrite4(cmd, !(cmd & 0xfc)); // long delay for cls and home

    return 0; // no error
  }
  }

  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Clear Screen
// IN: -
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_Clear(void)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  switch( mios32_lcd_parameters.lcd_type ) {

  case MIOS32_LCD_TYPE_CLCD:
  case MIOS32_LCD_TYPE_CLCD_PP:
  default:
    // -> send clear command
    return APP_LCD_Cmd(0x01);
  }

  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Sets cursor to given position
// IN: <column> and <line>
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_CursorSet(u16 column, u16 line)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  // exit with error if line is not in allowed range
  if( line >= MIOS32_LCD_MAX_MAP_LINES )
    return -1;

  // -> set cursor address
  return APP_LCD_Cmd(0x80 | (mios32_lcd_cursor_map[line] + column));
}


/////////////////////////////////////////////////////////////////////////////
// Sets graphical cursor to given position
// IN: <x> and <y>
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_GCursorSet(u16 x, u16 y)
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Initializes a single special character
// IN: character number (0-7) in <num>, pattern in <table[8]>
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_SpecialCharInit(u8 num, u8 table[8])
{
  if( lcd_testmode )
    return -1; // direct access disabled in testmode

  if( mios32_lcd_parameters.lcd_type >= 0x80 ) { // GLCD
  } else { // CLCD
    s32 i;

    // send character number
    APP_LCD_Cmd(((num&7)<<3) | 0x40);

    // send 8 data bytes
    for(i=0; i<8; ++i)
      if( APP_LCD_Data(table[i]) < 0 )
	return -1; // error during sending character

    // set cursor to original position
    return APP_LCD_CursorSet(mios32_lcd_column, mios32_lcd_line);
  }

  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Sets the background colour
// Only relevant for colour GLCDs
// IN: r/g/b value
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_BColourSet(u32 rgb)
{
  return -3; // not supported
}


/////////////////////////////////////////////////////////////////////////////
// Sets the foreground colour
// Only relevant for colour GLCDs
// IN: r/g/b value
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_FColourSet(u32 rgb)
{
  return -3; // not supported
}



/////////////////////////////////////////////////////////////////////////////
// Sets a pixel in the bitmap
// IN: bitmap, x/y position and colour value (value range depends on APP_LCD_COLOUR_DEPTH)
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_BitmapPixelSet(mios32_lcd_bitmap_t bitmap, u16 x, u16 y, u32 colour)
{
  if( x >= bitmap.width || y >= bitmap.height )
    return -1; // pixel is outside bitmap

  // all GLCDs support the same bitmap scrambling
  u8 *pixel = (u8 *)&bitmap.memory[bitmap.line_offset*(y / 8) + x];
  u8 mask = 1 << (y % 8);

  *pixel &= ~mask;
  if( colour )
    *pixel |= mask;

  return -3; // not supported
}



/////////////////////////////////////////////////////////////////////////////
// Transfers a Bitmap within given boundaries to the LCD
// IN: bitmap
// OUT: returns < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_BitmapPrint(mios32_lcd_bitmap_t bitmap)
{
  return -1; // no GLCD
}


/////////////////////////////////////////////////////////////////////////////
// Optional alternative pinning options
// E.g. for MIDIbox CV which accesses a CLCD at J15, and SSD1306 displays at J5/J28 (STM32F4: J5/J10B)
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_AltPinningSet(u8 alt_pinning)
{
  return -1;
}

u8 APP_LCD_AltPinningGet(void)
{
  return -1;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Terminal Functions
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#if 0
/////////////////////////////////////////////////////////////////////////////
// help function which parses a decimal or hex value
// returns >= 0 if value is valid
// returns -1 if value is invalid
/////////////////////////////////////////////////////////////////////////////
static s32 get_dec(char *word)
{
  if( word == NULL )
    return -1;

  char *next;
  long l = strtol(word, &next, 0);

  if( word == next )
    return -1;

  return l; // value is valid
}
#endif


/////////////////////////////////////////////////////////////////////////////
// Returns help page for implemented terminal commands of this module
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_TerminalHelp(void *_output_function)
{
  void (*out)(char *format, ...) = _output_function;

  out("  testlcdpin:     type this command to get further informations about this testmode.");

  return 0; // no error
}

#if 0 // Not used?
/////////////////////////////////////////////////////////////////////////////
// Parser for a complete line
// Returns > 0 if command line matches with UIP terminal commands
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_TerminalParseLine(char *input, void *_output_function)
{
  void (*out)(char *format, ...) = _output_function;
  char *separators = " \t";
  char *brkt;
  char *parameter;

  // since strtok_r works destructive (separators in *input replaced by NUL), we have to restore them
  // on an unsuccessful call (whenever this function returns < 1)
  int input_len = strlen(input);

  if( (parameter = strtok_r(input, separators, &brkt)) ) {
    if( strcasecmp(parameter, "testlcdpin") == 0 ) {
      char *arg;
      int pin_number = -1;
      int level = -1;
      
      if( (arg = strtok_r(NULL, separators, &brkt)) ) {
	if( strcasecmp(arg, "rs") == 0 )
	  pin_number = 1;
	else if( strcasecmp(arg, "e1") == 0 )
	  pin_number = 2;
	else if( strcasecmp(arg, "e2") == 0 )
	  pin_number = 3;
	else if( strcasecmp(arg, "rw") == 0 )
	  pin_number = 4;
	else if( strcasecmp(arg, "d0") == 0 )
	  pin_number = 5;
	else if( strcasecmp(arg, "d1") == 0 )
	  pin_number = 6;
	else if( strcasecmp(arg, "d2") == 0 )
	  pin_number = 7;
	else if( strcasecmp(arg, "d3") == 0 )
	  pin_number = 8;
	else if( strcasecmp(arg, "d4") == 0 )
	  pin_number = 9;
	else if( strcasecmp(arg, "d5") == 0 )
	  pin_number = 10;
	else if( strcasecmp(arg, "d6") == 0 )
	  pin_number = 11;
	else if( strcasecmp(arg, "d7") == 0 )
	  pin_number = 12;
	else if( strcasecmp(arg, "reset") == 0 ) {
	  pin_number = 0;
	  level = 0; // dummy
	}
      }
      
      if( pin_number < 0 ) {
	out("Please specifiy valid LCD pin name: rs, e1, e2, rw, d0, d1, ... d7\n");
      }

      if( (arg = strtok_r(NULL, separators, &brkt)) )
	level = get_dec(arg);
	
      if( level != 0 && level != 1 ) {
	out("Please specifiy valid logic level for LCD pin: 0 or 1\n");
      }

      if( pin_number >= 0 && level >= 0 ) {
	lcd_testmode = 1;

	// clear all pins
	MIOS32_BOARD_J15_PortInit(0);
	
	switch( pin_number ) {
	case 1: {
	  MIOS32_BOARD_J15_RS_Set(level);
	  out("J15A.RS and J15B.RS set to ca. %s", level ? "3.3V" : "0V");
	} break;

	case 2: {
	  MIOS32_BOARD_J15_E_Set(0, level);
	  out("J15A.E set to ca. %s", level ? "3.3V" : "0V");
	} break;

	case 3: {
	  MIOS32_BOARD_J15_E_Set(1, level);
	  out("J15B.E set to ca. %s", level ? "3.3V" : "0V");
	} break;

	case 4: {
	  MIOS32_BOARD_J15_RW_Set(level);
	  out("J15A.RW and J15B.RW set to ca. %s", level ? "3.3V" : "0V");
	} break;

	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12: {
	  u8 d_pin = pin_number-5;
	  MIOS32_BOARD_J15_DataSet(level ? (1 << d_pin) : 0x00);
	  out("J15A.D%d and J15B.D%d set to ca. %s", d_pin, d_pin, level ? "5V (resp. 3.3V)" : "0V");

	  int d7_in = MIOS32_BOARD_J15_GetD7In();
	  if( d7_in < 0 ) {
	    out("ERROR: LCD driver not enabled?!?");
	  } else if( d_pin == 7 && level == 1 && d7_in == 1 ) {
	    out("D7 input pin correctly shows logic-1");
	  } else if( d7_in == 0 ) {
	    out("D7 input pin correctly shows logic-0");
	  } else {
	    out("ERROR: D7 input pin shows unexpected logic-%d level!", d7_in);
	    out("Please check the D7 feedback line on your core module!");
	  }

	} break;

	default:
	  lcd_testmode = 0;
	  u8 prev_dev = mios32_lcd_device;

	  mios32_lcd_device = 0;
	  APP_LCD_Init(0);
	  MIOS32_LCD_PrintString("READY.");
	  mios32_lcd_device = 1;
	  APP_LCD_Init(0);
	  MIOS32_LCD_PrintString("READY.");

	  mios32_lcd_device = prev_dev;

	  out("LCD Testmode is disabled now.");
	  out("It makes sense to reset the application, so that the LCD screen will be re-initialized!");
	}

      } else {
	out("Following commands are supported:\n");
	out("testlcdpin rs 0  -> sets J15(AB):RS to ca. 0V");
	out("testlcdpin rs 1  -> sets J15(AB):RS to ca. 3.3V");
	out("testlcdpin e1 0  -> sets J15A:E to ca. 0V");
	out("testlcdpin e1 1  -> sets J15A:E to ca. 3.3V");
	out("testlcdpin e2 0  -> sets J15B:E to ca. 0V");
	out("testlcdpin e2 1  -> sets J15B:E to ca. 3.3V");
	out("testlcdpin rw 0  -> sets J15(AB):RW to ca. 0V");
	out("testlcdpin rw 1  -> sets J15(AB):RW to ca. 3.3V");
	out("testlcdpin d0 0  -> sets J15(AB):D0 to ca. 0V");
	out("testlcdpin d0 1  -> sets J15(AB):D0 to ca. 5V (resp. 3.3V)");
	out("testlcdpin d1 0  -> sets J15(AB):D1 to ca. 0V");
	out("testlcdpin d1 1  -> sets J15(AB):D1 to ca. 5V (resp. 3.3V)");
	out("testlcdpin d...  -> same for J15(AB):D2, D3, D4, D5, D6, D7");
	out("testlcdpin reset -> re-initializes LCD modules so that they can be used again.");
	out("The testmode is currently %s", lcd_testmode ? "enabled" : "disabled");
      }
      return 1; // command taken
    }
  }

  // restore input line (replace NUL characters by spaces)
  int i;
  char *input_ptr = input;
  for(i=0; i<input_len; ++i, ++input_ptr)
    if( !*input_ptr )
      *input_ptr = ' ';

  return 0; // command not taken
}


/////////////////////////////////////////////////////////////////////////////
// Prints the current configuration
/////////////////////////////////////////////////////////////////////////////
s32 APP_LCD_TerminalPrintConfig(void *_output_function)
{
  void (*out)(char *format, ...) = _output_function;

  out("lcd_type: 0x%02x - %s\n",
      mios32_lcd_parameters.lcd_type,
      MIOS32_LCD_LcdTypeName(mios32_lcd_parameters.lcd_type)
      ? MIOS32_LCD_LcdTypeName(mios32_lcd_parameters.lcd_type)
      : "unknown");
  out("lcd_num_x: %d\n", mios32_lcd_parameters.num_x);
  out("lcd_num_y: %d\n", mios32_lcd_parameters.num_y);
  out("lcd_width: %d\n", mios32_lcd_parameters.width);
  out("lcd_height: %d\n", mios32_lcd_parameters.height);

  return 0; // no error
}
#endif
