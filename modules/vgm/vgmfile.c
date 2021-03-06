/*
 * VGM Data and Playback Driver: VGM File Load/Save
 *
 * ==========================================================================
 *
 *  Copyright (C) 2017 Sauraen (sauraen@gmail.com)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */


#include "vgmfile.h"
#include "vgmplayer.h"
#include "vgmsdtask.h"
#include "vgmstream.h"
#include "vgm_heap2.h"
#include <string.h>


static void BufferRead(u8* cmdbuf, u32 bytes, u32* a, u32* bufstart, u8* buf){
    u32 b = 0;
    while(1){
        while(*a - *bufstart < VGM_SOURCESTREAM_BUFSIZE){
            cmdbuf[b++] = buf[(*a)++ - *bufstart];
            if(b >= bytes) return;
        }
        FILE_ReadSeek(*a);
        FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
        *bufstart = *a;
    }
}
static void BufferSkip(u32 bytes, u32* a, u32* bufstart, u8* buf){
    *a += bytes;
    if(*a - *bufstart >= VGM_SOURCESTREAM_BUFSIZE){
        FILE_ReadSeek(*a);
        FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE); 
        *bufstart = *a;
    }
}
static inline u32 ReadLittleEndianU32(u8* buf, u32 addr){
    return (u32)buf[addr+0] 
        | ((u32)buf[addr+1] << 8) 
        | ((u32)buf[addr+2] << 16) 
        | ((u32)buf[addr+3] << 24);
}



