//***************************************************************************
//*
//*    AWG9.C
//*
//*    Author: Mike McDonald (adapted from Bill Beckers F15-III:APG-70)
//*
//*    Fleet Defender: F-14 Tomcat
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <bios.h>
#include <string.h>
#include <dos.h>
#include <setjmp.h>
#include "types.h"
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "sounds.h"
#include "f15defs.h"
#include "cockpit.h"
#include "math.h"
#include "awg9.h"
#include "armt.h"

//***************************************************************************
extern  unsigned		Alt;
extern  long            RadarAlt;
extern  int             OurRoll, OurPitch, OurHead, OurAlt;
extern  COORD           Px,Py;
extern  long            PxLng,PyLng;
extern  unsigned        FTicks;
extern  int             Knots;
extern  int             VM[3][3];
extern  SWORD           FM[3][3];
extern  int             LastMisl;
extern  int             View;
extern  int             TB_CTRX,TB_CTRY;
extern  int             DisplayFrame;
extern  int             MouseCrtX, MouseCrtY, MouseDsp;
extern  int             RadarCursorSprite;
extern  int             CrtDisplays[];
extern  RPS             *CrtRps[];
extern  char            Txt[];
extern long             LMul  (int  x, int y);
extern int              LDivI (long x, int y);

extern int DesignateMode;
extern int SelectedTarg;

long LDist3D(long x, long y, int z);
extern long absL(long x);
extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);


//***************************************************************************
static int ASE_Tbl[2][19] =
{
	{ 91,88,85,82,75,71,63,55,46,50,57,65,69,73,77,83,89,95,100 }, // radar
	{100,95,89,83,77,73,69,65,55,50,46,55,63,71,75,82,85,88,91  }, // ir
};

int		 SELECTED_BAR_NUM=BAR_2;
int		 SELECTED_AZ=DEG40;
int		 AWG9_REALITY_LEVEL=AWG9_AUTHENTIC;
int		 RIO_ASSIST=RIO_ASSIST_AUTHENTIC;
unsigned AWG9_MODE=0;
int      MAXRNG=5;
int		 MINRNG=0;
int      RANGES[] =  { 5, 10, 20, 50, 100, 200 };
int      BRANGES[]=  { 238, 475, 950, 2375, 4750, 9500 }; // in 15-bit scale (1 bit == 4*32 ft)
int      RANGE=5;

int      DESIGNATED;
int      LOCKED;

unsigned PDT_ALT_1000;
unsigned PDT_ALT_100;
int      PDT_RNG;
char     ASPECT_S[5];
int      ASE_RADIUS;
int      SHOOT;
int      PW;
int      PDT_PROJ_X;
int      PDT_PROJ_Y;
int      PDT_PROJ;
int      STEER_DOT_X;
int      STEER_DOT_Y;
int      STEER_DOT;
int      TTI;
int      CLOSURE;
int      AWG9_SNIFF;
int      LOCKSHOOT=1;
int      IFF_timer;
int      proj_x, proj_y, proj_z;

static struct {
   int   deg;
   int   wdeg;
} Angles[] =
{
	{ 65, 0x2e36 },		// 65 degrees
	{ 40, 0x1c70 },		// 40 degrees
	{ 20, 0x0E38 },		// 20 degrees
	{ 10, 0x071c },		// 10 degrees
	{  5, 0x038e },		//  5 degrees
	{  3, 0x0222 },		//  3 degrees
};

extern int TIDTargetCnt;
extern TIDTarg TIDTargetList[];

int DesigTargCnt=0;
DTARGET DESIG_TARG[6];

TARGET  DESIG_TARGF;
TARGET  PREV_DESIG_TARGF;

TARGET  TARGFILE0[MAXTARGFIL];
TARGET	TARGFILE1[MAXTARGFIL];
TARGET	*TARGFILE;
TARGET	*AGEDTARGFILE;
int		TARGFILE0CNT;
int		TARGFILE1CNT;
int		*TARGFILECNT;
int		*AGEDTARGFILECNT;

TARGET *ACQLIST[MAXTARGFIL*2];
int ACQLISTCNT;

TARGET *FIRINGORDERLIST[MAXTARGFIL*2];
int FIRINGORDERCNT;

int DETECTED_TARG[MAXPLANE];
int DataLinkList[8];
int DataLinkCnt=0;

int     AZSWEEP;				// Azimuth sweep position
int     AZSWEEP_DIR;			// Azimuth sweep direction
int     CUR_BAR;				// Current bar position
int		DataLink=0;				// DataLink - Default Off

static int ACQ_X;
static int ACQ_Y;

int BEAM_HDG;					// Beam Heading
int BEAM_PIT;					// Beam Pitch
int	BEAM_ROL;					// Beal Roll

static int     STABLE_HDG;
static int     STABLE_PIT;

static int     BEAM_M[9];
       int     PLYR_M[9];

static int     CVG_MAX;
static int     CVG_MIN;

static int     KNOTS;
static int     ASPDEG;
static int     break_mode;

int BAD_GUY;
int	NumBars=0;
int AZSweep=0;
int ModerateMode=0;
int InRangeFlash=0;

// THIS IS NOT MINE!!! I DID NOT DO THIS!!! I PLEDGE TO RIP THE HEART
// FROM THE INDIVIDUAL WHO DID!  THIS IS JUST A WORSE VERSION OF GOTO!
// I WILL CHANGE THIS AT SOME LATER DATE - DON'T MESS WITH IT, DON'T CHANGE
// IT!  THIS MEANS YOU!!! Thanks for your support - MJM
static jmp_buf resetmark;


//***************************************************************************
int AWG9()
{
	setjmp(resetmark);
	InitMode();

	set_acq();
	CheckAZELChange();
	UpdateBeam();
	TargetTrack();
	UpdateSweep();
	SearchForTargets();
	if (AWG9_REALITY_LEVEL != AWG9_EASY) SpecialModeAcq();
	if (RIO_ASSIST == RIO_ASSIST_STANDARD) RioLockCheck();
	if ((AWG9_REALITY_LEVEL != AWG9_EASY) && DataLink) CheckDataLink();
	FiringOrder();
	UpdateTWSSelected();
	DetermineShootParams();
	Aim9Sound();
}


//***************************************************************************
int InitTWSData()
{
	int i;
	int cnt=0;

	DesigTargCnt=0;
	SelectedTarg=-1;

	for (i=0; i<F14_STATION_NUM; i++) {
		if (F14_STATION[i] == W_AIM_54) {
			DESIG_TARG[cnt].status=0;
			DESIG_TARG[cnt].target=-1;
			DESIG_TARG[cnt].ssi_index=-1;
			DESIG_TARG[cnt].src_station=i;
			cnt++;
		}
	}
}

//***************************************************************************
int GetWeaponCount(int type)
{
	int cnt=0, i;

	for(i=0;i<F14_STATION_NUM;i++) {
		if (F14_STATION[i] == type) cnt++;
	}
	return cnt;
}


//***************************************************************************
static mode_flip(mode)
{
	AWG9_CHANGE_MODE(mode);
	longjmp(resetmark,-1);
}

//***************************************************************************
int InitMode()
{
	MakeMatHPR(-OurHead,OurPitch,0,PLYR_M);
	memcpy (VM, FM, sizeof(FM));

	PW = ARMT_get_priority();

	if (IFF_timer) IFF_timer--;
	if (BAD_GUY) BAD_GUY--;

	if (LOCKED == -1) return 0;

	if ((AWG9_MODE == AWG9_STANDARD_SRCH) || (AWG9_MODE == AWG9_STANDARD_TRACK)) {
		StandardReset();
		return 0;
	}
}



//***************************************************************************
int AWG9_ACQ(int flag)
{
	aa_acquire(flag);
}

//***************************************************************************
int AWG9_MODE_BREAK()
{
	// If you are not currently locked or tracking then return
	if ((AWG9_MODE != AWG9_PDSTT) &&
		(AWG9_MODE != AWG9_STANDARD_TRACK)) return 0;

	// Just to be sure
	if (LOCKED == -1) {
		return 0;					// not locked
	}

	switch (AWG9_MODE) {
		case AWG9_PDSTT:
			DESIGNATED=-1;
			LOCKED=-1;
			AWG9_CHANGE_MODE(AWG9_BREAK);		// chg to search mode
		break;
		case AWG9_STANDARD_TRACK:
			DESIGNATED=-1;
			LOCKED=-1;
			AWG9_CHANGE_MODE(AWG9_BREAK);
		break;
	}
}

//***************************************************************************
int AWG9_EL(int dir)
{
	// Cannot change elevation while locked or tracking

	if ((AWG9_MODE == AWG9_PDSTT) ||
		(AWG9_MODE == AWG9_STANDARD_TRACK)) return 0;

	if (AWG9_MODE==AWG9_BORESIGHT) return 0;
	if (AWG9_MODE==AWG9_VSL) return 0;

	if (dir) {
		if (BEAM_PIT+EL_DELTA*DEG >= 60*DEG) return 0;
		BEAM_PIT += EL_DELTA*DEG;
	} else {
		if (BEAM_PIT-EL_DELTA*DEG <= -60*DEG) return 0;
		BEAM_PIT -= EL_DELTA*DEG;
	}
}

