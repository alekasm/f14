#include "library.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "string.h"
#include "speech.h"
#include "setup.h"
#include "planeai.h"

extern  COORD   Px, Py;
extern  int     OurHead;
extern  int     QTimer;
extern  int     whichourcarr;

static  int     JstarTimer;
static  int     JstarX;
		int     LastCall = -4*60;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct _wayPoint_ aiwaypoints[];
extern struct _path_ aipaths[];
extern int     orgPlayernum;


//***************************************************************************
AwacsPicture()
{
    struct  Planes  *psi, *Mini;
    int i, dx, dy, dist, angl, MinDist, MinAngl, eyex, eyey, eyedist, bogeyi;
	int cnt;
	struct PlaneData far *ptemp;
	int carrx, carry, carrangle, carrrelangle;
	UWORD carrdist;
	int do_id = 0;
	int is_target = 0;
	int waynum, bogey_org_num;
	int whatsrc;

    MinDist = 11250;             // 250 nm range for AWACS
    if (Gtime-LastCall<(Diff+1)*60) {
		LocationPicture (AWACSSRC, ENEMYSRC, 0, MinDist, 0, 1);
    } else {
        LastCall = Gtime;

		bogeyi = -999;
        for (i=0, psi=ps; i<NPLANES; i++, psi++) {
            if (psi->status&ACTIVE && psi->speed && !(psi->status&(CRASH+FRIENDLY))) {
                dx = psi->x-Px;
                dy = psi->y-Py;

                dist = Dist2D (dx, dy);
                angl = angle (dx, -dy);

                if (dist < MinDist) {
					bogeyi = i;
                    MinDist = dist;
                    Mini    = psi;
                    MinAngl = angl;
                }
            }
        }
		LocationPicture (AWACSSRC, ENEMYSRC, MinAngl, MinDist, bogeyi, 0);
    }
	return(0);
}

//***************************************************************************
BearingToText (UWORD angl, int isawac)
{
    angl = TrgMul (angl>>1, 360);

    if (angl<10) {
        TXA ("00");
		if(isawac) {
	        Speech (A_ZERO);
    	    Speech (A_ZERO);
		}
		else {
	        Speech (ZERO);
    	    Speech (ZERO);
		}
    } else if (angl<100) {
        TXA ("0");
		if(isawac) {
        	Speech (A_ZERO);
		}
		else {
        	Speech (ZERO);
		}
    }

    NSpeech (angl, isawac);

}

//***************************************************************************
NSpeech (int num, int isawac)
{
    char    s[10];
    int     i;

    itoa(num, s, 10);
    TXA (s);

    for (i=0; s[i]!=0; i++) {
		if(isawac)
        	Speech (A_ZERO+(s[i]-'0'));
		else
        	Speech (ZERO+(s[i]-'0'));
    }
}

