/************************************************************************
*                                                                       *
*       Project: Stealth Fighter(TM)                                    *
*                                                                       *
*<t>    Flight Equations                                                *
*                                                                       *
*       Author: J.Synoski, S.Meier, A.Hollis, S.Spanburg, M.McDonald    *
*                                                                       *
*       Copyright (C) 1993 by MicroProse Software, All Rights Reserved. *
*                                                                       *
************************************************************************/

#include "library.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "f15defs.h"
#include "armt.h"
#include "carr.h"
#include "3dobjdef.h"

extern int far MakeRotationMatrix();
extern struct RastPort *Rp1, *Rp2, *Rp3D;
extern  UWORD   QTimer;
extern  UWORD   Rtime;
extern  UWORD   Px,Py;
extern  long    PxLng,PyLng;
extern  int     atarg,gtarg;
extern  int     NextWayPoint;
extern  SWORD   FM[3][3];
extern  UWORD   OurHead;
extern  SWORD   OurPitch, OurRoll;
extern  UWORD   Alt;
extern	UWORD	OurAlt;
extern  int     vHead,vPitch,vRoll, VM[3][3];
extern  char    Txt[];
extern  int     ejectX,ejectY,ejectZ;
extern  int     BackSeat;
extern  int     ImHit;
extern  int     Panning;
extern  int     OutsideWidth;
extern  int     OutsideLength;
extern  int     Knots;
extern	int	    SteadyHead;
extern	int	    SteadyPitch;
extern  UBYTE   FlirTbl[];
extern	int	    DESIGNATED;
extern  UWORD   MouseSprite;
extern  int     CloseBase[];
extern  int     LOCKSHOOT;
extern  UWORD   *PalPtr3D;
extern  int     whichourcarr;
extern int PlaneAnimVals[];

extern  struct  chase
{
	long x,y;
	int  z;
	int head,pitch,roll;
} chs[];

extern carriers far carrier;
extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);

