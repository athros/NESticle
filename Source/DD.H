//platform dependant interface

//screen buffers
extern char *video;   // Pointer to video memory
extern char *screen; // Pointer to virtual screen
extern "C" int SCREENX,SCREENY,PITCH,BANKED;

//sound
#ifdef WIN95
typedef struct IDirectSoundBuffer     *LPDIRECTSOUNDBUFFER;
LPDIRECTSOUNDBUFFER createdsoundbuffer(struct SOUND *s);
void playsound(struct IDirectSoundBuffer *dsb,int x);
void playsoundlooped(struct SOUND *s);
void freesound(struct IDirectSoundBuffer *dsb);
int getsoundsize(struct IDirectSoundBuffer *dsb);
#endif

#ifdef DOS
void playsound(struct SOUND *s);
#endif

//input
extern class input *inputdevice[2];
extern class mouse m;
extern struct FONT *font[];

void setpalette(struct PALETTE *pal);
void setpalettenum(int index,struct COLOR *c);
extern struct COLORMAP *shadowmap;

//timer
extern volatile int timerbusy;
extern volatile unsigned uu,su;
void settimerspeed(int x);

void quitgame();
void *loadresource(char *name);

void disable();
void enable();

//in case of error
extern char errstr[80];
void cleanexit(int x);


//functions that need to be supplied by the game
int  initgame();
void updatescreen();
void terminategame();
void gametimer();

void changeresolution(int newx,int newy);


class surface
{
 #ifdef WIN95
 struct IDirectDrawSurface *dds;
 int xw,yw;
 #endif
 #ifdef DOS
 struct SCR *s;
 #endif

 int oldscrx,oldscry,oldpitch;

 public:
 surface(int txw,int tyw);
 ~surface();
 char *lock();
 void unlock();
 int blt(char *dest,int x,int y); //returns 1 on success

 int blt(int sx,int sy,int sxw,int syw,char *dest,int x,int y); //returns 1 on success
};


//class for clipping
class CLIP
{
 int oldscrx,oldscry; //old screen width
// char *oldscreen;
 public:
 CLIP(char *&dest,int x1,int y1,int x2,int y2)
  {
   oldscrx=SCREENX; oldscry=SCREENY;

   //oldscreen=screen;
   //screen+=x1+y1*PITCH;
   if (x1<0) x1=0;
   if (x2>SCREENX) x2=SCREENX;
   if (y1<0) y1=0;
   if (y2>SCREENY) y2=SCREENY;
   dest+=x1+y1*PITCH;
   SCREENX=x2-x1; SCREENY=y2-y1;
  };
 ~CLIP()
  {
   SCREENX=oldscrx;
   SCREENY=oldscry;
//   screen=oldscreen;
  }
};

#ifdef __WATCOMC__
int random(int x);
#endif



