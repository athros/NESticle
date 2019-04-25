//routines for intepreting split screens and the like
#include <stdlib.h>
#include <string.h>

#include "message.h"

#include "nes.h"
#include "nesvideo.h"
#include "slist.h"
#include "mmc.h"

#include "r2img.h"
#include "font.h"
#include "dd.h"

#include "m6502.h"



void sevent::print(int x,int y)
{
 if (type>=SE_VROM1K)
    font[5]->printf(x,y,"line=%d vrom1k%d=%d",line,type-SE_VROM1K,data);
  else
 switch (type)
 {
  case SE_NATABLE: font[1]->printf(x,y,"line=%d natable=%d",line,data); break; //t[0],pc); break;
  case SE_SCROLLX: font[1]->printf(x,y,"line=%d scrollx=%d",line,data); break; //t[0],pc); break;
  case SE_SCROLLY: font[1]->printf(x,y,"line=%d scrolly=%d",line,data); break; //t[0],pc); break;
  case SE_SPRITE:  font[1]->printf(x,y,"line=%d sprites=%s",line,data ? "on" : "off"); break; //t[0],pc); break;
  case SE_BG:  font[1]->printf(x,y,"line=%d bg=%s",line,data ? "on" : "off"); break; //t[0],pc); break;
  case SE_FRAMESTART:  font[2]->printf(x,y,"line=%d vframe start",line); break; //t[0],pc); break;
  case SE_BGPT: font[1]->printf(x,y,"line=%d bgptable=%d",line,data); break; //t[0],pc); break;
  case SE_SPRITEPT: font[1]->printf(x,y,"line=%d spriteptable=%d",line,data); break; //t[0],pc); break;
  case SE_IRQLINE: font[2]->printf(x,y,"line=%d IRQ!",line,data); break; //t[0],pc); break;

  case SE_VROM8K: font[5]->printf(x,y,"line=%d vrom8k=%d",line,data); break; //t[0],pc); break;
  case SE_VROM4K0: font[5]->printf(x,y,"line=%d vrom4k0=%d",line,data); break; //t[0],pc); break;
  case SE_VROM4K1: font[5]->printf(x,y,"line=%d vrom4k1=%d",line,data); break; //t[0],pc); break;

 }
}

void seventlist::print(int x,int y)
{for (int i=0; i<num; i++,y+=10)  s[i].print(x,y);}

int getvblankscanline();
extern byte hitflag;
void seventlist::add(sevent &t)
 {
  if (num>0 && s[num-1]==t) return;
  if (num>511) return;
  t.line=getscanline(); //(getscanline()+3)&(~7);
  s[num++]=t;
 } //add event



//integrate event into screen context
void scontext::operator +=(sevent &e)
{
 int i;
 switch (e.type)
  {
   case SE_NATABLE: natable=e.data; break;
   case SE_SCROLLX: sx=e.data; break;
   case SE_SCROLLY:
    if (e.data<=239) sy=e.data;
   break;
   case SE_SPRITE: sprites=e.data ? 1 : 0; break;
   case SE_BG: bg=e.data ? 1 : 0; break;
   case SE_BGPT: bgpt=e.data ? 1 : 0; break;
   case SE_SPRITEPT: spritept=e.data ? 1 : 0; break;
   case SE_VROM8K:
     for (i=0; i<4; i++)
     {
      ptn[0].setbank(i,e.data*8+i);
      ptn[1].setbank(i,e.data*8+i+4);
     }
    break;
   case SE_VROM4K0:
   case SE_VROM4K1:
     for (i=0; i<4; i++)
      ptn[e.type-SE_VROM4K0].setbank(i,e.data*4+i);
    break;
   case SE_VROM1K:
   case SE_VROM1K1:
   case SE_VROM1K2:
   case SE_VROM1K3:  ptn[0].setbank(e.type-SE_VROM1K,e.data); break;
   case SE_VROM1K4:
   case SE_VROM1K5:
   case SE_VROM1K6:
   case SE_VROM1K7:  ptn[1].setbank(e.type-SE_VROM1K4,e.data);  break;
  }
 if (line) line=e.line;
// msg.printf(2,"%d",line);
}

void scontextlist::add(scontext &t)
{
 if (scnum>0 && t==sc[scnum-1]) return; //dont add if last was the same
 sc[scnum++]=t;
}

extern byte doIRQ;
//create contexts list from eventlist
void scontextlist::create(seventlist &el)
{
 scnum=0;

 //create first context
 c.setline(0);    //top of screen
// add(c);      //add it

 for (int i=0; i<el.num; i++) //process all events
   if (el.s[i].line<=c.line+4) c+=el.s[i]; //does event apply to current context?
     else
     {
      add(c); //add current context to list
      c.setline(el.s[i].line); //start context at new line
      c+=el.s[i];      //integrate event
     }
 add(c);

 //add screen height to end
 sc[scnum].line=nv->yw;

 //force lines to boundaries
// if (!doIRQ)
//  for (i=0; i<scnum; i++)
//   sc[i].line=(sc[i].line+2)&(~7);
//  if (sc[i].line)
//    sc[i].line=(sc[i].line+16)&(~7);

 //resolve all heights
 for (i=0; i<scnum; i++)
  sc[i].setheight(sc[i+1].line);

 //find context with the greatest height
 int maxheight=0;
 for (i=0; i<scnum; i++)
  if (sc[i].height>maxheight)
   {maxheight=sc[i].height; biggestsc=&sc[i];}

 //set default for next time
 c=sc[scnum-1];
}






void scontext::print(int x,int y)
{
 font[this==nv->sc.biggestsc ? 2 : 1]->printf(x,y,"line=%d ht=%d scroll=%d,%d na=%d sp=%d pt=%d",line,height,sx,sy,natable,sprites,bgpt);
}

void scontextlist::print(int x,int y)
{
 for (int i=0; i<scnum; i++,y+=10)
  sc[i].print(x,y);
}

















