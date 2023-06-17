#include	"stdio.h"
#include	"stdlib.h"
#include <dos.h>
#include <process.h>
#include "library.h"

    union REGS D;


typedef unsigned int UWORD;


struct	OBJ3D {
	int	NUM;
	int	X;
	int	Y;
};
extern   struct OBJ3D far OBJECTS[96][16];
extern   int   IdentInvMat[];
extern   int   GroundObjectScale[200];
extern   char  GroundObjectNames[32][20];


int   LandObjectBase=0;

struct	PPNT{
	int	X;
	int	Y;
	int	Z;
	int	XT;
	int	YT;
	};

struct	CPT32{
	long	X;
	long	Y;
	long	Z;
	int	XT;
	int	YT;
	};

struct	PT3D{
	long	XR;
	long	YR;
	long	ZR;
	long	XP;
	long	YP;
	long	ZP;
	int	XT;
	int	YT;
	int	ZT;
	int	FLAGS;
	};
// bits   15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00
// FLAGS=  x   x   x  PD PYH PYL PXH PXL   x  RD RZH RZL RYH RYL RXH RXL
#define	RXLFLAG	1
#define	RXHFLAG	2
#define	RYLFLAG	4
#define	RYHFLAG	8
#define	RZLFLAG	16
#define	RZHFLAG	32
#define	RDFLAG	64
#define	ROUTS		63

#define	PXLFLAG	256
#define	PXHFLAG	512
#define	PYLFLAG	1024
#define	PYHFLAG	2048
#define	PDFLAG	4096
#define	POUTS		0x0f00

extern	long	GXLO32,GYLO32,GXHI32,GYHI32;
extern	int	GXLO,GYLO,GXHI,GYHI,GXMID,GYMID;

extern	long MulDiv32( long V1, long V2, long V3 );
extern	NPFLine( void far *P1, void far *P2, int TYPE);
extern   long  DotProduct( int p1, int P2, int P3, int P4, int P5, int P6);

extern   FlagRotate3D(struct PT3D far *P);
extern   Perspect3D(struct	PT3D far *P);
extern   ZClip( struct PT3D far *PIN,struct PT3D far *POUT,struct PT3D far *PDST,int TYPE);
extern   Npoly3D(int Num, struct	PT3D far *List[], char far *D, int TYPE, char far *S );


extern	int	Nviewmat[];

extern   int   mapscrollX;
extern   int   mapscrollY;
extern   int   mapzoom;
extern   int   oldeditmode;

extern   struct RastPort    *Rp;


#define	GOUROUD 1
#define	TEXTURE 2


struct	PT3{
	long	X;
	long	Y;
	long	Z;
	};
struct	RECT3{
	struct	PT3 P1;
	struct	PT3 P2;
	};



int	WORLDXOFF;
int	WORLDYOFF;
int	WORLDZOFF;
int	XBASE;
int	ZBASE;
int   WORLDXTRAX;
int   WORLDXTRAY;
int   WORLDXTRAZ;
extern char Txt[];
int MikeHeight;

extern   unsigned int GRPAGE;
extern unsigned int GRPAGE0[];


#define  HMAPBANK 0
#define  BMAPBANK 1
#define  SDEFBANK 2
#define  STMPBANK 3
#define  TILEBANK 3+8
#define  NUMBANKS 3+8+8


//////////////////////////////////////////////////////







struct   GRDPT{
   int   HT;
   struct	PT3D PT;
   };

struct   SBox{
   char  TYPE1;
   char  TYPE2;
   char  CLR1;
   char  CLR2;
   int   BITID;
   char  VIS1;
   char  VIS2;
   int   XL;
   int   YL;
   int   XH;
   int   YH;
   };

struct   STMP{
   char  MAP[64];
   };

//extern   struct   GRDPT far GPOINTS[2*16][16];
//extern   struct   SBox  far SBOXES[2*16][16];

extern	int	    GRNDDETAIL;


