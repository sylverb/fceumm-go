/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <stdarg.h>

#include "fceu.h"
#include  "fceu-types.h"
#include  "x6502.h"
#include  "fceu.h"
#include  "ppu.h"
#include  "fceu-sound.h"
#include  "general.h"
#include  "fceu-endian.h"
#include  "fceu-memory.h"

#include  "fceu-cart.h"
#include  "nsf.h"
#include  "fds.h"
#include  "ines.h"
#include  "unif.h"
#include  "cheat.h"
#include  "palette.h"
#include  "fceu-state.h"
#include  "video.h"
#include  "input.h"
#include  "file.h"
#include  "vsuni.h"

uint64 timestampbase;

FCEUGI *GameInfo = NULL;
void (*GameInterface)(int h);

void (*GameStateRestore)(int version);

#ifndef TARGET_GNW // It's using too much ram for the G&W, so we are using another method to handle that
readfunc ARead[0x10000];
writefunc BWrite[0x10000];
static readfunc *AReadG = NULL;
static writefunc *BWriteG = NULL;
static int RWWrap = 0;
#else
typedef struct
{
   uint32 min_range, max_range;
   readfunc read_func;
} mem_read_handler_t;

typedef struct
{
   uint32 min_range, max_range;
   writefunc write_func;
} mem_write_handler_t;

mem_read_handler_t MemRead[10];
mem_write_handler_t MemWrite[10];
static uint8 memRead_index = 0;
static uint8 memWrite_index = 0;
#endif

uint8 RAM[0x800];
uint8 PAL = 0;


static DECLFW(BNull)
{
}

static DECLFR(ANull)
{
	return(X.DB);
}

static DECLFW(BRAML)
{
	RAM[A] = V;
}

static DECLFR(ARAML)
{
	return RAM[A];
}

static DECLFW(BRAMH)
{
	RAM[A & 0x7FF] = V;
}

static DECLFR(ARAMH)
{
	return RAM[A & 0x7FF];
}

#ifndef TARGET_GNW
int AllocGenieRW(void)
{
   if (!AReadG)
   {
      if (!(AReadG = (readfunc*)FCEU_malloc(0x8000 * sizeof(readfunc))))
         return 0;
   }
   else
      memset(AReadG, 0, 0x8000 * sizeof(readfunc));

   if (!BWriteG)
   {
      if (!(BWriteG = (writefunc*)FCEU_malloc(0x8000 * sizeof(writefunc))))
         return 0;
   }
   else
      memset(BWriteG, 0, 0x8000 * sizeof(writefunc));

   RWWrap = 1;
   return 1;
}

void FlushGenieRW(void)
{
   int32 x;

   if (RWWrap)
   {
      for (x = 0; x < 0x8000; x++)
      {
         ARead[x + 0x8000] = AReadG[x];
         BWrite[x + 0x8000] = BWriteG[x];
      }
      free(AReadG);
      free(BWriteG);
      AReadG = NULL;
      BWriteG = NULL;
   }
   RWWrap = 0;
}
#endif

extern DECLFR(A200x);
extern DECLFR(A2002);
extern DECLFR(A2007);

readfunc FASTAPASS(1) GetReadHandler(int32 a)
{
#ifndef TARGET_GNW
	if (a >= 0x8000 && RWWrap)
		return AReadG[a - 0x8000];
	else
		return ARead[a];
#else
	if (a < 0x800) {
		return ARAML;
	}
	if (a < 0x2000) {
		return ARAMH;
	}

	if (a < 0x4000) {
		switch (a%8) {
			case 2:
				return A2002;
			case 7:
				return A2007;
			default :
				return A200x;
		}
	}

	for (int i=0; i<memRead_index;i++) {
		if ((a >= MemRead[i].min_range) && (a <= MemRead[i].max_range) ) {
			return MemRead[i].read_func;
		}
	}

	return ANull;
#endif
}

