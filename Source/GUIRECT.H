#ifndef _GUIRECT_
#define _GUIRECT_

//gui refresh types
#define GUIRFR_OBJSPACE   1
#define GUIRFR_OBJDEF     2
#define GUIRFR_OBJECT     3
#define GUIRFR_BGDEF      4
#define GUIRFR_BG         5
#define GUIRFR_SERIES     6
#define GUIRFR_FRAME      7
#define GUIRFR_STOPPED    8


extern int GUIdefx,GUIdefy;
void nextGUIdef();
void resetGUIdef();

//rectangle for receiving mouse clicks/drags/releases
class GUIrect
{
 public:
 GUIrect *parent; //this window's parent
 GUIrect *child,*lastchild; //linked list of children, topmost first
 GUIrect *prev,*next; //linked list of them
 char focus; //do we (or our children) have input focus?
 GUIrect *lastfocus; //last child of ours that had focus

 int x1,y1,x2,y2;

 friend void scanguitree(GUIrect *r,struct lpoint &p,GUIrect *parent);
 GUIrect(GUIrect *p,int,int,int,int);
 void unlink();
 void link(GUIrect *t);
 virtual ~GUIrect();

 virtual GUIrect *hittest(int x,int y); //see if coord is over this rect or children
 void reparent(GUIrect *p);
 virtual void bringtofront();
 virtual void sendtoback();
 void moverel(int dx,int dy); //move relative amount
 void moveto(int x,int y);
 void resize(int xw,int yw) {x2=x1+xw; y2=y1+yw;}
 int width() {return x2-x1;}
 int height() {return y2-y1;}
 void fill(char color);
 void outline(char color);

 //refresh this guirect's contents
 virtual void refresh(int r,void *c) {};

// int topmost(); //is this gui element topmost?
 static int setfocus(GUIrect *f);
 virtual int acceptfocus() {return 0;} //do we need to ever have focus?
 virtual void receivefocus();
 virtual void losefocus();
 virtual void losechildfocus(); //clear focus of all children
 void cyclefocus(int dir); //cycle focus between children needing focus

 virtual GUIrect *click(class mouse &m) {return 0;}; //mouse was clicked over this region
 virtual int release(class mouse &m) {return 1;}; //mouse was released over this region
 virtual int drag(class mouse &m) {return 0;}; //mouse is being dragged
 virtual void draw(char *dest); //draw this GUI element

 virtual int keyhit(char kbscan,char key); //return 1 if processed

 virtual char *getname() {return "guirect";}

 virtual int sendmessage(GUIrect *c,int msg) {return 1;}; //if child event happens

 static GUIrect *modal;
 static void setmodal(GUIrect *m);
};


class GUIcontents:public GUIrect
{
 public:
 GUIcontents(int xw,int yw):GUIrect(0,0,0,xw,yw) {};

 void resize(int xw,int yw);
 virtual void maximize() {};
 virtual void restore() {};
 virtual int keyhit(char kbscan,char key);
};





#endif