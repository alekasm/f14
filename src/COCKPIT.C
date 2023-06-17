//***************************************************************************
// File:	Cockpit.c
//
// Author: Mike McDonald
//
//
// Fleet Defender: F-14 Tomcat
// MicroProse, Inc.
// 180 LakeFront Drive
// HuntValley, MD 21030
//***************************************************************************
#include "library.h"

#define _include_code
#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "armt.h"
#include "awg9.h"
#include "string.h"
#include "sounds.h"

extern  RPS     *Rp1, RpX1, *Rp2, *Rp3, *Rp3D, TDBRpX;

extern  int 	WingRot;
extern  int     THRUST;
extern  int     ThrustI;
extern  int     Knots;
extern  int     Rtime;
extern  int     Px, Py;
extern  long	PxLng;
extern  long    PyLng;
extern  int     atarg,gtarg;
extern  int     ShowMouse;
extern  char    Txt[];
extern  int     Mscale;
extern  int     Tscale;
extern  UWORD   Alt;
extern  UWORD   QTimer;
extern  int     JAMMER;
extern  int     JAMMER_ACTIVE;
extern  int     MLAUNCH;
extern  int     AIDETECT;
extern	UWORD	OurAlt;
extern  int     OurRoll;
extern  long	RadarAlt;
extern	int	    OurHead;
extern	int	    AirSpeed;
extern  int     LIGHTS;
extern  int     REALFLIGHT;
extern  long    FuelFlow;
extern  int		BingoFuel;
extern  int		OurPitch;
extern	long	NEWAOA;
extern  int 	mapmode;
extern  SWORD	VVI;
extern  int		LightsOn;
extern	int		BEAM_PIT;
extern  int     MsgOnScreen;
extern  int     HookDown;
extern  int		NextWayPoint;
extern  int		Gees;

extern   char  Coursestr[];
extern   char  Slopestr[];
extern   char  Rangestr[];
extern   char  Decentstr[];
extern   int   MYCLCD;
extern   int   MYCP;
extern   int   LOADDRAG;
extern   int   MYLOAD;
extern   char  *loadstrs[];
extern   int   OurLoadOut;
extern   int PadLockee;
extern   int DifficultyLevel;
extern int Speedy;
extern int SNDDETAIL;

char    *CockpitFiles[2][6] = {
            "cpft2.pic","f14lft6.pic","f14rt8.pic","cpfd2.pic","BG-c2.pic","BG-c1.pic",
			"f14bac6.pic","f14bl6.pic","f14brt8.pic","f14bac5b.pic","RB-2L.PIC","RB-2R.PIC"
        };

#ifdef YEP
char    *CockpitFiles[2][6] = {
            "f14cpit7.pic","f14lft6.pic","f14rt8.pic","f14pit4b.pic","BG-c2.pic","BG-c1.pic",
			"f14bac6.pic","f14bl6.pic","f14brt8.pic","f14bac5b.pic","RB-2L.PIC","RB-2R.PIC"
        };
#endif

int     Lengths[2][6] = {SIZE3D,     110,110,50,162, 162,
                         139, 		 110,110,10,140, 140};

typedef struct {
	int x;
	int y;
	int l;
	int w;
	int fromcolor;
	int tocolor;
} clights;
clights far CockpitLights[40] =
{
		80,	63,	22,	6,	31,	17,	/* L STALL */
		88,	73,	12,	4,	18,	17,	/* WHEELS(GEAR) */
		89,	78,	11,	4,	18,	17,	/* BRAKES */
		90, 83,	11,	4,  18,	17,	/* HOOK */
		219,63,	21,	6,	31,	17,	/* R STALL */
		220,73,	12,	4,	18,	17,	/* SAM */
		220,78,	12,	4,	18,	17,	/* AAA */
		219,83,	12,	4,	18,	17,	/* AI */
		119,112,11,	3,	18, 17,	/* SEAM LOCK */
		119,116,11,	3,	18,	17,	/* COLLISION */
		119,120,11,	3,	18,	16,	/* HOT TRIG */
		91,	139,19,	5,	29,	16,	/* LEFT FIRE */
		99, 150,12,	2,	18,	17,	/* ADJ A/C */
		99, 153,12, 2,	18,	17,	/* LANDING CHK */
		99,	156,12,	2,	18,	17,	/* ACL READY */
		99,	159,12,	2,	18,	17,	/* A/P CPLR */
		99,	162,12,	2,	18, 17,	/* CMD CONTROL */
		100,165,12,	2,	18,	17,	/* 10 SECONDS */
		100,168,12,	2,	18,	17,	/* TILT */
		210,139,19,	5,	29,	16,	/* RIGHT FIRE */
		209,150,12,	5,	18,	17,	/* WAVE OFF */
		209,153,12,	5,	18,	17,	/* WING SWEEP */
		209,156,12,	5,	18,	17,	/* REDUCE SPEED */
		209,159,12,	5,	18,	17,	/* ALT LOW */
		142,106,10, 3,	18,	17,	/* GUN RATE - HIGH */
		142,111,10,	3,	18,	17,	/* GUN RATE - LOW */
		155,106,10,	3,	18,	17,	/* SW COOL - ON */
		155,111,10,	3,	18,	17,	/* SW COOL - OFF */
		168,106,10,	3,	18,	17,	/* MSL PREP - ON */
		168,111,10,	3,	18,	17,	/* MSL PREP - OFF */
		192,115,10,	3,	18,	17,	/* MSL MODE - NORM */
		192,120,10,	3,	18,	17,	/* MSL MODE - BRSIT */
		96, 127, 6, 2,  27,	 1, /* STORES 1 */
		107,127, 6,	2,	27, 	 1,	/* STORES 2 */
		118,127, 6, 2,	27, 	 1,	/* STORES 3 */
		129,127, 6, 2,	27, 	 1,	/* STORES 4 */
		185,127, 6, 2,	27,	 1,	/* STORES 5 */
		196,127, 6, 2,	27, 	 1,	/* STORES 6 */
		207,127, 6, 2,	27,	 1,	/* STORES 7 */
		218,127, 6,	2,	27,	 1,	/* STORES 8 */
};




UWORD   far VdiMask[250];
UWORD   far AdiMask[170];
UWORD	far LookDownLMask[130];
UWORD	far LookDownRMask[105];
UWORD	far	RawMask[185];
UWORD	far TopCockpitMask[2400];
UWORD	far HsdMask[300];
UWORD	far DDDMask[888];
UWORD	far TIDMask[480];
UWORD	far ElevMask[600];

extern SWORD PitchAngle;
extern UWORD DialSprite[];

int     MASTER_CAUTION=0;
int     DAMAGELITE[16];
int     RadarRepeat=0;
int     BackSeat    = 0;
int     Panning     = 0;
// int     LookingDown = 0;
int     DeClutter   = 0;
int		LLowFuelWarn=0;
int		RLowFuelWarn=0;
int		BingoFuelWarn=0;

UWORD   OutsideWidth  = 320;     /* size of outside view window */
UWORD   OutsideLength = 170;
// UWORD   OutsideLength = 199;

