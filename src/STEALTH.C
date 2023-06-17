/*  File:   Stealth.c                             */
/*  Author: Sid Meier                             */
/*                                                */
/*  Main game logic for Stealth: enemy radar,     */
/*  enemy planes,  missiles, countermeasures,     */
/*  gunfire, and lots more.  Does NOT include     */
/*  flight equations and graphics.                */

#include <Dos.h>
#include "library.h"
#define _include_data
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "sounds.h"
#include "stdio.h"
#include "f15defs.h"
#include "common.h"
#include "armt.h"
#include "keycodes.h"
#include "speech.h"
#include "event.h"
#include "setup.h"
#include "planeai.h"

extern  int     OurHead,OurPitch,OurRoll,Knots;
extern  long    PxLng;
extern  long    PyLng;
extern  COORD   Px,Py;
extern  UWORD   Alt,OurAlt,OldAlt;
extern  long    RadarAlt;
extern  int     VVI;
extern  UWORD   AirSpeed;
extern  int     Tscale;
extern  int     View;
extern  int     TickDelay;
extern  int     THRUST;
extern	int		ThrustI;
extern  char    BEND;
extern  UWORD   TimeOfDay;
extern  int     JAMMER[];
extern  int     FM[3][3];
extern	int 	REALFLIGHT;
extern  int     MASTER_CAUTION;

UWORD   Rtime = 0, QTime = 0, QTimer = 0, FrameRate10=40, FTicks=0;
int     AutoGearOk=1;
int     detected=0, atarg=-1,gtarg=-1,ImHit;
COORD   xlast[2],ylast[2],chuteX,chuteY;
int     zlast[2],chuteZ,chuteDZ;
char    Txt[80];
int     MinAlt,CloseBase[2],OldClose=-1;
int     CloseDist[2];
UWORD   Speedy=1;
int     AutoLand=FALSE;
int     nextlog=0;

extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

extern  int		LLowFuelWarn;
extern  int		RLowFuelWarn;
extern  int		BingoFuelWarn;
extern  int     orgPlayernum;
extern confData ConfigData;
extern struct PlaneData far *GetPlanePtr(int PlaneNo);


//***************************************************************************
InitStealth()
{
    int i,d;

    detected = 0;
    NextWayPoint = 1;
    Tscale = 1;
    gtarg = -1;
    atarg = -1;

    FrameRate = 4;
    InitCannonRate();

    Rtime = rnd(4096) & 0x7ff8;
    Gtime = 0;
	TotalGameTicks = 0L;

    InitFlightModel();
    UpdateCloseBase();

    Help();
}

//***************************************************************************
Stealth()
{
    int i,pl,d;

    doradar();
	doplanes();
	doboats();
    dosams();

	doaaa();

	dodecoys();
	dotracers();
    dochute();

    if (detected) detected--;

    if (!(Rtime&7)) {
        UpdateCloseBase();
    }

	CarrierLanding();
	CheckLandingAndCrash();

    if ((QTimer&3)==0) {
        if (Gtime==1) {
            UpdateJetSnd();
        }
    }
}


//***************************************************************************
UpdateCloseBase()
{
    int OldClose;

    OldClose = CloseBase[0];
    FindCloseBase(Px, Py, 0);
}

//***************************************************************************
FindCloseBase(x, y, c)
{
    COORD xydist(), d;
    int i;

    CloseDist[c] = 0x7fff;

    for (i=NTARGET; i<NRADAR; i++) {
        if (Rdrs[i].status&(AIRBASE+CARRIER)
            &&   Rdrs[i].status&(FRIENDLY+NEUTRAL)) {

            d=xydist(x-Rdrs[i].x, y-Rdrs[i].y);
            if (d<CloseDist[c]) {
                CloseDist[c] = d;
                CloseBase[c] = i;
            }
        }
    }
}

