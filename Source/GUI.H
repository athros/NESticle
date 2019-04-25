#ifndef _GUI_
#define _GUI_



#include "uutimer.h"

//messages that child windows send to parent
//button pushed
#define GUIMSG_PUSHED 1
//button is being held down
#define GUIMSG_HELDDOWN 2
//check box checked
#define GUIMSG_CHECKED 3
//check box unchecked
#define GUIMSG_UNCHECKED 4

//list box selection changed
#define GUIMSG_LISTBOXSELCHANGED 5
#define GUIMSG_LISTBOXDBLCLICKED 6

#define GUIMSG_EDITCHANGED 7

//messages send to contents by dialog box
#define GUIMSG_B1 20
#define GUIMSG_B2 21

//OK button pushed
#define GUIMSG_OK 20
//Cancel/close pushed
#define GUIMSG_CANCEL 22
#define GUIMSG_CLOSE 22


#include "guirect.h"
#include "guiroot.h"

//specific GUI types
class GUIbar:public GUIrect
{
 char color;
 public:
 GUIbar(GUIrect *p,char c,int x,int y,int xw,int yw):
  GUIrect(p,x,y,x+xw,y+yw),color(c) {};
 virtual void draw(char *d);
};


//----------------------------------------------
class GUIstatictext:public GUIrect
{
 protected:
 int fontnum;
 char text[80];
 public:
 GUIstatictext(GUIrect *p,int f,char *str,int x,int y);

 virtual void draw(char *dest); //draw this GUI element

 void settext(char *s);

 virtual char *getname() {return "guistatictext";}
};

class GUIstaticcenteredtext:public GUIstatictext
{
 public:
 GUIstaticcenteredtext(GUIrect *p,int f,char *str,int x,int y,int xw):
   GUIstatictext(p,f,str,x,y) {resize(xw,10);};

 virtual void draw(char *dest); //draw this GUI element
};


class GUIstaticimage:public GUIrect
{
 struct IMG *img;
 public:
 GUIstaticimage(GUIrect *p,struct IMG *i,int x,int y);

 virtual void draw(char *dest); //draw this GUI element
 virtual char *getname() {return "guistaticimage";}
};
//-----------------------------------------

//button
class GUIbutton:public GUIrect
{
 protected:
 uutimer btimer;
 public:
 char depressed;

 GUIbutton(GUIrect *p,int x,int y,int xw,int yw);

 virtual GUIrect * click(class mouse &m); //mouse was clicked over this region
 virtual int release(class mouse &m); //mouse was released over this region
 virtual void draw(char *dest); //draw this GUI element

 virtual char *getname() {return "guibutton";}
};

class GUItextbutton:public GUIbutton
{
 char text[80];
 int bwidth;
 public:
 void settext(char *s);
 GUItextbutton(GUIrect *p,char *s,int x, int y); //x,y center of string

 virtual void draw(char *dest); //draw this GUI element
 virtual char *getname() {return "guitextbutton";}
};

class GUIimagebutton:public GUIbutton
{
 struct IMG *img;
 public:
 GUIimagebutton(GUIrect *p,struct IMG *i,int x, int y); //x,y center of string

 virtual void draw(char *dest); //draw this GUI element
 virtual char *getname() {return "guiimagebutton";}
};


//check box
class GUIcheckbox:public GUIrect
{
 protected:
 int checked;
 int depressed;
 char text[80];
 public:

 GUIcheckbox(GUIrect *p,char *s,int x,int y,int state);
 virtual int acceptfocus() {return 1;} //do we need to be topmost ever?
 virtual int keyhit(char kbscan,char key); //return 1 if processed

 virtual GUIrect * click(class mouse &m); //mouse was clicked over this region
 virtual int release(class mouse &m); //mouse was released over this region
 virtual void draw(char *dest); //draw this GUI element

 void setstate(int s) {checked=s;}
 int getstate() {return checked;}
 virtual char *getname() {return "guicheckbox";}
};

//-----------------------------------

//GUIeditcontrol
class GUIedit:public GUIrect
{
 protected:
 char prompt[80]; //prompt for input
 int promptw;     //width of prompt in pixels
 int inputmaxw;   //maximum width of input

 public:
 int disabled;
 GUIedit(GUIrect *p,char *pmpt,int x,int y,int xw);
 virtual int acceptfocus() {return 1;} //do we need to be topmost ever?

 virtual GUIrect * hittest(int x,int y);
 virtual GUIrect * click(class mouse &m); //mouse was clicked over this region
 virtual void draw(char *dest); //draw this GUI element
 virtual void drawdata(char *dest,int x,int y,int xw) {return;}; //draw edited data

 virtual char *getname() {return "guiedit";}
};

class GUItextedit:public GUIedit
{
 protected:
 int maxinputlen;
 int inputlen;    //length of input in chars
 int inputw;       //current width of input
 char input[128];  //input

 void backspace();
 void addchar(char c);
 public:
 GUItextedit(GUIrect *p,char *pmpt,char *input,int x,int y,int xw,int maxinputlen);

