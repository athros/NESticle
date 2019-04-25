#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "message.h"

#include <ctype.h>

#include "r2img.h"
#include "font.h"
#include "dd.h"

inline void MESSAGE::draw(int x,int y)
{
 font[color]->draw(s,screen,x,y);
}

inline void MESSAGE::set(char *m,int c)
{
 if (s) free(s);
 s=strdup(m);
 color=c;
}


void msgbuffer::add(char *m,int color)
{
 msg[num%MAXMSG].set(m,color);
 num++;
 updated++;
}

void msgbuffer::printf(int color,char *format, ...)
{
 char s[200];
 va_list argptr;
 va_start(argptr,format);
 vsprintf(s,format,argptr);
 va_end(argptr);
 add(s,color);
}


void msgbuffer::error(char *format, ...)
{
 char s[200];
 va_list argptr;
 va_start(argptr,format);
 vsprintf(s,format,argptr);
 va_end(argptr);
 add(s,5); //error font
}


void msgbuffer::draw(int x,int y,int first,int total)
{
 if (first<0) first=0;
 for (int i=first; i<num && total>0; i++,total--,y+=10)
     msg[i%MAXMSG].draw(x,y);
}



void msgbufferwrap::add(char *m,int color)
{
 //message is too wide....
 char s[128];

 int i=0;
 for (int w=0; w<width && m[i]; i++)
   w+=font[color]->getwidth(s[i]=m[i]);

 if (m[i])
  for (i--; i>0 && !isspace(s[i]); i--); //back up to last space

 if (i<=0) return;
 s[i]=0; //terminate it

 msgbuffer::add(s,color); //add this line...
 if (m[i]) add(&m[i],color);  //add next line....
}