s32 VGM_File_ScanFile(char* filename, VgmFileMetadata* md){
    //Init
    md->filesize = 0;
    md->numcmds = 0;
    md->numcmdsram = 0;
    md->numblocks = 0;
    md->totalblocksize = 0;
    md->usage.all = 0;
    md->vgmdatastartaddr = 0;
    md->loopaddr = 0;
    md->loopsamples = 0;
    md->psgclock = 0;
    md->psgfreq0to1 = 1;
    md->opn2clock = 0;
    //Open file
    MUTEX_SDCARD_TAKE;
    s32 res = FILE_ReadOpen(&md->file, filename);
    if(res < 0) goto Error_Opening;
    md->filesize = FILE_ReadGetCurrentSize();
    if(md->filesize <= 0x40){
        DBG("File too small to have VGM header!");
        goto Error_Filesize;
    }
    u8* buf = malloc(0x40); //DMA target, have to use normal malloc
    res = FILE_ReadBuffer(buf, 0x40);
    if(res < 0) goto Error_withBufferOpen;
    if(buf[0] != 'V' || buf[1] != 'g' || buf[2] != 'm' || buf[3] != ' '){
        DBG("File doesn't have magic \"Vgm \" tag!");
        goto Error_withBufferOpen;
    }
    //Read header data
    u8 ver_lo = buf[8];
    u8 ver_hi = buf[9];
    md->psgclock = ReadLittleEndianU32(buf, 0x0C);
    md->loopaddr = ReadLittleEndianU32(buf, 0x1C) + 0x1C;
    md->loopsamples = ReadLittleEndianU32(buf, 0x20);
    md->opn2clock = ReadLittleEndianU32(buf, 0x2C);
    md->psgfreq0to1 = (~buf[0x2B]) & 1;
    if(md->opn2clock == 0) md->psgfreq0to1 = 0; //If there's no OPN2, it's probably a SMS
    u32 a;
    if(ver_hi < 1 || (ver_hi == 1 && ver_lo < 0x50)){
        a = 0x40;
    }else{
        a = ReadLittleEndianU32(buf, 0x34) + 0x34;
    }
    md->vgmdatastartaddr = a;
    free(buf);
    //Scan entire file for data blocks and usage
    u32 bufstart;
    u8 type, len;
    u32 thisblocksize;
    u8 cmdbuf[4];
    VgmChipWriteCmd cmd;
    buf = malloc(VGM_SOURCESTREAM_BUFSIZE); //DMA target, have to use normal malloc
    FILE_ReadSeek(a); FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
    bufstart = a;
    while(a < md->filesize){
        BufferRead(&type, 1, &a, &bufstart, buf);
        ++md->numcmds;
        if(type == 0x67){
            //Data block
            ++md->numblocks;
            BufferSkip(2, &a, &bufstart, buf); //Skip 0x66 0x00
            BufferRead(cmdbuf, 4, &a, &bufstart, buf);
            thisblocksize = ReadLittleEndianU32(cmdbuf, 0);
            if(thisblocksize + a >= md->filesize){
                DBG("VGM_File_LoadRAM error: block %d bytes @%d, file only %d bytes!", thisblocksize, a, md->filesize);
                res = -51;
                break;
            }
            md->totalblocksize += thisblocksize;
            BufferSkip(thisblocksize, &a, &bufstart, buf);
        }else if(type == 0x66){
            //End of stream
            break;
        }else if(type == 0x50){
            //PSG write
            BufferRead(cmdbuf, 1, &a, &bufstart, buf);
            cmd.cmd = type;
            cmd.data = cmdbuf[0];
            VGM_Cmd_UpdateUsage(&md->usage, cmd);
            if(cmd.data & 0x80) ++md->numcmdsram; //Don't count second halves of freq writes
        }else if((type & 0xFE) == 0x52){
            //OPN2 write
            BufferRead(cmdbuf, 2, &a, &bufstart, buf);
            cmd.cmd = type;
            cmd.addr = cmdbuf[0];
            cmd.data = cmdbuf[1];
            VGM_Cmd_UpdateUsage(&md->usage, cmd);
            if((cmd.addr & 0xF4) != 0xA0) ++md->numcmdsram; //Don't count second halves of freq writes
        }else if(type >= 0x80 && type <= 0x8F){
            //DAC and wait
            cmd.cmd = type;
            VGM_Cmd_UpdateUsage(&md->usage, cmd);
            ++md->numcmdsram;
        }else if((type >= 0x70 && type <= 0x7F) || type == 0x62 || type == 0x63){
            //Short wait, 60 Hz wait, or 50 Hz wait
            ++md->numcmdsram;
        }else if(type == 0x61){
            //Long wait
            BufferSkip(2, &a, &bufstart, buf);
            ++md->numcmdsram;
        }else{
            len = VGM_Cmd_GetCmdLen(type);
            BufferSkip(len, &a, &bufstart, buf);
        }
    }
    if(type != 0x66){
        DBG("VGM_File_ScanFile ran off end of file!");
        res = -52;
    }
Error_withBufferOpen:
    free(buf);
Error_Filesize:
    FILE_ReadClose(&md->file);
Error_Opening:
    MUTEX_SDCARD_GIVE;
    if(res < 0){
        DBG("VGM_File_ScanFile error %d on %s", res, filename);
    }else{
        DBG("VGM_File_ScanFile successful on %s (%d bytes):", filename, md->filesize);
        DBG("--%d cmds, %d cmds for RAM, %d blocks, %d total block size", 
            md->numcmds, md->numcmdsram, md->numblocks, md->totalblocksize);
        DBG("--VGM data starts at 0x%X, loop at 0x%X, loop %d samples",
            md->vgmdatastartaddr, md->loopaddr, md->loopsamples);
        DBG("--PSG clock %d, OPN2 clock %d", md->psgclock, md->opn2clock);
        VGM_Cmd_DebugPrintUsage(md->usage);
    }
    return res;
}


