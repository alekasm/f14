//***************************************************************************
// File: TID.C
//
// Author: Mike McDonald
//
//
// Fleet Defender: F-14 Tomcat
// MicroProse, Inc.
// 180 LakeFront Drive
// HuntValley, MD 21030
//
//***************************************************************************

#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"


int DesignateMode=0;
TIDTarg TIDTargetList[24];
int TIDTargetCnt=0;
extern int DesigTargCnt;
extern DTARGET DESIG_TARG[];

int Tscale;
int TID_MODE=TID_NAV;

extern  struct RastPort *RpCRT;
extern COORD    Px,Py;
extern unsigned Alt;
extern int      OurHead;
extern int      OurRoll;
extern UWORD    Rtime;
extern int		LOCKED;
int tempx,tempy;
extern TARGET	*TARGFILE;
extern TARGET	*AGEDTARGFILE;
extern int		*TARGFILECNT;
extern int		*AGEDTARGFILECNT;
extern TARGET *FIRINGORDERLIST[];
extern int FIRINGORDERCNT;
extern int      CLOSURE;
extern int      PDT_RNG;
extern int	DataLinkCnt;
extern int	DataLinkList[];
extern int	DataLink;
extern int  BackSeat;

int SelectedTarg;
int HighlightTarg;
int ClrTarg;

//***************************************************************************
DoTID(int CrtNr)
{
    int i;
    struct Planes *psi;
	TARGET t;

    RectFill(RpCRT, 0,0, RpCRT->Width1+1,RpCRT->Length1+1, 82);

	TIDTargetCnt=0;

	if (DataLink) {
   		for (i=0; i<DataLinkCnt; i++) {
			SetupNewTarget(&t,DataLinkList[i]);
			orient_to_player(&t);
			TID_draw_target(&t,0,1);
			TrackDisplayPosition(&t,1);
		}
	}

	if (AWG9_MODE==AWG9_PDSRCH) return 1;
	if (AWG9_MODE==AWG9_SRCH_WIDE) return 1;
	if (AWG9_MODE==AWG9_SRCH_NARROW) return 1;
	if (AWG9_MODE==AWG9_SRCH_MEDIUM) return 1;
	if (AWG9_MODE==AWG9_SRCH_AUTHENTIC) return 1;

	if (AWG9_MODE != AWG9_PDSTT) {
		TID_DisplayScanLimits();
	}

	TIDAttkInfo();

	if (AWG9_MODE == AWG9_PDSTT) {
		for (i = 0; i < *TARGFILECNT; i++) {
			TID_draw_target(TARGFILE + i,(TARGFILE[i].i==LOCKED),0);
		}
	} else {
		// show aged targets
   		for (i = 0; i < *AGEDTARGFILECNT; i++) {
			if (DopplerCheck(AGEDTARGFILE + i)) {
				TID_draw_target(AGEDTARGFILE + i,(AGEDTARGFILE[i].i==LOCKED),0);
			}
			TrackDisplayPosition(TARGFILE + i,0);
		}
	}

	/* Draw Our Plane */
	tempx=(RpCRT->Width1/2);
	tempy=(RpCRT->Length1);
	draw_circle (RpCRT,tempx,tempy-3,1,20,0);
    return 1;
}

//***************************************************************************
int TrackDisplayPosition(TARGET *tc,int dl)
{
	struct Planes *tempps;

	if (TIDTargetCnt>23) return(0);

	tempps = ps + tc->i;

	TIDxy(tempps->x,tempps->y);
	if ((tempx!=0) && (tempy!=0)) {
		TIDTargetList[TIDTargetCnt].x = tempx;
		TIDTargetList[TIDTargetCnt].y = tempy;
		TIDTargetList[TIDTargetCnt].i = tc->i;
		TIDTargetCnt++;
	}
}