RPS     RpCX1  =  {1, 119,138,	82,57, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT1  = &RpCX1;

RPS     RpCX2  =  {1, 226,136, 	30,44, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT2  = &RpCX2;

RPS     RpCX3  =  {1, 118,100, 	83,68, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT3  = &RpCX3;

RPS     RpCX4  =  {1, 260,150,	42,41, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT4  = &RpCX4;

RPS     RpCX5  =  {1, 119,106,	83,59, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT5  = &RpCX5;

RPS     RpCX6  =  {1, 88,33,146,117, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT6  = &RpCX6;

RPS     RpCX7  =  {1, RGTBAKCOLRCRTX-BAKCOLRCRTPAN,RGTBAKCOLRCRTY, CRTXSIZ-1, CRTYSIZ-1, JAM3, GREEN, BLACK, 1};
RPS     *RpCRT7  = &RpCX7;

RPS     *RpCRT;

RPS     *CrtRps[] =  {&RpCX1, &RpCX2, &RpCX3, &RpCX4, &RpCX5, &RpCX6, &RpCX7, &TDBRpX};
int     CrtDisplays [8];
int     CrtUpdate   [8];

int     CrtConfig[8] = {VDI,ADI,HSD,TEWS,DDD,TID,TSD,HUDCRT };

int     CurCRT=2;

int MasterArmSW=0;
int JetisonSW=0;
int Lightemup=0;
int lighton=0;
int VdiMode=1;
int msiz=0;

static  int OLDMM;
static  int OldFuel;
static  int OldThrust;
static  int OldLights[40];
static  int OldEMIS;
static  int OldMC;

int EgtL=15;
int EgtR=15;

// scaley(y) { return(y*14/16); }

extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

extern int MeatBall;
extern int NumBars;
extern int AZSweep;
extern int     LastSpike;
extern int     LastMud;
extern confData ConfigData;
extern int DataLink;
extern int ClrTarg;
extern int DesignateMode;
extern int HighlightTarg;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
int RadarRange=5;

//***************************************************************************
LoadNewCockpit()
{
    UBYTE   PalPkt[256*3 + 6];

	LoadPicToPal(1, CockpitFiles[BackSeat][View&VNUM], PalPkt);
	UpdatePalBuf(PalPkt);

   	ScreenOff();
    ShowPage(1);

    ResetCockpitLights();


    Rp3D->Width1  = 319;
    Rp3D->Length1 = Lengths[BackSeat][View&VNUM];

	if (View==PADLOCK || View==PILOT) {
		OutsideLength = 199;
	    Rp3D->Length1 = 199;
	} else {
		OutsideLength = 170;
	}

    switch (View) {
		case REARLEFT:
			msiz = BuildMask(TopCockpitMask,0,0,319,163,1);
			break;

		case REARRIGHT:
			msiz = BuildMask(TopCockpitMask,0,0,319,163,1);
			break;

		case PILOT:
        case PADLOCK:
			break;
		case RIGHT:
		case LEFT:
			msiz = BuildMask(TopCockpitMask,0,0,319,110,1);
			break;
		case DOWN:
			if (!BackSeat) {
				msiz = BuildMask(HsdMask,117,100,86,70,1);
			} else {
  				msiz = BuildMask(TIDMask,91,36,140,114,1);
			}
            break;

        default:
			BuildCockpitSprites();
            break;
    }
	if ((View!=REARLEFT) && (View!=REARRIGHT))
    	ChangeHUDColor(0);
}


//***************************************************************************
BuildCockpitSprites()
{
	int x;
	char ch;
	char temp[5];

	if (!BackSeat) {
		if (View==FRONT) {
			x = BuildMask(TopCockpitMask,0,0,319,150,1);
			x = BuildMask(VdiMask, 119,138,82,57, 1);
			x = BuildMask(AdiMask, 226,136,30,34,1);
			x = BuildMask(LookDownLMask, 0,110,57,44,1);
			x = BuildMask(LookDownRMask,261,110,59,39,1);
			x = BuildMask(RawMask, 260,150,42,41,1);
			LookDownRMask[MASKY]=0;
			LookDownLMask[MASKY]=0;
			UpdateLights();
		}
	} else {
		if (View==FRONT) {
			x = BuildMask(TopCockpitMask,0,0,319,150,1);
			x = BuildMask(DDDMask,116,106,88,60,1);
			x = BuildMask(ElevMask,86,106,16,58,1);
			DDDSwitchRefresh();
		}
	}
}

//***************************************************************************
LookDown()
{
	if (!BackSeat) {
		Rp3D->Length1 = 50;
   		RpCRT1->YBgn = 27;
   		RpCRT2->YBgn = 25;
		RpCRT4->YBgn = 39;

		VdiMask[MASKY] = 27;
		AdiMask[MASKY] = 25;
		RawMask[MASKY] = 39;
	} else {
		Rp3D->Length1 = 50;
	}
}

//***************************************************************************
LookUp ()
{
	if (!BackSeat) {
		Rp3D->Length1  = 150;
		RpCRT1->YBgn   = 138;
		RpCRT2->YBgn   = 136;
		RpCRT4->YBgn   = 150;

		VdiMask[MASKY] = 138;
		AdiMask[MASKY] = 136;
		RawMask[MASKY] = 150;
	} else {
		Rp3D->Length1  = 139;
	}
}

//***************************************************************************
Flip ()
{
	char str[30];
	char tmp[80];
	char s[10];
	static int lastview=-1;

	struct PlaneData far *tempplane;

//    FireColors ();
    MouseHIDE ();

	switch(View) {
		case PILOT:
		case PADLOCK:
			RectCopy(Rp3D, 0,0, OutsideWidth, 199, Rp1, (320-OutsideWidth) >>1,0);
//			RectCopy(Rp3D, 0,0, OutsideWidth, OutsideLength, Rp1, (320-OutsideWidth) >>1,(200-OutsideLength) >>1);
		break;
		case MAGICCAM2:
			if ((DifficultyLevel==0) && (PadLockee > -1)) {
				tempplane = GetPlanePtr(ps[PadLockee].type);
				_fstrcpy ((char *)tmp, (char far *)tempplane->name);
				SetFont(Rp3D,F_3x5B);
				RpPrint(Rp3D,0,10,tmp);
			}
		case FRONTAL:
			RectCopy(Rp3D, 0,0, OutsideWidth, OutsideLength, Rp1, (320-OutsideWidth) >>1,0);
//			RectCopy(Rp3D, 0,0, OutsideWidth, OutsideLength, Rp1, (320-OutsideWidth) >>1,(200-OutsideLength) >>1);
		break;
		case STEADY:
		case WINGMAN:
		case MAGICCAM1:
		case POVVIEW:
		case EJECTVIEW:
		case TOWERVIEW:
		case LSOVIEW:
			RectCopy(Rp3D, 0,0, OutsideWidth, OutsideLength, Rp1, (320-OutsideWidth) >>1,0);
//			RectCopy(Rp3D, 0,0, OutsideWidth, OutsideLength, Rp1, (320-OutsideWidth) >>1,(200-OutsideLength) >>1);
		break;
		case REARRIGHT:
			MaskCopy(TopCockpitMask);
		break;
		case REARLEFT:
			MaskCopy(TopCockpitMask);
		break;
		case FRONT:
			if (!BackSeat) {
				if (Status&TRAINING) RpPrint (Rp3D, 0,2, "TRNG" );
				if (Speedy>1) {
					SetFont (Rp3D, F_3x4);
					RpPrint (Rp3D, 5,8, "x");
					RpPrint (Rp3D, 0,8, itoa(Speedy,s,10));
				}
				MaskCopy(TopCockpitMask);
				if (!MsgOnScreen)
					VdiMask[3] = 57;
				else
					VdiMask[3] = 52;
				MaskCopy(VdiMask);
				MaskCopy(AdiMask);
				MaskCopy(RawMask);
			} else {
				MaskCopy(TopCockpitMask);
				MaskCopy(DDDMask);
				MaskCopy(ElevMask);
			}
		break;
		case DOWN:
			if (!BackSeat) {
				MaskCopy(LookDownLMask);
				MaskCopy(LookDownRMask);
				MaskCopy(AdiMask);
				MaskCopy(RawMask);
				MaskCopy(HsdMask);
				MaskCopy(VdiMask);
			} else {
				MaskCopy(TIDMask);
			}
		break;

		case RIGHT:
		case LEFT:
			MaskCopy(TopCockpitMask);
		break;
    }

    if (ShowMouse) MouseSHOW ();
//	if (lastview != View) {
    	ScreenOn();
//		lastview = View;
//	}
	if ((SNDDETAIL!=1) || (SNDDETAIL!=3))
    	SndSounds(View>FRONTAL ? N_JetFromOutside:N_JetFromInside);
}


//***************************************************************************
ConfigCrts ()
{
    int i;

    for (i=0; i<8; i++) {
        CrtDisplays[i] = CrtConfig[i];
    }
}


//***************************************************************************
DoCockpitDisplays ()
{
    static int LastCur = 2;
	char s[10];
	int alt,x,y;
	int althund;
	int x2,y2;
	static int FirstArm=0;
	int i;

	if (!BackSeat) {
		if (View==DOWN) {
			DoCRT(0);
			DoCRT(1);
 			DoCRT(2);
			DoCRT(3);
			DrawVVGauge();
			DrawAirSpeedGauge();
			DrawAltGauge();
			DrawRadarAlt();
			DrawCompass();
			DrawStores();
			ShowFuel();
			DrawEngineStat();
   		}

	    if (View==FRONT) {
			HUD();
			DrawVVGauge();
		  	DrawAirSpeedGauge();
			DrawAltGauge();
			DrawRadarAlt();
			DrawAOADisplay();
			DrawSweepDisplay();
			DrawStores();
			DrawCompass();
			DrawIndexer();
			DoCRT(0);
			DoCRT(1);
			DoCRT(3);
			UpdateCockpitLites();
			DisplayBall();
			DisplayMaster();
			GDisplay();
		}
		if ((View==REARLEFT || View==REARRIGHT)) {
			DisplayRioHead();
		}
		if (View==LEFT) {
			DisplayLeftStuff();
		}
		if (View==RIGHT) {
			DisplayRightStuff();
		}
	} else {
		if (View==FRONT) {
			DoCRT(4);
			DDDSwitches();
			DrawRadarElev();
			DrawRadarStatus();
			DrawRadarRange();
		}
		if (View==DOWN) {
			DoCRT(5);
			DrawRadarStatus();
			TIDShowRange();
			DrawWayPoint();
//			DisplayBackLights();
			DisplayTIDSwitches();
		}
		if (View==LEFT) {
			DrawRadarDials();
		}

	}
	if (View==PADLOCK || View==PILOT) {
		DoPadLockView();
	}
}

//***************************************************************************
DisplayMaster()
{
	static int prevcaut=-1;

	if (MASTER_CAUTION != prevcaut) {
		if (MASTER_CAUTION) {
			RectFill(Rp1,152,119,16,2,16);
		} else {
			RectFill(Rp1,152,119,16,2,18);
		}
	}
	prevcaut = MASTER_CAUTION;
}

//***************************************************************************
DrawRadarStatus()
{
	char s[25];
	int l;
	int offset=0;

	SetFont(Rp2,F_3x5B);
	Rp2->APen=34;

	switch(AWG9_MODE) {
		// PD Search

		case AWG9_SRCH_WIDE:
			strcpy(s,"PDSRCH - WIDE");
		break;
		case AWG9_SRCH_MEDIUM:
			strcpy(s,"PDSRCH - MED");
		break;
		case AWG9_SRCH_NARROW:
			strcpy(s,"PDSRCH - NRW");
		break;
		case AWG9_SRCH_AUTHENTIC:
			strcpy(s,"PDSRCH");
		break;


		// RWS
		case AWG9_RWS_WIDE:
			strcpy(s,"RWS - WIDE");
		break;
		case AWG9_RWS_MEDIUM:
			strcpy(s,"RWS - MED");
		break;
		case AWG9_RWS_NARROW:
			strcpy(s,"RWS - NRW");
		break;
		case AWG9_RWS_AUTHENTIC:
			strcpy(s,"RWS");
		break;

		// TWSA
		case AWG9_TWSA_MEDIUM:
			strcpy(s,"TWSA - MED");
		break;
		case AWG9_TWSA_NARROW:
			strcpy(s,"TWSA - NRW");
		break;
		case AWG9_TWSA_AUTHENTIC:
			strcpy(s,"TWSA");
		break;

		// TWSM
		case AWG9_TWSM_MEDIUM:
			strcpy(s,"TWSM - MED");
		break;
		case AWG9_TWSM_NARROW:
			strcpy(s,"TWSM - NRW");
		break;
		case AWG9_TWSM_AUTHENTIC:
			strcpy(s,"TWSM");
		break;

		// Easy Mode
		case AWG9_STANDARD_SRCH:
			strcpy(s,"SEARCH");
		break;
		case AWG9_STANDARD_TRACK:
			strcpy(s,"TRACK");
		break;

		// PD Single Target Track
		case AWG9_PDSTT:
			strcpy(s,"PDSTT");
		break;

		// Special Modes
		case AWG9_BORESIGHT:
			strcpy(s,"BRST");
		break;
		case AWG9_VSL:
			strcpy(s,"VSL");
		break;
		default:
			strcpy(s,"BAD MODE");
		break;
	}

	l = pstrlen(Rp2,s);

	if (View==FRONT)
		offset=171;
	else
		offset=0;

	RectFill(Rp2,131,(5+offset),57,6,37);
	RpPrint(Rp2,(159-(l/2)),(5+offset),s);
	RectCopy(Rp2,131,(5+offset),57,6,Rp1,131,(5+offset));
}

//***************************************************************************
int GDisplay()
{
	char txt[10], s[10];
	int	N;

	SetFont(Rp2,F_3x5B);
	Rp2->APen=83;

	RectFill(Rp2,267,140,20,5,63);
	N=abs(Gees+1);
	txt[0]=0;
	if (Gees<-1) strcpy(txt,"-");
	strcat(txt, itoa(N/16,s,10));
	strcat (txt, ".");
	strcat(txt, itoa((10*(N&15))/16,s,10));
	RpPrint(Rp2,269,140,txt);
	RectCopy(Rp2,267,140,20,5,Rp1,267,140);
}



//***************************************************************************
int DisplayTIDSwitches()
{
	if (DesignateMode) {
		ChangeColor(Rp1,117,172,8,5,5,17);
	} else  {
		ChangeColor(Rp1,117,172,8,5,17,5);
	}

	if (ClrTarg) {
		ChangeColor(Rp1,133,172,8,5,5,17);
		ClrTarg--;
		if (ClrTarg<0) ClrTarg=0;
	} else {
		ChangeColor(Rp1,133,172,8,5,17,5);
	}

	if (HighlightTarg) {
		ChangeColor(Rp1,149,172,8,5,5,17);
	} else {
		ChangeColor(Rp1,149,172,8,5,17,5);
	}

	if (DataLink) {
		ChangeColor(Rp1,117,184,8,5,5,17);
	} else {
		ChangeColor(Rp1,117,184,8,5,17,5);
	}

}

//***************************************************************************
DisplayLeftStuff()
{
	int i;
	int mseg;
	static int prevstat=-1;
	int stat;
	int prevthrust=-1;

	MouseHIDE();

	stat = !(Status & WHEELSUP);

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
		ScaleRotate(mseg,(1+104),(119+26),208,51,Rp2,(69+104),(126+26),0x0100,0,0,0);
	}

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		if (stat) {
//			if (stat != prevstat) {
//				RectCopy(Rp2,242,127,39,38,Rp1,242,127);
//			}

			ScaleRotate(mseg,143,7,26,12,Rp2,(237+13),(149+6),0x0100,0,0,0);

			ScaleRotate(mseg,(205+4),(106+3),8,6,Rp1,(280+4),(118+3),0x0100,0,0,0);
			ScaleRotate(mseg,(205+4),(106+3),8,6,Rp1,(294+4),(118+3),0x0100,0,0,0);
			ScaleRotate(mseg,(205+4),(106+3),8,6,Rp1,(298+4),(128+3),0x0100,0,0,0);

		} else {
//			if (stat != prevstat) {
//				RectCopy(Rp2,242,127,39,38,Rp1,242,127);
//			}

			ScaleRotate(mseg,176,18,14,34,Rp2,(262+7),(128+17),0x0100,0,0,0);

			ScaleRotate(mseg,(206+4),(94+3),8,6,Rp1,(280+4),(118+3),0x0100,0,0,0);
			ScaleRotate(mseg,(206+4),(94+3),8,6,Rp1,(294+4),(118+3),0x0100,0,0,0);
			ScaleRotate(mseg,(206+4),(94+3),8,6,Rp1,(298+4),(128+3),0x0100,0,0,0);
		}

		if (((Status & BRAKESON) && !(REALFLIGHT && (Damaged&D_HYDRAULICS))))
			ScaleRotate(mseg,(205+4),(106+3),8,6,Rp1,(284+4),(128+3),0x0100,0,0,0);
		else
			ScaleRotate(mseg,(206+4),(94+3),8,6,Rp1,(284+4),(128+3),0x0100,0,0,0);

		if (prevthrust != THRUST) {
//			RectCopy(Rp2,78,132,170,45,Rp1,78,132);

   			if ((THRUST <= 25))
				ScaleRotate(mseg,(130+14),(52+19),29,38,Rp2,(72+14),(136+19),0x0100,0,0,0);
			if (((THRUST > 25) && (THRUST < 50)) || (THRUST == 50))
				ScaleRotate(mseg,(166+15),(52+18),31,36,Rp2,(109+15),(133+18),0x0100,0,0,0);
			if (((THRUST > 50) && (THRUST < 75)) || (THRUST == 75))
				ScaleRotate(mseg,(206+14),(52+17),29,35,Rp2,(146+14),(130+17),0x0100,0,0,0);
			if ((THRUST > 75) && (THRUST < 100))
				ScaleRotate(mseg,(242+16),(53+16),33,33,Rp2,(181+16),(134+16),0x0100,0,0,0);
	   		if (THRUST >= 100)
				ScaleRotate(mseg,(282+17),(53+16),34,33,Rp2,(220+17),(139+16),0x0100,0,0,0);
		}
		RectCopy(Rp2,69,126,208,51,Rp1,69,126);
	}

	prevstat = stat;
	prevthrust = THRUST;

	if (ShowMouse) MouseShow();
}


//***************************************************************************
DisplayRightStuff()
{
	int i;
	int mseg;

	MouseHIDE();

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		if (HookDown)
			ScaleRotate(mseg,294,116,40,46,Rp1,(15+20),(106+23),0x0100,0,0,0);
		else
			ScaleRotate(mseg,250,116,40,46,Rp1,(15+20),(106+23),0x0100,0,0,0);
	}

	// Display Damaged Stuff

	if (Damaged&D_GEAR) {
		ChangeColor(Rp1,84,172,28,72,2,16);
		ChangeColor(Rp1,84,172,28,72,4,17);
	}

	if (Damaged&D_HOOK) {
		ChangeColor(Rp1,99,172,25,20,3,62);
		ChangeColor(Rp1,99,172,25,20,5,63);
	}

	if (BingoFuelWarn) {
		ChangeColor(Rp1,113,168,26,24,2,16);
		ChangeColor(Rp1,113,168,26,24,4,17);
	}

	if (Damaged&D_HYDRAULICS) {
		ChangeColor(Rp1,143,157,23,35,2,16);
		ChangeColor(Rp1,143,157,23,35,4,17);
	}

	if (Damaged&D_LEFTENGINE) {
		ChangeColor(Rp1,158,157,18,35,3,62);
		ChangeColor(Rp1,158,157,18,35,5,63);
	}

	if (LLowFuelWarn) {
		ChangeColor(Rp1,173,156,13,36,2,16);
		ChangeColor(Rp1,173,156,13,36,4,17);
	}

	if (RLowFuelWarn) {
		ChangeColor(Rp1,188,156,13,36,3,62);
		ChangeColor(Rp1,188,156,13,36,5,63);
	}


	if (Damaged&D_RIGHTENGINE) {
		ChangeColor(Rp1,199,157,17,35,2,16);
		ChangeColor(Rp1,199,157,17,35,4,17);
	}

//	if (Damaged&D_WINGSWEEP) {
//		ChangeColor(Rp1,211,166,35,26,3,62);
//		ChangeColor(Rp1,211,166,35,26,5,63);
//	}

	if (Damaged&D_AWG9) {
		ChangeColor(Rp1,247,166,29,26,3,62);
		ChangeColor(Rp1,247,166,29,26,5,63);
	}

	if (Damaged&D_FIRECONTROL) {
		ChangeColor(Rp1,257,166,34,26,3,62);
		ChangeColor(Rp1,257,166,34,26,5,63);
	}

	if (Damaged&D_NAV) {
		ChangeColor(Rp1,277,174,29,18,3,62);
		ChangeColor(Rp1,277,174,29,18,5,63);
	}

	if (ShowMouse) MouseShow();
}

//***************************************************************************
int DrawRadarElev()
{
	int beampit;
	RectFill(Rp2,86,106,16,58,33);

	beampit=BEAM_PIT;
	if (beampit>9100) beampit=9100;
	if (beampit<-8736) beampit=-8736;

	if (LightsOn)
		DrawLine(Rp2,88,133-((beampit/182)/2),100,133-((beampit/182)/2),54);
	else
		DrawLine(Rp2,88,133-((beampit/182)/2),100,133-((beampit/182)/2),16);
}


//***************************************************************************
int DrawRadarAlt()
{
	int newalt;
	int soffset;
	int x2,y2;
	int mseg;
	unsigned t,b;

	if (View==FRONT)
		soffset=0;
	else
		soffset=112;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		TransRectCopy(mseg,254,2,35,29,Rp2,9,(162-soffset),0,0);
	}
   newalt= (RadarAlt>3000)? 3000:RadarAlt;
   if (newalt<500)
      {
      t=MDiv(newalt,8192,500);
      }
   else
      {
      if (newalt<1000)
         {
         t= MDiv((newalt-500),4096,500)+8192;
         }
      else
         {
         t= MDiv((newalt-1000),4096,4000)+8192+4096;
         }
      }

   t=(t<<2);
	Rotate2D(0,-10,t,&x2,&y2);

	DrawLine(Rp2,26,(176-soffset),26-x2,(176+scaley(y2)-soffset),(LightsOn?54:2));
	RectCopy(Rp2,10,(163-soffset),32,26,Rp1,10,(163-soffset));
}

//***************************************************************************
int DrawAirSpeedGauge()
{
	char s[10];
	int soffset;

	if (View==FRONT)
		soffset=0;
	else
		soffset=110;

	SetFont(Rp2,F_3x5B);
	Rp2->APen=(LightsOn?54:2);
	RectFill(Rp2,70,144-soffset,11,5,(LightsOn?22:0));
	itoa((TAStoIAS(Knots)),s,10);
	RpPrint(Rp2,70,144-soffset,s);
	RectCopy(Rp2,70,144-soffset,11,5,Rp1,70,144-soffset);
}

//***************************************************************************
int DrawIndexer()
{
	int tempknot;
	int pos;
	int ballcolor;
#ifdef YEP
	if (!(Status & WHEELSUP)) {
		tempknot = TAStoIAS(Knots);

		if (tempknot<130)
			pos=0;
		if (tempknots>130) && (tempknots<138))
			pos=1;
		if (tempknots>138)
			pos=3;
	}
#endif
	switch(MeatBall) {
		case 0:
			ballcolor=22;
		break;
		case 1:
			ballcolor=22;
		break;
		case 2:
			ballcolor=19;
		break;
		case 3:
			ballcolor=17;
		break;
		case 4:
			ballcolor=17;
		break;
	}
	RectFill(Rp1,80,78,3,2,ballcolor);
}


//***************************************************************************
int DrawWayPoint()
{
	int mseg;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		ScaleRotate(mseg,(85+17),(89+13),34,26,Rp2,(215+17),(39+13),0x0100,0,0,0);
		switch(NextWayPoint) {
			case 0:
				ScaleRotate(mseg,(61+8),(90+8),16,16,Rp2,(225+8),(48+8),0x0100,0,0,0);
			break;
			case 1:
				ScaleRotate(mseg,(10+8),(90+8),16,16,Rp2,(223+8),(48+8),0x0100,0,0,0);
			break;
			case 2:
				ScaleRotate(mseg,(30+6),(90+8),12,16,Rp2,(225+6),(48+8),0x0100,0,0,0);
			break;
			case 3:
				ScaleRotate(mseg,(46+5),(90+8),10,16,Rp2,(227+5),(48+8),0x0100,0,0,0);
			break;
		}
		RectCopy(Rp2,215,39,34,26,Rp1,215,39);
	}
}
//***************************************************************************
int DrawRadarDials()
{
	static int NumBars=0;
	static int SweepSize=0;
	int mseg;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		ScaleRotate(mseg,(38+13),(116+8),26,16,Rp1,(94+13),(146+8),0x0100,0,0,0);
		ScaleRotate(mseg,(5+14),(115+8),28,16,Rp1,(80+14),(169+8),0x0100,0,0,0);


	switch(SELECTED_BAR_NUM) {
		case BAR_1:
			ScaleRotate(mseg,(5+11),(70+6),26,12,Rp1,(83+11),(170+6),0x0100,0,0,0);
			NumBars=0;
		break;
		case BAR_2:
			ScaleRotate(mseg,(33+11),(70+6),26,12,Rp1,(83+11),(170+6),0x0100,0,0,0);
			NumBars=1;
		break;
		case BAR_4:
			ScaleRotate(mseg,(61+11),(71+6),26,12,Rp1,(83+11),(170+6),0x0100,0,0,0);
			NumBars=2;
		break;
		case BAR_8:
			ScaleRotate(mseg,(89+11),(72+6),26,12,Rp1,(83+11),(170+6),0x0100,0,0,0);
			NumBars=3;
		break;

	}

//	RectCopy(Rp2,98,162,26,12,Rp1,98,162);

	switch(SELECTED_AZ) {
		case DEG10:
			ScaleRotate(mseg,(5+11),(70+6),26,12,Rp1,(96+11),(146+7),0x0100,0,0,0);
			SweepSize=0;
		break;
		case DEG20:
			ScaleRotate(mseg,(33+11),(70+6),26,12,Rp1,(96+11),(146+7),0x0100,0,0,0);
			SweepSize=1;
		break;
		case DEG40:
			ScaleRotate(mseg,(61+11),(71+6),26,12,Rp1,(96+11),(147+6),0x0100,0,0,0);
			SweepSize=2;
		break;
		case DEG65:
			ScaleRotate(mseg,(89+11),(72+6),26,12,Rp1,(96+11),(148+5),0x0100,0,0,0);
			SweepSize=3;
		break;
	}
	}
}

