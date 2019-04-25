#include "volbatch.h"

//data from guivol
struct GUIVOL:volbatch
{
 struct PALETTE *pal; //default palette
 struct IMG *cursor; //cursor
 struct FONT *font; //first font
 struct IMG *checkmark; //checkmark
 struct IMG *umark,*dmark,*lmark,*rmark; //arrows for scroll bar
 struct IMG *about; //buddy's head
 struct IMG *shadowsel; //dumb shadow

 struct IMG *play,*stop,*playlooped,*active; //control icons

 struct IMG *xmark;

 virtual int size();
};
extern GUIVOL guivol;



