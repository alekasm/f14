//***************************************************************************
//
//  File:  NMENUS.C
//
//  Author: Mike McDonald
//
//  Fleet Defender - F14 Tomcat
//  Microprose Software, Inc.
//  180 Lakefront Drive
//  Hunt Valley, Maryland  21030
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <bios.h>
#include <dos.h>
#include "types.h"
#include "library.h"
#include "keycodes.h"
#include "world.h"
#include "sdata.h"
#include "sounds.h"
#include "common.h"
#include "f15defs.h"
#include "cockpit.h"


//***************************************************************************
extern  RPS     *Rp1;
extern  RPS     *Rp2;
extern  char    SHOW_;
extern  int     GRPAGE0[];
extern  UBYTE   ExitCode;
extern  int     KbSense;
extern  UWORD   Joystick;
extern  UWORD   ThrustStick;
extern  UWORD   CoolyStick;
extern  UWORD   RudderStick;
extern  UWORD   MouseSprite;

extern	int	    GRNDDETAIL;
extern	int	    PLANEDETAIL;
extern  int     SKYDETAIL;
int MISSILETRAIL=1;
int MSGDETAIL=0;
int SNDDETAIL=0;
int RudderType;
extern  int     StickType;
extern  UWORD   MinStick[];
extern  UWORD   MaxStick[];
extern  UWORD   CenterPosition[];
extern  UWORD   LowRange[];
extern  UWORD   HighRange[];

static int        R_DISABLE=0;
static int        MOSspr = 0;
static int        MOShotx;
static int        MOShoty;
static int        MosON = 1;

#define NONE_JOY		0
#define ONE_JOY			1
#define	TWO_JOY			2
#define JOYWTHROT		3
#define	THRUSTMASTER	4
#define FSPRO			5
#define VPILOT			6
#define FOOTPEDALS		7

#define KEY_SENS_HIGH	8
#define KEY_SENS_MED	9
#define KEY_SENS_LOW	10

#define SNDS_ALL_ON		11
#define SNDS_ENGINE_OFF	12
#define SNDS_SPEECH		13
#define SNDS_ALL_OFF	14

#define MSG_ALL_ON		15
#define MSG_F14_SYS		16
#define	MSG_WORLD_CHAT	17
#define MSG_ALL_OFF		18

#define WD_SKY_HIGH		20
#define WD_SKY_MED		21
#define WD_SKY_LOW		22

#define WD_GROUND_HIGH	23
#define WD_GROUND_MED	24
#define WD_GROUND_LOW	25

#define WD_F14_HIGH		26
#define WD_F14_MED		27
#define WD_F14_LOW		28

#define WD_MT_ON		29
#define WD_MT_OFF		30

#define RECAL			50
#define RESUME_GAME		51
#define ABORT_GAME		52
#define EXIT_TO_DOS		53

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
	int funcval;
} hs;

hs menuhotspots[] = {
	93,26, 103, 32, NONE_JOY,			// 0
	93,34, 103, 40, ONE_JOY,			// 1
	93,42, 103, 48, TWO_JOY,			// 2
	93,50, 103, 56, JOYWTHROT,			// 3
	93,58, 103, 64, THRUSTMASTER,		// 4
	93,66, 103, 72, VPILOT,				// 5
	93,74, 103, 80, FSPRO,				// 6
	93,82, 103, 88, FOOTPEDALS,			// 7
	93,100,103,106, KEY_SENS_HIGH,		// 8
	93,108,103,114, KEY_SENS_MED,		// 9
	93,116,103,122, KEY_SENS_LOW,		// 10
	6,179,73,196, RESUME_GAME,			// 11
	81,179,148,196, ABORT_GAME,			// 12
	246,179,313,196, EXIT_TO_DOS,		// 13
	220,152,270,164, RECAL,				// 14
	190,26,201,32, SNDS_ALL_ON,			// 15
	190,34,201,40, SNDS_ENGINE_OFF,		// 16
	190,42,201,48, SNDS_SPEECH,			// 17
	190,50,201,57, SNDS_ALL_OFF,		// 18
	190,68,202,75, MSG_ALL_ON,			// 19
	190,76,202,83, MSG_F14_SYS,			// 20
	190,84,202,91, MSG_WORLD_CHAT,		// 21
	190,92,202,99, MSG_ALL_OFF,			// 22
	157,118,173,125, WD_SKY_HIGH,		// 23
	173,118,189,125, WD_SKY_MED,		// 24
	189,118,203,125, WD_SKY_LOW,		// 25
	157,126,173,133, WD_GROUND_HIGH,	// 26
	173,126,189,133, WD_GROUND_MED,		// 27
	189,126,203,133, WD_GROUND_LOW,		// 28
	157,134,173,141, WD_F14_HIGH,		// 29
	173,134,189,141, WD_F14_MED,		// 30
	189,134,203,141, WD_F14_LOW,		// 31
	173,150,189,157, WD_MT_ON,			// 32
	189,150,203,157, WD_MT_OFF,			// 33
};

