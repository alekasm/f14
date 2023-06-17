#include "library.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

extern RPS  *RpCRT;
extern RPS  *Rp3D;
extern int   OurHead;
extern int   OurPitch;
extern int	 OurRoll;
extern long  PxLng;
extern long  PyLng;
extern UWORD Px, Py;
extern UWORD Alt;
extern int   Rtime;
extern UBYTE FlirTbl[];
extern UWORD NOASPECT;
extern UWORD GRPAGE0[];
extern int   MouseDsp;
extern long	 ViewX;
extern long  ViewY;
extern int	 ViewZ;

extern int   MouseCrtX;
extern int   MouseCrtY;
extern int   vHead,vPitch,vRoll;

extern int ABOVECLOUDS;
extern int NEMSSEG;

RPS GunRp={ 1, 119,138,82,57,JAM3,GREEN,BLACK,1};

struct	FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
};

extern struct FPARAM FPARAMS;
extern struct PlaneData far *GetPlanePtr(int PlaneNo);

int   GunZoom=1;

//***************************************************************************
int DrawGunCameraView()
{
    int     TM[9], RM[9], PM[9],phd,pptchmin;
	RPS *Rtmp;
	int   PadHead,PadPitch;
	long  delx,dely;
	int   delz;
	int n;
	char  str[80];
	struct PlaneData far *tempplane;

	Rtmp = Rp3D;
	Rp3D = RpCRT;

	SetFont(Rp3D,F_3x5B);

	Rp3D->APen= 1;

	if (DESIGNATED == -1) {
		/* Establish our view point */
		ViewX = PxLng;
		ViewY = PyLng;
		ViewZ = Alt+3;
		vHead = OurHead;
		vPitch = OurPitch;
		vRoll = OurRoll;

		if (abs(vPitch) > 0x4000 || vPitch==0x8000) {
			vPitch = 0x8000 - vPitch;
    		vHead = vHead + 0x8000;
    		vRoll = 0x8000 - vRoll;
		}

		if (ViewZ<9) ViewZ = 9;
		NDraw3D(-vHead,vPitch,vRoll, ViewX,ViewY, (long) ViewZ);
		RpPrint(Rp3D,8,48,"NO LOCK 1X");
	} else {
		ViewX=ps[DESIGNATED].xL;
		ViewY=(0x100000-ps[DESIGNATED].yL);
		ViewZ=ps[DESIGNATED].alt;

		delx=PxLng-ViewX;
		dely=PyLng-ViewY;
		delz=Alt-ViewZ;

		delx>>=2;
		dely>>=2;
		delz>>=2;

		PM[0]=-delx;
		PM[1]=-delz;
		PM[2]=-dely;
		PadHead=ARCTAN(PM[0],PM[2]);
		PadPitch=ARCTAN(PM[1]/2,Dist2D(PM[0]/2,PM[2]/2));

		vHead=PadHead;
		vPitch=PadPitch;
		vRoll = OurRoll;

		delx/= GunZoom;
		dely/= GunZoom;
		delz/= GunZoom;
		ViewX+=delx;
		ViewY+=dely;
		ViewZ+=delz;

		if (abs(vPitch) > 0x4000 || vPitch==0x8000) {
    		vPitch = 0x8000 - vPitch;
    		vHead = vHead + 0x8000;
    		vRoll = 0x8000 - vRoll;
		}

   		if (ViewZ<9) ViewZ = 9;

		if (ViewZ<(120/4)) {
			if (n=OnTheDeck((long)(ViewX*100L),(long)(((long)(ViewZ))*400L),(long)((0x100000L-ViewY)*100L))) {
				if (ViewZ<(72/4))
					ViewZ=(72/4);
				if (n<0)
					ViewZ= (120/4);
			}
		}

		NDraw3D(-vHead,vPitch,vRoll, ViewX,ViewY, (long) ViewZ);

		tempplane = GetPlanePtr(ps[DESIGNATED].type);
		FTXT (tempplane->name);
		RpPrint(Rp3D,8,40,Txt);

		itoa(GunZoom,str,10);
		strcat(str,"X");
		RpPrint(Rp3D,8,48,str);
	}
	Rp3D = Rtmp;
}

int GunCameraZoom(int dir)
{
   GunZoom += dir;
   if (GunZoom<1) GunZoom=1;
   if (GunZoom>20) GunZoom=20;
}