#ifdef TARGET_GNW
uint8 fceu_read(int32 a) {
	if (a < 0x800) {
		return RAM[a];
	}
	if (a < 0x2000) {
		return RAM[a & 0x7FF];
	}
	if (a < 0x4000) {
		switch (a%8) {
			case 2:
				return A2002(a);
			case 7:
				return A2007(a);
			default :
				return A200x(a);
		}
	}
	for (int i=0; i<memRead_index;i++) {
		if ((a >= MemRead[i].min_range) && (a <= MemRead[i].max_range) ) {
			return MemRead[i].read_func(a);
		}
	}

	return(X.DB);
}
#endif

void FASTAPASS(3) SetReadHandler(int32 start, int32 end, readfunc func)
{
	int32 x;
	printf("SetReadHandler %lx-%lx %lx\n",start,end,(int32)func);
	if (!func)
		func = ANull;

#ifndef TARGET_GNW
	if (RWWrap)
		for (x = end; x >= start; x--)
      {
         if (x >= 0x8000)
            AReadG[x - 0x8000] = func;
         else
            ARead[x] = func;
      }
	else
		for (x = end; x >= start; x--)
			ARead[x] = func;
#endif
		MemRead[memRead_index].min_range = start;
		MemRead[memRead_index].max_range = end;
		MemRead[memRead_index].read_func = func;
		memRead_index++;
}

extern DECLFW(B2000);
extern DECLFW(B2001);
extern DECLFW(B2002);
extern DECLFW(B2003);
extern DECLFW(B2004);
extern DECLFW(B2005);
extern DECLFW(B2006);
extern DECLFW(B2007);
extern DECLFW(B4014);

writefunc FASTAPASS(1) GetWriteHandler(int32 a)
{
#ifndef TARGET_GNW
	if (RWWrap && a >= 0x8000)
		return BWriteG[a - 0x8000];
	else
		return BWrite[a];
#else
	if (a <= 0x7FF) {
		return BRAML;
	}
	if (a <= 0x1FFF) {
		return BRAMH;
	}

	if ((a >= 0x2000) && (a <= 0x3FFF) ) {
		switch (a%8) {
			case 0:
				return B2000;
			case 1:
				return B2001;
			case 2:
				return B2002;
			case 3:
				return B2003;
			case 4:
				return B2004;
			case 5:
				return B2005;
			case 6:
				return B2006;
			case 7:
				return B2007;
		}
	}
	for (int i=0; i<memWrite_index;i++) {
		if ((a >= MemWrite[i].min_range) && (a <= MemWrite[i].max_range) ) {
			return MemWrite[i].write_func;
		}
	}
	if (a == 0x4014)
		return B4014;

	return BNull;
#endif
}

#ifdef TARGET_GNW
void fceu_write(int32 a,uint8 v) {
	if (a <= 0x7FF) {
		RAM[a] = v;
		return;
	}
	if (a <= 0x1FFF) {
		RAM[a & 0x7FF] = v;
		return;
	}
	if (a <= 0x3FFF) {
		switch (a%8) {
			case 0:
				B2000(a,v);
				return;
			case 1:
				B2001(a,v);
				return;
			case 2:
				B2002(a,v);
				return;
			case 3:
				B2003(a,v);
				return;
			case 4:
				B2004(a,v);
				return;
			case 5:
				B2005(a,v);
				return;
			case 6:
				B2006(a,v);
				return;
			case 7:
				B2007(a,v);
				return;
		}
	}
	for (int i=0; i<memWrite_index;i++) {
		if ((a >= MemWrite[i].min_range) && (a <= MemWrite[i].max_range) ) {
			MemWrite[i].write_func(a,v);
			return;
		}
	}
	if (a == 0x4014) {
		B4014(a,v);
		return;
	}

	return;
}
#endif

void FASTAPASS(3) SetWriteHandler(int32 start, int32 end, writefunc func)
{
	int32 x;

	printf("SetWriteHandler %lx-%lx %lx\n",start,end,(int32)func);
	if (!func)
		func = BNull;

#ifndef TARGET_GNW
	if (RWWrap)
		for (x = end; x >= start; x--)
      {
			if (x >= 0x8000)
				BWriteG[x - 0x8000] = func;
			else
				BWrite[x] = func;
		}
	else
		for (x = end; x >= start; x--)
			BWrite[x] = func;
#else
	MemWrite[memWrite_index].min_range = start;
	MemWrite[memWrite_index].max_range = end;
	MemWrite[memWrite_index].write_func = func;
	memWrite_index++;
#endif
}

