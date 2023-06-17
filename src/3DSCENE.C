//***************************************************************************
//
//  File:   3Dscene.c
//
//  Auth:   Mike McDonald
//
//  Edit:
//
//
//***************************************************************************
#include "Library.H"
#include "f15defs.H"
#include <dos.h>

extern  void	Bgn3D(struct RastPort *Rp, int h, int p, int r,
                      int X, int Y, int Z, int Std, int Zoom);

extern  UWORD   GRPAGE0[];
extern  int     AnimVals[32];
extern  UWORD   *PalPtr3D;
extern  RPS     *Rp3D;
extern  int		ABOVECLOUDS;
extern  int		WORLDDETAIL;
extern  int     NEMSSEG;

void NDraw3D(int LHead, int LPitch, int LRoll, ULONG XP,ULONG YP,ULONG ZP);
extern DrawNLand( long X, long Y, long Z, char far *DST,char far *SRC);


//***************************************************************************
void NDraw3D(int LHead, int LPitch, int LRoll, ULONG XP,ULONG YP,ULONG ZP)
{
    int zoom;
	char far *ND;
	char far *NE;

    zoom = 0x100;

    Bgn3D(Rp3D, LHead, LPitch, LRoll, 0,0, (int) ZP, 1, zoom);

	NHorz((XP&0xffff)<<12,((0x100000L-YP)&0xffff)<<12, (int)ZP, LHead, LPitch, LRoll, 1, GRPAGE0[2], WORLDDETAIL);

	HandleSmoke(XP, YP, (UWORD) ZP);

	if (!ABOVECLOUDS) {
		FP_SEG(ND) = GRPAGE0[1];
		FP_OFF(ND) = 0;
		FP_SEG(NE) = NEMSSEG;
		FP_OFF(NE) = 0;
		DrawNLand(XP,ZP,(0x100000L-YP),ND,NE);
		Carrier3D();
		Boats3D();
	}

	Planes3D();

	DrawAAA();

	EndStack3D();
    End3D();
    DrawBullets();
}


//***************************************************************************
Init3DPal()
{
	int i,j;

	for (i=0;i<256;i++) {
		*(PalPtr3D+i)= 0xff00+i;
    }

	*(PalPtr3D+18) = 0x5f5f;    /* Dark Yellow Lines on Carrier */


	*(PalPtr3D+29) = 0x9191;    /* Solid windshield     */
	*(PalPtr3D+30) = 0x9091;    /* Dither windshield    */
	*(PalPtr3D+31) = 0x9393;	/* Beacon - Red */

	*(PalPtr3D+145) = 0xfa00;    /* cockpit              */

	*(PalPtr3D+128) = 0xfa00;    /* contrails            */
	*(PalPtr3D+129) = 0xfa00;    /* contrails            */

	*(PalPtr3D+84) = 256*ORANGEFLAME+ORANGEFLAME;  /* Blow-up flames  */
	*(PalPtr3D+13) = 256*BLUEFLAME+BLUEFLAME;      /* blue/white flame */
}

