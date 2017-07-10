/*
 * Simple driver for 24LC512 (512 kbit (64 kbyte)) I2C EEPROM.
 *
 * ==========================================================================
 *
 *  (C) 2017, Ricard Wanderlof, ricard2015@butoba.net
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <mios32.h>

#include "eeprom.h"

/////////////////////////////////////////////////////////////////////////////
// Local definitions
/////////////////////////////////////////////////////////////////////////////

// EEPROM I2C slave address (high nybble only, low nybble set by address pins).
#define EEPROM_SA 0xA0

// Block mode definitions

#ifdef EEPROM_24LC512

#define PAGESIZE 128 // bytes, from datasheet
#define TOTAL_SIZE 65536 // bytes, from datasheet

#endif

#if !defined(PAGESIZE) || !defined(TOTAL_SIZE)
#error No EEPROM type defined (PAGESIZE and TOTAL_SIZE not set)
#endif

#if (EEPROM_BLOCKSIZE > PAGESIZE)
#error EEPROM_BLOCKSIZE is larger than page size of EEPROM used
#endif

/////////////////////////////////////////////////////////////////////////////
// Local types
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Local variables
/////////////////////////////////////////////////////////////////////////////

static u8 transfer_buf[EEPROM_BLOCKSIZE + 2];

/////////////////////////////////////////////////////////////////////////////
// Local prototypes
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Initializes the EEPROM driver (IIC & GPIO pins)
// IN: mode - currently only 0 supported
// OUT: 0: success; -1 error (IIC initialization failed)
//                  -2 mode not supported
/////////////////////////////////////////////////////////////////////////////
s32 EEPROM_Init(u8 mode)
{
  if( mode > 0)
    return -2;

  return MIOS32_IIC_Init(0);
}

/////////////////////////////////////////////////////////////////////////////
// Reads data from EEPROM (block mode)
// IN: device_addr: EEPROM device address (0..7)
//     block_no: EEPROM block number
//     buffer: pointer to data buffer
//     buffer_len: count bytes to read
// OUT: 0 on success, < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 EEPROM_BlockRead(u8 device_addr, u16 block_no, u8 *buffer, u16 buffer_len)
{
  return EEPROM_Read(device_addr, block_no * EEPROM_BLOCKSIZE, buffer, buffer_len);
}

/////////////////////////////////////////////////////////////////////////////
// Writes data to EEPROM (block mode).
// Data must be at the maximum one page in length and may not cross a page
// boundary.
// IN: device_addr: EEPROM device address (0..7)
//     mem_addr: EEPROM write start address
//     buffer: pointer to data buffer
//     buffer_len: count bytes to write
// OUT: 0 on success, < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 EEPROM_BlockWrite(u8 device_addr, u16 block_no, u8 *buffer, u16 buffer_len)
{
  u16 eeprom_addr = block_no * EEPROM_BLOCKSIZE;

  if (buffer_len > EEPROM_BLOCKSIZE)
    return -2;

  if (eeprom_addr + buffer_len > TOTAL_SIZE)
    return -2;

  return EEPROM_Write(device_addr, eeprom_addr, buffer, buffer_len);
}

/////////////////////////////////////////////////////////////////////////////
// Reads data from EEPROM (blocking).
// IN: device_addr: EEPROM device address (0..7)
//     mem_addr: EEPROM start address
//     buffer: pointer to data buffer
//     buffer_len: count bytes to read
// OUT: 0 on success, < 0 on errors
/////////////////////////////////////////////////////////////////////////////
s32 EEPROM_Read(u8 device_addr, u16 mem_addr, u8 *buffer, u16 buffer_len)
{
  u32 res;
  u8 addr_buf[2];

  MIOS32_IIC_TransferBegin(EEPROM_IIC_PORT, IIC_Blocking);

  device_addr = EEPROM_SA | ( (device_addr & 0x07 ) << 1 );
  addr_buf[0] = mem_addr >> 8;
  addr_buf[1] = mem_addr & 0xff;

  // Write address
  res = MIOS32_IIC_Transfer(EEPROM_IIC_PORT, IIC_Write_WithoutStop, device_addr, &addr_buf[0], 2);
  // Wait for transfer to complete
  if (res == 0) res = MIOS32_IIC_TransferWait(EEPROM_IIC_PORT);
  // read data
  if( res == 0 && buffer_len ){
    res = MIOS32_IIC_Transfer(EEPROM_IIC_PORT, IIC_Read, device_addr, buffer, buffer_len);
    if (res == 0)
      res = MIOS32_IIC_TransferWait(EEPROM_IIC_PORT);
  }

  MIOS32_IIC_TransferFinished(EEPROM_IIC_PORT);

  return res;
}

/////////////////////////////////////////////////////////////////////////////
// Writes data to EEPROM (blocking mode).
// Data must be at the maximum one page in length and may not cross a page
// boundary.
// IN: device_addr: EEPROM device address (0..7)
//     mem_addr: EEPROM write start address
//     buffer: pointer to data buffer
//     buffer_len: count bytes to write
// OUT: 0 on success, < 0 on errors
//      (see README for error codes)
/////////////////////////////////////////////////////////////////////////////
s32 EEPROM_Write(u8 device_addr, u16 mem_addr, u8 *buffer, u16 buffer_len)
{
  u32 res;

  MIOS32_IIC_TransferBegin(EEPROM_IIC_PORT, IIC_Blocking);

  device_addr = EEPROM_SA | ( (device_addr & 0x07 ) << 1 );
  transfer_buf[0] = mem_addr >> 8;
  transfer_buf[1] = mem_addr & 0xff;
  memcpy(&transfer_buf[2], buffer, buffer_len);

  // Write address and data
  res = MIOS32_IIC_Transfer(EEPROM_IIC_PORT, IIC_Write, device_addr, &transfer_buf[0], buffer_len + 2);
  // Wait for transfer to complete
  if (res == 0) res = MIOS32_IIC_TransferWait(EEPROM_IIC_PORT);

  MIOS32_IIC_TransferFinished(EEPROM_IIC_PORT);

  return res;
}
