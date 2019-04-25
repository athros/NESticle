#ifndef _MMC_
#define _MMC_

//memory mapper types
#define MMC_NONE   0
#define MMC_SEQ    1
#define MMC_KONAMI 2
#define MMC_VROMSWITCH 3
#define MMC_5202   4
#define MMC_CASTLE3 5
#define MMC_F4xxx 6
#define MMC_F5xxx 7


//32K rombank representing a possible rom combination
struct ROMBANK
{
 char s[0x8000];

 void setlowpage(ROMPAGE *p); //set low page
 void sethighpage(ROMPAGE *p); //set high page
};

//class for memory mapper chip
class memorymapper
{
 public:
 memorymapper() {}
 virtual ~memorymapper() {};
 virtual void reset() {}; //reset
 virtual int write(word a,byte d) {return 0;}; //ROM write (return 1 if bank switch)

 virtual int write5000(word a,byte d) {ram[a]=d; return 0;} //expansion write
 virtual byte read5000(word a) {return ram[a];} //expansion read


 virtual int keyhit(char kbscan,char key) {return 0;}
 virtual byte ppuread(word a)=0;

 //save and restore mmc state
 virtual void save(char *t) {};
 virtual void restore(char *t) {};
};

memorymapper *newmemorymapper(int type);

extern memorymapper *mmc;

char *getmmctypestr(int type);

#endif
