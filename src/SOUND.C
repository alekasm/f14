#include <dos.h>
#include <stdio.h>
#include <fcntl.h>
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "sounds.h"
#include "common.h"
#include "speech.h"
#include "awg9.h"
#include "armt.h"
#include "snddrv.h"

int		SndPriority	  = 0;
int     CanDoSpeech   = 0;
char    SoundFile[]   = "Psound.14g";
char    SpeechDriver[]= "Digital.f14";

extern int SNDDETAIL;
extern  char    Txt[];
extern  UWORD   ObjHands [];
extern  UWORD   ObjSegs  [];
extern SndDataInfo SndData;

extern long LoadBigEMSFile();

// **** Internal functions found in speech overlay ***

int (_far *DigSysSetup)(int BoardAddress, int BoardType, int IRQ, int DRQ,
	long BytesLoaded, unsigned EMSsegment, unsigned EMShandle);

int  (_far *DigQueue)(int phrase);      // add phrase to queue
void (_far *DigFlushQueue)(void);       // flush the queue
int  (_far *DigStatus)    (void);       // return current playback status
int  (_far *DigAllOff)    (void);       // stop anything currently playing
void (_far *DigShutDown)  (void);       // stops and DMA in progress and resets driver


//***************************************************************************
InitSound()
{
    int i;

	switch(SndData.music_type) {
		case 0:
			SoundFile[0] = 'N';
		break;
		case 1:
			SoundFile[0] = 'I';
		break;
		case C_ADLIB:
		case C_COVOX:
		case C_SB:
		case C_SBPROOLD:
		case C_PAS:
			SoundFile[0] = 'A';
		break;
		case C_SBPRONEW:
		case C_PASPLUS:
		case C_GOLD:
			SoundFile[0] = 'P';
		break;
		case C_ROLAND:
		case C_LAPC:
			SoundFile[0] = 'R';
		break;
		case C_GM:
			SoundFile[0] = 'G';
		break;
		default:
			SoundFile[0] = 'N';
		break;
	}

    InitOverlay(LoadOverlay(SoundFile,0));
	SndSysSetup(SndData.music_address,SndData.music_type,SndData.music_irq);
	i = SoundFile[0];
    if (i=='i' || i=='I') SndPriority = 1;
}


OtherJetSound(int n, int plane, int mode)
{
	SndSounds(n,plane,mode);
}


//***************************************************************************
Sound(int n, int priority, int param)
{
	if (SNDDETAIL==3) return(0);
    if ((priority>SndPriority) && (!ejected)) {
#ifdef YEP
		sprintf(Txt,"Sound: %d",n);
		mprintf(Txt);
#endif
		SndSounds(n, param);
	}

	UpdateJetSnd();
}

//***************************************************************************
InitSpeech()
{
    UBYTE far  *fptr;
    int         fhand;
    long        size;
    int         pages;

    CanDoSpeech = 0;
	if (SndData.dig_type==0) return(-1);	// No Digital Card Capab. !!!

    _dos_open (SpeechDriver, O_RDONLY, &fhand);
    size  = filelength (fhand);
    pages = (size + 15) /16;
    _dos_close (fhand);

    if (AvailSysMem()>pages) {
		if (dig_load_overlay(SpeechDriver)==0) {
			size = LoadBigEMSFile ("game.dgt", SPEECHSEG);
				if (size!=0) {
					if (DigSysSetup (SndData.dig_address,
						SndData.dig_type,
                        SndData.dig_irq,
                        SndData.dig_drq,
                        size,
                        ObjSegs[SPEECHSEG],
                        ObjHands[SPEECHSEG])==0) {
						CanDoSpeech=1;
						}
				}
		}
	}
}


//***************************************************************************
Speech(n)
{
    if (CanDoSpeech && (SNDDETAIL<2) && !ejected) {
        DigQueue(n);
    }
}

//***************************************************************************
UnInitSpeech()
{
    if (CanDoSpeech) {
        DigShutDown();
    }
}

//***************************************************************************
UpdateJetSnd()
{
	if ((SNDDETAIL==1) || ejected) {
		SndSounds(N_NoJet);
	} else {
		SndSounds(N_OkJet);
	}
}

//***************************************************************************
AdjustSounds()
{
    SNDDETAIL = ++SNDDETAIL&3;
    TXT ("Sounds ");
    TXN (3-SNDDETAIL);
    Message (Txt,SYSTEM_MSG);
    UpdateJetSnd ();
}

//***************************************************************************
Aim9Sound()
{
	static int aimon=0;

    if ((PW==W_AIM_9) && !ejected) {
		aimon=1;
        if (SHOOT) {
            Sound(N_AIM9Lock,   2, 1);
        } else {
            Sound(N_AIM9Search, 2, 1);
        }
    } else {
		if (aimon) {
        	Sound(N_AIM9Lock,   2, 0);
        	Sound(N_AIM9Search, 2, 0);
			aimon=0;
		}
    }
}


