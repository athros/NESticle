#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <direct.h>

//functions for image 
#include "r2img.h"
#include "file.h"

extern "C" int PITCH,SCREENX,SCREENY;

IMG *loadimage(char *file)
{
 if (!strchr(file,'.')) strcat(file,".r2");

 FILEIO g;
 if (g.open(file)) return 0;

 IMG *i=(IMG *)g.readalloc(g.size());
 g.close();
 if (i->type!=IMG_TYPE) return 0;
 return i;
}    


SCR *loadscreen(char *file)
{
 if (!strchr(file,'.')) strcat(file,".scr");

 FILEIO g;
 if (g.open(file)) return 0;

 SCR *i=(SCR *)g.readalloc(g.size());
 g.close();
 if (i->type!=SCR_TYPE) return 0;
 return i;
}    

SCR *SCR::newSCR(int x,int y)
{
 SCR *s=(SCR *)malloc(sizeof(SCR)+x*y+8);
 if (!s) return 0;

 s->type=SCR_TYPE;
 s->xw=x;
 s->yw=y;
 s->size=sizeof(SCR)+x*y;
 return s;
}    



//****************************************************
//****************************************************


extern "C" {
 void __cdecl drawscr(char *s,char *d,int xw,int yw,int pitch);
};

//draws uncompressed bitmap
void SCR::draw(char *d,int dx,int dy)
{
 char *s=data();
 int xstart=0,xlen=xw;
 int ystart=0,ylen=yw;
 if (dx<0) {xstart=-dx; xlen+=dx; dx=0;}
 if (dy<0) {ystart=-dy; ylen+=dy; dy=0;}
 if (dx+xlen>SCREENX) {xlen=SCREENX-dx;}
 if (dy+ylen>SCREENY) {ylen=SCREENY-dy;}

 drawscr(data()+ystart*xw+xstart,d+dx+dy*PITCH,
       xlen,ylen,xw);
}



/*
//draws uncompressed bitmap
void SCR::draw(char *d,int dx,int dy)
{
 char *s=data();
 int xstart=0,xlen=xw;
 int ystart=0,ylen=yw;
 if (dx<0) {xstart=-dx; xlen+=dx; dx=0;}
 if (dy<0) {ystart=-dy; ylen+=dy; dy=0;}
 if (dx+xlen>SCREENX) {xlen=SCREENX-dx;}
 if (dy+ylen>SCREENY) {ylen=SCREENY-dy;}

 d+=dx+dy*PITCH;
 s+=ystart*xw+xstart;

 for (; ylen>0; ylen--,d+=PITCH,s+=xw)
  memcpy(d,s,xlen);
} */


void BMAP::draw(char *d,int x,int y,int o)
{
 if (type==IMG_TYPE) ((IMG *)this)->draw(d,x,y,o);
 if (type==SCR_TYPE) ((SCR *)this)->draw(d,x,y);
}    

void BMAP::write(char *file)
{
 if (!strchr(file,'.')) //no extension
  {
   if (type==IMG_TYPE) strcat(file,".R2");
   if (type==SCR_TYPE) strcat(file,".SCR");
  }

 FILEIO f;
 f.create(file);
 f.write(this,size);
 f.close();
}    



//compress uncompressed image into rle form
IMG *SCR::compress(char tcolor)
{
 if (!this) return 0;
// printf("\n");
 int bufsize=sizeof(IMG)+yw*sizeof(int)+xw*yw+30000;
// printf("allocating memory %d\n",bufsize);
 IMG  *img=(IMG *)malloc(bufsize);
 if (!img) return 0;
// printf("allocated memory %X %d\n",img,bufsize);
 img->type=IMG_TYPE;
 img->xw=xw;
 img->yw=yw;
 img->size=0;

 char *s=data();
 char *d=img->data();
 int  *yd=img->ydisp();
 
 for (int y=0; y<yw; y++) //go through all scanlines
 {
  yd[y]=d-((char *)img); //find y displacement

 //cprintf("line %d: ydisp=%d ",y,yd[y]);
  for (int x=xw; x>0; ) //encode scanline
   {
    unsigned int runlength;
    //encode transparent run
    for (runlength=0; *s==tcolor && x>0 && runlength<255; x--,runlength++,s++);
    *d=(unsigned char)runlength; d++; //store transparent run length
    //cprintf("%dT ",runlength);
    if (x<=0) break; //we're done with line

    //encode opaque run
    d++; //skip byte that'll hold run length
    for (runlength=0; *s!=tcolor && x>0 && runlength<255; x--,runlength++,s++,d++) *d=*s;
    //cprintf("%dO ",runlength);    
    *(d-runlength-1)=(unsigned char)runlength; //store it back at beginning of run
   }
// printf("\n");
  
 }   

 free(this); //kill uncompressed form
 
 img->size=d-((char *)img); //find size
 img=(IMG *)realloc(img,img->size); //reallocate it
 return img;
}    


