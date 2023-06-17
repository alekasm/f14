//***************************************************************************
// File: 3DPLANES.C
//
// Author: Mike McDonald
//
// Initial code by Sid Meier/Andy Hollis
//
// Description:  Process 3D planes and other single 3D objects
//
//***************************************************************************
#include <dos.h>
#include <stdlib.h>
#include "Library.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "F15defs.h"
#include "sounds.h"
#include "armt.h"
#include "planeai.h"
#include "proto.h"
#include "3dobjdef.h"

extern  RPS     *Rp3D;
extern 	RPS		*RpCRT;

extern  UBYTE   PlaneFlags[];
extern  SWORD   AnimVals[];
extern  int     O[3][3];

extern  int     OurHead,OurRoll,OurPitch;
extern  UWORD   Alt;
extern  long    ViewX,ViewY;
extern  int     ViewZ;
extern  long    PxLng,PyLng;
extern  int     View;
extern  int     sx,sy,sz;
extern  COORD   chuteX,chuteY;
extern  int     chuteZ,chuteDZ;
extern  int     MinAlt;
extern  int     THRUST;
extern  int     Gees;
extern	int		Knots;
extern  int     detected;
extern  int     Firing;
extern	UWORD	*PalPtr3D;
extern  int     LightsOn;
extern	int		DESIGNATED;

extern   int   NOTEXTURE;

extern	int		DeckPlaneScale;
extern 	int		PlaneScale;
extern  UBYTE   mS1n,mS2n;
extern int whichourcarr;
extern int Fticks;

Shift3X3(int *M, int SHFT);


#define	DEG		182

int PlaneAnimVals[32];
int LIGHTS = 0;
int	PLANEDETAIL=2;
int WingRot=(48*DEG);
extern int WingsReady;
extern int PreparePlaneWings;
int HookDown=0;
int HookRot=0;
int slposx=0;
int slposy=0;

extern	int	GFLSH;
int	pflashtbl[4]={251*256+251,146*256+146,147*256+147,146*256+146};

extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct PlaneData far *GetPlanePtr(int PlaneNo);

extern long labs(long X);

int Do3dObject(int Obj, long OX, long OY, UWORD OZ, int ID,int TYPE);

//***************************************************************************
Do3dObject(int Obj, long OX, long OY, UWORD OZ, int ID,int TYPE)
{
    long dx,dy;
    int dz;
	long t1;
	long t2;

	int	SCL;


    dx = OX - ViewX;
    dy = ViewY - (0x100000L-OY);
    dz = OZ - ViewZ;

	if (abs(dz) > 0x3ffe) return (1);


	t1 = labs(dy);
	if (t1>0x3fff) return(1);
	t1 = labs(dx);
	if (t1>0x3fff) return(1);


	switch(TYPE) {
		case PLANES_OBJ:
			SCL=40;
			break;
		case SHADOW_OBJ:
			SCL=40;
			break;
		case MISSILE_OBJ:
			SCL=4;
			break;
		case CHUTE_OBJ:
			SCL=30;
		break;
		case CHAFF_OBJ:
		case FLARE_OBJ:
			SCL=30;
		break;
		case COOLF14_OBJ:
			SCL=40;
			break;
		case REG_BOAT_OBJ:
			SCL=4;
			break;
		default:
			SCL=4;
			break;
		}
	Stack3DObject(Obj, (int) dx, dz, (int) dy, ID, SCL,TYPE);

}


//***************************************************************************
Boats3D()
{
	int i;
	int pic;
	struct PlaneData far *tempplane;


    for (i=LASTPLANE; i<MAXPLANE; i++) {
		if (whichourcarr == i) continue;
        if (ps[i].status&ACTIVE) {
            lproject(ps[i].xL, ps[i].yL, ps[i].alt);
            if (sz<0) {
				pic = GetPicType(ps[i].type);
				Do3dObject(pic, ps[i].xL,ps[i].yL,ps[i].alt,i,REG_BOAT_OBJ);
			}

        }
    }
}
//**************************************************//
Shift3X3(int *M, int SHFT)
{
	int i;
	for (i=0;i<9;i++)
	{
		*(M)= (*(M)>>SHFT);
		M++;
	}
}