//***************************************************************************
TIDShowRange()
{
	static RANGES[] = { 200,100,50,25,10 };
	char   s[10];
	int    l;
	int mseg;

	RectFill(Rp2,146,21,29,9,BLACK);
	l = pstrlen(Rp2,itoa(RANGES[Tscale],s,10));
	RpPrint(Rp2,160-(l/2),23,s);
	RectCopy(Rp2,146,21,29,9,Rp1,146,21);

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		ScaleRotate(mseg,(2+23),(160+15),46,31,Rp2,(164+23),(164+15),0x0100,0,0,0);
		switch(Tscale) {
			case 4:
				ScaleRotate(mseg,(54+9),(138+7),18,13,Rp2,(174+9),(177+7),0x0100,0,0,0);
			break;
			case 3:
				ScaleRotate(mseg,(15+8),(136+9),16,18,Rp2,(177+8),(173+9),0x0100,0,0,0);
			break;
			case 2:
				ScaleRotate(mseg,(2+5),(136+9),9,17,Rp2,(182+5),(173+9),0x0100,0,0,0);
			break;
			case 1:
				ScaleRotate(mseg,(34+8),(136+9),16,18,Rp2,(178+8),(173+9),0x0100,0,0,0);
			break;
			case 0:
				ScaleRotate(mseg,(77+10),(138+7),20,13,Rp2,(177+10),(176+7),0x0100,0,0,0);
			break;
		}
		RectCopy(Rp2,164,164,46,31,Rp1,164,164);
	}
}


