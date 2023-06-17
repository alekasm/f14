#include	"stdio.h"
#include	"stdlib.h"
#include <dos.h>
#include "mouse.h"
#include "keys.h"
    union REGS D;


unsigned int EMSSEG=0;
unsigned int EMSHANDLE=0;

typedef unsigned int UWORD;


int	KeyCode	=0;
char	KeyASCII=0;
int	KeyFlags=0;
int	Key=0;


struct   obj3d{
   int   NUM;
   int   XR;
   int   YR;
   int   ZR;
   int   *MAT;
   int   *ANIM;
   long  RELADD[3];
   };


main (argc, argv)
  int     argc;
  char  **argv;
{
   unsigned int SEG;
   char far *S;


	if (argc < 2 )
      {
      printf("No file specified");
      exit(1);
      }


	_dos_allocmem(0x1000,&SEG);
   FP_SEG(S)=SEG;
   FP_OFF(S)=0;

   if (EMMThere())
      {
      if (EMSSEG=EMSPageFrame() )
         {
         EMSHANDLE=EMSAllocatePages(16); // 256 k
         }
      }
	ToVGA();


   DrawMultiObject(S,argc,argv);

	ToTEXT();
   if (EMSHANDLE)
      EMSReleasePages(EMSHANDLE);
	exit(0);
}

WaitTicks(unsigned ticks)
{
    while ( (ticks--) ) {
        while ((inp (0x3DA) & 8) == 0);
        while ((inp (0x3DA) & 8) != 0);
    }
}

ToVGA()
{
	D.x.ax= 0x0013;
	int86(0x10,&D, &D );
}
ToTEXT()
{
	D.x.ax= 0x0003;
	int86(0x10,&D, &D );
}
GetPallet( Block )
	unsigned char	*Block;
{
	char	far *P;
	union	REGS	R;
	struct SREGS SR;

	P= Block;
	SR.es= FP_SEG(P);
	R.x.ax= 0x1017;
	R.x.bx=0;
	R.x.cx=256;
	R.x.dx=FP_OFF(P);
	int86x(0x10, &R, &R, &SR );
}
SetPallet( Block )
	unsigned char	*Block;
{
	char	far *P;
	union	REGS	R;
	struct SREGS SR;

	P= Block;
	SR.es= FP_SEG(P);
	R.x.ax= 0x1012;
	R.x.bx=0;
	R.x.cx=256;
	R.x.dx=FP_OFF(P);
	int86x(0x10, &R, &R, &SR );
}

FlushKeys()
{
	while( GetKeyBoard()!=0 );
}
KeyBoard()
{


	KeyCode=GetKeyBoard();

	KeyASCII=(char) KeyCode;
	Key=(unsigned) (KeyCode/256);

	D.x.ax=0x200;
	int86(0x16,&D,&D);

	KeyFlags=D.x.ax;

	return(KeyCode);
}




extern   char far *PDEST;
extern   int   *PalPtr3D;

int   xtbl[256];


int   GRPAGE0[3]= {0xa000,0xa000,0xa000};

extern   int   NGXLO;
extern   int   NGYLO;
extern   int   NGXHI;
extern   int   NGYHI;
extern   int   NGXMID;
extern   int   NGYMID;


int   WTR2=0;

Horz( long XC,long ZC,int YC,int HEAD,int PITCH,int ROLL,int ARIEL,UWORD BUFFER, int HRES);

