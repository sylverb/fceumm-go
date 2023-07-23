#ifndef _FCEU_ENDIAN_H
#define _FCEU_ENDIAN_H

#include "fceu-memory.h"
#include "filesystem.h"

int write32le_fs(fs_file_t *file, uint32 b);
int read32le_fs(fs_file_t *file, uint32 *Bufo);

void FlipByteOrder(uint8 *src, uint32 count);

void FCEU_en32lsb(uint8 *, uint32);
uint32 FCEU_de32lsb(const uint8 *);

#endif
