/*----------*/
/*
/*  File:   3DObject.C
/*
/*  Auth:   Andy Hollis  11/25/87 <<<<< Amazing !!!
/*
/*  Edit:   Hacked yet again for F15 III, AWH - 8/92
/*          AND YET AGAIN FOR F-14, MJM 4/93
/*
/*  Routines to draw a 3D object. - NOT!!
/*
/*----------*/

#include "library.h"
// #include "3d5.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "F15Defs.h"
#include <dos.h>


/*** EXTERNAL PROCEDURES ***/

    extern  void    far    MakeRotationMatrix ();

//    extern  void    far    SetViewMat();
    extern  void    far    MakeMatHPR();


/*** GLOBAL 3D SYSTEM VARIABLES ***/

    /* Don't move these guys or the file unpacker won't work right !! */
    /* Also, StackObject requires this */

            UWORD   Object = 0;
            long    XEB, YEB, ZEB;

            int     OX=0, OY=0, OZ=0;           /* relative object location */

            int     SecondMatrix=0;
    static  int     Dx=0, Dy=0, Dz=0;
            int     Dist2Obj=0, Class=0;

            int     VX=0, VY=0, VZ=0;           /* Viewer location */
            int     HCent, VCent;               /* screen center */

            int     SinHead=0, CosHead=0;
            int     SinPitch=0, CosPitch=0;
            int     SinRoll=0, CosRoll=0;

            int     V [3][3]={0};               /* Viewer matrix */
            int     O [3][3]={0};               /* Object Matrix */
            int     C [3][3]={0};               /* Concatenation of V & O */

/***************** Stuff for Scott's 3D ************************/

SWORD AnimVals [32];
SWORD HitMe;
UWORD CRASHED;

extern int GXLO, GYLO, GXHI, GYHI, GXMID, GYMID, LightX, LightY, LightZ;
extern int NGXLO, NGYLO, NGXHI, NGYHI, NGXMID, NGYMID;
extern SWORD GRPAGE;
extern SWORD GRPAGE0[];
extern SWORD FTicks;
extern int   Panning;
extern int   GrndAnimVals[];
extern int   PlaneAnimVals[];
extern int	 CarrierAnimVals[];
extern char  far *NPDEST;
extern UWORD *PalPtr3D;


//***************************************************************************
Bgn3D(struct RastPort *Rp, int LHead, int LPitch, int LRoll, int X,
		int Y, int Z, int Std, int Zoom)
{
    SetDrawWindow (Rp);
    SetViewRot (LHead, LPitch, LRoll,Zoom);
    SetViewPos (X, Y, Z);
}

//***************************************************************************
End3D()
{
    SWORD d5, d10, d20;

	static	int	LizardMouth;
	static int beacon=0;

	int	L;

    d5 = 128*FTicks;
    d10 = d5<<1;
    d20 = d10<<1;

    GrndAnimVals[16] += d20<<1;
    GrndAnimVals[17] += d5<<1;
    PlaneAnimVals[17] = GrndAnimVals[17];
    PlaneAnimVals[18]^= 1;

	if (++LizardMouth>20) LizardMouth=0;
	L= LizardMouth;
	if (L>10) L=20-L;
	GrndAnimVals[14]= L*182;
	GrndAnimVals[13]= (L>4)? 1:0;

	if (++beacon>2) beacon=0;
	if (beacon==0) {
		*(PalPtr3D+31) = 0x9393;
	} else
		*(PalPtr3D+31) = 0x9292;

	UpdateOurPlaneAnims();

	CarrierAnimVals[16] += d10<<1;
	CarrierAnimVals[17] += d5<<1;

    memcpy (C, V, sizeof(V));
}

//***************************************************************************
SetDrawWindow (struct RastPort *Rp)
{
	int LWidth, LHeight;

	/* Andy's 3D vars */
    HCent = (Rp->Width1 +1) >>1;
    VCent = (Rp->Length1+1) >>1;

    if (Rp->YBgn == 0) {
//        HCent += Panning;
        if (!(View&VFULL)) {     /* Change of horizon point */
            VCent = 56;
//            if (View==DOWN) VCent -= LOOKDOWNDY;
        }
    }

    SetOvrlyRp (Rp);

	/* Set up Scott's 3D vars */
    GXLO = Rp->XBgn;
    GYLO = Rp->YBgn;
    GXHI = Rp->XBgn + Rp->Width1;
    GYHI = Rp->YBgn + Rp->Length1;
    GXMID = HCent + Rp->XBgn;
    GYMID = VCent + Rp->YBgn;
	GRPAGE=GRPAGE0[Rp->PageNr];
	NGXLO = GXLO;
	NGXHI = GXHI;
	NGYLO = GYLO;
	NGYHI = GYHI;
	NGXMID = GXMID;
	NGYMID = GYMID;

	FP_SEG(NPDEST)=GRPAGE;
	FP_OFF(NPDEST)=0;
}

//***************************************************************************
SetViewRot(int LHead, int LPitch, int LRoll, int Zoom)
{
    MakeRotationMatrix (V, -LHead, -LPitch, -LRoll);
    MakeMatHPR (-LHead,-LPitch,LRoll, O);
//	SetViewMat (O, Zoom);
    NSetViewMat (O, Zoom);
}


//***************************************************************************
SetViewPos (int X, int Y, int Z)
{
    VX = X;          /* Viewer coords, also */
    VY = Y;
    VZ = Z;
}

