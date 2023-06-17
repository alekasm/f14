/*  File:   3dgraf.c                              */
/*  Author: Sid Meier                             */
/*                                                */
/*  Game-related 3D screen displays               */

#include <stdlib.h>
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"

extern  RPS     *Rp3D;

extern  COORD   Px,Py;
extern  int     VM[3][3];
extern  UWORD   Alt;
extern  int     View;
extern  long    PxLng,PyLng, ViewX,ViewY;
extern  int     ViewZ;
extern  int     Panning;

int sx,sy,sz;

static  int Pdist;



/*----------*/
project (x,y,z)
    COORD    x,y;
    int     z;
{
    int  dx,dy,dz;
    long xe,ye,ze;
    long row3();

    dx =- ((long) x - Px);
    dy =  ((long) y - Py);
    dz = (z - (int) Alt) >>(HTOV);

    if (View & VOFFSET) {
        dx -=  ((long) PxLng - ViewX) >>HTOV;
        dy -=  ((long) PyLng - ViewY) >>HTOV;
        dz -= -((long) Alt   - ViewZ) >>HTOV;
     }

    xe=row3(0,dx,dy,dz);
    ye=row3(1,dx,dy,dz);
    ze=row3(2,dx,dy,dz);

    sz = ze>>(BT2KM-3);

    if (ze>=0) {   /* It's behind us */
        sx=-1;
        return;
    }

    if (xe>-ze || xe<ze) {
        sx = -1;
        return;
    }

    sx = ((xe<<8)/ze) +160 /*+ Panning*/;

    sy = ((ye<<8)/ze);
    sy -= sy>>2;

    if (View&VFULL) {
        sy += Rp3D->Length1 >>1;
    } else {
        sy += 56;
    }


    if (sx<0 || sx>319)                {sx=-1;}
    if (sy<0 || sy > (Rp3D->Length1) ) {sx=-1;}
}

//***************************************************************************
lproject(long x, long y, int z)
{
    long    dx,dy;
    int     dz;
    long    xe,ye,ze;
    long    row3();
    int     ZShift;

    dx = -(x - ViewX);
    dy =   y - (0x100000-ViewY);
    dz =  (z - ViewZ);

    ZShift = HTOV+BT2KM-3;

    if (labs(dx)>23000L || labs(dy)>23000L) {
        dx >>=HTOV;
        dy >>=HTOV;
        dz >>=HTOV;

        ZShift -= HTOV;
    }

    xe = row3(0, (int)dx, (int)dy, dz);
    ye = row3(1, (int)dx, (int)dy, dz);
    ze = row3(2, (int)dx, (int)dy, dz);

    sz = ze>>ZShift;

    if (ze>=0) {   /* It's behind us */
        sx=-1;
        return;
    }

    if (xe>-ze || xe<ze) {
        sx = -1;
        return;
    }

    sx = ((xe<<8)/ze) +160;

    sy = ((ye<<8)/ze);
    sy -= sy>>2;

    if (View&VFULL) {
        sy += Rp3D->Length1 >>1;
    } else {
        sy += 56;
    }

    if (sx<0 || sx>319) {
		sx=-1;
	}

	if (sy<0 || sy > (Rp3D->Length1)) {
		sx=-1;
	}
}


long row3 (int r,int dx,int dy,int dz)
{
    long z;

    z  =  TrgMul(VM[0][r],dx);
    z +=  TrgMul(VM[1][r],dz);
    z +=  TrgMul(VM[2][r],dy);

    return z;
}

/*----------*/
DistTo (i) {
    return DistOf(Rdrs[i].x,Rdrs[i].y, 1);
}

/*----------*/
DistToPlane (i) {
    return DistOf(ps[i].x,ps[i].y, 0);
}

/*----------*/
DistOf (x,y, doAngle) {
    int dx,dy;

    dx = Px-x;
    dy = Py-y;

    Pdist = xydist(dx,dy);
    return Pdist;
}