char far *ScreenAddr;
char  far   *BitMaps[4];
int   PLAINVAL=0;


DrawSBox(struct SBox far *B,struct PT3D far *P1,struct PT3D far *P2,
                  struct	PT3D far *P3,struct	PT3D far *P4,char far *DST)
{
	struct	PT3D far *List[4];
   int   CLR;


   if ((B->VIS1&0x80)&&(GRNDDETAIL==2))
      {
      List[0]=P1;
      List[1]=P2;
      List[2]=P4;
      List[3]=P3;
      switch (GRNDDETAIL)
         {
         case 0:           //flat dither
            CLR=(2*B->CLR1)+0x6e;
            Npoly3D( 4, &List[0], DST, 0,(char far *)((CLR<<8)+CLR+1) );
            return;
            break;
         default:
            P1->XT=P3->XT= B->XL;
            P4->YT=P3->YT= B->YH;
            P1->YT=P2->YT= B->YL;
            P2->XT=P4->XT= B->XH;
            MapInBank(STMPBANK+((B->BITID/16)&15));
            Npoly3D( 4, &List[0], DST, (B->CLR1<<8)+2+4, BitMaps[(B->BITID/16)&3] );
            return;
            break;
         }

      }

   if (B->VIS1)
      {
      List[0]=P1;
      List[1]=P2;
      List[2]=P4;
      switch (GRNDDETAIL)
         {
         case 0:           //flat dither
            CLR=(2*B->CLR1)+0x6e;
            Npoly3D( 3, &List[0], DST, 0,(char far *)((CLR<<8)+CLR+1) );
            break;
         case 1:
            P1->XT=-(B->XL&0xf)+0x6e;
            P2->XT=-(B->YL&0xf)+0x6e;
            P3->XT=-(B->XH&0xf)+0x6e;
            P4->XT=-(B->YH&0xf)+0x6e;
            Npoly3D( 3, &List[0], DST, 1,(char far *)((CLR<<8)+CLR+1) );
            break;
         case 2:
         default:
            P1->XT= B->XL;
            P1->YT=P2->YT= B->YL;
            P2->XT=P4->XT= B->XH;
            P4->YT= B->YH;
            MapInBank(STMPBANK+((B->BITID/16)&15));
            Npoly3D( 3, &List[0], DST, (B->CLR1<<8)+2+4, BitMaps[(B->BITID/16)&3] );
            break;
         }

      }
   if (B->VIS2)
      {
      List[0]=P1;
      List[1]=P4;
      List[2]=P3;
      switch (GRNDDETAIL)
         {
         case 0:           //flat dither
            CLR=(2*B->CLR2)+0x6e;
            Npoly3D( 3, &List[0], DST, 0,(char far *)((CLR<<8)+CLR+1) );
            break;
         case 1:
            P1->XT=-(B->XL&0xf)+0x6e;
            P2->XT=-(B->YL&0xf)+0x6e;
            P3->XT=-(B->XH&0xf)+0x6e;
            P4->XT=-(B->YH&0xf)+0x6e;
            Npoly3D( 3, &List[0], DST, 1,(char far *)((CLR<<8)+CLR+1) );
            break;
         case 2:
         default:
            P1->XT=P3->XT= B->XL;
            P1->YT= B->YL;
            P4->XT= B->XH;
            P4->YT=P3->YT= B->YH;
            MapInBank(STMPBANK+((B->BITID/16)&15));
            Npoly3D( 3, &List[0], DST, (B->CLR2<<8)+2, BitMaps[(B->BITID/16)&3] );
            break;
         }

      }
}


