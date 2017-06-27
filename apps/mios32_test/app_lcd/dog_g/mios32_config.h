// $Id: mios32_config.h 775 2009-11-14 18:42:41Z tk $
/*
 * Local MIOS32 configuration file
 *
 * this file allows to disable (or re-configure) default functions of MIOS32
 * available switches are listed in $MIOS32_PATH/modules/mios32/MIOS32_CONFIG.txt
 *
 */

#ifndef _MIOS32_CONFIG_H
#define _MIOS32_CONFIG_H

// supported: 1..2 (see comments in app_lcd.c of pcd8544 driver)
#define APP_LCD_NUM_X 2

// supported: 2..4 (see comments in app_lcd.c of pcd8544 driver)
#define APP_LCD_NUM_Y 4


#endif /* _MIOS32_CONFIG_H */
