#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "types.h"
#ifdef DOS
extern "C" {
   void __cdecl ReadAnalogJoyPos(int *,int);
   int  __cdecl ReadAnalogJoyButtons();
 };
#endif

#ifdef WIN95
#include <windows.h>
#include <dinput.h>
#endif

#include "input.h"
#include "dd.h"

//#include "object.h"

//input device settings.
extern inputdevicesettings *ids;

//default keymaps
keymap defkeymap[2]=
{
 {
  0x47,0x48,0x49,
  0x4B,0x4D,
  0x4F,0x50,0x51,

  0x18,0x19,0x1a,
  0x26,0x27,0x28
 },
 {
  0x13,0x14,0x15,
  0x21,0x23,
  0x2F,0x30,0x31,

  0x10,0x11,0x12,
  0x1E,0x1F,0x20
 },
};



//refresh during keypress
int input::refreshkeyboard(char kbscan)
{
 if (inputdevice[0]) if (inputdevice[0]->key(kbscan)) return 1;
 if (inputdevice[1]) if (inputdevice[1]->key(kbscan)) return 1;
 return 0;
}

//refresh during imer
void input::refreshtimer()
{
 if (inputdevice[0]) inputdevice[0]->timer();
 if (inputdevice[1]) inputdevice[1]->timer();
}

//refresh during main loop
void input::refreshmain()
{
 if (inputdevice[0]) inputdevice[0]->main();
 if (inputdevice[1]) inputdevice[1]->main();
}




//binds the input device to an object...
void input::bind(object *t)
{
 if (!this) return;
  //unbind the object from whatever it's attached to
 unbind();

//disable();
 //bind to object
// o=t; if (o) o->in=this;

// enable();
}

//unbinds input device from object it's attached to
void input::unbind()
{
 if (!this) return;
// disable();

// if (o) o->in=NULL;
 o=NULL;
// enable();
}

unsigned input::getstat()
{
 if (!this) return 0;
// if (o && o->d && (stat&3)) return stat^3;
 return stat;
}

//--------------------------
// specific input devices
//-------------------------

class inputnone:public input
{
 public:
 inputnone():input(ID_NONE) {};
 virtual void read() {stat=0; but=0;}
};


#include "uutimer.h"
class joystick:public input
{
 protected:
 int installed;
 joythreshold *joythresh; //pointer to original

 joythreshold t; //copy of threshhold for the joystick
 lpoint pos;     //analog position of each axis
 int buttons;    //holding state of buttons
// int rfrtime;
 uutimer refreshtimer;

 public:
 virtual void savesettings() {*joythresh=t;}

 joystick(int type,joythreshold *jt):input(type),
  refreshtimer(3) //refresh every 3 ticks
  {
   installed=1;
//   rfrtime=0;
   joythresh=jt;
   #ifdef DOS
   t=*jt;
   int joypos[4];
   ReadAnalogJoyPos(joypos,1024);
   if (type!=ID_JOY2) {pos.x=joypos[0]; pos.y=joypos[1];}
                 else {pos.x=joypos[2]; pos.y=joypos[3];}
   t.l=pos.x*3/6;
   t.r=pos.x*9/6;
   t.u=pos.y*3/6;
   t.d=pos.y*9/6;
   #endif
   #ifdef WIN95
   t.l=32768-5000; t.r=32768+5000;
   t.u=32768-5000; t.d=32768+5000;
   #endif
  }

 virtual void read()
 {
  stat&=~0xF; //clear stat dirs
  if (pos.x>t.r) stat|=ID_RIGHT;  else
  if (pos.x<t.l) stat|=ID_LEFT;
  if (pos.y>t.d) stat|=ID_DOWN; else
  if (pos.y<t.u) stat|=ID_UP;
  oldbut=but;
  but=buttons<<4;
  stat|=((oldbut^but)&but);
 }
 virtual joythreshold *getjoythreshold() {return &t;};
};


//analog joystick 1
class analogjoy1:public joystick
{
 public:
 analogjoy1():joystick(ID_JOY1,&ids->aj[0]) {}

 virtual void main()
  {
   if (!installed) return;
   if (!refreshtimer.check()) return;
   refreshtimer.reset();
   #ifdef WIN95
   JOYINFO joypos;
   if (joyGetPos(0,&joypos)!=JOYERR_NOERROR) {pos.x=pos.y=buttons=0; installed=0; return;}
   pos.x=joypos.wXpos; pos.y=joypos.wYpos; buttons=joypos.wButtons;
   #endif
   #ifdef DOS
   int joypos[4];
   ReadAnalogJoyPos(joypos,1024);
   pos.x=joypos[0]; pos.y=joypos[1]; buttons=ReadAnalogJoyButtons()&3;
   #endif
  };
};


