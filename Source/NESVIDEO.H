#ifndef _NESVIDEO_
#define _NESVIDEO_

#include "guirect.h"
#include "ppu.h"

extern "C" {
 void __cdecl draw_tile_asm(struct bitmap8x8 *b,char *dest,int x,int y,byte pal);
 void __cdecl draw_sprite_asm(struct bitmap8x8 *b,char *dest,int x,int y,int o,byte pal);
 void __cdecl draw_sprite_behind_asm(struct bitmap8x8 *b,char *dest,int x,int y,int o,byte pal);
};
#ifdef __WATCOMC__
#pragma aux draw_tile_asm modify[eax ecx edx];
#pragma aux draw_sprite_asm modify[eax ecx edx];
#endif

//8x8 bitmap (for pattern tables
struct bitmap8x8
{
 byte s[8][8];
 inline void draw_tile(char *dest,int x,int y,byte pal)
  {draw_tile_asm(this,dest,x,y,pal);} //draw as tile
 inline void draw_sprite(char *dest,int x,int y,int o,int behind,byte pal) //draw as sprite (transparency and orientation)
  {
   if (!behind)  draw_sprite_asm(this,dest,x,y,o,pal);
          else   draw_sprite_behind_asm(this,dest,x,y,o,pal);
  } //draw as sprite

 //create 256 bitmap from nes pattern
 byte create(NES_pattern *np); //returns 1 if all transparent pixels
};

//1K bank of optimized patterns
struct pattern1k
{
 bitmap8x8 p[64];      //64 patterns=1K of ppu memory
 void create(int idx); //create 64 patterns from vrom1k[idx]
};

struct patterntable
{
 byte        bank[4];  //4 indexs banks of 1k patterns
 pattern1k  *pbank[4]; //pointers to 1k pattern banks
 int operator ==(patterntable &t) {return *((int *)this)==*((int *)&t);}
 void setbank(int banknum,int pnum);
 void restore(); //restore pbanks from banks

 void setbank4k(int x)
 {  //set default vrom/vram banks
   for (int i=0; i<4; i++)
    setbank(i,i+x*4);
 }
 void clear();
 patterntable() {clear();}

 //array operator
 inline bitmap8x8 &operator [](byte idx)
  {return pbank[idx/64]->p[idx&63];}
 void print();
};



struct patternmap
{
 char t[256]; //specifies if a certain pattern idx needs updating

 unsigned time; //time of last use
 patterntable from,to;

 //analyzes translation from pattern table a to pattern table b
 //and creates 256 array signifying which patterns have changed
 void create(patterntable &a,patterntable &b);
};


//class that creates a 32x30 surface
//and receives name table writes
//and updates surface accordingly
class natablecache
{
 class surface *s; //32x30 tile surface

 NES_natable nt;  //REAL nt
 patterntable pt; //pattern table used to draw surface

 char lineupdated[36];
 char updated[36][32]; //updated flags, when set, cooresponding tile needs to be redrawn

 friend class nesvideo;
 friend class namedlg;
 friend class patterntable;
 friend class nesstate;

 public:
 natablecache();
 ~natablecache();

 void createsurface();
 void freesurface();

 void refresh(int y1,int y2);
 void totalupdate();
 void individualupdate(byte *pu); //updated based on individual pattern updates

 void apply(struct patternmap &pm);
 void setpatterntable(patterntable &newp);

 void write(unsigned a,char d) //update tile at address a
  {
   if (((char *)&nt)[a]==d) return; //ignore it

   ((char *)&nt)[a]=d; //write byte

   //set update flags
   if (a<32*30)
    {
     lineupdated[a/32]=1;
     updated[a/32][a&31]=1; //set update flag for this tile
    }
   else //it's an attribute
    {
     a-=32*30; //get address into att table
     int x=(a&7)*4;
     int y=(a/8)*4;
     *((unsigned *)&lineupdated[y])=0x1010101;

     *((unsigned *)&updated[y][x])=0x1010101;
     *((unsigned *)&updated[y+1][x])=0x1010101;
     *((unsigned *)&updated[y+2][x])=0x1010101;
     *((unsigned *)&updated[y+3][x])=0x1010101;
    }
  // doanyupdate=1;
  }
  char read(unsigned a) {return ((char *)&nt)[a];}

  void draw(int x,int y, struct lrect *clip);
};

#include "slist.h"

//class for output to GUI
class nesvideo:public GUIcontents
{
 public:
 int xw,yw; //xw and yw of video
 int maximized;

 void reset();   //restores hardware
 void clear();   //sets hardware to default settings

 byte paletteupdated; //has any of the pal been updated?
 byte palupdateidx[32]; //which ones were updated?
 void refreshpalette();
 void resetpalette();

 int numpattern1k; //number of 1k pattern tables
 pattern1k  *pt1k;     //array of optimized patterns (in 1k chunks)
 byte pt1kupdated[8];   //(vram only) if patterns in 0-8k updated
 void createpatterntables();
 void freepatterntables();

 int numrealntc; //number of real nametable caches
 natablecache *realntc[4]; //only 2 REAL caches, but could be 4
 natablecache *ntc[4];    //4 mirrored
 void createnatablecaches();
 void freenatablecaches();
 void setmirroring(int type); //set mirroring
 void resetmirroring();  //set ntc[] based on mirroring
 byte mirroring; //current mirroring

 seventlist sl; //event list
 scontextlist sc; //screen context


 void drawbg(char *dest,lrect *r,byte sprites);
 void drawsprites(char *dest,lrect *clip);
 void drawsprites_8x8(char *dest);
 void drawsprites_8x16(char *dest);

 char forcedesktopfill;
 char dotakesnapshot;

 void check(char *dest,int x,int y);

 nesvideo(char *romfile);
 virtual ~nesvideo();

 //------------------------------------
 //gui related bullshit
 virtual int acceptfocus() {return 1;}
 virtual void losefocus() { return; } //workspace never loses focus
 virtual int keyhit(char kbscan,char key);
 virtual char *getname() {return "nesvideo";};
 virtual void restore();
 virtual void maximize();
 void resize(int xw,int yw);
 virtual void draw(char *dest);
 virtual GUIrect *nesvideo::click(mouse &m);
};

void updatepalette(int palidx,byte paldata); //update palette number #

//current nes video
extern nesvideo *nv;

#endif