//***************************************************************************
int DrawRadarRange()
{
	char s[10];
	int temprange=0;
	int l=0;

	RectFill(Rp2,150,92,23,9,BLACK);
	switch(RadarRange) {
		case 0:
			temprange=5;
		break;
		case 1:
			temprange=10;
		break;
		case 2:
			temprange=20;
		break;
		case 3:
			temprange=50;
		break;
		case 4:
			temprange=100;
		break;
		case 5:
			temprange=200;
		break;
	}
	itoa(temprange,s,10);
	l = pstrlen(Rp2,s);
	RpPrint(Rp2,(162-(l/2)),94,s);
	RectCopy(Rp2,150,92,23,9,Rp1,150,92);
}

//***************************************************************************
int DDDSwitchRefresh()
{
	MouseHIDE();

	if (AWG9_SNIFF) {
		ChangeColor(Rp1,229,112,14,5,16,34);
		DrawLine(Rp1,229,112,229,117,27);
		DrawLine(Rp1,229,111,243,111,26);
		DrawLine(Rp1,243,112,243,117,27);
	}
	if (!(AWG9_SNIFF)) {
		ChangeColor(Rp1,229,112,14,5,34,16);
		DrawLine(Rp1,229,112,229,117,29);
		DrawLine(Rp1,229,111,243,111,29);
		DrawLine(Rp1,243,112,243,117,29);
	}

	switch(RadarRange) {
		case 0:
			ChangeColor(Rp1,113,77,11,10,34,16);
			RpPrint(Rp2,150,92,"5");
		break;
		case 1:
			ChangeColor(Rp1,130,77,11,10,34,16);
			RpPrint(Rp1,150,92,"10");
		break;
		case 2:
			ChangeColor(Rp1,147,77,11,10,34,16);
			RpPrint(Rp1,150,92,"20");
		break;
		case 3:
			ChangeColor(Rp1,164,77,11,10,34,16);
			RpPrint(Rp1,150,92,"50");
		break;
		case 4:
			ChangeColor(Rp1,181,77,11,10,34,16);
			RpPrint(Rp1,150,92,"100");
		break;
		case 5:
			ChangeColor(Rp1,198,77,12,10,34,16);
			RpPrint(Rp1,150,92,"200");
		break;
		RectCopy(Rp2,150,92,23,9,Rp1,160,92);
	}

	switch(AWG9_MODE) {
		case AWG9_SRCH_WIDE:
		case AWG9_SRCH_NARROW:
		case AWG9_SRCH_MEDIUM:
		case AWG9_SRCH_AUTHENTIC:
			ChangeColor(Rp1,243,133,16,6,34,16);
			DrawLine(Rp1,243,134,243,138,29);
			DrawLine(Rp1,243,133,259,133,29);
			DrawLine(Rp1,259,134,259,138,29);
		break;
		case AWG9_PDSTT:
			ChangeColor(Rp1,225,133,16,6,34,16);
			DrawLine(Rp1,225,134,225,139,29);
			DrawLine(Rp1,225,133,241,133,29);
			DrawLine(Rp1,241,134,241,139,29);
		break;
		case AWG9_TWSA:
		case AWG9_TWSA_NARROW:
		case AWG9_TWSA_MEDIUM:
		case AWG9_TWSA_AUTHENTIC:
			ChangeColor(Rp1,225,147,16,6,34,16);
			DrawLine(Rp1,225,148,225,152,29);
			DrawLine(Rp1,225,147,241,147,29);
			DrawLine(Rp1,241,148,241,152,29);
		break;
		case AWG9_TWSM:
		case AWG9_TWSM_NARROW:
		case AWG9_TWSM_MEDIUM:
		case AWG9_TWSM_AUTHENTIC:
			ChangeColor(Rp1,243,147,16,6,34,16);
			DrawLine(Rp1,243,148,243,152,29);
			DrawLine(Rp1,243,147,259,147,29);
			DrawLine(Rp1,259,148,259,152,29);
		break;
		case AWG9_RWS_WIDE:
		case AWG9_RWS_NARROW:
		case AWG9_RWS_MEDIUM:
		case AWG9_RWS_AUTHENTIC:
			ChangeColor(Rp1,243,158,16,6,34,16);
			DrawLine(Rp1,243,159,243,163,29);
			DrawLine(Rp1,243,158,259,158,29);
			DrawLine(Rp1,259,159,259,163,29);
		break;
	}
	if (ShowMouse) MouseShow();
}