//***************************************************************************
int UpdateTWSSelected()
{
	int found=-1;
	int i;
	TARGET t;
	int asp;

	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM)) {
		if (SelectedTarg==-1) return(0);
		for (i=0; i < *TARGFILECNT; i++) {
			if (TARGFILE[i].i == SelectedTarg) {
				found=1;
				break;
			}
		}
		for (i=0; i < *AGEDTARGFILECNT; i++) {
			if (AGEDTARGFILE[i].i == SelectedTarg) {
				found=1;
				break;
			}
		}
	   	for (i=0; i<DataLinkCnt; i++) {
			if (DataLinkList[i] == SelectedTarg) {
				found=2;
			}
		}

		if (found==-1) return(0);

		SetupNewTarget(&t,SelectedTarg);
		orient_to_player(&t);
		orient_to_beam(&t);

		PDT_RNG = (t.conv)?t.d:t.d>>HTOV;

		// Force range displays
		if (PDT_RNG <= BRANGES[0])
			RANGE=0;

		if ((PDT_RNG > BRANGES[0]) && (PDT_RNG < BRANGES[1]))
			RANGE=1;

		if ((PDT_RNG > BRANGES[1]) && (PDT_RNG < BRANGES[2]))
			RANGE=2;

		if ((PDT_RNG > BRANGES[2]) && (PDT_RNG < BRANGES[3]))
			RANGE=3;

		if ((PDT_RNG > BRANGES[3]) && (PDT_RNG < BRANGES[4]))
			RANGE=4;

		if ((PDT_RNG > BRANGES[4]) && (PDT_RNG < BRANGES[5]))
			RANGE=5;

		if (PDT_RNG > BRANGES[5])
			RANGE=5;

		// build aspect angle string

		asp    = angle (Px-ps[SelectedTarg].x, -(Py-ps[SelectedTarg].y));
		asp   -= ps[SelectedTarg].head;
		asp   += 0x8000;
		ASPDEG = (abs(asp)/DEG+5)/10;

		switch (ASPDEG) {
			case 0 :
				strcpy(ASPECT_S,"T");
			break;
			case 18:
				strcpy(ASPECT_S,"H");
			break;
			default:
				strcpy(ASPECT_S,itoa(ASPDEG,Txt,10));
            	strcat(ASPECT_S,(asp > 0)?"L":"R");
			break;
		}


		if (AWG9_project(ps[SelectedTarg].xL,ps[SelectedTarg].yL,ps[SelectedTarg].alt)==-1)
			return 0;

		// projection point of target relative to ASE circle center
		PDT_PROJ_X = proj_x - TB_CTRX;
		PDT_PROJ_Y = proj_y - TB_CTRY;
		PDT_PROJ++;
	}
}

//***************************************************************************
int AWG9_IFF()
{
	int i;

	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM)) {
		if (SelectedTarg==-1) return(0);
		if (ps[SelectedTarg].status & FRIENDLY) {
 			IFF_timer = FrameRate * 2;
			Sound (N_IFFWarning, 1,0);

			for (i=0; i < *TARGFILECNT; i++) {
				if (TARGFILE[i].i == SelectedTarg) {
					TARGFILE[i].id = 1;
					break;
				}
			}
			for (i=0; i < *AGEDTARGFILECNT; i++) {
				if (AGEDTARGFILE[i].i == SelectedTarg) {
					AGEDTARGFILE[i].id = 1;
					break;
				}
			}
		} else {
			BAD_GUY = FrameRate * 2;
			for (i=0; i < *TARGFILECNT; i++) {
				if (TARGFILE[i].i == SelectedTarg) {
					TARGFILE[i].id = 2;
					break;
				}
			}
			for (i=0; i < *AGEDTARGFILECNT; i++) {
				if (AGEDTARGFILE[i].i == SelectedTarg) {
					AGEDTARGFILE[i].id = 2;
					break;
				}
			}
		}
		return 0;
	}

	// If no one is designated, then you can't IFF somebody!
	if (DESIGNATED == -1) return 0;

	if (ps[DESIGNATED].status & FRIENDLY) {
		IFF_timer = FrameRate * 2;
		Sound (N_IFFWarning, 1,0);
		if ((AWG9_MODE==AWG9_STANDARD_TRACK) || (AWG9_MODE == AWG9_PDSTT)) {
			DESIG_TARGF.id = 1;
			if (PREV_DESIG_TARGF.i == DESIG_TARGF.i)
				PREV_DESIG_TARGF.id = 1;

		}
		for (i=0; i < *TARGFILECNT; i++) {
			if (TARGFILE[i].i == DESIGNATED) {
				TARGFILE[i].id = 1;
				break;
			}
		}
		for (i=0; i < *AGEDTARGFILECNT; i++) {
			if (AGEDTARGFILE[i].i == DESIGNATED) {
				AGEDTARGFILE[i].id = 1;
				break;
			}
		}
	} else {
		BAD_GUY = FrameRate * 2;
		if ((AWG9_MODE==AWG9_STANDARD_TRACK) || (AWG9_MODE == AWG9_PDSTT)) {
			DESIG_TARGF.id = 2;
			if (PREV_DESIG_TARGF.i == DESIG_TARGF.i)
				PREV_DESIG_TARGF.id = 2;
		}
		for (i=0; i < *TARGFILECNT; i++) {
			if (TARGFILE[i].i == DESIGNATED) {
				TARGFILE[i].id = 2;
				break;
			}
		}
		for (i=0; i < *AGEDTARGFILECNT; i++) {
			if (AGEDTARGFILE[i].i == DESIGNATED) {
				AGEDTARGFILE[i].id = 2;
				break;
			}
		}
	}
}

//***************************************************************************
int AWG9_INIT()
{
	InitTWSData();

	if (Alt > 20) {
		AWG9_SNIFF = 0;
	} else {
		if (AWG9_REALITY_LEVEL==AWG9_EASY)
			AWG9_SNIFF = 0;
		else
			AWG9_SNIFF = 1;
	}

	switch(AWG9_REALITY_LEVEL) {
		case AWG9_EASY:
			AWG9_CHANGE_MODE(AWG9_STANDARD_SRCH);
		break;
		case AWG9_MODERATE:
			AWG9_CHANGE_MODE(AWG9_SRCH_WIDE);
		break;
		case AWG9_AUTHENTIC:
			AWG9_CHANGE_MODE(AWG9_SRCH_AUTHENTIC);
		break;
	}

	SelectedTarg=-1;
	LOCKSHOOT = 0;
	AZSWEEP = 0;
	AZSWEEP_DIR = 0;
	IFF_timer = 0;
	BAD_GUY = 0;
	if (AWG9_MODE != AWG9_STANDARD_SRCH) ResetBar();
}

