//************************************************************************
//*
//*    HUD2.C
//*
//*    Head-Up Display
//*
//*    Author: W. Becker
//*
//*    F-15 III
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//*************************************************************************
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"

//*************************************************************************

#define  DEG         182
#define  HUD_CTR     160
#define  HOR_CTR     58

#define  HDG_WINL    76
#define  HDG_WINW    13
#define  HDG_WINX	 123
#define  HDG_WINY    13


#define  HDG_CTR     HDG_WINL/2
#define  HDG_TGAP    4
#define  HDG_DGAP    HDG_TGAP * 5

#define  HI_CTR      HUD_CTR
#define  HI_Y        HDG_WINY+HDG_WINW
#define  HI_XMIN     HDG_WINX
#define  HI_XMAX     HDG_WINX+HDG_WINL


#define  PL_WINX     104
#define  PL_WINY     23
#define  PL_WINL     112
#define  PL_WINW     63
#define  PL_CTR_X    HUD_CTR-PL_WINX
#define  PL_CTR_Y    HOR_CTR-PL_WINY
#define  PL_GAP      20

#define  WL_X        HUD_CTR-1
#define  WL_Y        58


#define  BRKX_X      HUD_CTR
#define  BRKX_Y      51
#define  BRKX_S      17


#define  VV_X        HUD_CTR-PL_WINX
#define  VV_Y        WL_Y   -PL_WINY
#define  TRNG_X      92
#define  TRNG_Y      20
#define  TIME_X      100
#define  TIME_Y      26
#define  RB_WINX     200
#define  RB_WINY     30
#define  RB_WINW     40
#define  RB_WINL     10
#define  GUNCROSS_X  HUD_CTR
#define  GUNCROSS_Y  25
#define  PDT_ALT_X   212
#define  PDT_ALT_Y   58
#define  PDT_RNG_X   211
#define  PDT_RNG_Y   64
#define  TDBOX_L     15
#define  TDBOX_W     13
#define  TB_WINX     100
#define  TB_WINY     15
#define  TB_WINL     119
#define  TB_WINW     73

int      TB_CTRX=    HUD_CTR-1;
int      TB_CTRY=    51;

#define  TSPEED      (6000>>KH2BF)              // bullet speed

//***************************************************************************
extern RPS        *Rp1, RpX2, *Rp2, *Rp3D, RpX3D, *RpHud, RpHudX;
extern UWORD      HudSprite1;
extern UWORD      HudSprite2;
extern UWORD      HudSprite3;
extern UWORD      HudSprite4;
extern UWORD      HudSprite5;

extern int        OurHead, OurPitch, OurRoll, Gees;
extern unsigned   Alt,OurAlt;
extern int        MinKnots,Knots;
extern int        PitchAngle,ThrustI,VVI;
extern unsigned   FTicks;
extern int        Rtime;
extern int        DeClutter;

extern int        VM[3][3];
extern int        View,ViewZ;
extern long       PxLng,PyLng,ViewX,ViewY;
extern int        Px,Py;
extern int        sightX,sightY;
extern char       Txt[];
extern long       RadarAlt;
extern int        Speedy;
extern int		  MasterArmSW;
extern  int     Fticks;
extern  int		Gtime;
extern int SelectedTarg;

//***************************************************************************
static char   *deg_prt (int deg);
static long    row3    (int r, int dx, int dy, int dz);

static RPS  HDGRpX = { 1,HDG_WINX,HDG_WINY,HDG_WINL-1,HDG_WINW-1,JAM1,HUD_COLOR,BLACK,1};
       RPS *HDGRp  = &HDGRpX;

static RPS  PLRpX  = { 1,PL_WINX,PL_WINY,PL_WINL-1,PL_WINW-1,JAM1,HUD_COLOR,BLACK,1};
       RPS *PLRp   = &PLRpX;

static RPS  RBRpX  = { 1,RB_WINX,RB_WINY,RB_WINL-1,RB_WINW+7,JAM1,HUD_COLOR,BLACK,1};
       RPS *RBRp   = &RBRpX;

