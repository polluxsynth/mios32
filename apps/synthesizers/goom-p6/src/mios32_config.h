// $Id: mios32_config.h 2178 2015-05-25 11:25:29Z tk $
/*
 * Local MIOS32 configuration file
 *
 * this file allows to disable (or re-configure) default functions of MIOS32
 * available switches are listed in $MIOS32_PATH/modules/mios32/MIOS32_CONFIG.txt
 *
 */

#ifndef _MIOS32_CONFIG_H
#define _MIOS32_CONFIG_H

// The boot message which is print during startup and returned on a SysEx query
#define MIOS32_LCD_BOOT_MSG_LINE1 "Goom synth"
#define MIOS32_LCD_BOOT_MSG_LINE2 "Startup"

#define MIOS32_DONT_USE_BOARD // Use p6_board.c instead

#define MIOS32_DONT_USE_SRIO
#define MIOS32_NUM_SR 3 // 3x HC165's

#define MIOS32_DONT_USE_AIN

// I2S support has to be enabled explicitely
#define MIOS32_USE_I2S

#define MIOS32_I2S_MCLK_ENABLE  1

// Sample rate: use the same like in the original Goom project
// (72 MHz / 2048)
#define MIOS32_I2S_AUDIO_FREQ  35156

#endif /* _MIOS32_CONFIG_H */
