//routines for handling roms

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <process.h>
#include <sys\stat.h>

#include "types.h"

#include "message.h"
#include "file.h"

#include "m6502\m6502.h"

#include "keyb.h"

#define FREE(x) if (x) {free(x); x=0;}
#define DELETE(x) if (x) {delete x; x=0;}

#include "nes.h"
#include "nesvideo.h"
#include "nessound.h"

#include "r2img.h"

#include "mmc.h"
#include "slist.h"

//rom stuff
NESROMHEADER romhdr;
byte *trainer;
int numrom,numvrom;
ROMPAGE *ROM;
VROMPAGE *VROM;

//
byte *ram; //CPU address space
NES_ppumemory *ppu; //PPU address space
NES_sprite *spritemem; //Sprite memory

BATTERYMEM *batterymem; //battery backed memory

byte *expansion; //memory at 5000

int NESinitialized=0;

int romloaded=0;
void freerom();

//rom filename (without extension) eg "CASTLEVA"
char romfilename[128];

//-----------------------------------------------------
// NES rom loader

void loadrom(char *romfile)
{
 if (!romfile) return;

 //free old rom
 freerom();

 //get rom name without extension
 strcpy(romfilename,romfile);
 if (strchr(romfilename,'.')) *strchr(romfilename,'.')=0;

 char s[64];
 strcpy(s,romfilename);
 strcat(s,".nes"); //add extension

 msg.printf(2,"Loading rom %s...",s);
 FILEIO f;
 if (f.open(s)) {msg.error("Unable to open file %s",s); return;}

 //read header
 if (f.read(&romhdr,sizeof(romhdr))) {msg.error("Unable to read header"); return;}

 //see if header file exists ?
 FILEIO g;
 char hdrfile[64];
 strcpy(hdrfile,romfilename);
 strcat(hdrfile,".hdr");
 if (!g.open(hdrfile)) //does header file exist?
  {
   g.read(&romhdr,sizeof(romhdr));
   g.close();
   msg.printf(1,"Header read from %s",hdrfile);
  }

 if (!romhdr.validate()) {msg.error("Bad ROM header"); return;}

 //load trainer
 if (romhdr.trainer) trainer=(byte *)f.readalloc(512);

 //read ROM
 numrom=romhdr.num16k;
 ROM=(ROMPAGE *)f.readalloc(numrom*sizeof(ROMPAGE));

 //read VROM
 numvrom=romhdr.num8k;
 VROM=(VROMPAGE *)f.readalloc(numvrom*sizeof(VROMPAGE)); //read vrom
 msg.printf(1,"%dK ROM - %dK VROM",numrom*sizeof(ROMPAGE)/1024,numvrom*sizeof(VROMPAGE)/1024);

 int bytesleft=f.size()-f.getpos();
 if (bytesleft)
 msg.printf(1,"%d bytes left at end of .NES file",bytesleft);

 f.close();
// msg.printf(2,"%s loaded",romfile);
 romloaded=1;

 if (romhdr.battery)
  {
   batterymem=new BATTERYMEM;
   batterymem->read(romfilename); //read battery backed rom
  } else batterymem=0;

  /*
 char expfile[64];
 strcpy(expfile,romfilename);
 strcat(expfile,".500");
 if (!g.open(expfile)) //exp file exists?
  {
   expansion=(char *)g.readalloc(0x1000);
   g.close();
   msg.printf(1,"Expansion mem read from %s",expfile);
  } */

 //initialize nes hardware....
 initNEShardware();

 //create video space
 new nesvideo(romfilename);

 //create nes sound device
 ns=new nessound;
}

void freerom()
{
 if (batterymem)
  {
   batterymem->copyfromRAM();
   batterymem->write(romfilename); //write battery memory!
  }

 terminateNEShardware();
 FREE(trainer);
 FREE(ROM);
 FREE(VROM);
 FREE(batterymem);
 FREE(expansion);

 DELETE(ns);

 if (romloaded) msg.printf(2,"ROM freed");
 romloaded=0;

 //delete video space
 if (nv && nv->parent)  delete nv->parent;
}