MakeMidGridPt(struct GRDPT far *P1,struct GRDPT far *P2,struct GRDPT far *P3, int FRACT)
{
   int   H1,H2,HDEL;
	int	V[3];
   long  VL[3];

// if FRACT==1 add some rotated random HT(based on ??)

   *(P3)=*(P1);
   P3->HT+= P2->HT;
   P3->HT>>=1;
   P3->PT.XR+= P2->PT.XR;
   P3->PT.XR>>=1;;
   P3->PT.YR+= P2->PT.YR;
   P3->PT.YR>>=1;;
   P3->PT.ZR+= P2->PT.ZR;
   P3->PT.ZR>>=1;;


   FRACT=0;


   if (FRACT)
      {
      V[0]=0;
      V[2]=0;
	   Mult1X3( V,Nviewmat,VL);
      P3->PT.XR+=VL[0];
      P3->PT.YR+=VL[1];
      P3->PT.ZR+=VL[2];
      }


   FlagRotate3D(&P3->PT);
}



MakePTS(struct GRDPT far *PTSBASE)
{
   int   i,j,k;
	int	V[3];
	struct	PT3D	BGS;
   int   x,z;
   int   tx,tz;
   int   H1,H2,H3,H4;
   struct GRDPT far *P;
   struct GRDPT far *PL;
   struct	PT3D far *T;
   char far *PTR;


   FP_OFF(PTR)=0;
   FP_SEG(PTR)=MapInBank(HMAPBANK);

   P=PTSBASE;
   tz=1024*(-8)+WORLDZOFF;
   for (i=0,z=((-8)+ZBASE);i<16;i++,z++,tz+=1024)
      {
      tx=1024*(-8)+WORLDXOFF;
      for (j=0,x=((-8)+XBASE);j<16;j++,x++,tx+=1024)
         {
//         P->HT=( (*(PTR+256*(z&0xff)+(x&0xff))) <<4);
         P->HT=( ( (*(PTR+256*(z&0xff)+(x&0xff)))&0xff) <<3);
	      V[0]= tx;
	      V[1]= P->HT+WORLDYOFF;
	      V[2]= tz;

         if ( (abs(8-i)<4)&&(abs(8-j)<4) )
            {
            V[0]= 4*V[0]+WORLDXTRAX;
            V[1]= 4*V[1]+WORLDXTRAY;
            V[2]= 4*V[2]+WORLDXTRAZ;
            }

	      Mult1X3( V,Nviewmat,&(BGS.XR) );
	      T=&P->PT;
         *(T)=BGS;
         FlagRotate3D(T);
         P++;
         }
      }

}

MakeBoxes(struct SBox far *BOXBASE)
{
   struct SBox far *B;
   struct SBox far *BL;
   int   i,j,k,t;
   char far *PTR;

   B=BOXBASE;

   FP_OFF(PTR)=0;
   FP_SEG(PTR)=MapInBank(BMAPBANK);


   for (i=0;i<15;i++,B++)
      {
      for (j=0;j<15;j++,B++)
         {
         t=(abs(8-i)+abs(8-j))/2;
//         t=max(abs(8-i),abs(8-j))/2;
//         t=2*max(abs(8-i),abs(8-j));
         if (t>6) t=6;
         t=-t;
         B->CLR1=B->CLR2=t;

//         B->CLR1=B->CLR2= 0;
         if ( (i<PLAINVAL)||(j<PLAINVAL)||(i>=(15-PLAINVAL))||(j>=(15-PLAINVAL)) )
            {
            B->TYPE1=B->TYPE2=0;
            B->CLR1+=3;
            B->CLR2+=3;
            }
         else
            {
            B->TYPE1=B->TYPE2=2;

            B->BITID= *(PTR+256*((ZBASE-8+i)&0xff)+((XBASE-8+j)&0xff) );
//            B->BITID= bmap[(ZBASE-8+i)&0xff][(XBASE-8+j)&0xff];
//            if (t>3)
//               B->BITID+=16;

//            t=cmap[(ZBASE-8+i)&0xff][(XBASE-8+j)&0xff]&0xff;
//            B->CLR1+=t;
//            B->CLR2+=t;
            B->XL=64*(B->BITID&3);
            B->YL=64*((B->BITID/4)&3);
            B->XH=B->XL+63;
            B->YH=B->YL+63;
            }
         }
      }

}

