//Copyright(c) 1996 Bloodlust Software All rights reserved
//volume file manager
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "vol.h"
#include "volbatch.h"

extern char errstr[];
extern void cleanexit(int x);

char dsl[4]="DSL";

//error handling portion  0=abort, 1=revolumize
int erroraction=0; //action to be taken if there is an error opening file

//0-ignore, return 0     1-abort
int vwerror=1;      //action to be taken while writing file

int vdiagnose=0;

char *voltype[]=
{
 "header",
 "rawdata",
 "image",
 "sound",
 "midi",
 "palette",
 "screen",
 "block"
 
};

//read an element header
int volumefile::readheader()
{
 if (f.read(&hdr,sizeof(header)) || strcmp(hdr.key,dsl))
  {
   sprintf(errstr,"corrupt volume file %s",volname); cleanexit(1);
   return VOL_ERR;
  }
 return VOL_OK;
}    

//opens volume file
int volumefile::open(char *filename)
{
 if (!strchr(filename,'.')) strcat(filename,VOLEXT);

 if (f.open(filename))  return VOL_ERR;

 char name[16];
 GetFileName(filename,name); //get name of file

 if (readheader()==VOL_ERR || hdr.type!=V_VOLHEADER || stricmp(hdr.name,name)) //invalid header
  {sprintf(errstr,"corrupt volume file %s",name); close(); cleanexit(-1);}
 strcpy(volname,hdr.name);

 if (vdiagnose)  printf("open:   %8s\n",filename);
 return(VOL_OK); 
}    

//resets to beginning of file
void volumefile::reset()
{
 f.setpos(0);
 readheader();
}    

//closes volume file
void volumefile::close()
{
 f.close();
 if (vdiagnose) printf("close:  %8s\n\n",volname);
}    



//reads single file from volume file, allocs memory for it
void *volumefile::read()
{
 readheader();

 void *t=0;
 if (hdr.size)
  {
   t=f.readalloc(hdr.size);
   if (!t) {sprintf(errstr,"reading volume %s size: %d",volname,hdr.size); cleanexit(-3);}
  }

 if (vdiagnose)
  printf("read:   %8s size: %7d type: %s\n",hdr.name,hdr.size,voltype[hdr.type]);
 return(t);
}



//reads single file from volume file to pointer
void volumefile::read(void *t)
{
 readheader();
 if (hdr.size)
  {
    if (f.read(t,hdr.size)) {sprintf(errstr,"reading volume %s size: %d",volname,hdr.size); cleanexit(-3);}
  }

 if (vdiagnose)
  printf("read:   %8s size: %7d type: %s\n",hdr.name,hdr.size,voltype[hdr.type]);
}


#include "message.h"
void ** volumefile::readblock(int &num)
{
 readheader();

 void **t=0;
 num=0;
 if (hdr.size)
  {
   num=f.readint(); //get number of items
   if (num)
   {
    t=(void **)f.readalloc(hdr.size-4); 
    if (!t) {sprintf(errstr,"reading volume %s size: %d",volname,hdr.size); cleanexit(-3);}

// printf("t=%p t[0]=%p \n",t,t[0]);  
   
    for (int i=0; i<num; i++) //adjust indices
     if (t[i]) ((unsigned *)t)[i]+=((unsigned)t); //+ num*4;
   }
  }
// msg.printf(2,"t=%p 0=%p 1=%p\n",t,t ? t[0] : 0,t ? t[1] : 0);  

 if (vdiagnose)
  printf("read:   %8s size: %7d type: %s numelements: %d\n",hdr.name,hdr.size,voltype[hdr.type],num);

 //msg.

 return(t);
}    

//skips ahead
void volumefile::skip()
{
readheader(); 

f.setpos(f.getpos()+hdr.size);
if (vdiagnose)
  printf("skipped:%8s size: %5d type: %2s\n",hdr.name,hdr.size,voltype[hdr.type]);
}    



void GetFileName(char *fullname, char *name)
{
 if (!fullname) {name[0]=0; return;}
 for (int i=0; fullname[i] && fullname[i]!='.'; i++); //find period
 int p=i; //period location
 for ( ; i>0 && fullname[i]!='\\'; i--); //find beginning of name
 if (i>0) i++;
 for (int j=0; i<p; i++,j++) name[j]=fullname[i]; //copy name
 name[j]=0;
}

void GetFileExtension(char *fullname, char *ext)
{
 for (int i=0; fullname[i] && fullname[i]!='.'; i++); //find period
 if (!fullname[i]) {ext[0]=0; return;} //no period found
 i++;
 for (int j=0; j<3; j++,i++) ext[j]=fullname[i];
 ext[j]=0;
}




//------------------------------------
//batch file processing

int volbatch::read(char *volfilename)
{
 volumefile v;
 if (v.open(volfilename)) return 0;
// printf("%s vol opened\n",volfilename);

 //find number of pointers to be read
 int num=(size()-sizeof(volbatch))/4;

 //get pointer to first pointer
 void **p= (void **) (((char *)this)+sizeof(volbatch));

// printf("num=%d this=%p p=%p\n",num,this,p);

 for (int i=0; i<num; i++)
  {
   p[i]=v.read();
   if (!p[i]) return 0; //not all read!
  }
  
 v.close();
// printf("vol closed\n");
 return 1;
};


void volbatch::free()
{
 //find number of pointers
 int num=(size()-sizeof(volbatch))/4;

 //get pointer to first pointer
 void **p= (void **) (((char *)this)+sizeof(volbatch));

 for (int i=0; i<num; i++)
  if (p[i]) ::free(p[i]);
}


void volbatch::print()
{
 //find number of pointers
 int num=(size()-sizeof(volbatch))/4;

 //get pointer to first pointer
 void **p= (void **) (((char *)this)+sizeof(volbatch));

// for (int i=0; i<num; i++)
//  printf("#%d: %p\n",i,p[i]);
}   