s32 VGM_File_StartStream(VgmSource* sourcestream, char* filepath, VgmFileMetadata* md){
    if(sourcestream == NULL) return -100;
    if(sourcestream->type != VGM_SOURCE_TYPE_STREAM) return -100;
    VgmSourceStream* vss = (VgmSourceStream*)sourcestream->data;
    //Copy data from metadata to source/sourcestream
    sourcestream->psgclock = md->psgclock;
    sourcestream->opn2clock = md->opn2clock;
    sourcestream->psgfreq0to1 = md->psgfreq0to1;
    sourcestream->loopaddr = md->loopaddr;
    sourcestream->loopsamples = md->loopsamples;
    sourcestream->usage.all = md->usage.all;
    vss->file = md->file;
    vss->datalen = md->filesize;
    vss->vgmdatastartaddr = md->vgmdatastartaddr;
    vss->blocklen = md->totalblocksize;
    //Copy filepath
    u8 len = strlen(filepath);
    vss->filepath = vgmh2_malloc(len+1);
    memcpy(vss->filepath, filepath, len);
    vss->filepath[len] = 0;
    //Allocate memory for block
    if(!vss->blocklen) return 0; //No blocks, done!
    vss->block = malloc(vss->blocklen);
    if(vss->block == NULL) return -50;
    //Open file
    MUTEX_SDCARD_TAKE;
    s32 res = FILE_ReadReOpen(&vss->file);
    if(res < 0) goto Error_Opening;
    //Load data blocks from file
    u32 blockaddr = 0;
    u32 a = vss->vgmdatastartaddr;
    u32 bufstart;
    u16 blockcount = 0;
    u8 type;
    u32 thisblocksize;
    u8 cmdbuf[4];
    u8* buf = malloc(VGM_SOURCESTREAM_BUFSIZE); //DMA target, have to use normal malloc
    FILE_ReadSeek(a); FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
    bufstart = a;
    while(a < md->filesize){
        BufferRead(&type, 1, &a, &bufstart, buf);
        if(type == 0x67){
            //Data block
            BufferSkip(2, &a, &bufstart, buf); //Skip 0x66 0x00
            BufferRead(cmdbuf, 4, &a, &bufstart, buf);
            thisblocksize = ReadLittleEndianU32(cmdbuf, 0);
            if(thisblocksize + a >= md->filesize){
                DBG("VGM_File_LoadRAM error: block %d bytes @%d, file only %d bytes!", thisblocksize, a, md->filesize);
                res = -51;
                break;
            }
            //Load actual block
            FILE_ReadSeek(a);
            FILE_ReadBuffer((u8*)(vss->block + blockaddr), thisblocksize);
            blockaddr += thisblocksize;
            //See if we're done
            ++blockcount;
            if(blockcount >= md->numblocks) break;
            //Restore our buffer
            a += thisblocksize;
            FILE_ReadSeek(a);
            FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
            bufstart = a;
        }else if(type == 0x66){
            //End of stream
            break;
        }else{
            len = VGM_Cmd_GetCmdLen(type);
            BufferSkip(len, &a, &bufstart, buf);
        }
    }
    if(blockcount < md->numblocks){
        DBG("VGM_File_StartStream error: couldn't find enough blocks!");
        res = -52;
    }else if(blockaddr < md->totalblocksize){
        DBG("VGM_File_StartStream error: loaded blocks not long enough!");
        res = -53;
    }
    DBG("Loaded %d of %d block bytes", blockaddr, vss->blocklen);
    free(buf);
    FILE_ReadClose(&vss->file);
Error_Opening:
    MUTEX_SDCARD_GIVE;
    return res;
}

