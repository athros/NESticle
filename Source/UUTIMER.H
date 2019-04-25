#ifndef _UUTIMER_
#define _UUTIMER_
extern volatile unsigned uu;
//cheap timer
class uutimer
{
 public:
 unsigned x;
 unsigned dur; //duration of timer

 void clear() {dur=0; x=0xFFFFFFF;}
 void set(unsigned tdur) {dur=tdur; x=uu+dur;}
 void reset() {set(dur);}
 int check() {return uu>=x;}
 uutimer() {clear();}
 uutimer(unsigned tdur) {set(tdur);}
};
#endif

