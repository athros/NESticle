//classes for profiling
#ifndef _PROF_
#define _PROF_

#include "uutimer.h"

unsigned gettime(); //gets the current time in milliseconds

//maintains a count of events
class profcounter
{
 protected:
 unsigned cnt;     //current count
 unsigned lastcnt;

 float rate;  //rate of increase/sec
 float divisor;

 public:
 void clear() {cnt=0; lastcnt=0;}
 profcounter(float _divisor):divisor(_divisor) {clear();}
 profcounter():divisor(1.0) {clear();}

 inline void inc() {cnt++;} //increment counter
 inline void add(unsigned num) {cnt+=num;} //increment counter by num

 void update(float dt) //update rate over dt (dt is in secs)
  {rate=((float)(cnt-lastcnt))/dt/divisor; lastcnt=cnt;}
 float getrate() {return rate;}
 unsigned getcnt() {return cnt;}

 void draw(char *desc,int x,int y);
};


//times the duration of events
class proftimer:public profcounter
{
 unsigned entertime;

 public:

 proftimer():profcounter(10.0) {}

 //enters/leaves a section
 inline void enter() {entertime=gettime();}
 inline void leave() {add(gettime()-entertime);}

 void draw(char *desc,int x,int y);
 void drawmeter(char *desc,int x,int y,byte c);
};

//profcounters used in this app
struct anesprofile
{
 uutimer updatetimer;

 profcounter tiles; //tiles drawn per second
 profcounter cycles; //cycles per second
 profcounter frames;   //frames per second
 profcounter vframes;   //vframes per second

 proftimer cpu_timer;
 proftimer nesdraw_timer;
 proftimer gui_timer;
 proftimer input_timer;

 proftimer pageflip_timer;
 proftimer pageclear_timer;

 proftimer sound_timer;

 anesprofile();
 void update();
 void draw(int x,int y);
};
extern anesprofile pf;


#endif