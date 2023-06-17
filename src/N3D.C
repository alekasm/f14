
#include	"stdio.h"
#include	"stdlib.h"
#include <dos.h>

    union REGS D;


struct   intvec{
   int   X;
   int   Y;
   int   Z;
   };
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
extern   long  DotProduct(int V1, int V2, int V3, int V4, int V5, int V6);
extern	NPFLine( void far *P1, void far *P2, int TYPE);

extern   FlagRotate3D(struct PT3D far *P);
extern   Perspect3D(struct	PT3D far *P);
extern   ZClip( struct PT3D far *PIN,struct PT3D far *POUT,struct PT3D far *PDST,int TYPE);
extern   Npoly3D(int Num, struct	PT3D far *List[], char far *D, int TYPE, char far *S );

extern   LPoly(struct CPT32 far *PL, int CNT, char far *D, int TYPE, char far *S );

extern   struct	PT3D far *CheckPointRot(unsigned int Pt );

extern   int   NOTEXTURE;

#define	GOUROUD 1
#define	TEXTURE 2

struct   srfdef{
   int   ni;
   int   nj;
   int   nk;
   long  D;
   int   matnum;
   };
struct   ptdef{
   int   x;
   int   y;
   int   z;
   int   mat;
   };
struct   matdef{
   char  hpr;
   char  var;
   int   xr;
   int   yr;
   int   zr;
   };


struct   obj3d {
   int   NUM;
   int   XR;
   int   YR;
   int   ZR;
   int   *MAT;
   int   *ANIM;
   long  RELADD[3];
   };



#define  GPDONE      -1
#define  GPDOT       0
#define  GPLINE      1
#define  GPOBJ       2
#define  GPPLY       3
#define  GPGRDPLY    4
#define  GPTXTRPLY   5

#define  GPANIM   0x80


int   IdentInvMat[18]={ 0x7fff,0,0, 0,0x7fff,0, 0,0,0x7fff,
                        0x7fff,0,0, 0,0x7fff,0, 0,0,0x7fff };

int   Scale3d=1;

extern   int   Nviewmat[];

extern int   *PalPtr3D;

struct objptrs{
   void far *OBJBASE;
   unsigned int far *GOPBASE;
   struct   ptdef far *PTSDEFBASE;
   struct	PT3D far *PTSROT;
   char far *TXTR;
   struct   obj3d *OBJDEF;
   long  RELADD[5][3];
   int   *MATPTR;
   int  *ANIM;
   struct intvec *OFFS;
   unsigned char ORDER[256];
   int   dumppts[5][3];
   int   MATS[5][18];
   };


// gop defs

struct   anm{
   unsigned char  VAR;
   unsigned char  VALUE;
   };

struct	PT3D far POINTS[380];
struct	PT3D far *POINTBUF=POINTS;


struct objptrs *OBJ;


struct bspnode{
   unsigned char F;
   unsigned char B;
   };

struct bspnode far *BSPBASE;
unsigned char *FBPTR;
unsigned char *DRAWPTR;

struct emsobjhndl{
   int   objhndl;
   int   txtrhndl;
   };
struct emsobjhndl far objhndls[256];



char far *NPDEST;


DrawN3dObject( struct   obj3d *OBJPTR,int *VMAT,struct intvec *OFFS );

char far *MapEMSObject( int Num)
{
   char far *S;

   if ( Num<0 ) return(0);

   if (objhndls[Num].objhndl==0) return(0);
   if ( (FP_SEG(S)=MapEmsFile( objhndls[Num].objhndl, 0))==0 ) return(0);
   FP_OFF(S)=0;
   return(S);
}

char far *MapEMSObjectTexture( int Num)
{
   char far *S;

   if (objhndls[Num].txtrhndl==0) return(0);
   if ( (FP_SEG(S)=MapEmsFile( objhndls[Num].txtrhndl, 1))==0 ) return(0);
   FP_OFF(S)=0;
   return(S);
}


#if 0
MakeBspDrawList( int node )
{

// if eye infront of node poly then
//	follow tree back ptr
//	display poly (node)
//	follow tree front ptr
// else
//	follow tree front ptr
//	follow tree back ptr

   if (node==255)
      {
      *(DRAWPTR)=255;
      return;
      }
   if ((*(FBPTR+node))&1)
      {
      MakeBspDrawList( ((BSPBASE+node)->B)&0xff );
      *(DRAWPTR++)=node;
      MakeBspDrawList( ((BSPBASE+node)->F)&0xff );
      }
   else
      {
      MakeBspDrawList( ((BSPBASE+node)->F)&0xff );
      if ( (*(FBPTR+node)&0x80) )
      *(DRAWPTR++)=node;
      MakeBspDrawList( ((BSPBASE+node)->B)&0xff );
      }
}
#endif

