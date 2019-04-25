//functions for NES display
#include <string.h>
#include <stdlib.h>

#include "r2img.h"
#include "font.h"

#include "message.h"

#include "nesvideo.h"
#include "nes.h"
#include "mmc.h"
#include "slist.h"

#include "gui.h"

#include "config.h"

#include "dd.h"

#include "keyb.h"

#include "nessound.h"

#include "timing.h"
#include "file.h"

#include "prof.h"

#define FREE(x) if (x) {free(x); x=0;}
#define DELETE(x) if (x) {delete x; x=0;}


//color index from which all nes colors are based
#define CBASE 224

nesvideo *nv; //global nes video


#define min(a,b) (((a)<(b)) ? (a) : (b))
#define max(a,b) (((a)>(b)) ? (a) : (b))

extern int blah,blah2;


//--------------------------
//palette stuff

#include "nespal.h"

extern char palfile[];
void initializenespalette()
{
 FILEIO f;
 f.open(palfile);
 f.read(nespal,256*3);
 f.close();
// msg.printf(2,"NES Palette initialized");
}

//32 different indexes
void updatepalette(int palidx,byte paldata)
{
 setpalettenum(CBASE+palidx,&nespal[paldata]);
//  msg.printf(1,"palette[%d]=%X (%X,%X,%X)",palidx,paldata, nespal[paldata].r,nespal[paldata].g,nespal[paldata].b);
}

void nesvideo::refreshpalette()
{
 if (!paletteupdated) return;
 //update bg pals
 for (int i=0; i<32; i++)
  if (palupdateidx[i])
   {
    updatepalette(i,ppu->bgpal.c[i]);
    palupdateidx[i]=0;
   }
 paletteupdated=0;
}

void nesvideo::resetpalette()
{
 for (int i=0; i<32; i++) palupdateidx[i]=1;
 paletteupdated=1;
}

//--------------------------------
//pattern stuff

byte bitmap8x8::create(NES_pattern *np)
{
 memset(s,0,8*8); //clear it
 byte issolid=0;
 //create 256 color pattern out of nes bit pattern
 for (int y=0; y<8; y++)
  for (int x=0; x<8; x++)
   {
    if (np->low[y] &(0x80>>x)) s[y][x]|=1;
    if (np->high[y]&(0x80>>x)) s[y][x]|=2;
    issolid|=np->low[y]|np->high[y];
   }
 return !issolid;
}



void patterntable::setbank(int banknum,int pnum)
 {
  if (!nv || !nv->numpattern1k)
   {bank[banknum]=0; pbank[banknum]=0; return;}

  pnum%=nv->numpattern1k;
  bank[banknum]=pnum;          //set index
  pbank[banknum]=&nv->pt1k[pnum]; //point to pattern bank
//  msg.printf(2,"setbank %d=%d pbank=%X",banknum,pnum,pbank[banknum]);
 }


void patterntable::clear()
 {
  for (int i=0; i<4; i++) {bank[i]=0; pbank[i]=0;}
//  msg.error("clear");
 }


void patterntable::print()
{
 for (int i=0; i<4; i++)
  msg.printf(2,"bank[%d]=%d %X",i,bank[i],pbank[i]);
}

void pattern1k::create(int idx)
{
 //find 1k table to create patterns from
 NES_pattern *ptn;
 if (numvrom) ptn=&((NES_pattern *)VROM)[idx*64];
         else ptn=&((NES_pattern *)ppu)[idx*64];

 //create all optimized patterns
 for (int i=0; i<64; i++)
  p[i].create(&ptn[i]);
// blah++;
}

void patterntable::restore()
{
 for (int i=0; i<4; i++)
  setbank(i,bank[i]);
}

//------------------------------
//NES_natable

//rotation lookup
byte attriblookup[4][4];

void initattriblookup()
{
 for (int x=0; x<4; x++)
  for (int y=0; y<4; y++)
   {
    byte square=(x>>1) | (y&2);
    attriblookup[x][y]=square*2;
   }
}

