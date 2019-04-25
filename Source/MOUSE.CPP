
#include "mouse.h"
#include "gui.h"

#include "message.h"
#include "r2img.h"
#include "guivol.h"

mouse::mouse()
{
 oldb=0;
 x=y=oldx=oldy=0; click=rel=b=0;
 capture=0;
 showcursor();
}

void mouse::updatexy(int newx,int newy)
{ //set new x,y
 x=newx; y=newy;
// if (GUIrect::modal) capture=GUIrect::modal;

 if (x!=oldx || y!=oldy)
  {
   if (capture) capture->drag(*this);
  }
}

void enablegui();
void mouse::updatebut(int newb)
{
 oldb=b; //save old button state

 b=newb;

 click=b&(oldb^7); //press triggers
 rel= oldb&(b^7); //release triggers


 //do hit testing
 if (click) //button was clicked
 {
  if (click&1) enablegui();
  if (GUIrect::modal)
   {
     GUIrect::modal->click(*this);
//     msg.printf(2,"modal clicked");
   }
   else
  if (!capture) //not captured
  {
   capture=root->hittest(x,y);
   if (capture)
   {
    capture=capture->click(*this);
    if (capture) capture->drag(*this);
   }
  } else  capture=capture->click(*this);
 }
 if (rel) //button was released
  if (capture)
  {
   if (capture->release(*this)) capture=0;
   if (GUIrect::modal) capture=GUIrect::modal;
  }

}


void mouse::draw(char *dest)
{
 if (!hidden)
 guivol.cursor->draw(dest,x,y);
}







