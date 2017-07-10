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
#define MIOS32_LCD_BOOT_MSG_LINE2 "Start"

#define MIOS32_DONT_USE_BOARD // Use p6_board.c instead

#define MIOS32_DONT_USE_SRIO
#define MIOS32_DONT_USE_DIN
#define MIOS32_DONT_USE_DOUT

#define MIOS32_DONT_USE_AIN

// I2S support has to be enabled explicitely
#define MIOS32_USE_I2S

#define MIOS32_I2S_MCLK_ENABLE  1

// Use MIOS32 Banksticks interface for accessing P6 on-board EEPROM
#define MIOS32_IIC_BS_PORT 1
#define MIOS32_IIC_BS_NUM 1 // one EEPROM available

// Sample rate: use the same like in the original Goom project
// (72 MHz / 2048)
#define MIOS32_I2S_AUDIO_FREQ  35156

#endif /* _MIOS32_CONFIG_H */