void FCEUI_CloseGame(void)
{
	printf("FCEUI_CloseGame\n");
	if (!GameInfo)
      return;

#ifndef TARGET_GNW
   if (GameInfo->name)
      free(GameInfo->name);
#endif
   GameInfo->name = 0;
#ifndef TARGET_GNW
   if (GameInfo->type != GIT_NSF)
      FCEU_FlushGameCheats();
#endif
   GameInterface(GI_CLOSE);
   ResetExState(0, 0);
#ifndef TARGET_GNW
   FCEU_CloseGenie();
   free(GameInfo);
#endif
   GameInfo = 0;
}

void ResetGameLoaded(void)
{
	if (GameInfo)
      FCEUI_CloseGame();

	GameStateRestore = NULL;
	PPU_hook = NULL;
	GameHBIRQHook = NULL;

	if (GameExpSound.Kill)
		GameExpSound.Kill();
	memset(&GameExpSound, 0, sizeof(GameExpSound));

	MapIRQHook = NULL;
	MMC5Hack = 0;
	PEC586Hack = 0;
	PAL &= 1;
	pale = 0;
}

#ifdef TARGET_GNW
int iNESLoad(const char *name, const uint8_t *rom, uint32_t rom_size);
//int UNIFLoad(const char *name, const char *rom, uint32_t rom_size);
#else
int UNIFLoad(const char *name, FCEUFILE *fp);
int iNESLoad(const char *name, FCEUFILE *fp);
int FDSLoad(const char *name, FCEUFILE *fp);
int NSFLoad(FCEUFILE *fp);
#endif

#ifdef TARGET_GNW
FCEUGI *FCEUI_LoadGame(const char *name, const uint8_t *databuf, size_t databufsize,
      frontend_post_load_init_cb_t frontend_post_load_init_cb)
{
   ResetGameLoaded();

   GameInfo = malloc(sizeof(FCEUGI));
   memset(GameInfo, 0, sizeof(FCEUGI));

   GameInfo->soundchan = 0;
   GameInfo->soundrate = 0;
   GameInfo->name = 0;
   GameInfo->type = GIT_CART;
   GameInfo->vidsys = GIV_USER;
   GameInfo->input[0] = GameInfo->input[1] = -1;
   GameInfo->inputfc = -1;
   GameInfo->cspecial = 0;

   if (iNESLoad(name, databuf, databufsize))
      goto endlseq;
//   if (UNIFLoad(NULL, fp))
//      goto endlseq;

   FCEU_PrintError("An error occurred while loading the file.\n");
   return NULL;

endlseq:
   if (frontend_post_load_init_cb)
      (*frontend_post_load_init_cb)();

   FCEU_ResetVidSys();
//   if (GameInfo->type != GIT_NSF)
//      if (FSettings.GameGenie)
//         FCEU_OpenGenie();

   PowerNES();

//   if (GameInfo->type != GIT_NSF) {
//      FCEU_LoadGamePalette();
//      FCEU_LoadGameCheats();
//   }

   FCEU_ResetPalette();

   return(GameInfo);
}
#else
FCEUGI *FCEUI_LoadGame(const char *name, const uint8_t *databuf, size_t databufsize,
      frontend_post_load_init_cb_t frontend_post_load_init_cb)
{
   FCEUFILE *fp;

   ResetGameLoaded();

   GameInfo = malloc(sizeof(FCEUGI));
   memset(GameInfo, 0, sizeof(FCEUGI));

   GameInfo->soundchan = 0;
   GameInfo->soundrate = 0;
   GameInfo->name = 0;
   GameInfo->type = GIT_CART;
   GameInfo->vidsys = GIV_USER;
   GameInfo->input[0] = GameInfo->input[1] = -1;
   GameInfo->inputfc = -1;
   GameInfo->cspecial = 0;

   fp = FCEU_fopen(name, databuf, databufsize);

   if (!fp) {
      FCEU_PrintError("Error opening \"%s\"!", name);

      free(GameInfo);
      GameInfo = NULL;

      return NULL;
   }

   if (iNESLoad(name, fp))
      goto endlseq;
   if (NSFLoad(fp))
      goto endlseq;
   if (UNIFLoad(NULL, fp))
      goto endlseq;
   if (FDSLoad(NULL, fp))
      goto endlseq;

   FCEU_PrintError("An error occurred while loading the file.\n");
   FCEU_fclose(fp);

   if (GameInfo->name)
      free(GameInfo->name);
   GameInfo->name = NULL;
   free(GameInfo);
   GameInfo = NULL;

   return NULL;

endlseq:
   FCEU_fclose(fp);

   if (frontend_post_load_init_cb)
      (*frontend_post_load_init_cb)();

   FCEU_ResetVidSys();
   if (GameInfo->type != GIT_NSF)
      if (FSettings.GameGenie)
         FCEU_OpenGenie();

   PowerNES();

   if (GameInfo->type != GIT_NSF) {
      FCEU_LoadGamePalette();
      FCEU_LoadGameCheats();
   }

   FCEU_ResetPalette();

   return(GameInfo);
}
#endif