RPS  TDBRpX = { 1,TB_WINX,TB_WINY,TB_WINL-1,TB_WINW-1,JAM1,HUD_COLOR,BLACK,1};
RPS *TDBRp  = &TDBRpX;

static int  VEL_VECT_Y;


//***************************************************************************
int HUD()
{
   RpX2.APen=HUD_COLOR;
   if (Damaged&D_HUD) return 0;
   HUD_standard();
   HUD_master();
}


//***************************************************************************
int HUD_master ()
{
	switch (MODE_MASTER) {
		case MASTER_NAV:
			HUD_nav();
		break;
		case MASTER_AA :
			HUD_aa();
		break;
   }
}

//***************************************************************************
int	HUD_standard()
{
	waterline();
	vel_vector();
	pitch_ladder();
	hdg_scale();
}

//***************************************************************************
int waterline()
{
	if (DeClutter) return 0;
	DrawLine(Rp2,WL_X-12,WL_Y,WL_X-3,WL_Y,HUD_COLOR);
	DrawLine(Rp2,WL_X-3,WL_Y,WL_X-3,WL_Y+2,HUD_COLOR);
	DrawLine(Rp2,WL_X+3,WL_Y,WL_X+3,WL_Y+2,HUD_COLOR);
	DrawLine(Rp2,WL_X+3,WL_Y,WL_X+12,WL_Y,HUD_COLOR);
	DrawLine(Rp2,WL_X,WL_Y-30,WL_X,WL_Y-24,HUD_COLOR);
	DrawLine(Rp2,WL_X-3,WL_Y-27,WL_X+3,WL_Y-27,HUD_COLOR);
}

//***************************************************************************
int vel_vector()
{
	if (MODE_MASTER==MASTER_NAV) {
		VEL_VECT_Y = VV_Y + (PitchAngle>>6);
		if (DeClutter) return 0;
		ClipSprite (PLRp, VV_X-5, VEL_VECT_Y-4, HudSprite1);
	}
}


//***************************************************************************
int VSLScan()
{
	static int scanx=WL_X;
	static int scany=(WL_Y-10);
	static int dir=1;
	static int dir2=1;

	if (dir) {
		scany+=10;
		if (dir2)
			scanx+=2;
		else
			scanx-=2;
	} else {
		scany-=10;
		if (dir2)
			scanx+=2;
		else
			scanx-=2;
	}

	if (dir && (scany>(WL_Y-10)))
		dir2=0;

	if (!dir && (scany<(WL_Y-10)))
		dir2=1;

	if (scany>((WL_Y-10)+25)) {
		dir=0;
		scanx=WL_X;
	}

	if (scany<((WL_Y-10)-25)) {
		dir=1;
		scanx=WL_X;
	}

	diamondbox(Rp2,scanx,scany);
}

//***************************************************************************
int hdg_scale()
{
	int      deg10, off, i, x;
	unsigned deg;
	unsigned deg2;
	int ang;
	char     s[4];

	deg   = (unsigned)OurHead/(DEG/2);     // convert to 1/2 degrees
	deg10 = deg / 20;                      // nearest 10 degree
	off   = deg % 20;                      // 10 degree offset
	deg10--;                               // dumb compiler does 2 divs!!!

	// display tic marks/headings

	DrawLine(HDGRp,0,1,HDG_WINL-1,1,HUD_COLOR);

	for (x=HDG_CTR-HDG_DGAP*2-off,i=0;x<HDG_WINL;x+=HDG_TGAP,i+=HDG_TGAP) {
		DrawDot  (HDGRp,x,2,HUD_COLOR);
		if (i==HDG_DGAP) {
			DrawDot (HDGRp,x,3,HUD_COLOR);
			RpPrint (HDGRp,x?x-3:x-4,4,deg_prt(deg10++));
			i=0;
		}
	}
	hdg_indicator();
}


