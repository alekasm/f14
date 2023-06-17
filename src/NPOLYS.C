#include	"stdio.h"
#include	"stdlib.h"
#include <dos.h>
    union REGS D;


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

//extern	PPoly(struct PPNT *PL, int CNT, char far *D, int TYPE, char far *S );
//extern	PFLine3D(struct PPNT *P1,struct PPNT *P2,int TYPE,int *M1, int *VOFF, int *VM);
//extern	PPoly3D(struct PPNT *PL, int CNT, int *M, int *VOFF,int *VM, char far *D, int TYPE, char far *S );

extern	long MulDiv32( long V1, long V2, long V3 );
extern	NPFLine( void far *P1, void far *P2, int TYPE);
extern   long  DotProduct( int p1, int P2, int P3, int P4, int P5, int P6);

extern   FlagRotate3D(struct PT3D far *P);
extern   Perspect3D(struct	PT3D far *P);
extern   ZClip( struct PT3D far *PIN,struct PT3D far *POUT,struct PT3D far *PDST,int TYPE);
Npoly3D(int Num, struct	PT3D far *List[], char far *D, int TYPE, char far *S );


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


long	CLIPZ=0x20000L;




LPoly(struct CPT32 far *PL, int CNT, char far *D, int TYPE, char far *S )
{
	int	i;


   for (i=0;i<(CNT-1);i++)
      {
		NPFLine( PL+i, PL+i+1, TYPE);
      }
	NPFLine( PL+CNT-1, PL, TYPE);

	NPFFill( D, TYPE, S);

}


#if 0
/*

FlagRotate3D(struct PT3D far *P)
{
	long	ZVAL;

	ZVAL= P->ZR;

	P->FLAGS=0;

	if (P->XR > ZVAL)	P->FLAGS |= RXHFLAG;
	if ( -P->XR > ZVAL) P->FLAGS |= RXLFLAG;

	if (P->YR > ZVAL)	P->FLAGS |= RYHFLAG;
	if ( -P->YR > ZVAL) P->FLAGS |= RYLFLAG;

//	P->ZR /=256;
	if (P->ZR < CLIPZ)	P->FLAGS |= RZLFLAG;
// for now only clip zlo
// ignore the rdflag for now
}
*/

/*
Perspect3D(struct	PT3D far *P)
{
   long  DZ;

	P->FLAGS&=0x00ff;

   DZ= P->ZR;

	if (DZ>=CLIPZ)
		{
      DZ>>=8;
		P->XP= LongDiv(P->XR,DZ);
		P->XP+= GXMID;
		if (P->XP<GXLO32) P->FLAGS |= PXLFLAG;
		if (P->XP>GXHI32) P->FLAGS |= PXHFLAG;
		P->YP= LongDiv(P->YR,DZ);
		P->YP+= GYMID;
		if (P->YP<GYLO32) P->FLAGS |= PYLFLAG;
		if (P->YP>GYHI32) P->FLAGS |= PYHFLAG;
		P->FLAGS |= PDFLAG;
		}
}
*/

/*
ZClip( struct PT3D far *PIN,struct PT3D far *POUT,struct PT3D far *PDST,int TYPE)
{
	long	DEL;
	long	DELZ;
	long	DZ;


// clip Pin
	DELZ= POUT->ZR - PIN->ZR;
	DZ= CLIPZ - PIN->ZR;

	DEL= POUT->XR - PIN->XR;
	PDST->XR= PIN->XR+ MulDiv32(DEL,DZ,DELZ);
	DEL= POUT->YR - PIN->YR;
	PDST->YR= PIN->YR+ MulDiv32(DEL,DZ,DELZ);
	PDST->ZR=CLIPZ;
	switch ((TYPE&3))
		{
		case 2:
			DEL= POUT->YT - PIN->YT;
			PDST->YT= PIN->YT+ MulDiv32(DEL,DZ,DELZ);
			// fall thru
		case 1:
			DEL= POUT->XT - PIN->XT;
			PDST->XT= PIN->XT+MulDiv32(DEL,DZ,DELZ);
			break;
		}
	FlagRotate3D(PDST);
}
*/
#endif


