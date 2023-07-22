#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "fceu-types.h"
#include "x6502.h"
#include "fceu.h"
#include "fceu-sound.h"
#include "fceu-endian.h"
#include "fds.h"
#include "general.h"
#include "fceu-state.h"
#include "fceu-memory.h"
#include "ppu.h"
#include "video.h"

static void (*SPreSave)(void);
static void (*SPostSave)(void);

/* static int SaveStateStatus[10]; */

static SFORMAT SFMDATA[64];
int SFEXINDEX;

#define RLSB     FCEUSTATE_RLSB     /* 0x80000000 */

extern SFORMAT FCEUPPU_STATEINFO[];
extern SFORMAT FCEUSND_STATEINFO[];
extern SFORMAT FCEUCTRL_STATEINFO[];

SFORMAT SFCPU[] = {
   { &X.PC, 2 | RLSB, "PC\0" },
   { &X.A, 1, "A\0\0" },
   { &X.X, 1, "X\0\0" },
   { &X.Y, 1, "Y\0\0" },
   { &X.S, 1, "S\0\0" },
   { &X.P, 1, "P\0\0" },
   { &X.DB, 1, "DB"},
   { RAM, 0x800, "RAM" },
   { 0 }
};

SFORMAT SFCPUC[] = {
   { &X.jammed, 1, "JAMM" },
   { &X.IRQlow, 4 | RLSB, "IQLB" },
   { &X.tcount, 4 | RLSB, "ICoa" },
   { &X.count, 4 | RLSB, "ICou" },
   { &timestampbase, sizeof(timestampbase) | RLSB, "TSBS" },
   { &X.mooPI, 1, "MooP"},
   { 0 }
};

static int SubWrite(filesystem_file_t *file, SFORMAT *sf)
{
   uint32 acc = 0;

   while(sf->v)
   {
      if(sf->s == (~(uint32)0)) /* Link to another struct. */
      {
         uint32 tmp;

         if(!(tmp = SubWrite(file, (SFORMAT *)sf->v)))
            return(0);
         acc += tmp;
         sf++;
         continue;
      }

      acc += 8; /* Description + size */
      acc += sf->s & (~RLSB);

      if(file) /* Are we writing or calculating the size of this block? */
      {
         filesystem_write(file, sf->desc, 4);
         write32le_filesystem(file, sf->s & (~RLSB));

#ifdef MSB_FIRST
         if(sf->s & RLSB)
            FlipByteOrder((uint8 *)sf->v, sf->s & (~RLSB));
#endif
         filesystem_write(file, (char *)sf->v, sf->s & (~RLSB));

         /* Now restore the original byte order. */
#ifdef MSB_FIRST
         if(sf->s & RLSB)
            FlipByteOrder((uint8 *)sf->v, sf->s & (~RLSB));
#endif
      }
      sf++;
   }

   return acc;
}

static int WriteStateChunk(filesystem_file_t *file, int type, SFORMAT *sf)
{
   int bsize;

   filesystem_write(file, type, 1);

   bsize = SubWrite(0, sf);
   write32le_filesystem(file, bsize);

   if (!SubWrite(file, sf))
      return 0;
   return bsize + 5;
}

static SFORMAT *CheckS(SFORMAT *sf, uint32 tsize, char *desc)
{
   while (sf->v)
   {
      if (sf->s == (~(uint32)0))
      { /* Link to another SFORMAT structure. */
         SFORMAT *tmp;
         if ((tmp = CheckS((SFORMAT*)sf->v, tsize, desc)))
            return(tmp);
         sf++;
         continue;
      }
      if (!strncmp(desc, sf->desc, 4))
      {
         if (tsize != (sf->s & (~RLSB)))
            return(0);
         return(sf);
      }
      sf++;
   }
   return(0);
}

static int ReadStateChunk(filesystem_file_t *file, SFORMAT *sf, int size)
{
   SFORMAT *tmp;
   uint64 temp;
   temp = filesystem_seek(file, 0, LFS_SEEK_CUR);  // get the current position

   while(filesystem_seek(file, 0, LFS_SEEK_CUR) < (temp + size))  // while we are in this chunk
   {
      uint32 tsize;
      char toa[4];
      if(filesystem_read(file, toa, 4) <= 0)  // read a uint32
         return 0;

      read32le_filesystem(file, &tsize);  //read another

      if((tmp = CheckS(sf, tsize, toa)))
      {
         filesystem_read(file, (char *)tmp->v, tmp->s & (~RLSB));

#ifdef MSB_FIRST
         if(tmp->s & RLSB)
            FlipByteOrder((uint8 *)tmp->v, tmp->s & (~RLSB));
#endif
      }
      else
      {
         // TODO: just do an unconditional read so we can support compression
         filesystem_seek(file, tsize, LFS_SEEK_CUR);
      }
   }
   return 1;
}