//***************************************************************************
int AWG9_CHANGE_MODE(int mode)
{
	int      reset;
	static   srch=AWG9_SRCH_WIDE;
	static int SpecialMode=0;

	if (AWG9_MODE == mode) return 0;

	// Clean previous track file
	PREV_DESIG_TARGF.lastdist=0L;
	PREV_DESIG_TARGF.targetclosure=0L;
	PREV_DESIG_TARGF.lasttime=0;
	PREV_DESIG_TARGF.id=0;

	InitTWSData();

	if (AWG9_REALITY_LEVEL == AWG9_EASY) {
		if ((mode != AWG9_STANDARD_TRACK) && (mode != AWG9_STANDARD_SRCH) && (mode != AWG9_BREAK))
			mode = AWG9_STANDARD_SRCH;
	}

	// check for special modes

	switch (mode) {
		case AWG9_RWS:
			if (AWG9_REALITY_LEVEL==AWG9_MODERATE) {
				// Moderate Mode - Change between preset modes
				switch (ModerateMode) {
					case 0:
						mode = AWG9_RWS_NARROW;
					break;
					case 1:
						mode = AWG9_RWS_MEDIUM;
					break;
					case 2:
						mode = AWG9_RWS_WIDE;
					break;
				}
			}
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				mode = AWG9_RWS_AUTHENTIC;
			}
		break;

		case AWG9_PDSRCH:
			if (AWG9_REALITY_LEVEL==AWG9_MODERATE) {
				// Moderate Mode - Change between preset modes
				switch (ModerateMode) {
					case 0:
						mode = AWG9_SRCH_NARROW;
					break;
					case 1:
						mode = AWG9_SRCH_MEDIUM;
					break;
					case 2:
						mode = AWG9_SRCH_WIDE;
					break;
				}
			}
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				mode = AWG9_SRCH_AUTHENTIC;
			}
   		break;
		case AWG9_TWSA:
			if (AWG9_REALITY_LEVEL==AWG9_MODERATE) {
				if (ModerateMode==2) ModerateMode=0;
				switch(ModerateMode) {
					case 0:
						mode = AWG9_TWSA_NARROW;
					break;
					case 1:
						mode = AWG9_TWSA_MEDIUM;
					break;
				}
			}
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				mode = AWG9_TWSA_AUTHENTIC;
			}
		break;

		case AWG9_TWSM:
			if (AWG9_REALITY_LEVEL==AWG9_MODERATE) {
				if (ModerateMode==2) ModerateMode=0;
				switch(ModerateMode) {
					case 0:
						mode = AWG9_TWSM_NARROW;
					break;
					case 1:
						mode = AWG9_TWSM_MEDIUM;
					break;
				}
			}
			if (AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) {
				mode = AWG9_TWSM_AUTHENTIC;
			}
		break;

		case AWG9_SPECIAL:
			SpecialMode ^= 1;
			switch (SpecialMode) {
				case 0:
					mode = AWG9_VSL;
				break;
				case 1:
					mode = AWG9_BORESIGHT;
				break;
			}
		break;

		case AWG9_BREAK:
			if (AWG9_REALITY_LEVEL == AWG9_EASY)
				break_mode = AWG9_STANDARD_SRCH;
			mode = break_mode;
		break;
	}

	reset = FALSE;
	AWG9_MODE = mode;

	switch (AWG9_MODE) {
		case AWG9_STANDARD_SRCH:
			reset = TRUE;
			break_mode = AWG9_MODE;
			DESIGNATED =
			LOCKED = -1;
			BEAM_HDG = 0;
			SELECTED_AZ = DEG65;
			SELECTED_BAR_NUM = BAR_2;
		break;

		case AWG9_STANDARD_TRACK:
			reset = TRUE;
			LOCKED = DESIGNATED;
		break;

		case AWG9_SRCH_WIDE:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG65;
			SELECTED_BAR_NUM = BAR_2;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_SRCH_MEDIUM:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_4;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_SRCH_NARROW:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG20;
			SELECTED_BAR_NUM = BAR_8;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_RWS_WIDE:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG65;
			SELECTED_BAR_NUM = BAR_2;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_RWS_MEDIUM:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_4;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_RWS_NARROW:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG20;
			SELECTED_BAR_NUM = BAR_8;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_RWS_AUTHENTIC:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_SRCH_AUTHENTIC:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSA_AUTHENTIC:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSA_MEDIUM:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_2;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSA_NARROW:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG20;
			SELECTED_BAR_NUM = BAR_4;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSM_AUTHENTIC:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSM_MEDIUM:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_2;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSM_NARROW:
			reset      = TRUE;
			srch       =
			break_mode = AWG9_MODE;
			SELECTED_AZ = DEG20;
			SELECTED_BAR_NUM = BAR_4;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_VSL:
			reset      = TRUE;
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0x1C70;  // 40 deg.
      		DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_BORESIGHT:
			reset      = TRUE;
			break_mode = AWG9_MODE;
			BEAM_HDG   = 0;
			DESIGNATED =
			LOCKED     = -1;
		break;

		case AWG9_TWSM:
			reset = TRUE;
			DESIGNATED = -1;
			LOCKED = -1;
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_2;
			STABLE_HDG = OurHead;
			STABLE_PIT = OurPitch;
			BEAM_HDG = (((ACQ_X - DDD_CTR_X) * 120)/DDD_W) * DEG;
		break;

		case AWG9_TWSA:
			// Determine where the most threats are
			reset = TRUE;
			DESIGNATED = -1;
			LOCKED = -1;
			MakeMatHPR(-OurHead,OurPitch,0,PLYR_M);
			SetupNewTarget(&DESIG_TARGF, DESIGNATED);
			orient_to_player(&DESIG_TARGF);
			SELECTED_AZ = DEG40;
			SELECTED_BAR_NUM = BAR_2;
			STABLE_HDG = ARCTAN (DESIG_TARGF.rx,DESIG_TARGF.rz);
			STABLE_PIT = -ARCTAN (DESIG_TARGF.ry,DESIG_TARGF.rz);
			BEAM_HDG = (((ACQ_X - DDD_CTR_X) * 120)/DDD_W) * DEG;
		break;

		case AWG9_PDSTT:
			reset = TRUE;
			LOCKED = DESIGNATED;
		break;
	}


	// Check Current Beam pitch - if greater than bar-reset
	if ((abs(BEAM_PIT)/DEG) > ((SELECTED_BAR_NUM*BAR_SIZE)/DEG)) {
//		sprintf(Txt,"BEAM PITCH: %i  NEW BAR SETTING: %i",(abs(BEAM_PIT)/DEG),
//			((SELECTED_BAR_NUM*BAR_SIZE)/DEG));
//		Message(Txt);
		BEAM_PIT=0;
	}

	// reset target files
	if (reset) {
		ClearTargetFile();
		acquire(DDD_CTR_X,DDD_CTR_Y);    // center acq symbol
	}

	// This stuff should be able to co-exist with the easy mode
	if (abs(AZSWEEP) > Angles[SELECTED_AZ].deg)
		AZSWEEP = (AZSWEEP>0)? Angles[SELECTED_AZ].deg:-Angles[SELECTED_AZ].deg;
	if (CUR_BAR>SELECTED_BAR_NUM)
		CUR_BAR=(SELECTED_BAR_NUM-1);
}

//***************************************************************************
int CheckAZELChange()
{
	static int prevaz=BAR_2;
	static int prevbar=DEG40;

	if ((prevaz!=SELECTED_AZ) || (prevbar!=SELECTED_BAR_NUM)) {
// YOU STUPID FUCK!!!
//		ClearTargetFile();
		acquire(DDD_CTR_X,DDD_CTR_Y);    // center acq symbol

		// check if radar outside new limits

		if (abs(AZSWEEP) > Angles[SELECTED_AZ].deg)
			AZSWEEP = (AZSWEEP>0)? Angles[SELECTED_AZ].deg:-Angles[SELECTED_AZ].deg;

		// PROBLEM
		if (CUR_BAR>SELECTED_BAR_NUM)
			CUR_BAR=(SELECTED_BAR_NUM-1);

		if (prevaz!=SELECTED_AZ)
			prevaz=SELECTED_AZ;
		if (prevbar!=SELECTED_BAR_NUM)
			prevbar=SELECTED_BAR_NUM;
	}
}



//***************************************************************************
int ClearTargetFile()
{
	TARGFILE0CNT    = 0;
	TARGFILE1CNT    = 0;

	TARGFILECNT     = &TARGFILE0CNT;
	AGEDTARGFILECNT = &TARGFILE1CNT;

	TARGFILE        = TARGFILE0;
	AGEDTARGFILE    = TARGFILE1;
}

//***************************************************************************
int CheckMode(int mode, int mastermode)
{
	switch(mastermode) {
		case AWG9_TWSA:
			if ((AWG9_MODE==AWG9_TWSA_AUTHENTIC)
				|| (AWG9_MODE==AWG9_TWSA_MEDIUM)
				|| (AWG9_MODE==AWG9_TWSA_NARROW)) return 1;
		break;
		case AWG9_TWSM:
			if ((AWG9_MODE==AWG9_TWSM_AUTHENTIC)
				|| (AWG9_MODE==AWG9_TWSM_MEDIUM)
				|| (AWG9_MODE==AWG9_TWSM_NARROW)) return 1;
		break;
		case AWG9_RWS:
			if ((AWG9_MODE==AWG9_RWS_AUTHENTIC)
				|| (AWG9_MODE==AWG9_RWS_WIDE)
				|| (AWG9_MODE==AWG9_RWS_MEDIUM)
				|| (AWG9_MODE==AWG9_RWS_NARROW)) return 1;
	}
	return 0;
}

//***************************************************************************
int AWG9_LOCK()
{
	int xmin,xmax,ymin,ymax,i,new=-1;
	TARGET *t;

	if ((AWG9_MODE == AWG9_STANDARD_SRCH) || (AWG9_MODE == AWG9_STANDARD_TRACK)) {
		AWG9_ACQ(0);
		return 0;
	}

	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM)) {
		// Designate targets
		return 0;
	}

	if (DESIGNATED == -1) return 0;   // no designated target
	LOCKED = DESIGNATED;

	AWG9_CHANGE_MODE(AWG9_PDSTT);
}


//***************************************************************************
int ChangeModerateScan(int mode)
{
	static int srchmode=0;

	switch(AWG9_MODE) {
		case AWG9_TWSA_NARROW:
		case AWG9_TWSA_MEDIUM:
		case AWG9_TWSM_NARROW:
		case AWG9_TWSM_MEDIUM:
			if (srchmode==2) srchmode=0;
		break;
	}

	switch(srchmode) {
		case 0:  // Narrow to medium
			ModerateMode=1;
		break;
		case 1:	 // Medium to wide
			ModerateMode=2;
		break;
		case 2:	 // Wide to narrow
			ModerateMode=0;
		break;
	}

	switch(AWG9_MODE) {
		case AWG9_TWSA_NARROW:
		case AWG9_TWSA_MEDIUM:
			if (srchmode)
				ModerateMode=0;
			else
				ModerateMode=1;
			AWG9_CHANGE_MODE(AWG9_TWSA);
		break;
		case AWG9_TWSM_NARROW:
		case AWG9_TWSM_MEDIUM:
			if (srchmode)
				ModerateMode=0;
			else
				ModerateMode=1;
			AWG9_CHANGE_MODE(AWG9_TWSM);
		break;
		case AWG9_SRCH_NARROW:
		case AWG9_SRCH_MEDIUM:
		case AWG9_SRCH_WIDE:
			AWG9_CHANGE_MODE(AWG9_PDSRCH);
		break;
		case AWG9_RWS_NARROW:
		case AWG9_RWS_MEDIUM:
		case AWG9_RWS_WIDE:
			AWG9_CHANGE_MODE(AWG9_RWS);
		break;
	}
	srchmode=ModerateMode;
}



//***************************************************************************
int ChangeSRCHMode(int mode)
{
	static int srchmode=0;
	int tempmode;

	if (mode==-1) {
		switch(srchmode) {
			case 0:
				AWG9_CHANGE_MODE(AWG9_PDSRCH);
				tempmode=1;
			break;
			case 1:
				AWG9_CHANGE_MODE(AWG9_TWSM);
				tempmode=2;
			break;
			case 2:
				AWG9_CHANGE_MODE(AWG9_TWSA);
				tempmode=3;
			break;
			case 3:
				AWG9_CHANGE_MODE(AWG9_RWS);
				tempmode=0;
			break;
		}
		srchmode=tempmode;
	}

	if (mode==0) {
		AWG9_CHANGE_MODE(AWG9_PDSRCH);
		srchmode=1;
	}
	if (mode==1) {
		AWG9_CHANGE_MODE(AWG9_TWSM);
		srchmode=2;
	}
	if (mode==2) {
		AWG9_CHANGE_MODE(AWG9_TWSA);
		srchmode=3;
	}
	if (mode==3) {
		AWG9_CHANGE_MODE(AWG9_RWS);
		srchmode=0;
	}
}

