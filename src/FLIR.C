#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "F15defs.h"
#include "sounds.h"
#include "speech.h"
#include <dos.h>

UBYTE far *Screen;
UBYTE far *Source;

UBYTE far Palette[256*3];

UBYTE far NitePal[256*3];
UBYTE far DayPal[256*3];
UBYTE far DawnPal[256*3];
UBYTE far DuskPal[256*3];


char WorldColor[256] = {
	0,2,0,2,0,0,0,2,2,2,2,1,1,1,1,0,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
};

int SelectedPal=0;

int LightsOn = 0;
int OverG    = 0;
int PaletteChanged = FALSE;
int CLOUDAMNT;
int HUDFilterMode=0;

// static  int Night60;
static  int HudIntensity=31;
UWORD TransAmount;

union  REGS  D;
struct SREGS S;

extern	int	InXPLO;
extern UWORD GRPAGE0[];
extern UWORD TimeOfDay;
extern UBYTE FlirTbl[];

extern RPS *Rp2;
extern char Txt[];

extern	int	ABOVECLOUDS;
extern	int	OVERCAST;
extern	int	GAMNT;
extern  int BackSeat;
int	OldG=0;
int	OldOvercast=0;

#define NightStart	(20*60)
#define NightEnd	(5*60)

#define DayStart	(7*60)
#define DayEnd		(18*60)
#define Dusk		(19*60)
#define Dawn		(6*60)


//***************************************************************************
InitFlir()
{
    int     i,j;

    for (i=0,j=0; i<256; i++,j+=3) {
        FlirTbl[i] = 0x52 + ((Palette[j]*30+Palette[j+1]*59+Palette[j+2]*11)/(832));
	}

    FlirTbl[TSD_COLOR]      = TSD_MONO_COLOR;
    FlirTbl[TSD_MONO_COLOR] = TSD_MONO_COLOR;
    FlirTbl[WHITE]          = TSD_MONO_COLOR;
    FlirTbl[LGREY]          = 0x5B;

	TimeConditionCheck();
}

//***************************************************************************
UpdatePalBuf(UBYTE *PalPkt)
{
#define ID1         0
#define ID2         1
#define STARTCOLOR  4
#define ENDCOLOR    5

    int i,j;

    if (PalPkt[ID1]=='M' && PalPkt[ID2]=='0') {
        for (j=6, i=PalPkt[STARTCOLOR]*3; i < (PalPkt[ENDCOLOR]+1)*3; i++, j++) {
            Palette[i] = PalPkt[j];
        }
        PaletteChanged = 1;
    }
}



//***************************************************************************
UpdatePalette()
{
    static UWORD OldNight =0;
    static int   OldClouds=0;
    static int   OldCycle =0;
	static int NextPalFrame=0;
	static int OldPal=0;

	if ((PaletteChanged == 0) && (NextPalFrame^=1))
		return(1);

	FireColors();

    if (abs(GAMNT)<30) {
        OverG=0;
    } else if (!OverG) {
        OverG=1;
    }

    if (GAMNT!=OldG) {
        Sound(N_RedOut, 2, (abs(GAMNT)*15/100));
    }

	TimeConditionCheck();

	OldG = GAMNT;

	if (TransAmount!=OldNight || SelectedPal!=OldPal || OldClouds!=CLOUDAMNT || PaletteChanged ||
		InXPLO ||OldG!=GAMNT||(abs(GAMNT)==100)||(OldOvercast!=OVERCAST)) {

		if ((CLOUDAMNT+=InXPLO)>100) CLOUDAMNT=100;
		if (InXPLO) InXPLO=100;

        NSetGreyPal(52-InXPLO/2, 52-InXPLO/2, 52-InXPLO/2);
		InXPLO=0;

        OldNight = TransAmount;
		OldPal = SelectedPal;
        OldClouds = CLOUDAMNT;
		OldG = GAMNT;
        OldOvercast = OVERCAST;
        PaletteChanged = FALSE;
    }
}