static int ReadStateChunks(filesystem_file_t *file)
{
   int t;
   uint32 size;
   int ret = 1;

   while(true)
   {
      if(0 == filesystem_read(file, &t, 1))
         break;
      if (!read32le_filesystem(file, &size))
         break;

      switch(t)
      {
         case 1:
            if (!ReadStateChunk(file, SFCPU, size))
               ret = 0;
            break;
         case 2:
            if (!ReadStateChunk(file, SFCPUC, size))
               ret = 0;
            /* else
               X.mooPI = X.P; */ /* Quick and dirty hack. */
            break;
         case 3:
            if (!ReadStateChunk(file, FCEUPPU_STATEINFO, size))
               ret = 0;
            break;
         case 4:
            if (!ReadStateChunk(file, FCEUCTRL_STATEINFO, size))
               ret = 0;
            break;
         case 5:
            if (!ReadStateChunk(file, FCEUSND_STATEINFO, size))
               ret = 0;
            break;
         case 0x10:
            if (!ReadStateChunk(file, SFMDATA, size))
               ret = 0;
            break;
         default:
            printf("Seeking?!?!\n");
            if (filesystem_seek(file, size, LFS_SEEK_CUR) < 0)
               goto endo;
            break;
      }
   }
endo:
   return ret;
}

void FCEUSS_Save_Mem(void)
{
   // TODO: pass in name to function signature.
   filesystem_file_t *file = filesystem_open("NES_SAVESTATE", false);

   uint8 header[12] = {0};

   // 4 bytes - magic
   // OLD: 4 bytse - size
   // 8 bytes - fceu version numeric
   header[0] = 'F';
   header[1] = 'C';
   header[2] = 'S';
   header[3] = 0xFF;

   FCEU_en32lsb(header + 4, FCEU_VERSION_NUMERIC);
   filesystem_write(file, header, 12);

   FCEUPPU_SaveState();
   WriteStateChunk(file, 1, SFCPU);
   WriteStateChunk(file, 2, SFCPUC);
   WriteStateChunk(file, 3, FCEUPPU_STATEINFO);
   WriteStateChunk(file, 4, FCEUCTRL_STATEINFO);
   WriteStateChunk(file, 5, FCEUSND_STATEINFO);

   if (SPreSave)
      SPreSave();

   WriteStateChunk(file, 0x10, SFMDATA);

   if (SPreSave)
      SPostSave();

   filesystem_close(file);
}

void FCEUSS_Load_Mem(void)
{
   // TODO: pass in name to function signature.
   printf("Opening file\n");
   filesystem_file_t *file = filesystem_open("NES_SAVESTATE", false);

   uint8 header[12];
   int stateversion;
   int x;

   printf("reading header\n");
   filesystem_read(file, header, 12);

   if (memcmp(header, "FCS", 3) != 0)
      return;

   if (header[3] == 0xFF)
      stateversion = FCEU_de32lsb(header + 4);
   else
      stateversion = header[3] * 100;
   
   printf("reading chunks\n");
   x = ReadStateChunks(file);

   printf("meow\n");
   if (stateversion < 9500)
      X.IRQlow = 0;

   if (GameStateRestore)
      GameStateRestore(stateversion);

   if (x)
   {
      FCEUPPU_LoadState(stateversion);
      FCEUSND_LoadState(stateversion);
   }
   filesystem_close(file);
}

void ResetExState(void (*PreSave)(void), void (*PostSave)(void))
{
   SPreSave  = PreSave;
   SPostSave = PostSave;
   SFEXINDEX = 0;
   SFMDATA[0].s = 0;
}

void AddExState(void *v, uint32 s, int type, char *desc)
{
   /* prevent adding a terminator to the list if a NULL pointer was provided */
   if (v == NULL) return;
   memset(SFMDATA[SFEXINDEX].desc, 0, sizeof(SFMDATA[SFEXINDEX].desc));
   if (desc)
      strncpy(SFMDATA[SFEXINDEX].desc, desc, sizeof(SFMDATA[SFEXINDEX].desc));
   SFMDATA[SFEXINDEX].v = v;
   SFMDATA[SFEXINDEX].s = s;
   if (type)
      SFMDATA[SFEXINDEX].s |= RLSB;
   if (SFEXINDEX < 63)
      SFEXINDEX++;
   SFMDATA[SFEXINDEX].v = 0;   /* End marker. */
}

void FCEU_DrawSaveStates(uint8 *XBuf)
{
}

#endif