//***************************************************************************
int AWG9_POWERUP()
{
	int bh;

	if (!(AWG9_SNIFF ^= 1)) {
		Message("(RIO) Radar Active", RIO_NOW_MSG);
	} else {
		Message("(RIO) Radar Inactive", RIO_NOW_MSG);
		return 0;
	}

	LOCKED = -1;

	ClearTargetFile();

	switch (AWG9_MODE) {
		case AWG9_PDSTT:
			AWG9_CHANGE_MODE(AWG9_PDSRCH);
		break;
		case AWG9_TWSA:
			// Check for ThreatArea, if none then look straight ahead
			bh = BEAM_HDG;
//			AWG9_CHANGE_MODE(AWG9_TWSM);
			BEAM_HDG = bh;             // preserve beam heading
		default:
		break;
	}
}

//***************************************************************************
int CHANGE_NUM_BARS()
{
	int bartemp;

	switch(SELECTED_BAR_NUM) {
		case BAR_1:
			bartemp = BAR_2;
		break;
		case BAR_2:
			bartemp = BAR_4;
		break;
		case BAR_4:
			bartemp = BAR_8;
		break;
		case BAR_8:
			bartemp = BAR_1;
		break;
	}
	SELECTED_BAR_NUM=bartemp;
}

//***************************************************************************
int CHANGE_AZIMUTH()
{
	int aztemp;

	switch(SELECTED_AZ) {
		case DEG10:
			aztemp = DEG20;
		break;
		case DEG20:
			aztemp = DEG40;
		break;
		case DEG40:
			aztemp = DEG65;
		break;
		case DEG65:
			aztemp = DEG10;
		break;
	}
	SELECTED_AZ=aztemp;
}


//***************************************************************************
int AWG9_INC_RANGE()
{
	RANGE++;
	if (RANGE >= MAXRNG) RANGE=MAXRNG;
}

//***************************************************************************
int AWG9_DEC_RANGE()
{
	RANGE--;
	if (RANGE <= 0) RANGE=MINRNG;
}

//***************************************************************************
static set_acq()
{
	int x,y;
	static int MX=-1;

	if ((AWG9_MODE == AWG9_PDSTT) || (AWG9_MODE == AWG9_STANDARD_TRACK))
		return 0;

	// is cursor active in radar display?

	if ((MouseCrtX != -1) && (CrtDisplays[MouseDsp] == DDD)) {
		x = MouseCrtX - DDD_X - 1;
		y = MouseCrtY - DDD_Y - 1;

		// test if valid...
		if ((x < 0) || (y < 0) || (x > DDD_W) || (y > DDD_H)) {
			acquire(DDD_CTR_X,DDD_CTR_Y);    // center acq symbol
			return 0;
		}
		acquire(x,y);
	} else if ((MouseCrtX == -1) && (MX != -1))
		acquire(DDD_CTR_X,DDD_CTR_Y);    // center acq symbol

	MX = MouseCrtX;
}

//***************************************************************************
int UpdateBeam()
{
	int  pts[2][2];
	long alt;
	int found=0;
	int sr;
	static int prevdesig;

	// No designated target
	if (DESIGNATED == -1) return(0);

	SetupNewTarget(&DESIG_TARGF, DESIGNATED);
	orient_to_player(&DESIG_TARGF);
	orient_to_beam(&DESIG_TARGF);

	DESIG_TARGF.az = ARCTAN(DESIG_TARGF.rx,DESIG_TARGF.rz);    // target azimuth   angle
	DESIG_TARGF.el =-ARCTAN(DESIG_TARGF.ry,DESIG_TARGF.rz);    // target elevation angle

	if (DESIGNATED == PREV_DESIG_TARGF.i) {
		DESIG_TARGF.lastdist = PREV_DESIG_TARGF.lastdist;
		DESIG_TARGF.targetclosure = PREV_DESIG_TARGF.targetclosure;
		DESIG_TARGF.lasttime = PREV_DESIG_TARGF.lasttime;
		DESIG_TARGF.id = PREV_DESIG_TARGF.id;
	}

	UpdateConsoleXY(&DESIG_TARGF);

	switch (AWG9_MODE) {
		case AWG9_STANDARD_SRCH:
		case AWG9_PDSRCH:
		case AWG9_SRCH_AUTHENTIC:
			return(1);
		break;

		case AWG9_STANDARD_TRACK:
		break;

		case AWG9_TWSA:
			// keep beam on designated target
			// Center beam in area of most threats
			// So determine where the most threats are coming from
		break;

		case AWG9_PDSTT:
			BEAM_HDG += DESIG_TARGF.az - STABLE_HDG;
			BEAM_PIT -= DESIG_TARGF.el - STABLE_PIT;
			STABLE_HDG = DESIG_TARGF.az;
			STABLE_PIT = DESIG_TARGF.el;
		break;

		case AWG9_TWSM:
			// space stabilized beam
			BEAM_HDG -= OurHead  - STABLE_HDG;
			BEAM_PIT -= OurPitch - STABLE_PIT;
			STABLE_HDG = OurHead;
			STABLE_PIT = OurPitch;
		break;

		default:
		break;
	}

	if (AWG9_MODE == AWG9_STANDARD_TRACK) return 0;

	MakeMatHPR(-(OurHead+BEAM_HDG), OurPitch+BEAM_PIT, 0, BEAM_M);

	// test gimbal limits

	if (((abs(BEAM_HDG)/DEG)-Angles[SELECTED_AZ].deg >= 65) ||
		(((abs(BEAM_PIT)/DEG)-((SELECTED_BAR_NUM*BAR_SIZE)/DEG)) >= 65))
		mode_flip(AWG9_BREAK);
}

//***************************************************************************
int UpdateSweep()
{
	int   deg, ticks;

	ticks = FTicks*7/6;                       // approx 70 deg/sec
	deg   = (AZSWEEP_DIR) ? ticks : -ticks;

	if (abs(AZSWEEP + deg) > Angles[SELECTED_AZ].deg) {
		AZSWEEP_DIR ^= 1;
		deg = -deg;
		if (++CUR_BAR == SELECTED_BAR_NUM) ResetBar();
	}
	AZSWEEP += deg;
}

//***************************************************************************
int StandardReset()
{
	int i;
	for (i = 0; i < NPLANES; i++)
		DETECTED_TARG[i] = 0;

	ClearTargetFile();
}

//***************************************************************************
int TargetTrack()
{
	int asp, b_az, b_el;
	int found,sr;

	PDT_PROJ = 0;
	TTI = -1;

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE != AWG9_STANDARD_TRACK))
		return 0;

	// No Designated Target
	if (DESIGNATED == -1) return(0);

	// Target Still alive? Active?
	if (!(ps[DESIGNATED].status & ACTIVE) || !ps[DESIGNATED].speed)
		mode_flip(AWG9_BREAK);

	// Distance Check - Too Close - Break Lock
	if (!DESIG_TARGF.conv && (DESIG_TARGF.d <= FT500))
		mode_flip(AWG9_BREAK);

	if (!LockDistanceCheck(&DESIG_TARGF)) {
		mode_flip(AWG9_BREAK);
	}

	// Target within gimbal limits - Azimuth
	if (abs(DESIG_TARGF.az) > Angles[DEG65].wdeg)
		mode_flip(AWG9_BREAK);

	// Target within gimbal limits - Elevation
	if (abs(DESIG_TARGF.el) > Angles[DEG65].wdeg)
		mode_flip(AWG9_BREAK);

	// Erase target track and start a new one - why, because Bill is a Bozo &
	// a Butthead

	ResetTargetTrack();
	UpdateTargetClosure(&DESIG_TARGF);
	PREV_DESIG_TARGF.lastdist = DESIG_TARGF.lastdist;
	PREV_DESIG_TARGF.targetclosure = DESIG_TARGF.targetclosure;
	PREV_DESIG_TARGF.lasttime = DESIG_TARGF.lasttime;
	PREV_DESIG_TARGF.id = DESIG_TARGF.id;

	// Ok, Target passed track conditions, start tracking target
	AddTargetFile(&DESIG_TARGF);
	PDT_RNG = (DESIG_TARGF.conv)?DESIG_TARGF.d:DESIG_TARGF.d>>HTOV;

	CLOSURE = (int)DESIG_TARGF.targetclosure;

	// Force range displays
	if (PDT_RNG <= BRANGES[0])
		RANGE=0;

	if ((PDT_RNG > BRANGES[0]) && (PDT_RNG < BRANGES[1]))
		RANGE=1;

	if ((PDT_RNG > BRANGES[1]) && (PDT_RNG < BRANGES[2]))
		RANGE=2;

	if ((PDT_RNG > BRANGES[2]) && (PDT_RNG < BRANGES[3]))
		RANGE=3;

	if ((PDT_RNG > BRANGES[3]) && (PDT_RNG < BRANGES[4]))
		RANGE=4;

	if ((PDT_RNG > BRANGES[4]) && (PDT_RNG < BRANGES[5]))
		RANGE=5;

	if (PDT_RNG > BRANGES[5])
		RANGE=5;

	// build aspect angle string

	asp    = angle (Px-ps[DESIGNATED].x, -(Py-ps[DESIGNATED].y));
	asp   -= ps[DESIGNATED].head;
	asp   += 0x8000;
	ASPDEG = (abs(asp)/DEG+5)/10;

	switch (ASPDEG) {
		case 0 :
			strcpy(ASPECT_S,"T");
		break;
		case 18:
			strcpy(ASPECT_S,"H");
		break;
		default:
			strcpy(ASPECT_S,itoa(ASPDEG,Txt,10));
            strcat(ASPECT_S,(asp > 0)?"L":"R");
		break;
	}

	if (LOCKED == -1) return 0;

	// Target is currently locked, calculate TTI and projection points
	calc_TTI();

	if (AWG9_project(ps[LOCKED].xL,ps[LOCKED].yL,ps[LOCKED].alt)==-1)
		return 0;

	// projection point of target relative to ASE circle center
	PDT_PROJ_X = proj_x - TB_CTRX;
	PDT_PROJ_Y = proj_y - TB_CTRY;
	PDT_PROJ++;
}

