#include "Types.H"
// #include "3d5.h"
#include <io.h>
#include <stdio.h>
#include <dos.h>
#include <fcntl.h>
#include <string.h>
#include "world.h"
#include "library.h"

#define ERRORCHECKS 1

extern RPS *Rp1;
extern char Txt[];


extern UWORD Seg3D, EMSseg;
extern UWORD GRPAGE0[];

FILE    *File;

UWORD   ObjHands[NrEMSHandles];
UWORD   ObjSegs[NrEMSHandles];
int OldHandle = -1;


//***************************************************************************
UBYTE far *LoadEMSDataFile(char *FileName, int Slot)
{

    UBYTE far  *fptr;
    int         hand;
    int         fhand;
    UWORD       i;
    long        size;
    int         pages;

    int         free;
    int         total;


    _dos_open (FileName, O_RDONLY, &fhand);
    size  = filelength (fhand);
    pages = (size + 16383) /16384;

    EMSinit();

    EMSpages (&free, &total);
    if (free<pages)
      {
      _dos_close(fhand);
      ObjHands[Slot]=0;
      ObjSegs[Slot] = 0;
      return(0);
      }

    EMSalloc(pages, &hand);

    for (i=0; i<pages; i++) EMSmap(i,i,hand);

    ObjHands[Slot] = hand;
    FP_SEG(fptr) = ObjSegs[Slot] = EMSseg;
    FP_OFF(fptr) = 0;

    _dos_read(fhand, fptr, (UWORD) size, &i);
    _dos_close(fhand);

    return(fptr);
}

//***************************************************************************
void LoadObjFile(char *FileName, int Slot)
{
    UBYTE   far *fptr;

    fptr = LoadEMSDataFile(FileName, Slot);
}

//***************************************************************************
long LoadBigEMSFile(char *FileName, int Slot)
{
    UBYTE far  *fptr;
    int         hand;
    int         fhand;
    UWORD       i;
    long        size;
    int         pages;
    int         junk;
    int         free;
    int         total;


    _dos_open (FileName, O_RDONLY, &fhand);
    size  = filelength (fhand);
    pages = (size + 16383) /16384;

    EMSinit ();
    EMSpages (&free, &total);
    if (free<pages)
      {
      _dos_close(fhand);
      ObjHands[Slot]=0;
      ObjSegs[Slot] =0;
      return(0);
      }
//    EMSpages (&free, &total);
//    if (free<pages) pages=free;
    EMSalloc (pages, &hand);

    ObjHands[Slot] = hand;
    FP_SEG(fptr) = ObjSegs[Slot] = EMSseg;
    FP_OFF(fptr) = 0;

    for (i=0; i<pages; i++) {
        EMSmap (0,i,hand);
        _dos_read  (fhand, fptr, 16384, &junk);
    }

    _dos_close (fhand);

    return ((long)pages*16384);
}

//***************************************************************************
MapIn3dObjects(UWORD SegIX)
{
    UWORD   EMShand;

    EMShand = ObjHands[SegIX];

    if (EMShand && EMShand!=OldHandle) {
        OldHandle = EMShand;

        EMSmap (0,0,EMShand);
        EMSmap (1,1,EMShand);
        EMSmap (2,2,EMShand);
        EMSmap (3,3,EMShand);
    }

	Seg3D = ObjSegs [SegIX];
	return (Seg3D);
}

//***************************************************************************
int LoadEMSPicFile(char *FileName, int Slot, int page,int ALLOC)
{
    static int hand;
    UWORD i;
    int         free;
    int         total;

	LoadPicNoPal(1, FileName);

	if (ALLOC) {
    	EMSinit();
      EMSpages (&free, &total);
	  if (free<ALLOC)
         {
         ObjHands[Slot]=0;
         ObjSegs[Slot] =0;
         return(0);
         }
    	EMSalloc(ALLOC, &hand);
	 	ObjHands[Slot] = hand;
	}

	ObjSegs[Slot] = EMSseg;

    for (i=0; i<4; i++) EMSmap(i,i+4*page,hand);

    movedata(GRPAGE0[1],0,ObjSegs[Slot],0,(UWORD) 64000);
	return(EMSseg);
}

//***************************************************************************
MapInEMSSprite(UWORD SegIX,int page)
{
	UWORD SEG;

	SEG=0;
	if (SEG=MapIn3dObjects (SegIX))	{
    	EMSmap(0,4*page+0,ObjHands[SegIX]);
    	EMSmap(1,4*page+1,ObjHands[SegIX]);
		EMSmap(2,4*page+2,ObjHands[SegIX]);
		EMSmap(3,4*page+3,ObjHands[SegIX]);
	}
	return(SEG);
}

//***************************************************************************
FreeEMS()
{
    int i;

    for (i=0; i<NrEMSHandles; i++) {
        if (ObjHands[i]) EMSfree (ObjHands[i]);
    }
}

//***************************************************************************
Error (char *Str)
{
    ScreenOn ();
    Rp1->APen=WHITE;
    RpPrint (Rp1, 0,96, strupr(Str));
    getch ();
}