DrawMultiObject(char far *DST,int argc,char **argv)
{
   struct   obj3d OB[8];
   int   M[9];
   int   M2[9];
   int   ANIM[40];
   int   i;
   char far *D;
   int   H,P,R,BGS;
   int   X,Y,Z;

   int   DSTSEG;
   int wmove;

   long  ZP,XP;
   int   YP;
   int   HD,PTCH,RLL,HRES;
   int   w,anm;



   anm=100;

   for (i=0;i<40;i++)
      ANIM[i]=anm;


   HD=PTCH=RLL=HRES=0;

   _dos_allocmem(0x1000,&DSTSEG);
   GRPAGE0[1]=FP_SEG(DST);


   PDEST=DST;                      // set the draw buffer
   MakeMatHPR(0,0,0,M2);


   YP=60;

   for (i=1;(i<argc)&&(i<8);i++)
      {
      LoadEMSObject( &argv[i][0],i-1);
      OB[i-1].NUM=i-1;
      OB[i-1].MAT=M2;
      OB[i-1].ANIM=ANIM;
      OB[i-1].XR= 0;
      OB[i-1].YR= 0;
      OB[i-1].ZR= 0;
      }


   FP_SEG(D)=0xa000;
   FP_OFF(D)=0;


   R=X=Y=0;
   Z=-400;

   for (i=0;i<256;i++)
      xtbl[i]=(i<<8)+i;
   PalPtr3D=xtbl;


   MakeMatHPR(0,0,0,M);
   SetViewMat(M,0x100);

   BGS=0;

//   LoadClouds();



   HideMouse();
   while (1)
      {
      UpdateMouse();
		KeyBoard();
      if ( (MouseBtns&3)==3) break;

      switch (MouseBtns&3)
         {
         case 0:
            H+=(MouseXRel<<3);
            P+=(MouseYRel<<3);
            break;
         case 1:
            X+=MouseXRel;
            Y-=MouseYRel;
            break;
         case 2:
            Z+= MouseYRel;
            break;
         }

      switch (Key)
         {
         case SPACEBAR:
            BGS=1;
            break;
         case UPKEY:
            PTCH+=182;
            break;
         case PLUSKEY:
            YP+=100;
            break;
         case DOWNKEY:
            PTCH-=182;
            break;
         case MINUSKEY:
            if ((YP-=100)<10) YP=10;
            break;
         case LEFTKEY:
            RLL-=4*182;
            break;
         case RIGHTKEY:
            RLL+=4*182;
            break;
         case CKEY:
            HRES^=1;
            break;
         case FKEY:
            NGXLO=NGYLO=0;
            NGXHI=319;
            NGYHI=199;
            NGXMID=(NGXLO+NGXHI)/2;
            NGYMID=(NGYLO+NGYHI)/2;
            break;
         case SKEY:
            w=((160-NGXLO)*9)/10;
            NGXLO=160-w;
            NGXHI=160+w;
            w=((100-NGYLO)*9)/10;
            NGYLO=100-w;
            NGYHI=100+w;
            NGXMID=(NGXLO+NGXHI)/2;
            NGYMID=(NGYLO+NGYHI)/2;
            fmemfill(D,0,64000);
            break;
         case AKEY:
            anm^=1;
            for (i=0;i<40;i++)
               ANIM[i]=anm;
            break;

         }

//      HD+=(-RLL/8);

      if (BGS)
         {
         BGS+=10;
         if (BGS>300)
            BGS=0;
         }
      ZP+=ACosB(-10,HD);
      XP+=ASinB(-10,HD);
      wmove=1;

      if (MouseXRel||MouseYRel||BGS||wmove)
         {
         for (i=1;(i<(argc-1))&&(i<4);i++)
            {
            OB[i].ZR=-BGS;
            }
         MakeMatHPR(H,P,R,M);
         PTCH=P;
         RLL=H;
         HD+=(-RLL/8);


         fmemfill(DST,0,64000);
//         Horz( (XP<<10),(ZP<<10),YP, HD,PTCH,RLL,1,DSTSEG, HRES);

         DrawSuper3DObject( OB,0, X,Y,Z, M, ANIM );

//         NDraw3DObject( 0, X,Y,Z, M, ANIM,BGS );

         if (NGXLO==0)
            {
            fmemcpy(DST,D,64000);
            }
         else
            {
            for (i=NGYLO;i<=NGYHI;i++)
               {
               fmemcpy(DST+320*i+NGXLO,D+320*i+NGXLO,NGXHI-NGXLO+1);
               }
            }


         }
      }
   ShowMouse();
}

int   cldhndl=0;
int   wtrhndl=0;
int   wtr2hndl=0;
int   CLOUDSEG=0;
int   WATERSEG=0;
UWORD   GREYSEG=0;

char  far GREYBUF[512];

