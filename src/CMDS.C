/*  File:   Cmds.c                                */
/*  Author: Sid Meier / Andy Hollis /             */
/*  Mike McDonald								  */
/*                                                */
/*  Proccess all game related keyboard commands.  */
/*  Flight related commands are handled in        */
/*  FLIGHT2.C                                     */

#include <dos.h>
#include <bios.h>

#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "sounds.h"
#include "keycodes.h"
#include "F15defs.h"
#include "awg9.h"
#include "common.h"
#include "armt.h"
#include <stdlib.h>
#include <string.h>
#include "setup.h"


#define	MOUSETWSA	0x0001
#define	MOUSETWSM	0x0002
#define	MOUSERWS	0x0003
#define	MOUSEPDS	0x0004
#define DESTARG		0x0005
#define CLRTARG		0x0006
#define DATALINK	0x0007
#define HIGHTARG	0x0008

#define HUDFILTER	0x0009

#define THROTTLE_1	0x000a
#define THROTTLE_2	0x000b
#define THROTTLE_3	0x000c
#define THROTTLE_4	0x000d
#define THROTTLE_5	0x000e

#define RDR_RNG_5	0x0010
#define RDR_RNG_10	0x0011
#define RDR_RNG_20	0x0012
#define RDR_RNG_50	0x0013
#define RDR_RNG_100	0x0014
#define RDR_RNG_200	0x0015

#define RDR_ELVN_60	0x0016
#define RDR_ELVN_40	0x0017
#define RDR_ELVN_20	0x0018
#define RDR_ELV_0	0x0019
#define RDR_ELVP_20	0x001a
#define RDR_ELVP_40	0x001b
#define RDR_ELVP_60	0x001c

#define CHNG_AZ		0x001d
#define CHNG_DEG 	0x001e

#define CHNG_WAY	0x0020
#define CHNG_RNG	0x0021

extern int tempvar;
extern int AIWingMenu;

extern  RPS *Rp1, *Rp2, *Rp3D;

extern  UWORD   RudderStick;

extern  UWORD   RadarCursorSprite[];
extern  UWORD   MouseSprite;

extern  int     Px,Py,Alt,OurAlt, View,PovMisl,Mscale,vScale,Rtime;
extern  long    PxLng,PyLng;
extern  int     atarg,gtarg;
extern  int     ThrustI;

extern  int     ejectX,ejectY,ejectZ;
extern  int     Knots,OurRoll,OurPitch,THRUST,CloseBase,AutoGearOk;
extern  UWORD   AirSpeed;

extern int  MyMSGTYP;
extern int msgtime;
extern  char    Txt[];
extern  int     Speedy,MinAlt;
extern  UWORD   FrameRate10, FTicks;
extern  UBYTE   ExitCode;
extern  int     BackSeat;
extern  int     Panning;
extern  UWORD	Joystick;
extern  UWORD   ThrustStick;
extern  UWORD   CoolyStick;
extern  int     OutsideWidth;
extern  int     OutsideLength;
extern  UWORD   TimeOfDay;
extern  int     CrtDisplays[];
extern  int     CrtConfig[3][7];
extern  RPS     *CrtRps[];
extern  int     PadPitch;
extern  int     PadHead;
extern  int     CurCRT;
extern  int     DeClutter;
extern  int     REALFLIGHT;
extern  int     JAMMER;
extern  int     AnimVals[];
extern  int     LIGHTS;
extern  UBYTE   ctrl_b_flag;
extern  UWORD   StickType;
extern   int   HookDown;
extern  char    KeyJoyX,KeyJoyY;
extern   int   MYLOAD;

/* F14 Specific */
extern int VdiMode;
extern int JetisonSW;
extern int MasterArmSW;
int mapmode=0;
extern int RadarRepeat;
extern int AnimCnt;
extern int OurLoadOut;
extern int SNDDETAIL;

int     PauseHit = 0;
int     TickDelay=1,KbSense=2;
int     DBlaunch=0;

int     ThrustChange=  0;
int     MouseCrtX   = -1;
int     MouseCrtY   = -1;
int     MouseDsp    = -1;
int     ShowMouse   =  0;
int     MyMouseHits;
int 	tempsound=0;
int 	tempsound2=0;

int	SteadyHead=0;
int	SteadyPitch=0;

UWORD KEYPRINT;
UWORD KEYPRINT2;

UBYTE  mS1n,mS2n;
int	RUDDERVALS=0;

