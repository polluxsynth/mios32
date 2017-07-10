/*
 * Header file for eeprom.c
 *
 * ==========================================================================
 *
 * Copyright (C) 2017, Ricard Wanderlof (ricard2015@butoba.net)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

#ifndef _EEPROM_H
#define _EEPROM_H

/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

#include <mios32_config.h>

#ifndef EEPROM_IIC_PORT
#define EEPROM_IIC_PORT 1
#endif

// Define EEPROM type here for now. This sets total device and page sizes.
#define EEPROM_24LC512

// Size of the block of data we want to work with
#ifndef EEPROM_BLOCKSIZE
#define EEPROM_BLOCKSIZE 64
#endif

/////////////////////////////////////////////////////////////////////////////
// Global Types
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

s32 EEPROM_Init(u8 mode);

s32 EEPROM_Read(u8 device_addr, u16 mem_addr, u8 *buffer, u16 buffer_len);
s32 EEPROM_Write(u8 device_addr, u16 mem_addr, u8 *buffer, u16 buffer_len);

s32 EEPROM_BlockRead(u8 device_addr, u16 block_no, u8 *buffer, u16 buffer_len);
s32 EEPROM_BlockWrite(u8 device_addr, u16 block_no, u8 *buffer, u16 buffer_len);

#endif /* _EEPROM_H */