//***************************************************************************
int LockDistanceCheck(TARGET *t)
{
	long rng=0L;
	struct PlaneData far *tempplane;

	tempplane = GetPlanePtr(ps[t->i].type);

	if (AWG9_REALITY_LEVEL==AWG9_EASY) {
		rng = (t->conv) ? 4750L:152000L;	// 100 nmi
		if ((long)t->d > rng) {
			Message("(RIO) Target Out of Range",RIO_MSG);
			return(0);
		} else {
			return(1);
		}
	}

	if (AWG9_REALITY_LEVEL!=AWG9_EASY) {
		if (tempplane->radarsig==1) {
			rng = (t->conv) ? 1900L:60800L;		// 40 nmi
		}
		if (tempplane->radarsig==2) {
			rng = (t->conv) ? 4513L:144416L;	// 95 nmi
		}
		if (tempplane->radarsig==3) {
			rng = (t->conv) ? 4988L:159616L;	// 105 nmi
		}
		if (tempplane->radarsig==4) {
			rng = (t->conv) ? 5938L:190016L;	// 125 nmi
		}
		if (tempplane->radarsig==5) {
			rng = (t->conv) ? 6888L:220416L;	// 145 nmi
		}

		if ((long)t->d > rng) {
			Message("(RIO) Target Out of Range",RIO_MSG);
			return(0);
		} else {
			return(1);
		}
	}
}

//***************************************************************************
int UpdateTargetClosure(TARGET *tc)
{
	int temptime;
	long tempdist;
	int curtime;
	long dist;
	long targcl;
	int asp,newasp;
	int ncl;

	curtime = GETTIME();
	temptime = curtime - tc->lasttime;

	if ((temptime/60)<3) return(1);

	if (temptime<=0) temptime=1;

	tempdist = LDist3D((ps[tc->i].xL-PxLng),(ps[tc->i].yL-(0x100000-PyLng)),
		(ps[tc->i].alt - Alt));

	dist = tempdist - tc->lastdist;
	dist = -dist;

	// Put check to peg dist

	// Make sure temptime is greater than 1
	if ((temptime/60)<1)
		temptime=1;
	else
		temptime = (temptime/60);

	targcl = (dist*4)/temptime;
	targcl = (targcl*100)/168;
	targcl = (targcl + tc->targetclosure)/2;
	if (targcl>1300L) targcl=1300L;
	if (targcl<-1300L) targcl=-1300L;

	tc->lastdist = tempdist;
	tc->lasttime = curtime;
	tc->targetclosure = targcl;


	asp    = angle (Px-ps[tc->i].x, -(Py-ps[tc->i].y));
	asp   -= ps[tc->i].head;
	asp   += 0x8000;
	newasp = (abs(asp)/DEG+5)/10;

	if (newasp>=0)
		ncl = ps[tc->i].speed-Knots;
	if (newasp>9 && newasp<27)
		ncl = ps[tc->i].speed+Knots;
	if (newasp>0 && newasp<9)
		ncl = ps[tc->i].speed-Knots;
	if (newasp>27 && newasp<36)
		ncl = ps[tc->i].speed-Knots;
#ifdef YEP
	sprintf(Txt,"Target Closure: %ld  asp: %i ncl: %i bar: %d az: %d",tc->targetclosure,newasp,ncl,SELECTED_BAR_NUM,SELECTED_AZ);
	Message(Txt);
#endif
}


//***************************************************************************
long LDist3D(long x, long y, int z)
{
	int shifthold = 0;
	long returnval;
	long ox, oy;
	long doshift = 0x3FFFL;

	ox = x;
	oy = y;

	if((absL(x) + absL(y)) > doshift) {
		x >>= 8;
		y >>= 8;
		z >>= 8;
		shifthold = 8;
	}

	if((absL(x) + absL(y)) > doshift) {
		x >>= 8;
		y >>= 8;
		z >>= 8;
		shifthold = 16;
	}

	if(shifthold) {
		returnval = (long)(Dist3D((int) x, (int) y,z));
		returnval <<= shifthold;
	}
	else
		returnval = (long)(Dist3D((int) x, (int) y,z));

	return(returnval);
}


//***************************************************************************
static calc_TTI()
{
	long           x,y,z;
	unsigned long  d;
	unsigned       spd;
	struct Sams    *sp;
	struct MissileData far *tempmissile;

	sp = ss + LastMisl;

	if (sp->weapon == W_AIM_9) return 0;

	if (!sp->dist)                                     // no missile active
		return 0;

	if (sp->srctype != 1 || sp->source!=MAXPLANE)      // not our missile
		return 0;

	if (sp->losing) {                                  // lock lost...
		TTI = -2;
		return 0;
	}

	x =  sp->x   - ps[sp->target].xL;
	y =  sp->y   - ps[sp->target].yL;
	z = (long)sp->alt - ps[sp->target].alt;

	// calculate distance

	if (labs(x)>=18900L||labs(y)>=18900L||labs(z)>=18900L) {
		x >>= HTOV;
		y >>= HTOV;
		z >>= HTOV;                                     // convert to short
		d = Dist3D((int)x,(int)y,(int)z);             // distance to target
		d <<= HTOV;
	} else {
		d = Dist3D((int)x,(int)y,(int)z);             // distance to target
	}

	// calculate speed
	tempmissile = GetMissilePtr(sp->type);
	spd = (tempmissile->speed>sp->speed)?tempmissile->speed:sp->speed;
	spd = spd*3>>3;

	// calculate Time To Intercept in seconds
	if (spd == 0)
		TTI = 0;
	else
		TTI = d/spd;
}

//***************************************************************************
int SearchForTargets()
{
	int i;
	TARGET t;
	int found;
	int sr;
	struct PlaneData far *tempplane;
	int asp,newasp,ncl;

	if (AWG9_SNIFF) return 0;

	// Cannot track more than one target at a time in PDSTT
	if (AWG9_MODE == AWG9_PDSTT) return(0);


	// Scan for new targets
	for (i=0; i<NPLANES; i++) {
		if (i == DESIGNATED) continue;
		if (!(ps[i].status & ACTIVE)) continue;
		if (!ps[i].speed) continue;
		// This could be a problem - check detect_targ size
		if (DETECTED_TARG[i]) continue;

		SetupNewTarget(&t,i);
		orient_to_player(&t);

		MakeMatHPR(-(OurHead+BEAM_HDG), OurPitch+BEAM_PIT, 0, BEAM_M);
		orient_to_beam(&t);

		if (!InBeam(&t)) continue;

//		tempplane = GetPlanePtr(ps[t.i].type);
//		sprintf(Txt,"Plane: %Fs",tempplane->name);
//		Message(Txt);

		found=0;
		for (sr=0; sr < *AGEDTARGFILECNT; sr++) {
			if (t.i == AGEDTARGFILE[sr].i) {
				found=1;
				t.lastdist = AGEDTARGFILE[sr].lastdist;
				t.targetclosure = AGEDTARGFILE[sr].targetclosure;
				t.lasttime = AGEDTARGFILE[sr].lasttime;
				t.id = AGEDTARGFILE[sr].id;
				break;
			}
		}

		if (found) {
			UpdateTargetClosure(&t);
		} else {
			// Let's Seed the new track
			t.id=0;
			t.lastdist = LDist3D((ps[t.i].xL-PxLng),(ps[t.i].yL-(0x100000-PyLng)),
				(ps[t.i].alt - Alt));
			asp    = angle (Px-ps[t.i].x, -(Py-ps[t.i].y));
			asp   -= ps[t.i].head;
			asp   += 0x8000;
			newasp = (abs(asp)/DEG+5)/10;

			if (newasp>=0)
				ncl = ps[t.i].speed-Knots;
			if (newasp>=9)
				ncl = ps[t.i].speed+Knots+100;
			if (newasp>=27)
				ncl = ps[t.i].speed-Knots;
			t.targetclosure=(long)ncl;
			t.lasttime=190;
			UpdateTargetClosure(&t);
		}
		UpdateConsoleXY(&t);
		AddTargetFile(&t);
		DETECTED_TARG[i]++;
	}
}


//***************************************************************************
int ResetTargetTrack()
{
	int i;

	for (i=0; i < *TARGFILECNT; i++) {
		if (TARGFILE[i].i == DESIGNATED) {
			DelTargetFile(i);
			break;
		}
	}
}

//***************************************************************************
int ResetBar()
{
	TARGET *t;
	int    *c, i;

	ResetTargetTrack();

	for (i = 0; i < NPLANES; i++)
		DETECTED_TARG[i] = 0;

	CUR_BAR = 0;

	// Flip arrays
	t               = TARGFILE;
	TARGFILE        = AGEDTARGFILE;
	AGEDTARGFILE    = t;

	// Flip counts
	c               = TARGFILECNT;
	TARGFILECNT     = AGEDTARGFILECNT;
	AGEDTARGFILECNT = c;

	// initialize new target file
	*TARGFILECNT = 0;
}

