#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"
#include "proto.h"

extern  struct RastPort *RpCRT;
int BEAM_HDG;			/* Beam Heading */
int BEAM_PIT;			/* Beam Pitch */
int	BEAM_ROL;			/* Beal Roll */
int AZSWEEP;			/* Azimuth sweep position */
int AZSWEEP_DIR;		/* Azimuth sweep direction */
int CUR_BAR;			/* Current bar position */
int	BAD_GUY;
int CLOSURE;

extern int		OurHead;
extern int		OurPitch;
extern int		OurRoll;
extern char		Txt[];

extern TARGET	DESIG_TARGF;
extern TARGET	*TARGFILE;
extern TARGET	*AGEDTARGFILE;
extern int		*TARGFILECNT;
extern int		*AGEDTARGFILECNT;



static int hpts[6][2] = {
   { -35, 0 }, { -9, 0 }, { 35, 0 }, { 12, 0 },
};


//***************************************************************************
DoDDD(int CrtNr)
{
	if (Damaged&D_AWG9) {
    	RectFill (RpCRT, 0,0, RpCRT->Width1+1,RpCRT->Length1+1, BLACK);
		return(1);
	}

    RectFill(RpCRT, 0,0, RpCRT->Width1+1,RpCRT->Length1+1, 21);
	if (!AWG9_SNIFF) {
		show_az_carat();
		ShowTargets();
		if ((AWG9_MODE==AWG9_PDSTT) || (AWG9_MODE==AWG9_STANDARD_TRACK)) {
			DDD_horizon_line();
			DDDWeaponRanges();
			draw_circle(RpCRT,DDD_CTR_X+1,DDD_CTR_Y-4,10,20,0);
		}
	}
}

//************************************************************************
int show_az_carat()
{
	int   x;

	if ((AWG9_MODE == AWG9_PDSTT) || (AWG9_MODE == AWG9_STANDARD_TRACK))
		x = DDD_CTR_X + DDD_W*(DESIG_TARGF.az/DEG)/120;
	else
		x = DDD_CTR_X + DDD_W*((BEAM_HDG/DEG) + AZSWEEP)/120;

//	x+=5;

	if (AZSWEEP_DIR) {
		DrawLine(RpCRT,x,3,x,AZ_CARAT_Y+7,19);
		DrawLine(RpCRT,x-1,3,x-1,AZ_CARAT_Y+7,20);
		DrawLine(RpCRT,x-2,3,x-2,AZ_CARAT_Y+7,20);
	} else {
		DrawLine(RpCRT,x+1,3,x+1,AZ_CARAT_Y+7,20);
		DrawLine(RpCRT,x+2,3,x+2,AZ_CARAT_Y+7,20);
		DrawLine(RpCRT,x,3,x,AZ_CARAT_Y+7,19);
	}
}


//***************************************************************************
int DDDWeaponRanges()
{
	int CurrentWeapon;
	int CurrentRange;
	int y1,y2;

	CurrentRange = RANGES[RANGE];
	CurrentWeapon = ARMT_get_priority();

	if (CurrentWeapon != W_NONE) {
		y1 = 56 - ((long)56*ORDNANCE[CurrentWeapon].rmin/10)/CurrentRange-2;
		y2 = 56 - ((long)56*ORDNANCE[CurrentWeapon].rmax/10)/CurrentRange-2;
		if (y2<4) y2=4;
		if (y1>58) y1=58;

		DrawLine(RpCRT,12,y1,18,y1,19);
		DrawLine(RpCRT,12,y2,18,y2,19);
	}
}

//***************************************************************************
int DopplerCheck(TARGET *t)
{
	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM))
		return 1;

	if ((t->targetclosure > -50) && (t->targetclosure<50)) {
		if (((t->h > 16016) && (t->h < 16926)) || ((t->h > -16926) && (t->h < -16016))) {
			return 0;
		}
	}
	return 1;
}

//************************************************************************
int ShowTargets()
{
	int i;

	// show aged targets
	for (i = 0; i < *AGEDTARGFILECNT; i++)
		if (DopplerCheck(AGEDTARGFILE + i))
			DDDTargetDraw(AGEDTARGFILE + i, 1);

	// show new contacts
	for (i = 0; i < *TARGFILECNT; i++)
		if (DopplerCheck(TARGFILE + i))
			DDDTargetDraw(TARGFILE + i, 0);
}

//************************************************************************
int DDDTargetDraw(TARGET *t, int age)
{
	int   d, clr;
	int   x1,y1,x2,y2,off;

	clr = 19 + age;

	// draw block
	DrawLine(RpCRT,t->x-1,t->y-1,t->x+1,t->y-1,clr);
	DrawLine(RpCRT,t->x-1,t->y,t->x+1,t->y,clr);
}

