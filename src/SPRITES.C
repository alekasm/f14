#include <stdlib.h>
#include "library.h"
#include "f15defs.h"
#include "world.h"
#include <dos.h>

UWORD HudSprite1;
UWORD RadarCursorSprite[3];
UWORD HudSprite3;
UWORD HudSprite4;
UWORD HudSprite5;
UWORD MouseSprite;
UWORD TEWSprites[6];

extern RPS *Rp3D, *RpCRT;
extern	UWORD   GRPAGE0[];
extern	int	    GXMID;
extern	int	    GYMID;
extern	int	    Nviewmat[];
extern	int	    vRoll;
extern	int		ABOVECLOUDS;
extern   int MISSILETRAIL;


int	InXPLO=0;
int	XPLODETAIL=1;


//***************************************************************************
LoadSprites()
{
    int i,j;

    OpenSprite();
    LoadPicGmem("f14.pic",1);

	TEWSprites[0] = BuildSprite(1,34,1,11,11);
	TEWSprites[1] = BuildSprite(1,45,1,11,11);
	TEWSprites[2] = BuildSprite(1,58,2,6,8);
	TEWSprites[3] = BuildSprite(1,69,2,6,8);
	TEWSprites[4] = BuildSprite(1,58,13,3,3);
	TEWSprites[5] = BuildSprite(1,61,13,3,3);

    RadarCursorSprite[0] = BuildSprite(1, 14,1,6,6);
    RadarCursorSprite[2] = BuildSprite(1, 23,2,7,5);

    HudSprite1 = BuildSprite(1,1,0,11,7);
    HudSprite3 = BuildSprite(1,2,13,9,9);
    HudSprite4 = BuildSprite(1,11,13,9,9);

    MouseSprite = BuildSprite (1,27,14,9,11);
    MouseSETUP(MouseSprite,0,0);
	CloseSprite();
}

/****************************************************************************/
ScaleRotate(UWORD SOURCE, int XSC, int YSC, int WID, int HT, RPS *Rp, int XDC, int YDC, int SCALE, int ROLL, int TRNSCLR, int MIRROR)
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
//	X2= X1+T1-1;
//	Y2= Y1+T2-1;
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

#ifdef YEP
   if (ROLL==0)
      {
      SSX=TX=((256*32)/SCALE);
      SSY=TY=0;
      }
   else
      {
#endif
	   SSX = TX = ACosB(256*32,ROLL)/SCALE;
	   SSY = TY = ASinB(256*32,ROLL)/SCALE;
#ifdef YEP
      }
#endif

	if (MIRROR) TX=-TX;

	for (i=0;i<=(X2-X1);i++) {
		ADDRESS[i]= 320*((i*TY)/32)+ (i*TX)/32;
	}

	for (i=Y1;i<=Y2;i++) {
		ADDBASE= 320*(((X1-XDC)*TY+(i-YDC)*SSX)/32 +YSC) +
					((X1-XDC)*TX)/32+ -((i-YDC)*TY)/32+XSC;
		TransScaleLine(SOURCE, ADDBASE,
			ADDRESS, X1, i, (X2-X1)+1, GRPAGE0[Rp->PageNr],TRNSCLR);
	}
}

//***************************************************************************
TransRectCopy(UWORD SOURCE,int XSL,int YSL,int WID,int HT,RPS *Rp,int XD,int YD,int TRNSCLR,int MIRROR)
{
	UWORD	ADDRESS[321],ADDBASE;
	int		i,j;
	int 	X1,Y1,X2,Y2;
	long	T1,T2;

	X1 = XD;
	Y1 = YD;
	X2 = X1+WID;
	Y2 = Y1+HT;

	if (X1>(Rp->XBgn+Rp->Width1)) return(1);
	if (Y1>(Rp->YBgn+Rp->Length1)) return(1);
	if (X2<Rp->XBgn) return(1);
	if (Y2<Rp->YBgn) return(1);


	if (X1<Rp->XBgn) {
		XSL+= (MIRROR)? -(Rp->XBgn-X1):Rp->XBgn-X1;
		X1=Rp->XBgn;
	}
	if (X2>(Rp->XBgn+Rp->Width1)) X2=(Rp->XBgn+Rp->Width1);
	if (Y1<Rp->YBgn) {
		YSL+= Rp->YBgn-Y1;
		Y1=Rp->YBgn;
	}
	if (Y2>(Rp->YBgn+Rp->Length1)) Y2=(Rp->YBgn+Rp->Length1);

	if (MIRROR) {
   		for (i=0,j=(X2-X1)+XSL;i<=(X2-X1);i++)
         	ADDRESS[i]=j--;
	} else {
   		for (i=0;i<=(X2-X1);i++)
	   		ADDRESS[i]= XSL++;
	}

	ADDBASE=320*YSL;

	for (i=Y1;i<=Y2;i++) {
		TransScaleLine(SOURCE,ADDBASE,ADDRESS,X1,i,(X2-X1)+1,
				GRPAGE0[Rp->PageNr],TRNSCLR);
		ADDBASE+=320;
	}
}