void m_showmessages();
void m_getrominfo()
{
 if (!romloaded) msg.error("ROM not loaded");
  else  romhdr.printinfo(); //print info about rom
 m_showmessages();
}

void m_free()
{
 if (!romloaded) msg.error("ROM not loaded");
  else freerom();
}

void m_ramdump()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}

 char s[64];

 for (int i=0; i<99; i++)
  {
   sprintf(s,"%s.d%02d",romfilename,i);
   struct stat sb;
   if (stat(s,&sb)!=0) break;
  }

 FILEIO f;
 f.create(s);
 f.write(ram,0x8000);
 f.write(m6502rom+0x8000,0x8000);
 f.close();

 msg.printf(1,"RAM dumped to %s",s);
}



void m_romdump()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}
 char s[64];
 for (int i=0; i<numrom; i++)
  {
   sprintf(s,"%s.R%02X",romfilename,i);
   FILEIO f;
   f.create(s);
   f.write(ROM[i],sizeof(ROMPAGE));
   f.close();
  }
 msg.printf(1,"ROM dumped to %s.Rxx",romfilename);
}

//---------------------------------------------------------
//Battery backed memory

BATTERYMEM::BATTERYMEM()
{
  memset(data,0,0x2000);
}

void BATTERYMEM::read(char *romfilename)
{
 char s[64];
 strcpy(s,romfilename);
 strcat(s,".sav");

 //load savegame
 static FILEIO f;
 if (!f.open(s)) //try and open savegame...
   {
    f.read(data,0x2000); //read it!
    f.close();
    msg.printf(1,"Save game loaded from %s",s);
   }
}

void BATTERYMEM::write(char *romfilename)
{
 char s[64];
 strcpy(s,romfilename);
 strcat(s,".sav");

 //save savegame
 FILEIO f;
 f.create(s);
 f.write(data,0x2000); //read it!
 f.close();
 msg.printf(1,"Save game saved to %s",s);
}

void BATTERYMEM::copyfromRAM()
{
 if (!ram) return;
 memcpy(data,&ram[0x6000],0x2000);
}

void BATTERYMEM::copytoRAM()
{
 if (!ram) return;
 memcpy(&ram[0x6000],data,0x2000);
}



//---------------------------------------------------------
//NES rom header

char *getmmctypestr(int type);

int NESROMHEADER::validate() {return (str[0]=='N' && str[1]=='E' && str[2]=='S');}
void NESROMHEADER::printinfo()
{
 msg.printf(1,"16K ROM banks: %d",num16k);
 msg.printf(1,"8K VROM banks: %d",num8k);

 msg.printf(1,"%s mirroring",mirroring ? "Vertical" : "Horizontal");
 if (battery) msg.printf(1,"Battery backed RAM");
 if (trainer) msg.printf(1,"Trainer");
 if (fourscreen) msg.printf(1,"4 Screen VRAM");

 msg.printf(1,"Mapper#%d: %s",banktype,getmmctypestr(banktype));
}


void m_writeromheader()
{
 if (!romloaded) {msg.error("ROM not loaded"); return; }
 if (!nv) return;

 //put mirroring in rom
 romhdr.mirroring=nv->mirroring;

 char s[64];
 strcpy(s,romfilename);
 strcat(s,".nes");

 FILEIO f;
 if (f.open(s)) return;
 f.write(&romhdr,sizeof(romhdr));
 f.close();
 msg.printf(1,"NES ROM header written to %s",s);
}

extern COLOR nespal[256];

void m_savepalette()
{
 if (!romloaded) {msg.error("ROM not loaded"); return; }
 if (!nv) return;
 FILEIO f;
 f.create("aNES.pal");
 f.write(nespal,256*sizeof(COLOR));
 f.close();
 msg.printf(1,"NES Palette saved to aNES.pal");
}




//----------------------------------------------------
//loading and saving game state

