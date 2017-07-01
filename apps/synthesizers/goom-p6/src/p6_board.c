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

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>
#include <p6_board.h>

/////////////////////////////////////////////////////////////////////////////
// Global variables
/////////////////////////////////////////////////////////////////////////////
// Emulate SRIO
// Don't really have to be volatile, but SRIO uses DMA so let's play along
u32 mios32_p6_inbits;
u32 mios32_p6_inbits_prev;
u32 mios32_p6_inbits_changed;
u32 mios32_p6_outbits;


/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

// '165 chain
#define DIN_DATA_PIN   GPIOA, GPIO_Pin_0
#define DIN_CLK_PIN    GPIOA, GPIO_Pin_1
#define DIN_INH_PIN    GPIOA, GPIO_Pin_2
#define DIN_LATCH_PIN  GPIOA, GPIO_Pin_3

// '595
#define DOUT_OE_PIN    GPIOA, GPIO_Pin_5
#define DOUT_DATA_PIN  GPIOA, GPIO_Pin_6
#define DOUT_CLK_PIN   GPIOA, GPIO_Pin_7
#define DOUT_LATCH_PIN GPIOA, GPIO_Pin_8

// Bit-mapped button bits
#define S1_PIN         GPIOC, GPIO_Pin_6
#define S1_PINSHIFT    0
#define S4_PIN         GPIOC, GPIO_Pin_8
#define S4_PINSHIFT    3
#define S7_PIN         GPIOC, GPIO_Pin_9
#define S7_PINSHIFT    6

// Bit-mapped LED
#define LED1_PIN       GPIOC, GPIO_Pin_2

#define NUM_SRBITS 24 // Shift register bits: 8x '595
#define NUM_INBITS 28 // total number of input bits, including 1 dummy

#define NUM_OUTBITS 8 // Shift register bits

#define NUM_ENCODERS 6

// Inline functions so we can bundle pin ID's into one #define
// (MIOS32_SYS_STM_PINSET/PINGET are macros which are not lightly fooled)
static inline void MIOS32_BOARD_PinSet(GPIO_TypeDef *port, u16 pin, int value)
{
  MIOS32_SYS_STM_PINSET(port, pin, value);
}

static inline int MIOS32_BOARD_PinGet(GPIO_TypeDef *port, u16 pin)
{
  return MIOS32_SYS_STM_PINGET(port, pin);
}