#define	SMKNUM	30

struct SMK{
	long	X;
	int	Y;
	long	Z;
	int	TYP;
	int	CNT;
	int	PIC;
	unsigned int	UPCNT;
} SMOKES[SMKNUM];

struct SPRTCRD{
	UWORD PAGE;
	int XC;
	int YC;
	int WID;
	int HT;
	} SPRTVLS[]={

  0, 38, 45, 21, 17,
  0,115, 44, 41, 33,
  0,192, 42, 49, 39,
  0,268, 43, 53, 43,
  0, 55,154, 53, 43,
  0,116,105, 55, 45,
  0,191,148, 63, 51,
  0,263,110, 63, 51,
  1, 43, 42, 53, 43,
  1,117, 44, 55, 45,
  1,192, 45, 57, 47,
  1,272, 53, 63, 51,
  1, 53,102, 61, 49,
  1,149,101, 63, 51,
  1,272,123, 59, 49,
  1,100,159, 59, 49,
  1,195,159, 63, 51,

  2, 14, 10, 27, 19,
  2, 45, 13, 33, 24,
  2, 82, 14, 39, 27,
  2,126, 17, 47, 33,
  2,176, 17, 50, 33,
  2,223, 15, 40, 28,
  2,261, 13, 33, 24,
  2,293, 11, 28, 20,
  2,182, 47, 24, 18,
  2,216, 47, 29, 21,
  2,251, 43, 33, 23,
  2,293, 39, 43, 27,
  2, 25, 40, 43, 29,
  2, 75, 45, 43, 32,
  2,116, 47, 33, 22,
  2,150, 44, 27, 17,

  3, 16, 20, 20, 14,
  3, 25, 42, 41, 28,
  3, 72, 39, 51, 34,
  3,124, 43, 51, 42,
  3,176, 33, 51, 50,
  3,229, 31, 53, 54,
  3,288, 27, 62, 53,
  3, 54, 85, 62, 56,
  3,112, 89, 53, 47,
  3,168, 89, 57, 49,
  3,227, 84, 58, 51,
  3,288, 82, 62, 55,
  3, 35,141, 62, 54,
  3, 98,141, 62, 54,
  3,161,141, 62, 53,
  3,223,142, 60, 52,
  3,286,139, 65, 57,
  4,  3,  3,  4,  4,
  4,  9,  4,  7,  6,
  4,  8, 14, 14, 13,
  4, 30, 14, 28, 26,
  4, 19, 44, 36, 32,
  4, 67, 19, 45, 37,
  4,119, 25, 56, 48,
  4,181, 29, 67, 57,
  4,261, 32, 90, 62,
  4,  3, 67,  4,  6,
  4,  9, 67,  7,  6,
  4,  8, 77, 15, 13,
  4, 33, 78, 32, 28,
  4, 25,114, 48, 42,
  4, 80, 91, 60, 55,
  4, 82,150, 64, 61,
  4,150,100, 71, 73,
  4,226,101, 78, 75,
  4,117,144,  5,  9,
  4,127,148, 12, 17,
  4,141,151, 14, 22,
  4,158,151, 19, 23,
  4,180,151, 23, 23,
  4,205,152, 24, 24,
  4,229,153, 22, 26,
  4,254,154, 27, 28,
  4,280,152, 22, 24,
  4,303,153, 22, 26,
  4,126,182, 23, 26,
  4,151,183, 25, 28,
  4,177,183, 24, 28,
  4,202,183, 24, 28,
  4,227,183, 24, 28,
  4,252,183, 24, 28,
  4,278,184, 26, 30

};

struct XPLANM{
	int	START;
	int 	LAST;
	} XPLOANIMS[]={
		0,16,
		17,24,
		25,32,
		33,49,
		50,58,
		59,67,
		68,84
		};