UWORD mseg;

static RPS  MenuRX = { 0, 0, 0, 319, 199, JAM3, GREEN, BLACK, 1 };
static RPS *MenuRp = &MenuRX;

static RPS  MenuRX2 = { 1, 0, 0, 319, 199, JAM3, GREEN, BLACK, 1 };
static RPS *MenuRp2 = &MenuRX2;

extern char *F14SNAME[];
extern char	*F14VNUM[];
extern int 	lastmsgview;


//***************************************************************************
MainMenu()
{
	int key=0;
	int done=0;
	int MyMouseHits=0;
	int selectedfunc=0;
	int i;
    UBYTE PalPkt[256*3 + 6];

	MouseSETUP(MouseSprite,0,0);

	ScreenOff();

   	SndSounds(N_Pause);

   MouseHIDE();


	LoadPicture(1,"CFG-bac2.PIC");

	// Draw Settings

	mseg=MapInEMSSprite(MISCSPRTSEG,0);

	ShowPage(1);
	ScreenOn();
	DrawSettings();

   MouseSHOW();


	while (kbhit()) _bios_keybrd (_KEYBRD_READ); /* Flush kbd buffer */

	while (!done) {
		key=0;
		MyMouseHits=0;
		MyMouseHits = MouseHITS();

		if (kbhit())
			key = _bios_keybrd (_KEYBRD_READ);

		if (key) {
			switch (key) {
         		case ESCKEY:
					done++;
				break;

//				case CKEY:
//					if (StickType==0) RemoveKeyJoy();
//					ReDraw(StickType,1);
//					RudderStick=0;
//					SetJoysticks(1);  // StickType is set in here
//					MikesJoyStickRecal(153,120);
//				break;
		 		case LEFTARROW:
				case RIGHTARROW:
         		case ENTER:
         		case UPARROW:
         		case DOWNARROW:
         		default:
            	break;
			}
		}

		if (MyMouseHits & 1) {
			selectedfunc=-1;
			for (i=0; i<(sizeof(menuhotspots)/sizeof(hs)); i++) {
				if (MouseX>=menuhotspots[i].x1 && MouseX<=menuhotspots[i].x2
					&&  MouseY>=menuhotspots[i].y1 && MouseY<=menuhotspots[i].y2) {
						selectedfunc = menuhotspots[i].funcval;
					}
			}

			if (selectedfunc!=-1) {
				switch(selectedfunc) {
					case NONE_JOY:
						ReDraw(StickType,NONE_JOY);
                  SetJoysticks(0);
					break;
					case ONE_JOY:
						ReDraw(StickType,ONE_JOY);
						SetJoysticks(1);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;
					case TWO_JOY:
						ReDraw(StickType,TWO_JOY);
						SetJoysticks(2);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;
					case JOYWTHROT:
						ReDraw(StickType,JOYWTHROT);
						SetJoysticks(3);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;
					case THRUSTMASTER:
						ReDraw(StickType,THRUSTMASTER);
						SetJoysticks(4);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;
					case VPILOT:
						ReDraw(StickType,VPILOT);
						SetJoysticks(2);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;
					case FSPRO:
						ReDraw(StickType,FSPRO);
						SetJoysticks(5);  // StickType is set in here
						MikesJoyStickRecal(153,120);
					break;

					case FOOTPEDALS:
                  RudderStick^= 4;
                  if (RudderStick&4)
                     {
                     ReDraw(FOOTPEDALS,FOOTPEDALS);
   						MikesJoyStickRecal(153,120);
                     }
                  else
                     ReDraw(FOOTPEDALS,StickType);
					break;

					case KEY_SENS_HIGH:
						ReDraw((10-KbSense),(10-2));
						KbSense=2;
					break;
					case KEY_SENS_MED:
						ReDraw((10-KbSense),(10-1));
						KbSense=1;
					break;
					case KEY_SENS_LOW:
						ReDraw((10-KbSense),(10-0));
						KbSense=0;
					break;

					case SNDS_ALL_ON:
						ReDraw((15+SNDDETAIL),(15+0));
						SNDDETAIL=0;
					break;
					case SNDS_ENGINE_OFF:
						ReDraw((15+SNDDETAIL),(15+1));
						SNDDETAIL=1;
					break;
					case SNDS_SPEECH:
						ReDraw((15+SNDDETAIL),(15+2));
						SNDDETAIL=2;
					break;
					case SNDS_ALL_OFF:
						ReDraw((15+SNDDETAIL),(15+3));
						SNDDETAIL=3;
					break;

					case MSG_ALL_ON:
						ReDraw((19+MSGDETAIL),(19+0));
						MSGDETAIL=0;
					break;
					case MSG_F14_SYS:
						ReDraw((19+MSGDETAIL),(19+1));
						MSGDETAIL=1;
					break;
					case MSG_WORLD_CHAT:
						ReDraw((19+MSGDETAIL),(19+2));
						MSGDETAIL=2;
					break;
					case MSG_ALL_OFF:
						ReDraw((19+MSGDETAIL),(19+3));
						MSGDETAIL=3;
					break;

					case WD_SKY_HIGH:
						ReDraw((25-SKYDETAIL),(25-2));
						SKYDETAIL=2;
					break;

					case WD_SKY_MED:
						ReDraw((25-SKYDETAIL),(25-1));
						SKYDETAIL=1;
					break;

					case WD_SKY_LOW:
						ReDraw((25-SKYDETAIL),(25-0));
						SKYDETAIL=0;
					break;

					case WD_GROUND_HIGH:
						ReDraw((28-GRNDDETAIL),(28-2));
						GRNDDETAIL=2;
					break;

					case WD_GROUND_MED:
						ReDraw((28-GRNDDETAIL),(28-1));
						GRNDDETAIL=1;
					break;

					case WD_GROUND_LOW:
						ReDraw((28-GRNDDETAIL),(28-0));
						GRNDDETAIL=0;
					break;

					case WD_F14_HIGH:
						ReDraw((31-PLANEDETAIL),(31-2));
						PLANEDETAIL=2;
					break;

					case WD_F14_MED:
						ReDraw((31-PLANEDETAIL),(31-1));
						PLANEDETAIL=1;
					break;

					case WD_F14_LOW:
						ReDraw((31-PLANEDETAIL),(31-0));
						PLANEDETAIL=0;
					break;

					case WD_MT_ON:
						ReDraw((33-MISSILETRAIL),(33-1));
						MISSILETRAIL=1;
					break;

					case WD_MT_OFF:
						ReDraw((33-MISSILETRAIL),(33-0));
						MISSILETRAIL=0;
					break;

					case RKEY:
					case RECAL:
                  if (StickType)
                     {
       					MikesJoyStickRecal(153, 120);
                     }
					break;
					case RESUME_GAME:
						done++;
					break;
					case ALTQKEY:
					case ABORT_GAME:
						done++;
						EndGame(0);
						ExitCode = 0;
					break;
					case EXIT_TO_DOS:
						EndGame(0);
						ExitCode = 0;
						done++;
					break;
				}
			}
		}

	}
	MouseHIDE();
	ScreenOff();
	lastmsgview=-1;
	SaveJoyConfig();



	if (View<FRONTAL) {
    	LoadNewCockpit();
	} else {
       	ClearPage(0, BLACK);
		LoadPalToPal("F14ORG01.PIC",PalPkt);
		UpdatePalBuf(PalPkt);
	}

	SndSounds(N_UnPause);
	MouseSHOW();

	if ((SNDDETAIL==1) || (SNDDETAIL==3))
		SndSounds(N_OkJet);
	else
		SndSounds(N_NoJet);


}

