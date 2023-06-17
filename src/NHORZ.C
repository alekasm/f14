#include "stdio.h"
#include <stdlib.h>
#include <dos.h>
#include "library.h"
#include "world.h"
#include "f15defs.h"
#include <fcntl.h>
#include "planeai.h"
#include "3dobjdefs.h"

extern UWORD   *PalPtr3D;
extern	long	ViewX,ViewY;

extern UBYTE far AAFIRE[];
extern int FrameRate;

extern	long labs(long V);

UWORD	GREYSEG =0xA000;
UWORD   CLOUDSEG=0xA000;

extern   int   NGXLO;
extern   int   NGYLO;
extern   int   NGXHI;
extern   int   NGYHI;
extern   int   NGXMID;
extern   int   NGYMID;
extern	long	NXSLine( long A, int E, int LN, int HT, int M );
extern	long	NXSADDLine( int C, int HT, int M );

int   cldhndl=0;

extern	struct RastPort    *Rp;
extern	int	GMODE;
extern	int	GRPAGE;
extern	int	GRPAGE0[];
extern	int	GXLO;
extern	int	GYLO;
extern	int	GXHI;
extern	int	GYHI;
extern	int	GXMID;
extern	int	GYMID;
extern  int     detected;
extern  UWORD   Seg3D;
extern	int	Nviewmat[];
extern  int     CLOUDAMNT;
extern  int     ViewZ;

int GrndAnimVals[32];

extern int BelowDeck;

extern   char far GRTBLSEG[];


UWORD	CLOUDHEIGHT=8150;
// UWORD	CLOUDHEIGHT=125;
int		CLOUDTHICK=500;

struct OBJ3D far OBJECTS[96][16];
int   GroundObjectScale[200];
char  GroundObjectNames[32][20];



struct STK3D{
	int OBJ;
	int X;
	int Y;
	int Z;
    int ID;
	int NXT;
	UWORD DST;
	int TYPE;
	int SCL;
} far STACK3D[52];

LASTSTACK=0;
FARSTACK=-1;


int ABOVECLOUDS=0;
int	GRNDDETAIL=1;
int	OVERCAST=0;
int SKYDETAIL=1;
int WORLDDETAIL=0;

#define Width2 160

#define MTNTILE 95

int	MTNRAD= 6;
int	STKWID= 2;
int	FLSHXOFF;
int	FLSHZOFF;
int	GFLSH=0;
struct	SNRM{
	int	NI;
	int	NJ;
	int	NK;
	long	D;
};

UWORD ADDRESS[320];

int	MYWID=208;
#define MYHT	304

//***************************************************************************
ChangeWeather()
{
	static int BadWeather=0;

	if (BadWeather) {
		CLOUDHEIGHT=8150;
		CLOUDTHICK=500;
		BadWeather=0;
	} else {
		CLOUDHEIGHT=125;
		CLOUDTHICK=1000;
		BadWeather=1;
	}
}

//***************************************************************************
LoadClouds()
{
	int	DHNDL;
	unsigned 	len;

	cldhndl=ReadEMSPCXSize("cldwtr5.pcx",0,256,256);
	if(_dos_open("wld.gry",O_RDONLY,&DHNDL)==0) {
		_dos_read(DHNDL,GRTBLSEG,16*256,&len);
		_dos_close(DHNDL);
	}
}


//***************************************************************************
int   GRYSFHTTBL[16]={
   0x800,0x1000,0x1800,0x2000,
   0x2800,0x3000,0x3800,0x4000,
   0x4800,0x5000,0x5800,0x6000,
   0x6800,0x7000,0x7000,0x7000,
};

extern   int   HorzHeave;