//***************************************************************************
TimeConditionCheck()
{
	if ((TimeOfDay>=0) && (TimeOfDay<=NightEnd)) {
		// Night
		TransAmount = 0;
		SelectedPal = 5;
		LightsOn = 1;
		Night = 1;
	}
    if ((TimeOfDay>=NightEnd) && (TimeOfDay<=Dawn)) {
		// Night/Dawn Mix
		TransAmount = TimeOfDay - NightEnd;
		SelectedPal = 0;
		LightsOn = 1;
		Night = 1;
	}
	if ((TimeOfDay>=Dawn) && (TimeOfDay<=DayStart)) {
		// Dawn/Day Mix
		TransAmount = TimeOfDay - Dawn;
		SelectedPal = 1;
		LightsOn = 1;
		Night = 0;
	}
	if ((TimeOfDay>=DayStart) && (TimeOfDay<=DayEnd)) {
		// Day
		TransAmount = 0;
		SelectedPal = 2;
		LightsOn = 0;
		Night = 0;
	}
	if ((TimeOfDay>=DayEnd) && (TimeOfDay<=Dusk)) {
		// Day/Dusk Mix
		TransAmount = TimeOfDay - DayEnd;
		SelectedPal = 3;
		LightsOn = 0;
		Night = 0;
	}
	if ((TimeOfDay>Dusk) && (TimeOfDay<=NightStart)) {
		// Dusk/Night Mix
		TransAmount = TimeOfDay - Dusk;
		SelectedPal = 4;
		LightsOn = 1;
		Night = 1;
	}
	if ((TimeOfDay>=NightStart) && (TimeOfDay<=(24*60))) {
		// Night
		TransAmount = 0;
		SelectedPal = 5;
		LightsOn = 1;
		Night = 1;
	}
}


//***************************************************************************
NSetGreyPal(int TR, int TG, int TB)
{
 	int	i;
	int j;
	int palpos;
	int A;
	unsigned char far *P;
	int R,G,B;
	int N, CockpitLast;
	unsigned char TBLOCK[3*256];

    for (i=0,P=Palette; i<(256*3); i++,P++) {
        TBLOCK[i] = *P;
    }

	switch(SelectedPal) {
		case 0:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if ((WorldColor[i]==1) || ((WorldColor[i]==2) && (!BackSeat) && ((View==FRONT) || (View==DOWN)))) {
					TBLOCK[palpos] = NitePal[palpos] + ((DawnPal[palpos] - NitePal[palpos]) * TransAmount)/60;
					TBLOCK[palpos+1] = NitePal[palpos+1] + ((DawnPal[palpos+1] - NitePal[palpos+1]) * TransAmount)/60;
					TBLOCK[palpos+2] = NitePal[palpos+2] + ((DawnPal[palpos+2] - NitePal[palpos+2]) * TransAmount)/60;
				}
			}
		break;
		case 1:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if (WorldColor[i]==1) {
					TBLOCK[palpos] = DawnPal[palpos] + ((DayPal[palpos] - DawnPal[palpos]) * TransAmount)/60;
					TBLOCK[palpos+1] = DawnPal[palpos+1] + ((DayPal[palpos+1] - DawnPal[palpos+1]) * TransAmount)/60;
					TBLOCK[palpos+2] = DawnPal[palpos+2] + ((DayPal[palpos+2] - DawnPal[palpos+2]) * TransAmount)/60;
				}
			}
		break;
		case 2:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if (WorldColor[i]==1) {
					TBLOCK[palpos] = DayPal[palpos];
					TBLOCK[palpos+1] = DayPal[palpos+1];
					TBLOCK[palpos+2] = DayPal[palpos+2];
				}
				if (WorldColor[i]==1) {
					TBLOCK[palpos]   = (TBLOCK[palpos] + ((TR-TBLOCK[palpos])*CLOUDAMNT)/100);
					TBLOCK[palpos+1] = (TBLOCK[palpos+1] + ((TG-TBLOCK[palpos+1])*CLOUDAMNT)/100);
					TBLOCK[palpos+2] = (TBLOCK[palpos+2] + ((TB-TBLOCK[palpos+2])*CLOUDAMNT)/100);
				}
			}
		break;
		case 3:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if (WorldColor[i]==1) {
					// Kludge for Green corruption in 141,142,143
					if (TransAmount<5) TransAmount=5;
					TBLOCK[palpos] = DayPal[palpos] + ((DuskPal[palpos] - DayPal[palpos]) * TransAmount)/60;
					TBLOCK[palpos+1] = DayPal[palpos+1] + ((DuskPal[palpos+1] - DayPal[palpos+1]) * TransAmount)/60;
					TBLOCK[palpos+2] = DayPal[palpos+2] + ((DuskPal[palpos+2] - DayPal[palpos+2]) * TransAmount)/60;
				}
			}
		break;
		case 4:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if ((WorldColor[i]==1) || ((WorldColor[i]==2) && (!BackSeat) && ((View==FRONT) || (View==DOWN)))) {
					TBLOCK[palpos] = DuskPal[palpos] + ((NitePal[palpos] - DuskPal[palpos]) * TransAmount)/60;
					TBLOCK[palpos+1] = DuskPal[palpos+1] + ((NitePal[palpos+1] - DuskPal[palpos+1]) * TransAmount)/60;
					TBLOCK[palpos+2] = DuskPal[palpos+2] + ((NitePal[palpos+2] - DuskPal[palpos+2]) * TransAmount)/60;
				}
			}
		break;
		case 5:
			for (i=0,palpos=0; i<256; i++,palpos+=3) {
				if ((WorldColor[i]==1) || ((WorldColor[i]==2) && (!BackSeat) && ((View==FRONT) || (View==DOWN)))) {
					TBLOCK[palpos] = NitePal[palpos];
					TBLOCK[palpos+1] = NitePal[palpos+1];
					TBLOCK[palpos+2] = NitePal[palpos+2];
				}
			}
		break;
	}

	MCGAColors (0, 256, TBLOCK);
}