//***************************************************************************
DrawRegBoat(int NUM, int X, int Y, int Z, int ID, int SHFT)
{
    struct Planes  *psi;
    int             r;
	int Anim[50];
	int M[9];

    psi = ps+ID;

    MakeMatHPR(psi->head, -psi->pitch, psi->roll, O);
	InvertMatrix();

	if (SHFT) {
		Shift3X3(O,SHFT);
//		return(1);
	}

	NDraw3DObject(NUM, X,Y,Z, O, PlaneAnimVals,0);
}



//***************************************************************************
Planes3D()
{
	int i;
	int pic;
	struct MissileData far *tempmissile;

	/* Display all airplanes */

    for (i=0; i<NPLANES; i++) {
        if ((ps[i].status&ACTIVE) && (!DOINVISIBLE) && (!(bv[i].behaveflag & AI_INVISIBLE))) {
            lproject(ps[i].xL, ps[i].yL, ps[i].alt);
            if (sz<0) {
                if (sz>-32) {  // (32) 5.3 naut. miles
					pic = GetPicType(ps[i].type);
                    Do3dObject(pic, ps[i].xL,ps[i].yL,ps[i].alt, i,PLANES_OBJ);
                } else if (sz>-64) { // 21.5 naut. miles
                    if (sx!=-1) DrawDot(Rp3D, sx,sy, 228);
                    // if (sx!=-1) DrawDot(Rp3D, sx,sy, PLANECOLOR-(-sz>>3));
                }
            }
        }
    }


	/* Display all missiles */
    for (i=0; i<NSAM; i++) {
        if (ss[i].dist) {
			if ((i<NESAM) || ((ss[i].srctype==1 && ss[i].source==MAXPLANE) && (!(AnimIsRunning(ss[i].station))))) {
                lproject (ss[i].x, ss[i].y, ss[i].alt);
                if (sz<0) {
                    if (sz>-32) {
						tempmissile = GetMissilePtr(ss[i].type);
                        Do3dObject(tempmissile->objslot, ss[i].x,ss[i].y,ss[i].alt, i,MISSILE_OBJ);
                    } else {
                        if (sx!=-1) DrawDot (Rp3D, sx,sy, i<NESAM?HISMISSLCOLOR:ORANGEFLAME);  /* if far away, just draw a dot */
                    }
                }
            }
        }
    }

	/* Enemy parachute ? */
    if (chuteZ>0) {
        Do3dObject(N3DZ_CHUTE, S2L(chuteX),S2L(chuteY),chuteZ, 0,CHUTE_OBJ);
    }

	for(i = 0; i < NUMPTROOPS; i ++) {
	    if (paratroopers[i].chuteZ>0) {
    	    Do3dObject(N3DZ_CHUTE, S2L(paratroopers[i].chuteX),S2L(paratroopers[i].chuteY),paratroopers[i].chuteZ, -999,CHUTE_OBJ);
		}
	}

	for (i=0; i<NDECOY; i++) {
        if (ds[i].time) {
			lproject (S2L(ds[i].x),S2L(ds[i].y),ds[i].alt);
        	if (sz<0) {
        		if (sz>-32) {
            		if (ds[i].type==FLARE) {
	    	    		Do3dObject(N3DZ_FLARE, S2L(ds[i].x),S2L(ds[i].y),ds[i].alt, -999,FLARE_OBJ);
            		} else {
	    	    		Do3dObject(N3DZ_CHAFF, S2L(ds[i].x),S2L(ds[i].y),ds[i].alt, -999,CHAFF_OBJ);
            		}
            	} else {
            		if (sx!=-1) DrawDot(Rp3D, sx,sy, (ds[i].type==FLARE)?BLUEFLAME:225);
				}
			}
		}
	}

	// Display Our F14

    if ((View & VOFFSET) && (Alt || !ejected)) {
		Do3DObject(N3DZ_L_COOLF14, PxLng, 0x100000L-PyLng, Alt+2, OurHead,COOLF14_OBJ);
    }

}