//***************************************************************************
DrawXploSprite(RPS *Rp, int X, int Y, int NUM, int SCL, int ROLL)
{
	UWORD SEG;

	if (SEG=MapInEMSSprite(SPRTXPLSEG,SPRTVLS[NUM].PAGE))	{
		ScaleRotate( SEG, SPRTVLS[NUM].XC, SPRTVLS[NUM].YC, SPRTVLS[NUM].WID, SPRTVLS[NUM].HT,
			Rp, X, Y, SCL,ROLL, 0, 0 );
	}
}
//***************************************************************************
StartASmoke(long X, UWORD Y, long Z, int TYPE, int CNT)
{
	unsigned int	i,locnt,loi;
	int	R;


	for (loi=0;loi<SMKNUM;loi++) {
		if (SMOKES[loi].CNT<=0) break;
	}
	if (loi==SMKNUM) {
		for (loi=locnt=i=0;loi<SMKNUM;loi++) {
			if (SMOKES[loi].UPCNT>locnt) {
				locnt=SMOKES[loi].UPCNT;
				i=loi;
			}
		}
		loi=i;
	}
	SMOKES[loi].CNT=CNT;
	SMOKES[loi].X=X;
	SMOKES[loi].Y=Y;
	SMOKES[loi].Z=Z;
	SMOKES[loi].TYP=TYPE;
	SMOKES[loi].PIC=-1;
	SMOKES[loi].UPCNT=0;

	switch (SMOKES[loi].TYP) {
		case AIRXPLO:
			R=(Rnd2(255)&3)+3;
			SMOKES[loi].PIC= XPLOANIMS[R].START;
			SMOKES[loi].CNT= XPLOANIMS[R].LAST-XPLOANIMS[R].START;
			break;
		case GRNDXPLO:
			SMOKES[loi].PIC=0;
			SMOKES[loi].Y=50;
			SMOKES[loi].CNT=17;
			break;
		case SMKTRAIL:
		case EXHAUST:
			SMOKES[loi].PIC=XPLOANIMS[(Rnd2(256)&1)+1].START+(Rnd2(256)&3);
			break;
		case SMKPLUME:
			SMOKES[loi].PIC=XPLOANIMS[(Rnd2(256)&1)+1].START;
			break;
		case GRND2XPLO:
			SMOKES[loi].PIC=0;
			SMOKES[loi].Y=50;
			SMOKES[loi].X+=Rnd2(95);
			SMOKES[loi].Z+=Rnd2(95);
			SMOKES[loi].CNT=17;
			break;
		case AIR2XPLO:
			R=(Rnd2(255)&3)+3;
			SMOKES[loi].PIC= XPLOANIMS[R].START;
			SMOKES[loi].CNT= XPLOANIMS[R].LAST-XPLOANIMS[R].START;
			break;
		case CLUSTERXPLO:
			R=3;
			SMOKES[loi].PIC= XPLOANIMS[R].START;
			SMOKES[loi].CNT= 20;
			SMOKES[loi].Y=30;
			break;
		case CLUSTER2XPLO:
			R=3;
			SMOKES[loi].PIC= XPLOANIMS[R].START;
			SMOKES[loi].CNT= (XPLOANIMS[R].LAST-XPLOANIMS[R].START)/4;
			SMOKES[loi].Y=30;
			break;
		case CARRIERXPLO:
			SMOKES[loi].PIC=0;
			SMOKES[loi].Y=50;
			SMOKES[loi].CNT=2;
			break;
		}



}

//***************************************************************************
DrawASmoke(int X, int Y, int Z, int ID, int SCL )
{

	int	V[3];
	int	size;
	long	T;
 	int	R;
	int	XR[3];
	int	i,CNT;

	UWORD D;



	if (SMOKES[ID].TYP==SMKPLUME) {
		D= abs(X)+abs(Y/4)+abs(Z);
		if (D<1000 && Rp3D!=RpCRT) {
			R= 100-(D/10);
			if (R>InXPLO) InXPLO=R;
		}

		CNT= SMOKES[ID].CNT;
		for (i=0;i<4;i++,CNT+=2) {
			if (CNT==0)
				SMOKES[ID].PIC=XPLOANIMS[(Rnd2(256)&1)+1].START;
			XR[1]= Y+ 50*(7-(CNT&7));
			XR[0]= X+ (CNT&0x18)+10*(CNT&7)+ Rnd2(31);
			XR[2]= Z+ (CNT&0x18)+10*(CNT&7)+ Rnd2(31);
			Rotate(XR, Nviewmat);
			if (XR[2]<16) continue;
			if (XR[2]<16) XR[2]=16;
			T= XR[0];
			T= T<<8;
			T= T/(long)XR[2];
			XR[0]=T;
			T= XR[1];
			T= T<<8;
			T= T/(long)XR[2];
			XR[1]=T;
			XR[0]+= GXMID;
			XR[1]+= GYMID;
			size= (((0x7fff/XR[2])/SCL)<<2)+4;
			DrawXploSprite( Rp3D, XR[0], XR[1], SMOKES[ID].PIC+((7-CNT)&7), size,0 );
			if (XPLODETAIL==0) i=4;
		}
		return(1);
	}


	Rotate(&X, Nviewmat, &Y, &Z); /* KLUDGE!!! Really only uses first two parms */
	if (Z<16) return(1);
//	if (Z<256) Z=256;

	T= X;
	T= T<<8;
	T= T/(long)Z;
	X=T;
	T= Y;
	T= T<<8;
	T= T/(long)Z;
	Y=T;

	X+= GXMID;
	Y+= GYMID;

	size= (((0x7fff/Z)/SCL)<<2)+4;
	switch (SMOKES[ID].TYP) {
		case AIRXPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, 3*(size/2),0 );
			break;
		case GRNDXPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, size,vRoll );
			break;
		case SMKTRAIL:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, (size>>1),0 );
			break;
		case EXHAUST:
         if (MISSILETRAIL)
            DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+(SMOKES[ID].UPCNT&3), (size>>2),0);
			break;
		case GRND2XPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, 3*(size/2),vRoll );
			break;
		case AIR2XPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, 3*(size/2),0 );
			break;
		case CLUSTERXPLO:
//			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, (size/3),0 );
			break;
		case CLUSTER2XPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, (size/5),0 );
			break;
		case CARRIERXPLO:
			DrawXploSprite( Rp3D, X, Y, SMOKES[ID].PIC+SMOKES[ID].UPCNT, size>>1,vRoll );
			break;
	}
}

//***************************************************************************
ShowSmokesNoChange(long ViewX, long ViewY, UWORD ViewZ)
{
	int	i;
	int	OLDUP [SMKNUM];
	int	OLDCNT[SMKNUM];

	for (i=0;i<SMKNUM;i++) {
		OLDUP [i]=SMOKES[i].UPCNT;
		OLDCNT[i]=SMOKES[i].CNT;
	}

	HandleSmoke(ViewX, ViewY, ViewZ);

	for (i=0;i<SMKNUM;i++) {
		SMOKES[i].UPCNT=OLDUP [i];
		SMOKES[i].CNT  =OLDCNT[i];
	}
}

//***************************************************************************
HandleSmoke(long ViewX, long ViewY, UWORD ViewZ)
{
	int	i,Y;
	long	X,Z;
	int	V[3];

	for (i=0;i<SMKNUM;i++) {
		if (SMOKES[i].CNT>0) {
			X= SMOKES[i].X-ViewX;
			Y= SMOKES[i].Y-ViewZ;
    		Z = ViewY - (0x100000L-SMOKES[i].Z);
			if ((labs(X)>0x2000) || (labs(Z)>0x2000)) {
				if ((labs(X)<0x3000) && (labs(Z)<0x3000)) {
					if ( !ABOVECLOUDS || (SMOKES[i].TYP==AIRXPLO)
						               || (SMOKES[i].TYP==AIR2XPLO)
						               || (SMOKES[i].TYP==SMKTRAIL)
                                 || (SMOKES[i].TYP==EXHAUST) )
						DrawASmoke((int)X,Y,(int)Z,i, 1);
				}
			} else {
				if (!ABOVECLOUDS || (SMOKES[i].TYP==AIRXPLO)
						            || (SMOKES[i].TYP==AIR2XPLO)
						            || (SMOKES[i].TYP==SMKTRAIL)
						            || (SMOKES[i].TYP==EXHAUST) )
					Stack3Dobject((SPRTXPLSEG<<8), (int) X, Y, (int) Z, i, 1,SPRTXPL_OBJ);
			}


			switch (SMOKES[i].TYP) {
				case AIRXPLO:
					if (SMOKES[i].UPCNT<3)
						StartASmoke( SMOKES[i].X+(long)(Rnd2(127)), SMOKES[i].Y+Rnd2(127), SMOKES[i].Z+(long)(Rnd2(127)),AIR2XPLO, 20 );
					break;
				case GRNDXPLO:
					if (SMOKES[i].CNT<4)
						if ((Rnd2(255)&3))
							StartASmoke( SMOKES[i].X, SMOKES[i].Y, SMOKES[i].Z, GRND2XPLO, 20 );
					if (SMOKES[i].CNT==1)
						StartASmoke( SMOKES[i].X, SMOKES[i].Y, SMOKES[i].Z, SMKPLUME, 2000 );
					break;
				case CLUSTERXPLO:
					if (SMOKES[i].UPCNT<15)
						StartASmoke( SMOKES[i].X+127L-(long)(Rnd2(255)), SMOKES[i].Y, SMOKES[i].Z+127L-(long)(Rnd2(255)),CLUSTER2XPLO, 20 );
					break;
				case SMKPLUME:
					if ( ((SMOKES[i].CNT+Rnd2(255))&0x3c)==0 )
						StartASmoke( SMOKES[i].X, SMOKES[i].Y, SMOKES[i].Z, GRND2XPLO, 20 );
					break;
				case CARRIERXPLO:
					if (SMOKES[i].CNT<4)
						if (!(Rnd2(255)& 31))
							StartASmoke( SMOKES[i].X, SMOKES[i].Y, SMOKES[i].Z, GRND2XPLO, 20 );
					break;
				}

			SMOKES[i].UPCNT++;
			SMOKES[i].CNT--;
		}
	}
}