//analog joystick 2
class analogjoy2:public joystick
{
 public:
 analogjoy2():joystick(ID_JOY2,&ids->aj[1]) {}

 virtual void main()
  {
   if (!installed) return;
   if (!refreshtimer.check()) return;
   refreshtimer.reset();

   #ifdef WIN95
   JOYINFO joypos;
   if (joyGetPos(1,&joypos)!=JOYERR_NOERROR) {pos.x=pos.y=buttons=0; installed=0; return;}
   pos.x=joypos.wXpos; pos.y=joypos.wYpos; buttons=joypos.wButtons;
   #endif
   #ifdef DOS
   int joypos[4];
   ReadAnalogJoyPos(joypos,1024);
   pos.x=joypos[2]; pos.y=joypos[3]; buttons=(ReadAnalogJoyButtons()>>2)&3;
   #endif
  };
};


extern int blah;
//gravis gamepad
class gravis:public joystick
{
 static unsigned char gravtable[16];

 public:
 gravis():joystick(ID_GRAVIS,&ids->aj[0]) {}

 virtual void main()
  {
   if (!installed) return;
   if (!refreshtimer.check()) return;
   refreshtimer.reset();

   #ifdef WIN95
   JOYINFO joypos;
   if (joyGetPos(0,&joypos)!=JOYERR_NOERROR) {pos.x=pos.y=buttons=0; installed=0; return;}
   pos.x=joypos.wXpos; pos.y=joypos.wYpos;
   buttons=gravtable[(joypos.wButtons&15)];
   #endif
   #ifdef DOS
   int joypos[4];
   ReadAnalogJoyPos(joypos,1024);
   pos.x=joypos[0]; pos.y=joypos[1];
   buttons=gravtable[(ReadAnalogJoyButtons()&15)];
   #endif
  };
};
unsigned char gravis::gravtable[]={0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15,};


int keymap::read(char kbscan,unsigned &kstat)
{
if (!(kbscan&0x80)) //press
 {
  for (int i=0; i<sizeof(keymap); i++)
    if (kbscan==((char *)this)[i]) //compare scan codes
      {kstat|=1<<i; return 1;}    //set bit
 } else //release
 {
  kbscan&=0x7F;
  for (int i=0; i<sizeof(keymap); i++)
    if (kbscan==((char *)this)[i]) //compare scan codes
      {kstat&=~(1<<i); return 1;} //release bit
 }
 return 0;
}

class keyboard:public input
{
 protected:
 keymap *kmptr; //pointer to original keymap
 keymap km;         //keymap for this keybaord

 unsigned keystat;  //status of keys pressed (each bit represents a key in keymap)

 public:
 virtual void savesettings() {*kmptr=km;}

 keyboard(int type,keymap *tkm):input(type),km(*tkm),kmptr(tkm),keystat(0) {}

 virtual int key(char kbscan)
 {
//  if (!testing && (!o || !o->active)) return 0;
  return km.read(kbscan,keystat);
 }
 virtual void read()
 {
  //recalculate stats
  stat=0;
  if (keystat& ( (1<< 0) + (1<< 1) + (1<< 2)  ) ) stat|=ID_UP;  else //Up
  if (keystat& ( (1<< 5) + (1<< 6) + (1<< 7)  ) ) stat|=ID_DOWN;   //Down
  if (keystat& ( (1<< 2) + (1<< 4) + (1<< 7)  ) ) stat|=ID_RIGHT; else  //right
  if (keystat& ( (1<< 0) + (1<< 3) + (1<< 5)  ) ) stat|=ID_LEFT;    //Left

  //recalculate buttons
  oldbut=but;
  but=(keystat&0xFF00)>>4;
  stat|=((oldbut^but)&but);
 }

 virtual keymap *getkeymap() {return &km;};
};



class keyboard1:public keyboard
{
 public:
 keyboard1():keyboard(ID_KEY1,&ids->km[0]) {}
};

class keyboard2:public keyboard
{
 public:
 keyboard2():keyboard(ID_KEY2,&ids->km[1]) {}
};




//input initialization
input *newinputdevice(int idtype)
{ //if no inputdevicesettings, can't initialize
 if (!ids) return 0;

 switch (idtype)
  {
   case ID_NONE: return new inputnone;
   case ID_JOY1: return new analogjoy1;
   case ID_JOY2: return new analogjoy2;
   case ID_GRAVIS: return new gravis;
   case ID_KEY1: return new keyboard1;
   case ID_KEY2: return new keyboard2;
  }
 return 0;
}