NHorz(long XC,long ZC,int YC,int HEAD,int PITCH,int ROLL,int ARIEL,UWORD BUFFER, int HRES)
{
    static int LASTHEIGHT=-1;
	UWORD ADDRESS[320];
	int	i;
	void far *P;
	long XS,ZS,XADD,ZADD;
	long FA,FB;
	int	 HT,CHT,FC,FD,FE,FF;
	int	 ARADD,M;
    int  LinesToDo;
	long T1DEL,T2DEL;
	int	 DELSWITCH;
	int	 W,CXS,CYS,CXADD,CYADD,ZD,XZD;
	int	 YM,XM;
	char N[13],S[40];
    int  ADDBASE;
	int	 NWID;
	int  GSEG;
	int  GRY;
	int  MSK,SHFT;
	int	 MYM;


	XZD=1;
	ZD= 64;

	CLOUDSEG=MapEmsFile(cldhndl, 0);

	HT= 2*YC;  /* Why ?? */


   HRES=SKYDETAIL;


	ABOVECLOUDS=0;
	CHT = CLOUDHEIGHT - HT;
	if (CHT<0) {
		CHT= -(CHT>>1);
		ABOVECLOUDS=1;
	}

	CLOUDAMNT = (CHT<CLOUDTHICK) ? (100 - (CHT/(CLOUDTHICK/100))) : 0;

	if (SKYDETAIL==0) (CLOUDAMNT>>=2);

    ARADD = YC/8;
	if (ARADD<4) ARADD=4;

	if (ARADD>28) ARADD=28;

    LinesToDo= NGYHI-NGYMID+2;
    if (NGYMID-NGYLO >LinesToDo) LinesToDo=NGYMID-NGYLO;

	NWID= (((NGXHI-NGXLO)+7)&0xfff8);

	YM= abs(ASinB((128*(NWID/2))/160,ROLL))+abs(ACosB(LinesToDo,ROLL));
	if (YM>152) YM=152;
	if (YM<LinesToDo) YM=LinesToDo;

	XM= abs(ACosB((NWID/2)+8,ROLL))+abs(ASinB((148*LinesToDo)/100,ROLL));
	if (XM>208) XM=208;
	if (XM<160) XM=160;

	MYWID=XM;

	i= ACosB(MYWID,HEAD)+ ASinB(ACosB(256/XZD,PITCH),HEAD);
	FA= (long)(i)<<10;
	i= ASinB(-MYWID,HEAD)+ ACosB(ACosB(256/XZD,PITCH),HEAD);
	FB= (long)(i)<<10;

	FC= ACosB(-2048,HEAD);
	FD= ASinB(2048,HEAD);

	FE= -ASinB(ASinB(1024,PITCH),HEAD);
	FF= -ACosB(ASinB(1024,PITCH),HEAD);

	for (i=152-YM;i<=(152+YM);i++) {
		M= ACosB( 4*(MYHT/2-i)/3, PITCH)+ASinB(256/XZD,PITCH);
        M--;

		M+= HorzHeave;


      if (!ABOVECLOUDS)
         {
         M+= (HT/300);
         }


		MYM=M;
		if (abs(M)<2) M= (M<0)? -2:2;

      if (SKYDETAIL==0)
         {
         FP_SEG(P)=BUFFER;
         FP_OFF(P)=MYWID*(i);
         if (M>0)
            {
            GRY= (M<8)? 0x80+(M&15):0x88;
            }
         else
            {
            GRY= ((-M)<12)? 0xf0+((-M)&15):0xfc;
            if (ABOVECLOUDS)
               GRY= ((-M)<3)? 0x80+((-M)&15):0x82;

            }
         GRY= (GRY<<8)|GRY;
         fmemfill(P,GRY,MYWID); // remember to grey out the 0x84
         continue;
         }

      if (M>0) {
			if (ABOVECLOUDS || (SKYDETAIL==0)) {
            	FP_SEG(P)=BUFFER;
            	FP_OFF(P)=MYWID*(i);
            	GRY= (M<16)? 0x80+(M&15):0x8f;
            	GRY= (GRY<<8)|GRY;
            	fmemfill(P,GRY,MYWID); // remember to grey out the 0x84
			} else {
				XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), CHT, M );
				ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), CHT, M );
				XADD= NXSADDLine( FC, CHT, M );
				ZADD= NXSADDLine( FD, CHT, M );
				XS= XS<<1;
				ZS= ZS<<1;

				GRY=2048;

				MSK=0x7f;
