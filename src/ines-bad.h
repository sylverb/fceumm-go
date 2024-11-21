#ifndef _FCEU_INES_BAD_H
#define _FCEU_INES_BAD_H

#ifndef SD_CARD
#define SD_CARD 0
#endif

#if SD_CARD != 1 // TODO : set info in a file for SD_CARD = 1
{ 0xecf78d8a13a030a6LL, (uint8_t*)"Ai Sensei no Oshiete", INESB_HACKED },
{ 0x4712856d3e12f21fLL, (uint8_t*)"Akumajou Densetsu", INESB_HACKED },
{ 0x10f90ba5bd55c22eLL, (uint8_t*)"Alien Syndrome", INESB_HACKED },
{ 0x0d69ab3ad28ad1c2LL, (uint8_t*)"Banana", INESB_INCOMPLETE },
{ 0x85d2c348a161cdbfLL, (uint8_t*)"Bio Senshi Dan", INESB_HACKED },
{ 0x18fdb7c16aa8cb5cLL, (uint8_t*)"Bucky O'Hare", INESB_CORRUPT },
{ 0xe27c48302108d11bLL, (uint8_t*)"Chibi Maruko Chan", INESB_HACKED },
{ 0x9d1f505c6ba507bfLL, (uint8_t*)"Contra", INESB_HACKED },
{ 0x60936436d3ea0ab6LL, (uint8_t*)"Crisis Force", INESB_HACKED },
{ 0xcf31097ddbb03c5dLL, (uint8_t*)"Crystalis (Prototype)", INESB_CORRUPT },
{ 0x92080a8ce94200eaLL, (uint8_t*)"Digital Devil Story II", INESB_HACKED },
{ 0x6c2a2f95c2fe4b6eLL, (uint8_t*)"Dragon Ball", INESB_HACKED },
{ 0x767aaff62963c58fLL, (uint8_t*)"Dragon Ball", INESB_HACKED },
{ 0x97f133d8bc1c28dbLL, (uint8_t*)"Dragon Ball", INESB_HACKED },
{ 0x500b267abb323005LL, (uint8_t*)"Dragon Warrior 4", INESB_CORRUPT },
{ 0x02bdcf375704784bLL, (uint8_t*)"Erika to Satoru no Yume Bouken", INESB_HACKED },
{ 0xd4fea9d2633b9186LL, (uint8_t*)"Famista 91", INESB_HACKED },
{ 0xfdf8c812839b61f0LL, (uint8_t*)"Famista 92", INESB_HACKED },
{ 0xb5bb1d0fb47d0850LL, (uint8_t*)"Famista 93", INESB_HACKED },
{ 0x30471e773f7cdc89LL, (uint8_t*)"Famista 94", INESB_HACKED },
{ 0x76c5c44ffb4a0bd7LL, (uint8_t*)"Fantasy Zone", INESB_HACKED },
{ 0xb470bfb90e2b1049LL, (uint8_t*)"Fire Emblem Gaiden", INESB_HACKED },
{ 0x27da2b0c500dc346LL, (uint8_t*)"Fire Emblem", INESB_HACKED },
{ 0x23214fe456fba2ceLL, (uint8_t*)"Ganbare Goemon 2", INESB_HACKED },
{ 0xbf8b22524e8329d9LL, (uint8_t*)"Ganbare Goemon Gaiden", INESB_HACKED },
{ 0xa97041c3da0134e3LL, (uint8_t*)"Gegege no Kitarou 2", INESB_INCOMPLETE },
{ 0x805db49a86db5449LL, (uint8_t*)"Goonies", INESB_HACKED },
{ 0xc5abdaa65ac49b6bLL, (uint8_t*)"Gradius 2", INESB_HACKED },
{ 0x04afae4ad480c11cLL, (uint8_t*)"Gradius 2", INESB_HACKED },
{ 0x9b4bad37b5498992LL, (uint8_t*)"Gradius 2", INESB_HACKED },
{ 0xb068d4ac10ef848eLL, (uint8_t*)"Highway Star", INESB_HACKED },
{ 0xbf5175271e5019c3LL, (uint8_t*)"Kaiketsu Yanchamaru 3", INESB_HACKED },
{ 0xfb4b508a236bbba3LL, (uint8_t*)"Salamander", INESB_HACKED },
{ 0x1895afc6eef26c7dLL, (uint8_t*)"Super Mario Bros.", INESB_HACKED },
{ 0x3716c4bebf885344LL, (uint8_t*)"Super Mario Bros.", INESB_HACKED },
{ 0xfffda4407d80885aLL, (uint8_t*)"Sweet Home", INESB_CORRUPT },
{ 0x103fc85d978b861bLL, (uint8_t*)"Sweet Home", INESB_CORRUPT },
{ 0x7979dc51da86f19fLL, (uint8_t*)"110-in-1", INESB_CORRUPT },
{ 0x001c0bb9c358252aLL, (uint8_t*)"110-in-1", INESB_CORRUPT },
#endif
{ 0, 0, 0 }

#endif