/////////////////////////////////////////////////////////////////////////////
//! Internally used help function to initialize a pin
/////////////////////////////////////////////////////////////////////////////
static s32 MIOS32_BOARD_PinInitHlp(GPIO_TypeDef *port, u16 pin_mask, mios32_board_pin_mode_t mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = pin_mask;

  switch( mode ) {
  case MIOS32_BOARD_PIN_MODE_IGNORE:
    return 0; // don't touch
  case MIOS32_BOARD_PIN_MODE_ANALOG:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT_PD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    break;
  case MIOS32_BOARD_PIN_MODE_INPUT_PU:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    break;
  case MIOS32_BOARD_PIN_MODE_OUTPUT_PP:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    break;
  case MIOS32_BOARD_PIN_MODE_OUTPUT_OD:
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    break;
  default:
    return -2; // invalid pin mode
  }

  // init IO mode
  GPIO_Init(port, &GPIO_InitStructure);

  return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// Set up scan chains and initiate decoders
/////////////////////////////////////////////////////////////////////////////

s32 MIOS32_BOARD_P6_ScanInit(void)
{
  // input scan chain
  MIOS32_BOARD_PinInitHlp(DIN_DATA_PIN, MIOS32_BOARD_PIN_MODE_INPUT);
  MIOS32_BOARD_PinInitHlp(DIN_CLK_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);
  MIOS32_BOARD_PinInitHlp(DIN_INH_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);
  MIOS32_BOARD_PinInitHlp(DIN_LATCH_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);

  // initial pin state
  MIOS32_BOARD_PinSet(DIN_INH_PIN, 0); // Leave at 0 to enable clock
  MIOS32_BOARD_PinSet(DIN_LATCH_PIN, 1); // Parallel load when -> 0
  MIOS32_BOARD_PinSet(DIN_CLK_PIN, 1); // Clock data when rising, but must be high during parallel load

  // output scan chain
  MIOS32_BOARD_PinInitHlp(DOUT_OE_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);
  MIOS32_BOARD_PinInitHlp(DOUT_DATA_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);
  MIOS32_BOARD_PinInitHlp(DOUT_CLK_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);
  MIOS32_BOARD_PinInitHlp(DOUT_LATCH_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);

  // initial pin state
  MIOS32_BOARD_PinSet(DOUT_OE_PIN, 0); // Enable outputs (should we really here?)
  MIOS32_BOARD_PinSet(DOUT_LATCH_PIN, 0); // Latch on rising edge
  MIOS32_BOARD_PinSet(DOUT_CLK_PIN, 0); // Clock on rising edge

  // S1, S4 and S7 bits
  MIOS32_BOARD_PinInitHlp(S1_PIN, MIOS32_BOARD_PIN_MODE_INPUT_PD);
  MIOS32_BOARD_PinInitHlp(S4_PIN, MIOS32_BOARD_PIN_MODE_INPUT_PD);
  MIOS32_BOARD_PinInitHlp(S7_PIN, MIOS32_BOARD_PIN_MODE_INPUT_PD);

  // LED1
  MIOS32_BOARD_PinInitHlp(LED1_PIN, MIOS32_BOARD_PIN_MODE_OUTPUT_PP);

  int enc;
  for(enc=0; enc<NUM_ENCODERS; ++enc) {
    mios32_enc_config_t enc_config = MIOS32_ENC_ConfigGet(enc);
    enc_config.cfg.type = NON_DETENTED; // see mios32_enc.h for available types
    enc_config.cfg.sr = 0; // must be 0 if controlled from application
    enc_config.cfg.pos = 0; // doesn't matter if controlled from application
    enc_config.cfg.speed = NORMAL;
    enc_config.cfg.speed_par = 0;
    MIOS32_ENC_ConfigSet(enc, enc_config);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Scan chains
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_P6_ScanStart(void)
{
  // Shove the bits into proper places in output vector:
  // bits 0..8: 9 buttons
  // bits 9..14: 6 encoder buttons
  // bits 16..27: 6 quadrature encoder outputs (2 per encoder)
  static const u8 bitshift[NUM_SRBITS] = {
    1, 9, 16, 17, // S2, enc 1 button, enc 1
    2, 10, 18, 19, // S3, enc 2 button, enc 2
    4, 11, 20, 21, // S5, enc 3 button, enc 3
    5, 12, 22, 23, // S6, enc 4 button, enc 4
    7, 13, 24, 25, // S8, enc 5 button, enc 5
    8, 14, 26, 27, // S9, enc 6 button, enc 6
  };

  // Have no fancy hardware to handle this, so we need to do it here in code.
  int bit;
  
  // '165 Input chain
  
  MIOS32_BOARD_PinSet(DIN_LATCH_PIN, 0); // Pulse low
  MIOS32_BOARD_PinSet(DIN_LATCH_PIN, 0); // Pulse stretch
  MIOS32_BOARD_PinSet(DIN_LATCH_PIN, 1); // Leave high for readout

  mios32_p6_inbits = 0;
  for (bit = 0; bit < NUM_SRBITS; bit++) {
    MIOS32_BOARD_PinSet(DIN_CLK_PIN, 0);  // Pulse low
    MIOS32_BOARD_PinSet(DIN_CLK_PIN, 0);  // Pulse stretch
    MIOS32_BOARD_PinSet(DIN_CLK_PIN, 0);  // Pulse stretch
    mios32_p6_inbits |= MIOS32_BOARD_PinGet(DIN_DATA_PIN) << bitshift[bit];
    MIOS32_BOARD_PinSet(DIN_CLK_PIN, 1);  // Clock on rising edge
  }

  // Direct port bits
  mios32_p6_inbits |= MIOS32_BOARD_PinGet(S1_PIN) << S1_PINSHIFT;
  mios32_p6_inbits |= MIOS32_BOARD_PinGet(S4_PIN) << S4_PINSHIFT;
  mios32_p6_inbits |= MIOS32_BOARD_PinGet(S7_PIN) << S7_PINSHIFT;

  // Detect changes
  u32 changed = mios32_p6_inbits_prev ^ mios32_p6_inbits;
  mios32_p6_inbits_changed |= changed;
  mios32_p6_inbits_prev = mios32_p6_inbits;

  // update encoder states
  int enc;
  for (enc = 0; enc < NUM_ENCODERS; ++enc)
    MIOS32_ENC_StateSet(enc, (mios32_p6_inbits >> (16 + 2 * enc)) & 3);

  // '595 Output chain
  // We only have one '595, so we ouptut output 8 bits for LED2..9
  // Happily, the bits are in order, starting with bit 1 = LED2, etc
  // We don't use bit 0 at all, setting the corresponding LED1 is done
  // by directly controlling the I/O port, so that even if interrupts
  // are off we can signal something using LED1.
  for (bit = NUM_OUTBITS; bit >= 1; bit--) {
    MIOS32_BOARD_PinSet(DOUT_DATA_PIN, (mios32_p6_outbits >> bit) & 1);
    MIOS32_BOARD_PinSet(DOUT_CLK_PIN, 1); // Clock on rising edge
    MIOS32_BOARD_PinSet(DOUT_CLK_PIN, 0); 
  }
  MIOS32_BOARD_PinSet(DOUT_LATCH_PIN, 1); // Latch on rising edge
  MIOS32_BOARD_PinSet(DOUT_LATCH_PIN, 0); 

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//! Checks for pin changes, and calls given callback function with following parameters:
//! \code
//!   void DIN_NotifyToggle(u32 pin, u32 value)
//! \endcode
//! \param[in] _callback pointer to callback function
//! \return < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_P6_DIN_Handler(void *_callback)
{
  s32 bit;
  u32 changed;
  void (*callback)(u32 pin, u32 value) = _callback;

  changed = mios32_p6_inbits_changed;
  mios32_p6_inbits_changed = 0;

  for(bit = 0; bit < NUM_INBITS; ++bit)
    if (changed & (1 << bit)) {
      // call the notification function
      callback(bit, (mios32_p6_inbits & (1 << bit)) ? 1 : 0);
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
// On-Board LEDs
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//! Initializes LEDs of the board
//! \param[in] leds mask contains a flag for each LED which should be initialized<BR>
//! <UL>
//!   <LI>STM32F4DISCOVERY: 4 LEDs (flag 0: green, flag1: orange, flag2: red, flag3: blue)
//! </UL>
//! \return 0 if initialisation passed
//! \return -1 if no LEDs specified for board
//! \return -2 if one or more LEDs not available on board
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_LED_Init(u32 leds)
{
  if( leds & 1 ) {
    MIOS32_BOARD_PinInitHlp(GPIOC, GPIO_Pin_2, MIOS32_BOARD_PIN_MODE_OUTPUT_PP); // LED1
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//! Sets one or more LEDs to the given value(s)
//! \param[in] leds mask contains a flag for each LED which should be changed
//! \param[in] value contains the value which should be set
//! \return 0 if initialisation passed
//! \return -1 if no LEDs specified for board
//! \return -2 if one or more LEDs not available on board
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_LED_Set(u32 leds, u32 value)
{
  value &= leds; // Only allow changing of LEDs in mask
  if( leds & 1 ) { // LED1
    MIOS32_SYS_STM_PINSET(GPIOC, GPIO_Pin_2, value & 1);
  }
  if (leds & (((1 << NUM_INBITS) -1) << 1)) { // LED2..9
    MIOS32_IRQ_Disable();
    mios32_p6_outbits &= ~leds;
    mios32_p6_outbits |= value;
    MIOS32_IRQ_Enable();
  }
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
//! Returns the status of all LEDs
//! \return status of all LEDs
/////////////////////////////////////////////////////////////////////////////
u32 MIOS32_BOARD_LED_Get(void)
{
  u32 values = 0;

  if( GPIOC->ODR & GPIO_Pin_2 ) // LED1
    values |= 1;
  values |= mios32_p6_outbits;

  return values;
}


#if 0
/////////////////////////////////////////////////////////////////////////////
//! This function enables or disables one of the two DAC channels provided by 
//! STM32F103RE (and not by STM32F103RB).
//!
//! <UL>
//!  <LI>the first channel (chn == 0) is output at pin RA4 (J16:RC1 of the MBHP_CORE_STM32 module)
//!  <LI>the second channel (chn == 1) is output at pin RA5 (J16:SC of the MBHP_CORE_STM32 module)
//! </UL>
//! 
//! \param[in] chn channel number (0 or 1)
//! \param[in] enable 0: channel disabled, 1: channel enabled.
//! \return < 0 if DAC channel not supported (e.g. STM32F103RB)
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_DAC_PinInit(u8 chn, u8 enable)
{
#if defined(MIOS32_PROCESSOR_STM32F103RB)
  return -1; // generally not supported. Try DAC access for all other processors
#else
  if( chn >= 2 )
    return -1; // channel not supported

  if( enable ) {
    // enable DAC clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
    // connected to the DAC converter. In order to avoid parasitic consumption, 
    // the GPIO pin should be configured in analog
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    // init DAC
    DAC_InitTypeDef            DAC_InitStructure;
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;

    switch( chn ) {
      case 0:
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1, ENABLE);
	break;

      case 1:
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	break;

      default:
	return -2; // unexpected (chn already checked above)
    }
    
  } else {
    // disable DAC channel
    switch( chn ) {
      case 0: DAC_Cmd(DAC_Channel_1, DISABLE); break;
      case 1: DAC_Cmd(DAC_Channel_2, DISABLE); break;
      default:
	return -2; // unexpected (chn already checked above)
    }
  }

  return 0; // no error
#endif
}


/////////////////////////////////////////////////////////////////////////////
//! This function sets an output channel to a given 16-bit value.
//!
//! Note that actually the DAC will work at 12-bit resolution. The lowest
//! 4 bits are ignored (reserved for future STM chips).
//! \param[in] chn channel number (0 or 1)
//! \param[in] value the 16-bit value (0..65535). Lowest 4 bits are ignored.
//! \return < 0 if DAC channel not supported (e.g. STM32F103RB)
/////////////////////////////////////////////////////////////////////////////
s32 MIOS32_BOARD_DAC_PinSet(u8 chn, u16 value)
{
#if defined(MIOS32_PROCESSOR_STM32F103RB)
  return -1; // generally not supported. Try DAC access for all other processors
#else
  switch( chn ) {
    case 0:
      DAC_SetChannel1Data(DAC_Align_12b_L, value);
      DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
      break;

    case 1:
      DAC_SetChannel2Data(DAC_Align_12b_L, value);
      DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
      break;

    default:
      return -1; // channel not supported
  }

  return 0; // no error
#endif
}
#endif

//! \}
