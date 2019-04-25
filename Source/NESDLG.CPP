#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "types.h"

#include "r2img.h"
#include "font.h"
#include "dd.h"

#include "mouse.h"
#include "message.h"
#include "gui.h"
#include "guimenu.h"
#include "guicolor.h"

#include "file.h"

#include "keyb.h"

#include "config.h"

#include "guivol.h"

#include "nesvideo.h"
#include "sound.h"
#include "nessound.h"

#include "slist.h"

#include "timing.h"

void loadrom(char *filename);

void enablegui();
//------------------------------
//dir listing box

#ifdef __WATCOMC__
#define USERENTRY
#endif
#ifdef _MSC_VER
#define USERENTRY __cdecl  
#endif
#ifdef __BORLANDC__
#define USERENTRY _USERENTRY
#endif

typedef int (USERENTRY *qsortfuncptr)(const void *,const void *);

int filldirlistbox(char *filename,class GUIdirlistbox *b);

int USERENTRY dirlistcmp(char **a,char **b) {return stricmp(*a,*b);}

class GUIdirlistbox:public GUIstringlistbox
{
 char path[128];
 public:
 void sortdirlist() //sort all entries in list
 {
  qsort((void *)getitems(),getnumitems(),4,(qsortfuncptr)dirlistcmp);
 }

 void setpath(char *_path)
 {
  strcpy(path,_path);    //set path
  freeitems();
  enumdir(path,(DIRFUNCPTR)filldirlistbox,this); //fill dir list box
  sortdirlist();
 }

 GUIdirlistbox(GUIrect *p,int x,int y,int xw,int iy,char *_path)
  :GUIstringlistbox(p,x,y,xw,iy,10)
  {
   setpath(_path);
  }

 virtual char *getname() {return "guidirlist";}
 virtual int acceptfocus() {return 1;}
};

int filldirlistbox(char *filename,GUIdirlistbox *b)
{
 int num=b->getnumitems();
 char **s=(char **)b->resizeitems(num+1);
 s[num]=strdup(filename);
 return 1;
}




//----------------------
//   load rom
//----------------------
class loaddlg:public GUIcontents
{
 GUIstringlistbox *list;

 public:
 loaddlg():GUIcontents(140,150)
 {
//  list=new GUIstringlistbox(this,5,5,width()-10,9,10);
  list=new GUIdirlistbox(this,5,5,width()-10,14,"*.nes");
  list->setsel(0);
 }
 virtual char *getname() {return "loadrom";}

 int sendmessage(GUIrect *c,int guimsg)
  {
   if (guimsg==GUIMSG_OK || guimsg==GUIMSG_LISTBOXDBLCLICKED)
    {
     loadrom(list->getselptr());
     return 1;
    }
   return 1;
  };
 virtual void draw(char *dest) {fill(CLR_BOX); GUIrect::draw(dest);}
 virtual int acceptfocus() {return 1;}

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~loaddlg() {pos.close((GUIbox *)parent);}
};
DLGPOS loaddlg::pos;

void m_load()
{
 loaddlg::pos.open(new GUIonebuttonbox(guiroot,"Load .NES ROM",new loaddlg(),"Load",0,0));
 enablegui();
}

//--------------------------------------------------



//----------------------
//   view pattern tables
//----------------------

//shows a particular pattern table
class patterntableview: public GUIrect
{
 int pnum; //index to pattern table...
 int attrib; //attrib to use

 public:

 patterntableview(GUIrect *_parent,int _pnum)
   :GUIrect(_parent,0,0,16*8,16*8) {pnum=_pnum; attrib=0;}

 virtual void draw(char *dest)
  {
   if (!nv) return;
   if (!nv->sc.biggestsc) return;
   patterntable &pt=nv->sc.biggestsc->ptn[pnum];

   CLIP clip(dest,x1,y1,x2,y2);

   for (int i=0; i<256; i++)
     pt[i].draw_tile(dest,(i&15)*8,(i/16)*8,attrib);
  }
 void incattrib()
 {
  attrib++;
  attrib&=7;
 }
 virtual GUIrect *click(mouse &m)
 {
  incattrib();
  return 0;
 }

 virtual char *getname() {return "patterntable";}
};



class patterndlg:public GUIcontents
{
 patterntableview *p1,*p2;

 public:
 patterndlg():GUIcontents(16*8,16*8*2)
 {
  p1=new patterntableview(this,0);
  p2=new patterntableview(this,1);
  p2->moveto(p1->x1,p1->y2);//+8);
 }
 virtual char *getname() {return "patterndlg";}


 int sendmessage(GUIrect *c,int guimsg)
  {return 1; };
// virtual void draw(char *dest) {fill(CLR_BOX); GUIrect::draw(dest);}

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~patterndlg() {pos.close((GUIbox *)parent);}
};
DLGPOS patterndlg::pos;

void m_patternview()
{
 patterndlg::pos.open(new GUIbox(guiroot,"Pattern tables",new patterndlg(),0,0));
 enablegui();
}