inline byte NES_attributetable::getat(int x,int y)
  {return (a[y/4][x/4]>>attriblookup[x&3][y&3])&3;}

inline void NES_natable::drawtile(char *dest,int tx,int ty,patterntable &p)
  {
   p[nt.t[ty][tx]].draw_tile(dest,tx*8,ty*8,at.getat(tx,ty));
   pf.tiles.inc();
  }

//draws a name table at a x,y clipped to rectangle *clip
void NES_natable::draw(char *dest,int sx,int sy,lrect *clip,patterntable &p)
{
 int xstart,xend;
 int ystart,yend;

 xstart=max((clip->x1-sx)/8,0);
 xend=  min((clip->x2-sx)/8+1,32);
 ystart=max((clip->y1-sy)/8,0);
 yend=  min((clip->y2-sy)/8+1,30);

 CLIP C(dest,clip->x1,clip->y1,clip->x2,clip->y2);
 sx-=clip->x1; sy-=clip->y1;
 for (int y=ystart; y<yend; y++)
  for (int x=xstart; x<xend; x++)
   p[nt.t[y][x]].draw_tile(dest,sx+x*8,sy+y*8,at.getat(x,y));
}



//----------------------------------------------
//natable cache

void natablecache::createsurface(){s=new surface(32*8,30*8); totalupdate();}
void natablecache::freesurface() {delete s;}
natablecache::natablecache(){createsurface(); memset(&nt,0,sizeof(NES_natable));}
natablecache::~natablecache() {freesurface();}


//ensure that all tiles from sy1 to sy2 are updated
void natablecache::refresh(int sy1,int sy2)
{
 if (sy1<0) sy1=0;
 if (sy2>30) sy2=30;

 char *dest=0;

 for (int y=sy1; y<sy2; y++)
  if (lineupdated[y])
  {
   if (!dest) dest=s->lock(); if (!dest) return;
   for (int x=0; x<32; x++)
    if (updated[y][x])
     {
      nt.drawtile(dest,x,y,pt); //redraw individual tile to cache
      updated[y][x]=0;
     }
   lineupdated[y]=0;
  }

 //unlock surface
 if (dest) s->unlock();
}


void natablecache::totalupdate()
{
 memset(updated,1,30*32);
 memset(lineupdated,1,30);
}

//draw to screen at x,y clipped to clip
void natablecache::draw(int x,int y,lrect *clip)
{
 int y1=max(0,clip->y1-y);
 int x1=max(0,clip->x1-x);

 int x2=min(32*8,clip->x2-x);
 int y2=min(30*8,clip->y2-y);

 //make sure tiles in this range are updated
 refresh((y1/8),(y2/8)+1);
//refresh(0,30);

 //copy to screen!
 if (!s->blt(
  x1,y1, //ur coord within source
  x2,y2, //lower right coord within source
  screen,
  max(x,clip->x1),max(y,clip->y1)  //destination
  ))  totalupdate();
}


//pattern map cache, for holding recently used pattern maps
class patternmapcache
{
 patternmap *pm[8]; //cache of pattern maps
 public:
 //free all maps in cache
 void reset() {for (int i=0; i<8; i++) DELETE(pm[i]);}

 patternmapcache() {for (int i=0; i<8; i++) pm[i]=0;}
 ~patternmapcache() {reset();}

 //get a certain mattern map
 patternmap *get(patterntable &a,patterntable &b)
  {
   //see if it exists already
   for (int i=0; i<8; i++)
     if (pm[i] && pm[i]->from==a && pm[i]->to==b)
       {pm[i]->time=uu; return pm[i];}
   //doesn't exist so create one...
   for (i=0; i<8; i++)
    if (!pm[i]) {pm[i]=new patternmap; break;}
   //cache full
   if (i==8)
    {
     unsigned mintime=0xFFFFFFFF;
     int minmap;
     for (i=0; i<8; i++)
      if (pm[i]->time<mintime) {mintime=pm[i]->time; minmap=i;}
     i=minmap;
    }
   //set map
   pm[i]->create(a,b);
   return pm[i];
  }
};