//***************************************************************************
int SetupNewTarget(TARGET *t, int plane)
{
	long x,y,z;

	// translate to player coordinates

	t->conv = 0;
	x = ps[plane].xL - PxLng;
	y = (long)ps[plane].alt - Alt;
	z = (0x100000L-PyLng) - ps[plane].yL;

	if (labs(x)>=18900L || labs(y)>=18900L || labs(z)>=18900L) {
		x >>= HTOV;
		y >>= HTOV;
		z >>= HTOV;		// convert to short
		t->conv++;
	}

	t->ox = x;
	t->oy = y;
	t->oz = z;

	// figure heading relative to player
	t->h = OurHead - ps[plane].head;
	t->i = plane;
}

//***************************************************************************
int orient_to_player(TARGET *t)
{
	int   V[3];

	// rotate to player coordinate system

	V[0] = t->ox;
	V[1] = t->oy;
	V[2] = t->oz;
	Rotate (V, PLYR_M);

	t->rx = V[0];
	t->ry = V[1];
	t->rz = V[2];
	t->d  = Dist3D(t->rx,t->ry,t->rz);
}

//***************************************************************************
static orient_to_beam(TARGET *t)
{
	int   V[3];

	// rotate to radar beam coordinate system
	V[0] = t->ox;
	V[1] = t->oy;
	V[2] = t->oz;

	Rotate (V, BEAM_M);
	t->bx = V[0];
	t->by = V[1];
	t->bz = V[2];
}

//***************************************************************************
int InBeam(TARGET *t)
{
	int      az_angle, el_angle, bar_angle;
	long rng;
	struct PlaneData far *tempplane;


	tempplane = GetPlanePtr(ps[t->i].type);

	// Distance Check
	// Check to see if we are looking past our current scope range

	rng = (t->conv) ? (long)BRANGES[RANGE]:((long)BRANGES[RANGE])<<HTOV;

	if (AWG9_REALITY_LEVEL!=AWG9_EASY) {
		if ((long)t->d > rng) return(0);
	}

	rng = (t->conv) ? 9500L:304000L;	// 200 nmi

#ifdef YEP
	sprintf(Txt,"Plane: %Fs RadarSig: %d",tempplane->name,tempplane->radarsig);
	Message(Txt);
#endif

	if (AWG9_REALITY_LEVEL!=AWG9_EASY) {
		switch(AWG9_MODE) {
			case AWG9_RWS_AUTHENTIC:
			case AWG9_RWS_WIDE:
			case AWG9_RWS_NARROW:
			case AWG9_RWS_MEDIUM:
				// Set range for RWS
				if (tempplane->radarsig==1) {
					rng = (t->conv) ? 1900L:60800L;		// 40 nmi
				}
				if (tempplane->radarsig==2) {
					rng = (t->conv) ? 4275L:136800L;	// 90 nmi
				}
				if (tempplane->radarsig==3) {
					rng = (t->conv) ? 4988L:159616L;	// 105 nmi
				}
				if (tempplane->radarsig==4) {
					rng = (t->conv) ? 5938L:190016L;	// 125 nmi
				}
				if (tempplane->radarsig==5) {
					rng = (t->conv) ? 7838L:250816L;	// 165 nmi
				}
			break;
 			case AWG9_TWSA_MEDIUM:
			case AWG9_TWSA_NARROW:
			case AWG9_TWSA_AUTHENTIC:
			case AWG9_TWSM_MEDIUM:
			case AWG9_TWSM_NARROW:
			case AWG9_TWSM_AUTHENTIC:
				if (tempplane->radarsig==1) {
					rng = (t->conv) ? 1900L:60800L;		// 40 nmi
				}
				if (tempplane->radarsig==2) {
					rng = (t->conv) ? 4275L:136800L;	// 90 nmi
				}
				if (tempplane->radarsig==3) {
					rng = (t->conv) ? 4988L:159616L;	// 105 nmi
				}
				if (tempplane->radarsig==4) {
					rng = (t->conv) ? 5938L:190016L;	// 125 nmi
				}
				if (tempplane->radarsig==5) {
					rng = (t->conv) ? 7838L:250816L;	// 165 nmi
				}
			break;
			case AWG9_SRCH_AUTHENTIC:
			case AWG9_SRCH_WIDE:
			case AWG9_SRCH_NARROW:
			case AWG9_SRCH_MEDIUM:
				if (tempplane->radarsig==1) {
					rng = (t->conv) ? 2850L:91200L;		// 60 nmi
				}
				if (tempplane->radarsig==2) {
					rng = (t->conv) ? 5463L:174816L;	// 115 nmi
				}
				if (tempplane->radarsig==3) {
					rng = (t->conv) ? 6413L:205216L;	// 135 nmi
				}
				if (tempplane->radarsig==4) {
					rng = (t->conv) ? 7125L:228000L;	// 150 nmi
				}
				if (tempplane->radarsig==5) {
					rng = (t->conv) ? 9025L:288800L;	// 190 nmi
				}
			break;
			case AWG9_BORESIGHT:
			case AWG9_VSL:
				// Set range for BORESIGHT
				if (tempplane->radarsig==1) {
					rng = (t->conv) ? 238L:7616L;		// 5 nmi
				}
				if (tempplane->radarsig==2) {
					rng = (t->conv) ? 238L:7616L;		// 5 nmi
				}
				if (tempplane->radarsig==3) {
					rng = (t->conv) ? 238L:7616L;		// 5 nmi
				}
				if (tempplane->radarsig==4) {
					rng = (t->conv) ? 380L:12160L;		// 8 nmi
				}
				if (tempplane->radarsig==5) {
					rng = (t->conv) ? 570L:18240L;		// 12 nmi
				}
		}
	}

	// Distance Check - If easy mode then you can see to 200nmi

	if (AWG9_REALITY_LEVEL == AWG9_EASY) {
		rng = (t->conv) ? 9500L:304000L;	// 200 nmi
	}

	if ((long)t->d > rng) return 0;				// out of range
	if (t->rz < 0) return 0;					// behind player

	t->az = ARCTAN (t->rx,t->rz);				// target azimuth angle
	t->el =-ARCTAN (t->ry,t->rz);				// target elevation angle

	if (!t->conv && (t->d <= FT500))  return 0;	// too close

	// Is target inside radar gimbal limit?  Check azimuth
	if (abs(t->az) > Angles[DEG65].wdeg) return 0;

	// Is target inside radar gimbal limit?  Check elevation
	if (abs(t->el) > Angles[DEG65].wdeg) {
		return 0;
	}

	if (AWG9_MODE == AWG9_BORESIGHT) {
		if (abs(t->az) > Angles[DEG10].wdeg) return 0;
		if (abs(t->el) > Angles[DEG10].wdeg) return 0;
		return(1);
	}

	if (AWG9_MODE == AWG9_VSL) {
		if (abs(t->az) > Angles[DEG10].wdeg) return 0;
		return(1);
	}

	//***********************************************************************
	// If in easy mode, this is sufficient - got the target
	if ((AWG9_MODE == AWG9_STANDARD_SRCH) || (AWG9_MODE == AWG9_STANDARD_TRACK))
		return 1;
	//***********************************************************************]

	az_angle = ARCTAN(t->bx,t->bz);             // az angle rel to beam
	el_angle =-ARCTAN(t->by,t->bz);             // el angle rel to beam

	// Is target in radar cone?
	if (abs(az_angle) > Angles[SELECTED_AZ].wdeg) {
		return 0;
	}
	// Is target in radar cone?
	if (abs(el_angle) > (SELECTED_BAR_NUM*BAR_SIZE)) {
		return 0;
	}

	// Is target within 10 NM - detect immediately
	rng = (t->conv) ? 475:15200;	// 10 nmi
	if (t->d <= rng) return 1;

	// This is sufficeient for moderate mode - target is inside radar cone
	if (AWG9_REALITY_LEVEL == AWG9_MODERATE) return 1;

	// Is the target in the current bar?
#ifdef YEP
	bar_angle = (SELECTED_BAR_NUM - CUR_BAR) * BAR_SIZE;
//	bar_angle = (SELECTED_BAR_NUM/2 - CUR_BAR) * BAR_SIZE;

	if (el_angle > bar_angle) {
		return 0;
	}
	if (el_angle < bar_angle - BAR_SIZE) {
		return 0;
	}
#endif
	// detected by az sweep?
	if ((az_angle > AZSWEEP*DEG) && AZSWEEP_DIR) {
		return 0;
	}
	if ((az_angle < AZSWEEP*DEG) && !AZSWEEP_DIR) {
		return 0;
	}
	// OKAY, YES, WE FOUND THE TARGET!!!
	return 1;
}

//***************************************************************************
int UpdateConsoleXY(TARGET *t)
{
	unsigned rng;

	// Caculate display position
	if (AWG9_REALITY_LEVEL == AWG9_EASY) {
		rng  = (t->conv) ? BRANGES[RANGE]:BRANGES[RANGE]<<HTOV;
		t->x = DDD_CTR_X + ((long)DDD_W*(t->az/DEG))/120;
		t->y = 66 - ((long)66*(t->d))/rng;
		return(1);
	}

	if (AWG9_MODE != AWG9_PDSTT) {
		t->x = DDD_CTR_X + ((long)DDD_W*(t->az/DEG))/120;
		t->y = 32 - ((long)31*(t->targetclosure))/1300;
	} else {
		rng  = (t->conv) ? BRANGES[RANGE]:BRANGES[RANGE]<<HTOV;
		t->x = DDD_CTR_X + ((long)DDD_W*(t->az/DEG))/120;
		t->y = 66 - ((long)66*(t->d))/rng;
	}
}

