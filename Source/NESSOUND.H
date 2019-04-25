#ifndef _NESSOUND_
#define _NESSOUND_

#include "types.h"

#include "uutimer.h"

struct controlreg1
{
 byte playbackrate:4;
 byte envelopefixed:1;
 byte holdnote:1;
 byte dutycycle:2;
};

struct controlreg2
{
 byte freqrange:3;
 byte freqselect:1;     //0:low -> high 1:high to low
 byte freqchangespeed:3;
 byte freqvariable:1; //0=fixed freq 1=variable freq
};

struct frequencyreg
{
 unsigned short x;

 short getfreq() {return x&0x7FF;}
 byte gettime() {return x>>11;}
};


class neschannel
{
 friend class nessound;

 protected:

 union
 {
  byte   r[4]; //individual byte registers
  struct
  {
   controlreg1 cr1;
   controlreg2 cr2;
   frequencyreg fr;
  } sr;
 };

 byte enabled; //is the channel enabled?

 byte noteon; //is the note on?
 int dur; //duration of note left (in samples)
 int freq; //frequency (correct)

 int K; //freq counter increment
 int deltaK; //change in freq counter over time
 int count; //frequency counter (period is 0x10000)
 short vol; //current 16-bit volume

 public:
 void setenable(byte x);
 void reset();
 void startnote();
 void stopnote();
 void setdeltaK();

 neschannel() {reset();}
 void write(byte a,byte d);
 virtual void mix8(short *b,int nums) {}
 virtual void mix16(short *b,int nums) {}
 virtual void setvolume() {}
 virtual void print(int x,int y);
};

class squarewave:public neschannel
{
 public:
 virtual void mix16(short *b,int nums);
 virtual void setvolume();
};

class trianglewave:public neschannel
{
 public:
 virtual void mix16(short *b,int nums);
 virtual void setvolume();
};



//------------------

class nessound
{
 neschannel *ch[5]; //5 channels

 public:
 byte enablebits;
 void write(byte a,byte d);
 void setenable(byte d); //set sound enables
 void reset();
 void print();
 nessound();
 ~nessound();

 //mixing buffer
 static short mixingbuf[4096];

 //called periodically to do mixing
 void update(int nums);
 void mix(int nums); //mixes nums bytes and outputs it
 void mix16(short *buf, int nums); //16 bit mixing
};

extern nessound *ns;
extern byte soundenabled; //is sound enabled ?
void disablesound();
void enablesound();
void togglesound();

#endif