patternmapcache pmcache;

void natablecache::setpatterntable(patterntable &newp)
 {
  if (numvrom) //vrom
   {
    if (pt==newp) return;
    apply(*pmcache.get(pt,newp));
    pt=newp;
   }
  else //vram
   {
    pt=newp;
    //have we updated any of the patterns in the bg?
    for (int i=0; i<4; i++)
     if (nv->pt1kupdated[pt.bank[i]]) break;
    if (i==4) return; //not been updated
    //create pattern translation and apply it
    patternmap pm;
    for (i=0; i<4; i++)
     memset(&pm.t[i*64],nv->pt1kupdated[pt.bank[i]],64);
    apply(pm);
//    totalupdate();
   }
 }


void patternmap::create(patterntable &a,patterntable &b)
{
 from=a; to=b; time=uu;
 if (!numvrom) {memset(t,0,256); return;}

 memset(t,0,256); //assume no updating

 for (int i=0; i<4; i++) //go through each 1k bank
//  if (a.bank[i]!=b.bank[i]) //there is a change in banks!
 if (a.pbank[i]!=b.pbank[i])
  {
   if (!b.pbank[i]) {memset(&t[i*64],0,64); continue;}
   if (!a.pbank[i]) {memset(&t[i*64],1,64); continue;}
   byte *ap=((byte *)VROM)+a.bank[i]*1024;
   byte *bp=((byte *)VROM)+b.bank[i]*1024;

   for (int j=0; j<1024; j++) //find changes in this 1k bank
     if (ap[j]!=bp[j]) t[i*64+(j/16)]=1;
  }
// msg.printf(2,"pmap create!");
}


void natablecache::apply(patternmap &pm)
{
 for (int y=0; y<30; y++)
  for (int x=0; x<32; x++)
   if (pm.t[nt.nt.t[y][x]])
    {
     lineupdated[y]=1;
     updated[y][x]=1;
    }
}



//------------------------------------------
//nesvideo draw bg

//(x,y)-(x2,y2) is the region to draw bg to
void nesvideo::drawbg(char *dest,lrect *r,byte sprites)
{
 //create clipping rectangle
 lrect clip;
 //set left/right clipping
 clip.x1=r->x1; clip.x2=r->x2;

 if (!(ram[0x2001]&2)) //clip left/right
  {
   clip.x1+=8; clip.x2-=8;

   //draw black rects on both sides
   if (sprites==(ram[0x2001]&4)/4)
    {
     drawrect(screen,0,r->x1,r->y1,8,yw);
     drawrect(screen,0,r->x2-8,r->y1,8,yw);
    }
  }

 scontext *c=sc.sc;
 //go through all sections
 for (int i=0; i<sc.scnum; i++,c++)
  if (c->height>0 && c->sprites==sprites) //if section has a height
  {
   clip.y1=r->y1+max(c->line,0);
   clip.y2=min(r->y1+c->line+c->height,r->y2);

   //scroll positions
   int sx=r->x1-c->sx;
   int sy=r->y1-c->sy-STARTFRAME; //for top clipping

   int natable=c->natable&3;
   patterntable &pt=c->ptn[c->bgpt]; //find pattern table to use for this section

   if (!c->bg) //is bg turned off?
     drawrect(screen,0,clip.x1,clip.y1,clip.width(),clip.height()); //draw black rectangle
     else
   if (pt==ntc[0]->pt) //if samepattern table...
   { //use fast blitting from cache
    ntc[natable]  ->draw(sx,sy,&clip);
    ntc[natable^1]->draw(sx+32*8,sy,&clip);
    ntc[natable^2]->draw(sx,sy+30*8,&clip);
    ntc[natable^3]->draw(sx+32*8,sy+30*8,&clip);
   } else
   { //draw natables tile by tile
    ntc[natable]  ->nt.draw(dest,sx,sy,&clip,pt);
    ntc[natable^1]->nt.draw(dest,sx+32*8,sy,&clip,pt);
    ntc[natable^2]->nt.draw(dest,sx,sy+30*8,&clip,pt);
    ntc[natable^3]->nt.draw(dest,sx+32*8,sy+30*8,&clip,pt);
   }
 }

}


