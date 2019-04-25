#ifndef _SOUND_
#define _SOUND_

//sound sampling rate
extern int SOUNDRATE;

// Length of digitized sound output block in samples
#define BLOCK_LENGTH    256

//standard sound structure, 8-bit signed
struct SOUND
{
  unsigned long soundsize;
  signed   char soundptr[];
};

struct SOUND *ReadWavFile(char *name);
#endif