s32 VGM_File_LoadRAM(VgmSource* sourceram, VgmFileMetadata* md){
    if(sourceram == NULL) return -100;
    if(sourceram->type != VGM_SOURCE_TYPE_RAM) return -100;
    DBG("VGM_File_LoadRAM");
    VgmSourceRAM* vsr = (VgmSourceRAM*)sourceram->data;
    if(vsr->cmds != NULL){
        DBG("--Deleting all existing commands");
        vgmh2_free(vsr->cmds);
        vsr->cmds = NULL;
        vsr->numcmds = 0;
    }
    //Copy data from metadata to source/sourceram
    sourceram->psgclock = md->psgclock;
    sourceram->opn2clock = md->opn2clock;
    sourceram->psgfreq0to1 = md->psgfreq0to1;
    sourceram->loopaddr = md->loopaddr;
    sourceram->loopsamples = md->loopsamples;
    sourceram->usage.all = md->usage.all;
    vsr->numcmds = md->numcmdsram;
    vsr->cmds = vgmh2_malloc(vsr->numcmds * sizeof(VgmChipWriteCmd));
    if(vsr->cmds == NULL){
        DBG("VGM_File_LoadRAM out of memory for main data!");
        return -50;
    }
    //Open file
    MUTEX_SDCARD_TAKE;
    s32 res = FILE_ReadReOpen(&md->file);
    if(res < 0) goto Error_Opening;
    //Variables
    u32 a, c, bufstart, blockcount, blocksize, blockaddr;
    u8 type, len;
    u8 cmdbuf[4];
    u8* buf = malloc(VGM_SOURCESTREAM_BUFSIZE); //DMA target, have to use normal malloc
    //Temporarily load block data
    u8* block = NULL;
    if(md->totalblocksize > 0){
        block = malloc(md->totalblocksize);
        if(block == NULL){
            DBG("VGM_File_LoadRAM out of memory for temporary block data!");
            res = -50;
            goto Error_Inside;
        }
        blockaddr = 0;
        a = md->vgmdatastartaddr;
        blockcount = 0;
        FILE_ReadSeek(a); FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
        bufstart = a;
        while(a < md->filesize){
            BufferRead(&type, 1, &a, &bufstart, buf);
            if(type == 0x67){
                //Data block
                BufferSkip(2, &a, &bufstart, buf); //Skip 0x66 0x00
                BufferRead(cmdbuf, 4, &a, &bufstart, buf);
                blocksize = ReadLittleEndianU32(cmdbuf, 0);
                if(blocksize + a >= md->filesize){
                    DBG("VGM_File_LoadRAM error: block %d bytes @%d, file only %d bytes!", blocksize, a, md->filesize);
                    res = -51;
                    break;
                }
                //Load actual block
                FILE_ReadSeek(a);
                FILE_ReadBuffer((u8*)(block + blockaddr), blocksize);
                blockaddr += blocksize;
                //See if we're done
                ++blockcount;
                if(blockcount >= md->numblocks) break;
                //Restore our buffer
                a += blocksize;
                FILE_ReadSeek(a);
                FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
                bufstart = a;
            }else if(type == 0x66){
                //End of stream
                break;
            }else{
                len = VGM_Cmd_GetCmdLen(type);
                BufferSkip(len, &a, &bufstart, buf);
            }
        }
        if(blockcount < md->numblocks){
            DBG("VGM_File_LoadRAM error: couldn't find enough blocks!");
            res = -52;
            goto Error_Inside;
        }else if(blockaddr < md->totalblocksize){
            DBG("VGM_File_LoadRAM error: loaded blocks not long enough!");
            res = -53;
            goto Error_Inside;
        }
    }
    //Load VGM data
    a = md->vgmdatastartaddr;
    c = 0;
    blockaddr = 0;
    s32 lastpsgfreqwrite = -1, lastopn2freqwrite = -1;
    FILE_ReadSeek(a); FILE_ReadBuffer(buf, VGM_SOURCESTREAM_BUFSIZE);
    bufstart = a;
    while(a < md->filesize && c < vsr->numcmds){
        BufferRead(&type, 1, &a, &bufstart, buf);
        if(type == 0x67){
            //Data block
            BufferSkip(2, &a, &bufstart, buf); //Skip 0x66 0x00
            BufferRead(cmdbuf, 4, &a, &bufstart, buf);
            blocksize = ReadLittleEndianU32(cmdbuf, 0);
            BufferSkip(blocksize, &a, &bufstart, buf);
        }else if(type == 0xE0){
            //Seek in block
            BufferRead(cmdbuf, 4, &a, &bufstart, buf);
            u32 temp = ReadLittleEndianU32(cmdbuf, 0);
            blockaddr = temp;
        }else if(type == 0x66){
            //End of stream
            break;
        }else if(type == 0x50){
            //PSG write
            BufferRead(cmdbuf, 1, &a, &bufstart, buf);
            if(!(cmdbuf[0] & 0x80)){
                //Second half of freq write
                if(lastpsgfreqwrite < 0){
                    DBG("VGM_File_LoadRAM error: second half of PSG freq write with no first half!");
                }else{
                    vsr->cmds[lastpsgfreqwrite].data2 = cmdbuf[0];
                    lastpsgfreqwrite = -1;
                }
            }else{
                vsr->cmds[c].all = 0;
                vsr->cmds[c].cmd = type;
                vsr->cmds[c].data = cmdbuf[0];
                if(!(cmdbuf[0] & 0x10) && (cmdbuf[0] < 0xE0)){
                    //First half of freq write
                    if(lastpsgfreqwrite >= 0){
                        DBG("VGM_File_LoadRAM error: two first-half-PSG-freq-writes in a row!");
                    }
                    lastpsgfreqwrite = c;
                }
                ++c;
            }
        }else if((type & 0xFE) == 0x52){
            //OPN2 write
            BufferRead(cmdbuf, 2, &a, &bufstart, buf);
            if((cmdbuf[0] & 0xF4) == 0xA0){
                //Second half of freq write
                if(lastopn2freqwrite < 0){
                    DBG("VGM_File_LoadRAM error: second half of OPN2 freq write with no first half!");
                }else{
                    vsr->cmds[lastopn2freqwrite].data2 = cmdbuf[0];
                    lastopn2freqwrite = -1;
                }
            }else{
                vsr->cmds[c].all = 0;
                vsr->cmds[c].cmd = type;
                vsr->cmds[c].addr = cmdbuf[0];
                vsr->cmds[c].data = cmdbuf[1];
                if((cmdbuf[0] & 0xF4) == 0xA4){
                    //First half of freq write
                    if(lastopn2freqwrite >= 0){
                        DBG("VGM_File_LoadRAM error: two first-half-OPN2-freq-writes in a row!");
                    }
                    lastopn2freqwrite = c;
                }
                ++c;
            }
        }else if(type >= 0x80 && type <= 0x8F){
            //DAC and wait
            vsr->cmds[c].all = 0;
            vsr->cmds[c].cmd = type;
            vsr->cmds[c].addr = 0x2A;
            //Get block byte
            if(blockaddr < md->totalblocksize){
                vsr->cmds[c].data = block[blockaddr];
                ++blockaddr;
            }else{
                DBG("VGM_File_LoadRAM error: DAC-and-wait ran off end of block!");
                vsr->cmds[c].data = 0x80; //DAC zero
            }
            ++c;
        }else if((type >= 0x70 && type <= 0x7F) || type == 0x62 || type == 0x63){
            //Short wait, 60 Hz wait, or 50 Hz wait
            vsr->cmds[c].all = 0;
            vsr->cmds[c].cmd = type;
            ++c;
        }else if(type == 0x61){
            //Long wait
            BufferRead(cmdbuf, 2, &a, &bufstart, buf);
            vsr->cmds[c].all = 0;
            vsr->cmds[c].cmd = type;
            vsr->cmds[c].data = cmdbuf[0];
            vsr->cmds[c].data2 = cmdbuf[1];
            ++c;
        }else{
            len = VGM_Cmd_GetCmdLen(type);
            BufferSkip(len, &a, &bufstart, buf);
        }
    }
    if(c < vsr->numcmds){
        DBG("VGM_File_LoadRAM error: two few commands loaded!");
    }
    //Done
Error_Inside:
    if(block != NULL) free(block);
    free(buf);
    FILE_ReadClose(&md->file);
Error_Opening:
    MUTEX_SDCARD_GIVE;
    return res;
}