int FCEUI_Initialize(void) {
	if (!FCEU_InitVirtualVideo())
		return 0;
	memset(&FSettings, 0, sizeof(FSettings));
	FSettings.UsrFirstSLine[0] = 8;
	FSettings.UsrFirstSLine[1] = 0;
	FSettings.UsrLastSLine[0] = 231;
	FSettings.UsrLastSLine[1] = 239;
	FSettings.SoundVolume = 100;
	FCEUPPU_Init();
	X6502_Init();
	return 1;
}

void FCEUI_Kill(void) {
	FCEU_KillVirtualVideo();
#ifndef TARGET_GNW
	FCEU_KillGenie();
#endif
}

void FCEUI_Emulate(uint8 **pXBuf, int32 **SoundBuf, int32 *SoundBufSize, int skip) {
	int ssize;

	FCEU_UpdateInput();
#ifndef TARGET_GNW
	if (geniestage != 1) FCEU_ApplyPeriodicCheats();
#endif
	FCEUPPU_Loop(skip);

	ssize = FlushEmulateSound();

	timestampbase += timestamp;

	timestamp = 0;
	sound_timestamp = 0;

	*pXBuf = skip ? 0 : XBuf;
	*SoundBuf = WaveFinal;
	*SoundBufSize = ssize;
}


void ResetNES(void)
{
	if (!GameInfo)
      return;
	GameInterface(GI_RESETM2);
	FCEUSND_Reset();
	FCEUPPU_Reset();
	X6502_Reset();
}

int option_ramstate = 0;

void FCEU_MemoryRand(uint8 *ptr, uint32 size)
{
	int x = 0;
	while (size) {
#if 0
		*ptr = (x & 4) ? 0xFF : 0x00;	/* Huang Di DEBUG MODE enabled by default */
										/* Cybernoid NO MUSIC by default */
		*ptr = (x & 4) ? 0x7F : 0x00;	/* Huang Di DEBUG MODE enabled by default */
										/* Minna no Taabou no Nakayoshi Daisakusen DOESN'T BOOT */
										/* Cybernoid NO MUSIC by default */
		*ptr = (x & 1) ? 0x55 : 0xAA;	/* F-15 Sity War HISCORE is screwed... */
										/* 1942 SCORE/HISCORE is screwed... */
#endif
		uint8_t v = 0;
		switch (option_ramstate)
		{
		case 0: v = 0xff; break;
		case 1: v = 0x00; break;
		case 2: v = (uint8_t)rand(); break;
		}
		*ptr = v;
		x++;
		size--;
		ptr++;
	}
}

