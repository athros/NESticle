#ifndef _NES_
#define _NES_

#include "types.h"

#include "rom.h"

#define HMIRROR 0
#define VMIRROR 1

extern byte *ram; //CPU address space
extern struct NES_ppumemory *ppu; //PPU address space
extern struct NES_sprite *spritemem; //Sprite memory

extern int NESinitialized;
extern int romloaded;
void initNEShardware();           //create nes hardware
void terminateNEShardware();      //destroy
int resetNEShardware();           //reset and setup roms and mappers

int getscanline(); //get virtual scanline
extern byte CPUpaused,CPURunning,CPUtrace;
extern int CPUtickframe; //tick a frame at a time

//void m_execute();
void m_reset();
void m_stop();
void resetcpu();

void setIRQ(int line);

#endif
