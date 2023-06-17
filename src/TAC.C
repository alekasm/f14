/*  File:   Tac.c                                 */
/*  Author: Sid Meier / Andy Hollis               */
/*                                                */
/*  This module implements the 2D tactical        */
/*  CRT display.                                  */

#include "stdio.h"
#include "stdlib.h"
#include <dos.h>
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"


extern  struct RastPort *RpCRT;
extern COORD    Px,Py;
extern unsigned Alt;
extern int      OurHead;
extern int      sx,sy,sz;
extern UWORD    Rtime;
extern int Tscale;
extern UWORD ACMJetsw1;
extern UWORD ACMJetsw2;
extern int BackSeat;

extern long  PxLng;
extern long  PyLng;


extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);

int tacx=0;
int tacy=0;
int tacz=0;

/*  static function prototypes  */
concentric ();
show_range ();
/*  end of static prototypes  */

TacMap(RPS *Rp, long X, long Z, int zoom );


//***************************************************************************
DoTac (int CrtNr)
{
    COORD x,y;
    int size,i,a,b,dalt, Colr;
    struct Sams   *ssi;
    struct Radars *rri;
    struct Planes *psi;
    struct Decoy  *dsi;
	struct MissileData far *tempmissile;
	struct PlaneData far *tempplane;
	int mseg;
	int xpos;
	int ypos;

    size=1+Tscale;

	if (Damaged&D_AWG9) {
		RectFill(RpCRT,0,0,RpCRT->Width1+1,RpCRT->Length1+1, BLACK);
		return(1);
	}

	RectFill (RpCRT, 0,0, RpCRT->Width1+1,RpCRT->Length1+1, BLACK);

	TacMap(RpCRT, PxLng,(0x100000-PyLng), 0x100);

	if (!BackSeat)
		show_range();

    for (i=0, ssi=ss; i<NSAM; i++, ssi++){
        if (ssi->dist) {
            Tacxy ((int)(ssi->x>>HTOV),(int)(ssi->y>>HTOV));
            if (sz!=-1) {
				tempmissile = GetMissilePtr(ssi->type);
                Colr = (tempmissile->homing<=IR_2 ? YELLOW:16);
                if (ssi->losing)
					Colr = LGREY;
                if (i>=NESAM)
					Colr = WHITE;
                a = (ssi->head-OurHead);
                DrawLine (RpCRT, sx,sy, sx-sinX(a,size),sy+cosX(a,size), Colr);
            }
        }
    }

    for (i=0, rri=Rdrs; i<NRADAR; i++, rri++) {
        if (!(rri->status&DESTROYED)) {
            Tacxy (rri->x, rri->y);
            if (sz!=-1) {
				if (rri->status & FRIENDLY) {
					if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
							ScaleRotate(mseg,(164+5),(62+5),10,10,RpCRT,(RpCRT->XBgn+sx),
								(RpCRT->YBgn+sy),0x0100,0,0,0);
					}
				} else {
					if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
							ScaleRotate(mseg,(204+5),(62+5),10,10,RpCRT,(RpCRT->XBgn+sx),
								(RpCRT->YBgn+sy),0x0100,0,0,0);
					}
				}
			}
        }
	}

    for (i=0, psi=ps; i<MAXPLANE; i++, psi++){
        if ((psi->status&(ACTIVE+ALIVE))==(ACTIVE+ALIVE) && psi->speed) {
            Tacxy (psi->x,psi->y);
            if (sz!=-1) {
                if (DESIGNATED==i) TacBox ((!SHOOT||(Rtime&2))?WHITE:LGREY);
				tempplane = GetPlanePtr(psi->type);

                a = (psi->head-OurHead) + 0x0800;

				if (psi->status & FRIENDLY)
					b=2;
				else
					b=0;

				xpos = 154 + (((a>>12)&15)*10);

				switch(tempplane->bclass) {
					case FIGHTER_OBJCLASS:
						if (b==2) {
							if (tempplane->PlaneEnumId == SO_E_2CA)
								ypos=32;
							else
								ypos=22;
						} else {
							ypos=82;
						}
					break;
					case BOMBER_OBJCLASS:
						if (b==2)
							ypos=42;
						else
							ypos=102;
					break;
					case CARRIER_OBJCLASS:
						if (b==2) {
							ypos=52;
						} else {
							xpos=194;
							ypos=62;
						}
					break;
					case SHIP_OBJCLASS:
						if (b==2) {
							xpos=154;
							ypos=22;
						} else {
							xpos=194;
							ypos=62;
						}
					break;
					case CMISSILE_OBJCLASS:
						if (b==2)
							ypos=22;
						else
							ypos=72;
					break;
					default:
						if (b==2)
							ypos=22;
						else
							ypos=82;
					break;
				}

				if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
						ScaleRotate(mseg,(xpos+5),(ypos+5),10,10,RpCRT,(RpCRT->XBgn+sx),
							(RpCRT->YBgn+sy),0x0100,0,0,0);
				}
            }
        }
    }

    Tacxy (Px,Py);
	if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
		ScaleRotate(mseg,(214+5),(62+5),10,10,RpCRT,RpCRT->XBgn+sx,
			RpCRT->YBgn+sy,0x0100,0,0,0);
	}

    for (i=0, dsi=ds; i<NDECOY; i++, dsi++) {
        if (dsi->time) {
            Tacxy (dsi->x, dsi->y);
            if (sz!=-1) {
                switch (dsi->type) {
                    case FLARE:
						if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
								ScaleRotate(mseg,(184+5),(62+5),10,10,RpCRT,(RpCRT->XBgn+sx),
								(RpCRT->YBgn+sy),0x0100,0,0,0);
						}
					break;
                    case CHAFF:
						if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
								ScaleRotate(mseg,(184+5),(62+5),10,10,RpCRT,(RpCRT->XBgn+sx),
								(RpCRT->YBgn+sy),0x0100,0,0,0);
						}
					break;
                }
            }
        }
    }

	for (i=0; i<4; i++) {
		Tacxy(wps[i].x,wps[i].y);
		if (sz!=-1) {
			DrawDot(RpCRT,sx,sy,0x5a);
			DrawDot(RpCRT,sx-1,sy+1,0x5a);
			DrawDot(RpCRT,sx+1,sy+1,0x5a);
			DrawDot(RpCRT,sx-2,sy+2,0x5a);
			DrawDot(RpCRT,sx+2,sy+2,0x5a);
			DrawLine(RpCRT,sx-3,sy+3,sx+3,sy+3,0x5a);
		}
	}

    return 1;
}