SmoothWater()
{
   char far *D;
   int   i,j;

   FP_OFF(D)=0;
   FP_SEG(D)=WATERSEG;
   for (i=0;i<128;i++)
      {
      for (j=0;j<127;j++)
         {
         *(D)= (*(D)+*(D+1))/2;
         D++;
         }
      D++;
      }
   FP_OFF(D)=0;
   FP_SEG(D)=WATERSEG;
   for (i=0;i<127;i++)
      {
      for (j=0;j<128;j++)
         {
         *(D)= (*(D)+*(D+128))/2;
         D++;
         }
      }

}

LoadClouds()
{
   int   i,j;
   char far *DST;
   char far *D;
   int   G;


   cldhndl=ReadEMSPCXSize( "cldwtr3.pcx", 1, 256, 256);

   _dos_allocmem(256,&GREYSEG);

   FP_OFF(D)=0;
   FP_SEG(D)=GREYSEG;

   for (i=0;i<16;i++)
      {
      for (j=0;j<256;j++)
         {
         G=(j&15)-i;
         if (G<0) G=0;
         *(D++)= (G+(j&0xf0));
         }
      }





}


UWORD ADDRESS[320];

int	MYWID=208;
#define MYHT	304


int   ABOVECLOUDS=0;
int   CLOUDAMNT=0;
UWORD CLOUDHEIGHT=6800;
int   OVERCAST=1;
int   SKYDETAIL=1;



extern	long	NXSLine( long A, int E, int LN, int HT, int M );
extern	long	NXSADDLine( int C, int HT, int M );