 void paste();
 virtual void drawdata(char *dest,int x,int y,int xw);
 virtual int isvalidkey(char key);
 virtual int keyhit(char kbscan,char key); //return 1 if processed
 char *getinput() {return input;}
 void getinputw();
 void setinput(char *inp);
 virtual char *getname() {return "guitextedit";}
};

//a text number
class GUInumbertextedit:public GUItextedit
{
 public:
 GUInumbertextedit(GUIrect *p,char *pmpt,int input,int x,int y,int xw,int maxinputlen);
 virtual int isvalidkey(char key);
 int getstate();
 virtual char *getname() {return "guinumbertextedit";}
};

class GUInumberedit:public GUIedit
{
 protected:
 GUIimagebutton *up,*down;
 virtual void clip()=0;
 virtual void setmax()=0;
 virtual void clear()=0;
 public:
 GUInumberedit(GUIrect *p,char *pmpt,int x,int y,int xw);

 virtual int keyhit(char kbscan,char key); //return 1 if processed
 virtual void draw(char *dest);
};

class GUIintedit:public GUInumberedit
{
 int min,max; //min max of integer
 int n;
 public:
 GUIintedit(GUIrect *p,char *pmpt,int x,int y,int xw,int num,int min,int max);

 void add(int num);
 void minus(int num);
 virtual void clip();
 virtual void clear();
 virtual void setmax();


 virtual void drawdata(char *dest,int x,int y,int xw);
 int get() {return n;}
 void set(int num)
  {
   if (num==n) return;
   n=num; clip();
   if (parent) parent->sendmessage(this,GUIMSG_EDITCHANGED);
  }
 virtual char *getname() {return "guiintedit";}
 virtual int sendmessage(GUIrect *c,int msg);
};

class GUIfloatedit:public GUInumberedit
{
 float min,max; //min max of integer
 float n;
 public:
 GUIfloatedit(GUIrect *p,char *pmpt,int x,int y,int xw,float num,float min,float max);

 void add(float num);
 void minus(float num);
 virtual void clip();
 virtual void clear();
 virtual void setmax();

 virtual void drawdata(char *dest,int x,int y,int xw);
 float get() {return n;}
 void set(float num)
  {
   if (n==num) return;
   n=num; clip();
   if (parent) parent->sendmessage(this,GUIMSG_EDITCHANGED);
  }
 virtual char *getname() {return "guifloatedit";}
 virtual int sendmessage(GUIrect *c,int msg);
};


//------------------------------------

class GUIbox:public GUIrect
{
 protected:
 char title[80];
 GUIbutton *close; //close button

 public:
 GUIcontents *contents; //contents of this window
 GUIbox(GUIrect *p,char *titlestr,GUIcontents *c,int x,int y);
 virtual int acceptfocus() {return 1;} //do we need to be topmost ever?

 virtual void receivefocus()
  {
   bringtofront();
   GUIrect::receivefocus();
   if (contents) contents->receivefocus();
  }

 void settitle(char *s);
 virtual void refresh(int r,void *c)
  {
   if (contents) contents->refresh(r,c);
  };
 virtual void losechildfocus()
 {
  GUIrect::losechildfocus();
  if (contents->focus) focus=1; //keep focus if child still has it
 }

 void reposclosebutton();
 void resize(int xw,int yw)
  {
   GUIrect::resize(xw,yw);
   reposclosebutton();
  }

 virtual GUIrect *click(class mouse &m);
 virtual int release(class mouse &m);
 virtual int drag(class mouse &m);
 virtual void draw(char *dest);
 virtual int keyhit(char kbscan,char key); //return 1 if processed

 virtual int sendmessage(GUIrect *c,int msg);
 virtual char *getname() {return "guibox";}
};

class GUImaximizebox:public GUIbox
{
 GUIbutton *max; //maximize button
 public:
 int maximized; //are we maximized?
 GUImaximizebox(GUIrect *p,char *titlestr,GUIcontents *c,int x,int y);

 virtual void draw(char *dest);
 virtual char *getname() {return "guimaximizebox";}

 virtual void bringtofront()
  {
   if (maximized) return;
   GUIrect::bringtofront();
  }

 void maximize();
 void restore();
 void reposmaxbutton();
 virtual  GUIrect *hittest(int x,int y);
 virtual int sendmessage(GUIrect *c,int msg);
};

class GUIonebuttonbox:public GUIbox
{
 GUIbar *bar;
 GUIbutton *b1;
 public:
 GUIonebuttonbox(GUIrect *p,char *str,GUIcontents *c,char *b1name,int x,int y);

 virtual int sendmessage(GUIrect *c,int msg);
 virtual int keyhit(char kbscan,char key); //return 1 if processed

 virtual char *getname() {return "guionebuttonbox";}
};

class GUItwobuttonbox:public GUIbox
{
 GUIbar *bar;
 GUIbutton *b1,*b2;
 public:
 GUItwobuttonbox(GUIrect *p,char *str,GUIcontents *contents,char *b1name,char *b2name,int x,int y);

 virtual int sendmessage(GUIrect *c,int msg);
 virtual int keyhit(char kbscan,char key); //return 1 if processed

 virtual char *getname() {return "guitwobuttonbox";}
};