//***************************************************************************
int DDDSwitches()
{
	static int RadarOn=0;
	static int RadarMode=0;
	static int PrevRadar=0;
	char s[10];
	SetFont(Rp2,F_3x5B);
	Rp2->APen=1;
	Rp2->BPen=1;


	MouseHIDE();
	/* Add check to see if it is already on!!! */
	if (AWG9_SNIFF && RadarOn) {
		ChangeColor(Rp1,229,112,14,5,16,34);
		RadarOn=0;
		DrawLine(Rp1,229,112,229,117,27);
		DrawLine(Rp1,229,111,243,111,26);
		DrawLine(Rp1,243,112,243,117,27);
	}
	if (!(AWG9_SNIFF) && !(RadarOn)) {
		ChangeColor(Rp1,229,112,14,5,34,16);
		RadarOn=1;
		DrawLine(Rp1,229,112,229,117,29);
		DrawLine(Rp1,229,111,243,111,29);
		DrawLine(Rp1,243,112,243,117,29);
	}


	if (RadarRange!=PrevRadar) {
		switch(RadarRange) {
			case 0:
				ChangeColor(Rp1,113,77,11,10,34,16);
				RpPrint(Rp2,150,92,"5");
			break;
			case 1:
				ChangeColor(Rp1,130,77,11,10,34,16);
				RpPrint(Rp1,150,92,"10");
			break;
			case 2:
				ChangeColor(Rp1,147,77,11,10,34,16);
				RpPrint(Rp1,150,92,"20");
			break;
			case 3:
				ChangeColor(Rp1,164,77,11,10,34,16);
				RpPrint(Rp1,150,92,"50");
			break;
			case 4:
				ChangeColor(Rp1,181,77,11,10,34,16);
				RpPrint(Rp1,150,92,"100");
			break;
			case 5:
				ChangeColor(Rp1,198,77,12,10,34,16);
				RpPrint(Rp1,150,92,"200");
			break;
			RectCopy(Rp2,150,92,23,9,Rp1,160,92);
		}
		switch(PrevRadar) {
			case 0:
				ChangeColor(Rp1,113,77,11,10,16,34);
			break;
			case 1:
				ChangeColor(Rp1,130,77,11,10,16,34);
			break;
			case 2:
				ChangeColor(Rp1,147,77,11,10,16,34);
			break;
			case 3:
				ChangeColor(Rp1,164,77,11,10,16,34);
			break;
			case 4:
				ChangeColor(Rp1,181,77,11,10,16,34);
			break;
			case 5:
				ChangeColor(Rp1,198,77,12,10,16,34);
			break;
		}
		PrevRadar=RadarRange;
	}

	if (AWG9_MODE!=RadarMode) {
		switch(AWG9_MODE) {
			case AWG9_SRCH_WIDE:
			case AWG9_SRCH_NARROW:
			case AWG9_SRCH_MEDIUM:
			case AWG9_SRCH_AUTHENTIC:
				if ((RadarMode == AWG9_SRCH_WIDE) || (RadarMode == AWG9_SRCH_NARROW)
					|| (RadarMode == AWG9_SRCH_MEDIUM)) break;
				ChangeColor(Rp1,243,133,16,6,34,16);
				DrawLine(Rp1,243,134,243,138,29);
				DrawLine(Rp1,243,133,259,133,29);
				DrawLine(Rp1,259,134,259,138,29);
			break;
			case AWG9_PDSTT:
				ChangeColor(Rp1,225,133,16,6,34,16);
				DrawLine(Rp1,225,134,225,139,29);
				DrawLine(Rp1,225,133,241,133,29);
				DrawLine(Rp1,241,134,241,139,29);
			break;
			case AWG9_TWSA:
			case AWG9_TWSA_NARROW:
			case AWG9_TWSA_MEDIUM:
			case AWG9_TWSA_AUTHENTIC:
				if ((RadarMode == AWG9_TWSA_NARROW)	|| (RadarMode == AWG9_TWSA_MEDIUM)) break;
				ChangeColor(Rp1,225,147,16,6,34,16);
				DrawLine(Rp1,225,148,225,152,29);
				DrawLine(Rp1,225,147,241,147,29);
				DrawLine(Rp1,241,148,241,152,29);
			break;
			case AWG9_TWSM:
			case AWG9_TWSM_NARROW:
			case AWG9_TWSM_MEDIUM:
			case AWG9_TWSM_AUTHENTIC:
				if ((RadarMode == AWG9_TWSM_NARROW)	|| (RadarMode == AWG9_TWSM_MEDIUM)) break;
				ChangeColor(Rp1,243,147,16,6,34,16);
				DrawLine(Rp1,243,148,243,152,29);
				DrawLine(Rp1,243,147,259,147,29);
				DrawLine(Rp1,259,148,259,152,29);
			break;
			case AWG9_RWS_WIDE:
			case AWG9_RWS_NARROW:
			case AWG9_RWS_MEDIUM:
			case AWG9_RWS_AUTHENTIC:
				if ((RadarMode == AWG9_RWS_WIDE) || (RadarMode == AWG9_RWS_NARROW)
					|| (RadarMode == AWG9_RWS_MEDIUM)) break;
				ChangeColor(Rp1,243,158,16,6,34,16);
				DrawLine(Rp1,243,159,243,163,29);
				DrawLine(Rp1,243,158,259,158,29);
				DrawLine(Rp1,259,159,259,163,29);
			break;
		}

		switch(RadarMode) {
			case AWG9_SRCH_WIDE:
			case AWG9_SRCH_NARROW:
			case AWG9_SRCH_MEDIUM:
			case AWG9_SRCH_AUTHENTIC:
				if ((AWG9_MODE == AWG9_SRCH_WIDE) || (AWG9_MODE == AWG9_SRCH_NARROW)
					|| (AWG9_MODE == AWG9_SRCH_MEDIUM)) break;
				ChangeColor(Rp1,243,133,16,6,16,34);
				DrawLine(Rp1,243,134,243,138,28);
				DrawLine(Rp1,243,133,259,133,26);
				DrawLine(Rp1,259,134,259,138,28);
			break;
			case AWG9_PDSTT:
				ChangeColor(Rp1,225,133,16,6,16,34);
				DrawLine(Rp1,225,134,225,139,28);
				DrawLine(Rp1,225,133,241,133,26);
				DrawLine(Rp1,241,134,241,139,28);
			break;
			case AWG9_TWSA:
			case AWG9_TWSA_NARROW:
			case AWG9_TWSA_MEDIUM:
			case AWG9_TWSA_AUTHENTIC:
				if ((AWG9_MODE == AWG9_TWSA_NARROW)	|| (AWG9_MODE == AWG9_TWSA_MEDIUM)) break;
				ChangeColor(Rp1,225,147,16,6,16,34);
				DrawLine(Rp1,225,148,225,152,28);
				DrawLine(Rp1,225,147,241,147,26);
				DrawLine(Rp1,241,148,241,152,28);
			break;
			case AWG9_TWSM:
			case AWG9_TWSM_NARROW:
			case AWG9_TWSM_MEDIUM:
			case AWG9_TWSM_AUTHENTIC:
				if ((AWG9_MODE == AWG9_TWSM_NARROW)	|| (AWG9_MODE == AWG9_TWSM_MEDIUM)) break;
				ChangeColor(Rp1,243,147,16,6,16,34);
				DrawLine(Rp1,243,148,243,152,28);
				DrawLine(Rp1,243,147,259,147,26);
				DrawLine(Rp1,259,148,259,152,28);
			break;
			case AWG9_RWS_AUTHENTIC:
			case AWG9_RWS_WIDE:
			case AWG9_RWS_NARROW:
			case AWG9_RWS_MEDIUM:
				if ((AWG9_MODE == AWG9_RWS_WIDE) || (AWG9_MODE == AWG9_RWS_NARROW)
					|| (AWG9_MODE == AWG9_RWS_MEDIUM)) break;
				ChangeColor(Rp1,243,158,16,6,16,34);
				DrawLine(Rp1,243,159,243,163,28);
				DrawLine(Rp1,243,158,259,158,26);
				DrawLine(Rp1,259,159,259,163,28);
			break;
		}
		RadarMode=AWG9_MODE;
	}

	if (ShowMouse) MouseShow();
}