Horz( long XC,long ZC,int YC,int HEAD,int PITCH,int ROLL,int ARIEL,UWORD BUFFER, int HRES)
{
    static int LASTHEIGHT=-1;

	int	i;

	void	far *P;

	long	XS,ZS,XADD,ZADD;

	long 		FA,FB;
	int		HT,CHT,FC,FD,FE,FF;

	int	ARADD,M;

    int     LinesToDo;

	long	T1DEL,T2DEL;
	int	DELSWITCH;

	int	W, CXS,CYS,CXADD,CYADD,ZD,XZD;


	int	YM,XM;
	char	N[13],S[40];


    int ADDBASE;

	int	NWID;


   int   GSEG;


	XZD=1;
	ZD= 64;


   CLOUDSEG=MapEmsFile( cldhndl, 0);


	HT= 2*YC;  /* Why ?? */

	ABOVECLOUDS=0;
	CHT = CLOUDHEIGHT - HT;
	if (CHT<0)
		{
		CHT= -(CHT>>1);
		ABOVECLOUDS=1;
		}

    CLOUDAMNT = (CHT<300)? 100-CHT/3 :0;


	if (SKYDETAIL==0) (CLOUDAMNT>>=2);


    ARADD=  YC/8;

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


	for (i=152-YM;i<=(152+YM);i++)
		{
		M= ACosB( 4*(MYHT/2-i)/3, PITCH)+ASinB(256/XZD,PITCH);
        M--;
		if (abs(M)<2) M= (M<0)? -2:2;




		if (M>0)
			{
			if ( ABOVECLOUDS || (SKYDETAIL==0) )
				{
				NCloudFillLine2( 0,0, 0,
					0, CLOUDSEG, MYWID*(i), BUFFER
					, ((M)>64) ? GREYSEG: GREYSEG, MYWID );
				}
			else
				{
				XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), CHT, M );
				ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), CHT, M );
				XADD= NXSADDLine( FC, CHT, M );
				ZADD= NXSADDLine( FD, CHT, M );
				XS= XS<<1;
				ZS= ZS<<1;
				if (HRES)
					{
					NCloudLine2( (int)((XS-(XC<<1))>>13),(int)((ZS-(ZC<<1))>>13), (int)(XADD>>13),
						(int)(ZADD>>13), CLOUDSEG, MYWID*(i), BUFFER
					, ((M)>47) ? GREYSEG: GREYSEG+16*(12-(M/4)), MYWID );
					}
				else
					{
					NCloudLine( (int)((XS-(XC<<1))>>13),(int)((ZS-(ZC<<1))>>13), (int)(XADD>>13),
						(int)(ZADD>>13), CLOUDSEG, MYWID*(i), BUFFER
					, ((M)>47) ? GREYSEG: GREYSEG+16*(12-(M/4)), MYWID );
					}
				}


			}
		else
			{
         GSEG= (ABOVECLOUDS)? CLOUDSEG+2048:CLOUDSEG+8;
         if (ABOVECLOUDS)
            {
			   XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), CHT, -M );
			   ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), CHT, -M );
			   XADD= NXSADDLine( FC, CHT, -M );
			   ZADD= NXSADDLine( FD, CHT, -M );
			   XS= XS<<1;
			   ZS= ZS<<1;
            if (HRES)
               {
				   NCloudLine2( (int)((XS-(XC<<1))>>11),(int)((ZS-(ZC<<1))>>11), (int)(XADD>>11),
					   (int)(ZADD>>11), GSEG, MYWID*(i), BUFFER
				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
               }
            else
               {
				   NCloudLine( (int)((XS-(XC<<1))>>11),(int)((ZS-(ZC<<1))>>11), (int)(XADD>>11),
					   (int)(ZADD>>11), GSEG, MYWID*(i), BUFFER
				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
               }
//            if (HRES)
//               {
//				   NCloudLine2( (int)((XS-(XC<<1))>>13),(int)((ZS-(ZC<<1))>>13), (int)(XADD>>13),
//					   (int)(ZADD>>13), GSEG, MYWID*(i), BUFFER
//				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
//               }
//            else
//               {
//				   NCloudLine( (int)((XS-(XC<<1))>>13),(int)((ZS-(ZC<<1))>>13), (int)(XADD>>13),
//					   (int)(ZADD>>13), GSEG, MYWID*(i), BUFFER
//				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
//               }
            }
         else
            {
			   XS= NXSLine( FA, FE, (4*(MYHT/2-i)/3), HT, -M );
			   ZS= NXSLine( FB, FF, (4*(MYHT/2-i)/3), HT, -M );
			   XADD= NXSADDLine( FC, HT, -M );
			   ZADD= NXSADDLine( FD, HT, -M );
			   XS= XS<<1;
			   ZS= ZS<<1;
            if (HRES)
               {
				   NCloudLine2( (int)((XS-(XC<<1))>>6),(int)((ZS-(ZC<<1))>>6), (int)(XADD>>6),
					   (int)(ZADD>>6), GSEG, MYWID*(i), BUFFER
				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
               }
            else
               {
				   NCloudLine( (int)((XS-(XC<<1))>>6),(int)((ZS-(ZC<<1))>>6), (int)(XADD>>6),
					   (int)(ZADD>>6), GSEG, MYWID*(i), BUFFER
				   , ((-M)>8) ? GREYSEG: GREYSEG+16*(8-(-M)), MYWID );
               }

            }

			}

		}


		if ( 1 )
			{
			W=NWID /*MYWID*/;
// make the address table

			CXADD= ACosB(32*2,ROLL);
			CYADD= ASinB(24*2,ROLL);
			CXS= (-W/2)*CXADD;
			CYS= (-W/2)*CYADD;

			for (i=0;i<W;i++)
				{
				ADDRESS[i]= MYWID*(CYS/ZD)+ (CXS/(2*ZD));
				CXS+= CXADD;
				CYS+= CYADD;
				}
// draw all the lines
			for (i=NGYLO;i<=NGYHI;i++)
				{
	ADDBASE= MYWID*(ACosB(i-NGYMID,ROLL)+MYHT/2)+ ASinB((-2*(i-NGYMID))/3,ROLL)+ MYWID/2;

            if (HRES)
               {
				   NScaleLine2( BUFFER, ADDBASE,
								   ADDRESS, NGXLO/*160-W/2*/, i, NWID/*W*/, GRPAGE0[1] );
               }
            else
               {
   				NScaleLine( BUFFER, ADDBASE,
								ADDRESS, NGXLO/*160-W/2*/, i, NWID/*W*/, GRPAGE0[1] );
               }
				}
			}
}

/****************************************************************************/