//------------------------------------

//track button for scroll bars
class GUItrackbutton:public GUIbutton
{
 protected:
 class GUItrack *ptrack; //track parent that we're on
 int tpos; //thumb sliding mouse relative variable
 public:
 GUItrackbutton(class GUItrack *p,int size);

 virtual char *getname() {return "guitrackthumb";}
};


class GUIvtrackbutton:public GUItrackbutton
{
public:
 GUIvtrackbutton(class GUIvtrack *p,int size);
 virtual GUIrect *click(class mouse &m);
 virtual int drag(class mouse &m);
};

class GUIhtrackbutton:public GUItrackbutton
{
public:
 GUIhtrackbutton(class GUIhtrack *p,int size);
 virtual GUIrect *click(class mouse &m);
 virtual int drag(class mouse &m);
};


//track for scroll bars
class GUItrack:public GUIrect
{
 protected:
 GUItrackbutton *thumb;
 int tracklen; //length of track
 int trackpos; //position of track thumb

 class GUIscrollbar *pscroll;

 public:
 GUItrack(GUIscrollbar *p,int x,int y,int xw,int yw);

 void movethumb(int tpos); //move trackpos
 void setthumb();          //set trackpos rigidly against pos stepping
 int gettrackpos() {return trackpos;}

 void settrackfrompos(int pos,int min,int max);

 virtual void positionthumb()=0;
 virtual void setthumbsize(int posrange)=0;

 virtual void draw(char *dest);
};

//vertical track
class GUIvtrack:public GUItrack
{
 public:
 GUIvtrack(class GUIvscrollbar *p);
 virtual void positionthumb();
 virtual void setthumbsize(int posrange);
 virtual char *getname() {return "guivtrack";}
};

//horizontal track
class GUIhtrack:public GUItrack
{
 public:
 GUIhtrack(class GUIhscrollbar *p);
 virtual void positionthumb();
 virtual void setthumbsize(int posrange);
 virtual char *getname() {return "guihtrack";}
};

//scroll bar
class GUIscrollbar:public GUIrect
{
 protected:
 int pos;      //positin with scroll range
 int min,max; //min/max range
 void clip(); //clip position to min/max

 //track
 public:
 GUIimagebutton *up,*down;
 GUItrack *track;

 GUIscrollbar(GUIrect *p,int x,int y,int xw,int yw);

 void setrange(int tmin, int tmax);
 void setpos(int p);
 int getpos() {return pos;}
 void scroll(int x) {setpos(pos+x);}
 void scrollup() {scroll(-1);}
 void scrolldown() {scroll(+1);}

 void setposfromtrack(int tpos,int tlen);
 virtual int acceptfocus() {return 1;}

 virtual void draw(char *dest);

 virtual int sendmessage(GUIrect *c,int msg);

 virtual char *getname() {return "guiscrollbar";}
};

//vertical scroll bar
class GUIvscrollbar:public GUIscrollbar
{
 public:
 GUIvscrollbar(GUIrect *p,int x,int y,int height);
 virtual int keyhit(char,char);

 virtual char *getname() {return "guivscrollbar";}
};

//horizontal scroll bar
class GUIhscrollbar:public GUIscrollbar
{
 public:
 GUIhscrollbar(GUIrect *p,int x,int y,int width);
 virtual int keyhit(char,char);

 virtual char *getname() {return "guihscrollbar";}
};


//--------------------------------------
//list box
typedef void *ITEMPTR;

class GUIlistbox:public GUIrect
{
 uutimer timer;
 uutimer dblclick;
 int depressed;

protected:
 int numitems; //number of items in listbox
 ITEMPTR *items; //pointer to array of itemptrs

 int sel; //currently selected item
 int itemheight; //height of each item
 int itemv; //number of displayable items vertically

 GUIvscrollbar *scroll;

public:
 void freeitems();
 ITEMPTR *resizeitems(int n);
 ITEMPTR *getitems() {return items;}
 int getnumitems() {return numitems;}

 void clearsel();
 void setsel(int s);
 int getselnum() {return sel;}
 ITEMPTR getselptr() {return (sel<0 || !numitems) ? 0 : items[sel]; }

 virtual GUIrect *click(class mouse &m);
 virtual int release(class mouse &m);
 virtual int drag(class mouse &m);

 GUIlistbox(GUIrect *p,int x,int y,int xw,int iy,int iheight);
 virtual ~GUIlistbox() {freeitems();};


 virtual int acceptfocus() {return 1;}
 virtual void draw(char *dest);
 virtual void drawitems(char *dest,int x,int y) {};

 virtual int keyhit(char kbscan,char key); 

 virtual char *getname() {return "guilistbox";}
};


class GUIstringlistbox:public GUIlistbox
{
 public:
 virtual void drawitems(char *dest,int x,int y);

 GUIstringlistbox(GUIrect *p,int x,int y,int xw,int iy,int iheight)
  :GUIlistbox(p,x,y,xw,iy,iheight) {};
 char *getselptr() {return (char *)GUIlistbox::getselptr();}

 virtual char *getname() {return "guistringlistbox";}
};


#include "dlgpos.h"

#endif