//------------------------------------------------------------
// Sprites

inline void NES_sprite::draw_8x8(char *dest,patterntable &pt)
{
 if (x<SCREENX && y<SCREENY+STARTFRAME)
  pt[p].draw_sprite(dest,x,y+1-STARTFRAME,flipx|(flipy<<1),behindbg,attrib);
}

inline void NES_sprite::draw_8x16(char *dest,patterntable *ptn)
{
 if (x<SCREENX && y<SCREENY+STARTFRAME+8)
 {
  int pnum=p&(~1)^flipy;
  patterntable &pt=ptn[p&1]; ////nv->sc.biggestsc->ptn[p&1]; //get pattern from last screen context

  pt[pnum].  draw_sprite(dest,x,y+1-STARTFRAME,flipx|(flipy<<1),behindbg,attrib);
  pt[pnum^1].draw_sprite(dest,x,y+1+8-STARTFRAME,flipx|(flipy<<1),behindbg,attrib);
 }
}

inline void nesvideo::drawsprites_8x8(char *dest)
{
 //find sprite pattern table to show...
 patterntable &p=sc.biggestsc->ptn[sc.biggestsc->spritept];//(ram[0x2000]&8) ? 1 : 0];

 //draw every sprite
 for (int i=63; i>=0; i--) spritemem[i].draw_8x8(dest,p);
}

inline void nesvideo::drawsprites_8x16(char *dest)
{
 //find sprite pattern table(s) to show...
 patterntable *pt=sc.biggestsc->ptn;

// NES_sprite sp[64]; //temp sprite area
// memcpy(sp,spritemem,256);
// qsort(sp,64,sizeof(NES_sprite),(qsortfuncptr)spritecmp);

 //draw every sprite
 for (int i=63; i>=0; i--)  spritemem[i].draw_8x16(dest,pt);

}

void nesvideo::drawsprites(char *dest,lrect *r)
{
 //force clipping
 CLIP clip(dest,r->x1,r->y1,r->x2,r->y2);
  //draw sprites
 if (ram[0x2000]&32) //8x16 sprites ?
        drawsprites_8x16(dest);
  else  drawsprites_8x8(dest);
}


//---------------------------------
//main video interface

void cleardesktop() {if (nv) nv->forcedesktopfill=16;}

void nesvideo::resetmirroring()
{
 if (romhdr.fourscreen)
  {
   ntc[0]=realntc[0];
   ntc[1]=realntc[1];
   ntc[2]=realntc[2];
   ntc[3]=realntc[3];
  }
  else
 if (mirroring==VMIRROR)
  {
   ntc[0]=realntc[0];
   ntc[1]=realntc[1];
   ntc[2]=realntc[0];
   ntc[3]=realntc[1];
//   msg.printf(1,"Vertical mirroring set");
  } else
  {
   ntc[0]=realntc[0];
   ntc[1]=realntc[0];
   ntc[2]=realntc[1];
   ntc[3]=realntc[1];
//   msg.printf(1,"Horizontal mirroring set");
  }
}

void nesvideo::setmirroring(int type)
{
 if (mirroring==type) return; //ignore it
 mirroring=type;
 resetmirroring();
}


void nesvideo::createnatablecaches()
{
 //create name/attribute table caches
 for (int i=0; i<numrealntc; i++)
  realntc[i]=new natablecache();

// msg.printf(2,"Name table caches created: %dK",32*8*30*8/1024);
 resetmirroring();
}

void nesvideo::freenatablecaches()
{
 for (int i=0; i<numrealntc; i++)
   DELETE(realntc[i]);
// msg.printf(2,"Name table caches freed");
}

void nesvideo::createpatterntables()
{
 numpattern1k=numvrom*8;   //8 per 8k of table
 if (!numvrom) numpattern1k=8;

 //allocate pattern tables
 pt1k=(pattern1k *)calloc(numpattern1k,sizeof(pattern1k));

 //create pattern tables from vrom/vram
 for (int i=0; i<numpattern1k; i++) pt1k[i].create(i);

// msg.printf(2,"Pattern table caches created %d",numpattern1k);
}

