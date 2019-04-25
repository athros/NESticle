#include <stdlib.h>
#include <string.h>
#include "types.h"

#include "r2img.h"
#include "dd.h"

#include "file.h"
#include "message.h"

#include "nesvideo.h"

#include "nes.h"
#include "rom.h"

#include "ppu.h"


struct pcx_header {
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  short  xmin,ymin;
  short  xmax,ymax;
  short  hres;
  short  vres;
  char palette16[48];
  char reserved;
  char color_planes;
  short  bytes_per_line;
  short  palette_type;
  char filler[58];

 pcx_header(int xw,int yw)
 {
  memset(this,0,sizeof(pcx_header));
  manufacturer=10;
  version=5;
  encoding=1;
  bits_per_pixel=8;
  xmax=xw-1; ymax=yw-1;
  color_planes=1;
  bytes_per_line=xw;
  palette_type=1;
 };
};

extern COLOR nespal[256];

//takes a snapshot from dest bounded by r
void takesnapshot(char *dest,lrect &r)
{
 char *s=(char *)malloc(r.width()*r.height());
 if (!s) return;

 //copy from screen
 char *t=s;
 for (int y=r.y1; y<r.y2; y++)
  for (int x=r.x1; x<r.x2; x++,t++)
    *t=(dest[y*PITCH+x]&0x1F)+16;


 char name[80];
 strcpy(name,romfilename);
 strcat(name,".pcx");

 FILEIO f;
 f.create(name);

 //write header
 pcx_header hdr(r.width(),r.height());
 f.write(&hdr,sizeof(hdr));

 //write data
 f.write(s,r.width()*r.height());

 //write palette
 f.writechar(12);

 COLOR p[256];
 memset(p,0,256*sizeof(COLOR));
 p[255].r=p[255].g=p[255].b=0xFF;

 for (int i=0; i<32; i++)
  p[i+16]=nespal[ppu->bgpal.c[i]];
 f.write(p,256*sizeof(COLOR));
 f.close();

 free(s);

 msg.printf(1,"Snapshot saved to %s %dx%d",name,r.width(),r.height());
}