//***************************************************************************
int TIDAttkInfo()
{
	char s[5];
	char t[10];
	int r;
	char l[5];

	SetFont(RpCRT,F_3x5B);

	// Display Range, Time-to-go, and RangeRate

	if (BackSeat) {
		if (AWG9_MODE == AWG9_PDSTT) {
			r = BIT_2_NM(PDT_RNG);
			itoa(r,s,10);
			RpPrint(RpCRT,40,15,"RA");
			RpPrint(RpCRT,50,15,s);

			sprintf(Txt,"TA %s",ASPECT_S);
			RpPrint(RpCRT,85,15,Txt);

			sprintf(Txt,"ALT %d",ps[LOCKED].alt);
			RpPrint(RpCRT,40,22,Txt);
			itoa(CLOSURE,t,10);
			RpPrint(RpCRT,125,60,t);
		}
	} else {
		if (AWG9_MODE == AWG9_PDSTT) {
			r = BIT_2_NM(PDT_RNG);
			itoa(r,s,10);
			RpPrint(RpCRT,8,5,"RA");
			RpPrint(RpCRT,18,5,s);

			sprintf(Txt,"TA %s",ASPECT_S);
			RpPrint(RpCRT,60,5,Txt);

			sprintf(Txt,"ALT %d",ps[LOCKED].alt);
			RpPrint(RpCRT,8,12,Txt);
		}
	}

#ifdef YEP
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

	sprintf(Txt,"TA %s",ASPECT_S);
	RpPrint(RpCRT,85,15,Txt);

	RpPrint(RpCRT,40,22,"ALT 15000");


	itoa(CLOSURE,t,10);
	RpPrint(RpCRT,125,60,t);
#endif


	if (BackSeat) {
	// Weapon Display Info

	switch (MODE_AA) {
		case AA_MRM:
			strcpy (l, "SP");
			strcpy (t, itoa(get_count(),s,10));
		break;

		case AA_SRM:
			strcpy (l, "SW");
			strcpy(t, itoa(get_count(),s,10));
		break;

		case AA_LRM:
			strcpy (l,"PH");
			strcpy(t, itoa(get_count(),s,10));
		break;

		case AA_GUN:
			strcpy (l,"G");
   			if (!Rounds)
				strcpy (t,"XXX");
	  		else
				strcpy (t, itoa(Rounds,s,10));
		break;
	}

	RpPrint(RpCRT, 110-(pstrlen(RpCRT,l)), 90, l);
	RpPrint(RpCRT, 110-(pstrlen(RpCRT,t)), 97, t);
	}
}



//***************************************************************************
int TIDDisplayHorz()
{
	int         i;
	int         pts [6][2];
	static int  hpts[6][2] = {{60,0},{-60,0}};
	int xc,yc;

	xc = (RpCRT->Width1/2);
	yc = (RpCRT->Length1/2);

	// rotate horizon line

	for (i=0;i<2;i++)
		rotate_pt(hpts[i],pts[i],OurRoll);

	// draw horizon line

	DrawLine(RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],20);
	DrawLine(RpCRT,xc+pts[3][0],yc+pts[3][1],xc+pts[4][0],yc+pts[4][1],20);
}


//***************************************************************************
int TIDxy(int x,int y)
{
    int dx,dy;

    dx = ((int)(x-Px)) >> (7 - Tscale);
    dy = ((int)(y-Py)) >> (7 - Tscale);

	tempx=0;
	tempy=0;

    if ((UWORD)(abs(dx)+abs(dy)) <= RpCRT->Width1)  {
        Rotate2D(dx, dy, OurHead, &tempx, &tempy);
		tempy += tempy>>1;
        tempy += RpCRT->Length1;
        tempx += (RpCRT->Width1/2);
	}
}


//***************************************************************************
static TID_show_range()
{
	static RANGES[] = { 200,100,50,25,10 };
	char   s[10];
	int    l;

	RpCRT->FontNr = F_3x4;
	RpCRT->APen   = 20;

	l = pstrlen(RpCRT,itoa(RANGES[Tscale],s,10));

	RpPrint (RpCRT,75-l,8,s);
}

//***************************************************************************
int TID_DisplayScanLimits()
{
	DrawLine(RpCRT,(RpCRT->Width1/2),(RpCRT->Length1),(RpCRT->Width1),(RpCRT->Length1/2),20);
	DrawLine(RpCRT,(RpCRT->Width1/2),(RpCRT->Length1),0,(RpCRT->Length1/2),20);
}

