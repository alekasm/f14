//***************************************************************************
// FILE: GMAIN.C
//
// Fleet Defender - F14 Tomcat
// Project Manager: Scott Spanburg
// Revised by: Mike McDonald
//
// Adapted from F-15 Strike Eagle code by Sid Mieyer, Andy Hollis
//
//***************************************************************************
#include <stdio.h>
#include <fcntl.h>
#include <dos.h>
#include "types.h"
#include "io.h"
#include "library.h"
#include "f15defs.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "common.h"
#include "event.h"
#include "snddrv.h"
#include "awg9.h"

extern  RPS     *Rp1, *Rp2, *Rp3D;
extern  char    Txt [];
extern  UBYTE   BEND;
extern  UWORD   GRPAGE0[];
extern  UWORD   Joystick;
extern  UWORD   Speedy;
extern  UWORD   CenterPosition[];
extern  char  *CockpitFiles[2][6];

extern int EMSHANDLE;
extern int NEMSSEG;
extern int AWG9_REALITY_LEVEL;
extern int RIO_ASSIST;
extern int World;

extern  int     StickType;


RPS     *Rp;
UBYTE   ExitCode=C_NORMAL;
int 	ExitErrorCode=0;
int     DisplayFrame=1;
int     F14SNUM[2] = { 5252,18 };
int     F14SCNUM[2] = { 2525,18 };
char    *F14CNUM[5] = { "F14R" };
char    *F14SNAME[30] = { "Quality Assurance" };
char    *F14SPER[30] = { "Vaughn Thomas" };
char	*F14VNUM[5]  = { "1.18" };

int _training;
int DifficultyLevel=0;

confData ConfigData;

SndDataInfo SndData;


//***************************************************************************
main(argc, argv, envp)
int     argc;
char    **argv;
char    **envp;
{
    int z;

	save_video_state();
    InitOverlay(LoadOverlay("Mgraphic.exe","Fonts.F15"));
    TurnOnGraphicsMode(0);

	mclear_screen();
	mprintf("****  DEBUGGING INFORMATION IS ON!!! - NOT FOR QA ****");

    InitOptions();
	LoadSoundConfig();
    InitSound();
    InitGraphicPages();
    InitGraph('M');

	WaitScreen();

	set_ctrl_b();
    MouseON();

	GetJoyConfig();
	LoadConfig();
    LoadFiles();
    InitSpeech();
    InitLogFile();

    ConfigCrts();
	InitCarrier();
	LoadScottMission();
    InitStealth();

	ScreenOff();
	ClearPage(0,0);

	LoadSprites();
	Init3DPal();
	InitDamnPalettes();
	InitFlir();


	ARMT_INIT();
    AWG9_INIT();

    MainGameLoop();

	PutNumLockBack();

    MouseOFF();
    reset_ctrl_b();
    FreeEMS();
    restore_video_state();
	DisplayErrorMessages();
    Exit(ExitCode);
}

//***************************************************************************
int WaitScreen()
{
	LoadPicture(1,"gm-load.PIC");
	ShowPage(1);
	ScreenOn();
}

//***************************************************************************
int InitAbort()
{
	Release3DMemory();
    MouseOFF();
    reset_ctrl_b();
    FreeEMS();
    restore_video_state();

	SndShutdown();

	DisplayErrorMessages();
    Exit(ExitCode);
}

//***************************************************************************
int InitWeather()
{
}

//***************************************************************************
int DisplayErrorMessages()
{
	if (ExitErrorCode != 0) {
		printf("\nProgram Termination:  ");
		switch(ExitErrorCode) {
			case -10:
				printf("Not Enough Memory to load objects.\n");
			break;
			case -12:
				printf("Not Enough Memory to Load Planes Objects.\n");
			break;
			case -14:
				printf("Not Enough Memory to Load Missile Objects.\n");
			break;
			case -16:
				printf("Not Enough Memory to Load Radar Objects.\n");
			break;
			case -40:
				printf("F14PD01.DAT File Not Found.\n");
			break;
			case -42:
				printf("F14RD01.DAT File Not Found.\n");
			break;
			case -44:
				printf("F14MD01.DAT File Not Found.\n");
			break;
			case -46:
				printf("MISSION.F14 File Not Found.\n");
			break;
			case -60:
				printf("3D Object Not Found.\n");
			break;
		}
	}
}