//***************************************************************************
union REGS D;

UBYTE BlueFlame[] = {63,63,63, 60,62,63, 53,60,63, 47,57,63,
                     43,56,63, 40,55,63, 50,59,63, 56,61,63};

UBYTE OrangeFlame[]={62,63,22, 61,57,21, 61,49,21, 61,41,21,
                     61,33,21, 61,25,21, 61,33,21, 61,41,21,
                     61,49,21, 61,57,21 };

//***************************************************************************
FireColors ()
{
    static ix=0;
    static jx=0;

    UBYTE   Buf[6];

    Palette[3*ORANGEFLAME+0] = Buf[0] = OrangeFlame[jx++];
    Palette[3*ORANGEFLAME+1] = Buf[1] = OrangeFlame[jx++];
    Palette[3*ORANGEFLAME+2] = Buf[2] = OrangeFlame[jx++];
    Palette[3*BLUEFLAME  +0] = Buf[3] = BlueFlame[ix++];
    Palette[3*BLUEFLAME  +1] = Buf[4] = BlueFlame[ix++];
    Palette[3*BLUEFLAME  +2] = Buf[5] = BlueFlame[ix++];

    MCGAColors(ORANGEFLAME, 2, Buf);

    if (jx>=(3*10)) jx=0;
    if (ix>=(3*8)) ix=0;
}

//***************************************************************************
ChangeHUDColor(int change)
{
    UBYTE   Buf[3];

    HudIntensity += change;

    if (HudIntensity<0 ) HudIntensity=0;
    if (HudIntensity>31) HudIntensity=31;

    Palette[3*HUD_COLOR+0] = Buf[0] = 0;
    Palette[3*HUD_COLOR+1] = Buf[1] = HudIntensity+32;
    Palette[3*HUD_COLOR+2] = Buf[2] = 0;

    MCGAColors(HUD_COLOR,1,Buf);
}

//***************************************************************************
int ChangeHUDFilter()
{
    UBYTE   Buf[3];

	Palette[3*HUD_COLOR+0] = Buf[0] = 0x64;
    Palette[3*HUD_COLOR+1] = Buf[1] = 0x35;
    Palette[3*HUD_COLOR+2] = Buf[2] = 0;

    MCGAColors(HUD_COLOR,1,Buf);
}

//***************************************************************************
int InitSpecialPal()
{
	int i,j;
    UBYTE PalPkt[256*3 + 6];

	unsigned char far *P;

	for (i=0,P=Palette; i<256*3; i++,P++) {
		DayPal[i] = *P;
		NitePal[i] = *P;
		DawnPal[i] = *P;
		DuskPal[i] = *P;
	}

	LoadPalToPal("DAYPAL.pic",PalPkt);

	if (PalPkt[0]=='M' && PalPkt[1]=='0') {
		for (j=6,i=PalPkt[4]*3; i<(PalPkt[5]+1)*3; i++,j++) {
			DayPal[i] = PalPkt[j];
		}
	}

	LoadPalToPal("NITEPAL.pic",PalPkt);

	if (PalPkt[0]=='M' && PalPkt[1]=='0') {
		for (j=6,i=PalPkt[4]*3; i<(PalPkt[5]+1)*3; i++,j++) {
			NitePal[i] = PalPkt[j];
		}
	}

	LoadPalToPal("DAWNPAL.pic",PalPkt);

	if (PalPkt[0]=='M' && PalPkt[1]=='0') {
		for (j=6,i=PalPkt[4]*3; i<(PalPkt[5]+1)*3; i++,j++) {
			DawnPal[i] = PalPkt[j];
		}
	}

	LoadPalToPal("DUSKPAL.pic",PalPkt);

	if (PalPkt[0]=='M' && PalPkt[1]=='0') {
		for (j=6,i=PalPkt[4]*3; i<(PalPkt[5]+1)*3; i++,j++) {
			DuskPal[i] = PalPkt[j];
		}
	}

}