//            	SHFT=max( abs((int)(XADD>>13)),abs((int)(ZADD>>13)) );
				DELSWITCH=0;

				if (SHFT>0x200) {
					DELSWITCH++;
					MSK>>=1;
				}
				if (SHFT>0x400) {
					DELSWITCH++;
					MSK>>=1;
				}
            	if (SHFT>0x800) {
               		DELSWITCH++;
              	 	MSK>>=1;
				}

            	MSK=0;
            	if (abs(MYM)<16) {
               		P=GRTBLSEG;
               		MSK=16*(15-abs(MYM))+FP_SEG(P);
				}


         if (HRES==1)
            {
            MyWaterline2((int)((XS-(XC<<1))>>(13)),(int)((ZS-(ZC<<1))>>(13)),
                        (int)(XADD>>(13)),(int)(ZADD>>(13)),
                        CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                        MSK, MYWID );
            }
         else
            {
            MyWaterline((int)((XS-(XC<<1))>>(13)),(int)((ZS-(ZC<<1))>>(13)),
                        (int)(XADD>>(13)),(int)(ZADD>>(13)),
                        CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                        MSK, MYWID );

            }

			}
		} else {
			GSEG= (ABOVECLOUDS)? CLOUDSEG+2056:CLOUDSEG+8;
			if (ABOVECLOUDS) {
				XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), CHT, -M );
				ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), CHT, -M );
				XADD= NXSADDLine( FC, CHT, -M );
				ZADD= NXSADDLine( FD, CHT, -M );
				XS= XS<<1;
				ZS= ZS<<1;
				GRY=2056;
				MSK=0x7f;
//            	SHFT=max( abs((int)(XADD>>11)),abs((int)(ZADD>>11)) );

				DELSWITCH=0;
            	if (SHFT>0x200) {
               		DELSWITCH++;
               		MSK>>=1;
				}

				if (SHFT>0x400) {
    				DELSWITCH++;
    				MSK>>=1;
				}
				if (SHFT>0x800)	{
    				DELSWITCH++;
    				MSK>>=1;
				}

				MSK=0;
				if (abs(MYM)<16) {
    				P=GRTBLSEG;
    				MSK=16*(15-abs(MYM))+FP_SEG(P);
				}

         if (HRES==1)
            {
				MyWaterline2((int)((XS-(XC<<1))>>(11)),(int)((ZS-(ZC<<1))>>(11)),
                           (int)(XADD>>(11)),(int)(ZADD>>(11)),
                           CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                           MSK, MYWID );

            }
         else
            {
				MyWaterline((int)((XS-(XC<<1))>>(11)),(int)((ZS-(ZC<<1))>>(11)),
                           (int)(XADD>>(11)),(int)(ZADD>>(11)),
                           CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                           MSK, MYWID );

            }
			} else {
				XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), HT, -M );
				ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), HT, -M );
				XADD= NXSADDLine( FC, HT, -M );
				ZADD= NXSADDLine( FD, HT, -M );
				XS= XS<<1;
				ZS= ZS<<1;
				GRY=8;

            	SHFT=max( abs((int)(XADD>>6)),abs((int)(ZADD>>6)) );

            	MSK=0;
            	if (abs(M)<ARADD) {
               		MSK=(ARADD-abs(M))/2;
               		P=GRTBLSEG;
               		MSK= 16*(MSK)+FP_SEG(P);
				}