//***************************************************************************
int DrawCompass()
{
	int x,y;
	int soffset;
	int mseg;

	if (View==FRONT)
		soffset=0;
	else
	  	soffset=112;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		TransRectCopy(mseg,218,2,32,35,Rp2,219,(166-soffset),0,0);
	}

	Rotate2D(0,10,-OurHead,&x,&y);
	DrawLine(Rp2,235,178-soffset,235-x,178-scaley(y)-soffset,(LightsOn?54:2));

	RectCopy(Rp2,219,166-soffset,32,27,Rp1,219,166-soffset);
}

//***************************************************************************
int DrawVVGauge()
{
	int vv;
	int x,y;
	int soffset;
	int mseg;
	int tv;

	if (View==FRONT)
		soffset=0;
	else
		soffset=112;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		TransRectCopy(mseg,189,2,25,22,Rp2,26,(136-soffset),0,0);
	}

	tv=VVI*6;              // = ft/min /10
	if (tv>400) tv=400;
	if (tv<-400) tv=-400;
	tv= tv*65;             // 2500 is about 90 degrees on dial
	x=-ACosB(8,tv);
	y=ASinB(8,tv);

	DrawLine(Rp2,38,147-soffset,38+x,(147-scaley(y)-soffset),(LightsOn?54:2));
	RectCopy(Rp2,26,137-soffset,23,19,Rp1,26,137-soffset);
}

//***************************************************************************
int DrawAltGauge()
{
	int font=0;
	unsigned int alt;
	int x,y;
	unsigned int althund;
	int newalt;
	char s[10];
	int soffset;
	int mseg;


	if (View==FRONT)
		soffset=0;
	else
		soffset=110;

	GetFont(Rp2,font);
	SetFont(Rp2,F_3x5B);
	Rp2->APen=(LightsOn?54:2);
	alt = OurAlt;
	althund = OurAlt%1000;

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
		TransRectCopy(mseg,1,35,33,29,Rp2,55,(164-soffset),0,0);
	}

	itoa((alt/1000),s,10);
	RectFill(Rp2,59,(175-soffset),10,6,22);
	RpPrint(Rp2,61,(175-soffset),s);
	Rotate2D(0,-10,althund*65,&x,&y);
	DrawLine(Rp2,71,(177-soffset),70-x,(177+scaley(y)-soffset),(LightsOn?54:2));

	if (!MsgOnScreen)
		RectCopy(Rp2,52,(162-soffset),39,28,Rp1,52,(162-soffset));
	else
		RectCopy(Rp2,52,(162-soffset),39,28,Rp1,52,(162-soffset));

	SetFont(Rp2,font);
}

//***************************************************************************
int DrawStores()
{
	int i,j;
	int status;
	int ypos;
	int prior;
	int storelight[8][2] = {
		96, 127,	/* 1 */
		107,127,	/* 3 */
		118,127,	/* 7 */
		129,127,	/*  9 - 11 */
		185,127,    /* 10 - 12 */
		196,127,	/* 8 */
		207,127,	/* 4 */
		218,127,	/* 2 */
	};

	prior = AAGetStation();

	if (View==FRONT)
		ypos=127;
	else
		ypos=16;

	for (i=0; i<12; i++) {
		if (i==4) continue;
		if (i==5) continue;

		if (F14_STATION[i]==-1) continue;

		/* Currently the only stores status is filled/empty - need */
		/* currently designated */
		if (F14_STATION[i]!=0) {
			if (LightsOn)
				status=2;
			else
				status=2;
		} else
			status=5;

		if (i==prior) status=16;

		if (i==0) j=0;
		if (i==1) j=7;

		if (i==2) j=1;
		if (i==3) j=6;

		if (i==6) j=2;
		if (i==7) j=5;


		if (i==8) j=4;
		if (i==9) j=3;

		if (i==10) j=4;
		if (i==11) j=3;

		if (status==16) {
			RectFill(Rp2,storelight[j][0],ypos,6,3,BLACK);
			DrawDot(Rp2,storelight[j][0]+1,ypos,WHITE);
			DrawDot(Rp2,storelight[j][0]+3,ypos,WHITE);
			DrawDot(Rp2,storelight[j][0]+5,ypos,WHITE);
			DrawDot(Rp2,storelight[j][0]+0,ypos+1,WHITE);
			DrawDot(Rp2,storelight[j][0]+2,ypos+1,WHITE);
			DrawDot(Rp2,storelight[j][0]+4,ypos+1,WHITE);
			DrawDot(Rp2,storelight[j][0]+1,ypos+2,WHITE);
			DrawDot(Rp2,storelight[j][0]+3,ypos+2,WHITE);
			DrawDot(Rp2,storelight[j][0]+5,ypos+2,WHITE);
		} else {
			RectFill(Rp2,storelight[j][0],ypos,6,3,status);
		}
		RectCopy(Rp2,storelight[j][0],ypos,6,2,Rp1,storelight[j][0],ypos);
	}
}


