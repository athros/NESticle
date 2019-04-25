extern "C" {
 void __cdecl drawimager2(struct IMG *s,char *d,int x,int y,int o);
 void __cdecl loadpalette8(struct VBE_palette *c,int first,int num);
 void __cdecl loadpalette6(struct VBE_palette *c,int first,int num);
 #ifdef DOS
 void __cdecl drawrect(char *d,int color,int x,int y,int xw,int yw);
 #endif
 void __cdecl drawrectmap(char *d,struct COLORMAP *c,int x,int y,int xw,int yw);
 void __cdecl drawhline(char *d,int color,int x,int y,int x2);
 void __cdecl drawvline(char *d,int color,int x,int y,int y2);
};

#ifdef __WATCOMC__
#pragma aux drawimager2 modify[eax ecx edx];
#pragma aux drawhline  modify[eax ecx edx];
#pragma aux drawvline  modify[eax ecx edx];
#pragma aux drawrect   modify[eax ecx edx];
#endif


#ifdef WIN95
 //hardware accellerated
 void drawrect(char *d,int color,int x,int y,int xw,int yw);
#endif
void drawbox(char *d,int color,int x,int y,int xw,int yw);

#ifdef DOS
void mode256();
void modetext();
#endif

void line(char *dest, int x1, int y1, int x2, int y2, int Color);

#define MAXFADELEVEL 32
//rbg
struct COLOR
{
 unsigned char r,g,b;
 void fade(COLOR &d,int level);
};

//palette
struct PALETTE
{
 COLOR c[]; //colors in palette

// void set(int first,int num) {loadpalette((COLOR *)&c,first,num);}
 PALETTE *duplicate(int num);
 void fade(PALETTE &p,int num,int level); //level 0-31
 int findclosestmatch(COLOR &a,int num);
};

//color index mapping
struct COLORMAP
{
 char c[256];
 COLORMAP() {};
 void clear();
 void createshademap(PALETTE &p,int rl,int gl,int bl);
};


//bitmap header
struct BMAP {
 int type; //type byte
 int size; //size of image structure in bytes
 int xw,yw; //dimensions

 void write(char *file); //writes bitmap to file
 void draw(char *d,int x,int y,int o); //draw
};

//RLE encoded
#define IMG_TYPE 1

#define IMG_FLIPY 1
#define IMG_FLIPX 2

struct IMG:BMAP
{
 //int ydisp[]; //offset from beginning of struct for each scanline
 //char data[]; //data for rle bitmap

 int *ydisp() {return (int *)(((char *)this)+sizeof(IMG));} //returns pointer to ydisp array
 char *data() {return (char *)(ydisp()+yw);} //returns pointer to rle data
 void drawmap(COLORMAP &map,char *d,int x,int y,int o=0);
 void draw(char *d,int x,int y,int o=0)
  {
   drawimager2(this,d,x,y,o);
  };
 void convertcolor(char a,char b); //converts all color a to color b
 void colormap(COLORMAP &map); //convert colors based on colormap

 IMG *duplicate();
};

//uncompressed
#define SCR_TYPE 2
struct SCR:BMAP
{
 //char data[]; //data for uncompressed bitmap

 char *data() {return ((char *)this)+sizeof(SCR);} //returns a pointer to the actual image data

 IMG *compress(char tcolor); //compressed this to a RLE format, with transparent color
 void draw(char *d,int x,int y);

 static SCR *newSCR(int x,int y);
};

//loads image from a .r2 file
IMG *loadimage(char *file);

//loads screen from a .scr file
SCR *loadscreen(char *file);


//reads uncompressed bitmap from a .BBM or .LBM file
SCR *ReadLBMFile(char *file);


