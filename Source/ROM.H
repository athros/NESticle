#ifndef _ROM_
#define _ROM_



struct NESROMHEADER
{
 char str[4]; //"NES" string
 char num16k; //number of 16k rom banks
 char num8k; //number of 8k rom banks

 char mirroring:1;//type of mirroring (1-vertical
 char battery:1;  //battery backed rom?
 char trainer:1;  //trainer?
 char fourscreen:1;  //4 screen vram
 char banktype:4; //type of bank selector

 char reserved[9];
 void printinfo();
 int validate();
};

//structure for battery backed memory
struct BATTERYMEM
{
 byte data[0x2000]; //save game data

 public:
 BATTERYMEM();
 void read(char *romname); //reads from file
 void write(char *romname); //reads from file

 void copytoRAM();
 void copyfromRAM();

};

void loadrom(char *romfile);
void freerom();

extern char romname[128]; //name of romfile
extern char romfilename[];

typedef byte ROMPAGE[0x4000];  //one page of ROM
typedef byte VROMPAGE[0x2000]; //one page of VROM

extern NESROMHEADER romhdr;
extern byte *trainer,*expansion;
extern BATTERYMEM *batterymem;
extern int numrom,numvrom; //number of rom/vrom pages
extern ROMPAGE  *ROM;  //array of 16k rom pages
extern VROMPAGE *VROM; //array of 8k vrom pages

#endif