//***************************************************************************
DrawShadow(int NUM, int X, int Y, int Z, int Head, int SHFT)
{
    MakeMatHPR (Head, -0, 0, O);
    InvertMatrix ();
	if (SHFT)
		Shift3X3(O,SHFT);
	NDraw3DObject(NUM, X, Y, Z, O, AnimVals);
}

//***************************************************************************
DrawChute(int NUM, int X, int Y, int Z, int SHFT)
{
    MakeMatHPR(0, 0, 0, O);
    InvertMatrix();
	if (SHFT)
		Shift3X3(O,SHFT);
	NDraw3DObject(N3DZ_CHUTE, X/2, Y/2, Z/2, O, AnimVals);
}

//***************************************************************************
DrawDecoy(int NUM, int X, int Y, int Z, int SHFT)
{
    MakeMatHPR(0, 0, 0, O);
    InvertMatrix();
	if (SHFT)
		Shift3X3(O,SHFT);
	NDraw3DObject(NUM, X/2, Y/2, Z/2, O, AnimVals);
}



//***************************************************************************
DrawMissile(int NUM, int X, int Y, int Z, int ID, int SHFT)
{
    struct Sams *ssi;

	MakeMatHPR(ss[ID].head, -ss[ID].pitch, ss[ID].roll,O);
    InvertMatrix();

    ssi = ss+ID;

	if (SHFT)
		Shift3X3(O,SHFT);

    if (ssi->srctype==1 && ssi->source==MAXPLANE) {
		switch(ssi->weapon) {
			case W_AIM_7:
				NDraw3DObject(N3DZ_SPARROW, X, Y, Z, O, AnimVals);
			break;
			case W_AIM_9:
				NDraw3DObject(N3DZ_SIDEWIND, X, Y, Z, O, AnimVals);
			break;
			case W_AIM_54:
				NDraw3DObject(N3DZ_PHOENIX, X, Y, Z, O, AnimVals);
			break;
			default:
				NDraw3DObject(N3DZ_SPARROW, X, Y, Z, O, AnimVals);
			break;
		}
    } else {
		switch(ssi->weapon) {
			case W_AIM_7:
				NDraw3DObject(N3DZ_SPARROW, X, Y, Z, O, AnimVals);
			break;
			case W_AIM_9:
				NDraw3DObject(N3DZ_SIDEWIND, X, Y, Z, O, AnimVals);
			break;
			case W_AIM_54:
				NDraw3DObject(N3DZ_PHOENIX, X, Y, Z, O, AnimVals);
			break;
			default:
				NDraw3DObject(N3DZ_HARPOON, X, Y, Z, O, AnimVals);
			break;
		}
	}
}



extern struct obj3d {
   int   NUM;
   int   XR;
   int   YR;
   int   ZR;
   int   *MAT;
   int   *ANIM;
   long  RELADD[3];
};


//***************************************************************************
extern	int   IdentInvMat[];
int   MissileAnimVals[16];

struct obj3d CoolObj[16]={
	-1,0,0,0, &IdentInvMat,&AnimVals, 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+1), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+2), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+3), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+4), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+5), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+6), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+7), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+8), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+9), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+10), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+11), 0L,0L,0L,
	-1,0,0,0, &IdentInvMat,&(MissileAnimVals+12), 0L,0L,0L,
	N3DZ_LTAIL,0,0,0, &IdentInvMat,&(MissileAnimVals+13), 0L,0L,0L,
	N3DZ_RTAIL,0,0,0, &IdentInvMat,&(MissileAnimVals+14), 0L,0L,0L,
};

//***************************************************************************
Init3DMissiles()
{
	int i;

	for (i=0; i<12; i++) {
		MissileAnimVals[i+1]=0;
		CoolObj[i+1].XR = 0;
		CoolObj[i+1].YR = 0;
		CoolObj[i+1].ZR = 0;
		switch(F14_STATION[i]) {
			case W_AIM_9:
				CoolObj[i+1].NUM = N3DZ_SIDEWIND;
			break;
			case W_AIM_7:
				CoolObj[i+1].NUM = N3DZ_SPARROW;
			break;
			case W_AIM_54:
				CoolObj[i+1].NUM = N3DZ_PHOENIX;
			break;
			case W_FUEL_TANK:
				CoolObj[i+1].NUM = N3DZ_FUELTANK;
			break;
		}
	}
}