MakeBoxVis(struct SBox far *BOXBASE,struct GRDPT far *PTSBASE)
{

   struct SBox far *B;
   struct SBox far *BL;
   struct GRDPT far *P;
   int   i,j,k,t;
   int   tx,tz,H1,H2,H3,H4;

   B=BOXBASE;
   P=PTSBASE;

   tz=(8*1024)-WORLDZOFF;
   for (i=0;i<15;i++,tz-=1024,B++,P++)
      {
      tx=(8*1024)-WORLDXOFF;
      for (j=0;j<15;j++,tx-=1024,B++,P++)
         {
         B->VIS1=B->VIS2=0;

         if ( (P->HT==0)&&((P+1)->HT==0)&&((P+17)->HT==0)&&(B->BITID==-1))
//         if ( (P->HT==0)&&((P+1)->HT==0)&&((P+17)->HT==0))
            {
            B->VIS1=0;
//            B->VIS1=1;
//            B->CLR1=0xe0;

            }
         else
            {
            if (DotProduct(-( (P+1)->HT-P->HT ),1024,((P+1)->HT-(P+17)->HT ), tx,-WORLDYOFF,tz) > 0 )
               {
               B->VIS1=1;
               t=abs(P->HT-(P+1)->HT)+abs((P+1)->HT-(P+17)->HT);
               t/=64;
               if (t>1) t=1;
               t=1-t;

               B->CLR1-= t;

               if (GRNDDETAIL==1)
                  {
                  B->XL=( (P)->HT ) >>6;
                  B->YL=( (P+1)->HT ) >>6;
                  B->XH=( (P+16)->HT ) >>6;
                  B->YH=( (P+17)->HT ) >>6;
                  }

               }
            }

         if ( (P->HT==0)&&((P+16)->HT==0)&&((P+17)->HT==0)&&(B->BITID==-1))
//         if ( (P->HT==0)&&((P+16)->HT==0)&&((P+17)->HT==0))
            {
            B->VIS2=0;
//            B->VIS2=1;
//            B->CLR2=0xe0;
            }
         else
            {
            if (DotProduct( ( (P+16)->HT-(P+17)->HT ),1024,-((P+16)->HT-P->HT ), tx,-WORLDYOFF,tz) > 0 )
               {
               B->VIS2=1;
               t=abs(P->HT-(P+16)->HT)+abs((P+16)->HT-(P+17)->HT);
               t/=64;
               if (t>2) t=2;
               t=2-t;

               B->CLR2-= t;

               if (GRNDDETAIL==1)
                  {
                  B->XL=( (P)->HT )>>6;
                  B->YL=( (P+1)->HT )>>6;
                  B->XH=( (P+16)->HT )>>6;
                  B->YH=( (P+17)->HT )>>6;
                  }

               }
            }

         }
      }

}

struct SBox far *BXBS;
struct GRDPT far *PTBS;


int   MAXLEV=5;