//***************************************************************************
DrawAOADisplay()
{
	int inewaoa;

	RectFill(Rp2,82,109,3,17,29);
	inewaoa = (PitchAngle/182) * 3;
	if (inewaoa > 30) inewaoa=30;
	if (inewaoa <0) inewaoa=0;
	RectFill(Rp2,82,125 - inewaoa,3,inewaoa,2);
	DrawLine(Rp2,82,110,84,110,34);
	DrawLine(Rp2,82,115,84,115,34);
	DrawLine(Rp2,82,120,84,120,34);
 	DrawLine(Rp2,82,125,84,125,34);
	RectCopy(Rp2,82,109,3,17,Rp1,82,109);
}


//***************************************************************************
DrawSweepDisplay()
{
	static int prevrot;
	int pos;


	if (prevrot!=WingRot) {
		if (WingRot<=(30*DEG))
	   		pos=0;
		if (((WingRot>(30*DEG)) && (WingRot<(40*DEG))) || (WingRot==(40*DEG)))
			pos=1;
		if (((WingRot>(40*DEG)) && (WingRot<(50*DEG))) || (WingRot==(50*DEG)))
			pos=2;
		if ((WingRot>(50*DEG)) && (WingRot<(60*DEG)))
			pos=3;
		if (WingRot>=(60*DEG))
			pos=4;

		prevrot=WingRot;
		RectFill(Rp2,234,105,1,21,33);
		DrawDot(Rp2,234,108,32);
		DrawDot(Rp2,234,113,32);
		DrawDot(Rp2,234,118,32);
		DrawDot(Rp2,234,123,32);
		RectCopy(Rp2,234,105,1,21,Rp1,234,105);

		switch(pos) {
			case 0:
				DrawLine(Rp2,234,105,234,107,2);
			break;
			case 1:
 				DrawLine(Rp2,234,109,234,112,2);
			break;
			case 2:
				DrawLine(Rp2,234,114,234,117,2);
			break;
			case 3:
				DrawLine(Rp2,234,119,234,122,2);
			break;
			case 4:
				DrawLine(Rp2,234,124,234,126,2);
			break;
		}
		RectCopy(Rp2,234,105,1,21,Rp1,234,105);
	}
}

//***************************************************************************
int DrawEngineStat()
{
	int Lrpm,Rrpm;
	int Lff,Rff;

	(int)Lrpm = (ThrustI>144 ? 144: ThrustI);
	(int)Rrpm = (ThrustI>144 ? 144: ThrustI);

	Lff = (THRUST/4)-6;
	Rff = (THRUST/4)-6;

	if (Lrpm<0) Lrpm=0;
	if (Rrpm<0) Rrpm=0;
	if (Lff<0) Lff=0;
	if (Rff<0) Rff=0;

	if ((Lrpm>22) && (EgtL<15)) EgtL++;
	if ((Rrpm>22) && (EgtR<15)) EgtR++;
	if ((Lrpm>102) && (EgtL<22)) EgtL++;
	if ((Rrpm>102) && (EgtR<22)) EgtR++;
	if ((Lff<20) && (EgtL>15)) EgtL--;
	if ((Rff<20) && (EgtR>15)) EgtR--;

	if (Lff<3 && EgtL>5) EgtL--;
	if (Rff<3 && EgtR>5) EgtR--;

	Lrpm=(Lrpm/4)-5;
	Rrpm=(Rrpm/4)-5;


	RectFill(Rp2,51,103,3,31,BLACK);
	RectFill(Rp2,59,103,3,31,BLACK);

	RectFill(Rp2,71,103,3,31,BLACK);
	RectFill(Rp2,79,103,3,31,BLACK);

	RectFill(Rp2,91,103,3,31,BLACK);
	RectFill(Rp2,99,103,3,31,BLACK);

	RectFill(Rp2,51,135-Lrpm,3,Lrpm,(LightsOn?54:2));
	RectFill(Rp2,59,135-Lrpm,3,Lrpm,(LightsOn?54:2));

	RectCopy(Rp2,51,103,3,31,Rp1,51,103);
	RectCopy(Rp2,59,103,3,31,Rp1,59,103);

	RectFill(Rp2,71,135-EgtL,3,EgtL,(LightsOn?54:2));
	RectFill(Rp2,79,135-EgtR,3,EgtR,(LightsOn?54:2));

	RectCopy(Rp2,71,103,3,31,Rp1,71,103);
	RectCopy(Rp2,79,103,3,31,Rp1,79,103);

	RectFill(Rp2,91,135-Lff,3,Lff,(LightsOn?54:2));
	RectFill(Rp2,99,135-Rff,3,Rff,(LightsOn?54:2));

	RectCopy(Rp2,91,103,3,31,Rp1,91,103);
	RectCopy(Rp2,99,103,3,31,Rp1,99,103);
}

//***************************************************************************
DisplayBackLights()
{
	if (CheckLights(SAM_LIGHT))
		ChangeColor(Rp1,290,72,309,77,18,17);
	else
		ChangeColor(Rp1,290,72,309,77,17,18);

	if (CheckLights(AAA_LIGHT))
		ChangeColor(Rp1,290,80,309,85,18,17);
	else
		ChangeColor(Rp1,290,80,309,85,17,18);


	if (CheckLights(AI_LIGHT))
		ChangeColor(Rp1,290,88,309,93,18,17);
	else
		ChangeColor(Rp1,290,88,309,93,17,18);

}


//***************************************************************************
DisplayLights(int lightnumber,int mode)
{
	if (mode) {
		ChangeColor(Rp1,CockpitLights[lightnumber].x,
				CockpitLights[lightnumber].y,
				CockpitLights[lightnumber].l,
				CockpitLights[lightnumber].w,
				CockpitLights[lightnumber].fromcolor,
				CockpitLights[lightnumber].tocolor);
	}
	else {
		ChangeColor(Rp1,CockpitLights[lightnumber].x,
				CockpitLights[lightnumber].y,
				CockpitLights[lightnumber].l,
				CockpitLights[lightnumber].w,
				CockpitLights[lightnumber].tocolor,
				CockpitLights[lightnumber].fromcolor);
	}
}

//***************************************************************************
DoCRT(int i)
{
    int a, d;

    RpCRT = CrtRps[i];
    d = CrtDisplays[i];

    switch (d) {
        case VDI:
			if (VdiMode) {
				a=VdiCrt(i);
				FlirCopy(VdiMask);
			} else {
				a=DrawGunCameraView();
				if (ConfigData.GunCamera==2)
					FlirCopy(VdiMask);
			}
		break;
        case HSD:
			if (RadarRepeat) {
				if (AWG9_REALITY_LEVEL == AWG9_EASY) {
					DoTac(i);
				} else {
					DoTID(i);
				}
			} else {
				a=HsdCrt(i);
				FlirCopy(HsdMask);
			}
		break;
		case DDD:
			a=DoDDD(i);
		break;
		case TID:
			if (AWG9_REALITY_LEVEL==AWG9_EASY) {
				a=DoTAC(i);
			} else {
				a=DoTID(i);
			}
		break;
		case TEWS:
			a=TEWS_GFX();
		break;
		case ADI:
			a=AdiCrt(i);
		break;
        default:
			a=0;
		break;
    }
    CrtUpdate[i] = a;
}


//***************************************************************************
int ZoomDisplay()
{
	if (!BackSeat) {
		if (View==FRONT) {
         	GunCameraZoom(1);
		}
		if (View==DOWN) {
			if (Tscale<4) Tscale++;
		}
	} else {
		if (View==FRONT) {
			AWG9_DEC_RANGE();
			if (RadarRange>0) RadarRange--;
		}
		if (View==DOWN) {
			if (Tscale<4) Tscale++;
		}
	}
}

