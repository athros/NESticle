#include <stdlib.h>

#ifdef WIN95
#include "windows.h"
#endif

#ifdef DOS
extern unsigned ticks;
#endif
#include "types.h"

#include "prof.h"

#include "timing.h"

#include "message.h"

#include "r2img.h"
#include "font.h"
#include "dd.h"




anesprofile::anesprofile():
  updatetimer(200), //update every 100 ticks
  cycles(1000000.0)
  {};


void anesprofile::update()
{
 if (!updatetimer.check()) return; //dont update yet

 //calculate number of seconds since last update
 float dt=((float)updatetimer.dur)/((float)TIMERSPEED);

 tiles.update(dt);
 cycles.update(dt);
 frames.update(dt);
 vframes.update(dt);

 cpu_timer.update(dt);
 nesdraw_timer.update(dt);
 gui_timer.update(dt);
 input_timer.update(dt);
 pageflip_timer.update(dt);
 pageclear_timer.update(dt);

 sound_timer.update(dt);

 updatetimer.reset();
}

void anesprofile::draw(int x,int y)
{

/*
 tiles.draw("tiles/sec:",x,y); y+=10;
 frames.draw("frames/sec:",x,y); y+=10;
 vframes.draw("vframes/sec:",x,y); y+=10;
 cycles.draw("6502 Mhz:",x,y); y+=10;
 y+=10;

 pageclear_timer.draw("Pageclear:",x,y); y+=10;
 pageflip_timer.draw("Pageflip:",x,y); y+=10;
 nesdraw_timer.draw("NES rendering:",x,y); y+=10;
 gui_timer.draw("GUI rendering:",x,y); y+=10;
 cpu_timer.draw("6502 Emulation:",x,y); y+=10;
 input_timer.draw("Input devices:",x,y); y+=10;
  */

 pageclear_timer.drawmeter("Pageclear:",x,y,0xA0); y+=10;
 pageflip_timer.drawmeter("Pageflip:",x,y,0xA0); y+=10;
 gui_timer.drawmeter("GUI rendering:",x,y,0x40); y+=10;
 input_timer.drawmeter("Input devices:",x,y,0x30); y+=15;

 nesdraw_timer.drawmeter("NES rendering:",x,y,0x70); y+=10;
 cpu_timer.drawmeter("6502 Emulation:",x,y,0x20); y+=10;
 sound_timer.drawmeter("Sound:",x,y,0x10); y+=10;

// font[1]->printf(x,y,"%d",proftimer::gettime());
}

//-----------------------------------
//profcounter

void profcounter::draw(char *str,int x,int y)
{
 int sw=font[0]->getwidth(str);
 font[3]->draw(str,screen,x-sw-8,y);

 font[1]->printf(x,y,"%.02f",getrate());
}

//------------------------------------
//proftimer

unsigned gettime()
{
 #ifdef WIN95
 return timeGetTime();
 #endif
 #ifdef DOS
 return ticks*1000/TIMERSPEED;
 #endif
}

void proftimer::draw(char *str,int x,int y)
{
 int sw=font[0]->getwidth(str);
 font[3]->draw(str,screen,x-sw-8,y);

 font[1]->printf(x,y,"%5.02f%%",getrate());
}

extern int guienabled;
void proftimer::drawmeter(char *str,int x,int y,byte c)
{
 if (guienabled)
 {
  int sw=font[0]->getwidth(str);
  font[3]->draw(str,screen,x-sw-1,y);
 }

 int bw=(int)(getrate()+0.99)/2;

 if (bw<0) bw=0;
 if (bw>50) bw=50;

 drawrect(screen,c+3,x,y+1,bw,8);
 drawrect(screen,c+15,x+bw,y+1,50-bw,8);

}














