void set_keyboard_func(int (*kfunc)());

//last kbscan read
extern volatile char kbscan;

#define KB_SHIFT 1
#define KB_CTRL  2
#define KB_ALT   4
//status of the control/alt/shift keys
extern volatile char kbstat;

//list of all keys
extern volatile char keydown[128];

void pushkey(char kbscan);
char waitkey();
char getkey();
int keyhit();

char scan2ascii(char s);

char *getkeyname(char key);

//shitty scan code defines
#define KB_UP    0x48
#define KB_DOWN  0x50
#define KB_LEFT  0x4B
#define KB_RIGHT 0x4D
#define KB_HOME  0x47
#define KB_END   0x4F
#define KB_PGUP  0x49
#define KB_PGDN  0x51

#define KB_INS   0x52
#define KB_DEL   0x53
#define KB_ESC   0x01
#define KB_TAB   0x0F

#define KB_BKSP  0x0E
#define KB_ENTER 0x1C
#define KB_SPACE 0x39

#define KB_F1    0x3B
#define KB_F2    0x3C
#define KB_F3    0x3D
#define KB_F4    0x3E
#define KB_F5    0x3F
#define KB_F6    0x40
#define KB_F7    0x41
#define KB_F8    0x42
#define KB_F9    0x43
#define KB_F10   0x44
#define KB_F11   0x85
#define KB_F12   0x86


#ifdef DOS
void init_keyboard();
void terminate_keyboard();
extern volatile char kbint;
//whether or not to store keyscans in local queue
extern int keyqueue;
//whether or not to call bios instead
extern int keybios;
//whether or not to allow repeating
extern int keyrepeat;
#endif

#ifdef WIN95
void wm_keydown(char k);
void wm_keyup(char k);
#endif





