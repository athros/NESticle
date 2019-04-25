#ifndef _CLIP_
#define _CLIP_

class clipdata
{
 public:
 clipdata();
 virtual ~clipdata() {}
 virtual void paste()=0; //paste the clip data
};


class clip_images:public clipdata
{
 class objectdef *od; //objectdef that the images reference
 int numimages; //number of images copied
 int size; //size in bytes
 struct image *i; //pointer to images copied

 public:
 clip_images(struct frame *fptr,class objectdef *od); //copy images from a frame
 virtual ~clip_images();

 virtual void paste();
};

class clip_frame:public clipdata
{
 class objectdef *od; //objectdef of frames
 int numf; //number of frames
 int size; //size of frames in bytes
 struct frame *f; //pointer to frames copied

 public:
 clip_frame(struct frame *fptr,int tnumf,class objectdef *od); //copy images from a frame
 virtual ~clip_frame();

 virtual void paste();
};



class clip_effects:public clipdata
{
 class objectdef *od; //objectdef that the images reference
 int numeffects; //number of effects copied
 int size; //size in bytes
 struct effect *e; //pointer to images copied

 public:
 clip_effects(struct frame *fptr,class objectdef *od); //copy effects from a frame
 virtual ~clip_effects();

 virtual void paste();
};



//clipboard class
class CLIPBOARD
{
 class clipdata *cd; //current data contained in clipboard
 public:
 CLIPBOARD():cd(0) {};
 ~CLIPBOARD() {if (cd) delete cd;}

 void adddata(clipdata *t)
  {
   if (cd) delete cd; //remove old data
   cd=t;
  }
 void paste()
  {
   if (cd) cd->paste();
  }
};
extern CLIPBOARD clipboard; //main clipboard




#endif