//***************************************************************************
CheckLandingAndCrash()
{

	MinAlt=0;

    if ((RadarAlt<=0) || (OurAlt < 1)) {
		if ((Status&TRAINING) || (ConfigData.CrashMode==0)) {

			OurAlt  += 10;
			RadarAlt+= 10;
			if (OurAlt > 10) {
				OurAlt += 490;
				RadarAlt += 490;
			}

			Alt += 10/4;
			OurRoll = OurPitch = 0;
			MakeRotationMatrix (FM, OurHead,OurPitch,OurRoll);
			if (REALFLIGHT) {
				PxLng++;
				GetHPR(FPARAMS.MAT,&OurHead,&OurPitch,&OurRoll);
				MakeMatHPR( OurHead,0,0, FPARAMS.MAT);
				FPARAMS.VEL[1]=0;
			}
			return(1);
		}

		if (ConfigData.CrashMode==1) {
			Sound(N_LoudExplo,2,0);
			WaitTicks (60);
			LogIt (SLG_AIR_CRASH, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
			EndGame(0);
		}

		if (ConfigData.CrashMode==2) {
			Sound(N_LoudExplo,2,0);
			WaitTicks (60);
			LogIt (SLG_AIR_CRASH, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
  			EndGame(0);
		}
	}
}


//***************************************************************************
dropdecoy(int type, int local)
{
    int i,j;
    char *Str;

    j=-1;

    if (local == MAXPLANE) {
        if (Stores[type]-- <= 0) {
            Stores[type] = 0;
            if (type==FLARE)
	            Message("(RIO) Flares exhausted",RIO_MSG);
			else
	            Message("(RIO) Chaff exhausted",RIO_MSG);
            return 0;
        }
        Sound(N_DropChaff,2,0);
		lastCounter = Gtime + 3;
    }

    for (i=0; i<NDECOY; i++) {
        if (ds[i].time ==0) j=i;
    }

    if (j!=-1) {
        ds[j].type = type;

        if (local == MAXPLANE) {
	        ds[j].time = (15-Diff*3)*FrameRate;
            ds[j].x = Px;
            ds[j].y = Py;
			ds[j].alt = Alt;

            if (type==FLARE) {
                Str = "(RIO) Flare";
            } else {
                Str = "(RIO) Chaff";
            }

            TXT (Str);
            TXA (" released (");
            TXN (Stores[type]);
            TXA (" left)");
            Message(Txt,RIO_MSG);
        } else if(local){
	        ds[j].time = (12)*FrameRate;
            ds[j].x = ps[local].x;
            ds[j].y = ps[local].y;
			ds[j].alt = ps[local].alt;
        } else {
	        ds[j].time = (12)*FrameRate;
            ds[j].x = ps[0].x;
            ds[j].y = ps[0].y;
			ds[j].alt = ps[0].alt;
        }
    }
}

//***************************************************************************
#define FLAREDALT 3
#define CHAFFDALT 1
dodecoys()
{
    int i;

    for (i=0; i<NDECOY; i++) {
        if (ds[i].time) {
            -- ds[i].time;
            if (!ds[i].time) ds[i].type = 0;
            if (ds[i].type==FLARE) {
				if(ds[i].alt > FLAREDALT)
                	ds[i].alt -= FLAREDALT;
				else {
					ds[i].time = 0;
					ds[i].type = 0;
					ds[i].alt = 0;
				}
            } else {
				if(ds[i].alt > CHAFFDALT)
                	ds[i].alt -= CHAFFDALT;
				else {
					ds[i].time = 0;
					ds[i].type = 0;
					ds[i].alt = 0;
				}
            }
        }
    }
}


//***************************************************************************
dochute()
{
	int cnt;

    if (chuteZ>0) {
        if (chuteDZ>-16) chuteDZ -= 4;   /*  was 12  */
        chuteZ += chuteDZ;
    }

	for(cnt = 0; cnt < NUMPTROOPS; cnt ++) {
	    if (paratroopers[cnt].chuteZ>0) {
//	        if (paratroopers[cnt].chuteDZ>-16)
//				paratroopers[cnt].chuteDZ -=12;
	        if (paratroopers[cnt].chuteDZ>-1)
				paratroopers[cnt].chuteDZ -=1;
    	    paratroopers[cnt].chuteZ += paratroopers[cnt].chuteDZ;
		}
	}
}


//***************************************************************************
// Fix damage and restore weapons
Help()
{
    int i;

    Damaged = 0;

	MASTER_CAUTION=0;
	ARMT_INIT();

	Rounds = 675;

    Stores[CHAFF] = 18;
    Stores[FLARE] = 12;

    if (REALFLIGHT) {
		Fuel = 19800;  //16200 internal 3600 ext
    } else {
        Fuel = 19800;
    }

	LLowFuelWarn=0;
	RLowFuelWarn=0;
	BingoFuelWarn=0;
}

//***************************************************************************
JettisonFuel ()
{
	if (Fuel>16200) Fuel=16200;
}

//***************************************************************************
EndGame(int endcode)
{
//	if (ejected && endcode!=EJECTEND && endcode!=BADEJECT) return 0;
	BEND = TRUE;
/*
#if OLDLOGDATA
	LogIt(endcode, 0);
#else
	LogIt(endcode, 0, 0, PxLng, PyLng);
#endif  */
}

int GetSide(int planenum) {
	if(ps[planenum].status & FRIENDLY)
		return(SS_FRIENDLY);
	if(ps[planenum].status & NEUTRAL)
		return(SS_NEUTRAL);
	return(SS_ENEMY);
}
/*----------*/
/* Add entry to gamelog */
/*----------*/
#if OLDLOGDATA
//   OLD VERSION OF LOGIT, OLD DATA TYPE
LogIt (what, who, which)
{
    if (nextlog>=NLOG-1) DumpLogFile ();

    gamelog[nextlog].time  = Gtime;
    gamelog[nextlog].x     = (Px+64)>>7;
    gamelog[nextlog].y     = (Py+64)>>7;
    gamelog[nextlog].what  = what;
    gamelog[nextlog].who   = who;
    gamelog[nextlog].which = which;

    gamelog[++nextlog].what = ENDOFLOG;
}
#else
LogIt (int event, int donetonum, long x, long y, UWORD z, int missileOwner, int missileType, long misc, int src)
{
	int damagelevel = 0;
	struct PlaneData far *ptemp;
	int missileOrgOwner;

    if (nextlog>=NLOG-1) DumpLogFile ();

	if((missileOwner == -1) || (missileOwner == MAXPLANE))
		missileOrgOwner = orgPlayernum;
	else if((missileOwner >= 0) && (missileOwner < MAXPLANE))
		missileOrgOwner = bv[missileOwner].orgnum;
	else if((missileOwner < 0) && (missileOwner != -999))
		missileOrgOwner = -missileOwner;
	else
		missileOrgOwner = missileOwner;

	if((src == LOG_AIR) || (src == LOG_BOAT)) {
		if((donetonum >= 0) && (donetonum < MAXPLANE)) {
	    	gamelog[nextlog].objectNum  = (int)bv[donetonum].orgnum;
			ptemp = GetPlanePtr(ps[donetonum].type);
    		gamelog[nextlog].objectType  = ptemp->PlaneEnumId;
    		gamelog[nextlog].speed = ps[donetonum].speed;
    		gamelog[nextlog].side = GetSide(donetonum);

			if(ptemp->bclass == CIVILIAN_OBJCLASS)
				gamelog[nextlog].side = SS_CIVILIAN;

			if(donetonum > NPLANES) {
				gamelog[nextlog].damageLevel = bv[donetonum].verticlesep;
				if(gamelog[nextlog].damageLevel > 6)
					gamelog[nextlog].damageLevel = 6;
			}
			else {
				if(bv[donetonum].behaveflag & DAMAGERADAR)
					damagelevel += 2;
				if(bv[donetonum].behaveflag & DAMAGEENGINE)
					damagelevel += 2;
				if(bv[donetonum].behaveflag & DAMAGECONTROLSURFACE)
					damagelevel += 2;
				gamelog[nextlog].damageLevel = damagelevel;
			}
		}
		else if(donetonum == -1) {
	    	gamelog[nextlog].objectNum  = orgPlayernum;
    		gamelog[nextlog].objectType  = -1;
    		gamelog[nextlog].speed = Knots;
    		gamelog[nextlog].side = SS_FRIENDLY;
			gamelog[nextlog].damageLevel = 0;
		}
	}
	else {
		if(donetonum >= 0) {
    		gamelog[nextlog].objectNum  = donetonum;
		}
		else {
    		gamelog[nextlog].objectNum  = -999;
		}
	   	gamelog[nextlog].objectType  = -999;
   		gamelog[nextlog].speed = 0;
    	gamelog[nextlog].side = SS_ENEMY;
//    	gamelog[nextlog].side = GetSide(donetonum);

	}
    gamelog[nextlog].x     = x;
    gamelog[nextlog].y     = y;
    gamelog[nextlog].z     = (long)z;
    gamelog[nextlog].dramaticEvent  = event;
    gamelog[nextlog].missileOwner = missileOrgOwner;
    gamelog[nextlog].missileType = missileType;
    gamelog[nextlog].time = TimeOfDay;

	nextlog ++;

//    gamelog[++nextlog].what = ENDOFLOG;
}
#endif


/*----------*/
UpdateTime1 ()
{
    static UWORD OldTime=0;

    FTicks = GETTIME () - OldTime;
    OldTime += FTicks;

//    FTicks=4; /* uncomment this to peg frame rate */

    if (FTicks >1000) FTicks=1000;
    if (FTicks==0)    FTicks=1;

	TotalGameTicks += (unsigned long)FTicks;

    FrameRate10 = 600/FTicks;

    if (Speedy>1) FTicks *= Speedy;
}

/*----------*/
UpdateTime2 () {
    static TickAccum=0;

    if (FTicks<4) {                 /* Don't exceed 15 FPS */
        TickDelay = 4-FTicks;
        FTicks = 4;
    } else {
        TickDelay = 0;              /* Don't run any slower than 3 FPS */
        if (FTicks>20) FTicks=20;
    }

//    if (TickDelay!=0) WaitTicks (TickDelay); /* Delay, if machine is too fast */

    FrameRate = (60 + (FTicks>>1) ) /FTicks;
    TickAccum += FTicks;
    if (TickAccum>=15) {
        TickAccum -= 15;
        QTimer = QTime++;                   /* quarter-seconds */
        if ((QTimer&3)==0) {
            ++Gtime;                        /* seconds */
            if (Gtime%30 == 0) {
                TimeOfDay++;                /* minutes (goes 2X fast!) */
                if (TimeOfDay==24*60) TimeOfDay=0;
            }
        }
    } else {
        QTimer = -1;
    }

    ++Rtime;
}

