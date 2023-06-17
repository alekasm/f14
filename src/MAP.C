/*  File:   Map.c                                 */
/*  Author: Sid Meier / Andy Hollis               */
/*                                                */
/*  This module implements the 2D scrolling map   */
/*  routined for the CRT screen.                  */


#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"

extern struct RastPort *Rp1, *Rp2, *RpCRT, RpCX1;

#ifdef YEP
extern  UWORD   MapMiscSprite1;
extern  UWORD   MapMiscSprite2;
extern  UWORD   MapMiscSprite3;
extern  UWORD   MapMiscSprite4[];
extern  UWORD   MapPlaneSprites [];
extern  UWORD   MapPlaneSprites2[];
extern  UWORD   TargetPointSprite[];
extern  UWORD   BasePointSprite[];
#endif

extern  int     OurHead;
extern  long    PxLng,PyLng;
extern  COORD   Px,Py;
extern  int     Rtime;

int Mscale = 5;
int Mapx =  0;
int Mapy =  0;

static int CColors[] = { 20, 14, 16 };

#ifdef YEP
/*----------*/
/*
/* Draw a map centered at Px,Py using zoom parameters */
/*
/*----------*/
DoMap (int CrtNr)
{
    int x1,y1,x2,y2,i, x,y,rad;
    long CenterX, CenterY;

    i = MAPXSIZ<<Mscale;
    Mapx = rng (Px, i,0x7fff-i);

    i = MAPYSIZ<<Mscale;
    Mapy = rng (Py, i,0x7fff-i);

    CenterX = (long)Mapx<<HTOV;
    CenterY = (long)Mapy<<HTOV;

    Map3d (RpCRT, CenterX, CenterY, 6+Mscale);

    OverlaySequencePoints (CenterX, CenterY, 0, 32<<(1+Mscale), TSD);

    if (!RealTSD) {
        for (i=1; i<NRADAR; i++) {
            if (Rdrs[i].rclass != NONE && !(Rdrs[i].status&DESTROYED)) {
                if ( MapLoc (Rdrs[i].x, Rdrs[i].y, &x,&y) ) {
                    y2 = 0;
                    if (Rdrs[i].status&RSEEN) y2 = 1;
                    if (Rdrs[i].status&RTRACK) y2 = 2;
                    ShowSprite (RpCRT, x-1,y-1, MapMiscSprite4[y2]);
                    if (Rdrs[i].status&RACTIVE) {
                        rad = r[Rdrs[i].rclass].range;
                        rad-= rad*(3-Diff)/8;
                        rad = ((long)rad * 26L)>>(1+Mscale);
                        draw_circle (RpCRT,x,y,rad,CColors[y2],0);
                    }
                }
            }

            if ((Rdrs[i].status&(DESTROYED+FRIENDLY+AIRBASE+TANKER)) == FRIENDLY+AIRBASE
                || Rdrs[i].status&CARRIER) {
                if ( MapLoc (Rdrs[i].x, Rdrs[i].y, &x,&y) ) {
                    ShowSprite (RpCRT, x-1,y-1, MapMiscSprite2);
                }
            }
        }

        for (i=0; i<NPLANES; i++) {
            if (ps[i].status & ALIVE) {
                if (ps[i].status&ACTIVE && ps[i].speed) {
                    if (!(ps[i].status&ALERTED) || (Rtime&1)) {
                        if (MapLoc (ps[i].x, ps[i].y, &x,&y)) {
                            ShowSprite (RpCRT, x-1,y-1, MapPlaneSprites2 [((ps[i].head+0x1000) >>13) &7]);
                        }
                    }
                }
            }
        }

        for (i=0; i<NSAM; i++) {
        if (ss[i].dist) {
                mplot ((int)(ss[i].x>>HTOV), (int)(ss[i].y>>HTOV), YELLOW, Mscale<=5);
            }
        }
    }

    Project2D (PxLng-CenterX, (0x100000-PyLng)-CenterY, 0, 32<<(1+Mscale), &x,&y);
    ClipSprite (RpCRT, x-1,y-1, MapPlaneSprites [((OurHead+0x1000) >>13) &7]);

    return 1;

}
#endif

/*----------*/
X2map (x)
{
    return ((int)(x-Mapx)>>(1+Mscale))+CRTXCENT;
}

/*----------*/
Y2map (y)
{
    return ((int)(y-Mapy)>>(1+Mscale))+CRTYCENT;
}

/* Plot on map, using zoom parameters */
/*----------*/
mplot (px,py, color, size)
COORD    px,py;
int     color,size;
{
    int x,y;

    x = X2map(px);
    y = Y2map(py);
    if (color!=-1) {
        DrawDot (RpCRT, x,y, color);
        if (size) {
            DrawDot (RpCRT, x+1,  y, color);
            DrawDot (RpCRT, x,  y+1, color);
            DrawDot (RpCRT, x+1,y+1, color);
        }
    }
}

/*----------*/
MapLoc (px,py, x,y)
    COORD    px,py;
    int     *x,*y;
{
    *x = X2map(px);
    *y = Y2map(py);

    if ((*x>=0) && (*x<CRTXSIZ) && (*y>=0) && (*y<CRTYSIZ)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#ifdef YEP
/*----------*/
OverlaySequencePoints (long CenterX, long CenterY, int Angle, int Scale, int Display) {
    int     x,y, x1,y1;

    Project2D (wps[0].xL-CenterX, wps[0].yL-CenterY, Angle, Scale, &x,&y);
    Project2D (wps[1].xL-CenterX, wps[1].yL-CenterY, Angle, Scale, &x1,&y1);
    if (Display==TSD) DrawLine (RpCRT, x,y,x1,y1, TSD_COLOR);
    ClipSprite (RpCRT, x-3,y-2, BasePointSprite[RBMflag]);

    Project2D (wps[2].xL-CenterX, wps[2].yL-CenterY, Angle, Scale, &x,&y);
    if (Display==TSD) DrawLine (RpCRT, x,y,x1,y1, TSD_COLOR);

    if (!(Status&PRIMARYHIT)) ClipSprite (RpCRT, x1-3,y1-2, TargetPointSprite[RBMflag]);
    if (!(Status&SECONDHIT))  ClipSprite (RpCRT,  x-3, y-2, TargetPointSprite[RBMflag]);

    Project2D (PxLng-CenterX, (0x100000-PyLng)-CenterY, Angle, Scale, &x,&y);
    ClipSprite (RpCRT, x-1,y-1, MapPlaneSprites [((OurHead+0x1000) >>13) &7]);
}
#endif
/*----------*/
Project2D (long x0, long y0, int Angle, int Scale, int *x, int *y ) {

    x0 /=Scale;
    y0 /=Scale;

    if (Angle==0) {
        *x = x0;
        *y = y0;
    } else {
        Rotate2D ((int) x0, (int) y0, Angle, x, y);
    }

    *x += CRTXCENT;
	*y += CRTYCENT;
}

/*----------*/
DeProject2D (long x0, long y0, int Angle, int Scale, int *x, int *y ) {

    *y -= CRTYCENT;
    *x -= CRTXCENT;

    if (Angle==0) {
        *x = x0;
        *y = y0;
    } else {
        Rotate2D ((int) x0, (int) y0, -Angle, x, y);
    }

    x0 *=Scale;
    y0 *=Scale;

}

/*----------*/
Rotate2D (int x0, int y0, int Angle, int *x, int *y) {

    *x = ACosB (x0, Angle) + ASinB (y0, Angle);
    *y = ACosB (y0, Angle) - ASinB (x0, Angle);
}