struct m6502regs
{
 word pc;
 word af;
 byte x;
 byte y;
 byte s;
 void save() {pc=m6502pc; af=m6502af; x=m6502x; y=m6502y; s=m6502s;}
 void restore() {m6502pc=pc; m6502af=af; m6502x=x; m6502y=y; m6502s=s;}
};

class nesstate
{
 //cpu stuff
 byte ram6000[0x2000]; //battery backed ram
 byte lowram[0x800];   //low ram
 m6502regs regs; //processor regs

 //ppu stuff
 NES_sprite spmem[64]; //sprite memory
 NES_patterntable pt[2]; //2 pattern tables
 NES_natable nat[4]; //4 name/attribute tables
 NES_palette bgpal;
 NES_palette spritepal;

 //ports and stuff
 byte port2000[0x10];
 byte port4000[0x20];

 //mmc stuff
 char mmcstate[32]; //memory to store mmc info

 //last screen context
 scontext lastsc;
 byte mirroring; //last mirroring

 public:
 void save()
  {
   memcpy(ram6000,ram+0x6000,0x2000); //copy battery mem
   memcpy(lowram,ram,0x800); //copy lowram
   regs.save();
   memcpy(spmem,spritemem,64*sizeof(NES_sprite));
   memcpy(pt,ppu->pt,2*sizeof(NES_patterntable));
   bgpal=ppu->bgpal;
   spritepal=ppu->spritepal;
   memcpy(port2000,ram+0x2000,0x10);
   memcpy(port4000,ram+0x4000,0x20);
   port4000[0x15]=ns->enablebits;
   mmc->save(mmcstate);

   mirroring=nv->mirroring;
   lastsc=nv->sc.c; //store last screen context
   for (int i=0; i<4; i++)
    nat[i]=nv->ntc[i]->nt;
  };
 void restore()
  {
   memcpy(ram+0x6000,ram6000,0x2000); //copy battery mem
   memcpy(ram,lowram,0x800); //copy lowram
   regs.restore();
   memcpy(spritemem,spmem,64*sizeof(NES_sprite));
   memcpy(ppu->pt,pt,2*sizeof(NES_patterntable));
   for (int i=0; i<8; i++)
    nv->pt1kupdated[i]=1;
   ppu->bgpal=bgpal;
   ppu->spritepal=spritepal;
   memcpy(ram+0x2000,port2000,0x10);
   memcpy(ram+0x4000,port4000,0x20);
   ns->setenable(port4000[0x15]);
   mmc->restore(mmcstate);

   nv->setmirroring(mirroring);
   for (i=0; i<4; i++)  nv->ntc[i]->nt=nat[i];
   nv->sc.c=lastsc;
   nv->sc.c.ptn[0].restore();
   nv->sc.c.ptn[1].restore();
  };

 //read/write state to file
 void write(FILEIO &f) {f.write(this,sizeof(nesstate));}
 void read(FILEIO &f) {f.read(this,sizeof(nesstate));}
};

void m_savestate()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}
 if (!nv) return;

 char s[64];
 strcpy(s,romfilename);
 strcat(s,".sta");

 FILEIO f;
 f.create(s);
 nesstate *state=new nesstate; //create nes state
 state->save(); //record nes state
 state->write(f); //write to file
 delete state; //free state
 f.close();

 msg.printf(1,"Game state saved to %s",s);
}

void m_resume();
void m_loadstate()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}
 if (!nv) return;

 char s[64];
 strcpy(s,romfilename);
 strcat(s,".sta");

 FILEIO f;
 if (f.open(s)) {msg.error("Unable to open %s",s); return;}

 nesstate *state=new nesstate; //create nes state
 state->read(f); //read from file
 msg.printf(1,"Game state restored from %s",s);

 resetNEShardware();
 state->restore(); //restore nes state!
 nv->reset(); //reset nes video
 CPURunning=0;
 CPUpaused=1;
 m_resume();

 delete state; //free state
 f.close();
}






























