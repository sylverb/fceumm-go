#ifdef TARGET_GNW
#include "build/config.h"
#endif

#if !defined(TARGET_GNW) || (defined(TARGET_GNW) &&  defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0)
/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
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

#include "mapinc.h"

static uint8 reg, mirr;
static SFORMAT StateRegs[] =
{
	{ &reg, 1, "REGS" },
	{ &mirr, 1, "MIRR" },
	{ 0 }
};

static void Sync(void) {
	setprg8(0x6000, (ROM_size == 17) ? 32 : 31); /* FIXME: Verify these */
	setprg32(0x8000, reg);
	setchr8(0);
}

static DECLFW(M283Write) {
	reg = V;
	Sync();
}

static void M283Power(void) {
	reg = 0;
	Sync();
	SetReadHandler(0x6000, 0x7FFF, CartBR);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000, 0xFFFF, M283Write);
}

static void M283Reset(void) {
	reg = 0;
}

static void StateRestore(int version) {
	Sync();
}

void Mapper283_Init(CartInfo *info) {
	info->Reset = M283Reset;
	info->Power = M283Power;
	GameStateRestore = StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
#endif
