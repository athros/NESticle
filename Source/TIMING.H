
//cycles per hblank
extern int TIMERSPEED;
extern int HBLANKCYCLES;
extern int VBLANKLINES;

//line to begin drawing at
#define STARTFRAME 8

//total lines in viewable frame
#define FRAMELINES 232

//total lines
#define TOTALLINES (VBLANKLINES+FRAMELINES)

//number of cycles to execute per frame
#define CYCLESPERTICK (TOTALLINES*HBLANKCYCLES)

//cycles during frame drawing
#define FRAMECYCLES (FRAMELINES*HBLANKCYCLES)

//cycles during vblank
#define VBLANKCYCLES (VBLANKLINES*HBLANKCYCLES)




/*#define TIMERSPEED 60

//cycles per second
#define CYCLES     1800000

//number of cycles to execute per frame
#define CYCLESPERTICK (CYCLES/TIMERSPEED)

//lines
#define STARTFRAME 8
#define ENDFRAME   232
#define TOTALLINES 265

#define FRAMELINES (ENDFRAME-STARTFRAME)
#define VBLANKLINES (TOTALLINES-FRAMELINES)

//cycles during frame draw
#define FRAMECYCLES (CYCLESPERTICK*FRAMELINES/TOTALLINES)

//cycles during vblank
#define VBLANKCYCLES (CYCLESPERTICK*VBLANKLINES/TOTALLINES)

//cycles per hblank
#define HBLANKCYCLES (CYCLESPERTICK/TOTALLINES)
*/



/*
#define TIMERSPEED 60

//cycles per second
#define CYCLES     1800000

//number of cycles to execute per frame
#define CYCLESPERTICK (CYCLES/TIMERSPEED)

//lines
#define FRAMELINES 240
#define TOTALLINES 265
#define VBLANKLINES (TOTALLINES-FRAMELINES)

//cycles during frame draw
#define FRAMECYCLES (CYCLESPERTICK*FRAMELINES/TOTALLINES)

//cycles during vblank
#define VBLANKCYCLES (CYCLESPERTICK*VBLANKLINES/TOTALLINES)

//cycles per hblank
#define HBLANKCYCLES (CYCLESPERTICK/TOTALLINES)
*/


/*
//ticks per second
#define TIMERSPEED 60

//cycles per hblank
#define HBLANKCYCLES 115

#define TICKLINES   265
#define FRAMELINES  240

//number of cycles to execute per frame
#define CYCLESPERTICK (TICKLINES*HBLANKCYCLES)

//cycles during frame drawing
#define FRAMECYCLES (FRAMELINES*HBLANKCYCLES)

//cycles during vblank
#define VBLANKCYCLES (CYCLESPERTICK-FRAMECYCLES)

//cycles per second
#define CYCLES (CYCLESPERTICK*TIMERSPEED)

  */








