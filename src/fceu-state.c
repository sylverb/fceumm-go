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
#include <assert.h>
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

static int SubWrite(FILE *file, SFORMAT *sf)
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
         fwrite(sf->desc, 1, 4, file);
         write32le_fs(file, sf->s & (~RLSB));

#ifdef MSB_FIRST
         if(sf->s & RLSB)
            FlipByteOrder((uint8 *)sf->v, sf->s & (~RLSB));
#endif
         fwrite((char *)sf->v, 1, sf->s & (~RLSB), file);

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

static int WriteStateChunk(FILE *file, int type, SFORMAT *sf)
{
   int bsize;

   fwrite((unsigned char *)&type, 1, 1, file);

   bsize = SubWrite(0, sf);
   write32le_fs(file, bsize);

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

static int ReadStateChunk(FILE *file, SFORMAT *sf, int size)
{
   SFORMAT *tmp;

   while(size > 0)
   {
      uint32 tsize;
      char toa[4];
      if(fread((unsigned char *)toa, 1, 4, file) <= 0)  // read a uint32
         return 0;
      size -= 4;

      size -= read32le_fs(file, &tsize);  //read another

      if((tmp = CheckS(sf, tsize, toa)))
      {
         size -= fread((unsigned char *)tmp->v, 1, tmp->s & (~RLSB), file);

#ifdef MSB_FIRST
         if(tmp->s & RLSB)
            FlipByteOrder((uint8 *)tmp->v, tmp->s & (~RLSB));
#endif
      }
      else
      {
         // I don't think this ever executes
         printf("fseek %ld\n", tsize);
         assert(0);
         fseek(file, tsize, SEEK_CUR);
      }
   }
   return 1;
}

static int ReadStateChunks(FILE *file)
{
   uint8_t t;
   uint32 size;
   int ret = 1;

   while(true)
   {
      if(0 == fread(&t, 1, 1, file))
         break;
      if (!read32le_fs(file, &size))
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
            // I *think* this never happens?
            printf("Seeking?!?! %ld\n", size);
            assert(0);
            if (fseek(file, size, SEEK_CUR) < 0)
               goto endo;
            break;
      }
   }
endo:
   return ret;
}

void FCEUSS_Save_Fs(const char *path) // TODO rename to FCEUSS_Save_fs
{
   FILE *file = fopen(path, "wb");

   uint8 header[12] = {0};

   // 4 bytes - magic
   // OLD: 4 bytse - size
   // 8 bytes - fceu version numeric
   header[0] = 'F';
   header[1] = 'C';
   header[2] = 'S';
   header[3] = 0xFF;

   FCEU_en32lsb(header + 4, FCEU_VERSION_NUMERIC);
   fwrite(header, 1, 12, file);

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

   fclose(file);
}

void FCEUSS_Load_Fs(const char *path) // TODO rename to FCEUSS_Save_fs
{
   FILE *file = fopen(path,"rb");

   uint8 header[12];
   int stateversion;
   int x;

   fread(header, 12, 1, file);

   if (memcmp(header, "FCS", 3) != 0)
      return;

   if (header[3] == 0xFF)
      stateversion = FCEU_de32lsb(header + 4);
   else
      stateversion = header[3] * 100;
   
   x = ReadStateChunks(file);

   if (stateversion < 9500)
      X.IRQlow = 0;

   if (GameStateRestore)
      GameStateRestore(stateversion);

   if (x)
   {
      FCEUPPU_LoadState(stateversion);
      FCEUSND_LoadState(stateversion);
   }
   fclose(file);
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