/* F14 Specific variables */
extern int hsdmode;
extern int Lightemup;
extern int DesignateMode;
extern int DataLink;
extern int ClrTarg;
extern int HighlightTarg;
extern int SelectedPal;
extern int TransAmount;

extern struct FPARAM{
	long	X;
  	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;


#define	NUM_FRONT_TOP	3
#define NUM_FRONT_LEFT	6
#define NUM_FRONT_RIGHT 1
#define NUM_FRONT_BOT	2
#define	NUM_BACK_TOP	20
#define NUM_BACK_LEFT	2
#define NUM_BACK_BOT	8


typedef struct {
    int     X;
    int     Y;
    int     XX;
    int     YY;
    int     KeyVal;
} cs;


cs FrontTopSpots[NUM_FRONT_TOP] =
{	90,105,104,119, SHIFTJ,   // JETTISON WEAPON
	215,107,223,118, MKEY,
	208,163,217,175, HUDFILTER,
};

cs FrontLeftSpots[NUM_FRONT_LEFT] =
{
	255,139,284,167, GKEY,
	77,142,113,179, THROTTLE_1,
	110,142,146,177, THROTTLE_2,
	146,144,180,178, THROTTLE_3,
	180,144,210,178, THROTTLE_4,
	208,144,250,178, THROTTLE_5,
};

cs FrontRightSpots[NUM_FRONT_RIGHT] =
{
	11,110,46,142, HKEY,
};

cs BackTopSpots[NUM_BACK_TOP] =
{	229,112,243,117, RKEY,		// RADAR ON
	245,112,259,117, IKEY,  	// IFF
	225,134,241,139, LKEY,
	243,134,259,138, MOUSEPDS,
	225,148,241,152, MOUSETWSA,
	243,148,259,152, MOUSETWSM,
	243,159,259,163, MOUSERWS,
	86,105,99,114, RDR_ELVP_60,
	86,115,99,122, RDR_ELVP_40,
	86,122,99,130, RDR_ELVP_20,
	86,130,100,138,RDR_ELV_0,
	86,138,100,146,RDR_ELVN_20,
	86,146,100,154,RDR_ELVN_40,
	88,154,102,162,RDR_ELVN_60,
	113,77,125,89, RDR_RNG_5,
	130,76,142,89, RDR_RNG_10,
	147,76,159,89, RDR_RNG_20,
	164,76,176,89, RDR_RNG_50,
	181,76,193,89, RDR_RNG_100,
	198,76,210,89, RDR_RNG_200,
};

cs BackBotSpots[NUM_BACK_BOT] =
{	229,112,243,117, RKEY,
	243,159,259,163, DELETE,
	114,169,128,179, DESTARG,
	130,169,144,179, CLRTARG,
	146,169,160,179, HIGHTARG,
	114,181,128,191, DATALINK,
	213,40,250,62, CHNG_WAY,
	166,165,208,194, CHNG_RNG,
};

cs BackLeftSpots[NUM_BACK_LEFT] =
{   98,144,126,166, CHNG_AZ,
	82,168,108,182, CHNG_DEG,
};


//***************************************************************************
TakeInputs ()
{
    KBStatus &= 0x2F;  /* clear all the "lock" keys, except NUM-LOCK */

    KEYPRINT = 0;
    if (kbhit()) {
        KEYPRINT = _bios_keybrd (_KEYBRD_READ);
    }

    CheckMouseInput();
	while (kbhit()) _bios_keybrd (_KEYBRD_READ); /* Flush kbd buffer */

    if (Joystick) {
        GetJoystickInputs();
    } else {
        mS1n = ((((int) KeyJoyX&255)-128)*(KbSense+1))/3 + 128;
        mS2n = ((((int) KeyJoyY&255)-128)*(KbSense+1))/3 + 128;
    }
    if (ctrl_b_flag) KEYPRINT=ALTQKEY;
}

//***************************************************************************
GetJoystickInputs()
{
    struct STX     j;
    static UWORD CoolyKeys[]={ 0, PAD4, PAD2, PAD6, PAD8};
    static UWORD CHKeys[] = { 0, -1, -2, PAD4,    -4, 0, 0, PAD2,
                             -8,  0,  0, PAD6,  KKEY, 0, 0, PAD8};
    int    CoolyVal;
    int    Trigs;

    j =ReadJoy2(Joystick|RudderStick|ThrustStick|CoolyStick);
    mS1n = j.x+128;
    mS2n = j.y+128;

	// Rudders
	if (RudderStick) {
		RUDDERVALS=j.w;
		RUDDERVALS= (RUDDERVALS<0)? -(abs(RUDDERVALS)&0xfff8): (RUDDERVALS&0xfff8);
	}

    ThrustChange=0;
    if (ThrustStick) {
        ThrustChange = 128-j.z;
        if (ThrustChange<5) ThrustChange=1; // 1 is the flag
    }

	// ThrustMaster
    if (CoolyStick) {
        CoolyVal = (128-j.z+16) /32;
        if (CoolyVal) KEYPRINT = CoolyKeys[CoolyVal];
    }

	// Get Trigger Data
   	if (StickType==0)
      	Trigs=0;
   	else
      	Trigs = (~inp(0x201)>>4)&15;

	// CH PRO Sticks
    if (StickType==J_CHPRO) {
        Trigs = CHKeys[Trigs];
        if (Trigs>0) {
            KEYPRINT = Trigs;
            return(1);
        }
        Trigs = -Trigs;
    }

	/* Trigger stuff */
    if (Trigs&1) KEYPRINT = ENTER;

    if (!(Trigs&14)) {
        DBlaunch=0;
    }

    if (DBlaunch>0) {
        --DBlaunch;

    } else if (Trigs&2) {
        KEYPRINT = SPACEBAR;
        DBlaunch = 90/FTicks;   /* Fire every 1.5 seconds */

    } else if (Trigs&4) {
        KEYPRINT = BACKSPACE;
        DBlaunch = 30/FTicks;

    } else if (Trigs&8) {
        KEYPRINT = LKEY;
        DBlaunch = 30/FTicks;
    }
}

//***************************************************************************
CheckMouseInput()
{
    int        i, spr;
    int        MouseMoved;
    static int OldMouseX;
    static int OldMouseY;
    static int WasInCrt=0;
	static int lasttime;
	int curtime;

    MouseDsp=-1;

	/* Make mouse cursor disappear after a second of non-movement */
    MouseMoved = (MouseX!=OldMouseX || MouseY!=OldMouseY);

    if (MouseMoved) {
        OldMouseX=MouseX;
        OldMouseY=MouseY;
        ShowMouse = FrameRate;
    } else {
        if (ShowMouse!=0) --ShowMouse;
    }

    MyMouseHits = MouseHITS();


    if (View>=FRONTAL) {
        if (Button) {
            if (MouseX>160 && MouseY>100) {
                OutsideWidth  = MouseX*2-320;
                Rp3D->Width1  = OutsideWidth-1;

                OutsideLength = MouseY*2-200;
				if (OutsideLength>170) OutsideLength=170;
                Rp3D->Length1 = OutsideLength-1;

                MouseHIDE();
                ClearPage (0, BLACK);
                MouseSHOW();
            }
        }
    }

	if (!(BackSeat)) {
		if (View==FRONT) { /* Select Cockpit hotspots */
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_FRONT_TOP; i++) {
                    	if (MouseX>=FrontTopSpots[i].X && MouseX<=FrontTopSpots[i].XX
                    	&&  MouseY>=FrontTopSpots[i].Y && MouseY<=FrontTopSpots[i].YY) {
                        	KEYPRINT = FrontTopSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}
		if (View==LEFT) {
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_FRONT_LEFT; i++) {
                    	if (MouseX>=FrontLeftSpots[i].X && MouseX<=FrontLeftSpots[i].XX
                    	&&  MouseY>=FrontLeftSpots[i].Y && MouseY<=FrontLeftSpots[i].YY) {
                        	KEYPRINT = FrontLeftSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}
		if (View==RIGHT) {
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_FRONT_RIGHT; i++) {
                    	if (MouseX>=FrontRightSpots[i].X && MouseX<=FrontRightSpots[i].XX
                    	&&  MouseY>=FrontRightSpots[i].Y && MouseY<=FrontRightSpots[i].YY) {
                        	KEYPRINT = FrontRightSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}

	} else {
		if (View==FRONT) { /* Select Cockpit hotspots */
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_BACK_TOP; i++) {
                    	if (MouseX>=BackTopSpots[i].X && MouseX<=BackTopSpots[i].XX
                    	&&  MouseY>=BackTopSpots[i].Y && MouseY<=BackTopSpots[i].YY) {
                        	KEYPRINT = BackTopSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}
		if (View==LEFT) {
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_BACK_LEFT; i++) {
                    	if (MouseX>=BackLeftSpots[i].X && MouseX<=BackLeftSpots[i].XX
                    	&&  MouseY>=BackLeftSpots[i].Y && MouseY<=BackLeftSpots[i].YY) {
                        	KEYPRINT = BackLeftSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}

		if (View==DOWN) {
        	if ((WasInCrt||MouseMoved) && (spr = CursorInCrt())) {
            	MouseSETUP(spr,4,4);
            	if (!WasInCrt) WasInCrt ^= 1;
            	ShowMouse = FrameRate;
        	} else {
            	if (WasInCrt) {
                	MouseSETUP(MouseSprite,0,0);
                	WasInCrt ^= 1;
            	}
            	if (MyMouseHits & 1) {
                	for (i=0; i<NUM_BACK_BOT; i++) {
                    	if (MouseX>=BackBotSpots[i].X && MouseX<=BackBotSpots[i].XX
                    	&&  MouseY>=BackBotSpots[i].Y && MouseY<=BackBotSpots[i].YY) {
                        	KEYPRINT = BackBotSpots[i].KeyVal;
                    	}
                	}
            	}
			}
		}
	}
}


//***************************************************************************
CursorInCrt()
{
    int     Start, End, i, j, dtype;
    RPS     *Rp;

    MouseCrtX = -1;

	if (BackSeat) {
		if (View==FRONT)
			i=4;
		if (View==DOWN)
			i=5;

		dtype = CrtDisplays[i];
		Rp = CrtRps[i];
		MouseCrtX = MouseX-Rp->XBgn;
		MouseCrtY = MouseY-Rp->YBgn;
		MouseDsp  = i;


		if ((MouseCrtX>=0 && MouseCrtX<=Rp->Width1)
			&& (MouseCrtY>=0 && MouseCrtY<=Rp->Length1)) {
				switch (dtype) {
					case DDD:
						if (MyMouseHits) KEYPRINT = (MyMouseHits&1)?RDRMOUSEACQ:LKEY;
 						return RadarCursorSprite[2];
					case TID:
						if (MyMouseHits) KEYPRINT = (MyMouseHits&1)?RDRMOUSEACQ:LKEY;
						return RadarCursorSprite[0];
                    case RDR:
                        if (MyMouseHits) KEYPRINT = (MyMouseHits&1)?RDRMOUSEACQ:LKEY;
                        return RadarCursorSprite[2];
				}
            } else {
                MouseCrtX = -1;
			}
	}
    return 0;
}

//***************************************************************************
ProcessInputs()
{
	if (ThrustChange) {
		THRUST = (ThrustChange-1)*144/128;
		if (THRUST>130) {
			THRUST=144;
		} else {
			if (THRUST>100)
				THRUST=100;
		}
	}

	if (MasterArmSW)
		docannon();

    if (KEYPRINT) {
    	Cmds();
    }

    if (THRUST && !ThrustI) {
        Status &= ~BRAKESON;
        if (AirSpeed==0) {
			if ((SNDDETAIL!=1) && (SNDDETAIL!=3))
            	Sound(N_SpoolUp, 1,0);
        }
    }
}

//***************************************************************************
Cmds()
{
    int i,Move, prevwps;

    switch (KEYPRINT) {
        case DECPOWER:
			THRUST = rng (THRUST-10, 0,100);
		break;

        case INCPOWER:
			THRUST = rng (THRUST+ (THRUST<10?5:10), 0,100);
		break;

        case AKEY:
			THRUST = 144;
			Status &= ~BRAKESON;
		break;

		case VKEY:
			VdiMode ^=1;
		break;

        case MAXPOWER:
			THRUST = 100;
			Status &= ~BRAKESON;
		break;

        case MINPOWER:
			THRUST = 0;
		break;

        case BKEY:
			Status ^= BRAKESON;
         if (Status&BRAKESON)
            Message("Air brake on",RIO_NOW_MSG);
         else
            Message("Air brake off",RIO_NOW_MSG);
		break;

        case GKEY:
			if ((!OnTheDeck(FPARAMS.X, FPARAMS.Y, FPARAMS.Z)) && !ejected) {
				if (!(Status&WHEELSUP) || (Knots<350) || !REALFLIGHT) {
					if (!REALFLIGHT || (Status&WHEELSUP) || !(Damaged&D_HYDRAULICS)) {
						Status ^= WHEELSUP;
                  if (Status&WHEELSUP)
                     Message("Landing gear raised",RIO_NOW_MSG);
                  else
                     Message("Landing gear lowered",RIO_NOW_MSG);

						AutoGearOk = FALSE;
						//  Sound (N_GearOpenSeq, 1);
					}
				}
			}
			if (!(Status&WHEELSUP)) NoAccel();
		break;

		case ALTAKEY:
			AnimCnt++;
			if (AnimCnt>13) AnimCnt=0;
			sprintf(Txt,"AnimCnt: %i",AnimCnt);
			Message(Txt);
		break;

		case CTRLL:
			LaunchAircraft();
		break;
		case EKEY:
			EasyLanding();
		break;

		case SHIFTW:
			ChangeWeather();
		break;

        case SHIFTE:
			if (!ejected) {
				ejected = 1;
                ejectX = Px;
                ejectY = Py;
                ejectZ = Alt+8;

                Sound (N_Eject, 2,0);

                if (Knots+(abs(OurRoll)>>5)+(abs(OurPitch)>>5) > 500+(Rtime%500)) {
					LogIt (SLG_AIR_BAD_EJECT, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
                    EndGame (BADEJECT);
                }
				else {
					LogIt (SLG_AIR_GOOD_EJECT, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
				}
            }
		break;

        case PKEY:
			if (AutoAlt) {
            Message ("Autopilot off",RIO_NOW_MSG);
				AutoAlt=0;
			} else if (!(Damaged&D_NAV)) {
				AutoAlt = Alt>25 ?Alt:25;
            Message ("Autopilot on",RIO_NOW_MSG);
			}
		break;

        case SPACEBAR:
			if (MasterArmSW)
				firemissile();
		break;

        case FKEY:
			dropdecoy(FLARE, MAXPLANE);
		break;

        case CKEY:
			dropdecoy(CHAFF, MAXPLANE);
		break;

        case JKEY:
			if (!(Damaged&D_ECM)) {
				JAMMER ^= 1;
			}
		break;

        case MKEY:
			MasterArmSW ^=1;
		break;

        case KEY1:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 11;
			else
				AA_MODE (AA_GUN);
		break;
        case KEY2:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 12;
			else
				AA_MODE (AA_SRM);
		break;

        case KEY3:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 13;
			else
				AA_MODE (AA_MRM);
		break;

        case KEY4:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 14;
			else
				AA_MODE (AA_LRM);
		break;

        case KEY5:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 15;
			else
				ARMT_MODE(MASTER_NAV);
		break;

        case KEY6:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 16;
		break;

        case KEY7:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 17;
		break;

		case KEY8:
			if(AIWingMenu)
				behavecommands = (AIWingMenu * 10) + 18;
		break;
		case KEY9:
			behavecommands = (AIWingMenu * 10) + 19;
		break;

		case KEY0:
			behavecommands = (AIWingMenu * 10) + 20;
		break;

		case LEFTARROW:
			if((AIWingMenu) && (StickType != 0))
				behavecommands = (AIWingMenu * 10) + 11;
		break;

		case RIGHTARROW:
			if((AIWingMenu) && (StickType != 0))
				behavecommands = (AIWingMenu * 10) + 12;
		break;

		case UPARROW:
			if((AIWingMenu) && (StickType != 0))
				behavecommands = (AIWingMenu * 10) + 13;
		break;

		case DOWNARROW:
			if((AIWingMenu) && (StickType != 0))
				behavecommands = (AIWingMenu * 10) + 14;
		break;

        case SHIFTJ:
			JetisonSwitch(1);
			ARMT_jettison_all();
		break;

        case ZOOMKEY:
			if (!(View&VOFFSET))
				ZoomDisplay();
		break;
        case UNZOOMKEY:
			if (!(View&VOFFSET))
				UnZoomDisplay();
		break;
		case SHIFTR:
			RadarRepeat^=1;
		break;

		case SKEY:
			prevwps = NextWayPoint;
			if (++NextWayPoint>3)
				NextWayPoint=0;
			UpdateOurWPS(prevwps);
			ShowOurWPS();
		break;

        case SHIFTS:
			prevwps = NextWayPoint;
			if (--NextWayPoint<0)
				NextWayPoint=3;
			UpdateOurWPS(prevwps);
			ShowOurWPS();
		break;

        case ALTLKEY:
			CheckPlaneLand(-1);
		break;

        case SHIFTP:
			AwacsPicture();
		break;

		case TRIMUP:
			TrimIt(2);
		break;

		case TRIMDOWN:
			TrimIt(-2);
		break;

		case TRIMSPD:
		case TRIMSPD2:
			ToggleAutoTrim();
		break;

        case RKEY:
			AWG9_POWERUP();
		break;

        case INSERT:
		case CTRLQ:
			if (AWG9_REALITY_LEVEL==AWG9_MODERATE) {
				ChangeModerateScan(-1);
			}
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				CHANGE_AZIMUTH();
			}
		break;

        case DELETE:
		case CTRLA:
			ChangeSRCHMode(-1);
		break;
		case MOUSETWSA:
			ChangeSRCHMode(2);
		break;
		case MOUSETWSM:
			ChangeSRCHMode(1);
		break;
		case MOUSERWS:
			ChangeSRCHMode(3);
		break;
		case MOUSEPDS:
			ChangeSRCHMode(0);
		break;

        case LKEY:
			AWG9_LOCK();
		break;

        case KKEY:
			AWG9_MODE_BREAK();
		break;

        case HOME:
		case CTRLE:
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				CHANGE_NUM_BARS();
			}
		break;

        case PAGEUP:
		case CTRLR:
			AWG9_EL(1);
		break;

        case PAGEDOWN:
		case CTRLF:
			AWG9_EL(0);
		break;

        case IKEY:
			AWG9_IFF();
		break;

        case END:
		case CTRLD:
			AWG9_CHANGE_MODE(AWG9_SPECIAL);
		break;

        case BACKSPACE:
			AWG9_ACQ(0);
		break;

        case RDRMOUSEACQ:
			if ((BackSeat) && (View==FRONT)) {
				AWG9_ACQ(1);
			}
			if ((BackSeat) && (View==DOWN)) {
				if (DesignateMode)
					DesignateTIDTarg();
				if (HighlightTarg)
					SelectDesig();
			}
		break;

		case DESTARG:
			DesignateMode ^= 1;
			HighlightTarg=0;
		break;

		case CLRTARG:
			InitTWSData();
			DesignateMode=0;
			HighlightTarg=0;
			ClrTarg=10;
		break;

		case DATALINK:
			DataLink ^= 1;
		break;

		case HIGHTARG:
			HighlightTarg ^= 1;
			DesignateMode=0;
			SelectDesig();
		break;

		case HUDFILTER:
			ChangeHUDFilter();
		break;

		case THROTTLE_1:
		break;
		case THROTTLE_2:
		break;
		case THROTTLE_3:
		break;
		case THROTTLE_4:
		break;
		case THROTTLE_5:
		break;


		case RDR_RNG_5:
		break;
		case RDR_RNG_10:
		break;
		case RDR_RNG_20:
		break;
		case RDR_RNG_50:
		break;
		case RDR_RNG_100:
		break;
		case RDR_RNG_200:
		break;

		case RDR_ELVN_60:
		break;
		case RDR_ELVN_40:
		break;
		case RDR_ELVN_20:
		break;
		case RDR_ELV_0:
		break;
		case RDR_ELVP_20:
		break;
		case RDR_ELVP_40:
		break;
		case RDR_ELVP_60:
		break;

        case QUOTE:
			BackSeat^=1;
			View = FRONT;
			LookUp();
		break;

        case TKEY:
			NoAccel();
		break;

        case SHIFTT:
			if (Speedy<8) {
               	Speedy++;
               	FrameRate=(FrameRate*Speedy)/(Speedy-1);
               	if (FrameRate>15) FrameRate=15;
			}
		break;

        case ALTFKEY:
			TXT("F/S: ");
			TXN(FrameRate10);
			TXA(", FTicks: ");
			TXN(FTicks);
			Message(Txt,DEBUG_MSG);
		break;

        case ALTMKEY:   {
                            UWORD    free, total;

                            TXT ("Conventional Memory Available: ");
                            free = AvailSysMem ();
                            strcat (Txt, ltoa((long)free*16L, ntmp, 10));
                            Message (Txt,SYSTEM_MSG);

                            EMSpages (&free, &total);
                            TXT ("EMS memory: free-");
                            strcat (Txt, ltoa((long)free*16384L, ntmp, 10));
                            TXA ("  total-");
                            strcat (Txt, ltoa((long)total*16384L, ntmp, 10));
                            Message(Txt,SYSTEM_MSG);
                        }
                        break;

        case ALTQKEY:
			EndGame(0);
			ExitCode = 0;
		break;

        case ALTTKEY:
			Status ^= TRAINING;
			if(Status & TRAINING)
				LogIt (SLG_TRAINING_TURNED_ON, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
			else
				LogIt (SLG_TRAINING_TURNED_OFF, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
		break;
	}

    if (Status&TRAINING) {
		switch (KEYPRINT) {
        	case ALTMINUS:
				if (TimeOfDay<4) TimeOfDay=23*60+55;
                TimeOfDay -= 5;
				sprintf(Txt,"Time Of Day: %i  SP: %d TRANS: %d\n\r",(TimeOfDay/60),SelectedPal,TransAmount);
				mprintf(Txt);
           	break;

        	case ALTPLUS:
				TimeOfDay += 5;
                if (TimeOfDay>=24*60) TimeOfDay -=24*60;
				sprintf(Txt,"Time Of Day: %i  SP: %d TRANS: %d\n\r",(TimeOfDay/60),SelectedPal,TransAmount);
				mprintf(Txt);
			break;

        	case ALTRKEY:
				Help();
				ReArmWingman();
				JetisonSwitch(0);
				LogIt (SLG_AIR_REFUEL, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
			break;

        	case ALTSKEY:
				PyLng += 256L<<Mscale;
				SlewOurWingmen(0L, (-256L<<Mscale));
			break;
        	case ALTXKEY:
				PyLng -= 256L<<Mscale;
				SlewOurWingmen(0L, (256L<<Mscale));
			break;
        	case ALTZKEY:
				PxLng -= 256L<<Mscale;
				SlewOurWingmen((-256L<<Mscale), 0L);
			break;
        	case ALTCKEY:
				PxLng += 256L<<Mscale;
				SlewOurWingmen((256L<<Mscale), 0L);
			break;
		}
	}
}


//***************************************************************************
LocalCmds()
{
    int i;

	tempvar = 0;
	behavecommands = 0;
	if(PauseHit) {
		PauseMode();
	}
	PauseHit = 0;

    if (!(View&VOFFSET) && (KEYPRINT==ZOOMKEY || KEYPRINT==UNZOOMKEY))
		return(1);

    switch (KEYPRINT) {
        case ESCKEY:
			MainMenu();
		break;

        case ALTPKEY:
//			PauseMode();
			Message("                                 ...PAUSE...", WING_MENU_MSG);
			PauseHit = 1;
		break;

        case ALTVKEY:
			AdjustSounds();
		break;

        case SHIFTL:
			LIGHTS^=1;
		break;

		case CTRLH:
			ChangeHUDColor (+2);
		break;

		case SHIFTH:
			ChangeHUDColor (-2);
		break;

        case HKEY:
			SetHook((HookDown^1));
		break;

		case DKEY:
			DeClutter ^= 1;
		break;

		case F1KEY:
			View = FRONT;
			LookUp();
		break;
        case F2KEY:
			View = FRONTAL;
		break;
		case F3KEY:
			View=LSOVIEW;
		break;
		case F4KEY:
			View=TOWERVIEW;
		break;
        case F5KEY:
			View = STEADY;
		break;
        case F6KEY:
			View = PILOT;
		break;
        case F7KEY:
			NewMissileView();
		break;
		case F8KEY:
			NewLockView(PADLOCK);
		break;
        case F9KEY:
			NewTacView(MAGICCAM1);
		break;
        case F10KEY:
			NewTacView(MAGICCAM2);
		break;

		case PAD1:
		case PAD2:
		case PAD3:
		case PAD4:
		case PAD5:
		case PAD6:
		case PAD7:
		case PAD8:
		case PAD9:
			ProcessCoolyCmds();
		break;
        case LEFTANGLE:
			if (!RudderStick)
			{
				if(RUDDERVALS > 0)
					RUDDERVALS = 0;
				else if((RUDDERVALS - 8) > -127) {
					RUDDERVALS-=8;
				}
			}
		break;

        case RIGHTANGLE:
			if (!RudderStick)
			{
				if(RUDDERVALS < 0)
					RUDDERVALS = 0;
				else if((RUDDERVALS + 8) < 127) {
					RUDDERVALS+=8;
				}
			}
		break;

		case SLASH:
			RioMap();
		break;

        case ZOOMKEY:
			--vScale;
		break;
        case UNZOOMKEY:
			++vScale;
		break;

		/*************  Wingman Behavior and Communication commands **********/

		case ALTF1KEY:
		case ALTF2KEY:
		case ALTF3KEY:
		case ALTF4KEY:
		case ALTF5KEY:
		case ALTF6KEY:
		case ALTF7KEY:
		case ALTF8KEY:
		case ALTF9KEY:
		case ALTF10KEY:
			behavecommands = 1 + ((KEYPRINT-ALTF1KEY) >>8);
		break;

		case SF1KEY:
		case SF2KEY:
		case SF3KEY:
		case SF4KEY:
		case SF5KEY:
		case SF6KEY:
		case SF7KEY:
		case SF8KEY:
		case SF9KEY:
		case SF10KEY:
			behavecommands = 11 + ((KEYPRINT-SF1KEY) >>8);
		break;
        default:        return(1);
    }
    KEYPRINT=0;
}

//***************************************************************************
ProcessCoolyCmds()
{
	if (View==STEADY || View==MAGICCAM1
         || View==MAGICCAM2 || View==TOWERVIEW
         || View==POVVIEW ) {
    	UpdateSteadyAngles();

    } else if (View==PILOT) {
    	UpdatePadlockAngles();

    } else if (BackSeat) {
        BackSeatCoolyCmds();

    } else {
        FrontSeatCoolyCmds();
    }

    KEYPRINT = 0;
}

//***************************************************************************
int FrontSeatCoolyCmds()
{
    switch (KEYPRINT) {
        case PAD8:
			View=FRONT;
			LookUp();
		break;

        case PAD2:
			View=DOWN;
			LookDown();
		break;

		case PAD1:
			View=REARLEFT;
		break;

		case PAD3:
			View=REARRIGHT;
		break;

		case PAD6:
			View=RIGHT;
		break;

		case PAD4:
			View=LEFT;
		break;

        case PAD5:
			View=FRONT;
			LookUp();
		break;
    }

}

//***************************************************************************
int BackSeatCoolyCmds ()
{
    switch (KEYPRINT) {
        case PAD8:
			View=FRONT;
		break;
        case PAD2:
			View=DOWN;
		break;
		case PAD1:
			View=REARLEFT;
		break;
		case PAD3:
			View=REARRIGHT;
		break;
		case PAD6:
			View=RIGHT;
		break;
		case PAD4:
			View=LEFT;
		break;
        case PAD5:
			View=FRONT;
		break;
	}
}

//***************************************************************************
UpdateSteadyAngles()
{
	switch (KEYPRINT) {
		case PAD8:
			SteadyPitch-=0x200;
		break;

		case PAD2:
			SteadyPitch+=0x200;
		break;

		case PAD4:
			SteadyHead-=(0x400);
		break;

		case PAD6:
			SteadyHead+=(0x400);
		break;

        case PAD5:
            SteadyHead  = 0;
            SteadyPitch = 0;
		break;
	}

	if (abs(SteadyPitch)>0x3c00) {
		SteadyPitch = (SteadyPitch>0)? 0x3c00:-0x3c00;
	}
}

//***************************************************************************
UpdatePadlockAngles()
{
	switch (KEYPRINT) {
		case PAD8:
			PadPitch-=0x200;
		break;

		case PAD2:
			PadPitch+=0x200;
		break;

		case PAD4:
			PadHead-=(0x400);
			if (PadHead<-0x6c00) PadHead = 0x6c00;
		break;

		case PAD6:
			PadHead+=(0x400);
			if (PadHead>0x6c00) PadHead = -0x6c00;
		break;

        case PAD5:
            PadHead  = 0;
            PadPitch = 0;
		break;
	}

	if (abs(PadPitch)>0x3c00) {
		PadPitch = (PadPitch>0)? 0x3c00:-0x3c00;
	}
}

//***************************************************************************
PauseMode ()
{
    int z;

	SndSounds(N_Pause);

    do {
        while (!kbhit()) ;
    } while (_bios_keybrd (_KEYBRD_READ) == ALTPKEY);

    SndSounds(N_UnPause);

	if(MyMSGTYP == WING_MENU_MSG) {  /*  Kludge  */
		if(msgtime)
			msgtime=GETTIME() - 60;   // End Menu Msg if still there
	}
}


//***************************************************************************
NoAccel()
{
    if (Speedy!=1) {
        FrameRate = FrameRate/Speedy;
		if (FrameRate<3) FrameRate=3;
        Speedy = 1;
    }
}

UBYTE   PoliticallyCorrectNumlock;
//***************************************************************************
KBInit()
{
    while (kbhit()) getch();            // flush keyboard buffer

    if (KBType&0x10)
      {
      PoliticallyCorrectNumlock=KBStatus;
      KBStatus|=0x20;    // turn on NUM-LOCK for 101 keyboards
      }



}
PutNumLockBack()
{
    if (KBType&0x10)
      {
      KBStatus=PoliticallyCorrectNumlock;
      }
}


