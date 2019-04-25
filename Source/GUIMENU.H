//*************************************
//           menu classes
//*************************************

//menu item
typedef void (*MENUFUNCPTR)();
class menuitem
{
public:
 char *text;     //text for this menu item
 MENUFUNCPTR func; //function to call
 char key;       //shortcut key
 class menu *submenu;  //submenu

 void draw(int x,int y,int xw,int sel); //draw menuitem
 int width(); //finds width of this item
 int height(); //finds height of this item
 int menufunc(); //does the menu function
};

//menu
class menu
{
public:
class menuitem m[];
int getnumitems(); //returns the number of items in menu

int getmaxwidth(); //returns the maximum width of all items
int gettotalwidth(); //returns the total width of all items

int getmaxheight(); //returns the maximum height of all items
int gettotalheight(); //returns the total height of all items

int keyhit(char kbscan,char key);
};

//menu gui class
class GUImenu:public GUIrect
{
protected:
 class menu *tmenu; //menu represented by this GUI
 class menuitem *selmi; //currently selected menuitem, if any
public:
 GUImenu(GUIrect *p,class menu *m,int x,int y);

 virtual GUIrect *click(class mouse &m); //mouse was clicked over this region
 virtual int release(class mouse &m); //mouse was released over this region
 virtual int drag(class mouse &m); //mouse is being dragged

 virtual int acceptfocus() {return 1;} //menu can get focus
 virtual void losefocus(); //menu never loses focus
 virtual void losechildfocus(); //children cannot die

 virtual void receivefocus()
  {
   bringtofront();
   GUIrect::receivefocus();
  }
 virtual void sendtoback() {} 
 virtual class menuitem *menuhittest(int x,int y, int &sx,int &sy)=0;

  //width height of menu
 virtual int getmenuwidth() =0;
 virtual int getmenuheight() =0;

 virtual int domenuitem(menuitem *i);

 virtual char *getname() {return "guimenu";}
};


class GUIvmenu:public GUImenu
{
 public:
 GUIvmenu(class GUImenu *p,class menu *m,int x,int y);

 virtual void draw(char *dest); //draw this GUI element
 virtual class menuitem *menuhittest(int x,int y, int &sx,int &sy);
 virtual char *getname() {return "guivmenu";}

 virtual int getmenuwidth() {return tmenu->getmaxwidth()+8;};
 virtual int getmenuheight() {return tmenu->gettotalheight();}

};

class GUIhmenu:public GUImenu
{
 public:
 GUIhmenu(GUIrect *p,class menu *m,int x,int y);

 virtual void draw(char *dest); //draw this GUI element
 virtual class menuitem *menuhittest(int x,int y, int &sx,int &sy);
 virtual int keyhit(char kbscan,char key);

 virtual int getmenuwidth() {return tmenu->gettotalwidth();};
 virtual int getmenuheight() {return tmenu->getmaxheight();}

 virtual char *getname() {return "guihmenu";}
};


class GUIpopupmenu:public GUIvmenu
{
 GUIrect *report;
 public:
 GUIpopupmenu(GUIrect *treport,class menu *m, int x,int y);
 ~GUIpopupmenu();
 virtual int domenuitem(menuitem *t);
 virtual char *getname() {return "guipopup";}
 virtual int release(mouse &m);
 virtual void draw(char *dest);
};