/*
//draws compressed rle bitmap
void IMG::draw(char *d,int dx,int dy,int o)
{
    o=o;
 int *yd=ydisp();
 unsigned char *s;
 d+=dx+dy*PITCH;

 for (int y=0; y<yw; y++)
  {
   s=((unsigned char *)this)+yd[y]; //get pointer to start of rle line
   
   for (int x=xw; x>0; ) //draw line
    {
     unsigned int runlength=*s; s++;
     d+=runlength; x-=runlength;
     if (x<=0) break;

     runlength=*s; s++;
     memcpy(d,s,runlength);
     s+=runlength; d+=runlength; x-=runlength;
    }

    d-=xw;
    d+=PITCH;
  }
}    

*/




//draws compressed rle bitmap as shadow map
void IMG::drawmap(COLORMAP &map,char *d,int dx,int dy,int o)
{
 int *yd=ydisp();
 unsigned char *s;
 d+=dx+dy*PITCH;

 for (int y=0; y<yw; y++)
  {
   s=((unsigned char *)this)+yd[y]; //get pointer to start of rle line
   
   for (int x=xw; x>0; ) //draw line
    {
     unsigned int runlength=*s; s++;
     d+=runlength; x-=runlength;
     if (x<=0) break;

     runlength=*s; s++;
     for (int i=0; i<runlength; i++,d++)
       *d=map.c[*d];
     s+=runlength; x-=runlength;
    }
    d+=PITCH-xw;
  }
}    






void IMG::convertcolor(char a,char b)
{
 unsigned char *s=(unsigned char *)data();
 unsigned int runlength;
 int *yd=ydisp();

 for (int y=0; y<yw; y++)
 {
  s=((unsigned char *)this)+yd[y]; //get pointer to start of rle line
   
  for (int x=xw; x>0; ) //line
   {

   //transparent
   runlength=*s; s++;
   x-=runlength;
   if (x<=0) break;
   //opaque
   runlength=*s; s++;
 
   for ( ; runlength>0 && x>0; s++,runlength--,x--)
    if (*s==a) *s=b;
  }
 }

}    




IMG *IMG::duplicate()
{
 IMG *t=(IMG *)malloc(size);
 memcpy(t,this,size);
 return t;
}

void IMG::colormap(COLORMAP &map)
{
 unsigned char *s=(unsigned char *)data();
 unsigned int runlength;
 int *yd=ydisp();

 for (int y=0; y<yw; y++)
 {
  s=((unsigned char *)this)+yd[y]; //get pointer to start of rle line
   
  for (int x=xw; x>0; ) //line
   {

   //transparent
   runlength=*s; s++;
   x-=runlength;
   if (x<=0) break;
   //opaque
   runlength=*s; s++;
 
   for ( ; runlength>0 && x>0; s++,runlength--,x--)
    *s=map.c[*s];
  }
 }
}


void COLORMAP::clear()
{
 for (int i=0; i<256; i++)
  c[i]=i;
}    


PALETTE *PALETTE::duplicate(int num)
{
  //get memory for other palette
 PALETTE *p=(PALETTE *)malloc(num*sizeof(COLOR));
 memcpy(p,this,num*sizeof(COLOR));
 return p;
}

inline void COLOR::fade(COLOR &d,int level)
{
 d.r=r*level/MAXFADELEVEL;
 d.g=g*level/MAXFADELEVEL;
 d.b=b*level/MAXFADELEVEL; 
}

    
void PALETTE::fade(PALETTE &p,int num, int level)
{
 for (int i=0; i<num; i++)
   c[i].fade(p.c[i],level);
}

inline int SQUARE(int x) {return x*x;}

int PALETTE::findclosestmatch(COLOR &a,int num)
{
int closest=0;
int distance=0xFFFFFFF;

for (int i=0; i<num; i++)
 {
  int dist=SQUARE(c[i].r-a.r)+SQUARE(c[i].g-a.g)+SQUARE(c[i].b-a.b);
  if (dist<distance) {closest=i; distance=dist;}
 }
 
return closest;
}