#ifdef YEP
//***************************************************************************
SetGreyPal(int TR, int TG, int TB)
{
 	int	i;
	int j;
	int palpos;
	int A;
	unsigned char far *P;
	int R,G,B;
	int N, CockpitLast;
	unsigned char TBLOCK[3*256];

    CockpitLast = (View>=FRONTAL ? 0x10 : 0x40);

	N= (GAMNT==OldG)? CockpitLast*3:256*3;

    for (i=0,P=Palette; i<N; i++,P++) {
        TBLOCK[i] = *P;
    }

	R=Palette[3*14];
	G=Palette[3*14+1];
	B=Palette[3*14+2];

	R= (R+ ((TR-R)*CLOUDAMNT)/100);
	G= (G+ ((TG-G)*CLOUDAMNT)/100);
	B= (B+ ((TB-B)*CLOUDAMNT)/100);

	R= (R*(60-Night60))/60;
    G= (G*(60-Night60))/60;
    B= (B*(60-Night60))/60;


	TBLOCK[(3*14)+0]= R;
	TBLOCK[(3*14)+1]= G;
	TBLOCK[(3*14)+2]= B;

	R=Palette[3*13];
	G=Palette[3*13+1];
	B=Palette[3*13+2];

	R= (R+ ((TR-R)*CLOUDAMNT)/100);
	G= (G+ ((TG-G)*CLOUDAMNT)/100);
	B= (B+ ((TB-B)*CLOUDAMNT)/100);

	R= (R*(60-Night60))/60;
    G= (G*(60-Night60))/60;
    B= (B*(60-Night60))/60;

	TBLOCK[(3*13)+0]= R;
	TBLOCK[(3*13)+1]= G;
	TBLOCK[(3*13)+2]= B;


	for (i=CockpitLast*3,P=Palette+CockpitLast*3; i<256*3; i+=3,P+=3) {
		R=*(P+0)&0x3f;
		G=*(P+1)&0x3f;
		B=*(P+2)&0x3f;
		N=i/3;

		// If we are overcast, then make clouds (128-143) look stormy
		if ((ABOVECLOUDS==0) && (OVERCAST==1)) {
			if ((N>0x7f)&&(N<0x90)) {
		   		R= (R+ ((0x1f-R)*(N-0x7f))/16);
		   		G= (G+ ((0x1f-G)*(N-0x7f))/16);
		   		B= (B+ ((0x1f-B)*(N-0x7f))/16);
			}
		}

		// If lights On, the turn lights on color
        if (LightsOn) {
            if (N==0x90) {
                R=20; G=25; B=0;    // Cockpit color #1
            } else if (N==0x91) {
                R=14; G=19; B=0;    // Cockpit color #2
            } else if (N==0x96) {
                R=38; G=33; B=0;    // Doorways on buildings
            }
        }


		if (!((N>=0x50)&&(N<=0x5f)) && N!=TSD_MONO_COLOR) {
			// If we are going thru clouds, then let's grey us out
			R= (R+ ((TR-R)*CLOUDAMNT)/100);
			G= (G+ ((TG-G)*CLOUDAMNT)/100);
			B= (B+ ((TB-B)*CLOUDAMNT)/100);

			// It's getting dark out, let's darken the world palette
			if (!(((N>=0x40)&&(N<=0x4f)) || ((N>=0x90)&&(N<=0x9f)))) {
				R= (R*(60-Night60))/60;
			    G= (G*(60-Night60))/60;
			    B= (B*(60-Night60))/60;
			}
		}
		TBLOCK[i+0]= R;
		TBLOCK[i+1]= G;
		TBLOCK[i+2]= B;
	}


	for (i=10,palpos=10*3; i<12; i++,palpos+=3) {
		TBLOCK[palpos] = DayPal[palpos] + ((NitePal[palpos] - DayPal[palpos]) * Night60)/60;
		TBLOCK[palpos+1] = DayPal[palpos+1] + ((NitePal[palpos+1] - DayPal[palpos+1]) * Night60)/60;
		TBLOCK[palpos+2] = DayPal[palpos+2] + ((NitePal[palpos+2] - DayPal[palpos+2]) * Night60)/60;
	}


#ifdef YEP
 	if (GAMNT || (GAMNT!=OldG)) {
		A=abs(GAMNT);
		N= (GAMNT>0)? 0:63;
		for (i=0;i<(3*256);i+=3) {
			R=TBLOCK[i]&0x3f;
			G=TBLOCK[i+1]&0x3f;
			B=TBLOCK[i+2]&0x3f;
			R= (R+ ((N-R)*A)/100);
			G= (G+ ((0-G)*A)/100);
			B= (B+ ((0-B)*A)/100);
			TBLOCK[i+0]= R;
			TBLOCK[i+1]= G;
			TBLOCK[i+2]= B;
		}
	}
#endif

#ifdef YEP
	/* Dim-cockpit colors for dawn and dusk */
	if (View==FRONT || View==LEFT || View==DOWN || View==RIGHT) {
		if (View==RIGHT) {
			for (i=18,palpos=18*3; i<32; i++,palpos+=3) {
				TBLOCK[palpos] = DayPal[palpos] + ((NitePal[palpos] - DayPal[palpos]) * Night60)/60;
				TBLOCK[palpos+1] = DayPal[palpos+1] + ((NitePal[palpos+1] - DayPal[palpos+1]) * Night60)/60;
				TBLOCK[palpos+2] = DayPal[palpos+2] + ((NitePal[palpos+2] - DayPal[palpos+2]) * Night60)/60;
			}
		} else {
			TBLOCK[6] = DayPal[6] + ((NitePal[6] - DayPal[6]) * Night60)/60;
			TBLOCK[6+1] = DayPal[6+1] + ((NitePal[6+1] - DayPal[6+1]) * Night60)/60;
			TBLOCK[6+2] = DayPal[6+2] + ((NitePal[6+2] - DayPal[6+2]) * Night60)/60;

			for (i=10,palpos=10*3; i<12; i++,palpos+=3) {
				TBLOCK[palpos] = DayPal[palpos] + ((NitePal[palpos] - DayPal[palpos]) * Night60)/60;
				TBLOCK[palpos+1] = DayPal[palpos+1] + ((NitePal[palpos+1] - DayPal[palpos+1]) * Night60)/60;
				TBLOCK[palpos+2] = DayPal[palpos+2] + ((NitePal[palpos+2] - DayPal[palpos+2]) * Night60)/60;
			}

			for (i=16,palpos=16*3; i<32; i++,palpos+=3) {
				TBLOCK[palpos] = DayPal[palpos] + ((NitePal[palpos] - DayPal[palpos]) * Night60)/60;
				TBLOCK[palpos+1] = DayPal[palpos+1] + ((NitePal[palpos+1] - DayPal[palpos+1]) * Night60)/60;
				TBLOCK[palpos+2] = DayPal[palpos+2] + ((NitePal[palpos+2] - DayPal[palpos+2]) * Night60)/60;
			}
		}

		for (i=35,palpos=35*3; i<61; i++,palpos+=3) {
			TBLOCK[palpos] = DayPal[palpos] + ((NitePal[palpos] - DayPal[palpos]) * Night60)/60;
			TBLOCK[palpos+1] = DayPal[palpos+1] + ((NitePal[palpos+1] - DayPal[palpos+1]) * Night60)/60;
			TBLOCK[palpos+2] = DayPal[palpos+2] + ((NitePal[palpos+2] - DayPal[palpos+2]) * Night60)/60;
		}

		if (LightsOn) {
			for (j=32; j<35; j++)
				for (i=0; i<3; i++)
					TBLOCK[(j*3)+i] = NitePal[(j*3)+i];

			for (j=58; j<62; j++)
				for (i=0; i<3; i++)
					TBLOCK[(j*3)+i] = NitePal[(j*3)+i];

			for (i=0; i<3; i++)
				TBLOCK[(1*3)+i] = NitePal[(1*3)+i];

		}
	}
#endif

	R=Palette[(3*95)+0];
	G=Palette[(3*95)+1];
	B=Palette[(3*95)+2];

	R= (R+ ((TR-R)*CLOUDAMNT)/100);
	G= (G+ ((TG-G)*CLOUDAMNT)/100);
	B= (B+ ((TB-B)*CLOUDAMNT)/100);

	R= (R*(60-Night60))/60;
    G= (G*(60-Night60))/60;
    B= (B*(60-Night60))/60;

	TBLOCK[(3*95)+0]= R;
	TBLOCK[(3*95)+1]= G;
	TBLOCK[(3*95)+2]= B;

	MCGAColors (0, 256, TBLOCK);
}
#endif