void hand(X6502 *X, int type, uint32 A)
{
}

void PowerNES(void)
{
	if (!GameInfo)
      return;

#ifndef TARGET_GNW
	FCEU_CheatResetRAM();
	FCEU_CheatAddRAM(2, 0, RAM);

	FCEU_GeniePower();
#else
	memRead_index = 0;
	memWrite_index = 0;
#endif

	FCEU_MemoryRand(RAM, 0x800);

#ifndef TARGET_GNW
	SetReadHandler(0x0000, 0xFFFF, ANull);
	SetWriteHandler(0x0000, 0xFFFF, BNull);

	SetReadHandler(0, 0x7FF, ARAML);
	SetWriteHandler(0, 0x7FF, BRAML);

	SetReadHandler(0x800, 0x1FFF, ARAMH);	/* Part of a little */
	SetWriteHandler(0x800, 0x1FFF, BRAMH);	/* hack for a small speed boost. */
#endif

	InitializeInput();
	FCEUSND_Power();
	FCEUPPU_Power();

	/* Have the external game hardware "powered" after the internal NES stuff.
		Needed for the NSF code and VS System code.
	*/
	GameInterface(GI_POWER);
#ifndef TARGET_GNW
	if (GameInfo->type == GIT_VSUNI)
		FCEU_VSUniPower();
#endif

	timestampbase = 0;
	X6502_Power();
#ifndef TARGET_GNW
	FCEU_PowerCheats();
#endif
}

void FCEU_ResetVidSys(void)
{
	int w;

	if (GameInfo->vidsys == GIV_NTSC)
		w = 0;
	else if (GameInfo->vidsys == GIV_PAL)
   {
      w = 1;
      dendy = 0;
   }
	else
		w = FSettings.PAL;

	PAL = w ? 1 : 0;

   if (PAL)
      dendy = 0;

   normal_scanlines = dendy ? 290 : 240;
   totalscanlines = normal_scanlines + (overclock_enabled ? extrascanlines : 0);

	FCEUPPU_SetVideoSystem(w || dendy);
	SetSoundVariables();
}

FCEUS FSettings;

void FCEU_printf(char *format, ...)
{
	char temp[2048];

	va_list ap;

	va_start(ap, format);
	vsprintf(temp, format, ap);
	FCEUD_Message(temp);

	va_end(ap);
}

void FCEU_PrintError(char *format, ...)
{
	char temp[2048];

	va_list ap;

	va_start(ap, format);
	vsprintf(temp, format, ap);
	FCEUD_PrintError(temp);

	va_end(ap);
}

void FCEUI_SetRenderedLines(int ntscf, int ntscl, int palf, int pall)
{
	FSettings.UsrFirstSLine[0] = ntscf;
	FSettings.UsrLastSLine[0] = ntscl;
	FSettings.UsrFirstSLine[1] = palf;
	FSettings.UsrLastSLine[1] = pall;
	if (PAL || dendy)
   {
		FSettings.FirstSLine = FSettings.UsrFirstSLine[1];
		FSettings.LastSLine = FSettings.UsrLastSLine[1];
	}
   else
   {
		FSettings.FirstSLine = FSettings.UsrFirstSLine[0];
		FSettings.LastSLine = FSettings.UsrLastSLine[0];
	}
}

void FCEUI_SetVidSystem(int a)
{
	FSettings.PAL = a ? 1 : 0;

	if (!GameInfo)
      return;

   FCEU_ResetVidSys();
   FCEU_ResetPalette();
}

int FCEUI_GetCurrentVidSystem(int *slstart, int *slend)
{
	if (slstart)
		*slstart = FSettings.FirstSLine;
	if (slend)
		*slend = FSettings.LastSLine;
	return(PAL);
}

void FCEUI_SetGameGenie(int a)
{
	FSettings.GameGenie = a ? 1 : 0;
}

int32 FCEUI_GetDesiredFPS(void)
{
	if (PAL || dendy)
		return(838977920);	/* ~50.007 */
	else
		return(1008307711);	/* ~60.1 */
}