SetMatsPtrsOrder(int *VMAT)
{
   int   i,j,matnum,srfnum,ptsnum;
   int   far *P;
   char far *P1;
   struct   matdef far *PM;
   struct   srfdef far *SRF;
   unsigned char fblist[256];
   unsigned char root;
   int   HD,PCH,RLL,VAL;
   int   M[9];
   int   V[3];
   struct	PT3D far *PT;

// set up the ptrs

   P= (int far *)(OBJ->OBJBASE);

   P++;                       // skip past radius
   P+= 7;                     // skip past texture name 14 chars
   matnum=*(P++);             // get # of mats
   ptsnum=*(P++);             // get # of points
   srfnum=*(P++);             // get # of srfs


   OBJ->dumppts[0][0]=- (OBJ->OFFS)->X;
   OBJ->dumppts[0][1]=- (OBJ->OFFS)->Y;
   OBJ->dumppts[0][2]=- (OBJ->OFFS)->Z;

   if (matnum)
      {
      OBJ->MATPTR= OBJ->MATS;
      Mult3X3((OBJ->OBJDEF)->MAT,VMAT,OBJ->MATS);
      Trans3X3((OBJ->OBJDEF)->MAT,&OBJ->MATS[0][9]);
      Rotate(OBJ->dumppts[0],&OBJ->MATS[0][9]);

      PM=(struct matdef far *)(P);

      for (i=1;i<matnum;i++)
         {
// do articulations here, add the mat sizes to P
         HD=PCH=RLL=0;
         VAL= *(OBJ->ANIM+PM->var);
         if (PM->hpr==0) HD=VAL;
         if (PM->hpr==1) PCH=VAL;
         if (PM->hpr==2) RLL=VAL;
         MakematHPR(HD,PCH,RLL,M);
         Trans3X3(M,&OBJ->MATS[i][9]);

//         Copy3X3(IdentInvMat,M);       // take out


         Mult3X3(M,OBJ->MATS, OBJ->MATS[i]);

         V[0]= PM->xr;
         V[1]= PM->yr;
         V[2]= PM->zr;

         Mult1X3(V,OBJ->MATS,OBJ->RELADD[i]);
         OBJ->RELADD[i][0]+= OBJ->RELADD[0][0];
         OBJ->RELADD[i][1]+= OBJ->RELADD[0][1];
         OBJ->RELADD[i][2]+= OBJ->RELADD[0][2];

         OBJ->dumppts[i][0]= (OBJ->dumppts[0][0]- PM->xr);
         OBJ->dumppts[i][1]= (OBJ->dumppts[0][1]- PM->yr);
         OBJ->dumppts[i][2]= (OBJ->dumppts[0][2]- PM->zr);
         Rotate(OBJ->dumppts[i],&OBJ->MATS[i][9]);
         PM++;
         }
      P= (int far *)(PM);
      }
   else
      {
      OBJ->MATPTR= Nviewmat;      //IdentInvMat;
      // maybe set up the reladd here
      }

   SRF=(struct srfdef far *)(P);
   for (i=0;i<srfnum;i++)
      {
      if (Scale3d!=1)
         {
         if (DotProduct((OBJ->dumppts[(SRF->matnum&0x7fff)][0]),
                        (OBJ->dumppts[(SRF->matnum&0x7fff)][1]),
                        (OBJ->dumppts[(SRF->matnum&0x7fff)][2]),
                        SRF->ni,
                        SRF->nj,
                        SRF->nk ) >= -((SRF->D)>>Scale3d) )
                        {
                        fblist[i]=1;
                        }
                     else
                        {
                        fblist[i]=0;
                        }
         }
      else
         {

         if (DotProduct(OBJ->dumppts[(SRF->matnum&0x7fff)][0],
                        OBJ->dumppts[(SRF->matnum&0x7fff)][1],
                        OBJ->dumppts[(SRF->matnum&0x7fff)][2],
                        SRF->ni,
                        SRF->nj,
                        SRF->nk ) >= -(SRF->D) )
                        {
                        fblist[i]=1;
                        }
                     else
                        {
                        fblist[i]=0;
                        }
         }

      if ( (SRF->matnum)&0x8000 )
         fblist[i]|=0x80;
      SRF++;
      }
   P=(int far *)(SRF);
   root= (*(P++))&0xff;
   BSPBASE= (struct bspnode far *)(P);
   FBPTR= fblist;
   DRAWPTR= OBJ->ORDER;
   MakeBspDrawList( root );
   P+= srfnum;

// set up pts base here
   OBJ->PTSDEFBASE=(struct ptdef far *)(P);
   OBJ->PTSROT= POINTBUF;

// clear the rotated,persp flags
   for (i=0,PT=POINTBUF;i<ptsnum;i++,PT++)
      PT->FLAGS=0;

   POINTBUF+=ptsnum;                // set enough room here

// then set up gop base here
   P1= (char far *)(P);
   P1+= ptsnum*sizeof(struct ptdef);

   OBJ->GOPBASE=(unsigned int far *)(P1);

}