//-------------------------------------------------------------



//----------------------
//   view name tables
//----------------------

#include "nes.h"
class namedlg:public GUIcontents
{
 public:
 namedlg():GUIcontents(32*2*8,30*8) {}
 virtual char *getname() {return "nametabledlg";}

 int sendmessage(GUIrect *c,int guimsg){ return 1; };
 virtual void draw(char *dest)
  {
   if (!nv) return;
   if (SCREENX<=320) return;

   lrect clip={x1,y1,x2,y2};
   nv->realntc[0]->s->blt(0,0,32*8,30*8,dest,x1,y1);
   nv->realntc[1]->s->blt(0,0,32*8,30*8,dest,x1+32*8,y1);

//   nv->realntc[0]->draw(x1,y1,&clip);
//   nv->realntc[1]->draw(x1+32*8,y1,&clip);

//   CLIP clip(dest,x1,y1,x2,y2);
   //find screen pattern table to show...
//   pattern *p=(pattern *)&nv->ptn[(ram[0x2000]&16) ? 1 : 0];
//   ppu->nat[0].draw(dest,0,0,p);
//   ppu->nat[(nv->mirroring==VMIRROR) ? 1 : 2].draw(dest,32*8,0,p);

//   ppu->nat[2].draw(dest,0,3*8,p);
//   ppu->nat[3].draw(dest,32*8,30*8,p);

  }

 virtual GUIrect *click(mouse &m)
 {
  int x=m.x-x1,y=m.y-y1;

//  msg.printf(1,"nametable=%d (%d,%d)",x<32*8 ? 0 : ((nv->mirroring==VMIRROR) ? 1 : 2),
//       x%(32*8),y);
  return 0;
 }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~namedlg() {pos.close((GUIbox *)parent);}
};
DLGPOS namedlg::pos;

void m_nametableview()
{
 namedlg::pos.open(new GUIbox(guiroot,"Name tables",new namedlg(),0,0));
 enablegui();
}




//----------------------
//   view palette
//----------------------


extern COLOR nespal[256];


//first index of nes color
#define CBASE 224

#define CSIZE 48



int cfontnum[3]={5,2,7}; //rgb fonts
class colorcomponent:public GUIrect
{
 byte type; //0=r 1=g 2=b
 byte val;

 GUIstatictext *str;
 GUIscrollbar *sb;


 public:
 colorcomponent(GUIrect *p,int x,int y,int xw,int yw,byte _type):
   GUIrect(p,x,y,x+xw,y+yw), type(_type)
   {
    str=new GUIstatictext(this,cfontnum[type],"",0,4);

    sb=new GUIhscrollbar(this,14,2,width()-16);
    sb->setrange(0,0xFF);
    val=0;
   };

 void setval(byte value)
 {
  if (value==val) return;
  sb->setpos(value);
  char s[10];
  sprintf(s,"%02X",value);
  str->settext(s);
  val=value;
  parent->sendmessage(this,value);
 }

 virtual void draw(char *dest)
  {
   setval(sb->getpos());
   GUIrect::draw(dest);
  }

 virtual int acceptfocus() {return 1;}
};


class colorviewdlg:public GUIcontents
{
 byte cnum; //color number 0-23
 byte cidx; //color index 0-256
 COLOR *c;
 char indexstr[16];

 colorcomponent *cc[3];

 public:
 void setcolor(byte colornum)
  {
   if (!nv) return;
   cnum=colornum;
   cidx=ppu->bgpal.c[cnum];
   c=&nespal[cidx];
   sprintf(indexstr,"%02X",cidx);

   char s[32];
   sprintf(s,"%s Color #%X",cnum<16 ? "Bg" : "Sprite",cnum&0xF);
   if (parent) ((GUIbox *)parent)->settitle(s);

   cc[0]->setval(c->r);
   cc[1]->setval(c->g);
   cc[2]->setval(c->b);
  }

  //update palette
 void updatepalette()
 {
  for (int i=0; i<32; i++)
   if (ppu->bgpal.c[i]==cidx) nv->palupdateidx[i]=1;
  nv->paletteupdated=1;
//  msg.printf(2,"pal updated");
 }

 colorviewdlg():GUIcontents(CSIZE+100,CSIZE)
  {
   indexstr[0]=0; c=0;
   for (int i=0; i<3; i++)
    cc[i]=new colorcomponent(this,CSIZE,CSIZE*i/3,width()-CSIZE,CSIZE/3,i);
  }
 virtual char *getname() {return "colorviewdlg";}