//***************************************************************************
TacBox (int color)
{
    DrawLine (RpCRT, sx-5,sy-5, sx+5,sy-5, color);
    DrawLine (RpCRT, sx+5,sy-5, sx+5,sy+5, color);
    DrawLine (RpCRT, sx+5,sy+5, sx-5,sy+5, color);
    DrawLine (RpCRT, sx-5,sy+5, sx-5,sy-5, color);
}

//***************************************************************************
Tacxy(COORD x,COORD y)
{
    int dx,dy;

    sz=-1;

    dx = ((int)(x-Px)) >> (7 - Tscale);   // 7,6,5,4
    dy = ((int)(y-Py)) >> (7 - Tscale);

    if ((UWORD)(abs(dx)+abs(dy)) <= (RpCRT->Width1))  {
        Rotate2D (dx, dy, OurHead, &sx, &sy);

        sy -= sy>>3;
        sy += (RpCRT->Length1/2);
        sx += (RpCRT->Width1/2);

        if (sx>=0 && sx<(RpCRT->Width1) && sy>=0 && sy<(RpCRT->Length1)) sz=0;
    }


}

#ifdef YEP
//***************************************************************************
int TacisplayPosition()
{
	if (TACTargetCnt>23) return(0);

	tempps = ps + tc->i;

	TIDxy(tempps->x,tempps->y);
	if ((tempx!=0) && (tempy!=0)) {
		TIDTargetList[TIDTargetCnt].x = tempx;
		TIDTargetList[TIDTargetCnt].y = tempy;
		TIDTargetList[TIDTargetCnt].i = tc->i;
		TIDTargetCnt++;
	}
}
#endif


