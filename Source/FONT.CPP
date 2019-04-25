#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "file.h"
#include "r2img.h"
#include "font.h"

//font constructor
FONT::FONT()
{
 size=sizeof(FONT);
 for (int a=0; a<128; a++) i[a]=0;
}

inline IMG *FONT::getimgptr(char c)
{
 if (!i[c]) return 0;
 return (IMG *)( ((char *)this)+i[c]);
}

int FONT::draw(char c,char *d,int x,int y)
{ 
 IMG *img=getimgptr(c);
 if (!img) return 6; //space
 img->draw(d,x,y,0);
 return img->xw;
}

void FONT::draw(char *str,char *d,int x,int y)
{
 for (; *str; str++)
  {
   IMG *img=getimgptr(*str);
   if (img)
    {
     img->draw(d,x,y,0);
     x+=img->xw;
    } else x+=6; //space it
  }
}



int FONT::getwidth(char c)
{
 IMG *img=getimgptr(c);
 if (img) return img->xw;
     else return 6; //space it
}

int FONT::getwidth(char *str)
{
 int len=0;
 for (; *str; str++)
  {
   IMG *img=getimgptr(*str);
   if (img) len+=img->xw;
       else len+=6; //space it
  }
 return len;
}


extern char *screen;
//printf w/font to char *screen
void __cdecl FONT::printf(int x,int y,char *format,...)
{
 char s[256];

 va_list argptr;
 va_start(argptr,format);
 vsprintf(s,format,argptr);
 va_end(argptr);

 draw(s,screen,x,y);
}    

void FONT::write(char *filename)
{
 FILEIO f;
 f.create(filename);
 f.write(this,size);
 f.close();
 return;
}    


FONT *FONT::addsymbol(FONT *f,IMG *x,char c)
{
 f->i[c]=f->size;
 f->size+=x->size;

  //realloc memory for new symbol
 f=(FONT *)realloc(f,f->size);
 if (!f) return 0;
  //copy symbol
 memcpy(f->getimgptr(c),x,x->size);
  //return new font
 return f;
}    
    

FONT *loadfont(char *fontfile)
{
 FILEIO f(fontfile);
 return (FONT *)f.readalloc(f.size());
}    

FONT *FONT::duplicate()
{
 FONT *f=(FONT *)malloc(size);
 memcpy(f,this,size);
 return f;
}

void FONT::convertcolor(char a,char b)
{
 for (int j=0; j<128; j++)
  {
   IMG *img=getimgptr(j);
   if (img) img->convertcolor(a,b);
  }
}    
