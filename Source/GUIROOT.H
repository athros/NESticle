#ifndef _GUIROOT_
#define _GUIROOT_

#include "guirect.h"

//root for which all elements are children
class ROOT:public GUIrect
{
 public:
 ROOT();
 virtual ~ROOT();

 void resize(int xw,int yw);

 virtual int keyhit(char kbscan,char key)
  {
    if (modal) return modal->keyhit(kbscan,key);
    return GUIrect::keyhit(kbscan,key);
  };
 virtual GUIrect *hittest(int x,int y)
  {
   GUIrect *result=GUIrect::hittest(x,y);
   if (result && !modal) setfocus(result);
   return result;
  };
 virtual void refresh(int r,void *c);

 virtual void bringtofront() {return;};
 virtual void sendtoback() {return;};

 virtual int acceptfocus() {return 1;} //root has focus
 virtual void losefocus() {return;} //root never loses focus

 virtual char *getname() {return "a32root";}
};

//root for which all gui elements are children
class GUIroot:public GUIrect
{
 public:
 GUIroot(ROOT *p);
 ~GUIroot();

 //no alteration in ordering
// virtual void bringtofront() {return;};
// virtual void sendtoback() {return;};
 virtual void refresh(int r,void *c)
  {
   for (GUIrect *g=child; g; g=g->next)  g->refresh(r,c);
  };

 virtual int acceptfocus() {return 1;} //root has focus
 virtual void losefocus() {return;} //root never loses focus

 virtual char *getname() {return "guiroot";}
 virtual int keyhit(char kbscan,char key); //return 1 if processed

// virtual GUIrect *hittest(int x,int y); //special processing
};

extern class ROOT *root;             //root of everything
extern class GUIroot *guiroot;      //root of all gui elements






#endif