//***************************************************************************
char *deg_prt(int deg)
{
	static char t[3];
    char s[3];

	if (deg <= 0) deg += 36;
	else
		if (deg > 36) deg -= 36;

	t[0] = '\0';

	if (strlen (itoa (deg,s,10)) == 1)
		strcat (t,"0");
	strcat (t,s);
	return t;
}

//***************************************************************************
int hdg_indicator()
{
	int ang;

	ang = rng(((AutoHead-OurHead)/DEG)*2+HI_CTR,HI_XMIN,HI_XMAX);

	DrawLine (Rp2, ang, HI_Y, ang-2, HI_Y+2, HUD_COLOR);
	DrawLine (Rp2, ang, HI_Y, ang+2, HI_Y+2, HUD_COLOR);
}


//***************************************************************************
int pitch_ladder ()
{
	int i, base_deg, deg, off, dx, dy;
	int pts[4][2];

	if (DeClutter) return 0;

	// find delta x/y between pitch lines

	pts[0][0] =
	pts[1][0] = 0;
	pts[0][1] = PL_GAP;
	pts[1][1] = ((OurPitch % (DEG*5)) * PL_GAP)/(DEG*5);

	for (i=0;i<2;i++)
		rotate_pt (pts[i],pts[i+2],OurRoll);

	// closest 5 degrees

	base_deg = (OurPitch / (DEG*5)) * 5;

	for (i=-2;i<=2;i++) {
		dx  = -pts[2][0]*i+pts[3][0]+PL_CTR_X;
		dy  = -pts[2][1]*i+pts[3][1]+PL_CTR_Y;
		deg = base_deg + (i * 5);

		switch (deg) {
			case 0:
				horiz_line (deg, dx, dy);
			break;
			case  90:
			case -90:
				ClipSprite (PLRp,dx-4,dy-4,deg<0?HudSprite3:HudSprite4);
			break;
			default:
				HUDPitchLine(deg, dx, dy);
			break;
      }
   }

}