 virtual int acceptfocus() {return 1;}
 int sendmessage(GUIrect *t,int guimsg)
  {
   if (!c || !nv) return 0;
   if (t==cc[0]) c->r=(byte)guimsg;
   if (t==cc[1]) c->g=(byte)guimsg;
   if (t==cc[2]) c->b=(byte)guimsg;
   updatepalette();
   return 1;
  };
 virtual void draw(char *dest)
  {
   if (!nv) return;

   fill(CLR_BOX);
   drawrect(dest,cnum+CBASE,x1+4,y1+4,CSIZE-8,CSIZE-8);
   font[1]->drawcentered(indexstr,dest,x1+CSIZE/2,y1+CSIZE/2-5);

   GUIrect::draw(dest);
  }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~colorviewdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS colorviewdlg::pos;

void viewcolor(byte colornum)
{
 if (!colorviewdlg::pos.opened)
        colorviewdlg::pos.open(new GUIbox(guiroot,"",new colorviewdlg(),0,0));
 ((colorviewdlg *)colorviewdlg::pos.opened->contents)->setcolor(colornum);
 enablegui();
}

int isviewcoloropen()
{
 return colorviewdlg::pos.opened ? 1 : 0;
}

//---------------------------------

class paletteviewdlg:public GUIcontents
{
 public:
 paletteviewdlg():GUIcontents(16*16,16*2) {}
 virtual char *getname() {return "paletteviewdlg";}

 int sendmessage(GUIrect *c,int guimsg){ return 1; };
 virtual void draw(char *dest)
  {
   if (!nv) return;

   int x;
   //bg pal
   for (x=0; x<16; x++)
    drawrect(dest,CBASE|x,x1+x*16,y1,16,16);

   //sprite pal
   for (x=0; x<16; x++)
    drawrect(dest,CBASE|16|x,x1+x*16,y1+16,16,16);
  }
 virtual GUIrect *click(mouse &m)
  {
   if (!nv) return 0;
   viewcolor(((m.x-x1)/16)+((m.y-y1)/16)*16);
   return 0;
  }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~paletteviewdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS paletteviewdlg::pos;

void m_paletteview()
{
 paletteviewdlg::pos.open(new GUIbox(guiroot,"Palettes",new paletteviewdlg(),0,0));
 enablegui();
}

///----------------------


class nestimingdlg:public GUIcontents
{
 GUIintedit *hblank, *vblank,*vfps;
 GUIstatictext *mhz;
 public:
 void setall()
 {
  hblank->set(HBLANKCYCLES);
  vblank->set(VBLANKLINES);
  vfps->set(TIMERSPEED);
 }

 nestimingdlg():GUIcontents(140,80)
 {
  hblank=new GUIintedit(this,"HBlank cycles: ",5,5,30,HBLANKCYCLES,10,999);
  vblank=new GUIintedit(this,"VBlank lines: ",5,23,30,VBLANKLINES,1,999);
  vfps=new GUIintedit(this,"Virtual FPS: ",5,41,30,TIMERSPEED,1,999);
  new GUIstatictext(this,3,"6502 Mhz: ",30,60);

  hblank->moverel((x2-10)-hblank->x2,0);
  vblank->moverel((x2-10)-vblank->x2,0);
  vfps->moverel  ((x2-10)-vfps->x2,0);
 }
 virtual char *getname() {return "nestimingdlg";}

 virtual int acceptfocus() {return 1;}

 int sendmessage(GUIrect *c,int guimsg)
  {
   if (guimsg==GUIMSG_EDITCHANGED)
    {
     if (c==hblank) {HBLANKCYCLES=hblank->get();}
     if (c==vblank) {VBLANKLINES=vblank->get();}
     if (c==vfps) {settimerspeed(vfps->get());}
    }
   return 1;
  };
 virtual void draw(char *dest)
  {
   setall();
   fill(CLR_BOX); GUIrect::draw(dest);
   font[1]->printf(x1+85,y1+60,"%5.2f",((float)TIMERSPEED*CYCLESPERTICK)/1000000);
  }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~nestimingdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS nestimingdlg::pos;

void m_nestimingview()
{
 nestimingdlg::pos.open(new GUIbox(guiroot,"NES Timing",new nestimingdlg(),0,0));
 enablegui();
}


//-----------------------------------------


//---------------------------------

class waveoutputdlg:public GUIcontents
{
 public:
 waveoutputdlg():GUIcontents(BLOCK_LENGTH/2,48) {}
 virtual char *getname() {return "waveoutputdlg";}

 int sendmessage(GUIrect *c,int guimsg){ return 1; };
 virtual void draw(char *dest)
  {
   if (!ns) return;
   short *t=ns->mixingbuf;
   int yb=(y1+y2)/2;

   for (int i=0; i<BLOCK_LENGTH/2; i++,t+=2)
    {
     int height=abs(*t)*32/0x2000+1;
     if (*t<0) drawrect(dest,0x61,x1+i,yb-height,1,height);
          else drawrect(dest,0x61,x1+i,yb,1,height);
    }
  }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~waveoutputdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS waveoutputdlg::pos;

void m_waveoutputview()
{
 waveoutputdlg::pos.open(new GUIbox(guiroot,"Wave output",new waveoutputdlg(),0,0));
 enablegui();
}














































