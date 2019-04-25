
#ifndef CONFIG_H
#define CONFIG_H

#include "input.h"

#define CONFIGVERSION 0x100

#define CFG_NOSOUND 0
#define CFG_SB      1
#define CFG_SB16    2
#define CFG_SBAWE   3

#define CFG_FILLEDDLG 1
#define CFG_SHOWFPS 2
#define CFG_SHOWGUITREE 3
#define CFG_LOADACTIVATE 4
#define CFG_NOFILLEDDESKTOP 5
#define CFG_SHOWMESSAGE 6
#define CFG_SHOWREGIONS 7


#define MAXOPTIONS 32

//structure defining configuration file
struct config
{
 config();
 int load(char *file);
 void save(char *file);

 int version; //version of the config file
 int crc;

 //sound blaster settings
 int soundcard;
 int sbport;
 int sbirq;
 int sbdma;
 int sbdma16;

 //input settings
 int pinput[4]; //player input devices
 inputdevicesettings ids;

 char option[MAXOPTIONS];

 char localname[16];

 //sets/gets options
 char set(int num,char val) {return option[num]=val;};
 char get(int num) {return option[num];}
 char toggle(int num) {return option[num]=!option[num];}
};

extern config *cfg; //standard config file
extern char configfile[];

#endif