//***************************************************************************
int AnimIsRunning(UWORD SlotNMount)
{
	if (CoolObj[SlotNMount+1].NUM == -1) return(0);
	return(1);
}

//***************************************************************************
CoolF14MissileAnims()
{
   int   i;

   for (i=1;i<13;i++)
      {
      if ((CoolObj[i].NUM!=-1)&&(F14_STATION[i-1] == W_NONE))
         {
         MissileAnimVals[i]=0;
         switch (CoolObj[i].NUM)
            {
            case N3DZ_SIDEWIND:
               MissileAnimVals[i]=1;
               if ((CoolObj[i].ZR-= (6*Fticks))<-1000)
                  CoolObj[i].NUM=-1;
               break;
            case N3DZ_FUELTANK:
               if ((CoolObj[i].YR-=(Fticks))<-400)
                  CoolObj[i].NUM=-1;
               break;
			case N3DZ_PHOENIX:
               if (CoolObj[i].YR<-35)
                  {
                  MissileAnimVals[i]=1;
                  if ((CoolObj[i].ZR-= (6*Fticks))<-1000)
                     CoolObj[i].NUM=-1;
                  }
               else
                  {
                  CoolObj[i].YR-=(Fticks);
                  }
			break;
            default:
               if (CoolObj[i].YR<-20)
                  {
                  MissileAnimVals[i]=1;
                  if ((CoolObj[i].ZR-= (6*Fticks))<-1000)
                     CoolObj[i].NUM=-1;
                  }
               else
                  {
                  CoolObj[i].YR-=(Fticks);
                  }
               break;
            }
         }
      }
}