//***************************************************************************
int AddTargetFile(TARGET *t)
{
	int i,j;

	if (*TARGFILECNT < MAXTARGFIL)
		TARGFILE[(*TARGFILECNT)++] = *t;
}

//***************************************************************************
int DelTargetFile(int n)
{
	int i;

	if (n >= *TARGFILECNT) return 0;
	for (i=n; i < *TARGFILECNT-1; i++)
		TARGFILE[i] = TARGFILE[i+1];
	(*TARGFILECNT)--;
}

//***************************************************************************
int RioLockCheck()
{
	int dist,n,i;
	unsigned rng;
	static int prev_bad_guy=0;   // Guy we where last fighting with
//	struct PlaneData far *tempplane;

	if ((AWG9_MODE == AWG9_STANDARD_TRACK) || (AWG9_MODE == AWG9_PDSTT))
		return(0);

	for (i = 0,dist=0x7fff,n=-1; i < *TARGFILECNT; i++) {
		rng = (TARGFILE[i].conv) ? NM80:NM80<<HTOV;
		if (TARGFILE[i].d > rng ) continue;
		if (TARGFILE[i].d < dist) {
			n = i;
			dist = TARGFILE[i].d;
		}
	}

	if (n == -1) return(0);                    // no new contacts

	// Check to see if this is a bad guy

//	tempplane = GetPlanePtr(ps[TARGFILE[n].i].type);

	if(ps[TARGFILE[n].i].status & FRIENDLY) {
		return(0);
	}
	else {
		Message("(RIO) Bandit Locked Up!");
	}

	LOCKED =
	DESIGNATED = TARGFILE[n].i;
	acquire(TARGFILE[n].x, TARGFILE[n].y);

	if (AWG9_REALITY_LEVEL == AWG9_EASY) {
		mode_flip(AWG9_STANDARD_TRACK);
	} else
		mode_flip(AWG9_PDSTT);
}



//***************************************************************************
int SpecialModeAcq()
{
	int dist,n,i;
	unsigned rng;

	if ((AWG9_MODE!=AWG9_BORESIGHT) && (AWG9_MODE!=AWG9_VSL)) return 0;

	for (i = 0,dist=0x7fff,n=-1; i < *TARGFILECNT; i++) {
		rng = (TARGFILE[i].conv) ? NM10:NM10<<HTOV;
		if (TARGFILE[i].d > rng ) continue;
		if (TARGFILE[i].d < dist) {
			n = i;
			dist = TARGFILE[i].d;
		}
	}

	if (n == -1) return 0;                    // no new contacts

	LOCKED =
	DESIGNATED = TARGFILE[n].i;
	acquire(TARGFILE[n].x, TARGFILE[n].y);
	mode_flip(AWG9_PDSTT);
}

//***************************************************************************
int StandardSpecialModeAcq()
{
	int dist,n,i;

	for (i = 0,dist=0x7fff,n=-1; i < *TARGFILECNT; i++) {
		if (TARGFILE[i].d < dist) {
			n = i;
         	dist = TARGFILE[i].d;
		}
	}

	if (n == -1) return 0;                    // no new contacts
	LOCKED =
	DESIGNATED = TARGFILE[n].i;
	acquire(TARGFILE[n].x, TARGFILE[n].y);
	mode_flip(AWG9_STANDARD_TRACK);
}

//***************************************************************************
static acquire(x, y)
{
	ACQ_X = x;
	ACQ_Y = y;
}

//***************************************************************************
int aa_acquire(int flag)
{
	int     xmin,xmax,ymin,ymax,i,new,*tf;
	TARGET *t;

	BuildAcqList();

	if (!flag) {
		// KEYBOARD acquisition
		if (!ACQLISTCNT) return 0;

		for (i = 0; i < ACQLISTCNT; i++)
			if (ACQLIST[i]->i == DESIGNATED) break;

		if (i == ACQLISTCNT) i = -1;
		if (++i >= ACQLISTCNT) i = 0;

		if (i== -1) return(-1);

		DESIGNATED = ACQLIST[i]->i;
		LOCKED = -1;

	  	acquire(ACQLIST[i]->x, ACQLIST[i]->y);
#ifdef YEP
		if (MouseCrtX != -1)
			MousePOS(ACQLIST[i]->x + DDD_X + 1 + CrtRps[MouseDsp]->XBgn,
					ACQLIST[i]->y + DDD_Y + 1 + CrtRps[MouseDsp]->YBgn);
#endif
	} else {
		// MOUSE acquisition

		acquire(MouseCrtX-DDD_X-1, MouseCrtY-DDD_Y-1);

		xmin = ACQ_X - 4;
		ymin = ACQ_Y - 3;
		xmax = ACQ_X + 4;
		ymax = ACQ_Y + 3;

		// find any possible target under acq symbol

		for (i = 0,new = -1; i < ACQLISTCNT; i++) {
			t = ACQLIST[i];
			if (t->x >= xmin && t->y >= ymin && t->x <= xmax && t->y <= ymax) {
  				new = t->i;
  				break;
			}
		}

		if (new == -1) {
			if ((AWG9_MODE !=AWG9_STANDARD_SRCH) && (AWG9_MODE !=AWG9_STANDARD_TRACK)) {
			}
			return 0;
		}

		DESIGNATED = new;
		LOCKED = -1;
	}

	if (RIO_ASSIST == RIO_ASSIST_STANDARD) {
		if (!(ps[(ACQLIST[i]->i)].status & FRIENDLY)) {
			Message("(RIO) Take the Shot! Take the Shot!");
		}
		else {
			Message("(RIO) Hey watch it! That's a Friendly!");
		}
	}

	switch(AWG9_MODE) {
		case AWG9_SRCH_NARROW:
		case AWG9_SRCH_MEDIUM:
		case AWG9_SRCH_WIDE:
		case AWG9_RWS_NARROW:
		case AWG9_RWS_MEDIUM:
		case AWG9_RWS_WIDE:
		case AWG9_SRCH_AUTHENTIC:
		case AWG9_RWS_AUTHENTIC:
			LOCKED = DESIGNATED;
			AWG9_CHANGE_MODE(AWG9_PDSTT);
		break;
		case AWG9_STANDARD_TRACK:
			LOCKED = DESIGNATED;
		break;
		case AWG9_PDSTT:
			LOCKED = DESIGNATED;
		break;
		case AWG9_STANDARD_SRCH:
			LOCKED = DESIGNATED;
			AWG9_CHANGE_MODE(AWG9_STANDARD_TRACK);
		break;
	}
}


//***************************************************************************
int DesignateTIDTarg()
{
	int xmin,xmax,ymin,ymax,i,new;

	int Tidx;
	int Tidy;

	if (!CheckMode(AWG9_MODE,AWG9_TWSA) && !(CheckMode(AWG9_MODE,AWG9_TWSM)))
		return(0);

	Tidx = MouseCrtX-1;
	Tidy = MouseCrtY-1;

	xmin = Tidx - 5;
	ymin = Tidy - 4;
	xmax = Tidx + 5;
	ymax = Tidy + 4;

	// find any possible target under the mouse pointer
	new=-1;

	for (i = 0; i < TIDTargetCnt; i++) {
			if ((TIDTargetList[i].x >= xmin) &&
				(TIDTargetList[i].y >= ymin) &&
				(TIDTargetList[i].x <= xmax) &&
				(TIDTargetList[i].y <= ymax)) {
  					new = TIDTargetList[i].i;
  				break;
			}
	}

	if (new==-1) return(0);

	if (DesigTargCnt>(GetWeaponCount(W_AIM_54)-1))
		return(0);

	// Ok, now add the target to the firing list
	DESIG_TARG[DesigTargCnt].status=1;  // Change to Designated
	DESIG_TARG[DesigTargCnt].target=new;
	DesigTargCnt++;
}

//***************************************************************************
int SelectDesig()
{
	int xmin,xmax,ymin,ymax,i,new;

	int Tidx;
	int Tidy;

	if (!CheckMode(AWG9_MODE,AWG9_TWSA) && !(CheckMode(AWG9_MODE,AWG9_TWSM)))
		return(0);

	Tidx = MouseCrtX-1;
	Tidy = MouseCrtY-1;

	xmin = Tidx - 5;
	ymin = Tidy - 4;
	xmax = Tidx + 5;
	ymax = Tidy + 4;

	// find any possible target under the mouse pointer
	new=-1;

	for (i = 0; i < TIDTargetCnt; i++) {
			if ((TIDTargetList[i].x >= xmin) &&
				(TIDTargetList[i].y >= ymin) &&
				(TIDTargetList[i].x <= xmax) &&
				(TIDTargetList[i].y <= ymax)) {
  					SelectedTarg = TIDTargetList[i].i;
//					DESIGNATED = SelectedTarg;
  				break;
			}
	}

	if (new==-1) return(0);
}


//***************************************************************************
int UpdateTIDTargs()
{
	// Update Firing Order since missile fired
}

//***************************************************************************
int BuildAcqList()
{
	int i,j;

	ACQLISTCNT = 0;

	// add/sort targets in targfile
	for (i = 0; i < *TARGFILECNT; i++)
		AddAcqList(TARGFILE + i,0);

	// add aged targets
	for (i = 0; i < *AGEDTARGFILECNT; i++)
		if (!InAcqList(AGEDTARGFILE[i].i))
			AddAcqList(AGEDTARGFILE + i,0);
}