Npoly3D(int Num, struct	PT3D far *List[], char far *D, int TYPE, char far *S )
{
	int		i,Npts,Xpts;
	struct	PT3D	XTRAPTS[4];
	struct	PT3D far *P;
	struct	PT3D far *PNXT;
	int		OFlags,NFlags;
	struct	PT3D far *NList[32];
   int   T;


   T=BuildPolyList(Num, List,NList, D, S, TYPE);
   if (T<1) return;

/*
// check rotate flags for all out
	for (i=0,OFlags=-1;i<Num;i++)
		{
		OFlags&= List[i]->FLAGS;
		}
	if (OFlags&ROUTS)	return;

//	if (OFlags&PDFLAG)
// if all pts perspected no clipping is needed
// just copy the list, add the first to end and skip clipping alltogether

*/

// build new list of ptrs here
	   for (i=Npts=Xpts=0,P=List[0],NFlags=-1;i<Num;i++,P=PNXT)
		   {
		   PNXT=List[i+1];
		   if ((i+1)>=Num) PNXT= List[0];			// wrap to first point

		   if (P->FLAGS&RZLFLAG)						//clip line to zlo
			   {
			   if (PNXT->FLAGS&RZLFLAG) continue;	// both out go to next line seg
			   ZClip(PNXT,P,&XTRAPTS[Xpts],TYPE);	//always clip from good to bad
			   NList[Npts++]=&XTRAPTS[Xpts];			// add xtra point to list
			   NFlags&=XTRAPTS[Xpts++].FLAGS;		// update new flags- test later
			   continue;
			   }
		   if (PNXT->FLAGS&RZLFLAG)
			   {
			   NList[Npts++]=P;							// add good pt
			   NFlags&=P->FLAGS;								// update new flags
			   ZClip(P,PNXT,&XTRAPTS[Xpts],TYPE);
			   NList[Npts++]=&XTRAPTS[Xpts];			//add new good"bad" pt
			   NFlags&=XTRAPTS[Xpts++].FLAGS;
			   continue;
			   }
		   NList[Npts++]=P;								// line was ok add first point
		   NFlags&=P->FLAGS;								// update new flags
		   }
	   NList[Npts++]=NList[0];							// add the first point to end

	   if (NFlags&ROUTS)	return;						// all out in some rotate plane
// check for perspected- perspect if not already
	   for (i=0,OFlags=-1;i<(Npts-1);i++)
		   {
		   P=NList[i];
		   if ( !(P->FLAGS&PDFLAG) )
			   Perspect3D(P);
		   OFlags&=P->FLAGS;
		   }


   if (OFlags&POUTS)		return;					// all off to one side of screen

// draw the poly lines
   Npts--;
	for (i=0,P=NList[0];i<Npts;i++)
		{
      PNXT=NList[i+1];
		NPFLine( &(P->XP),&(PNXT->XP), TYPE);
      P=PNXT;
		}
	NPFFill( D, TYPE, S);

}

Nline3D(struct	PT3D far *P1, struct	PT3D far *P2, char far *D, int CLR)
{
	struct	PT3D	XTRAPT;
   int OFlags;

   OFlags=(P1->FLAGS&P2->FLAGS);

// check rotate flags for all out
	if (OFlags&ROUTS)	return;
	if (OFlags&RZLFLAG)	return;

//	if (OFlags&PDFLAG)
// if all pts perspected no clipping is needed
// just copy the list, add the first to end and skip clipping alltogether

// build new list of ptrs here


   if (P1->FLAGS&RZLFLAG)						//clip line to zlo
		{
		ZClip(P2,P1,&XTRAPT,0);	//always clip from good to bad
      P1=&XTRAPT;
		}
   if (P2->FLAGS&RZLFLAG)						//clip line to zlo
		{
		ZClip(P1,P2,&XTRAPT,0);	//always clip from good to bad
      P2=&XTRAPT;
		}


   if (P1->FLAGS&P2->FLAGS&ROUTS) return;


	if ( !(P1->FLAGS&PDFLAG) )
		Perspect3D(P1);
	if ( !(P2->FLAGS&PDFLAG) )
		Perspect3D(P2);

   if (P1->FLAGS&P2->FLAGS&POUTS)		return;


   NDLine(&(P1->XP),&(P2->XP),D,CLR);

}
NDot3D(struct	PT3D far *P1, char far *D, int CLR)
{

// check rotate flags for all out
	if (P1->FLAGS&(ROUTS|RZLFLAG))	return;

	if ( !(P1->FLAGS&PDFLAG) )
		Perspect3D(P1);

   if (P1->FLAGS&POUTS)		return;


   NDDot(&(P1->XP),D,CLR);

}