//***************************************************************************
LoadFiles()
{
	NewEMSFunction();

	switch(ConfigData.World) {
		case 0:
			LoadTheWorld("NC07F.WLD");
		break;
		case 1:
			LoadTheWorld("MEDO6.WLD");
		break;
		case 2:
			LoadTheWorld("USA08.WLD");
		break;
		default:
			LoadTheWorld("USA08.WLD");
		break;
	}

	Init3DPlanes();
	LoadSpecialObjects();
	LoadClouds();

	// Explosions & Padlock Part
	if (LoadEMSPicFile("EXPAGE1.pic",SPRTXPLSEG,0,5*4+1)) {
    	LoadEMSPicFile("EXPAGE2.pic",SPRTXPLSEG,1,0);
    	LoadEMSPicFile("EXPAGE3.pic",SPRTXPLSEG,2,0);
    	LoadEMSPicFile("EXPAGE4.pic",SPRTXPLSEG,3,0);
    	LoadEMSPicFile("EXPAGE5.pic",SPRTXPLSEG,4,0);
	}

	// Misc. Cockpit sprites
	if (LoadEMSPicFile("F14SPR3.PIC",MISCSPRTSEG,0,8)) {
		LoadEMSPicFile("F14MISC3.PIC",MISCSPRTSEG,1,0);
	}

	if (LoadEMSPicFile("RIO-1.PIC",HEAD1SEG,0,8)) {
		LoadEMSPicFile("RIO-2.PIC",HEAD1SEG,1,0);
	}

	LoadGroundObjects();
}

//***************************************************************************
int InitDamnPalettes()
{
    UBYTE   PalPkt[256*3 + 6];

	LoadPalToPal("F14ORG01.PIC",PalPkt);
	UpdatePalBuf(PalPkt);

	/* Load initial front cockpit */
	View=FRONT;
    LoadPicToPal(1, CockpitFiles[0][FRONT], PalPkt);
	UpdatePalBuf(PalPkt);

	InitSpecialPal();

	UpdatePalette();
    ShowPage(1);

    BuildCockpitSprites();
}


//***************************************************************************
int LoadConfig()
{
	FILE *f ;

	f = fopen ("config.f14", "rb") ;
	fread(&_training,   sizeof (_training),   1, f) ;
	fread(&ConfigData, sizeof (ConfigData), 1, f) ;
	fclose(f);

	AWG9_REALITY_LEVEL = ConfigData.Radar;
	RIO_ASSIST = ConfigData.RioAssist;

	AWG9_REALITY_LEVEL = AWG9_AUTHENTIC;
	RIO_ASSIST = RIO_ASSIST_AUTHENTIC;
//	ConfigData.damage=1;
//	ConfigData.GunCamera=2;
//	ConfigData.landings=2;
//	ConfigData.crashmode=2;
//	DifficultyLevel	= ConfigData.diffLevel;
//	DifficultyLevel = 0;
	if (_training) Status ^= TRAINING;
//	ConfigData.world=1;
//	ConfigData.carrier=2;
//	ConfigData.Squadron=5;
}


//***************************************************************************
int LoadSoundConfig()
{
	FILE *f ;

	f = fopen("config.snd", "rb") ;
	fread(&SndData, sizeof(SndData), 1, f) ;
	fclose(f);
}

//***************************************************************************
MainGameLoop()
{

	SetJoysticks(StickType);
    KBInit();

    do {
        UpdateTime1();
		UpdateTime2();

        TakeInputs();
        LocalCmds();

        if ((--DisplayFrame)==0) {
            GenDsp();
            DoCockpitDisplays();
			Messages();
        }

		ProcessInputs();
		FLIGHT();

		Stealth();

		DoPlayerOnTheCat();  // keeps plane in sink with boat

        AWG9();
        TEWS_SYS();
        UpdatePalette();

        if (DisplayFrame==0) {
            Flip();
            DisplayFrame=Speedy;
        }
    } while (!BEND);

	Release3DMemory();

    MouseHIDE();
    ClearPage(0, BLACK);
    ClearPage(1, BLACK);

    SndShutdown();
    UnInitSpeech();
    DumpLogFile();
	GetRidOfKeyJoy();
}

//***************************************************************************
InitGraphicPages()
{
    SetGraphicPage(0, GRPAGE0[0] = AllocGraphicPage(0)/*+320/16*/);
    SetGraphicPage(1, GRPAGE0[1] = AllocGraphicPage(1));
    SetGraphicPage(2, GRPAGE0[2] = AllocGraphicPage(2));
    Rp = Rp2;
}