//***************************************************************************
static concentric()
{
	int i,r;

	for (i=0,r=7;i<4;i++,r+=7)
		draw_circle (RpCRT,(RpCRT->Width1/2),(RpCRT->Length1/2),r,0x1E,0);
	DrawLine(RpCRT,CRTXCENT,CRTYCENT-24,CRTXCENT,CRTYCENT+25,0x1E);
	DrawLine(RpCRT,CRTXCENT-7*4,CRTYCENT,CRTXCENT+7*4,CRTYCENT,0x1E);
}

//***************************************************************************
static show_range()
{
	static RANGES[] = { 200,100,50,25,10 };
	char   s[10];
	int    l;

	RpCRT->FontNr = F_3x4;
	RpCRT->APen   = WHITE;

	l = pstrlen(RpCRT,itoa(RANGES[Tscale],s,10));

	RpPrint (RpCRT,75-l,2,s);
}

extern	UWORD   GRPAGE0[];
#define  HMAPBANK 0
//***************************************************************************
TacScaleRotate256(UWORD SOURCE, int XSC, int YSC, int WID, int HT, RPS *Rp, int XDC, int YDC, int SCALE, int ROLL, int TRNSCLR, int MIRROR )
{
	UWORD	ADDRESS[321],ADDBASE;
	int		i;
	int 	XS,YS,XADD,YADD,TX,TY,SSX,SSY;
	int 	X1,Y1,X2,Y2;
	long	T1,T2;

	if (SCALE<1) SCALE=1;
	T1= ((long)WID*(unsigned long)SCALE)/256L;
	T2= ((long)HT*(unsigned long)SCALE)/256L;

	X1= XDC-T1/2;
	Y1= YDC-T2/2;
	X2= X1+T1;
	Y2= Y1+T2;

	if (X1>(Rp->XBgn+Rp->Width1)) return(1);
	if (Y1>(Rp->YBgn+Rp->Length1)) return(1);
	if (X2<Rp->XBgn) return(1);
	if (Y2<Rp->YBgn) return(1);

	if (X1<Rp->XBgn) X1=Rp->XBgn;
	if (X2>(Rp->XBgn+Rp->Width1)) X2=(Rp->XBgn+Rp->Width1);
	if (Y1<Rp->YBgn) Y1=Rp->YBgn;
	if (Y2>(Rp->YBgn+Rp->Length1)) Y2=(Rp->YBgn+Rp->Length1);


	SSX = TX = ACosB(256*32,ROLL)/SCALE;
	SSY = TY = ASinB(256*32,ROLL)/SCALE;

	if (MIRROR) TX=-TX;

	for (i=0;i<=(X2-X1);i++) {
		ADDRESS[i]= 256*((i*TY)/32)+ (i*TX)/32;
	}

	for (i=Y1;i<=Y2;i++) {
		ADDBASE= 256*(((X1-XDC)*TY+(i-YDC)*SSX)/32 +YSC) +
					((X1-XDC)*TX)/32+ -((i-YDC)*TY)/32+XSC;
		TacTransScaleLine(SOURCE, ADDBASE,
			ADDRESS, X1, i, (X2-X1)+1, GRPAGE0[Rp->PageNr],TRNSCLR);
	}
}


TacMap(RPS *Rp, long X, long Z, int zoom )
{
   int   XP,ZP;
   int   WID,HT,XDC,YDC;
   char far *PTR;
   int   SEG;

   XP= (X>>12);
   ZP= (Z>>12);
   WID = Rp->Width1+1;
   HT  = Rp->Length1+1;
   XDC= Rp->XBgn+WID/2;
   YDC= Rp->YBgn+HT/2;

   FP_OFF(PTR)=0;
   FP_SEG(PTR)=SEG=MapInBank(HMAPBANK);

   TacScaleRotate256(SEG, XP,ZP,WID, HT, Rp,XDC, YDC, (0x100<<Tscale), OurHead,0x52,0);

}