//***************************************************************************
UnZoomDisplay()
{
	if (!BackSeat) {
		if (View==FRONT) {
	        GunCameraZoom(-1);
		}
		if (View==DOWN) {
			if (Tscale>0) --Tscale;
		}
	} else {
		if (View==FRONT) {
			AWG9_INC_RANGE();
			if (RadarRange<5) RadarRange++;
		}
		if (View==DOWN) {
			if (Tscale>0) --Tscale;
		}
	}
}

//***************************************************************************
MakeItGreen()
{
	VdiMask[MASKX] = RpCRT->XBgn;
	VdiMask[MASKY] = RpCRT->YBgn;
	FlirCopy(VdiMask);
}

//***************************************************************************
ResetCockpitLights()
{
	int i;

	OLDMM =
	OldFuel =
	OldEMIS =
	OldMC   =
	OldThrust = -1;

	for (i=0; i<40; i++) OldLights[i]=0;
}

//***************************************************************************
UpdateCockpitLites()
{
	UpdateLights();
}


//***************************************************************************
UpdateLights()
{
	int status;
	UWORD mseg;

	/* Check if status has changed */

	status = CheckLights(GEAR_LIGHT);
	if (!(OldLights[GEAR_LIGHT] == status)) {
		DisplayLights(GEAR_LIGHT,status);
		OldLights[GEAR_LIGHT]=status;
	}

	status = CheckLights(BRAKE_LIGHT);
	if (!(OldLights[BRAKE_LIGHT] == status)) {
		DisplayLights(BRAKE_LIGHT,status);
		OldLights[BRAKE_LIGHT]=status;
	}

	status = CheckLights(HOTTRIG_LIGHT);

	if (!(OldLights[HOTTRIG_LIGHT] == status)) {
		DisplayLights(HOTTRIG_LIGHT,status);
		OldLights[HOTTRIG_LIGHT]=status;
		if (status) {
			MouseHIDE();
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,113,149,18,19,Rp2,213,102,0,0);
			}
			RectCopy(Rp2,213,102,18,19,Rp1,213,102);
			if (ShowMouse) MouseShow();
		} else {
			MouseHIDE();
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,134,149,18,19,Rp2,213,102,0,0);
			}
			RectCopy(Rp2,213,102,18,19,Rp1,213,102);
			if (ShowMouse) MouseShow();
		}
	}

	status = CheckLights(SEAMLOCK_LIGHT);
	if (!(OldLights[SEAMLOCK_LIGHT] == status)) {
		DisplayLights(SEAMLOCK_LIGHT,status);
		OldLights[SEAMLOCK_LIGHT]=status;
	}
	status = CheckLights(SAM_LIGHT);
	if (!(OldLights[SAM_LIGHT] == status)) {
		DisplayLights(SAM_LIGHT,status);
		OldLights[SAM_LIGHT]=status;
	}

	status = CheckLights(AI_LIGHT);
	if (!(OldLights[AI_LIGHT] == status)) {
		DisplayLights(AI_LIGHT,status);
		OldLights[AI_LIGHT]=status;
	}

	status = CheckLights(AAA_LIGHT);
	if (!(OldLights[AAA_LIGHT] == status)) {
		DisplayLights(AAA_LIGHT,status);
		OldLights[AAA_LIGHT]=status;
	}
	status = CheckLights(LEFTFIRE_LIGHT);
	if (!(OldLights[LEFTFIRE_LIGHT] == status)) {
		DisplayLights(LEFTFIRE_LIGHT,status);
		OldLights[LEFTFIRE_LIGHT]=status;
	}
	status = CheckLights(RIGHTFIRE_LIGHT);
	if (!(OldLights[RIGHTFIRE_LIGHT] == status)) {
		DisplayLights(RIGHTFIRE_LIGHT,status);
		OldLights[RIGHTFIRE_LIGHT]=status;
	}

}

//***************************************************************************
static CheckLights(int i)
{
	static int tempflash=0;

	switch(i) {
		case 0:
			return(0);
		case 1:
			return !(Status & WHEELSUP);
		case 2:
			return ((Status & BRAKESON) && !(REALFLIGHT && (Damaged&D_HYDRAULICS)));
		case 3:
			return HookDown;
		case 5:
			return MLAUNCH;
		case 6:
			if (LastMud>1) {
				if (tempflash) {
					tempflash ^= 1;
					return(1);
				} else {
					tempflash ^= 1;
					return(0);
				}
			}
			if (LastSpike>1) {
				return(1);
			} else {
				return(0);
			}
		case 7:
			return AIDETECT;
		case 8:
			return !(LOCKED == -1);
		case 10:
			return MasterArmSW;
		case 11:
			return (Damaged&D_LEFTENGINE);
		case 19:
			return (Damaged&D_RIGHTENGINE);
	}
}

//***************************************************************************
ShowFuel()
{
    int f, val;
	int Lfuel,Rfuel;
	int Rfbar,Lfbar;


//    f = (Fuel+49)/50;
	f = Fuel;
	Lfuel = f/2;
	Rfuel = f/2;

    if (f!=OldFuel) {
        OldFuel = f;

		Lfbar = Lfuel/300;
		Rfbar = Rfuel/300;

		if (Lfbar>140) Lfbar=140;
		if (Rfbar>140) Lfbar=140;

        MouseHIDE ();
        Rp1->FontNr=F_3x5A;
        Rp1->APen=(LightsOn?54:2);

        RectFill (Rp1, 234,96, 20,5, BLACK);
        RpPrint  (Rp1, 234,96, TXTN(f));
		RectFill(Rp1,234,106,20,5,BLACK);
		RpPrint(Rp1,234,106,TXTN(BingoFuel));

		RectFill(Rp1,220,122,15,5,BLACK);
		RpPrint(Rp1,220,122,TXTN(Lfuel));

		RectFill(Rp1,252,122,15,5,BLACK);
		RpPrint(Rp1,252,122,TXTN(Rfuel));

		RectFill(Rp2,238,114,5,27,BLACK);
		RectFill(Rp2,238,141-Lfbar,5,Lfbar,(LightsOn?54:2));
		RectCopy(Rp2,238,114,5,27,Rp1,238,114);

		RectFill(Rp2,244,114,5,27,BLACK);
		RectFill(Rp2,244,141-Rfbar,5,Rfbar,(LightsOn?54:2));
		RectCopy(Rp2,244,114,5,27,Rp1,244,114);
    }
}


//***************************************************************************
EngineAlreadyOn()
{
}

//***************************************************************************
EngineMonitor()
{
#ifdef YEP
    if (THRUST!=0 && TempLeft<930) { // wind up the numbers at engine startup
        TempLeft += 9;
        TempRight = TempLeft+3;

        if ( OilLeft< 75)  {
            OilLeft += 12;
            OilRight = OilLeft +1;
        }

        if ((QTimer&31)==0) {        // every 8 seconds after warm-up, let them wander a little

            if (OilLeft>=75) {
                OilLeft   =  75+rnd2(5);
                OilRight  =  75+rnd2(4);
            }

            if (TempLeft>=930) {
                TempLeft  = 930+rnd2(21);
                TempRight = 930+rnd2(23);
            }
        }

        OldThrust = -1;
    }
    if (Fuel==0) OilLeft = OilRight = TempLeft = TempRight = 0;
#endif
}

//***************************************************************************
JetisonSwitch(int mode)
{
	UWORD mseg;

	MouseHide();
	if (mode) {
		JetisonSW++;
		if (View==FRONT && (!BackSeat)) {
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,111,125,20,20,Rp2,88,102,0,0);
			}
			RectCopy(Rp2,88,102,20,20,Rp1,88,102);
		}
	}
	else
	{
		JetisonSW=0;
		if (View==FRONT && (!BackSeat)) {
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,134,125,20,20,Rp2,88,102,0,0);
			}
			RectCopy(Rp2,88,102,20,20,Rp1,88,102);
		}
	}
	if (ShowMouse) MouseSHOW();
}

//***************************************************************************
MasterArmSwitch()
{
	UWORD mseg;
	MouseHIDE();

	if (!MasterArmSW) {
		MasterArmSW++;
		if (View==FRONT) {
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,113,149,18,19,Rp2,213,102,0,0);
			}
			RectCopy(Rp2,213,102,18,19,Rp1,213,102);
			ChangeColor(Rp1,119,120,11,3,18,16);
		}
	}
	else
	{
		MasterArmSW=0;
		if (View==FRONT) {
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,0))) {
				TransRectCopy(mseg,134,149,18,19,Rp2,213,102,0,0);
			}
			RectCopy(Rp2,213,102,18,19,Rp1,213,102);
			ChangeColor(Rp1,119,120,11,3,16,18);
		}
	}
	if (ShowMouse) MouseSHOW();
}