//***************************************************************************
int InAcqList(int n)
{
	int i;

	for (i=0; i<ACQLISTCNT; i++)
		if (ACQLIST[i]->i == n) return 1;
	return 0;
}

//***************************************************************************
int AddAcqList(TARGET *t, int start)
{
	int i,j;
	long rng;
	struct PlaneData far *tempplane;

	// Distance Check

	if (AWG9_REALITY_LEVEL == AWG9_EASY) {
		// Distance Check - Out of Range - Break Lock
		if (t->conv && (t->d > NM130))
			return(0);
	}

	// Distance Check - This will be dependent on radar signature
	tempplane = GetPlanePtr(ps[t->i].type);

	if (AWG9_REALITY_LEVEL != AWG9_EASY) {
		if (tempplane->radarsig==1) {
			rng = (t->conv) ? 1900L:60800L;		// 40 nmi
		}
		if (tempplane->radarsig==2) {
			rng = (t->conv) ? 4513L:144416L;	// 95 nmi
		}
		if (tempplane->radarsig==3) {
			rng = (t->conv) ? 4988L:159616L;	// 105 nmi
		}
		if (tempplane->radarsig==4) {
			rng = (t->conv) ? 5938L:190016L;	// 125 nmi
		}
		if (tempplane->radarsig==5) {
			rng = (t->conv) ? 6888L:220416L;	// 145 nmi
		}

		// Distance Check - Out of Range
		if ((long)t->d > rng) {
			return(0);
		}
	}

	for (i=start; i<ACQLISTCNT; i++) {
		if (ACQLIST[i]->d > t->d) {
			for (j=ACQLISTCNT-1; j>=i; j--)
				ACQLIST[j+1] = ACQLIST[j];
			break;
		}
	}

	ACQLIST[i] = t;
	ACQLISTCNT++;
}

//***************************************************************************
int DetermineShootParams()
{
	static flash=1;

	LOCKSHOOT  =  0;
	SHOOT      =  0;				// initialize variables
	ASE_RADIUS = -1;
	STEER_DOT  =  0;

//	if (AWG9_SNIFF) return 0;
	if (PW == W_NONE) return 0;		// no weapon in priority

	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM)) {
		if (SelectedTarg==-1) return(0);
		LOCKSHOOT=1;
	} else {
		if (LOCKED == -1) return(0);
		LOCKSHOOT = 1;
	}

	if (!PDT_PROJ) return 0;
	if (!ResizeASE()) return 0;		// no ase circle

	// set steering dot
	STEER_DOT++;
	STEER_DOT_X = PDT_PROJ_X/2;
	STEER_DOT_Y = PDT_PROJ_Y/2;

//	if (Dist2D(Xaspn(STEER_DOT_X),Yaspn(STEER_DOT_Y))>ASE_RADIUS) return 0;
	if (BIT_2_NM(PDT_RNG*10) > ORDNANCE[PW].rmax) return 0;

	SHOOT++;
	LOCKSHOOT = flash;
	if (DisplayFrame==0) flash ^= 1;
}

//***************************************************************************
int ResizeASE()
{
	int d;

	// within rmin?


	if ((AWG9_MODE == AWG9_PDSTT) || (AWG9_MODE == AWG9_STANDARD_TRACK)) {
		if (BIT_2_NM(PDT_RNG*10)<ORDNANCE[PW].rmin) {
			ASE_RADIUS = 0;
			return ASE_RADIUS;
		}
	}

	// set default radius size

	switch (PW) {
		case W_AIM_54:
			ASE_RADIUS = 35;
		break;

		case W_AIM_7:
			ASE_RADIUS = 30;
		break;

		case W_AIM_9:
			ASE_RADIUS = 22;
		break;
	}

	if (LOCKED == -1) return ASE_RADIUS;

	if ((AWG9_MODE == AWG9_STANDARD_SRCH) || (AWG9_MODE == AWG9_STANDARD_TRACK))
		return ASE_RADIUS;

	d = BIT_2_NM(PDT_RNG*10);

	if (d > ORDNANCE[PW].rmax) {
				// outside rmax
		d = 100 - (d - ORDNANCE[PW].rmax)/2;
		if (d < 75) d = 75;
	} else {	// change size based on aspect
		d = ASE_Tbl[MODE_AA==AA_SRM?1:0][ASPDEG];
	}

	ASE_RADIUS = (ASE_RADIUS * d + 50)/100;
	return ASE_RADIUS;
}

//***************************************************************************
int AWG9_project(long x, long y, int z)    // USES LONG 20-bit SCOTT-STYLE COORDS!!
{
	long  dx,dy;
	int   dz;
	long  xe,ye,ze;
	long   row3();

    dx =-(x - PxLng);
    dy = (y - (0x100000-PyLng));
    dz = (z - (int) (Alt+9));

    if (labs(dx)>23000L || labs(dy)>23000L) {
        dx >>=HTOV;
        dy >>=HTOV;
        dz >>=HTOV;
    }

    xe=row3(0,(int)dx,(int)dy,dz);
    ye=row3(1,(int)dx,(int)dy,dz);
    ze=row3(2,(int)dx,(int)dy,dz);

    if (ze >= 0) return -1;
//    if (xe>-ze || xe<ze)   return (proj_x = -1);

    proj_x  = ((xe<<8)/ze) + 160;
    proj_y  = ((ye<<8)/ze);
    proj_y -= proj_y >> 2;
	proj_y += 50;
    proj_z  = ze >> (HTOV+BT2KM-3);

    return 0;
}

//***************************************************************************
static long row3(int r,int dx,int dy,int dz)
{
	long  z;

	z  = TrgMul(VM[0][r],dx);
	z += TrgMul(VM[1][r],dz);
	z += TrgMul(VM[2][r],dy);
	return z;
}


//***************************************************************************
int FiringOrder()
{
	if (!CheckMode(AWG9_MODE,AWG9_TWSA)) {
		return 0;
	}
	BuildFiringList();
}


//***************************************************************************
int BuildFiringList()
{
	int i,j,cnt;

	FIRINGORDERCNT = 0;

	// add/sort targets in targfile
	for (i = 0; i < *TARGFILECNT; i++) {
		AddFiringList(TARGFILE + i,0);
	}

	// add aged targets
	for (i = 0; i < *AGEDTARGFILECNT; i++) {
		if (!InFiringList(AGEDTARGFILE[i].i))
			AddFiringList(AGEDTARGFILE + i,0);
	}

	// Get Phoenix cnt
	cnt = GetWeaponCount(W_AIM_54);

	for (i=0,j=0; i<FIRINGORDERCNT,j<cnt; i++,j++) {
		DESIG_TARG[j].target = FIRINGORDERLIST[i]->i;
		DESIG_TARG[j].status = 1;
	}
}

//***************************************************************************
int InFiringList(int n)
{
	int i;

	for (i=0; i<FIRINGORDERCNT; i++)
		if (FIRINGORDERLIST[i]->i == n) return 1;
	return 0;
}

//***************************************************************************
int AddFiringList(TARGET *t, int start)
{
	int i,j;
	unsigned rng;

	// Need to keep firing order the same once established

	for (i=start; i<FIRINGORDERCNT; i++) {
		rng = (TARGFILE[i].conv) ? 95:3040;  // 2 miles
		if (FIRINGORDERLIST[i]->d > (t->d+rng)) {
			for (j=FIRINGORDERCNT-1; j>=i; j--)
				FIRINGORDERLIST[j+1] = FIRINGORDERLIST[j];
			break;
		}
	}
	FIRINGORDERLIST[i] = t;
	FIRINGORDERCNT++;
}

//***************************************************************************
CheckDataLink()
{
	int i;
	TARGET t;
	long rng;
	int dlorig=0;
	struct PlaneData far *tempplane;
	int temptime;
	int curtime;
	static int lasttime;


	curtime = GETTIME();
	temptime = curtime - lasttime;

	// Perform D/L check every 5 seconds
	if ((temptime/60)<5) return(1);

	lasttime=curtime;

	for (i=0; i<8; i++) {
		DataLinkList[i] = -999;
	}
	DataLinkCnt=0;

	for (i=0; i<NPLANES; i++) {
		if (!(ps[i].status & ACTIVE)) continue;
		if (!ps[i].speed) continue;

		tempplane = GetPlanePtr(ps[i].type);

		if ((tempplane->PlaneEnumId != SO_E_2CA) &&
			(tempplane->PlaneEnumId != SO_F_14FDA) &&
			(tempplane->PlaneEnumId != SO_F_14FDB) &&
			(tempplane->PlaneEnumId != SO_F_14FDC) &&
			(tempplane->PlaneEnumId != SO_F_14MCA) &&
			(tempplane->PlaneEnumId != SO_F_14MCB)) continue;

		SetupNewTarget(&t,i);
		orient_to_player(&t);
		rng = (t.conv) ? 4750L:152000L;	// 100 nmi
		if (t.d > rng) continue;

		if (tempplane->PlaneEnumId == SO_E_2CA) {
			dlorig = 1;
		}
	}

	dlorig=1;
	if (dlorig==1) {
		for (i=0; i<NPLANES; i++) {
			if (DataLinkCnt>7) break;
			if (!(ps[i].status & ACTIVE)) continue;
			if (!ps[i].speed) continue;
			if (DETECTED_TARG[i]) continue;

			SetupNewTarget(&t,i);
			orient_to_player(&t);
			rng = (t.conv) ? 9500L:304000L;	// 200 nmi
			if (t.d > rng) continue;

			if (ps[i].status & FRIENDLY) {
				continue;
			} else {
				DataLinkList[DataLinkCnt] = i;
				DataLinkCnt++;
			}
		}
	}
}