//***************************************************************************
int DrawSettings()
{
	SetFont(MenuRp,F_3x5B);
	MenuRp->APen=10;

	MouseHIDE();

	RectFill(MenuRp,223,8,85,35,18);
	RpPrint(MenuRp,225,10,"Quality Assurance");
	RpPrint(MenuRp,225,18,"Vaughn Thomas");
	RpPrint(MenuRp,225,26,"Version: 1.18");

	// Draw Stick
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[StickType].x1+5),
				(menuhotspots[StickType].y1+3),0x0100,0,0,0);

	// Rudders?
	if (RudderStick)
		ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[7].x1+5),
				(menuhotspots[7].y1+3),0x0100,0,0,0);

	// Keyboard Sens
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(10-KbSense)].x1+5),
			(menuhotspots[(10-KbSense)].y1+3),0x0100,0,0,0);

	// Sound
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(15+SNDDETAIL)].x1+5),
			(menuhotspots[(15+SNDDETAIL)].y1+3),0x0100,0,0,0);

	// Messages
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(19+MSGDETAIL)].x1+5),
			(menuhotspots[(19+MSGDETAIL)].y1+3),0x0100,0,0,0);

	// World Detail - Sky
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(25-SKYDETAIL)].x1+5),
			(menuhotspots[(25-SKYDETAIL)].y1+3),0x0100,0,0,0);
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(28-GRNDDETAIL)].x1+5),
			(menuhotspots[(28-GRNDDETAIL)].y1+3),0x0100,0,0,0);
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(31-PLANEDETAIL)].x1+5),
			(menuhotspots[(31-PLANEDETAIL)].y1+3),0x0100,0,0,0);

	// Missile Trails
	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[(33-MISSILETRAIL)].x1+5),
			(menuhotspots[(33-MISSILETRAIL)].y1+3),0x0100,0,0,0);

   MouseSHOW();

}