/*
            	for (MSK=0;MSK<13;MSK++) {
               		if (GRYSFHTTBL[MSK]>SHFT) break;
               	}
            	P=GRTBLSEG;
            	if (abs(MYM)<7)	{
               	if (MSK<(15-2*abs(MYM)))
                  	MSK=(15-2*abs(MYM));
               	}
            	if (MSK)
               	MSK= 16*(MSK)+FP_SEG(P);
*/

         if (HRES==1)
            {
				MyWaterline2((int)((XS-(XC<<1))>>(6)),(int)((ZS-(ZC<<1))>>(6)),
                           (int)(XADD>>(6)),(int)(ZADD>>(6)),
                           CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                           MSK, MYWID );
            }
         else
            {
				MyWaterline((int)((XS-(XC<<1))>>(6)),(int)((ZS-(ZC<<1))>>(6)),
                           (int)(XADD>>(6)),(int)(ZADD>>(6)),
                           CLOUDSEG+(GRY), MYWID*(i), BUFFER,
                           MSK, MYWID );

            }

			}
		}
	}

		if (1) {
			W=NWID /*MYWID*/;
			// make the address table

			CXADD= ACosB(32*2,ROLL);
			CYADD= ASinB(24*2,ROLL);
			CXS= (-W/2)*CXADD;
			CYS= (-W/2)*CYADD;

			for (i=0;i<W;i++) {
				ADDRESS[i]= MYWID*(CYS/ZD)+ (CXS/(2*ZD));
				CXS+= CXADD;
				CYS+= CYADD;
			}

			// draw all the lines
			for (i=NGYLO;i<=NGYHI;i++) {
				ADDBASE= MYWID*(ACosB(i-NGYMID,ROLL)+MYHT/2)+ ASinB((-2*(i-NGYMID))/3,ROLL)+ MYWID/2;

   			NScaleLine( BUFFER, ADDBASE, ADDRESS,
                        NGXLO, i, NWID, GRPAGE0[1] );

//            	if (HRES) {
//				   	NScaleLine2( BUFFER, ADDBASE,
//								   	ADDRESS, NGXLO/*160-W/2*/, i, NWID/*W*/, GRPAGE0[1] );
//				} else {
//   					NScaleLine( BUFFER, ADDBASE,
//									ADDRESS, NGXLO/*160-W/2*/, i, NWID/*W*/, GRPAGE0[1] );
//				}
//


         }
		}
}

//***************************************************************************
EndStack3D()
{
	int	    i;
	long SCL;
	int	SHFT;
	long	t1,t2,t3;
	int		X,Y,Z;

	for (i=FARSTACK;i!=-1;) {
		SCL= STACK3D[i].SCL;
		X= STACK3D[i].X;
		Y= STACK3D[i].Y;
		Z= STACK3D[i].Z;
		if ( SCL>1 )
			{
			t1= X;
			t1*= SCL;
			t2= Y;
			t2*= SCL;
			t3= Z;
			t3*= SCL;
			if ((labs(t1) > 6000) || (labs(t2) > 6000) || (labs(t3) > 6000))
				{
				X= (t1 >>= 3);
				Y= (t2 >>= 3);
				Z= (t3 >>= 3);
				SHFT= 3;
				}
			else
				{
				X=t1;
				Y=t2;
				Z=t3;
				SHFT=0;
				}
			}

		switch(STACK3D[i].TYPE) {
			case CARRIER_OBJ_1:
			case CARRIER_OBJ_2:
			case CARRIER_OBJ_6:
			case CARRIER_OBJ_7:
			case CARRIER_OBJ_8:
			case CARRIER_OBJ_9:
			case CARRIER_OBJ_10:
			case CARRIER_OBJ_11:
				DrawCarrier(X, Y, Z,STACK3D[i].ID,SHFT,STACK3D[i].TYPE);
			break;
			case COOLF14_OBJ:
				DrawCoolF14(X, Y, Z,SHFT);
			break;

			case MISSILE_OBJ:
                DrawMissile(STACK3D[i].OBJ, X, Y, Z, STACK3D[i].ID,SHFT);
			break;
			case SPRTXPL_OBJ:
				if (MapIn3dObjects((STACK3D[i].OBJ >>8))) {
					DrawASmoke(X, Y, Z, STACK3D[i].ID, 1,SHFT);
				}
			break;
			case SHADOW_OBJ:
				DrawShadow(STACK3D[i].OBJ, X, Y, Z, STACK3D[i].ID,SHFT);
			break;
			case PLANES_OBJ:
				DrawTexPlane(STACK3D[i].OBJ, X, Y, Z, STACK3D[i].ID,SHFT);
			break;
			case CHUTE_OBJ:
				DrawChute(STACK3D[i].OBJ, X, Y, Z,SHFT);
			break;
			case FLARE_OBJ:
			case CHAFF_OBJ:
				DrawDecoy(STACK3D[i].OBJ, X, Y, Z,SHFT);
			break;
			case REG_BOAT_OBJ:
				DrawRegBoat(STACK3D[i].OBJ, X, Y, Z, STACK3D[i].ID,SHFT);
			break;
			default:
				Message("Error",DEBUG_MSG);
			break;
		}
		i=STACK3D[i].NXT;
	}

	LASTSTACK=0;
	FARSTACK=-1;

}