void nesvideo::freepatterntables()
{
 FREE(pt1k);
 numpattern1k=0;
// msg.printf(2,"Pattern table caches freed");
}


void nesvideo::reset()
{
 resetpalette();
 for (int i=0; i<numrealntc; i++)
  realntc[i]->totalupdate();
 for (i=0; i<8; i++)
  pt1kupdated[i]=1;
// msg.printf(1,"NES video reset");
}

void nesvideo::clear()
{
 //clear context list (and its default)
 sc.clear();

 //set default vrom banks
 sc.c.ptn[0].setbank4k(0);
 sc.c.ptn[1].setbank4k(1);

 //set default natable pattern tables
 for (int i=0; i<numrealntc; i++)
  realntc[i]->setpatterntable(sc.c.ptn[0]);

 reset();
}


static DLGPOS dlgpos;
nesvideo::nesvideo(char *romfile):
 GUIcontents(256,224),maximized(0),pt1k(0)
{
 if (nv) { delete nv->parent;} //delete old video
 nv=this;
 dotakesnapshot=0;
 xw=width(); yw=height();

 //if 4screen rom, then set up 4 real nametables
 numrealntc=romhdr.fourscreen ? 4 : 2;
 for (int i=0; i<4; i++) realntc[i]=0;

 //set default mirroring
 setmirroring(romhdr.mirroring);

 initattriblookup();
 createpatterntables();
 createnatablecaches();

 //clear context list (and its default)
 pmcache.reset();

 clear(); //set it all from beginning

 //wrap us up in box
 GUImaximizebox *p=new GUImaximizebox(guiroot,romfile,this,0,0);
 dlgpos.open(p);
 if (SCREENX<=320)   p->maximize();

 msg.printf(2,"NES video initialized: %dx%d",xw,yw);
 cleardesktop();
}

nesvideo::~nesvideo()
{
 cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn

 freenatablecaches();
 freepatterntables();
 msg.printf(2,"NES video destroyed");
 nv=0;

 freerom();
 dlgpos.close((GUIbox *)parent);

 //clear context list (and its default)
 sc.clear();
 pmcache.reset();
}

int ccx,ccy;
int docolorcheck;

extern volatile byte inemu;
extern byte showvframeinfo;
extern byte IRQline;
void viewcolor(byte colornum);
void takesnapshot(char *dest,lrect &r);

