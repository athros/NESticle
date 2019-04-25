#ifndef _SLIST_
#define _SLIST_

int getscanline();


//structure describing an event that occurs during a virtual frame
struct sevent
{
 unsigned line; //line that it occurred
 byte type; //type of event
 byte data; //actual data

 //event types
 #define SE_NONE    0
 #define SE_SCROLLX 1
 #define SE_SCROLLY 2
 #define SE_NATABLE 3

 //bg on/off and its pattern table
 #define SE_BG      4
 #define SE_BGPT    5
 //sprites on/off and their pattern table
 #define SE_SPRITE   6
 #define SE_SPRITEPT 7

 #define SE_FRAMESTART 8

 #define SE_IRQLINE 9

 #define SE_VROM8K  10
 #define SE_VROM4K0 11
 #define SE_VROM4K1 12

 #define SE_VROM1K  13
 #define SE_VROM1K1 14
 #define SE_VROM1K2 15
 #define SE_VROM1K3 16
 #define SE_VROM1K4 17
 #define SE_VROM1K5 18
 #define SE_VROM1K6 19
 #define SE_VROM1K7 20

 void print(int x,int y);
 int operator ==(sevent &t) {return type==t.type && data==t.data;}
};


//list of all events that have occured during a frame
struct seventlist
{
 int num; //number of events recorded so far
 sevent s[512]; //list of events

 void reset() {num=0;} //no events
 void add(sevent &t);

 void addevent(byte type,byte data)
    {sevent t; t.type=type; t.data=data; add(t);}
 void print(int x,int y);
 seventlist() {reset();}
};



//------------------------------------
//screen context

struct scontext
{
 byte sx,sy; //scroll position
 byte natable; //name table
 byte sprites,bg;     //0-off 1-on
 byte bgpt,spritept; //pattern table for bg/sprite
 byte extra;
 patterntable ptn[2]; //pattern table state

 unsigned line;    //top line of context
 unsigned height;  //height of context

 void clear()
  {
   memset(this,0,sizeof(*this));
   ptn[0].clear();
   ptn[1].clear();
  }
 void reset() {line=0;}

 void setline(int l) {line=l;} //set upper limit of context
 void setheight(int bottom) //set lower limit of context
  {height=bottom-line;}

 void operator +=(sevent &e); //add event to context
 int operator ==(scontext &t) //comparison
 {
  return
   *((int *)this)==*((int *)&t) &&
   *(((int *)this)+1)==*(((int *)&t)+1) 
   && ptn[0]==t.ptn[0] && ptn[1]==t.ptn[1];
 }

 void print(int x,int y); //print
};


//list of all contexts, formed by combining all events
struct scontextlist
{
 int scnum;
 scontext c; //default/current context

 scontext *biggestsc; //the context with the LARGEST height
                      //will be used for natablecache and sprite patterns

 scontext sc[64]; //screen contexts

 void reset() {scnum=0;}
 void clear() {scnum=0; biggestsc=0; c.clear();}
 void add(scontext &t);

 void create(seventlist &el); //create from event list

 scontextlist() {clear();}
 void print(int x,int y); //print to screen
};


#endif








