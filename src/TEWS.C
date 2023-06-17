//************************************************************************
//*
//*    TEWS.C
//*
//*    Threat Warning System
//*
//*    Author: W. Becker
//*
//*    F-15 III
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//*************************************************************************
#include	<stdio.h>
#include	<stdlib.h>
#include "types.h"
#include "library.h"
#include "keycodes.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "math.h"
#define RADARON 0x100


#define  CLR0        0
#define  CLR1        0x52
#define  CLR2        0x54
#define  CLR3        0x56

#define  NM40        1900        // 40 NM in short coords
#define  NM80        3800        // 80 NM in short coords

#define  SPR_DIAMOND    0        // sprite indices
#define  SPR_SQUARE     2
#define  SPR_SMALLSQR   4

#define  MAXTHREATS  50
#define  MAXMISSILE  10
#define  MAXSYMBOLS  10

#define  THREAT_RL   0           // radar launch
#define  THREAT_PL   1           // plane launch
#define  THREAT_3AT  2           // AAA   track
#define  THREAT_RT   3           // radar track

#define  TRNG10      0
#define  TRNG20      1
#define  TRNG40      2

//////////////////////////////////////////////////////////////////////////

extern RPS     *RpCRT;
extern COORD   Px,Py;
extern int     OurRoll, OurPitch, OurHead, OurAlt;
extern UWORD   TEWSprites[];
extern UWORD   FTicks;

static int  SYMBL_T;
static int  MISSL_T;
static int  TOTAL_T;

typedef struct
{
	int type;
	int index;
} thr;

thr far THREATS[MAXTHREATS];
thr far SYMBOLS[MAXSYMBOLS];

static int  MTHREATS[MAXMISSILE];
static int  QUADLITE[4] = { 0, 0, 0, 0 };

static int  BITPERPIX_X[] = { 23, 45, 90 };  // bits/pix in x
static int  BITPERPIX_Y[] = { 25, 50,100 };  // bits/pix in y
static int  MAXBITS    [] = { 475,950 ,1900};// max bits on display
static int  OORBITS    [] = { 593,1188,2375};// out of range circle
static int  CRT_X, CRT_Y;

static int  TEWS_RNG = TRNG40;
// static int TEWS_RNG = TRNG10;
static int  flash_off=0;

int    MLAUNCH;
int    AIDETECT;
int    JAMMER_ACTIVE = 0;
int    JAMMER[2]     = {1, 0};

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);


//************************************************************************
//
// TEWS_SYS
//
//************************************************************************
TEWS_SYS ()
{
	int i;
	TOTAL_T = 0;

	for (i=0;i<4;i++)
		if (QUADLITE[i]>0) QUADLITE[i]--;

	RadarThreats();
	PlaneThreats();
	MissileThreats();
	SortThreats();
}

//***************************************************************************
int RadarThreats()
{
	int i, w;
	struct Radars *p;
	struct RadarData far *rri;
	struct MissileData far *tempmissile;

	for (i=JAMMER_ACTIVE=0; i<NRADAR; i++) {
		p = Rdrs + i;
		if (p->rclass==RL_NONE) continue;
		if (p->status&DESTROYED) continue;
		if (!(p->status&RTRACK)) continue;
//		if (!threat_in_rng (p->x,p->y)) continue;
		if (TOTAL_T >= MAXTHREATS) break;
		// SCOTT
		rri = GetRadarPtr(p->rclass);
		if (rri->detectiontype!=RADAR) continue;
		tempmissile = GetMissilePtr(rri->missiletype);
		w = tempmissile->homing;                 // weapon type
		THREATS[TOTAL_T].type  = (w==GUN)?THREAT_3AT:THREAT_RT;
		THREATS[TOTAL_T++].index = i;
		if (rri->missiletype==ML_NONE) continue;
		if (JAMMER[0]) JAMMER_ACTIVE = 1;
	}
}

//***************************************************************************
int PlaneThreats()
{
	int i;
	struct PlaneData far *tempplane;

	for (AIDETECT=i=0; i<NPLANES; i++) {
		if (!(ps[i].status & ACTIVE)) continue;
		if (bv[i].target != -1) continue;
		if (!(ps[i].status & LOCK)) continue;
		if (ps[i].status & CRASH) continue;
		if (!ps[i].speed) continue;
		if (!threat_in_rng (ps[i].x,ps[i].y)) continue;
		tempplane = GetPlanePtr(ps[i].type);
		if (tempplane->rclass==RL_NONE) continue;
		if (!(bv[i].behaveflag & RADARON)) continue;  /*  added so that planes with radar off don't show up  */
		if (TOTAL_T >= MAXTHREATS) break;
		AIDETECT = 1;
		THREATS[TOTAL_T].type  = THREAT_PL;
		THREATS[TOTAL_T++].index = i;
   }
}


