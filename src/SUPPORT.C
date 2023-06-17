#include "library.h"
#include "world.h"
#include "sdata.h"
#include <dos.h>

rng(int x,int a,int b)
{
    if (x>b) return b;
    if (x>=a) return x;
    if (x>-0x4000) return a;
    return b;
}

rng2(int x,int a,int b)
{
    if (x>b) return b;
    if (x<a) return a;
    return x;
}

xydist(int x,int y)
{
    long d;

    x = abs(x); y=abs(y);
    d = (x>y) ? (long) x+(y>>1): (long) y+(x>>1);
    if (d>0x7fff) d=0x7fff;
    return d;
}

uangle(int a, UWORD b)
{
    if (b>32767) {
        b>>=1;
        a>>=1;
    }
    return ARCTAN (a, b);
}


//********************************************
//
//  DON'T USE THIS!   USE SCOTT'S ARCTAN ROUTINE.  MUCH MORE ACCURATE!!
//
//*********************************************

//#define PREC 14
//#define POINT3 4915  /* constant .3 * 2**PREC  */
//
//angle (dx,dy)
//int dx,dy;
//{
//    int a,angl,max,xmax,ratio;
//    long min;
//
///*  dy=-dy;      adjust for y=-y coordinate system */
//
//    if (dx==0) return (dy>0) ? 0x0000: 0x8000;
//    if (dy==0) return (dx>0) ? 0x4000: 0xc000;
//
//    if (abs(dx)>abs(dy)) {
//        min = (long) abs(dy)<<PREC;
//        max = abs(dx);
//        xmax = TRUE;
//    } else {
//        min = (long) abs(dx)<<PREC;
//        max = abs(dy);
//        xmax = FALSE;
//    }
//
//    ratio = min/max;
//
//    a = ((long) ((40<<8) - (((long)abs(POINT3-ratio)*(11<<8))>>PREC) ) * ratio)>>PREC;
//
//    if (dx>0) {
//        if (dy>0) {
//            angl = xmax ? 0x4000-a : a;
//        } else {
//            angl = xmax ? 0x4000+a : 0x8000-a;
//        }
//    } else {
//        if (dy>0) {
//            angl = xmax ? 0xC000+a : -a;
//        } else {
//            angl = xmax ? 0xC000-a : 0x8000+a;
//        }
//    }
//    return angl;
//}

sinX(int angle,int x)
{
    return TrgMul (Isin(angle), x);
}

cosX(int angle,int x)
{
    return sinX(angle+0x4000,x);
}

wrd(int x)
{
    x = ((x&255)<128) ? x&255 : (x&255)- 256;
    return x;
}

sgn(int x)
{
    if (x==0) return 0;
    return (x>0) ? 1 : -1;
}

/****************************************************************************/
extern int Replay;
static UWORD rand2=0;
seedme()
{
    rand2 = (BIOS_TICK&7) <<13;
    srand (rand2);
}

seedme2()
{
    rand2 = (BIOS_TICK&7) <<13;
}

rnd(Max) /* deterministic */
{
    return ((long) Max*rand()) >>15;
}


rnd2(Max) /* deterministic, no seed */
{

    rand2 = rand2*5 +8191;

    return ((long) Max*rand2) >>16;
}

/****************************************************************************/

/*----------*/
WaitTicks(unsigned ticks)
{
    while (ticks--) {
        while ((inp (0x3DA) & 8) == 0);
        while ((inp (0x3DA) & 8) != 0);
    }
}

