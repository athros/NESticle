

#define MBLEFT   1
#define MBRIGHT  2
#define MBMIDDLE 4

class mouse
{
public:
 int hidden;
 int oldb; //last mouse button state
 int oldx,oldy; //last coords

 int x,y; //current mouse position
 int b;   //current holding down of mouse buttons
 char click; //click state of buttons
 char rel;  //release state of buttons

 class GUIrect *capture; //is the mouse captured? (in dragging)

 mouse(); //constructor

 void draw(char *dest); 
 void updatexy(int newx,int newy);
 void updatebut(int newbut);
 void reset() {oldx=x; oldy=y;}
 void showcursor() {hidden=0;}
 void hidecursor() {hidden=1;}
};