//************************************************************************
int DDD_horizon_line()
{
	int   i;
	int   pts[6][2], ctr[2][2];

	ctr[0][0] = 0;
	ctr[0][1] = ((long)OurPitch * DDD_H/2)/16384;
	rotate_pt(ctr[0],ctr[1],OurRoll);

	for (i=0;i<4;i++) {
		rotate_pt(hpts[i],pts[i],OurRoll);
		pts[i][0] += ctr[1][0] + DDD_CTR_X;
		pts[i][1] += ctr[1][1] + (DDD_CTR_Y-4);
	}

	DrawLine (RpCRT,pts[0][0],pts[0][1],pts[1][0],pts[1][1],19);
	DrawLine (RpCRT,pts[2][0],pts[2][1],pts[3][0],pts[3][1],19);
}

//************************************************************************
int show_rmin_rmax()
{
	int y;

	if ((AWG9_MODE !=AWG9_PDSTT) || (AWG9_MODE !=AWG9_STANDARD_TRACK))
		return 0;
	if (PW == W_NONE) return 0;

	// display rmin line
	y = DDD_H - ((long)DDD_H*ORDNANCE[PW].rmin/100)/RANGES[RANGE];

	DrawLine (RpCRT,DDD_W-3,y,DDD_W,y,HORZ_CLR);

	// display rmax line
	y = DDD_H - ((long)DDD_H*ORDNANCE[PW].rmax/100)/RANGES[RANGE];

	DrawLine (RpCRT,DDD_W-3,y,DDD_W,y,HORZ_CLR);
}

//************************************************************************
static show_range_carat()
{
	int  i, cl, y;
	int  p[4][2];

	if ((AWG9_MODE !=AWG9_PDSTT) || (AWG9_MODE !=AWG9_STANDARD_TRACK))
		return 0;

	y = DDD_H-((long)DDD_H*PDT_RNG)/BRANGES[RANGE];

	// range carat

	DrawDot (RpCRT, RR_CARAT_X-1, y-1, CARAT_CLR);
	DrawDot (RpCRT, RR_CARAT_X-1, y+1, CARAT_CLR);
	DrawDot (RpCRT, RR_CARAT_X  , y  , CARAT_CLR);
}

//************************************************************************
static show_steer_dot()
{
	int x,y;

	if (!STEER_DOT) return 0;

	x = (STEER_DOT_X * 10 + 15)/30 + DDD_CTR_X;
	y = (STEER_DOT_Y * 10 + 15)/30 + DDD_CTR_Y;

	DrawDot (RpCRT,x ,y ,BLIP_CLR);
}

//************************************************************************
static show_ase_circle ()
{
	int r;

	if (ASE_RADIUS <= 0) return 0;

	// scale for radar display
	r = (ASE_RADIUS * 10 + 15)/30;
	draw_circle (RpCRT,DDD_CTR_X,DDD_CTR_Y,r,HORZ_CLR,0);
}

//************************************************************************
draw_circle (RPS *rp, int xc, int yc, int r, int color, int dash)
{
//
// FOLEY-VAN DAM p.87
//
	int x,y,d,delE,delSE,c1,c2;

   // initialize

   c1    =
   c2    = dash;
   x     = 0;
   y     = r;
   d     = 1 - r;
   delE  = 3;
   delSE = -2 * r + 5;

   circle_pt (rp,x,y,xc,yc,color,&c1,&c2,dash);

   while (y > x) {
      if (d < 0) {         // select E
         d     += delE;
         delE  += 2;
         delSE += 2;
         x     += 1;
      } else {
         d     += delSE;   // select SE
         delE  += 2;
         delSE += 4;
         x     += 1;
         y     -= 1;
      }
      circle_pt (rp,x,y,xc,yc,color,&c1,&c2,dash);
   }
}

//***************************************************************************
static
circle_pt (RPS *rp,int x,int y,int xc,int yc,int clr,int *c1,int *c2,int dash)
{

   if      (*c1) (*c1)--;
   else if (*c2) (*c2)--;
   else *c1=*c2=dash;

   if (!(*c1)) {

      DrawDot (rp,Xasp( x)+xc,Yasp( y)+yc,clr);
      DrawDot (rp,Xasp( y)+xc,Yasp( x)+yc,clr);
      DrawDot (rp,Xasp( y)+xc,Yasp(-x)+yc,clr);
      DrawDot (rp,Xasp( x)+xc,Yasp(-y)+yc,clr);
      DrawDot (rp,Xasp(-x)+xc,Yasp(-y)+yc,clr);
      DrawDot (rp,Xasp(-y)+xc,Yasp(-x)+yc,clr);
      DrawDot (rp,Xasp(-y)+xc,Yasp( x)+yc,clr);
      DrawDot (rp,Xasp(-x)+xc,Yasp( y)+yc,clr);

   }
}