Reject(int *VM)
{
   long  ZVAL;
   unsigned long v;

   Mult1X3( &(OBJ->OFFS)->X,VM, (OBJ->RELADD[0]) );
//   Mult1X3( &(OBJ->OBJDEF)->XR,VM, (OBJ->RELADD[0]) );

   OBJ->RELADD[0][0]+= (OBJ->OBJDEF)->RELADD[0];
   OBJ->RELADD[0][1]+= (OBJ->OBJDEF)->RELADD[1];
   ZVAL= (OBJ->RELADD[0][2]+= (OBJ->OBJDEF)->RELADD[2]);
   v=*( (unsigned int far *)(OBJ->OBJBASE) );
   v<<=15;
   ZVAL+= v;

//   return(0);

   if (ZVAL<0) return(1);
   if (ZVAL<labs(OBJ->RELADD[0][0])) return(1);
   if (ZVAL<labs(OBJ->RELADD[0][1])) return(1);
   return(0);
}


#if 0
struct	PT3D far *CheckPointRot(unsigned int Pt )
{
   struct	PT3D far *P;
   struct   ptdef far *PDEF;
   int   V[3];
   long  VL[3];
   int   m,trn;

   P=((OBJ->PTSROT)+(Pt&0xff));

   if (P->FLAGS&RDFLAG) return(P);


// rotate the point here put at P and flag it
   PDEF= ((OBJ->PTSDEFBASE)+(Pt&0xff));
//   choose the correct matrix here
   m=PDEF->mat;
   if (m&0xfff0)
      {
      trn=((m>>4)&0xff);
      V[0]=PDEF->x+(*(OBJ->ANIM+trn));
      V[1]=PDEF->y+(*(OBJ->ANIM+trn+1));
      V[2]=PDEF->z+(*(OBJ->ANIM+trn+2));
      m=(m&0xf);
      }
   else
      {
      V[0]=PDEF->x;
      V[1]=PDEF->y;
      V[2]=PDEF->z;
      }
   Mult1X3(V,OBJ->MATPTR+18*m,VL);
// add the reladd here
   P->XR=VL[0]+OBJ->RELADD[m][0];
   P->YR=VL[1]+OBJ->RELADD[m][1];
   P->ZR=VL[2]+OBJ->RELADD[m][2];

   FlagRotate3D(P);
   return(P);
}
#endif