s32 VGM_File_SaveRAM(VgmSource* sourceram, char* filename){
    if(sourceram == NULL) return -100;
    if(sourceram->type != VGM_SOURCE_TYPE_RAM) return -100;
    DBG("VGM_File_SaveRAM file %s", filename);
    VgmSourceRAM* vsr = (VgmSourceRAM*)sourceram->data;
    s32 res;
    if(FILE_FileExists(filename)){
        DBG("--Deleting existing file");
        if((res = FILE_Remove(filename)) < 0) return res;
    }
    //===============================Scan data==================================
    u32 datalen = 0, blocklen = 0, t = 0, i;
    u8 type;
    VgmChipWriteCmd cmd;
    for(i=0; i<vsr->numcmds; ++i){
        cmd = vsr->cmds[i];
        type = cmd.cmd;
        if(type == 0x50){
            //PSG write
            datalen += 2;
            if((cmd.data & 0x80) && !(cmd.data & 0x10) && (cmd.data < 0xE0)){
                //It's a main write, not attenuation, and not noise--i.e. frequency
                datalen += 2;
            }
        }else if((type & 0xFE) == 0x52){
            //OPN2 write
            datalen += 3;
            if((cmd.addr & 0xF4) == 0xA4){
                //Frequency MSB write
                datalen += 3;
            }
        }else if(type >= 0x80 && type <= 0x8F){
            //OPN2 DAC write
            ++datalen;
            ++blocklen;
            t += type - 0x80;
        }else if(type >= 0x70 && type <= 0x7F){
            //Short wait
            ++datalen;
            t += type - 0x6F;
        }else if(type == 0x61){
            //Long wait
            datalen += 3;
            t += cmd.data | ((u32)cmd.data2 << 8);
        }else if(type == 0x62){
            //60 Hz wait
            ++datalen;
            t += VGM_DELAY62;
        }else if(type == 0x63){
            //50 Hz wait
            ++datalen;
            t += VGM_DELAY63;
        }else{
            //Unsupported command, should not be here
        }
    }
    ++datalen; //0x66 End of Data command
    if(blocklen > 0){
        datalen += blocklen + 12; //for block, block setup command, and block pointer reset
    }
    DBG("--Data length %d (plus header), block length %d, total time %d", datalen, blocklen, t);
    //===============================Open file==================================
    if((res = FILE_UpdateFreeBytes()) < 0) return res;
    if(FILE_VolumeBytesFree() < datalen + 0x40) return FILE_ERR_WRITECOUNT;
    MUTEX_SDCARD_TAKE;
    FILE_WriteOpen(filename, 1);
    //==============================Write header================================
    //"Vgm "
    FILE_WriteBuffer((u8*)"Vgm ", 4);
    //Length
    FILE_WriteWord(datalen + 0x3C); //plus header, minus current position
    //Version
    FILE_WriteWord(0x00000151);
    //SN76489 clock
    FILE_WriteWord(sourceram->psgclock);
    //YM2413 clock, GD3 offset
    FILE_WriteWord(0);
    FILE_WriteWord(0);
    //Total number of samples
    FILE_WriteWord(t);
    //Loop offset and number of samples
    FILE_WriteWord(sourceram->loopaddr);
    FILE_WriteWord(sourceram->loopsamples);
    //Rate--not used
    FILE_WriteWord(0);
    //SN76489 feedback and shift register width
    FILE_WriteHWord(0x0003); //Discrete SN76489AN, which is correct for the synth,
    FILE_WriteByte(15); //but may be incorrect if this VGM was loaded
    //SN76489 flags
    FILE_WriteByte((~sourceram->psgfreq0to1) & 1); //May be incorrect
    //YM2612 clock
    FILE_WriteWord(sourceram->opn2clock);
    //YM2151 clock
    FILE_WriteWord(0);
    //VGM data offset
    FILE_WriteWord(0x0000000C); //@0x40
    //Sega PCM clock and interface register
    FILE_WriteWord(0);
    FILE_WriteWord(0);
    //===============================Write block================================
    if(blocklen > 0){
        DBG("--Writing block");
        //Block command
        FILE_WriteByte(0x67);
        FILE_WriteByte(0x66);
        FILE_WriteByte(0x00);
        FILE_WriteWord(blocklen);
        for(i=0; i<vsr->numcmds; ++i){
            //Write block data
            cmd = vsr->cmds[i];
            type = cmd.cmd;
            if(type >= 0x80 && type <= 0x8F){
                FILE_WriteByte(cmd.data);
            }
        }
        //Block pointer reset command
        FILE_WriteByte(0xE0);
        FILE_WriteWord(0);
    }
    //=============================Write VGM data===============================
    DBG("--Writing VGM data");
    for(i=0; i<vsr->numcmds; ++i){
        cmd = vsr->cmds[i];
        type = cmd.cmd;
        if(type == 0x50){
            //PSG write
            FILE_WriteByte(type);
            FILE_WriteByte(cmd.data);
            if((cmd.data & 0x80) && !(cmd.data & 0x10) && (cmd.data < 0xE0)){
                //It's a main write, not attenuation, and not noise--i.e. frequency
                FILE_WriteByte(type);
                FILE_WriteByte(cmd.data2);
            }
        }else if((type & 0xFE) == 0x52){
            //OPN2 write
            FILE_WriteByte(type);
            FILE_WriteByte(cmd.addr);
            FILE_WriteByte(cmd.data);
            if((cmd.addr & 0xF4) == 0xA4){
                //Frequency MSB write
                FILE_WriteByte(type);
                FILE_WriteByte(cmd.addr & 0xFB);
                FILE_WriteByte(cmd.data2);
            }
        }else if(type >= 0x80 && type <= 0x8F){
            //OPN2 DAC write--data already taken care of
            FILE_WriteByte(type);
        }else if(type >= 0x70 && type <= 0x7F){
            //Short wait
            FILE_WriteByte(type);
            t += type - 0x6F;
        }else if(type == 0x61){
            //Long wait
            FILE_WriteByte(type);
            FILE_WriteByte(cmd.data);
            FILE_WriteByte(cmd.data);
            t += cmd.data | ((u32)cmd.data2 << 8);
        }else if(type == 0x62){
            //60 Hz wait
            FILE_WriteByte(type);
            t += VGM_DELAY62;
        }else if(type == 0x63){
            //50 Hz wait
            FILE_WriteByte(type);
            t += VGM_DELAY63;
        }else{
            //Unsupported command, should not be here
        }
    }
    FILE_WriteByte(0x66); //End of Data
    //==================================Done====================================
    FILE_WriteClose();
    MUTEX_SDCARD_GIVE;
    DBG("--Done");
    return 0;
}