//***************************************************************************
int horiz_line (deg, xc, yc)
{
	int         i;
	int         pts [6][2];
	static int  hpts[6][2] = {{9,0},{30,0},{38,3},{-9,0},{-30,0},{-38,3}};


	// rotate horizon line

	for (i=0;i<6;i++)
		rotate_pt (hpts[i],pts[i],OurRoll);

	// draw horizon line

	DrawLine (PLRp,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
	DrawLine (PLRp,xc+pts[3][0],yc+pts[3][1],xc+pts[4][0],yc+pts[4][1],HUD_COLOR);
}

//***************************************************************************
int HUDPitchLine(deg, xc, yc)
{
	int         i,l;
	int         pts [10][2];
	char		s[10];
	static int  hpts[2][2] = {{-27,0},{27,0}};
	static int  hpts2[6][2] = {{-27,0},{-8,0},{-6,0},{6,0},{8,0},{27,0}};
	static int  hpts3[6][2] = {{-27,0},{-5,0},{5,0},{27,0},{-27,6},{27,6}};
	static int  hpts4[10][2] = {{-27,0},{-20,0},{-18,0},{-5,0},{5,0},{18,0},
								{20,0},{27,0},{27,6},{-27,6}};

	l = (pstrlen(PLRp,itoa ((deg),s,10))-1)/2;

	// rotate horizon line
	if (deg >0) {
		if (!(deg % 10)) {
			for(i=0; i<7; i++)
				rotate_pt (hpts3[i],pts[i],OurRoll);
				DrawLine (PLRp,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
	  			DrawLine (PLRp,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
				RpPrint (PLRp,xc+pts[4][0]-l,yc+pts[4][1]-3,s);
				RpPrint (PLRp,xc+pts[5][0]-l,yc+pts[5][1]-3,s);
		}
		else {
			if (MODE_MASTER==MASTER_NAV) {
				for (i=0;i<7;i++)
					rotate_pt (hpts3[i],pts[i],OurRoll);
			   	DrawLine (PLRp,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
 		   		DrawLine (PLRp,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
				RpPrint (PLRp,xc+pts[4][0]-l,yc+pts[4][1]-3,s);
				RpPrint (PLRp,xc+pts[5][0]-l,yc+pts[5][1]-3,s);
			}
		}
	}

	if (deg <0) {
		if (!(deg % 10)) {
			for (i=0; i<10; i++)
				rotate_pt (hpts4[i],pts[i],OurRoll);
			DrawLine (PLRp,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
			DrawLine (PLRp,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
 		   	DrawLine (PLRp,xc+pts[4][0],yc+pts[4][1],xc+pts[5][0],yc+pts[5][1],HUD_COLOR);
 		   	DrawLine (PLRp,xc+pts[6][0],yc+pts[6][1],xc+pts[7][0],yc+pts[7][1],HUD_COLOR);
			RpPrint  (PLRp,xc+pts[8][0]-l,yc+pts[8][1]-3,s);
			RpPrint  (PLRp,xc+pts[9][0]-l,yc+pts[9][1]-3,s);
		}
		else {
			if (MODE_MASTER==MASTER_NAV) {
				for (i=0; i<10; i++)
					rotate_pt (hpts4[i],pts[i],OurRoll);
			   	DrawLine (PLRp,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
				DrawLine (PLRp,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
 		   		DrawLine (PLRp,xc+pts[4][0],yc+pts[4][1],xc+pts[5][0],yc+pts[5][1],HUD_COLOR);
 		   		DrawLine (PLRp,xc+pts[6][0],yc+pts[6][1],xc+pts[7][0],yc+pts[7][1],HUD_COLOR);
				RpPrint  (PLRp,xc+pts[8][0]-l,yc+pts[8][1]-3,s);
				RpPrint  (PLRp,xc+pts[9][0]-l,yc+pts[9][1]-3,s);
			}
		}
	}
}

//***************************************************************************
int pitch_line (deg, xc, yc)
{
	int         i, l, d;
	int         pts [8][3];
	int         rpts[8][3];
	char        s[10];
	static int  hpts0[4][2] = {{ 0, 0 }, { 12, 0 }, { 12, 3 }, { 20, 0 }};
	static int  hpts1[4][2] = {{ 0, 0 }, { 12, 0 }, { 12,-3 }, { 20, 0 }};

	d = deg;
	if (deg > 90) deg = deg-180;
	if (deg <-90) deg = deg+180;

	// build pitch line

	for (i=0;i<4;i++) {
		rotate_pt (deg>=0?hpts0[i]:hpts1[i],pts[i],deg*DEG/2);
		pts[i  ][0]+= 5;
		pts[i+4][0] = -pts[i][0];
		pts[i+4][1] =  pts[i][1];
	}

	for (i=0;i<8;i++)
		rotate_pt (pts[i],rpts[i],OurRoll);

	// draw pitch line

	l = (pstrlen(Rp2,itoa (abs(deg),s,10))-1)/2;

	for (i=0;i<2*4;i+=4) {
		if (d > 0)
			DrawLine(PLRp,xc+rpts[i][0],yc+rpts[i][1],
					xc+rpts[i+1][0],yc+rpts[i+1][1],HUD_COLOR);
		else
			dashline (PLRp,xc+rpts[i][0],yc+rpts[i][1],
					xc+rpts[i+1][0],yc+rpts[i+1][1],HUD_COLOR, 2);
		DrawLine (PLRp,xc+rpts[i+1][0],yc+rpts[i+1][1],
					xc+rpts[i+2][0],yc+rpts[i+2][1],HUD_COLOR);
		RpPrint  (PLRp,xc+rpts[i+3][0]-l,yc+rpts[i+3][1]-3,s);
	}
}

//***************************************************************************
int dashline (RPS *rp, int x0, int y0, int x1, int y1, int clr, int pix)
{
	int dx,dy,xi,yi,d,n,c,f;
	int *x,*y,*t;

	if (x0 > x1) {
		n = x0;
		x0 = x1;
		x1 = n;
		n = y0;
		y0 = y1;
		y1 = n;
	}

	xi = 1;
	yi = (y1 < y0)?-1:1;
	dx = x1-x0;
	dy = abs(y1-y0);
	x  = &x0;
	y  = &y0;

	if (dy > dx) {
		n = dx;
		dx = dy;
		dy = n;
		n = xi;
		xi = yi;
		yi = n;
		t = x;
		x = y;
		y = t;
	}

	f = 1;
	c = pix;
	n = dx;
	d = -((n+1)/2);

	while (n--) {
		if (f) DrawDot (rp,x0,y0,clr);
		if (!--c) {
			f ^= 1;
			c  = pix;
		}
		*x+=xi;
		d+=dy;

		if (d > 0) {
			d-=dx;
			*y+=yi;
		}
	}
}


//***************************************************************************
//
// HUD_nav - Display HUD symbology related to NAV master modes
//
//***************************************************************************
int HUD_nav ()
{
}

//***************************************************************************
//
// HUD_aa - Display HUD symbology related to AA master modes
//
//***************************************************************************
int HUD_aa()
{
	static int off=1;

	missile_cnt();
	bst_circle();
	if (AWG9_MODE == AWG9_VSL)
		VSLScan();

	if (SHOOT) {
		off ^= 1;
	} else
		off = 1;

	if (off) {
		range_bar();
		PDT_range();
		PDT_closure();
		TD_box();
	}

	gun_reticle();
	break_x();
}




//***************************************************************************
int missile_cnt ()
{
	char   t[10],s[6],l[5];
	static char mo[2]="",so[2]="";

	SetFont(Rp2,F_3x5B);
//	if (!(Status & WHEELSUP)) return 0;

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

	RpPrint(Rp2, 165-(pstrlen(Rp2,l)), 78, l);
	RpPrint(Rp2, 164-(pstrlen(Rp2,t)), 86, t);
	if (!MasterArmSW) {
		DrawLine(Rp2,152,78,168,87,HUD_COLOR);
		DrawLine(Rp2,168,78,152,87,HUD_COLOR);
	}
}

//***************************************************************************
int get_count()
{
	int cnt=0, i;

	for(i=0;i<F14_STATION_NUM;i++)
		switch (F14_STATION[i]) {
			case W_AIM_9:
				if (MODE_AA == AA_SRM) cnt++;
			break;
			case W_AIM_54:
				if (MODE_AA == AA_LRM) cnt++;
			break;
      		case W_AIM_120:
      		case W_AIM_7:
				if (MODE_AA == AA_MRM) cnt++;
			break;
		}
	return cnt;
}

//***************************************************************************
int ase_circle ()
{
	int d=0;

	if (ASE_RADIUS <= 0) return 0;
	if (PW == W_AIM_120) d = (ASE_RADIUS < 25) ? 2 : 3;
	draw_circle(Rp2,WL_X,WL_Y-5,ASE_RADIUS,HUD_COLOR,d);
}

//************************************************************************
int bst_circle ()
{
   if (AWG9_MODE != AWG9_BORESIGHT) return 0;
   draw_circle (Rp2,WL_X,WL_Y,12,HUD_COLOR,0);
}

//************************************************************************
#pragma optimize ("",off)     // fix for C600 compiler bug
int range_bar ()
{
	int  i,y1,y2,r, v;
	char s[4];
	int mindist=0;

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE != AWG9_STANDARD_TRACK)
		&&  (!CheckMode(AWG9_MODE,AWG9_TWSA))
		&& (!CheckMode(AWG9_MODE,AWG9_TWSM)) && (SelectedTarg==-1))
			return 0;

	// draw bar...

	for (i=0,y1=0; i<10; i++,y1+=5) {
		DrawLine(RBRp,1,y1,2,y1,HUD_COLOR);
		DrawLine(RBRp,1,y1+1,2,y1+1,HUD_COLOR);
	}

	// show ranges

	r = RANGES[RANGE];
	RpPrint(Rp2,RB_WINX+4,RB_WINY+1,itoa(r,s,10));
	RpPrint(Rp2,RB_WINX+4,RB_WINY+45,itoa(mindist,s,10));

	// show rmax/rmin

	if (PW != W_NONE) {
		y1 = RB_WINW - ((long)RB_WINW*ORDNANCE[PW].rmin/10)/r - 1;
		y2 = RB_WINW - ((long)RB_WINW*ORDNANCE[PW].rmax/10)/r - 1;
		DrawLine (RBRp,4,y1,8,y1,HUD_COLOR);
		DrawLine (RBRp,4,y2,8,y2,HUD_COLOR);
	}
}

#pragma optimize ("",on)     // fix for C600 compiler bug

int PDT_closure()
{
	int i,y1,y2,r,v;
	char s[10];

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE != AWG9_STANDARD_TRACK)
		&&  (!CheckMode(AWG9_MODE,AWG9_TWSA))
		&& (!CheckMode(AWG9_MODE,AWG9_TWSM)) && (SelectedTarg==-1))
			return 0;

	// draw bar

	for (i=0,y1=0; i<8; i++,y1+=5) {
		DrawLine(Rp2,114,30+y1,115,30+y1,HUD_COLOR);
		DrawLine(Rp2,114,31+y1,115,31+y1,HUD_COLOR);
	}
	RpPrint(Rp2,106,31,"10");
	RpPrint(Rp2,110,56,"0");
	RpPrint(Rp2,106,65,"-2");

	y1 = abs((1000-CLOSURE))/35;
	y1+=31;
	if (y1>63) y1=63;
	if (y1<31) y1=34;
	DrawDot(Rp2,117,y1,HUD_COLOR);
	DrawDot(Rp2,118,y1-1,HUD_COLOR);
	DrawDot(Rp2,119,y1-2,HUD_COLOR);
	DrawDot(Rp2,118,y1+1,HUD_COLOR);
	DrawDot(Rp2,119,y1+2,HUD_COLOR);
}

//************************************************************************
int PDT_range ()
{
	int i, y, r, r10;
	char s[10], txt[10];

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE != AWG9_STANDARD_TRACK)
		&&  (!CheckMode(AWG9_MODE,AWG9_TWSA))
		&& (!CheckMode(AWG9_MODE,AWG9_TWSM)) && (SelectedTarg==-1))
			return 0;

	// range carat

	y = RB_WINW-((long)RB_WINW*PDT_RNG)/BRANGES[RANGE]-1;

	if (y >= 0 && y <= RB_WINW) {
		y += RB_WINY;
		DrawDot(Rp2, RB_WINX-2,y-2,HUD_COLOR);
   		DrawDot(Rp2, RB_WINX-1,y-1,HUD_COLOR);
		DrawDot(Rp2, RB_WINX-1,y+1,HUD_COLOR);
		DrawDot(Rp2, RB_WINX-2,y+2,HUD_COLOR);
		DrawDot(Rp2, RB_WINX,y,HUD_COLOR);
	}
}

//************************************************************************
static PDT_alt()
{
	int  l;
	char s[10];

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE != AWG9_STANDARD_TRACK)
		&&  (!CheckMode(AWG9_MODE,AWG9_TWSA))
		&& (!CheckMode(AWG9_MODE,AWG9_TWSM)) && (SelectedTarg==-1))
			return 0;

	l = pstrlen (Rp2,itoa(PDT_ALT_1000,s,10));

	RpPrint (Rp2,PDT_ALT_X+7-l, PDT_ALT_Y, s);
	RpPrint (Rp2,PDT_ALT_X+8  , PDT_ALT_Y, "-");
	RpPrint (Rp2,PDT_ALT_X+12 , PDT_ALT_Y, itoa(PDT_ALT_100 ,s,10));
}

//************************************************************************
int gun_reticle ()
{
	int sx, sy;

	if (MODE_AA != AA_GUN) return 0;

	sx = HUD_CTR + (sightX >> 4) - TB_WINX;
	sy = HOR_CTR + (sightY >> 4) - TB_WINY;
	DrawLine(TDBRp,sx,sy-10,sx,sy-6,HUD_COLOR);
	DrawLine(TDBRp,sx+6,sy,sx+10,sy,HUD_COLOR);
	DrawLine(TDBRp,sx,sy+6,sx,sy+10,HUD_COLOR);
	DrawLine(TDBRp,sx-6,sy,sx-10,sy,HUD_COLOR);
	DrawDot(TDBRp,sx,sy,HUD_COLOR);
}

//************************************************************************
int break_x()
{
	static int off = 0;

	if ((ASE_RADIUS != 0) || (MODE_AA==AA_LRM)) return 0;

	if (!off) {
		DrawLine(Rp2,BRKX_X-BRKX_S,BRKX_Y-BRKX_S,BRKX_X+BRKX_S,BRKX_Y+BRKX_S,HUD_COLOR);
		DrawLine(Rp2,BRKX_X-BRKX_S,BRKX_Y+BRKX_S,BRKX_X+BRKX_S,BRKX_Y-BRKX_S,HUD_COLOR);
	}

	off ^= 1;
}

//************************************************************************
int TD_box ()
{
	int x,y,l;
	static int off=0;

	if (!PDT_PROJ) return 0;

	x = PDT_PROJ_X + TB_CTRX - TB_WINX;
	y = PDT_PROJ_Y + TB_CTRY - TB_WINY;

	// limit TD box

	l = 0;

	if ((x < 8) || (x > TB_WINL) || (y < 0) || (y > (TB_WINW-5))) l++;

	if (l) {
		if (x<8) x=10;
		if (x>TB_WINL) x=TB_WINL - 4;
		if (y<0) y=4;
		if (y>(TB_WINW-5)) y=TB_WINW - 5;
	}

	diamondbox(TDBRp,x,y);
}

//************************************************************************
int steering_dot ()
{
	int x,y;

	if (!STEER_DOT) return 0;
	x = STEER_DOT_X + TB_CTRX - TB_WINX;
	y = STEER_DOT_Y + TB_CTRY - TB_WINY;
	DrawLine (TDBRp,x-1,y-1,x,y-1,HUD_COLOR);
	DrawLine (TDBRp,x-1,y  ,x,y  ,HUD_COLOR);
}


//***************************************************************************
static rectbox (RPS *RP, int x, int y, int w, int h)
{

   DrawLine (RP,x    ,y    ,    x,y+h-1,HUD_COLOR);
   DrawLine (RP,x    ,y    ,x+w-1,    y,HUD_COLOR);
   DrawLine (RP,x+w-1,y+h-1,    x,y+h-1,HUD_COLOR);
   DrawLine (RP,x+w-1,y+h-1,x+w-1,    y,HUD_COLOR);

}

//***************************************************************************
int diamondbox(RPS *RP, int x, int y)
{
	DrawLine(RP,x,y,x+4,y+4,HUD_COLOR);
	DrawLine(RP,x+4,y+4,x,y+8,HUD_COLOR);
	DrawLine(RP,x,y+8,x-4,y+4,HUD_COLOR);
	DrawLine(RP,x-4,y+4,x,y,HUD_COLOR);
}

//************************************************************************
draw_shoot_cue (int missile, RPS *rp, int x, int y, int color)
{
	switch (missile) {
		case W_AIM_9:
		case W_AIM_7:   // triangle
			DrawLine (rp,x  ,y  ,x-3,y+3,color);
			DrawLine (rp,x  ,y  ,x+3,y+3,color);
			DrawLine (rp,x-3,y+3,x+3,y+3,color);
		break;

		default:
		break;
	}
}

