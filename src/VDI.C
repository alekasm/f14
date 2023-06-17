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
#include "proto.h"

#define 	DEG			182
#define		PL_GAP		10
#define		PL_CTR_X	CRTXCENT
#define		PL_CTR_Y	CRTYCENT


extern  struct RastPort *RpCRT;

extern COORD    Px,Py;
extern unsigned Alt;
extern int      OurHead;
extern int      sx,sy,sz;
extern UWORD    Rtime;
extern int 		OurRoll;
extern int 		OurPitch;
extern int		MsgOnScreen;

/*  static prototypes  */
VdiPitchLadder();
horiz_line (int deg, int xc, int yc);
new_horiz (int deg, int xc, int yc);
pitch_line (int deg, int xc, int yc);
dashline (RPS *rp, int x0, int y0, int x1, int y1, int clr, int pix);
show_range ();
DisplayScanLimits();
draw_target (struct Planes *t, int age);
RangeBar();
TD_box ();
/*  End static prototypes  */

extern int Tscale;
int inity=30;

int VdiCrt (int CrtNr)
{
	static pos;
	static int t1;
	int xpos=0;
	int xpos2=25;
	static int ypos=-30;
	static int ypos2=-10;
	int p1,p2,p3,p4;
	int p5,p6,p7,p8;
	int x1,x2,y1,y2;
	int i;
	int basedeg;
	int pts[4][2];
	static int hpts[2][2]= {{-50,0},{-10,0}};
	int a[4][2] = {{-50,-45},
					{50,-45},
					{50,0},
					{-50,0}};
	int l1[2][2] = {{-40,0},{-30,0}};
	int l2[2][2] = {{40,0},{30,0}};
	int b[8] = {0,30,71,30,71,54,0,54};
	int t[8] = {0,0,0,0,0,0,0,0};
	int c[4][2] = {{0,0},{0,0},{0,0},{0,0}};
	int d[4][2] = {{0,0},{0,0},{0,0},{0,0}};
	static int  cpts[6][2] = {{-12+CRTXCENT,CRTYCENT},{-5+CRTXCENT,CRTYCENT},
								{-5+CRTXCENT,3+CRTYCENT},{5+CRTXCENT,3+CRTYCENT},
								{5+CRTXCENT,CRTYCENT},{12+CRTXCENT,CRTYCENT}};

	if (Damaged&D_PDISPLAYS) {
		RectFill(RpCRT,0,0,RpCRT->Width1+1,RpCRT->Length1+1, BLACK);
		return(1);
	}

	RectFill (RpCRT, 0,0, RpCRT->Width1+1,RpCRT->Length1+1, BLUE);

	basedeg = (OurPitch / DEG);

	if (basedeg >0) {
		a[2][1]+= basedeg;
		a[3][1]+= basedeg;
	}
	if (basedeg <0) {
		a[2][1]+=basedeg;
		a[3][1]+=basedeg;
	}

	/* Init. Poly */
	p1 = -16;
	p2 = -24;
	p3 = -27;
	p4 = -13;

	p5 = 16;
	p6 = 24;
	p7 = 27;
	p8 = 13;

	ypos=ypos+2;
	ypos2=ypos2+2;

	if (ypos>=30)
		ypos=a[2][1]-5;
	if (ypos2>=30)
		ypos2=a[2][1]-5;


	c[0][0]=p1;
	c[0][1]=ypos;

	c[1][0]=p2;
	c[1][1]=ypos;

	c[2][0]=p3;
	c[2][1]=ypos+7;

	c[3][0]=p4;
	c[3][1]=ypos+7;


	d[0][0]=p5;
	d[0][1]=ypos2;

	d[1][0]=p6;
	d[1][1]=ypos2;

	d[2][0]=p7;
	d[2][1]=ypos2+7;

	d[3][0]=p8;
	d[3][1]=ypos2+7;

	for (i=0; i<4; i++)
		rotate_pt(c[i],pts[i],OurRoll);

	t[0] = pts[0][0]+CRTXCENT;
	t[1] = pts[0][1]+CRTYCENT;
	t[2] = pts[1][0]+CRTXCENT;
	t[3] = pts[1][1]+CRTYCENT;
	t[4] = pts[2][0]+CRTXCENT;
	t[5] = pts[2][1]+CRTYCENT;
	t[6] = pts[3][0]+CRTXCENT;
	t[7] = pts[3][1]+CRTYCENT;

	DrawPoly(RpCRT,4,t,19);


	for (i=0; i<4; i++)
		rotate_pt(d[i],pts[i],OurRoll);

	t[0] = pts[0][0]+CRTXCENT;
	t[1] = pts[0][1]+CRTYCENT;
	t[2] = pts[1][0]+CRTXCENT;
	t[3] = pts[1][1]+CRTYCENT;
	t[4] = pts[2][0]+CRTXCENT;
	t[5] = pts[2][1]+CRTYCENT;
	t[6] = pts[3][0]+CRTXCENT;
	t[7] = pts[3][1]+CRTYCENT;

	DrawPoly(RpCRT,4,t,19);


	/* Sky area */
	for (i=0; i<4; i++)
		rotate_pt(a[i],pts[i],OurRoll);

	t[0] = pts[0][0]+CRTXCENT;
	t[1] = pts[0][1]+CRTYCENT;
	t[2] = pts[1][0]+CRTXCENT;
	t[3] = pts[1][1]+CRTYCENT;
	t[4] = pts[2][0]+CRTXCENT;
	t[5] = pts[2][1]+CRTYCENT;
	t[6] = pts[3][0]+CRTXCENT;
	t[7] = pts[3][1]+CRTYCENT;

	DrawPoly(RpCRT,4,t,LGREEN);


	/* Left and Right Horizon lines */
	for (i=0; i<2; i++)
		rotate_pt(l1[i],pts[i],OurRoll);
	x1 = pts[0][0] + CRTXCENT;
	y1 = pts[0][1] + CRTYCENT;
	x2 = pts[1][0] + CRTXCENT;
	y2 = pts[1][1] + CRTYCENT;

	DrawLine(RpCRT,x1,y1,x2,y2,19);

	for (i=0; i<2; i++)
		rotate_pt(l2[i],pts[i],OurRoll);
	x1 = pts[0][0] + CRTXCENT;
	y1 = pts[0][1] + CRTYCENT;
	x2 = pts[1][0] + CRTXCENT;
	y2 = pts[1][1] + CRTYCENT;

	DrawLine(RpCRT,x1,y1,x2,y2,19);

	VdiPitchLadder();
	if (MODE_MASTER == MASTER_NAV) {
		vdiils(RpCRT,CRTXCENT,CRTYCENT,HUD_COLOR);
	}
	DrawLine (RpCRT,cpts[0][0],cpts[0][1],cpts[1][0],cpts[1][1],HUD_COLOR);
	DrawLine (RpCRT,cpts[1][0],cpts[1][1],cpts[2][0],cpts[2][1],HUD_COLOR);
	DrawLine (RpCRT,cpts[3][0],cpts[3][1],cpts[4][0],cpts[4][1],HUD_COLOR);
	DrawLine (RpCRT,cpts[4][0],cpts[4][1],cpts[5][0],cpts[5][1],HUD_COLOR);

	RangeBar();

	return 1;
}