int   LEV=0;
DrawASurf( char far *P )
{
   struct	PT3D far *PT;
   struct	PT3D far *PT2;
   struct	PT3D far *List[32];
   struct	PT3D far *(*L);

   struct objptrs *OLDOBJ;
   struct   obj3d *OBJDEFPTR;
   struct   ptdef far *PDEF;
   struct   intvec   TOFFS;


   long  VL[3];
   int   V[3];

   int   i,NUM,CLR;
   int anm,val;

   while ( *(P)!=GPDONE )
      {
      if (*(P)&GPANIM )
         {
         anm=(*(P++)&0x7f);
         val=*(P++)&0xff;
         if (  (*(OBJ->ANIM+anm)&0xff)!= val  )
            {
            switch (*(P++))
               {
               case GPDOT:
                  P+= 2;
                  continue;
               case GPLINE:
                  P+=3;
                  continue;
               case GPOBJ:
                  P+=2;
                  continue;
               case GPPLY:
                  P+= *(P)+2;
                  continue;
               case GPGRDPLY:
                  P+= *(P)*2 +1;
                  continue;
               case GPTXTRPLY:
                  P+= *(P)*3 +2;
                  // fix this for base color under texture !!!
                  continue;
               default:
                  return;
               }
            }
         }
      switch ( (*(P++)&0x7f) )
         {
         case GPDOT:
            PT=CheckPointRot(*(P++));
            CLR=*(PalPtr3D+ (*(P++)&0xff) );
            NDot3D(PT,NPDEST, CLR);
            break;
         case GPLINE:
            PT=CheckPointRot(*(P++));
            PT2=CheckPointRot(*(P++));
            CLR=*(PalPtr3D+ (*(P++)&0xff) );
            Nline3D(PT,PT2,NPDEST, CLR);
            break;
         case GPOBJ:
            if (LEV>0) return;
            OLDOBJ= OBJ;

            LEV++;

            OBJDEFPTR=OBJ->OBJDEF;
            OBJDEFPTR+= ( *(P++)  );

            PDEF= ((OBJ->PTSDEFBASE)+(*(P++)&0xff));

            V[0]=PDEF->x+OBJDEFPTR->XR;
            V[1]=PDEF->y+OBJDEFPTR->YR;
            V[2]=PDEF->z+OBJDEFPTR->ZR;

            TOFFS.X=V[0]-OBJ->dumppts[PDEF->mat][0];
            TOFFS.Y=V[1]-OBJ->dumppts[PDEF->mat][1];
            TOFFS.Z=V[2]-OBJ->dumppts[PDEF->mat][2];


            Mult1X3(&TOFFS.X,OBJ->MATPTR+18*(PDEF->mat),VL);

            Mult1X3(V,OBJ->MATPTR+18*(PDEF->mat),OBJDEFPTR->RELADD);

            OBJDEFPTR->RELADD[0]+= OBJ->RELADD[0][0];
            OBJDEFPTR->RELADD[1]+= OBJ->RELADD[0][1];
            OBJDEFPTR->RELADD[2]+= OBJ->RELADD[0][2];

            OBJDEFPTR->RELADD[0]+= -VL[0];
            OBJDEFPTR->RELADD[1]+= -VL[1];
            OBJDEFPTR->RELADD[2]+= -VL[2];


            DrawN3dObject( OBJDEFPTR,OBJ->MATPTR+18*(PDEF->mat),&TOFFS);

            OBJ=OLDOBJ;

			// remap in the object
            if ( (MapEMSObject(OBJ->OBJDEF->NUM))==0  ) return;
            MapEMSObjectTexture(OBJ->OBJDEF->NUM);
            LEV--;

            break;
         case GPPLY:
            NUM=*(P++);
            for (i=0,L=&List[0];i<NUM;i++)
               *(L++)=CheckPointRot(*(P++));
            CLR=*(PalPtr3D+ (*(P++)&0xff) );
			if ((CLR&0xff00) == 0xff00)
				CLR = ((CLR<<8)&0xff00)|(CLR&0xff);
            Npoly3D( NUM, &List[0], NPDEST, 0, CLR );
            break;
         case GPGRDPLY:
            NUM=*(P++);
            for (i=0,L=List;i<NUM;i++)
               {
               *(L++)=PT=CheckPointRot(*(P++));
               PT->XT=*(P++);
               }
            Npoly3D( NUM, &List[0], NPDEST, 1, 0 );
            break;
         case GPTXTRPLY:
            NUM=*(P++);
            for (i=0,L=List;i<NUM;i++)
               {
               *(L++)= PT=CheckPointRot(*(P++));
               PT->XT=(*(P++)&0xff);
               PT->YT=(*(P++)&0xff);
               }
            if ((OBJ->TXTR==0)||NOTEXTURE)
               {
               // P++;  to get to base color
               CLR=*(PalPtr3D+ (*(P++)&0xff) );
               if ((CLR&0xff00) == 0xff00)
                  CLR = ((CLR<<8)&0xff00)|(CLR&0xff);
               Npoly3D( NUM, &List[0], NPDEST, 0, CLR );
               }
            else
               {
               Npoly3D( NUM, &List[0], NPDEST, (*(P++)<<8)+2, OBJ->TXTR);
               // P++; for base color
               }
            break;
         default:
            return;
         }
      }
}

