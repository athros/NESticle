#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "file.h"

#ifdef __BORLANDC__
#include <dir.h>
#endif

#ifdef __WATCOMC__
#include <dos.h>
#endif

#ifdef _MSC_VER
#include "windows.h"
#endif


void enumdir(char *path, DIRFUNCPTR func,void *context)
{
// #ifdef WIN95
 #ifdef __BORLANDC__
 ffblk ff;
 int done=findfirst(path,&ff,0);
 while (!done)
  {
   if (!func(ff.ff_name,context)) break;
   done=findnext(&ff);
  }
 #endif
 #ifdef __WATCOMC__
 find_t ff;
 int done=_dos_findfirst(path,0,&ff);
 while (!done)
  {
   if (!func(ff.name,context)) break;
   done=_dos_findnext(&ff);
  }
 _dos_findclose(&ff);
 #endif

 #ifdef _MSC_VER
 WIN32_FIND_DATA ff;
 HANDLE h=FindFirstFile(path,&ff);
 if (h==INVALID_HANDLE_VALUE) return;
 do
 {
   if (!func(ff.cFileName,context)) break;
 } while (FindNextFile(h,&ff));

 FindClose(h);
 #endif
// #endif
  /*
 #ifdef DOS
 find_doslf ff;
 unsigned h=findfirst_doslf(path,&ff);
 if (!h) return;
 do
 {
   if (!func(ff.filename,context)) break;
 } while (findnext_doslf(h,&ff));

 findclose_doslf(h);
 #endif*/
}




//---------------------------------------------
//---------------------------------------------
#include "message.h"

int FILEIO::open(char *filename)
{
 if (h) close(); //already open
 _fmode=O_BINARY;
 #ifdef WIN95
 h=::open(filename,O_RDWR|O_BINARY);
 #endif
 #ifdef DOS
// h=open_doslf(filename);
 h=::open(filename,O_RDWR|O_BINARY);
 #endif
 if (h==-1) {h=0; return -1;} //error
 return 0;
}

int FILEIO::create(char *filename)
{
 if (h) close();
 _fmode=O_BINARY;
 #ifdef WIN95
 #ifdef __WATCOMC__
 h=creat(filename,S_IWRITE|S_IREAD);
 #endif
 #ifdef __BORLANDC__
 h=_rtl_creat(filename,0);
 #endif
 #ifdef _MSC_VER
 h=creat(filename,S_IWRITE|S_IREAD);
 #endif
 #endif

 #ifdef DOS
 h=creat(filename,S_IWRITE|S_IREAD);
// h=create_doslf(filename);
 #endif

 if (h==-1) {h=0; return -1;} //error
 return 0;
}

void FILEIO::close()
{
 if (!h) return;
 ::close(h);
 h=0;
}

int FILEIO::read(void *t,unsigned size)
{
 if (!h) return -1;
 return ::read(h,t,size)<size ? -1 : 0 ;
}

int FILEIO::write(void *t,unsigned size)
{
 if (!h) return -1;
 return ::write(h,t,size)<size ? -1 : 0;
}

unsigned FILEIO::size()
{
 return h ? filelength(h) : 0;
}

unsigned FILEIO::getpos()
{
 return h ? lseek(h,0,SEEK_CUR) : 0;
}

void FILEIO::setpos(unsigned p)
{
 if (h) lseek(h,p,SEEK_SET);
}




/*
#ifdef DOS
//---------------------------------------
//dos long filename functions

unsigned findfirst_doslf(char *path,find_doslf *ff)
{
 REGS regs;
 SREGS sregs;
 memset( &sregs, 0, sizeof(sregs) );

 regs.x.eax = 0x714E;
 regs.x.esi = 0x0;
 regs.x.ecx = 0x0;
 sregs.ds=   FP_SEG( path );
 regs.x.edx= FP_OFF( path );
 sregs.es=   FP_SEG( ff );
 regs.x.edi= FP_OFF( ff );
 int386x( 0x21, &regs, &regs, &sregs );
 return (regs.x.cflag&INTR_CF) ? 0 : regs.x.eax; //return handle
};


unsigned findnext_doslf(unsigned handle,find_doslf *ff)
{
 REGS regs;
 SREGS sregs;
 memset( &sregs, 0, sizeof(sregs) );

 regs.x.eax = 0x714F;
 regs.x.esi = 0x0;
 regs.x.ebx = handle;
 sregs.es=   FP_SEG( ff );
 regs.x.edi= FP_OFF( ff );
 int386x( 0x21, &regs, &regs, &sregs );
 return !(regs.x.cflag&INTR_CF); //return 0 on error/done
};
void findclose_doslf(unsigned handle)
{
 REGS regs;
 SREGS sregs;
 memset( &sregs, 0, sizeof(sregs) );

 regs.x.eax = 0x71A1;
 regs.x.ebx = handle;
 int386x( 0x21, &regs, &regs, &sregs );
};

void canonicalize_doslf(char *path,char *cpath)
{
 REGS regs;
 SREGS sregs;
 memset( &sregs, 0, sizeof(sregs) );

 regs.x.eax = 0x7160;
 regs.x.ecx = 0x0002;
 sregs.ds=   FP_SEG( path );
 regs.x.esi= FP_OFF( path );
 sregs.es=   FP_SEG( cpath );
 regs.x.edi= FP_OFF( cpath );
 int386x( 0x21, &regs, &regs, &sregs );
 if (regs.x.cflag&INTR_CF) //if error
   strcpy(cpath,path); //just copy it
 msg.printf(2,"canoc: %s -> %s %d",path,cpath,regs.x.eax);

}

unsigned open_doslf(char *path)
{
 char s[256]; //canonicalized path
 canonicalize_doslf(path,s);
 return open(s,O_RDWR|O_BINARY);
};
unsigned create_doslf(char *path)
{
 char s[256]; //canonicalized path
 canonicalize_doslf(path,s);
 return creat(s,S_IWRITE|S_IREAD);
};

#endif
*/



/*
//dos long file name functions
#ifdef DOS
struct QWORD {unsigned low,high;};
typedef unsigned DWORD;

struct find_doslf //structure for long file names dir searching
{
DWORD attrib;
QWORD createtime;
QWORD accesstime;
QWORD modifytime;
DWORD sizehigh,sizelow;
char reserved[8];
char filename[260];
};

unsigned findfirst_doslf(char *path,find_doslf *ff);
unsigned findnext_doslf(unsigned handle,find_doslf *ff);
void     findclose_doslf(unsigned handle);
unsigned open_doslf(char *path);
unsigned create_doslf(char *path);
#endif
  */



