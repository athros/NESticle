#ifndef INPUT_H
#define INPUT_H

//input device types
#define ID_NONE   0
#define ID_GRAVIS 1
#define ID_KEY1   2
#define ID_KEY2   3 
#define ID_JOY1   4
#define ID_JOY2   5
#define ID_GRIP1  6
#define ID_GRIP2  7


//input directions
#define ID_RIGHT 0x1
#define ID_LEFT  0x2
#define ID_UP    0x4
#define ID_DOWN  0x8

#define ID_BUT0  0x10
#define ID_BUT1  0x20
#define ID_BUT2  0x40
#define ID_BUT3  0x80


//defines keys for an input device
struct keymap
{
char ul,u,ur;
char l,r;
char dl,d,dr;
char button[6]; //6 button keys

int read(char kbscan,unsigned &kstat);
};

struct joythreshold
{
int l,r,u,d; //thresholds
};




//defines an input device
class input {
public:
int type;     //ID_XXXX

int testing; //are we testing it?

//current status of the device
//   DULR  directions and button triggers.
unsigned stat;
unsigned oldbut;  //old buttons
unsigned but;     //holding status of all the buttons/keys

//the object bound to this device
class object *o;


//member functions-------
input(int t) {type=t; stat=but=0; o=0; testing=0;}
virtual ~input() {};

void reset()
{
 if (!this) return;
 stat&=0xF; //clear button triggers
}

unsigned getstat();


virtual void read()=0; //read input device into stat/but
virtual int  key(char kbscan) {return 0;}
virtual void timer() {}
virtual void main() {}



virtual struct keymap *getkeymap() {return 0;}
virtual struct joythreshold *getjoythreshold() {return 0;}
virtual void savesettings() { } //save settings back to ids

//bind/unbind input device to an object
void bind(object *t);
void unbind();

static int  refreshkeyboard(char kbscan);
static void refreshmain();
static void refreshtimer();
};
input *newinputdevice(int idtype);



//---------------------------
//     input device settings
//---------------------------

extern keymap defkeymap[2];

// structure containing input device settings, calibrations for each input device.
struct inputdevicesettings
{

//the keymaps for the keyboard inputdevices
keymap km[2];

//analog joystick center, min max thresholds
joythreshold aj[2];

int gripslot[2]; //the grip slots for grip1 and 2
};

#endif
