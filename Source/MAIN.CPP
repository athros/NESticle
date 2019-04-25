//6502 nes emulator front end
//main platform independant
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mem.h>
#include <malloc.h>
#include <direct.h>

#include "config.h"

#include "keyb.h"

#include "types.h"
#include "r2img.h"
#include "font.h"

#include "file.h"
#include "vol.h"
#include "mouse.h"
#include "message.h"

#include "dd.h"

#include "guivol.h"
#include "gui.h"

#include "uutimer.h"

#include "nesvideo.h"
#include "nessound.h"
#include "nes.h"
#include "timing.h"

#include "command.h"

#include "m6502\m6502.h"

#include "prof.h"

#include "sound.h"


char appname[]="NESticle";
char configfile[]="NESticle.cfg";
char palfile[]="NESticle.pal";


float version=0.2;

#ifdef WIN95
int SCREENX=640;
int SCREENY=480;
#else
int SCREENX=256;
int SCREENY=224;
#endif

anesprofile pf;

//config info
config *cfg;

//input devices
input *inputdevice[2];
inputdevicesettings *ids;

//sound sampling rate (default)
int SOUNDRATE=22050;


GUIVOL guivol;
int GUIVOL::size() {return sizeof(GUIVOL);}

FONT *font[16]; //fonts
COLORMAP *shadowmap; //shaded colormapping

volatile unsigned uu=1,su=1;

//buffer for messages
msgbuffer msg;

//root of a32
class ROOT *root=0;

//main root of gui tree
GUIroot *guiroot=0;

extern byte showcpuutilization;
extern int guienabled;
void enablegui();
void disablegui();
void togglegui();


int keyboard();
void initdefaultgui();


 //create all the fonts and colors
void createfonts()
{
 static byte fontcolor[8][2]=
  {
   {0xc6,0xd8}, //peach
   {0xf,0}, //white
   {6*16,0}, //green
   {19,0}, //grey
   {0,15}, //black
   {0x21,0}, //red
   {0xA3,0}, //pink
   {0x82,0} //blue
  };
 for (int i=0; i<8; i++)
  {
   font[i]=guivol.font->duplicate();
   font[i]->convertcolor((byte)0xca,fontcolor[i][0]);
   font[i]->convertcolor((byte)0xdb,fontcolor[i][1]);
  }
 }


void initializenespalette();
//initialize
int initgame()
{
 //set up keyboard handler
 set_keyboard_func(keyboard);

 //initialize input devices
 ids=&cfg->ids; //copy over input settings

 inputdevice[0]=newinputdevice(cfg->pinput[0]);
 inputdevice[1]=newinputdevice(cfg->pinput[1]);

 //load up graphics and shit
 if (!guivol.read("gui.vol")) return -1;

 //get palette
 initializenespalette();

 cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn

 //fixup pal to 8bit
 for (int i=0; i<256; i++)
 {
  guivol.pal->c[i].r<<=2;
  guivol.pal->c[i].g<<=2;
  guivol.pal->c[i].b<<=2;
 }

 setpalette(guivol.pal);

 //create fonts
 createfonts();

 //create root
 root=new ROOT;

 //create root of all GUI
 new GUIroot(root);

 initdefaultgui();
 return 0;
}


void m_stop();
void terminategame()
{
 m_stop();
 freerom();
 delete root;
 guivol.free();
 delete cfg;
}

void tickemu(); //tick emulation
void gametimer()
{
 uu++;
 input::refreshtimer();
 inputdevice[0]->read();
 inputdevice[1]->read();

 if (CPURunning || (CPUpaused && (CPUtrace || CPUtickframe>0)))
  {
   tickemu();
   if (CPUtickframe>0) CPUtickframe--;
  }
}


int keyboard()
{
// if (kbscan==KB_ESC) {quitgame(); return 1;}
 if (kbscan==KB_ESC) {togglegui(); return 1;}

 if (input::refreshkeyboard(kbscan)) return 1; //dont add key to queue

 if (kbscan&0x80) return 1; //release

 char key=scan2ascii(kbscan);
// if (root && root->keyhit(kbscan,key)) enablegui();
 if (root && root->keyhit(kbscan,key)) return 1;

 if (key==' ' && nv)
  {
   if (guienabled) {disablegui(); ((GUImaximizebox *)nv->parent)->maximize();}
    else {enablegui(); if (SCREENX>320) ((GUImaximizebox *)nv->parent)->restore();}
  }
 return 1;
}



void updatescreen()
{
 pf.input_timer.enter();
 input::refreshmain();
 pf.input_timer.leave();

 if (guienabled)
 {
  pf.gui_timer.enter();
   //draw everything
  root->draw(screen);
  m.draw(screen); //draw cursor
  pf.gui_timer.leave();
 } else if (nv) nv->draw(screen);


 pf.frames.inc();
 su++;

 pf.update();

 if (showcpuutilization)
 {
  pf.gui_timer.enter();
  pf.draw(SCREENX-60,50);
  pf.gui_timer.leave();
 }


 if (cfg->get(CFG_SHOWFPS))
  {
   drawrect(screen,0,SCREENX-70+23,40-1,30,10);
   font[3]->printf(SCREENX-70,40,"fps=%4.1f",pf.frames.getrate());
  }

//  font[0]->printf(SCREENX-150,80,"%X",m6502pc);

//  font[0]->printf(SCREENX-150,80,"%d",blah);
//font[0]->printf(SCREENX-150,80,"%d %d",blah,blah2);
}



//------------------------------
//       ROOT node
//------------------------------

ROOT::ROOT():GUIrect(0,0,0,SCREENX,SCREENY){}
ROOT::~ROOT() {}

void ROOT::resize(int xw,int yw)
{
 if (guiroot) guiroot->resize(xw,yw);
 if (nv && nv->maximized) nv->resize(xw,yw);
 GUIrect::resize(xw,yw);
}

void ROOT::refresh(int r,void *c)
 {for (GUIrect *g=child; g; g=g->next) g->refresh(r,c);};


 //-----------------




int cmd_loadrom(char *p)
{
 char s[128];
 if (sscanf(p,"%s",s)<1) return 0;
 loadrom(p);
 return 1;
}

int cmd_runrom(char *p)
{
 char s[128];
 if (sscanf(p,"%s",s)<1) return 0;
 loadrom(p);
 if (nv)
 {
  disablegui();
  ((GUImaximizebox *)nv->parent)->maximize();
  m_reset();
 }
 return 1;
}

void m_loadstate();
int cmd_restorerom(char *p)
{
 char s[128];
 if (sscanf(p,"%s",s)<1) return 0;
 loadrom(p);
 if (nv)
 {
  disablegui();
  ((GUImaximizebox *)nv->parent)->maximize();
  m_loadstate();
  if (!CPURunning) m_reset();
 }
 return 1;
}