extern int guienabled;
//update screen
void nesvideo::draw(char *dest)
{
//return;
 if (guienabled) pf.gui_timer.leave();

 pf.nesdraw_timer.enter();

 cfg->set(CFG_NOFILLEDDESKTOP,maximized);

 if (!sc.scnum || !sc.biggestsc) {fill(0); return;} //no contexts to draw!

 //update name caches' pattern table settings
 patterntable &pt=sc.biggestsc->ptn[sc.biggestsc->bgpt];
 for (int i=0; i<numrealntc; i++)
  realntc[i]->setpatterntable(pt);

 //update patterntables
 if (!numvrom)
  {
   for (int i=0; i<8; i++)
    if (pt1kupdated[i]) {pt1k[i].create(i); pt1kupdated[i]=0;}
  }

 //clear screen if necessary
 if (forcedesktopfill>0) {fill(0); forcedesktopfill--;}

 //find x,y coords of where to draw nes screen
 lrect r;
 r.x1=x1+(width()-xw)/2; if (r.x1<0) r.x1=0;
 r.y1=y1+(height()-yw)/2; if (r.y1<0) r.y1=0;
 r.x2=r.x1+xw;
 r.y2=r.y1+yw;

 //draw background that should contain sprites
 drawbg(dest,&r,1);

 //draw sprites
 drawsprites(dest,&r);

 //draw background that should not contain sprites
 drawbg(dest,&r,0);

 pf.nesdraw_timer.leave();

 if (dotakesnapshot)
 {
  takesnapshot(dest,r);
  dotakesnapshot=0;
 }


 //erase edges
 if (guienabled)
  {
   drawrect(dest,0,x1,y1,width(),r.y1-y1);   //top
   drawrect(dest,0,x1,r.y2,width(),y2-r.y2); //bottom
   drawrect(dest,0,x1,r.y1,r.x1-x1,r.y2-r.y1);   //left
   drawrect(dest,0,r.x2,r.y1,x2-r.x2,r.y2-r.y1); //right
  }

 if (docolorcheck)
 {
  byte c=dest[(ccy+y1)*PITCH+(ccx+x1)];
  viewcolor(c-CBASE);
  docolorcheck=0;
 }

//font[0]->printf(50,20,"ram[0x2000]=%X",ram[0x2000]);
//font[0]->printf(50,30,"ram[0x2001]=%X",ram[0x2001]);
//font[0]->printf(50,50,"%d",sl.num);

if (showvframeinfo)
 {
/*  font[3]->printf(50,20,"hblankcycles=%d",HBLANKCYCLES);
  font[3]->printf(50,30,"totallines=%d",TOTALLINES);
  font[3]->printf(50,40,"vblanklines=%d",VBLANKLINES);*/
  font[3]->printf(50,50,"sprite[0].y=%d",spritemem[0].y);

  sl.print(50,60);
  sc.print(200,50);
 }
 ns->print();

//reenter gui timing
if (guienabled) pf.gui_timer.enter();
}

int nesvideo::keyhit(char kbscan,char key)
{
 /*if (key==',') {HBLANKCYCLES--; return 1;}
 if (key=='.') {HBLANKCYCLES++; return 1;}
 if (key=='n') {VBLANKLINES--; return 1;}
 if (key=='m') {VBLANKLINES++; return 1;}                               */
 if (kbscan==KB_BKSP) {realntc[0]->totalupdate(); realntc[1]->totalupdate();}
 return mmc ? mmc->keyhit(kbscan,key) : 0;
}

int isviewcoloropen();

#include "mouse.h"
GUIrect * nesvideo::click(mouse &m)
{
 ccx=m.x-x1;
 ccy=m.y-y1;

 //color check
 if (kbstat&KB_SHIFT)
  {
   int x=ccx,y=ccy+STARTFRAME;
   int ysize=(ram[0x2000]&32) ? 16 : 8;
   for (int i=0; i<64; i++)
   {
    NES_sprite *s=&spritemem[i];
    if (x>=s->x && x<s->x+8 && y>=s->y && y<s->y+ysize)
      { //sprite hit!
        msg.printf(2,"Sprite #%d: (%d,%d) p=%X at=%d %d %c%c%c",i,s->x,s->y,s->p,s->attrib,
         s->unknown,
          s->flipx ? 'X' : ' ',s->flipy ? 'Y' : ' ',s->behindbg ? 'B' : ' ');
       return 0;
      }
    }
   return 0;
  }

 if ((kbstat&KB_CTRL) || isviewcoloropen()) {docolorcheck=1; return 0;}

// msg.printf(2,"line=%d",ccy+STARTFRAME);

 return 0;
}





//--------------------------
//GUI related bullshit
void enablegui();
void nesvideo::restore()
 {
//msg.printf(2,"nesvideo restore");
  cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn
  maximized=0;
  resize(xw,yw);
  enablegui();
  cleardesktop();
 }

void nesvideo::maximize()
 {
//msg.printf(2,"nesvideo maximize");
  cfg->set(CFG_NOFILLEDDESKTOP,1); //force desktop to not be drawn
  moveto(0,0);
  maximized=1;
  resize(SCREENX,SCREENY);
  cleardesktop();
 }
void nesvideo::resize(int txw,int tyw)
{
//msg.printf(2,"nesvideo resize %d,%d",txw,tyw);
 GUIrect::resize(txw,tyw);
 if (maximized && parent)
   ((GUImaximizebox *)parent)->reposmaxbutton();
}