DoBox(struct SBox far *B,struct GRDPT far *PL1,struct GRDPT far *PL2,
         struct GRDPT far *PL3,struct GRDPT far *PL4,
         int X, int Z, int LEV, int XI, int YI)
{
   int   SZ,SOFF,SZ2;
   struct SBox far *TBXBS;
   struct GRDPT far *TPTBS;

   char far *PTR;

   if ( (B->VIS1|B->VIS2) ==0) return;

   if ( PL1->PT.FLAGS& PL2->PT.FLAGS& PL3->PT.FLAGS& PL4->PT.FLAGS&ROUTS  )
      return;

   SZ= (1024>>LEV);
   SZ2= (1024>>LEV);
//   if (LEV<3)
//      SZ2= (2048>>LEV);

   if ( (LEV>=MAXLEV)|| ( ((X+SZ)<(-SZ2)|| (X>SZ2)|| (Z+SZ)<(-SZ2)|| (Z>(SZ2)))&&

          !( (B->BITID==32)&&(LEV<3)&&(abs(X)<(4*SZ))&&(abs(Z)<(4*SZ)) ) )
//   if ( (LEV>=MAXLEV)|| ( ((X+SZ)<(-SZ)|| (X>SZ)|| (Z+SZ)<(-SZ)|| (Z>(SZ)))&&
//
//          !((B->BITID==32)&&(LEV<3))  )
                                                              )
      {
      DrawSBox( B,&PL1->PT,&PL2->PT,&PL3->PT,&PL4->PT,ScreenAddr);
      }
   else
      {
      TBXBS=BXBS;
      TPTBS=PTBS;
      BXBS++;
      PTBS+=5;

      MakeMidGridPt(PL1,PL2,TPTBS,LEV+1);
      MakeMidGridPt(PL1,PL3,TPTBS+1,LEV+1);
      MakeMidGridPt(PL2,PL4,TPTBS+3,LEV+1);
      MakeMidGridPt(PL3,PL4,TPTBS+4,LEV+1);
      MakeMidGridPt(PL1,PL4,TPTBS+2,0);

      SOFF= 16*YI+2*XI+64*B->BITID;



      FP_OFF(PTR)=SOFF;
      FP_SEG(PTR)=MapInBank(SDEFBANK);


      if (B->VIS1&0x80) B->VIS2|=0x80;

      (TBXBS)->TYPE1=B->TYPE1;
      (TBXBS)->TYPE2=B->TYPE2;
      (TBXBS)->VIS1=B->VIS1;
      (TBXBS)->VIS2=B->VIS2;
      (TBXBS)->CLR1= B->CLR1;
      (TBXBS)->CLR2= B->CLR2;


      (TBXBS)->BITID= *(PTR);
//      (TBXBS)->BITID= (PTR+(B->BITID&127))->MAP[16*YI+2*XI];
//      (TBXBS)->BITID= SSTAMPS[B->BITID].MAP[8*YI+XI];
      if ( (LEV==2)&&((TBXBS)->BITID!=-1)&&(GRNDDETAIL==2) )
         {
         (TBXBS)->BITID+=128;
         TBXBS->XH= ((TBXBS->XL= ((TBXBS->BITID&3)<<6))+63)  ;
         TBXBS->YH= ((TBXBS->YL= (((TBXBS->BITID/4)&3)<<6))+63)  ;
         }
      else
         {
         (TBXBS)->BITID=B->BITID;
         (TBXBS)->XL=   B->XL;
         (TBXBS)->YL=   B->YL;
         (TBXBS)->XH=  (B->XL+B->XH +1)>>1;
         (TBXBS)->YH=  (B->YL+B->YH +1)>>1;
         }
      DoBox(TBXBS,PL1,TPTBS,TPTBS+1,TPTBS+2, X,Z,LEV+1, 2*XI,2*YI);


      FP_OFF(PTR)=SOFF;
      FP_SEG(PTR)=MapInBank(SDEFBANK);

      if ( B->VIS1 )
         {
         (TBXBS)->VIS1=(TBXBS)->VIS2= (B->VIS1|0x80);
         (TBXBS)->TYPE1=(TBXBS)->TYPE2=B->TYPE1;
         (TBXBS)->CLR1=(TBXBS)->CLR2=B->CLR1;


         (TBXBS)->BITID= *(PTR+1);

//         (TBXBS)->BITID= (PTR+(B->BITID&127))->MAP[16*YI+2*XI+1];
//         (TBXBS)->BITID= SSTAMPS[B->BITID].MAP[8*YI+XI+1];
         if ( (LEV==2)&&((TBXBS)->BITID!=-1)&&(GRNDDETAIL==2) )
            {
            (TBXBS)->BITID+=128;
            TBXBS->XH= ((TBXBS->XL= ((TBXBS->BITID&3)<<6))+63)  ;
            TBXBS->YH= ((TBXBS->YL= (((TBXBS->BITID/4)&3)<<6))+63)  ;
            }
         else
            {
            (TBXBS)->BITID=B->BITID;
            (TBXBS)->XL=   (B->XL+B->XH +1)>>1;
            (TBXBS)->YL=   B->YL;
            (TBXBS)->XH=   B->XH;
            (TBXBS)->YH=  (B->YL+B->YH +1)>>1;
            }
         DoBox(TBXBS,TPTBS,PL2,TPTBS+2,TPTBS+3, X+(SZ/2),Z,LEV+1, 2*XI+1,2*YI);
         }

      FP_OFF(PTR)=SOFF;
      FP_SEG(PTR)=MapInBank(SDEFBANK);

      if ( B->VIS2 )
         {
         (TBXBS)->VIS1=(TBXBS)->VIS2= (B->VIS2|0x80);
         (TBXBS)->TYPE1=(TBXBS)->TYPE2=B->TYPE2;

      (TBXBS)->BITID= *(PTR+8);

         (TBXBS)->CLR1=(TBXBS)->CLR2=B->CLR2;
//      (TBXBS)->BITID= (PTR+(B->BITID&127))->MAP[8*(2*YI+1)+2*XI];
//         (TBXBS)->BITID= SSTAMPS[B->BITID].MAP[8*(YI+1)+XI];
         if ( (LEV==2)&&((TBXBS)->BITID!=-1)&&(GRNDDETAIL==2) )
            {
            (TBXBS)->BITID+=128;
            TBXBS->XH= ((TBXBS->XL= ((TBXBS->BITID&3)<<6))+63)  ;
            TBXBS->YH= ((TBXBS->YL= (((TBXBS->BITID/4)&3)<<6))+63)  ;
            }
         else
            {
            (TBXBS)->BITID=B->BITID;
            (TBXBS)->XL=   B->XL;
            (TBXBS)->YL=  (B->YL+B->YH +1)>>1;
            (TBXBS)->XH=  (B->XL+B->XH +1)>>1;
            (TBXBS)->YH=   B->YH;
            }
         DoBox(TBXBS,TPTBS+1,TPTBS+2,PL3,TPTBS+4, X,Z+(SZ/2),LEV+1, 2*XI,2*YI+1);
         }



      FP_OFF(PTR)=SOFF;
      FP_SEG(PTR)=MapInBank(SDEFBANK);

      (TBXBS)->TYPE1=B->TYPE1;
      (TBXBS)->TYPE2=B->TYPE2;
      (TBXBS)->VIS1=B->VIS1;
      (TBXBS)->VIS2=B->VIS2;
      (TBXBS)->CLR1= B->CLR1;
      (TBXBS)->CLR2= B->CLR2;

      (TBXBS)->BITID= *(PTR+8+1);

//      (TBXBS)->BITID= (PTR+(B->BITID&127))->MAP[8*(2*YI+1)+2*XI+1];
//      (TBXBS)->BITID= SSTAMPS[B->BITID].MAP[8*(YI+1)+XI+1];
      if ( (LEV==2)&&((TBXBS)->BITID!=-1)&&(GRNDDETAIL==2) )
         {
         (TBXBS)->BITID+=128;
         TBXBS->XH= ((TBXBS->XL= ((TBXBS->BITID&3)<<6))+63)  ;
         TBXBS->YH= ((TBXBS->YL= (((TBXBS->BITID/4)&3)<<6))+63)  ;
         }
      else
         {
         (TBXBS)->BITID=B->BITID;
         (TBXBS)->XL=  (B->XL+B->XH +1)>>1;
         (TBXBS)->YL=  (B->YL+B->YH +1)>>1;
         (TBXBS)->XH=   B->XH;
         (TBXBS)->YH=   B->YH;
         }
      DoBox(TBXBS,TPTBS+2,TPTBS+3,TPTBS+4,PL4, X+(SZ/2),Z+(SZ/2),LEV+1, 2*XI+1,2*YI+1);

      BXBS--;
      PTBS-=5;
      }
}