//***************************************************************************
int ReDraw(int prev, int current)
{
	MouseHIDE();

	RectCopy(MenuRp2,menuhotspots[prev].x1,menuhotspots[prev].y1,(menuhotspots[prev].x2 - menuhotspots[prev].x1),
			(menuhotspots[prev].y2-menuhotspots[prev].y1),MenuRp,menuhotspots[prev].x1,
			menuhotspots[prev].y1);

	ScaleRotate(mseg,185,104,6,6,MenuRp,(menuhotspots[current].x1+5),
			(menuhotspots[current].y1+3),0x0100,0,0,0);

	MouseSHOW();
}

//***************************************************************************
MouseSETUP(int MouseSeg, int MouseHotX, int MouseHotY)
{
   if (MouseSeg == MOSspr) return 0;

   MOShotx = MouseHotX;
   MOShoty = MouseHotY;
   MOSspr  = MouseSeg;

   MouseTYPE(MOShotx, MOShoty, MOSspr);
}

//***************************************************************************
pstrlen(RPS *rp, char *s)
{
	char  *t = s;
	int   fn, n;

	fn = rp->FontNr;
	n = 0;
	while (*t != 0) n += CharWidth (fn, *t++);
	if (LibCanDo (TextIs16Bit))  n /= 2;
	return n;
}
MikesJoyStickRecal(int x, int y)
{
	MouseHIDE();
   JoyRecal (x,y);
	LoadPicture(1,"CFG-bac2.PIC");
   RedrawJoyBox();
	DrawSettings();
	MouseSHOW();


}