//***************************************************************************
int TID_draw_target(TARGET *tc, int locked,int dl)
{
	int   d, clr,temp;
	int   x1,y1,x2,y2,off;
	int taillength=5;
	int font;
	int alt;
	int i;
	int found;
	struct Planes *tempps;
	char s[10];
	int cnt=0;
	int ipos=0;


	tempps = ps + tc->i;

	RpCRT->APen = 85;

	if (locked)
		clr = 85;
	else
		clr = 85;


	d = (((tempps->head-OurHead)+0x0800) >> 12)&15;
	off=0;

	font = GetFont(RpCRT,0);
	SetFont(RpCRT,F_3x4);

	if (SelectedTarg==tc->i) {
		clr=89;
		RpCRT->APen = 89;
	}

	if (dl)
		tc->id=0;

	TIDxy(tempps->x,tempps->y);

	if (tc->id==0) {
		if (dl) {
	 		DrawLine (RpCRT,tempx-2,tempy,tempx-2,tempy+2,clr);
			DrawLine (RpCRT,tempx+2,tempy,tempx+2,tempy+2,clr);
			DrawLine (RpCRT,tempx-2,tempy+2,tempx+2,tempy+2,clr);
		} else {
			DrawLine (RpCRT,tempx-2,tempy-2,tempx+2,tempy-2,clr);
			DrawLine (RpCRT,tempx-2,tempy-1,tempx-2,tempy  ,clr);
			DrawLine (RpCRT,tempx+2,tempy-1,tempx+2,tempy  ,clr);
		}
	}

	if (tc->id==1) {
		if (dl) {
			DrawLine (RpCRT,tempx-1,tempy+2,tempx+1,tempy+2,clr);
			DrawLine (RpCRT,tempx-2,tempy+1,tempx-2,tempy  ,clr);
			DrawLine (RpCRT,tempx+2,tempy+1,tempx+2,tempy  ,clr);
		} else {
			DrawLine (RpCRT,tempx-1,tempy-2,tempx+1,tempy-2,clr);
			DrawLine (RpCRT,tempx-2,tempy-1,tempx-2,tempy  ,clr);
			DrawLine (RpCRT,tempx+2,tempy-1,tempx+2,tempy  ,clr);
		}
	}

	if (tc->id==2) {
		if (dl) {
			DrawLine(RpCRT,tempx-2,tempy,tempx,tempy+2,clr);
			DrawLine(RpCRT,tempx,tempy+2,tempx+2,tempy,clr);
		} else {
			DrawLine(RpCRT,tempx-2,tempy,tempx,tempy-2,clr);
			DrawLine(RpCRT,tempx,tempy-2,tempx+2,tempy,clr);
		}
	}

	if (!dl) {
		if (AWG9_MODE==AWG9_RWS_AUTHENTIC) return 0;
		if (AWG9_MODE==AWG9_RWS_NARROW) return 0;
		if (AWG9_MODE==AWG9_RWS_WIDE) return 0;
		if (AWG9_MODE==AWG9_RWS_MEDIUM) return 0;
	}

	/* Do Alt. */
	alt = tempps->alt/(1000/4);

	if ((alt >5) && (alt<=15))
		RpPrint(RpCRT,tempx-6,tempy-2,"1");
	if ((alt >15) && (alt<=25))
		RpPrint(RpCRT,tempx-6,tempy-2,"2");
	if ((alt >25) && (alt<=35))
		RpPrint(RpCRT,tempx-6,tempy-2,"3");
	if ((alt >35) && (alt<=45))
		RpPrint(RpCRT,tempx-6,tempy-2,"4");
	if ((alt >45) && (alt<=55))
		RpPrint(RpCRT,tempx-6,tempy-2,"5");


	if (AWG9_MODE != AWG9_PDSTT) {
		found=0;
		if (CheckMode(AWG9_MODE,AWG9_TWSA)) {
			for (i=0; i<6; i++) {
				if (FIRINGORDERLIST[i]->i == tc->i) {
					found=1;
					break;
				}
			}
			ipos=i+1;
		} else {
			for (i=0; i<5; i++) {
				// Check if Designated
				if (DESIG_TARG[i].status == 1) {
					cnt++;
					if (DESIG_TARG[i].target == tc->i) {
						found=1;
						break;
					}
				}
			}
			ipos=cnt;
		}

		if (found) {
			itoa((ipos),s,10);
			RpPrint(RpCRT,tempx+4,tempy-2,s);
		}
	}

	switch (d) {
		case 0:
			x1 =
			x2 = tempx;
			y1 = tempy;
			y2 = tempy - taillength;
		break;
		case 1:
		case 2:
		case 3:
			y1 = tempy;
			y2 = tempy - taillength;
			x1 = tempx;
			x2 = tempx + taillength;
		break;
		case 4:
			x1 = tempx;
			x2 = tempx + taillength;
			y1 = tempy;
			y2 = tempy;
		break;
   		case 5:
   		case 6:
   		case 7:
      		x1 = tempx;
      		y1 = tempy;
      		x2 = tempx + taillength;
      		y2 = tempy + taillength;
    	break;
   		case 8:
	  		x1 =
	  		x2 = tempx;
	  		y1 = tempy;
	  		y2 = tempy + taillength;
		break;
   		case 9:
   		case 10:
   		case 11:
	  		x1 = tempx;
	  		y1 = tempy;
	  		x2 = tempx - taillength;
	  		y2 = tempy + taillength;
		break;
   		case 12:
	  		x1=tempx;
	  		y1=tempy;
	  		x2 = tempx - taillength;
	  		y2 = tempy;
		break;
		case 13:
   		case 14:
   		case 15:
	  		x1=tempx;
	  		y1=tempy;
	  		x2=tempx - taillength;
	  		y2=tempy - taillength;
  		break;
   }
   DrawLine (RpCRT,x1,y1,x2,y2,clr);
}