/////////////////////////////////////////////////////////////////////////////
static VdiPitchLadder()
{
	int i, base_deg, deg, off, dx, dy;
	int pts[4][2];


	// find delta x/y between pitch lines

	pts[0][0] =
	pts[1][0] = 0;
	pts[0][1] = PL_GAP;
	pts[1][1] = ((OurPitch % (DEG*10)) * PL_GAP)/(DEG*10);

	for (i=0;i<2;i++)
		rotate_pt (pts[i],pts[i+2],OurRoll);

	// closest 5 degrees

	base_deg = (OurPitch / (DEG*10)) * 10;

	for (i=-2;i<=2;i++) {
		dx  = -pts[2][0]*i+pts[3][0]+PL_CTR_X;
		dy  = -pts[2][1]*i+pts[3][1]+PL_CTR_Y;
		deg = base_deg + (i * 10);

		switch (deg) {
			case 0:
		   		horiz_line (deg, dx, dy);
			break;
//      case  90:
//      case -90:
			default:
				new_horiz (deg,dx,dy);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
static
horiz_line (int deg, int xc, int yc)
{
	int         i;
	int         pts [2][2];
	static int  hpts[2][2] = {{-2,0},{2,0}};
	// rotate horizon line
	for (i=0;i<2;i++)
		rotate_pt (hpts[i],pts[i],OurRoll);

	DrawLine (RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
}

/////////////////////////////////////////////////////////////////////////////
static
new_horiz (int deg, int xc, int yc)
{
	int         i,l;
	int         pts [9][2];
	char		s[10];
	static int  hpts[2][2] = {{-12,0},{12,0}};
	static int  hpts2[6][2] = {{-12,0},{-8,0},{-6,0},{6,0},{8,0},{12,0}};
	static int  hpts3[5][2] = {{-25,0},{-5,0},{5,0},{25,0},{0,0}};
	static int  hpts4[9][2] = {{-25,0},{-20,0},{-18,0},{-5,0},{5,0},{18,0},
								{20,0},{25,0},{0,0}};

	l = (pstrlen(RpCRT,itoa ((deg/10),s,10))-1)/2;

	// rotate horizon line
	if (deg >0) {
		if (!(deg % 30)) {
			for(i=0; i<5; i++)
				rotate_pt (hpts3[i],pts[i],OurRoll);
				DrawLine (RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
	  			DrawLine (RpCRT,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
				RpPrint (RpCRT,xc+pts[4][0]-l,yc+pts[4][1]-3,s);
		}
		else {
			for (i=0;i<2;i++)
				rotate_pt (hpts[i],pts[i],OurRoll);
				DrawLine (RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
		}
	}

	if (deg <0) {
		if (!(deg % 30)) {
			for (i=0; i<9; i++)
				rotate_pt (hpts4[i],pts[i],OurRoll);
			   	DrawLine (RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
				DrawLine (RpCRT,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
 		   		DrawLine (RpCRT,xc+pts[4][0],yc+pts[4][1],xc+pts[5][0],yc+pts[5][1],HUD_COLOR);
 		   		DrawLine (RpCRT,xc+pts[6][0],yc+pts[6][1],xc+pts[7][0],yc+pts[7][1],HUD_COLOR);
				RpPrint  (RpCRT,xc+pts[8][0]-l,yc+pts[8][1]-3,s);
		}
		else {
			for (i=0; i<6; i++)
				rotate_pt (hpts2[i],pts[i],OurRoll);
			   	DrawLine (RpCRT,xc+pts[0][0],yc+pts[0][1],xc+pts[1][0],yc+pts[1][1],HUD_COLOR);
				DrawLine (RpCRT,xc+pts[2][0],yc+pts[2][1],xc+pts[3][0],yc+pts[3][1],HUD_COLOR);
 		   		DrawLine (RpCRT,xc+pts[4][0],yc+pts[4][1],xc+pts[5][0],yc+pts[5][1],HUD_COLOR);
			}
	}
}




//////////////////////////////////////////////////////////////////////////

static
pitch_line (int deg, int xc, int yc)
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

	l = (pstrlen(RpCRT,itoa (abs(deg),s,10))-1)/2;

	for (i=0;i<2*4;i+=4) {
		if (d > 0)
			DrawLine (RpCRT,xc+rpts[i  ][0],yc+rpts[i  ][1],
					xc+rpts[i+1][0],yc+rpts[i+1][1],HUD_COLOR);
		else
			dashline (RpCRT,xc+rpts[i  ][0],yc+rpts[i  ][1],
					xc+rpts[i+1][0],yc+rpts[i+1][1],HUD_COLOR, 2);
		RpPrint  (RpCRT,xc+rpts[i+3][0]-l,yc+rpts[i+3][1]-3,s);
	}
}


//////////////////////////////////////////////////////////////////////////

static
dashline (RPS *rp, int x0, int y0, int x1, int y1, int clr, int pix)
{
int dx,dy,xi,yi,d,n,c,f;
int *x,*y,*t;

   if (x0 > x1) {
      n = x0; x0 = x1; x1 = n;
      n = y0; y0 = y1; y1 = n;
   }

   xi = 1;
   yi = (y1 < y0)?-1:1;
   dx = x1-x0;
   dy = abs(y1-y0);
   x  = &x0;
   y  = &y0;

   if (dy > dx) {
      n = dx; dx = dy; dy = n;
      n = xi; xi = yi; yi = n;
      t = x;  x  = y;  y  = t;
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



/////////////////////////////////////////////////////////////////////////////
VDITacxy (x,y)
    COORD x,y;
{
    int dx,dy;

    sz=-1;

    dx = ((int)(x-Px)) >> (7 - Tscale);   // 7,6,5,4
    dy = ((int)(y-Py)) >> (7 - Tscale);

    if ((UWORD)(abs(dx)+abs(dy)) <= CRTXSIZ)  {
        Rotate2D (dx, dy, OurHead, &sx, &sy);

        sy -= sy>>3;
        sy += CRTYSIZ;
        sx += CRTXCENT;

        if (sx>=0 && sx<CRTXSIZ && sy>=0 && sy<CRTYSIZ) sz=0;
		if (sy>CRTYSIZ) sz=-1;
	}


}


/////////////////////////////////////////////////////////////////////////////
static
show_range ()
{
static RANGES[] = { 80, 40, 20, 10 };
char   s[4];
int    l;

   RpCRT->FontNr = F_3x4;
   RpCRT->APen   = WHITE;

   l = pstrlen(RpCRT,itoa(RANGES[Tscale],s,10));

   RpPrint (RpCRT,65-l,3,s);

}


static DisplayScanLimits()
{
	DrawLine(RpCRT,CRTXCENT,CRTYSIZ,CRTXSIZ,CRTYCENT,0x1E);
	DrawLine(RpCRT,CRTXCENT,CRTYSIZ,0,CRTYCENT,0x1E);
}

static
draw_target (struct Planes *t, int age)
{
int   d, clr,temp;
int   x1,y1,x2,y2,off;
int taillength=5;
	int font;
	int alt;

   clr = WHITE;

   d = (((t->head-OurHead)+0x0800) >> 12)&15;



   off=0;
   font = GetFont(RpCRT,0);
   SetFont(RpCRT,F_3x4);
   RpCRT->APen = WHITE;


   VDITacxy(t->x,t->y);

   DrawLine (RpCRT,sx-2,sy-2,sx+2,sy-2,clr);
   DrawLine (RpCRT,sx-2,sy-2,sx-2,sy  ,clr);
   DrawLine (RpCRT,sx+2,sy-2,sx+2,sy  ,clr);

   /* Do Alt. */
   alt = t->alt/(1000/4);

   if ((alt >5) && (alt<=15))
	RpPrint(RpCRT,sx-6,sy-2,"1");
   if ((alt >15) && (alt<=25))
	RpPrint(RpCRT,sx-6,sy-2,"2");
   if ((alt >25) && (alt<=35))
	RpPrint(RpCRT,sx-6,sy-2,"3");
   if ((alt >35) && (alt<=45))
	RpPrint(RpCRT,sx-6,sy-2,"4");
   if ((alt >45) && (alt<=55))
	RpPrint(RpCRT,sx-6,sy-2,"5");


   RpPrint(RpCRT,sx+4,sy-2,"1");
   SetFont(RpCRT,font);


   switch (d) {
   case 0:
      x1 =
      x2 = sx;
      y1 = sy;
      y2 = sy - taillength;
      break;
   case 1:
   case 2:
   case 3:
      y1 = sy;
      y2 = sy - taillength;
      x1 = sx;
      x2 = sx + taillength;
      break;
   case 4:
      x1 = sx;
      x2 = sx + taillength;
      y1 = sy;
      y2 = sy;
      break;
   case 5:
   case 6:
   case 7:
      x1 = sx;
      y1 = sy;
      x2 = sx + taillength;
      y2 = sy + taillength;
      break;
   case 8:
	  x1 =
	  x2 = sx;
	  y1 = sy;
	  y2 = sy + taillength;
	  break;
   case 9:
   case 10:
   case 11:
	  x1 = sx;
	  y1 = sy;
	  x2 = sx - taillength;
	  y2 = sy + taillength;
	  break;
   case 12:
	  x1=sx;
	  y1=sy;
	  x2 = sx - taillength;
	  y2 = sy;
	  break;
   case 13:
   case 14:
   case 15:
	  x1=sx;
	  y1=sy;
	  x2=sx - taillength;
	  y2=sy - taillength;
	  break;
   }

   DrawLine (RpCRT,x1,y1,x2,y2,clr);

}


/////////////////////////////////////////////////////////////////////////////
#pragma optimize("",off)  // fix for c600 compiler bug

static RangeBar()
{
	int i,y1,y2,r,v;
	int y;
	char s[4];

	if ((AWG9_MODE != AWG9_PDSTT) && (AWG9_MODE !=AWG9_STANDARD_TRACK))
		return 0;

	draw_circle(RpCRT,CRTXCENT,CRTYCENT,15,HUD_COLOR);

	y = 40 - ((long)40*PDT_RNG)/BRANGES[RANGE]-1;

	DrawLine(RpCRT,CRTXCENT-24,5,CRTXCENT-24,45,19);
	DrawLine(RpCRT,CRTXCENT-25,5,CRTXCENT-25,45,19);

	// Show Ranges

	r = RANGES[RANGE];

	RpCRT->APen=19;
	RpPrint(RpCRT,CRTXCENT-24,CRTYCENT+20,itoa(r,s,10));

	// show rmax/rmin

	if (PW != W_NONE) {
		y1 = 40 - ((long)40*ORDNANCE[PW].rmin/10)/r - 1;
		y2 = 40 - ((long)40*ORDNANCE[PW].rmax/10)/r - 1;
		DrawLine(RpCRT,CRTXCENT-24,y1+5,CRTXCENT-21,y1+5,HUD_COLOR);
		DrawLine(RpCRT,CRTXCENT-24,y1+6,CRTXCENT-21,y1+6,HUD_COLOR);
		DrawLine(RpCRT,CRTXCENT-24,y2+5,CRTXCENT-21,y2+5,HUD_COLOR);
		DrawLine(RpCRT,CRTXCENT-24,y2+6,CRTXCENT-21,y2+6,HUD_COLOR);
	}

	if (y >= 0 && y <= 50) {
		y += 4;
		DrawLine(RpCRT,CRTXCENT-25,y,CRTXCENT-29,y,HUD_COLOR);
		DrawLine(RpCRT,CRTXCENT-25,y+1,CRTXCENT-29,y+1,HUD_COLOR);
   }
   TD_box();
}
#pragma optimize("",on)

//************************************************************************

static
TD_box ()
{
int x,y,l;
static int off=0;

   if (!PDT_PROJ) return 0;

   x = PDT_PROJ_X + CRTXCENT;
   y = PDT_PROJ_Y + CRTYCENT;

   // limit TD box

	DrawLine(RpCRT,x-1,y-3,x+1,y-3,19);
	DrawLine(RpCRT,x-1,y-2,x+1,y-2,19);
	DrawLine(RpCRT,x-3,y-1,x+3,y-1,19);
	DrawLine(RpCRT,x-3,y,x+3,y,19);
	DrawLine(RpCRT,x-3,y+1,x+3,y+1,19);
}