//***************************************************************************
Stack3Dobject(int OBJ, int X, int Y, int Z, int ID, UWORD SCL, int objtype)
{
	int	i,prev;
	UWORD DST;
	int	V[3];

	if (LASTSTACK>49) return 0;

	DST=Dist3D(X,Y,Z);

	/* Mike KLUDGE */

	if (objtype == CARRIER_OBJ_2) {
		if (DST < 30000)
			DST += 2000;
	}

	if (objtype == CARRIER_OBJ_1)
	{
		if (ViewZ < (66/4))
		{
			DST=0;
#ifdef YEP
			if ((DST-=1000) < 0)
			{
				DST = 0;
			}
#endif
		} else
		{
			if (DST < 30000)
				DST+=1000;
		}
	}

	V[0]=X;
	V[1]=Y;
	V[2]=Z;
	Rotate(V,Nviewmat);
	V[2]+= 500;

//	if (abs(V[0]/2)>(V[2])) return 0;
//	if (abs(V[1]/2)>(V[2])) return 0;

	STACK3D[LASTSTACK].OBJ=OBJ;
	STACK3D[LASTSTACK].X=X;
	STACK3D[LASTSTACK].Y=Y;
	STACK3D[LASTSTACK].Z=Z;
	STACK3D[LASTSTACK].ID=ID;
	STACK3D[LASTSTACK].NXT=-1;
	STACK3D[LASTSTACK].DST=DST;
	STACK3D[LASTSTACK].TYPE=objtype;
	STACK3D[LASTSTACK].SCL=SCL;

	if (LASTSTACK==0) {
		FARSTACK=0;
		LASTSTACK++;
		return 1;
	}

	for (i=FARSTACK,prev=-1;i!=-1;) {

		if (DST>STACK3D[i].DST) break;
		prev=i;
		i=STACK3D[i].NXT;
	}
	STACK3D[LASTSTACK].NXT=i;

	if (prev==-1)
		FARSTACK=LASTSTACK;
	else
		STACK3D[prev].NXT=LASTSTACK;
	LASTSTACK++;

    return 1;
}


//***************************************************************************
DrawAAA()
{
	long dx, dy;
	int dz;
	long t1;
	long t2;
	int cnt;
	int acnt;
	int adx[] = { 525, 525,   0, -525, -525};
	int ady[] = {-525, 525,   0,  525, -525};
    UWORD       Save;
    int        VM[9];
    int        Angle2;

    UBYTE far  *Fred=AAFIRE;
    static UWORD Angle1=0;

    Save = Seg3D;
    Seg3D = FP_SEG(Fred);

	for(cnt = 0; cnt < NGTARGETS; cnt ++) {
		if(gt[cnt].gflags & AAA_FIRING) {
			dz = GetQHeight (gt[cnt].x, gt[cnt].y) - ViewZ;
			if (abs(dz) > 0x3ffe)
				continue;
			for(acnt = 0; acnt < 5; acnt++) {

				dx = (gt[cnt].x + (long)adx[acnt]) - ViewX;
		    	dy = ViewY - (0x100000L-(gt[cnt].y + (long)ady[acnt]));

				t1 = labs(dy);
				if (t1>0x3fff)
					continue;
				t1 = labs(dx);
				if (t1>0x3fff)
					continue;

        		Angle2 = (Angle1+((cnt + (acnt<<2))&15)*512)&8191;
        		MakeMatRPH (0, 5471, ((cnt + (acnt<<2))&15)*0x1000, VM);
        		NDraw3DObject(N3DZ_AAA, (int)dx, dz, (int)dy, VM, &Angle2, 0);
			}
		}
	}

    Angle1 += 2048/FrameRate;
    Seg3D = Save;
}


