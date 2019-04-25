#ifndef _FONT_
#define _FONT_

struct FONT
{
 int size;
 unsigned short i[128]; //indices relative to beginning of font for each symbol image

 struct IMG *getimgptr(char c);
  
 int draw(char c,char *d,int x,int y);
 void draw(char *str,char *d,int x,int y);
 void __cdecl printf(int x,int y,char *format,...);

 int getwidth(char *s);
 int getwidth(char c);

 void drawcentered(char *str,char *d,int x,int y)
      {draw(str,d,x-getwidth(str)/2,y);}

 //font creation functions
 void write(char *filename); //write font to a file
 static FONT *read(char *filename); //read font from a file

 FONT(); //font constructor
 static FONT *addsymbol(FONT *f,struct IMG *x,char c);
 FONT *duplicate();
 void convertcolor(char a,char b);
};    

FONT *loadfont(char *fontfile);

#endif