GetQHeight(long X, long Z )
{

// the normals are (H1-H2)i+1024j+(H2-H4)k  poly1
//             and (H3-H4)i+1024j+(H1-H3)k  poly2

   int   XB,ZB,XOFF,ZOFF;

   int H1,H2,H3,H4;
   long  L;
   unsigned char far *PTR;

   XB=(X/4096)&0xff;
   ZB=(Z/4096)&0xff;
   XOFF=X&4095;
   ZOFF=Z&4095;

   FP_OFF(PTR)=0;
   FP_SEG(PTR)=MapInBank(HMAPBANK);


   H1=(*(PTR+256*ZB+XB)&0xff);
   H2=(*(PTR+256*ZB+XB+1)&0xff);
   H3=(*(PTR+256*(ZB+1)+XB)&0xff);
   H4=(*(PTR+256*(ZB+1)+XB+1)&0xff);

	H1 <<=4;
	H2 <<=4;
	H3 <<=4;
	H4 <<=4;

	MikeHeight = H1;

   if (XOFF>ZOFF)
      {
      L= -MulDiv32((long)(H1-H2),(long)(XOFF),4096L) -
         MulDiv32((long)(H2-H4),(long)(ZOFF),4096L);
      }
   else
      {
      L= -MulDiv32((long)(H3-H4),(long)(XOFF),4096L) -
         MulDiv32((long)(H1-H3),(long)(ZOFF),4096L);
      }

     return((int)(8*(H1+L)));
}