//***************************************************************************
int MissileThreats()
{
	int i;
	struct MissileData far *tempmissile;

	for (MLAUNCH=MISSL_T=i=0; i<NSAM; i++) {
		tempmissile = GetMissilePtr(ss[i].type);
		if (!ss[i].dist) continue;
		if (!threat_in_rng((int)(ss[i].x>>HTOV),(int)(ss[i].y>>HTOV))) continue;
		if (ss[i].srctype==1 && ss[i].source==MAXPLANE) continue;
		if (ss[i].srctype==0) MLAUNCH=1;
		if (tempmissile->homing != RDR_1 &&
			tempmissile->homing != RDR_2) continue;
		launch_chk(i);
		if (MISSL_T >= MAXMISSILE) break;
		MTHREATS[MISSL_T++] = i;
   }
}


//***************************************************************************
int SortThreats()
{
	int i,j;

	for (j=THREAT_RL,SYMBL_T=0; j<=THREAT_RT; j++)
		for (i=0; i<TOTAL_T; i++)
			if (THREATS[i].type == j)
				if (SYMBL_T >= MAXSYMBOLS)
					return 0;
				else
					SYMBOLS[SYMBL_T++] = THREATS[i];
}


//***************************************************************************
static launch_chk(int i)
{
	int j;

	if (ss[i].srctype) return 0;

	for (j=0; j<TOTAL_T; j++)
		if (THREATS[j].type == THREAT_RT && THREATS[j].index== ss[i].source)
			THREATS[j].type = THREAT_RL;
}


//***************************************************************************
static threat_in_rng(x, y)
{
	return ((unsigned)Dist2D (x-Px,Py-y) <= NM80);
}

//************************************************************************
//
// TEWS_RANGE
//
//************************************************************************
TEWS_RANGE(int i)
{
   TEWS_RNG += i;

   if (TEWS_RNG > TRNG40) TEWS_RNG = TRNG40;
   if (TEWS_RNG < TRNG10) TEWS_RNG = TRNG10;
}

//************************************************************************
//
// TEWS_SPOTTED
//
//************************************************************************
TEWS_SPOTTED(int i)
{
	unsigned a, quad;
	a  = ARCTAN(Rdrs[i].x-Px,Py-Rdrs[i].y);  // angle to radar
	a -= OurHead;                            // add our heading
	quad = (a+0x2000)/0x4000;
	QUADLITE[quad] = FrameRate;
}

//************************************************************************
//
// TEWS_GFX
//
//************************************************************************
int TEWS_GFX()
{
	int f,i;

	if (Damaged&D_ECM) {
		RectFill(RpCRT,0,0,RpCRT->Width1+1,RpCRT->Length1+1, BLACK);
		return(1);
	}


	f = GetFont (RpCRT,0);
	SetFont (RpCRT, F_3x4);
	RpCRT->APen = 0x13;
	flash_off ^= 1;

	RectFill(RpCRT,0,0,RpCRT->Width1+1,RpCRT->Length1+1,CLR0);

	concentric();
	arrows();

	// show main radar threats

	for (i=SYMBL_T-1;i>=0;i--)
		switch (SYMBOLS[i].type) {
			case THREAT_RL:
				show_radar(SYMBOLS[i].index,1);
			break;
			case THREAT_PL:
				show_plane(SYMBOLS[i].index);
			break;
			case THREAT_3AT:
			case THREAT_RT:
				show_radar(SYMBOLS[i].index,0);
			break;
		}

	show_missiles();        // show missile threats
	show_range();			// show TEWS range
	show_jamming();			// show jamming status

	SetFont(RpCRT,f);
	return 1;
}

//***************************************************************************
int concentric()
{
	int i,r;

	for (i=0,r=5;i<4;i++,r+=5)
		draw_circle (RpCRT,RpCRT->Width1/2,RpCRT->Length1/2,r,CLR1,0);

	DrawLine(RpCRT,(RpCRT->Width1/2),(RpCRT->Length1/2)-5*4,
				(RpCRT->Width1/2),(RpCRT->Length1/2)+5*4,CLR1);
	DrawLine(RpCRT,(RpCRT->Width1/2)-6*4,(RpCRT->Length1/2),
				(RpCRT->Width1/2)+6*4,(RpCRT->Length1/2),CLR1);
}


