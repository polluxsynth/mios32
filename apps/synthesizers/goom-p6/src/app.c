// $Id: app.c 2172 2015-05-15 16:48:50Z tk $
/*
 * Goom Synth
 *
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>
#include "app.h"
#include "eeprom.h"
#include "synth.h"
#include "param.h"

#include <FreeRTOS.h>
#include <portmacro.h>

#include <p6_board.h>

/////////////////////////////////////////////////////////////////////////////
// Local definitions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

static u32 pin_status;

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// This hook is called after startup to initialize the application
/////////////////////////////////////////////////////////////////////////////
void APP_Init(void)
{
  // initialize all LEDs
  MIOS32_BOARD_LED_Init(0xffffffff);

  // init EEPROM driver
  EEPROM_Init(0);

  // init P6 I/O scan
  MIOS32_BOARD_P6_ScanInit();

  // init Synth
  SYNTH_Init(0);

  // init Param handler
  PARAM_Init(0);
}


/////////////////////////////////////////////////////////////////////////////
// This task is running endless in background
/////////////////////////////////////////////////////////////////////////////
void APP_Background(void)
{
  u32 pin_changed;
  static u32 old_pin_status;

  MIOS32_IRQ_Disable(); // atomic
  pin_changed = pin_status ^ old_pin_status; // buttons changed (up or down)
  old_pin_status = pin_status; // save pin status for next time
  MIOS32_IRQ_Enable();

  if (pin_changed & 0x1ff) // only consider buttons 1..9
    // We use old_pin_status here to get the status that the pin_changed vector
    // actually refers to. Otherwise, pin_changed might have changed status
    // between the calculation of pin_changed above and its usage here.
    PARAM_ButtonHandle(old_pin_status & 0x1ff);

//  u32 timestamp = MIOS32_TIMESTAMP_Get();
//  if ((timestamp & 0x3ff) == 0)
//MIOS32_MIDI_SendDebugMessage("%s: SR2: %04x (%sBUSY)\n", __func__, I2C1->SR2, I2C1->SR2 & 0002 ? "" : "NOT ");
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called each mS from the main task which also handles DIN, ENC
// and AIN events. You could add more jobs here, but they shouldn't consume
// more than 300 uS to ensure the responsiveness of buttons, encoders, pots.
// Alternatively you could create a dedicated task for application specific
// jobs as explained in $MIOS32_PATH/apps/tutorials/006_rtos_tasks
/////////////////////////////////////////////////////////////////////////////
void APP_Tick(void)
{
  // PWM modulate the status LED (this is a sign of life)
  //u32 timestamp = MIOS32_TIMESTAMP_Get();
  //MIOS32_BOARD_LED_Set(1, (timestamp % 20) <= ((timestamp / 100) % 10));

  // run P6 I/O scan
  MIOS32_BOARD_P6_ScanStart();
  MIOS32_ENC_UpdateStates();

  // Process I/O changes
  MIOS32_BOARD_P6_DIN_Handler(APP_DIN_NotifyToggle);
  MIOS32_ENC_Handler(APP_ENC_NotifyChange);

  // update synth
  SYNTH_Update_1mS();
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called each mS from the MIDI task which checks for incoming
// MIDI events. You could add more MIDI related jobs here, but they shouldn't
// consume more than 300 uS to ensure the responsiveness of incoming MIDI.
/////////////////////////////////////////////////////////////////////////////
void APP_MIDI_Tick(void)
{
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a MIDI package has been received
/////////////////////////////////////////////////////////////////////////////
void APP_MIDI_NotifyPackage(mios32_midi_port_t port, mios32_midi_package_t midi_package)
{
  // -> synth
  SYNTH_MIDI_NotifyPackage(port, midi_package);
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called before the shift register chain is scanned
/////////////////////////////////////////////////////////////////////////////
void APP_SRIO_ServicePrepare(void)
{
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called after the shift register chain has been scanned
/////////////////////////////////////////////////////////////////////////////
void APP_SRIO_ServiceFinish(void)
{
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a button has been toggled
// On P6, pin_value is 0 when button released, and 1 when button pressed
/////////////////////////////////////////////////////////////////////////////
void APP_DIN_NotifyToggle(u32 pin, u32 pin_value)
{
  // Don't do much here; we don't want to call parameter handler at this
  // point as we don't want to load the interrupt we're running off of with
  // any slow code, such as LCD writes. So just update the file global
  // pin_status and let the background task handle the actual work.
  if (pin_value)
    pin_status |= 1 << pin;
  else
    pin_status &= ~(1 << pin);
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when an encoder has been moved
// incrementer is positive when encoder has been turned clockwise, else
// it is negative
/////////////////////////////////////////////////////////////////////////////
void APP_ENC_NotifyChange(u32 encoder, s32 incrementer)
{
  PARAM_EncoderChange(encoder, incrementer);
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a pot has been moved
/////////////////////////////////////////////////////////////////////////////
void APP_AIN_NotifyChange(u32 pin, u32 pin_value)
{
}