extern struct FPARAM {
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

extern confData ConfigData;

long    Povx,Povy,ViewX,ViewY;
int     Povz,ViewZ;
int     PovPitch,PovMisl,RPov;
int     vScale = 2;
int	    PadHead=0;
int	    PadPitch=0;
int     PadLockee=-1;

//***************************************************************************
//
// Set up view parameters to Generate a 3d Display Frame
//
//***************************************************************************
GenDsp ()
{
    static  LastView = FRONT;
    static  LastSeat = 0;

    UBYTE   PalPkt[256*3 + 6];

    if (ejected) View = EJECTVIEW;

    SetupView();

    if (View==FRONT || View==DOWN) {
        memcpy (VM, FM, sizeof (FM));
    } else {
        MakeRotationMatrix (VM, vHead, vPitch, vRoll);
    }


    if ((View!=LastView) || (BackSeat!=LastSeat)) {
        ScreenOff();
        MouseSETUP(MouseSprite,0,0);
        MouseHIDE();

        if (View<FRONTAL) {
			if (LastView==DOWN) {
				LoadPicToPal(1,"F14ORG01.PIC",PalPkt);
				UpdatePalBuf(PalPkt);
			}
			LoadNewCockpit();
        } else {
            if (LastView<FRONTAL) {
                LoadPicToPal(1,"F14ORG01.PIC", PalPkt);
				UpdatePalBuf(PalPkt);
            }
			if (View==PADLOCK || View==PILOT) {
            	Rp3D->Length1 = 199;
            	Rp3D->Width1  = OutsideWidth - 1;
            	ClearPage(0, BLACK);
			} else {
            	Rp3D->Length1 = OutsideLength - 1;
            	Rp3D->Width1  = OutsideWidth - 1;
            	ClearPage(0, BLACK);
			}
        }
        LastView = View;
        LastSeat = BackSeat;
    }

    NDraw3D(-vHead,vPitch,vRoll, ViewX,ViewY, (long) ViewZ);

    CheckImHit();
	return 1;
}

//***************************************************************************
//*
//* Set up ViewX, ViewY, ViewZ, vHead, vPitch, vRoll
//*
//***************************************************************************
SetupView()
{
    SWORD   dx,dy,dz,vDist,n,vvScale,vSave;
    int     TM[9], RM[9], PM[9],phd,pptchmin;
    UWORD   dd;
	int 	whichcarr;

   long  TX,TY;


	/* Establish our view point */
    ViewX = PxLng;
    ViewY = PyLng;
    ViewZ = Alt+3;

    Povx = ViewX;
    Povy = 0x100000 - PyLng;
    Povz = Alt;

    vvScale = vScale = rng (vScale, 2,8);

	/* Various CAMERA Views */
    switch (View) {
	    case FRONTAL:
		case DOWN:
        case FRONT:
            vHead = OurHead;
            vPitch = OurPitch;
            vRoll = OurRoll;
		break;

	    case PILOT:
	    case PADLOCK:
   		    MakeMatHPR (OurHead,-OurPitch,OurRoll, TM);
			if (View==PADLOCK) {
    			if (PadLockee==-1) {
                	PadHead=0;
                	PadPitch=0;
                } else {
				    PM[0]=(ps[PadLockee].x-Px);
				    PM[1]=(ps[PadLockee].alt-(int) Alt)>>(HTOV);
				    PM[2]=(ps[PadLockee].y-Py);

				    Rotate(PM,TM);
				    PadHead=ARCTAN(PM[0],-PM[2]);
				    PadPitch=-ARCTAN(PM[1],Dist2D(PM[0],PM[2]));
				}
			}

			if (PadHead==0x8000)
				PadHead=0x7fff;
			if (abs(PadHead)>0x6c00)
				PadHead= (PadHead>0)? 0x6c00:-0x6c00;
			phd= abs(PadHead);

			pptchmin=-0x1400;
			if (phd<0x2e00)
				pptchmin=0xd00;
			else
				pptchmin=0x100;

			pptchmin=-pptchmin;
			if (PadPitch>pptchmin)
				PadPitch=pptchmin;

			MakematHPR(PadHead,PadPitch,0,PM);
			Mult3X3(TM,PM,RM);
			GetHPR(RM,&vHead,&vPitch,&vRoll );
			vPitch= -vPitch;
		break;

		case RIGHT:
			vHead  = OurHead + 0x4000;
			vPitch = -OurRoll;
			vRoll  = OurPitch;
			break;
		case LEFT:
			vHead  = OurHead + 0xC000;
			vPitch = OurRoll;
			vRoll  = -OurPitch;
			break;
		case REARRIGHT:
			vHead = OurHead + 0x7000;
			vPitch = -OurPitch;
			vRoll = -OurRoll;
			break;
		case REARLEFT:
			vHead = OurHead + 0x9000;
			vPitch = -OurPitch;
			vRoll = -OurRoll;
			break;

		case TOWERVIEW:
		case LSOVIEW:
		 	if(View == TOWERVIEW) {
//         		ViewX = ((carrier.x+1950L)/100L);
//         		ViewY = (0x100000-((carrier.z+2500L)/100L));
         		ViewX = ((carrier.x+carrier.twrx1-450L)/100L);
         		ViewY = (0x100000-((carrier.z+carrier.twrz1+400L)/100L));
		 		ViewZ = (96/4+4);
		 	}
		 	else {
				whichcarr = ConfigData.Carrier;
				if((whichcarr > 2) || (whichcarr < 0))
					whichcarr = 0;
				if(whichcarr == 1) {
	         		ViewX = ((carrier.x-1700L)/100L);
    	     		ViewY = (0x100000-((carrier.z+10200L)/100L));
				}
				else if(whichcarr == 2) {
	         		ViewX = ((carrier.x-1700L)/100L);
    	     		ViewY = (0x100000-((carrier.z+8800L)/100L));
				}
				else {
	         		ViewX = ((carrier.x-2500L)/100L);
    	     		ViewY = (0x100000-((carrier.z+9500L)/100L));
				}
         		ViewZ = (66/4+4);
		 	}
		 	if(planelanding < 0) {
	         	TX= PxLng-ViewX;
    	     	TY= PyLng-ViewY;
		 	}
		 	else {
	         	TX= ps[planelanding].xL-ViewX;
    	     	TY= (0x100000-ps[planelanding].yL)-ViewY;
		 	}
         	if ((labs(TX)>16384L)||(labs(TY)>16384L) )
            	{
            	TX>>=8;
            	TY>>=8;
            	}
		 	if(planelanding < 0) {
        	 	vPitch=ARCTAN(Alt-ViewZ,Dist2D((int)TX,(int)TY));
		 	}
		 	else {
        	 	vPitch=ARCTAN(ps[planelanding].alt-ViewZ,Dist2D((int)TX,(int)TY));
		 	}
         	vHead= ARCTAN((int)TX,(int)TY);
         	vRoll = 0;
		break;

        case WINGMAN:
            vHead = OurHead - 0x4000;
            vPitch = 0;
            vRoll = 0;
            ViewX = PxLng + sinX (OurHead+0x4000, 24<<vvScale);
            ViewY = PyLng + cosX (OurHead+0x4000, 24<<vvScale);
            break;

        case STEADY:
            vHead = OurHead+SteadyHead;
            vPitch = SteadyPitch;
            vRoll = 0;

            ViewX = PxLng -ASinB(ACosB(4<<vvScale,vPitch),vHead);
            ViewY = PyLng -ACosB(ACosB(4<<vvScale,vPitch),vHead);
            ViewZ = Alt + (1<<vvScale)- ASinB(4<<vvScale,vPitch);
        break;

        case POVVIEW:
        case MAGICCAM1:
        case MAGICCAM2:
            if (View!=POVVIEW) {
                RPov=0;
				if (PadLockee!=-1) RPov = 32;
            } else {
                RPov = PovMisl;

			}

            vSave = vvScale;

            if (!(RPov&64)) {
                if (!(RPov&32)) {
                    if (ss[RPov].dist) {
						// If the missile hits the target and it is
						// destroyed then goto that view
                        Povx = ss[RPov].x;
                        Povy = ss[RPov].y;
                        Povz = ss[RPov].alt;
                    } else {
                        ss[RPov].head = OurHead;
                        ss[RPov].pitch = OurPitch;
                    }
                    vvScale = 5;
                } else {
                    Povx = ps[PadLockee].xL;
                    Povy = ps[PadLockee].yL;
                    Povz = ps[PadLockee].alt;
                    vvScale = 3;
                }
            }
            vvScale = vSave;

            dx = (int) (Povx>>HTOV) - (int) Px;
            dy = (int) (Povy>>HTOV) - (int) Py;
            dd = Dist2D(dx,dy);
            vHead = angle(dx,-dy);
			vHead += SteadyHead;
            vPitch = - uangle((Povz-(int)Alt)>>HTOV, dd);
			vPitch += SteadyPitch;
            vRoll = 0;
            vDist = cosX(vPitch, 4<<vvScale);

            if (RPov&96) {
                if (View==MAGICCAM1) {
                    ViewX  = PxLng + sinX (vHead +0x8000, vDist);
                    ViewY  = PyLng + cosX (vHead +0x8000, vDist);
                    ViewZ  = Alt + sinX (vPitch, 4<<vvScale) + (1<<vvScale);
                    vPitch = -vPitch;
                } else {
                    ViewX  = Povx + sinX (vHead, vDist);
                    ViewY  = (((32768L<<HTOV)-Povy)) + cosX (vHead, vDist);
                    ViewZ  = Povz - sinX (vPitch, 4<<(vvScale)) + (1<<vvScale);
                    vHead += 0x8000;
                }
            } else {
				/* Missile View */
                vHead  = ss[RPov].head-0x2000 + SteadyHead  ;
				vPitch = ss[RPov].pitch+0x1000+ SteadyPitch ;
                vDist  = cosX (vPitch, 16<<vvScale);
                ViewX  = Povx - sinX (vHead, vDist);
                ViewY  = ((32768L<<HTOV)-Povy) - cosX (vHead, vDist);
                ViewZ  = Povz - sinX(vPitch,16<<vvScale);
            }
		break;

        case EJECTVIEW:
            vPitch = -12<<8;
            vRoll = 0;
            ViewX = (long) ejectX<<HTOV;
            ViewY = (long) (32768-ejectY)<<HTOV;
            ViewZ = ejectZ;
		break;
    }

    if (abs(vPitch) > 0x4000 || vPitch==0x8000) {
        vPitch = 0x8000 - vPitch;
        vHead = vHead + 0x8000;
        vRoll = 0x8000 - vRoll;
    }

    if (ViewZ<9) ViewZ = 9;

   if (ViewZ<(120/4))
      {
      if ( n=OnTheDeck((long)(ViewX*100L),(long)(((long)(ViewZ))*400L),(long)((0x100000L-ViewY)*100L)) )
         {
         if (ViewZ<(72/4))
            ViewZ=(72/4);
         if (n<0)
            ViewZ= (120/4);
         }
      }

	return 1;
}

//***************************************************************************
NewMissileView()
{
    int i;

    if (View==POVVIEW) {
        for (i=0; i<NSAM; i++) {
            PovMisl++;
            if (PovMisl>=NSAM) PovMisl=0;
            if (ss[PovMisl].dist) break;
        }
    }
    View=POVVIEW;
	return 1;
}

//***************************************************************************
NewTacView(int NewView)
{
    int i;

	NewLockView(NewView);
    View=NewView;
	return 1;
}

//***************************************************************************
NewLockView(int NewView)
{
    int i;

    if (PadLockee==-1) PadLockee=0;

    if (View!=NewView && DESIGNATED!=-1 && WithinVisualRange (DESIGNATED, NewView) ) {
        PadLockee=DESIGNATED;
    } else {
        for (i=0; i<NPLANES; i++) {
            PadLockee++;
            if (PadLockee>=NPLANES) PadLockee=0;
            if (ps[PadLockee].speed) {
                if (WithinVisualRange (PadLockee, NewView)) break;
            }
        }
        if (i==NPLANES) PadLockee=-1;
    }
    View=NewView;
	return 1;
}

#ifdef YEP
//***************************************************************************
NewLockView(int NewView)
{
    int i;
	struct PlaneData far *tempplane;

    if (PadLockee==-1) PadLockee=0;

    if (View!=NewView && DESIGNATED!=-1 && WithinVisualRange (DESIGNATED, NewView) ) {
        PadLockee=DESIGNATED;
    } else {
        for (i=0; i<MAXPLANE; i++) {
            PadLockee++;
			if (PadLockee == whichourcarr) PadLockee++;
            if (PadLockee>=MAXPLANE) PadLockee=0;
            if (ps[PadLockee].speed) {
                if (WithinVisualRange (PadLockee, NewView)) break;
            }
        }
        if (i==MAXPLANE) PadLockee=-1;
    }
    View=NewView;
	return 1;
}
#endif

//***************************************************************************
WithinVisualRange (int Plane, int View)
{
    return (View!=PADLOCK || Dist2D (ps[Plane].x-Px, ps[Plane].y-Py)<1024);
}

//***************************************************************************
extern	RPS *HDGRp;
extern	RPS *PLRp;
extern	RPS *RBRp;
extern	RPS *TDBRp;
extern	UWORD	GRPAGE0[];
extern	int	GXMID;
extern	int	GYMID;
extern	UWORD	GRPAGE;


//***************************************************************************
DoPadLockView()
{
	int	VM[9],TM[9];
	int	HudOffX,HudOffY;
	UWORD SEG;

	// draw padlock plane here

#ifdef YEP
	if (MapIn3dObjects(COOLSEG)) {
		MakeMatHPR(OurHead,-OurPitch,OurRoll,VM);
		Trans3X3(VM,TM);
		GRPAGE=GRPAGE0[Rp2->PageNr];
		Draw3DObject(CANOPYBOWPX &255, 0,0,0, TM,0);
	}
#endif

	MakeMatHPR(OurHead,-OurPitch,OurRoll,VM);
	Trans3X3(VM,TM);
	GRPAGE=GRPAGE0[Rp2->PageNr];
	NDraw3DObject(N3DZ_CANOPY, 0,0,0, TM, PlaneAnimVals);


	if (abs(PadHead)<0x2600) {
		HudOffX = GXMID- PadHead/38;
		HudOffY = GYMID-10 - PadPitch/51;
		FloatingHUD(HudOffX, HudOffY);
	}

	if (SEG=MapInEMSSprite(SPRTXPLSEG,2)) {
		if (abs(PadHead)>0x4200) {
			HudOffX= GXMID+(0x6c00-PadHead)/38;
			HudOffY= GYMID- (PadPitch)/51;
			if (PadHead<0)
				ScaleRotate(SEG,200,131,240,138,Rp2,HudOffX+55+132+41,HudOffY+31,0x100,0,BLACK, 1);
			else
				ScaleRotate(SEG,200,131,240,138,Rp2,HudOffX+41,HudOffY+31,0x100,0,BLACK,0);
		}
	}
	return 1;
}

//***************************************************************************
FloatingHUD(int HudOffX,int HudOffY)
{
	int	i;
	int	TM[9];
	int	oldpage;

//		if ( (HudOffX<(-196/2)) || (HudOffX>(160+196/2)) ) return(0);
//		if ( (HudOffY<(-92/2)) || (HudOffY>(100+92/2)) ) return(0);

	oldpage=Rp2->PageNr;
	Copy3X3(VM,TM);
    memcpy (VM, FM, sizeof (FM));
	Rp2->PageNr =
	HDGRp->PageNr =
	PLRp->PageNr =
	RBRp->PageNr =
	TDBRp->PageNr = 2;

	RectFill(Rp2, 62,0,258-62+1,92-1+1, BLACK);
	HUD();
	Copy3X3(TM,VM);
	Rp2->PageNr =
	HDGRp->PageNr =
	PLRp->PageNr =
	RBRp->PageNr =
	TDBRp->PageNr = oldpage;
	ScaleRotate(GRPAGE0[2],160,46,196,92,Rp2,HudOffX,HudOffY,0x100,0,BLACK,0);
}

//***************************************************************************
CheckImHit()
{
    if (ImHit) {
        ImHit = FALSE;
        if (!(View&VOFFSET)) {
            RectFill (Rp3D, 0,0, 320,Rp3D->Length1+1, ORANGEFLAME);
            Shake (60);
        }
    }
	return 1;
}