//***************************************************************************
DrawCoolF14(int X, int Y, int Z, int SHFT)
{
    int M[9];
    int V2[3];
	int	bgsscl;
	int	j;

   int   dst,mydetail;

	bgsscl= 1;

	/* Compensate for slight pitch of F-14 */

    MakeMatHPR(OurHead, -(OurPitch+(3*DEG)), OurRoll, O);
	InvertMatrix();

    Trans3X3(O, M);
    V2[0] = -4*X;
    V2[1] = -4*Y;
    V2[2] = -4*Z;
    Rotate(V2, M);

   dst=X/8+Y/8+Z/8;

   mydetail=2;
   if (dst>400)
      mydetail--;
   if (dst>800)
      mydetail--;

	if (SHFT)
      {
      mydetail=0;
		Shift3X3(O,SHFT);
      }
   if (mydetail<0) mydetail=0;
   if (mydetail>PLANEDETAIL)
      mydetail=PLANEDETAIL;


   if (mydetail==0)
      {
      if (Status&WHEELSUP)
         DrawSuper3DObject(CoolObj,N3DZ_FLIGHT_F14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
      else
         DrawSuper3DObject(CoolObj,N3DZ_DECK_F14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
      }
   if (mydetail==1)
      {
		if (V2[0]>0)
         {
         AnimVals[14]=WingRot;
  		   AnimVals[30]=-(slposx*DEG)+(slposy*DEG);
			DrawSuper3DObject(CoolObj,N3DZ_L_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
			NDraw3DObject(N3DZ_C_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
			AnimVals[14]=-WingRot;
		   AnimVals[30]=(slposx*DEG)+(slposy*DEG);
			DrawSuper3DObject(CoolObj,N3DZ_R_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
         }
      else
         {
			AnimVals[14]=-WingRot;
         AnimVals[30]=(slposx*DEG)+(slposy*DEG);
			DrawSuper3DObject(CoolObj,N3DZ_R_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
			NDraw3DObject(N3DZ_C_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
			AnimVals[14]=WingRot;
         AnimVals[30]=-(slposx*DEG)+(slposy*DEG);
			DrawSuper3DObject(CoolObj,N3DZ_L_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
         }
      }
   if (mydetail==2)
      {
	   if (V2[0]>0)
         {
		   AnimVals[14]=WingRot;
		   AnimVals[29]=(WingRot/8);
  		   AnimVals[30]=-(slposx*DEG)+(slposy*DEG);
		   DrawSuper3DObject(CoolObj, N3DZ_L_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
		   DrawSuper3DObject(CoolObj, N3DZ_LM_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
		   DrawSuper3DObject(CoolObj, N3DZ_RM_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
		   AnimVals[14]=-WingRot;
		   AnimVals[29]=-(WingRot/8);
		   AnimVals[30]=(slposx*DEG)+(slposy*DEG);
		   DrawSuper3DObject(CoolObj, N3DZ_R_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
	      }
      else
         {
         AnimVals[14]=-WingRot;
         AnimVals[29]=-(WingRot/8);
         AnimVals[30]=(slposx*DEG)+(slposy*DEG);
         DrawSuper3DObject(CoolObj, N3DZ_R_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
         DrawSuper3DObject(CoolObj, N3DZ_RM_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
         DrawSuper3DObject(CoolObj, N3DZ_LM_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
         AnimVals[14]=WingRot;
         AnimVals[29]=-(WingRot/8);
         AnimVals[30]=-(slposx*DEG)+(slposy*DEG);
         DrawSuper3DObject(CoolObj, N3DZ_L_COOLF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, AnimVals);
	      }
      }
}


//***************************************************************************
int HookDownAnim()
{
	HookDown^=1;
}

//***************************************************************************
UpdateOurPlaneAnims ()
{
	int stickx,sticky;

   int   t1,t2;
   int   beforeval;

	stickx=(mS1n&0xff)>>3;
	sticky=(mS2n&0xff)>>3;
	stickx=16-stickx;
	sticky=16-sticky;
	slposx = ((stickx*300)/(Knots+200));
	slposy = ((sticky*300)/(Knots+200));


	if (!(HookDown) && (HookRot>0)) {
		HookRot-=(2*DEG);
	}

	if (HookDown && (HookRot<(45*DEG))) {
		HookRot+=(2*DEG);
	}
	AnimVals[27]=HookRot;


   if (Knots<275)
      WingRot-= (5*DEG*Fticks)/60;
   else
      WingRot+= (5*DEG*Fticks)/60;
   if (WingRot<(10*DEG)) WingRot=(10*DEG);
   if (WingRot>(48*DEG)) WingRot=(48*DEG);
/*
	if (Knots>100) PreparePlaneWings=0;

	if (PreparePlaneWings && (WingRot>(10*DEG))) {
		WingRot-=(2*DEG);
	}

	if ((Knots>50 && Knots<200) && (WingRot>(10*DEG))) {
		WingRot-=(2*DEG);
	}

	if (Knots>350 && (WingRot<(48*DEG))) {
		WingRot+=(2*DEG);
	}
*/

	AnimVals[9] = !(LIGHTS);
	AnimVals[10]= LIGHTS;
    AnimVals[15]= (rnd2(4)!=0 && Gees>48)? 0:1;

	// centerline tank

	AnimVals[4]=0;
	AnimVals[3]= (Firing)? 1:0;

	if (THRUST>100)	{
		AnimVals[13]= 1;
		AnimVals[23]-= DEG/6;
	}
	else
		AnimVals[23]+= DEG/6;

	if (AnimVals[23]>=0) {
		AnimVals[13]= 0;
		AnimVals[23]=0;
	}

	if (AnimVals[23]<(-DEG)) AnimVals[23]=(-DEG);


	if ((Status&BRAKESON) && !(Alt==MinAlt && OurPitch==0)) {
		if (AnimVals[19]>=0x4000)
			Sound(N_AirBrakeOn, 1,0);
		beforeval = AnimVals[19];
		AnimVals[19] -=5*DEG;
		if((beforeval > 0x2000) && (AnimVals[19] <= 0x2000))
			Sound(N_AirBrakeOff, 1,0);
	}
	else {
		if (AnimVals[19]<=0x2000)
			Sound(N_AirBrakeOn, 1,0);
		beforeval = AnimVals[19];
		AnimVals[19]+= 5*DEG;
		if((beforeval < 0x4000) && (AnimVals[19] >= 0x4000))
			Sound(N_AirBrakeOff, 1,0);
	}

	if (AnimVals[19]<0x2000) AnimVals[19]=0x2000;

	AnimVals[20]=1;
	if (AnimVals[19]>=0x4000) {
		AnimVals[19]=0x4000;
		AnimVals[20]=0;
	}
/*
	if (!(Status&WHEELSUP)) {
		AnimVals[6]=1;
		AnimVals[7]=1;
		AnimVals[8]=1;
	} else {
		AnimVals[6]=0;
		AnimVals[7]=0;
		AnimVals[8]=0;
	}
*/
	// New Stuff

   t1=((DEG*Fticks)/3);
   t2=t1-(t1/4);
   if (!(Status&WHEELSUP))
      {
      t1=-t1;
      t2=-t2;
      }
   beforeval = AnimVals[25];
   AnimVals[24]+=t1;
   AnimVals[25]+=t2;
   if (AnimVals[24]<0)
      AnimVals[24]=0;
   if (AnimVals[25]<0)
      AnimVals[25]=0;
   if(((beforeval == 0) && (AnimVals[25] > 0)) || ((beforeval == (70*DEG)) && (AnimVals[25] < (70*DEG))))
		Sound(N_GearOpenSeq, 1,0);
   if(((beforeval > 0) && (AnimVals[25] <= 0)) || ((beforeval < (70*DEG)) && (AnimVals[25] >= (70*DEG))))
		Sound(N_GearCloseSeq, 1,0);

   if (AnimVals[24]>=(70*DEG))
      {
      AnimVals[24]=(70*DEG);
      AnimVals[6]=0;
      }
   else
      {
      AnimVals[6]=1;
      }

   if (AnimVals[25]>=(70*DEG))
      {
      AnimVals[25]=(70*DEG);
      AnimVals[7]=0;
      }
   else
      {
      AnimVals[7]=1;
      }

	if (AnimVals[6]||AnimVals[7])
		AnimVals[8]=1;
	else
		AnimVals[8]=0;

	CoolF14MissileAnims();
}

//***************************************************************************
InitOurCoolPlane(InAir)
{
    if (InAir) {            // gear up/down
        AnimVals[5]  =
        AnimVals[6]  =
        AnimVals[7]  =
        AnimVals[8]  = 0;
        AnimVals[21] =
        AnimVals[22] = 0x4000;
    } else {
        AnimVals[6]  =
        AnimVals[7]  = 1;
        AnimVals[5]  =
        AnimVals[8]  =
        AnimVals[21] =
        AnimVals[22] = 0;
    }
    AnimVals[19] = 0x4000;  // speed brake down
    AnimVals[20] = 0;
}



//***************************************************************************
DrawTexPlane(int NUM, int X, int Y, int Z, int ID, int SHFT)
{
    struct Planes  *psi;
    int r;
	int Anim[50];
	int M[9];
	int MedAnimVals[32] = { 0 };
	static int MedWingRot;


    int V2[3];
	int	bgsscl;
	struct PlaneData far *planetemp;
	bgsscl= 1;

    psi = ps+ID;

	planetemp = GetPlanePtr(psi->type);

	if ((planetemp->PlaneEnumId == SO_F_14FDA) || (planetemp->PlaneEnumId == SO_F_14FDB) ||
		(planetemp->PlaneEnumId == SO_F_14FDC) || (planetemp->PlaneEnumId == SO_F_14MCA) ||
		(planetemp->PlaneEnumId == SO_F_14MCB) || (planetemp->PlaneEnumId == SO_F_14MCC)) {

			MakeMatHPR(psi->head, -psi->pitch, psi->roll, O);
   			InvertMatrix();

			Trans3X3(O, M);
    		V2[0] = -4*X;
    		V2[1] = -4*Y;
    		V2[2] = -4*Z;
       		Rotate(V2, M);

			if (SHFT)
				Shift3X3(O,SHFT);

    		MedAnimVals[9] = !(LightsOn);	// Formation is On
    		MedAnimVals[10] = LightsOn;     // Lights is On
    		MedAnimVals[12] = 1;


			if(psi->status & TAKEOFF) {
				MedAnimVals[6] = 1;
				MedAnimVals[7] = 1;
			}
			else if((psi->status & LANDING) && ((bv[ID].Behavior != AICarrierLand) || (bv[ID].threathead < 7))) {
				MedAnimVals[6] = 1;
				MedAnimVals[7] = 1;
			}
			else {
				MedAnimVals[6] = 0;
				MedAnimVals[7] = 0;
			}


			MedAnimVals[13]= ((psi->status&TAKEOFF)
                      ||(detected && !psi->alert && r<0x1000)); // afterburner

			if (psi->speed < 100)
				MedAnimVals[13] = 0;

    		MedAnimVals[15]= r<0x3000;                                // vapor trail
			MedAnimVals[21]=0;
			MedAnimVals[22]=0;

			MedAnimVals[27]=0;


         MedWingRot = (48*DEG);
         if (psi->speed<350)
            {
            if (psi->speed<250)
               MedWingRot = (48*DEG);
            else
               MedWingRot= (10*DEG)+((38*(psi->speed-250))/100)*DEG;
            }


/*
			if (psi->speed <= 150)
				MedWingRot = (10*DEG);

			if ((psi->speed > 150) && (psi->speed < 250))
				MedWingRot = (25*DEG);

			if ((psi->speed > 250) && (psi->speed < 350))
				MedWingRot = (35*DEG);

			if (psi->speed >= 350)
				MedWingRot = (48*DEG);

*/


			if (V2[0]>0) {
				MedAnimVals[14]=MedWingRot;
				DrawSuper3DObject(CoolObj,N3DZ_L_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
				NDraw3DObject(N3DZ_C_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
				MedAnimVals[14]=-MedWingRot;
				DrawSuper3DObject(CoolObj,N3DZ_R_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
			} else {
				MedAnimVals[14]=-MedWingRot;
				DrawSuper3DObject(CoolObj,N3DZ_R_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
				NDraw3DObject(N3DZ_C_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
				MedAnimVals[14]=MedWingRot;
				DrawSuper3DObject(CoolObj,N3DZ_L_MEDF14, X*bgsscl,Y*bgsscl, Z*bgsscl, O, MedAnimVals);
			}
			return(1);
		}

		*(PalPtr3D+146)= pflashtbl[GFLSH&3];
		r = abs(psi->roll);
    	PlaneAnimVals[ 9]=
		PlaneAnimVals[12] = !(psi->alert==0 && LightsOn);            // var12 = lights
		PlaneAnimVals[10] = (LightsOn);

		if(psi->status & TAKEOFF) {
			PlaneAnimVals[6] = 1;
		}
		else if((psi->status & LANDING) && ((bv[ID].Behavior != AICarrierLand) || (bv[ID].threathead < 7))) {
			PlaneAnimVals[6] = 1;
		}
		else {
			PlaneAnimVals[6] = 0;
		}

		PlaneAnimVals[13]= ((psi->status&TAKEOFF)
                      ||(detected && !psi->alert && r<0x1000)); // afterburner

		if (psi->speed < 100)
			PlaneAnimVals[13] = 0;

    	PlaneAnimVals[15]= r<0x3000;                                // vapor trail
    	PlaneAnimVals[25]= 1;                                       // Donkey dick for Tanker
		PlaneAnimVals[21]=0;
		PlaneAnimVals[22]=0;

    	MakeMatHPR(psi->head, -psi->pitch, psi->roll, O);
		InvertMatrix();

		if (SHFT)
			Shift3X3(O,SHFT);

		NDraw3DObject(NUM, X,Y,Z, O, PlaneAnimVals);

		*(PalPtr3D+146)= 146*256+146;
}