DrawSurfs()
{
   char far *P;
   unsigned char *L;

   L=OBJ->ORDER;
   P=(char far *)OBJ->OBJBASE;
   while (*(L)!=255)
      {
      FP_OFF(P)=*((OBJ->GOPBASE)+*(L));
      DrawASurf( P );
      L++;
      }
}



DrawN3dObject( struct   obj3d *OBJPTR,int *VMAT, struct intvec *OFFS )
{
   struct objptrs object;
   char far *S;

   OBJ= &object;
// set the object base , bank in the object


   OBJ->OBJDEF= OBJPTR;

   if ( (OBJ->OBJBASE=MapEMSObject(OBJPTR->NUM))==0  ) return;


   OBJ->OFFS=OFFS;
   OFFS->X+= OBJPTR->XR;
   OFFS->Y+= OBJPTR->YR;
   OFFS->Z+= OBJPTR->ZR;


   if ( Reject(VMAT) ) return;


   OBJ->TXTR=MapEMSObjectTexture(OBJPTR->NUM);

   OBJ->ANIM= OBJPTR->ANIM;

	SetMatsPtrsOrder(VMAT);
	DrawSurfs();

   POINTBUF=OBJ->PTSROT;

}

struct   obj3d OB[3];

NDraw3DObject( int N,int XOFF, int YOFF, int ZOFF, int *MAT, int *ANIM, int BGS )
{
   struct   intvec   OFFS;

   OB[0].NUM=  N;
   OB[0].XR=   XOFF;
   OB[0].YR=   YOFF;
   OB[0].ZR=   ZOFF;
   OB[0].MAT=  MAT;
   OB[0].ANIM= ANIM;
   OB[0].RELADD[0]= OB[0].RELADD[1]= OB[0].RELADD[2]= 0;
   OFFS.X=0;
   OFFS.Y=0;
   OFFS.Z=0;
   DrawN3dObject( OB, Nviewmat,&OFFS);

}

DrawSuper3DObject(struct   obj3d OB[],int N,int XOFF, int YOFF, int ZOFF, int *MAT, int *ANIM)
{
   struct   intvec   OFFS;

   OB[0].NUM=  N;
   OB[0].XR=   XOFF;
   OB[0].YR=   YOFF;
   OB[0].ZR=   ZOFF;
   OB[0].MAT=  MAT;
   OB[0].ANIM= ANIM;
   OB[0].RELADD[0]= OB[0].RELADD[1]= OB[0].RELADD[2]= 0;
   OFFS.X=0;
   OFFS.Y=0;
   OFFS.Z=0;

   DrawN3dObject( OB, Nviewmat,&OFFS);

}

#if 0
SetViewMat(int *MAT, int ZOOM)
{
   Copy3X3(MAT,Nviewmat);
   Trans3X3(Nviewmat,Nviewmat+9);
   *(Nviewmat+2)= -*(Nviewmat+2);
   *(Nviewmat+5)= -*(Nviewmat+5);
   *(Nviewmat+8)= -*(Nviewmat+8);

   *(Nviewmat+1)= -*(Nviewmat+1)+(*(Nviewmat+1)>>2);
   *(Nviewmat+4)= -*(Nviewmat+4)+(*(Nviewmat+4)>>2);
   *(Nviewmat+7)= -*(Nviewmat+7)+(*(Nviewmat+7)>>2);

}
#endif


/*********************************************************/



LoadEMSObject( char *Name,int ID)
{
   int   objhndl, txtrhndl;
   int far *OBJB;
   char far *P;
   char str[14];
   int   i;

   if ( (ID<0)|| (ID>255) ) return(0);

   if ( (objhndls[ID].objhndl=LoadEmsFile(Name)) ==0 ) return(0);
   if ( (FP_SEG(OBJB)=MapEmsFile( objhndls[ID].objhndl, 0))==0  ) return(0);


//   NOTEXTURE=1;   // !!!!!!


   FP_OFF(OBJB)=0;

   OBJB+=1;    // skip radius
   P= (char far *)(OBJB);

   if ( (*(P)==0)|| NOTEXTURE ) {
      objhndls[ID].txtrhndl=0;
   } else {
      for (i=0;i<13;i++)
         str[i]=*(P++);
      str[i]=0;
      objhndls[ID].txtrhndl=ReadEMSPCXSize(str,0, 256,200);
   }
   return(1);
}