IsLand(long X, long Z )
{

// the normals are (H1-H2)i+1024j+(H2-H4)k  poly1
//             and (H3-H4)i+1024j+(H1-H3)k  poly2

   int   XB,ZB,XOFF,ZOFF;

   int H1,H2,H3,H4;
   long  L;
   char far *PTR;

   XB=(X/4096)&0xff;
   ZB=(Z/4096)&0xff;
   XOFF=X&4095;
   ZOFF=Z&4095;

   FP_OFF(PTR)=0;
   FP_SEG(PTR)=MapInBank(BMAPBANK);


   H1=(*(PTR+256*ZB+XB));
   return(H1);
}




long  WORLDXPOS;
long  WORLDZPOS;

DrawLandObjects( int STMP, int X, int Y, int Z )
{

   int   i,xt,yt,zt,scl;
   long  XL,ZL;


   for (i=0;(i<16)&&(OBJECTS[STMP][i].NUM!=-1);i++)
      {
      scl=GroundObjectScale[OBJECTS[STMP][i].NUM];
      xt=((OBJECTS[STMP][i].X)<<5)+X;
      zt=(OBJECTS[STMP][i].Y<<5)+Z;
      XL= (long)xt+WORLDXPOS;
      ZL= (long)zt+WORLDZPOS;

      yt=GetQHeight(XL,ZL );

	  yt/=4;

      if (scl<0)
         {
         NDraw3DObject( OBJECTS[STMP][i].NUM+LandObjectBase,(xt/-scl),((Y+yt)/-scl),(zt/-scl), IdentInvMat, 0,0);
         }
      else
         {
         NDraw3DObject( OBJECTS[STMP][i].NUM+LandObjectBase,(xt*scl),((Y+yt)*scl),(zt*scl), IdentInvMat, 0,0);
         }
      }
}