//***************************************************************************
static arrows()
{
	int x,y,color;
#ifdef YEP
	if (QUADLITE[0]) Message("QUAD 1");
	if (QUADLITE[1]) Message("QUAD 2");
	if (QUADLITE[2]) Message("QUAD 3");
	if (QUADLITE[3]) Message("QUAD 4");
#endif

	if (QUADLITE[0]) {
		ChangeColor(RpCRT,0,0,(RpCRT->Width1),(RpCRT->Length1/2),
			CLR1,CLR3);
	}
	if (QUADLITE[1]) {
		ChangeColor(RpCRT,(RpCRT->Width1/2),0,(RpCRT->Width1),(RpCRT->Length1),
			CLR1,CLR3);
	}
	if (QUADLITE[2]) {
		ChangeColor(RpCRT,0,(RpCRT->Length1/2),(RpCRT->Width1),(RpCRT->Length1),
			CLR1,CLR3);
	}
	if (QUADLITE[3]) {
		ChangeColor(RpCRT,0,0,(RpCRT->Width1/2),(RpCRT->Length1),
			CLR1,CLR3);
	}



#ifdef YEP
	// draw north arrow
	color = QUADLITE[0]?CLR3:CLR1;

	x = (RpCRT->Width1/2)-1;
	y = (RpCRT->Length1/2)-5*4-5;

	DrawLine(RpCRT,x-2,y  ,x+2,y  ,color);
	DrawLine(RpCRT,x-1,y+1,x+1,y+1,color);
	DrawDot(RpCRT,x  ,y+2,        color);

	// draw south arrow

	color = QUADLITE[2]?CLR3:CLR1;

	x = (RpCRT->Width1/2)-1;
	y = (RpCRT->Length1/2)+5*4+4;

	DrawLine(RpCRT,x-2,y  ,x+2,y  ,color);
	DrawLine(RpCRT,x-1,y-1,x+1,y-1,color);
	DrawDot (RpCRT,x  ,y-2,        color);

	// draw west arrow
	color = QUADLITE[3]?CLR3:CLR1;

	x = (RpCRT->Width1/2)-6*4-5;
	y = (RpCRT->Length1/2)-1;

	DrawLine(RpCRT,x  ,y-2,x  ,y+2,color);
	DrawLine(RpCRT,x+1,y-1,x+1,y+1,color);
	DrawDot (RpCRT,x+2,y  ,        color);

	// draw east arrow
	color = QUADLITE[1]?CLR3:CLR1;
	x = (RpCRT->Width1/2)+6*4+4;
	y = (RpCRT->Length1/2)-1;

	DrawLine(RpCRT,x  ,y-2,x  ,y+2,color);
	DrawLine(RpCRT,x-1,y-1,x-1,y+1,color);
	DrawDot (RpCRT,x-2,y  ,        color);
#endif
}


//***************************************************************************
int show_range ()
{
	static RANGES[] = { 10, 20, 40 };
	char   s[4];
	int    l;

	l = pstrlen(RpCRT,itoa(RANGES[TEWS_RNG],s,10));
	RpPrint (RpCRT,64-l,4,s);
}


//***************************************************************************
int show_missiles ()
{
	int i,j;
	for (i=0;i<MISSL_T;i++) {
		j = MTHREATS[i];
		convert_xy ((int)(ss[j].x>>HTOV),(int)(ss[j].y>>HTOV));
		ClipSprite (RpCRT,CRT_X-1,CRT_Y-1,TEWSprites[SPR_SMALLSQR]);
   }
}


//***************************************************************************
int show_radar (int i,int flash)
{
	int  x;
	char s[5];
	struct RadarData far *tempradar;

	convert_xy (Rdrs[i].x,Rdrs[i].y);
	if (flash && flash_off) return 0;

	ClipSprite(RpCRT,CRT_X-5,CRT_Y-5,TEWSprites[SPR_SQUARE]);
	tempradar = GetRadarPtr(Rdrs[i].rclass);
	x = (pstrlen(RpCRT,itoa(tempradar->tewsnum,s,10))-1)/2;
	RpPrint(RpCRT,CRT_X-x,CRT_Y-2,s);
}


//***************************************************************************
int show_plane(int i)
{
	int  start,x;
	char s[5];
	struct PlaneData far *tempplane;
	struct RadarData far *tempradar;

	tempplane = GetPlanePtr(ps[i].type);

	convert_xy(ps[i].x,ps[i].y);
	ClipSprite(RpCRT,CRT_X-5,CRT_Y-5,TEWSprites[SPR_DIAMOND]);
	tempradar = GetRadarPtr(tempplane->rclass);
	x = (pstrlen(RpCRT,itoa(tempradar->tewsnum,s,10))-1)/2;
	RpPrint(RpCRT,CRT_X-x,CRT_Y-2,s);
}


//***************************************************************************
int show_jamming()
{
	static ticks=0, off=0;

	if (JAMMER_ACTIVE && !off)
		RpPrint (RpCRT,8,8,"JAMMER ACTIVE");

	ticks += FTicks;
	if (ticks > 20) {
		ticks = 0;
		off ^= 1;
	}
}


//***************************************************************************
int convert_xy(x,y)
{
	int dx,dy,r,a;

	dx    = x  - Px;
	dy    = Py - y;
	r     = Dist2D (dx,dy);

	if (r > MAXBITS[TEWS_RNG]) {              // out of range-use outside
		a  = ARCTAN (dy,dx);                   // angle to source
		dx = ACosB  (OORBITS[TEWS_RNG],a);     // new dx
		dy = ASinB  (OORBITS[TEWS_RNG],a);     // new dy
	}

	CRT_X = cosX (OurHead,dx) - sinX (OurHead,dy);
	CRT_Y = cosX (OurHead,dy) + sinX (OurHead,dx);
	CRT_X =  CRT_X/BITPERPIX_X[TEWS_RNG] + (RpCRT->Width1/2) - 1;
	CRT_Y = -CRT_Y/BITPERPIX_Y[TEWS_RNG] + (RpCRT->Length1/2) - 1;
}