void COLORMAP::createshademap(PALETTE &p,int rl,int gl,int bl)
{
 for (int i=0; i<256; i++)
 {
  COLOR x;
  //calculate shaded color
  x.r=p.c[i].r*rl/MAXFADELEVEL;
  x.g=p.c[i].g*gl/MAXFADELEVEL;
  x.b=p.c[i].b*bl/MAXFADELEVEL;

  c[i]=p.findclosestmatch(x,256);
 }     
}

//*****************************************************
//*****************************************************
//********* decodes lbm files *************************
//*****************************************************
//*****************************************************

void expandword(unsigned short s,char *d,int plane)
{
for (int i=15; i>=0; i--)
  {
   *d|=(char) ( ((s>>i)&1)<<plane );
   d++;
  }
}    

void expandbyte(unsigned char s,char *d,int plane)
{
for (int i=7; i>=0; i--)
  {
   *d|=(char) ( ((s>>i)&1)<<plane );
   d++;
  }
}    


unsigned short reverseword(unsigned short x)
{
 return( (unsigned short)((x>>8)|(x<<8)));
}    


//no compression
unsigned DecodeIFF(char *s,char *d,int bx,int by,int compression)
{
 int i,j,k;
 
 int wpl; //words per scan line per plane
 wpl=(bx+15)>>4; //words per scanline
    
// cprintf("wpl%d",wpl);

 char *dw=d;
 for ( ;by>0; by--) //go through all lines
  {
   for (i=0; i<=bx; i++) dw[i]=0; //erase next widthlength of bytes
//     printf("n=%d x=%d plane=%d y=%d\n",k,k,j,by);
   
   for (j=0; j<8; j++) //go through all planes
    {
     char *td=dw;
     if (!compression)
     { //not compressed
     for (k=0; k<wpl; k++,s+=2,td+=16) //go through all words in each plane
      expandword(reverseword( *((unsigned short *)s) ),td,j);
     } else
     { //compressed
      k=wpl*2;
      while (k>0)
       {
        signed n=(signed)(*((signed char*)s)); s++;
        if (n>=0)
          {
            
            for (n++,k-=n;  n>0; n--,s++,td+=8)
              expandbyte((unsigned char)*s,td,j);
          } else
         if (n!=((signed)-128)) 
          {
            unsigned char c=*s; s++; 
            for (n=1-n,k-=n;  n>0; n--,td+=8)
              expandbyte(c,td,j);
          }
       }//while
      } //if (!compression)
     } //for j=0
   dw+=bx;
  }

return(dw-d); 
}    








unsigned int FindChunk(char *s, unsigned size,char c1, char c2, char c3, char c4)
{
unsigned int i=0;
do
{
if (s[i]==c1 && s[i+1]==c2 && s[i+2]==c3 && s[i+3]==c4)
   break;
i++;   
} while (i<size);

if (i==size) return 0;
  else return i;
}    

//reads a lbm/bbm file and decodes it uncompressed
SCR *ReadLBMFile(char *file)
{
//read lbm/bbm file
FILEIO f;
if (f.open(file)) return 0;
int filesize=f.size();
char *s=(char *)f.readalloc(filesize);
f.close();

//get header info from file
if (s[0]!='F' || s[1]!='O' || s[2]!='R' || s[3]!='M')
 { printf("error: not IFF file.\n"); free(s);  return(0); }

if (!FindChunk(s,filesize,'I','L','B','M'))
 {printf("error: not old DP2 format.\n"); free(s); return(0);} 

int i=FindChunk(s,filesize,'B','M','H','D');
if (!i) { printf("error: BMHD tag not found.\n"); free(s);  return(0); }
i+=8; unsigned bx=reverseword(*((unsigned short *)&s[i]));
i+=2; unsigned by=reverseword(*((unsigned short *)&s[i]));
i+=2; 
i+=6; unsigned compression=s[i];

//position at data start
i=FindChunk(s,filesize,'B','O','D','Y');
if (!i) { printf("error: BODY tag not found.\n"); free(s); return(0); }
i+=8; 



SCR *scr=SCR::newSCR(bx,by);
//scr->size=sizeof(SCR)+
DecodeIFF(&s[i],scr->data(),bx,by,compression);
free(s);
//scr=(SCR *)realloc(scr,scr->size);

return(scr);
}    



void drawbox(char *d,int color,int x1,int y1,int x2,int y2)
{
 //top
 drawhline(d,color,x1,y1,x2);
 //bottom
 drawhline(d,color,x1,y2-1,x2);
 //left
 drawvline(d,color,x1,y1,y2);
 //right
 drawvline(d,color,x2-1,y1,y2);
}
  