DrawNLand( long X, long Y, long Z, char far *DST,char far *SRC )
{

	int	i,j;
   struct	PT3D far *P1;
	struct	PT3D far *List[4];

	unsigned SEG;
   int   HT;

   int   t;
   int   tx,tz;

   struct SBox far *BG;
   struct GRDPT far *GPT;

   char far *TMP;


   switch (GRNDDETAIL)
      {
      case 1:
         MAXLEV=3;
         break;
      case 2:
         MAXLEV=5;
         break;
      case 0:
      default:
         MAXLEV=0;
         break;
      }




   WORLDXPOS=X;
   WORLDZPOS=Z;


   ScreenAddr=DST;
   BitMaps[0]=SRC;
   BitMaps[1]=SRC;
   BitMaps[2]=SRC;
   BitMaps[3]=SRC;


   WORLDXTRAX= -(X&3);
   WORLDXTRAY= 0;
   WORLDXTRAZ= -(Z&3);
   X>>=2;
   Y>>=2;
   Z>>=2;

	WORLDXOFF= -(X&1023);
	WORLDYOFF= -Y;
	WORLDZOFF= -(Z&1023);
	XBASE= (X/1024L);
	ZBASE= (Z/1024L);

   FP_OFF(TMP)=0;
   FP_SEG(TMP)=GRPAGE0[2];

   MakePTS(TMP);
   MakeBoxes(TMP+32768);
   MakeBoxVis(TMP+32768,TMP);

   BG= BXBS= TMP+32768;
   GPT= PTBS= TMP;

   BXBS+=256;
   PTBS+=256;


	for (i=0,tz=(-8*1024)-(Z&1023);i<8;i++,tz+=1024)
		{
		for (j=0,tx=(-8*1024)-(X&1023);j<8;j++,tx+=1024)
			{
         DoBox( BG+16*i+j,GPT+16*i+j,GPT+16*i+j+1,
                           GPT+16*(i+1)+j,GPT+16*(i+1)+j+1,
                  tx,tz,0, 0,0);
         if ( (i>5)&&(j>5)&&((BG+16*i+j)->BITID>0) )
            DrawLandObjects((BG+16*i+j)->BITID,(tx<<2),(WORLDYOFF<<2),(tz<<2));

			}
		}
	for (i=0,tz=(-8*1024)-(Z&1023);i<8;i++,tz+=1024)
		{
		for (j=14,tx=6*1024-(X&1023);j>=8;j--,tx-=1024)
			{
         DoBox( BG+16*i+j,GPT+16*i+j,GPT+16*i+j+1,
                           GPT+16*(i+1)+j,GPT+16*(i+1)+j+1,
                  tx,tz,0, 0,0);
         if ( (i>5)&&(j<10)&&((BG+16*i+j)->BITID>0) )
            DrawLandObjects((BG+16*i+j)->BITID,(tx<<2),(WORLDYOFF<<2),(tz<<2));
			}
		}
	for (i=14,tz=6*1024-(Z&1023);i>=8;i--,tz-=1024)
		{
		for (j=0,tx=(-8*1024)-(X&1023);j<8;j++,tx+=1024)
			{
         DoBox( BG+16*i+j,GPT+16*i+j,GPT+16*i+j+1,
                           GPT+16*(i+1)+j,GPT+16*(i+1)+j+1,
                  tx,tz,0, 0,0);
         if ( (i<10)&&(j>5)&&((BG+16*i+j)->BITID>0) )
            DrawLandObjects((BG+16*i+j)->BITID,(tx<<2),(WORLDYOFF<<2),(tz<<2));
			}
		}
	for (i=14,tz=6*1024-(Z&1023);i>=8;i--,tz-=1024)
		{
		for (j=14,tx=6*1024-(X&1023);j>=8;j--,tx-=1024)
			{
         DoBox( BG+16*i+j,GPT+16*i+j,GPT+16*i+j+1,
                           GPT+16*(i+1)+j,GPT+16*(i+1)+j+1,
                  tx,tz,0, 0,0);
         if ( (i<10)&&(j<10)&&((BG+16*i+j)->BITID>0) )
            DrawLandObjects((BG+16*i+j)->BITID,(tx<<2),(WORLDYOFF<<2),(tz<<2));
			}
		}

}
