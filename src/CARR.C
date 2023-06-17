//***************************************************************************
//* File Name: carr.c
//*
//* Project: Fleet Defender: F-14 Tomcat
//* Author: M. McDonald
//*
//* Description:  	Support functions for drawing carrier/carrier landing/
//*					carrier takeoff/etc.
//*
//* Copyright 1993 MicroProse Software, Inc.
//***************************************************************************
#include <dos.h>
#include <stdlib.h>
#include "Library.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "F15defs.h"
#include "speech.h"
#include "sounds.h"
#include "carr.h"
#include "proto.h"
#include "planeai.h"
#include "setup.h"
#include "3dobjdef.h"

#define	DEG			182
#define	ANYCAT		-1
#define	NOCATSAVAIL	-1
#define MAXCATS		4

extern  int 	MissionWeather;
extern  int     OurRoll,OurPitch;
extern	UWORD	OurHead;
extern  UWORD   Alt,OurAlt;
extern	long	RadarAlt;
extern  int		MinAlt;
extern  long    ViewX,ViewY;
extern  int     ViewZ;
extern  long    PxLng,PyLng;
extern  int     View;
extern  int     VVI;
extern	int		Knots;
extern  UWORD   AirSpeed;
extern  int		CanCrash;
extern  int     THRUST;
extern	int		ThrustI;
extern  int     O[3][3];
extern	int		PlaneAnimVals[];
extern  int     whichourcarr;     // holds value for which boat our carrier
extern  int     Fticks;
extern  int     LightsOn;

extern	int Shift3X3(int *M, int SHFT);
extern   int   Scale3d;


extern  SWORD   AnimVals[];
extern   int   HookRot;

extern struct FPARAM{
	long	X;
  	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

/* Carrier specific var's */

int CarrierAnimVals[48];
int CarrierScale=4;
int	CARRIERSCALEKLUDGE=0;
int PlaneScale=40;
int MeatBall;
int StormLevel=0;
int WaveHeight;
int WaveSpeed;

carriers far carrier;

int WireCaught=0;
int PreparePlaneWings=0;

extern int  AvailCat[];
extern int	BlastShield[];
extern int  CatReady[];
extern int	WingsReady;
extern long labs(long X);

int BelowDeck;
extern   int   TRIM;
extern   int   DirectLiftControl;
extern   int   HookDown;

extern confData ConfigData;


//***************************************************************************
InitCarrier()
{
	int i;
	int CAT0X[] = {-108, -102, -107};
	int CAT0Z[] = { 170,  190,  184};
	int CAT1X[] = { -66,  -63,  -60};
	int CAT1Z[] = { 130,  124,  146};
	int CAT2X[] = { -18,  -20,  -42};
	int CAT2Z[] = {-214, -168, -187};
	int CAT3X[] = {  55,   55,   43};
	int CAT3Z[] = {-216, -185, -187};

	int WIR0X1[] = {-61,  -51,  -53};
	int WIR0Z1[] = {348,  368,  340};
	int WIR0X2[] = { 59,   33,   31};
	int WIR0Z2[] = {324,  356,  330};
	int WIR0CX[] = { -1,   -9,  -11};
	int WIR0CZ[] = {336,  362,  334};
	int WIR1X1[] = {-69,  -57,  -62};
	int WIR1Z1[] = {308,  326,  298};
	int WIR1X2[] = { 51,   27,   24};
	int WIR1Z2[] = {284,  314,  288};
	int WIR1CX[] = { -9,  -15,  -18};
	int WIR1CZ[] = {296,  320,  292};
	int WIR2X1[] = {-77,  -63,  -67};
	int WIR2Z1[] = {268,  284,  256};
	int WIR2X2[] = { 43,   21,   17};
	int WIR2Z2[] = {244,  272,  246};
	int WIR2CX[] = {-17,  -21,  -25};
	int WIR2CZ[] = {256,  278,  250};
	int WIR3X1[] = {-85,  -69,  -74};
	int WIR3Z1[] = {223,  242,  214};
	int WIR3X2[] = { 35,   15,   10};
	int WIR3Z2[] = {199,  230,  204};
	int WIR3CX[] = {-25,  -27,  -32};
	int WIR3CZ[] = {211,  236,  208};

	int CBX1[] =   {-145, -130, -134};
	int CBX2[] =   { 120,  114,  116};
	int CBZ1[] =   {-520, -524, -498};
	int CBZ2[] =   { 545,  564,  575};

	int WBX1[] =   { -85,  -69,  -74};
	int WBX2[] =   {  59,   33,   31};
	int WBZ1[] =   { 223,  230,  204};
	int WBZ2[] =   { 348,  368,  340};

	int TWRX1[] =   {  95,   90,   76};
	int TWRZ1[] =   {  84,  135,  -30};
	int TWRX2[] =   { 120,  110,  101};
	int TWRZ2[] =   { 230,  250,   80};

	int whichcarr;

	if(whichourcarr >= 0) {
		carrier.x = ps[whichourcarr].xL * 100L;
		carrier.z = ps[whichourcarr].yL * 100L;
	}
	else {
		carrier.x = 914813L * 100L;
		carrier.z = 744376L * 100L;
	}

	carrier.y = 0;
	carrier.wcheading = -0x07e0;
	carrier.heading = 0x0;
	carrier.deckht = 66L * 100L;
	carrier.pitch=0;
	carrier.roll=0;

	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	carrier.catpult[0].x = ((long)CAT0X[whichcarr] * 100)/4;
	carrier.catpult[0].z = ((long)CAT0Z[whichcarr] * 100)/4;
	carrier.catpult[0].heading = 0;

	carrier.catpult[1].x = ((long)CAT1X[whichcarr] * 100)/4;
	carrier.catpult[1].z = ((long)CAT1Z[whichcarr] * 100)/4;
	carrier.catpult[1].heading = -0x400;

	carrier.catpult[2].x = ((long)CAT2X[whichcarr] * 100)/4;
	carrier.catpult[2].z = ((long)CAT2Z[whichcarr] * 100)/4;
	carrier.catpult[2].heading = 0;

	carrier.catpult[3].x = ((long)CAT3X[whichcarr] * 100)/4;
	carrier.catpult[3].z = ((long)CAT3Z[whichcarr] * 100)/4;
	carrier.catpult[3].heading = -0x0280;

	carrier.wires[0].x1 = ((long)WIR0X1[whichcarr] * 100)/4;
	carrier.wires[0].z1 = ((long)WIR0Z1[whichcarr] * 100)/4;
	carrier.wires[0].x2 = ((long)WIR0X2[whichcarr] * 100)/4;
	carrier.wires[0].z2 = ((long)WIR0Z2[whichcarr] * 100)/4;
	carrier.wires[0].cx = ((long)WIR0CX[whichcarr] * 100)/4;
	carrier.wires[0].cz = ((long)WIR0CZ[whichcarr] * 100)/4;

	carrier.wires[1].x1 = ((long)WIR1X1[whichcarr] * 100)/4;
	carrier.wires[1].z1 = ((long)WIR1Z1[whichcarr] * 100)/4;
	carrier.wires[1].x2 = ((long)WIR1X2[whichcarr] * 100)/4;
	carrier.wires[1].z2 = ((long)WIR1Z2[whichcarr] * 100)/4;
	carrier.wires[1].cx = ((long)WIR1CX[whichcarr] * 100)/4;
	carrier.wires[1].cz = ((long)WIR1CZ[whichcarr] * 100)/4;

	carrier.wires[2].x1 = ((long)WIR2X1[whichcarr] * 100)/4;
	carrier.wires[2].z1 = ((long)WIR2Z1[whichcarr] * 100)/4;
	carrier.wires[2].x2 = ((long)WIR2X2[whichcarr] * 100)/4;
	carrier.wires[2].z2 = ((long)WIR2Z2[whichcarr] * 100)/4;
	carrier.wires[2].cx = ((long)WIR2CX[whichcarr] * 100)/4;
	carrier.wires[2].cz = ((long)WIR2CZ[whichcarr] * 100)/4;

	carrier.wires[3].x1 = ((long)WIR3X1[whichcarr] * 100)/4;
	carrier.wires[3].z1 = ((long)WIR3Z1[whichcarr] * 100)/4;
	carrier.wires[3].x2 = ((long)WIR3X2[whichcarr] * 100)/4;
	carrier.wires[3].z2 = ((long)WIR3Z2[whichcarr] * 100)/4;
	carrier.wires[3].cx = ((long)WIR3CX[whichcarr] * 100)/4;
	carrier.wires[3].cz = ((long)WIR3CZ[whichcarr] * 100)/4;

	carrier.cbx1 = ((long)CBX1[whichcarr] * 100)/4;
	carrier.cbx2 = ((long)CBX2[whichcarr] * 100)/4;
	carrier.cbz1 = ((long)CBZ1[whichcarr] * 100)/4;
	carrier.cbz2 = ((long)CBZ2[whichcarr] * 100)/4;

	carrier.wbx1 = ((long)WBX1[whichcarr] * 100)/4;
	carrier.wbx2 = ((long)WBX2[whichcarr] * 100)/4;
	carrier.wbz1 = ((long)WBZ1[whichcarr] * 100)/4;
	carrier.wbz2 = ((long)WBZ2[whichcarr] * 100)/4;

	carrier.twrx1 = ((long)TWRX1[whichcarr] * 100)/4;
	carrier.twrz1 = ((long)TWRZ1[whichcarr] * 100)/4;
	carrier.twrx2 = ((long)TWRX2[whichcarr] * 100)/4;
	carrier.twrz2 = ((long)TWRZ2[whichcarr] * 100)/4;

	for (i=0; i<47; i++)
		CarrierAnimVals[i]=0;

	for (i=3; i<10; i+=2)
		CarrierAnimVals[i]=(45*DEG);
}


//***************************************************************************
InitCarrierObjects()
{
	int XP,YP,ZP;
	int V[3];
	int M[9];
	int TM[9];

	V[0] = (int)carrier.wires[0].cx;
	V[1] = (int)carrier.deckht;
	V[2] = (int)carrier.wires[0].cz+850;

	/* Why do we need to take neg. heading to get correct showing? */
	MakeMatHPR(carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);


	PxLng = ((carrier.x+V[0])/100L);
	PyLng = 0x100000 - ((carrier.z+V[2])/100L);

	OurHead = carrier.heading;
	OurAlt = (int)(V[1]/100L);
	MinAlt = (int)(V[1]/100L);
	RadarAlt = OurAlt;
	Alt = OurAlt>>2;
	CarrierAnimVals[2]=0;
	CarrierAnimVals[3]=90;
	CarrierAnimVals[4]=0;
	CarrierAnimVals[5]=90;
	CarrierAnimVals[6]=0;
	CarrierAnimVals[7]=90;
	CarrierAnimVals[8]=0;
	CarrierAnimVals[9]=90;
}

//***************************************************************************
int InitPlaneOnCarrier(int reqcat)
{
	int V[3];
	int M[9];
	int TM[9];
	int cat;

	if((Alt > 20) || (PlayerCat < 0)) {
		InitScottFlight();
		return(0);
	}

//  This now done in loadmiss.c
//	cat = RequestCat(reqcat, -1);
//	PlayerCat = cat;

#ifdef SRECATTEST
	TXT("Player Cat ");
	TXN(PlayerCat);
	Message(Txt);
#endif

	if (cat==NOCATSAVAIL) {
		cat=0;
		Message("No Cats Available",DEBUG_MSG);
	}

	AirSpeed=0;
	OurAlt=(int)(carrier.deckht/100L);

	V[0] = (int)carrier.catpult[cat].x;
	V[1] = (int)carrier.deckht;
	V[2] = (int)carrier.catpult[cat].z+350;

	MakeMatHPR(carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);

	PxLng =((carrier.x+V[0])/100L);
	PyLng =0x100000 - ((carrier.z+V[2])/100L);

	OurPitch=0x2000;
	OurHead = GetCatHeading(cat);
	InitScottFlight();
	THRUST=ThrustI=0;
	MinAlt = (int)(carrier.deckht/100L);
	RadarAlt = OurAlt;
	Alt = OurAlt>>2;
	return(0);
}

//***************************************************************************
int GetCatHeading(int cat)
{
	return(carrier.catpult[cat].heading+carrier.heading);
}



//***************************************************************************
Carrier3D()
{

    long dx = (carrier.x/100L) - ViewX;
	long dy = ViewY - (0x100000L - (carrier.z/100L));
    int dz = (int)((carrier.y/100L) - ViewZ);
	int V[3];
	int M[9];
	long t1;
	int t2;
	long t3;


	t2 = abs(dz);
	if (t2 > 16000) return (1);

	t1 = labs(dx);
	if (t1 > 16000L) return(1);

	t1 = labs(dy);
	if (t1 > 16000L) return(1);

	carrier.heading = 0;

	// Do not stack blast doors if they are not turned on!!!!
	// special landing boxes kludge - put check for easy landing
//	Stack3DObject(0, (int) dx, dz, (int) dy, carrier.heading,1,CARRIER_OBJ_11);

	Stack3DObject(0, (int) dx, dz, (int) dy, carrier.heading,CarrierScale,CARRIER_OBJ_1);


	MakeMatHPR(carrier.heading,0,0,M);
	V[0] = (carrier.catpult[0].x/100L);
	V[2] = (carrier.catpult[0].z/100L) + 20;
	V[1] = 0;
	Rotate(V,M);

	Stack3DObject(0, (((int)dx) + V[0]), (dz+(66/4)), (((int)dy)+V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_6);

	V[0] = (carrier.catpult[1].x/100L);
	V[2] = (carrier.catpult[1].z/100L) + 20;
	V[1] = 0;
	Rotate(V,M);
	Stack3DObject(0, ((int) dx + V[0]), (dz+(66/4)), ((int) dy + V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_7);

	V[0] = (carrier.catpult[2].x/100L);
	V[2] = (carrier.catpult[2].z/100L) + 20;
	V[1] = 0;
	Rotate(V,M);
	Stack3DObject(0, ((int) dx + V[0]), (dz+(66/4)), ((int) dy + V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_8);

	V[0] = (carrier.catpult[3].x/100L);
	V[2] = (carrier.catpult[3].z/100L) + 20;
	V[1] = 0;
	Rotate(V,M);
    Stack3DObject(0, ((int) dx + V[0]), (dz+(66/4)), ((int) dy + V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_9);

	V[0] = 25;
	V[2] = 25;
	V[1] = 0;
	Rotate(V,M);
    Stack3DObject(0, ((int) dx + V[0]), (dz+(66/4)), ((int) dy + V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_10);
//    Stack3DObject(0, ((int) dx + V[0]), (dz+(66/4)), ((int) dy + V[2]), carrier.heading,CarrierScale,CARRIER_OBJ_11);

	if (ViewZ < (66/4))
		Stack3DObject(0, (int) dx, dz, (int) dy, carrier.heading,CarrierScale,CARRIER_OBJ_2);
}


//***************************************************************************
int bogusx;
int bogusy;
int bogusz;
int	lodetailcqr;
DrawCarrier(int X, int Y, int Z,int heading, int SHFT, int objtype)
{
    int M[9];
    int V[3];
	int	MYO[9];
	int i;
	unsigned int dist;
	int bgsscl;

	bgsscl=1;

	MakeMatHPR(heading,0,0,O);
	InvertMatrix();


	if (objtype == CARRIER_OBJ_2) {
		bogusx = X*bgsscl;
		bogusy = Y*bgsscl;
		bogusz = Z*bgsscl;
		return(1);
	}

	if (objtype == CARRIER_OBJ_1) {
		UpdateCarrierAnim();

// 		If easy mode then draw landing gates
//   	if (objtype==CARRIER_OBJ_11)
//      {
//			lodetailcqr=1;
//      	NDraw3DObject(N3DZ_GATES, X,Y, Z, O, CarrierAnimVals);
//      	return(1);
//      }

		if (SHFT) {
			Shift3X3(O,SHFT);
         Scale3d=SHFT;
			NDraw3DObject(N3DZ_TINY_CARR, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
         Scale3d=1;
			lodetailcqr=1;
			return(1);
		}

		dist = Dist3D(X,Y,Z);

		if (dist > 8000) {
			NDraw3DObject(N3DZ_TINY_CARR, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			lodetailcqr=1;
			return;
		}
		if (dist > 5000) {
			NDraw3DObject(N3DZ_WAKE, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BASE_3, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_TOWER_3, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			lodetailcqr=1;
			return;
		}
		if (dist > 1750) {
			NDraw3DObject(N3DZ_WAKE, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BASE_2, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_DECK, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_TOWER_1, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BALL, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			lodetailcqr=1;
			return;
		}

		lodetailcqr=0;

		if (ViewZ >= (66/4)) {
			bogusx = X*bgsscl;
	   		bogusy = Y*bgsscl;
			bogusz = Z*bgsscl;
			NDraw3DObject(N3DZ_WAKE, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BASE_1, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_DECK, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_WIRES, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BALL, X*bgsscl,Y*bgsscl, Z*bgsscl, O, CarrierAnimVals);
		} else {
			NDraw3DObject(N3DZ_BALL, bogusx,bogusy, bogusz, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_WIRES, bogusx,bogusy, bogusz, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_DECK, bogusx,bogusy, bogusz, O, CarrierAnimVals);
			NDraw3DObject(N3DZ_BASE_1, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		}
		return(1);
	}

	if (SHFT||lodetailcqr) {
		return(1);
	}
/*
	if (Dist3D(X,Y,Z) > 1750) {
		return(1);
	}
*/
	switch(objtype) {
		case CARRIER_OBJ_6:
			NDraw3DObject(N3DZ_BD_1, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		break;
		case CARRIER_OBJ_7:
			NDraw3DObject(N3DZ_BD_2, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		break;
		case CARRIER_OBJ_8:
			NDraw3DObject(N3DZ_BD_3, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		break;
		case CARRIER_OBJ_9:
			NDraw3DObject(N3DZ_BD_4, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		break;
		case CARRIER_OBJ_10:
			NDraw3DObject(N3DZ_TOWER_1, bogusx,bogusy, bogusz, O, CarrierAnimVals);
		break;
	}
	return(1);
}


//***************************************************************************
int InBox(int x,int z,int xl,int zl,int xh,int zh)
{
	if ((x<xl) || (x>xh) || (z<zl) || (z>zh))
		return(0);
	else
		return(1);
}

//***************************************************************************
int InBoxLng(long x,long z,long xl,long zl,long xh,long zh)
{
	if ((x<xl) || (x>xh) || (z<zl) || (z>zh))
		return(0);
	else
		return(1);
}

//***************************************************************************
int OnCarrier(long PX, long PZ, long PY)
{
	int V[3];
	int M[9];
	int TM[9];
	int cbx1,cbx2,cbz1,cbz2;
	int px,py,pz;


	if (((labs(PX - carrier.x))<0x4fff) &&
		((labs(PZ - carrier.z))<0x4fff)) {
		V[0] = (PX - carrier.x);
		V[1] = (PY - carrier.deckht);
		V[2] = (PZ - carrier.z);

		MakeMatHPR(-carrier.heading,0,0,M);
		Trans3x3(M,TM);
		Rotate(V,TM);

		if (InBox(V[0],V[2],(int)carrier.cbx1,(int)carrier.cbz1,(int)carrier.cbx2,(int)carrier.cbz2)) {
			return(1);
		}
		}
	return(0);
}

//***************************************************************************
int GlideScopeCheck(long PX, long PZ, long PY)
{
	int x,y;
	long dx;
	long dy;
	long dz;
	int xoff;
	int V[3];
	int M[9];
	int TM[9];


	dx = labs(((carrier.x+carrier.wires[2].cx)-PX)/100L);
	dz = labs(((carrier.z+carrier.wires[2].cz)-PZ)/100L);

	dy = (((PY - (carrier.deckht))/100L)/4L);
	x = Dist2D((int)(dx),(int)(dz));
	y = ARCTAN((int)(dy),x);

#ifdef YEP
	dx = (((carrier.x+carrier.wires[2].cx)-PX)/100L);
	dz = (((carrier.z+carrier.wires[2].cz)-PZ)/100L);

	V[0] = (int)dx;
	V[1] = (PY - carrier.deckht);
	V[2] = (int)dz;

	MakeMatHPR(-carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);

	x = Dist2D(V[0],V[2]);
	xoff = ARCTAN(V[0],x);

	sprintf(Txt,"V[0]: %d V[2]: %d x: %d xoff: %d",V[0],V[2],x,xoff);
	Message(Txt);
#endif
	return(y);

}


//***************************************************************************
DisplayBall()
{
	int y;
	y = GlideScopeCheck(FPARAMS.X,FPARAMS.Z,FPARAMS.Y);

	if (y<=455) MeatBall=4;
	if ((y>455) && (y<545)) MeatBall=3;
	if ((y>545) && (y<728)) MeatBall=2;  /* Perfect */
	if ((y>728) && (y<819)) MeatBall=1;
	if (y>=819) MeatBall=0;
}


//***************************************************************************
int CarrierLanding()
{
   DoLSO();
}


//***************************************************************************
UpdateCarrierAnim()
{
	static int StrobeLight=0;
	static int tempflag=0;

	StrobeLight++;
	if (StrobeLight>8) StrobeLight=1;
	CarrierAnimVals[26]=StrobeLight;
	CarrierAnimVals[28]=LightsOn;


	// Cat #1
 	if (BlastShield[0] && (CarrierAnimVals[3]>(15*DEG))) {
		CarrierAnimVals[2]=1;
		CarrierAnimVals[3]-=(5*DEG);
		if (CarrierAnimVals[3]==(15*DEG)) CatReady[0]=1;
	}
	if ((!BlastShield[0]) && (CarrierAnimVals[3]<(90*DEG))) {
		CarrierAnimVals[3]+=(5*DEG);
		if (CarrierAnimVals[3]==(90*DEG)) {
			CarrierAnimVals[2]=0;
			CatReady[0]=0;
		}
	}

	// Cat #2
 	if (BlastShield[1] && (CarrierAnimVals[5]>(15*DEG))) {
		CarrierAnimVals[4]=1;
		CarrierAnimVals[5]-=(5*DEG);
		if (CarrierAnimVals[5]==(15*DEG)) CatReady[1]=1;
	}
	if ((!BlastShield[1]) && (CarrierAnimVals[5]<(90*DEG))) {
		CarrierAnimVals[5]+=(5*DEG);
		if (CarrierAnimVals[5]==(90*DEG)) {
			CarrierAnimVals[4]=0;
			CatReady[1]=0;
		}
	}

	// Cat #3
 	if (BlastShield[2] && (CarrierAnimVals[7]>(15*DEG))) {
		CarrierAnimVals[6]=1;
		CarrierAnimVals[7]-=(5*DEG);
		if (CarrierAnimVals[7]==(15*DEG)) CatReady[2]=1;
	}
	if ((!BlastShield[2]) && (CarrierAnimVals[7]<(90*DEG))) {
		CarrierAnimVals[7]+=(5*DEG);
		if (CarrierAnimVals[7]==(90*DEG)) {
			CarrierAnimVals[6]=0;
			CatReady[2]=0;
		}
	}

	// Cat #4
 	if (BlastShield[3] && (CarrierAnimVals[9]>(15*DEG))) {
		CarrierAnimVals[8]=1;
		CarrierAnimVals[9]-=(5*DEG);
		if (CarrierAnimVals[9]==(15*DEG)) CatReady[3]=1;
	}
	if ((!BlastShield[3]) && (CarrierAnimVals[9]<(90*DEG))) {
		CarrierAnimVals[9]+=(5*DEG);
		if (CarrierAnimVals[9]==(90*DEG)) {
			CarrierAnimVals[8]=0;
			CatReady[3]=0;
		}
	}


	CarrierAnimVals[10]=1;
	CarrierAnimVals[11]=1;

	CarrierAnimVals[24]=1;
	CarrierAnimVals[25]=0;

	CarrierAnimVals[18]=0;
	CarrierAnimVals[19]=0;
	CarrierAnimVals[20]=0;
	CarrierAnimVals[21]=0;
	CarrierAnimVals[22]=0;
	CarrierAnimVals[23]=0;

	switch(MeatBall) {
		case 0: CarrierAnimVals[18]=1;
				CarrierAnimVals[19]=1;
				break;
		case 1: CarrierAnimVals[19]=1;
				CarrierAnimVals[20]=1;
				break;
		case 2: CarrierAnimVals[20]=1;
				CarrierAnimVals[23]=1;
				break;
		case 3: CarrierAnimVals[23]=1;
				CarrierAnimVals[21]=1;
				break;
		case 4: CarrierAnimVals[21]=1;
				CarrierAnimVals[22]=1;
				break;
	}
}


//***************************************************************************
int KeepPlaneOnCarrier(int reqcat, int planenum)
{
	int V[3];
	int M[9];
	int TM[9];
	int cat;
	int basenum;
	int whichcarr;
	int ONDECKX[] = { 16, 14, 12};
	int ONDECKY[] = { 30, 52, 10};

	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;


	cat = reqcat;
	if(planenum == -1)
		basenum = whichourcarr;
	else
		basenum = (ps[planenum].base * -1) - 1;

	if(basenum != whichourcarr) {
		cat = -1;
		V[0] = 0;
		V[1] = (int)carrier.deckht;
		V[2] = 0;
	}
	else if(cat >= 0) {
		if(cat >= 4) {
			V[0] = ONDECKX[whichcarr] * 100;
			V[1] = (int)carrier.deckht;
			V[2] = ONDECKY[whichcarr] * 100;
		}
		else {
			V[0] = (int)carrier.catpult[cat].x;
			V[1] = (int)carrier.deckht;
			V[2] = (int)carrier.catpult[cat].z+350;
		}
	}
	else {
		V[0] = (int)ailoc[planenum].xloc;
		V[1] = (int)carrier.deckht;
		V[2] = (int)ailoc[planenum].yloc;
	}

	MakeMatHPR(carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);


	if(planenum == -1) {
		return(1);
		if (!WireCaught) {
		AirSpeed=0;
		OurAlt=(int)(carrier.deckht/100L);

		PxLng =((carrier.x+V[0])/100L);
		PyLng =0x100000 - ((carrier.z+V[2])/100L);
#ifdef YEP
		OurPitch=-carrier.pitch;
		OurRoll=-carrier.roll;
#endif
		OurHead = GetCatHeading(cat);
		InitScottFlight();
		THRUST=ThrustI=0;
		MinAlt = (int)(carrier.deckht/100L);
		RadarAlt = OurAlt;
		Alt = OurAlt>>2;
		}
	} else if(basenum == whichourcarr) {
		if(cat >= 0) {
			ailoc[planenum].xloc =(carrier.x+V[0]);
			ailoc[planenum].yloc =(carrier.z+V[2]);
			ps[planenum].head = GetCatHeading(cat);
		}
		else if(cat== -5) {
			if((int)ailoc[planenum].xloc > 0) {
				ps[planenum].head = GetCatHeading(2) - 0x4000;
			}
			else {
				ps[planenum].head = GetCatHeading(2) + 0x4000;
			}
		}
		ps[planenum].xL =((carrier.x+V[0])/100L);
		ps[planenum].yL =((carrier.z+V[2])/100L);
		ps[planenum].x = (ps[planenum].xL + 16)>>HTOV;
		ps[planenum].y = (ps[planenum].yL + 16)>>HTOV;

		ps[planenum].pitch=0;
		ps[planenum].roll=0;
		ps[planenum].alt = (int)((carrier.deckht/100L)>>2) + 2;
	}
	else {
		ps[planenum].xL =((ailoc[basenum].xloc+V[0])/100L);
		ps[planenum].yL =((ailoc[basenum].yloc+V[2])/100L);
		ps[planenum].x = (ps[planenum].xL + 16)>>HTOV;
		ps[planenum].y = (ps[planenum].yL + 16)>>HTOV;
		ps[planenum].head = ps[basenum].head;

		ps[planenum].pitch=0;
		ps[planenum].roll=0;
		ps[planenum].alt = (int)((carrier.deckht/100L)>>2) + 2;
	}
}

#ifdef YEP
//***************************************************************************
ils(RPS *rp,int ox,int oy,int color)
{
   int x,y,radius=22;      //   < 50 km              > runway length

   if (!(Status&WHEELSUP)&&(MODE_MASTER==MASTER_NAV)&&(CloseDist[0]<1281)&&(CloseDist[0]>100)) { // 50 kilometers*3281(km in feet)/32/4
	  x=angle(Rdrs[CloseBase[0]].x-Px,abs(Rdrs[CloseBase[0]].y-Py));
      if ((abs(x))<0x910) {                // within 10 degrees of ils beam?
         x=OurHead-angle(Rdrs[CloseBase[0]].x-Px,-(Rdrs[CloseBase[0]].y-Py)); // airport arrow indicator
         y=angle(Alt,(Dist2D(Rdrs[CloseBase[0]].x-Px,Rdrs[CloseBase[0]].y-Py)));
         x=min(radius,max(-radius,(   x/182)/2));
         y=min(radius,max(-radius,(50-y/182)/2));
		 y=scaley(y);
         DrawLine(rp,ox-x ,oy-10,ox-x ,oy+10,color); // bank indicator - vert line
         DrawLine(rp,ox-10,oy-y ,ox+10,oy-y ,color); // glide slope    - horz line
         return(x);
      }
   }
}
#endif


//***************************************************************************
ils(RPS *rp,int ox,int oy,int color)
{
	int x,y,radius=22;      //   < 50 km              > runway length
	int dx,dz;
	int ang;

	dx = (int)(((carrier.x+carrier.wires[2].cx)/100L) - PxLng);
	dz = abs((int)(((carrier.z+carrier.wires[2].cz)/100L) - (0x100000 - PyLng)));

	ang = angle(dx,dz);

	x=OurHead - ang;

//	x=min(radius,max(-radius,(x/182)/2));
	x=(x/50);

	DrawLine(rp,ox-x ,oy-15,ox-x ,oy+15,color); // bank indicator - vert line
}

//***************************************************************************
vdiils(RPS *rp,int ox,int oy,int color)
{
	int x,y,radius=22;      //   < 50 km              > runway length
	int dx,dz;
	int ang;

	dx = (int)(((carrier.x+carrier.wires[2].cx)/100L) - PxLng);
	dz = abs((int)(((carrier.z+carrier.wires[2].cz)/100L) - (0x100000 - PyLng)));

	ang = angle(dx,dz);

	x=OurHead - ang;

//	x=min(radius,max(-radius,(x/182)/2));
	x=(x/50);

	DrawLine(rp,ox-x ,oy-15,ox-x ,oy+15,color); // bank indicator - vert line
	DrawLine(rp,ox-(x+1), oy-15,ox-(x+1),oy+15,color);
	DrawLine(rp,ox-(x-1), oy-15,ox-(x-1),oy+15,color);
}
/***********************************************************/
/***********************************************************/
extern   int   DEBUG1;
extern   int   ONTHECAT;
extern   int   ONTHEHOOK;

OnTheDeck(long X, long Y, long Z )
{
  	int V[3];
  	int V2[3];
	int M[9];
	int TM[9];
	int x1,x2,z1,z2;
	int i;

   if ( (labs(X-carrier.x)>50000L) || (labs(Z-carrier.z)>50000L)|| (Y>12000) )
      return(0);

   V[0]= (int)((X-carrier.x)/10L);
   V[2]= (int)((Z-carrier.z)/10L);
   V[1]= 0;
	MakeMatHPR(-carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);
// detect tower return-1 if in tower tower is taller !!!
   if (InBox(V[0],V[2],(int)(carrier.twrx1/10L),(int)(carrier.twrz1/10L),
         (int)(carrier.twrx2/10L),(int)(carrier.twrz2/10L) )  )
      {
      return(-1);
      }
   if (Y>=7100) return(0);

   if ( InBox(V[0],V[2],(int)(carrier.cbx1/10L),(int)(carrier.cbz1/10L),
                        (int)(carrier.cbx2/10L),(int)(carrier.cbz2/10L) ) )
      {



		V[0]-=(carrier.wires[0].x1/10L);
		V[2]-=(carrier.wires[0].z1/10L);
		MakeMatHPR(carrier.wcheading,0,0,M);
		Rotate(V,M);
      x2= 200;
      z2= 120;

      if (InBox( V[0],V[2], 0,-4*z2, 4*x2,0 )
               &&(HookDown)
            &&(abs(OurHead-(carrier.heading+carrier.wcheading))<(20*182)) )
         {
         if (V[2]>-z2) return(2);
         if (V[2]>-(2*z2)) return(3);
         if (V[2]>-(3*z2)) return(4);
         return(5);
         }
      return(1);
      }
   return(0);
}

SetPosOnCat( long *X, long *Y, long *Z, int Cat, int Length)
{
   int   V[3];
   int   M[9];
   int   TM[9];


	V[0]=0;
	V[1]=0;
	V[2]=0;
	if(Length > 1) {
		V[2]=-(Length);
		MakeMatHPR(-carrier.catpult[Cat].heading,0,0,M);
		Rotate(V,M);
	}
	V[0] += (int)carrier.catpult[Cat].x;
	V[1] += (int)carrier.deckht;
	V[2] += (int)carrier.catpult[Cat].z+350;
	MakeMatHPR(carrier.heading,0,0,M);
	Trans3x3(M,TM);
	Rotate(V,TM);



   *(X)= (carrier.x+V[0]);
   *(Y)= (V[1]);
   *(Z)= (carrier.z+V[2]);
}

int   HorzHeave=0;
int   Heaving=0;
int HeavAngle=0;
int   HeaveDir=1;
int   CATVEL=0;

DoPlayerOnTheCat()
{
	long  X,Y,Z;

#ifdef YEP
	switch(StormLevel) {
		case 0:
			WaveHeight=2;
			WaveSpeed=120;
		break;
		case 1:
			WaveHeight=4;
			WaveSpeed=150;
		break;
		case 2:
			WaveHeight=7;
			WaveSpeed=170;
		break;
	}
	Heaving=ACosB(WaveHeight,HeavAngle);
	HeavAngle+=(WaveSpeed*Fticks);
	if (HeavAngle>0)
		HeavAngle+=(150*Fticks);
#endif
   HorzHeave=0;
   switch (View)
      {
      case FRONTAL:
      case DOWN:
      case FRONT:
      case PILOT:
      case PADLOCK:
      case RIGHT:
      case LEFT:
      case REARRIGHT:
      case REARLEFT:
      case STEADY:
      case POVVIEW:
      case MAGICCAM1:
         // is it us ???
         if ( (ONTHECAT||ONTHEHOOK) )
            HorzHeave= Heaving+2;
         break;
      case TOWERVIEW:
      case LSOVIEW:
         HorzHeave= Heaving;
         break;
      case WINGMAN:
      case MAGICCAM2:
      case EJECTVIEW:
         break;
      }

   if (ONTHECAT==0)
      {
      DoPlayerOnTheHook();
      return;
      }

   if (ONTHECAT==1)
      {
      // waiting for throttle up
      OurPitch=-1*182;
   	AirSpeed=Knots=CATVEL=0;
      if (ThrustI>=100) ONTHECAT=2;
      }
   else
      {
      CATVEL+= 30*Fticks;
      ONTHECAT+= (Fticks*(CATVEL/60));
      AirSpeed= ((CATVEL/16)<<4);

//      ((AirSpeed*Fticks)/40);
//      AirSpeed+= (10*Fticks);

      OurPitch=-3*182+ ((ONTHECAT>>3)&0xff);

      }

   SetPosOnCat(&X,&Y,&Z,PlayerCat,ONTHECAT);
	OurAlt=(int)(Y/100L);
	PxLng =(X/100L);
	PyLng =0x100000 - (Z/100L);
	OurHead = GetCatHeading(PlayerCat);
	InitScottFlight();
	MinAlt = (int)(carrier.deckht/100L);
	RadarAlt = OurAlt;
	Alt = OurAlt>>2;

	if (ONTHECAT>8900) {
		ONTHECAT= Heaving= 0;
		PlaneLaunched(PlayerCat,-1);
		GetNextLaunch(-1);
		PlayerCat = -999;
		LogIt (SLG_AIR_TAKEOFF, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
	}


   DEBUG1= ONTHECAT; //OnTheDeck( FPARAMS.X, FPARAMS.Y, FPARAMS.Z );
}

int   LandingAngle;
int   LandingVel;
int   TimeOnHook=0;
long   XSpot;
long   ZSpot;
int   WavedOff=0;
int   BallCalled=0;
int   LSOMSGTIME=0;



DoPlayerOnTheHook()
{
   int   V[3];
   int   M[9];
   int   D;
   long  WX,WY,WZ;

   if (ONTHEHOOK==0)
      {
      for (D=36;D<48;D++)
         CarrierAnimVals[D]=0;
      return;
      }


   if (TimeOnHook==0)
      {
      LandingVel=600;
      LandingAngle= OurHead;
      XSpot= (FPARAMS.X-carrier.x);
      ZSpot= (FPARAMS.Z-carrier.z);
      }

   if (  (TimeOnHook>(20*60))||((TimeOnHook>(3*60))&&(THRUST==0)) )
//   if (TimeOnHook>(10*60) )
      EndGame (ENDOFLOG);

   TimeOnHook+= Fticks;

   D= ( TimeOnHook< 100 )? -900:0;
   OurPitch-= MulDiv( OurPitch-D,Fticks,30 );
   OurRoll= 0;


   D= carrier.heading+carrier.wcheading;
   OurHead-= MulDiv( OurHead-D,Fticks,30);
   LandingVel-= MulDiv(400,Fticks,240);
   if (LandingVel<240) LandingVel=0;

   MakeMatHPR(OurHead,0,0,M);
   V[0]= V[1]=0;
   V[2]=-LandingVel;
   Rotate(V,M);
	XSpot += MulDiv( V[0],Fticks,60);
	ZSpot += MulDiv( V[2],Fticks,60);
	PxLng =(carrier.x+XSpot)/100L;
	PyLng =0x100000 - ((carrier.z+ZSpot)/100L);


	MinAlt=OurAlt=(int)(carrier.deckht/100L);
	RadarAlt = OurAlt;
	Alt = OurAlt>>2;
   AirSpeed= LandingVel;


// set the wire stretch here
   V[0]=0;
   V[1]=2;/*-2+8;*/
   V[2]=25;
   Rotate(V,M);
   WZ= V[2]+(int)(1*((ZSpot-carrier.wires[ONTHEHOOK-1].cz)/25L));
   WX= -V[0]+(int)(1*((XSpot-carrier.wires[ONTHEHOOK-1].cx)/25L));
   WY=V[1];

   if (WZ>0)
      {
      WZ=WX=WY=0;
      }

   CarrierAnimVals[36+3*(ONTHEHOOK-1)]=WX;
   CarrierAnimVals[37+3*(ONTHEHOOK-1)]=WY;
   CarrierAnimVals[38+3*(ONTHEHOOK-1)]=WZ;
   if (AnimVals[27]>(10*DEG))
      {
      AnimVals[27]=10*DEG;
      HookRot=10*DEG;

      }


//   DEBUG1= ONTHEHOOK;
}
//***************************************************************************
SlopeCheck( long X,long Y, long Z, int *HD, int *PTCH, int *DELHT, int *DST )
{
   long  dx;
   long  dy;
   long  dz;
   int   V[3];
   int   M[9];

// this does not work if carrier.heading !=0 !!!!!!!!

   dx= (X-(carrier.x+carrier.wires[2].cx))/100L;
   dz= (Z-(carrier.z+carrier.wires[2].cz))/100L;
   dy= (Y-6600)/100L;/*400L*/

   if ( (labs(dx)>8000)||(labs(dz)>8000) )
      {
      *(DST)= 0x7fff;
      return(1);
      }
   V[0]=(int)dx;
   V[1]=(int)dy;
   V[2]=(int)dz;
//  	MakeMatHPR(-(carrier.heading-carrier.wcheading),0,0,M);
//   Rotate(V,M);
   *(DST)=Dist2D((int)(dx),(int)(dz));
   *(HD)= ARCTAN(V[0],V[2])+carrier.wcheading;           // positive means to right of carrier
   *(PTCH)= ARCTAN(V[1],*(DST)*4)-(637); // 637 = 3.5*DEG
   *(DELHT)= V[1]- ASinB(*(DST)*4,637);

}



//***************************************************************************
SetGear(int G)
{
   if (ejected) return;
   switch (G)
      {
      case 1:     // lower gear
         if (!(Status&WHEELSUP) ) return;
         if (Knots>350) return;
         Status &= (~WHEELSUP);
         Message ("Landing gear lowered",RIO_MSG);
         break;
      case 0:     // raise gear
         if ((Status&WHEELSUP) ) return;
         if ( (Alt==MinAlt)||(Damaged&D_HYDRAULICS)  ) return;
         Status |= WHEELSUP;
         Message ("Landing gear raised",RIO_MSG);
         break;
      }
}
SetDLC(int DLC)
{
   DirectLiftControl= DLC;
}

SetHook(int Hook)
{
	if(HookDown==Hook) {
		return(1);
	}

	HookDown= Hook;
   if (Hook)
      Message ("Hook Lowered",RIO_NOW_MSG);
   else
      Message ("Hook Raised",RIO_NOW_MSG);
}

char debugstr[120]= {0};
extern	int	RUDDERVALS;

//#define MAXPAT 21
int   landpt=MAXPAT-1;
struct landpt{
   int   X;
   int   Z;
   int   HT;
   int   HEAD;
   int   PITCH;
   int   ROLL;
   int   KNTS;
   int   THRUST;
   int   GEAR;
   int   DLC;
   int   HOOK;
   int   TRIM;
   } LandPat[MAXPAT]={

          26,    110, 96,   10*DEG, -3*DEG,      0,   135,  40,1,1,1,100,
         214,   1104,354,  8*DEG+(DEG/2),-3*DEG+(DEG/4),0, 135,50,1,1,1,100,

//         0/4, 6150/4,360,    0*DEG,  0*DEG,  0*DEG,   135,  50,0,0,1,64,
      -450/4, 7350/4,400,  -20*DEG,  0*DEG, 20*DEG,   140,  50,0,0,1,64,
     -1500/4,10150/4,400,  -40*DEG,  0*DEG, 25*DEG,   150,  50,0,0,1,64,
     -3000/4,10300/4,450,  -90*DEG,  0*DEG, 25*DEG,   150,  50,0,0,1,64,
     -4650/4,10150/4,450, -100*DEG,  0*DEG, 25*DEG,   150,  50,0,0,1,64,
     -6200/4, 9300/4,500, -135*DEG,  0*DEG, 25*DEG,   170,  50,0,0,1,64,
     -7350/4, 7350/4,550, -150*DEG,  0*DEG, 25*DEG,   190,  50,0,0,1,64,
     -7650/4, 6150/4,600, -165*DEG,  0*DEG,  0*DEG,   200,  50,0,0,1,64,
     -7650/4, 4500/4,600, -180*DEG,  0*DEG,  0*DEG,   220,  50,0,0,1,64,
     -7650/4, 2850/4,600, -180*DEG,  0*DEG,  0*DEG,   250,  50,0,0,1,64,
     -7650/4,  600/4,600, -180*DEG,  0*DEG,  0*DEG,   260,  50,0,0,1,64,
     -7650/4, -450/4,600, -180*DEG,  0*DEG,  0*DEG,   280,  50,0,0,1,64,
     -7150/4,-2300/4,650, -200*DEG,  0*DEG, 30*DEG,   300,  50,0,0,1,64,
     -6000/4,-3750/4,650, -220*DEG,  0*DEG, 30*DEG,   325,  50,0,0,1,64,
     -4400/4,-4400/4,700, -240*DEG,  0*DEG, 30*DEG,   350,  50,0,0,1,64,
     -2500/4,-4650/4,700, -270*DEG,  0*DEG, 40*DEG,   375,  50,0,0,1,64,
     -1000/4,-4350/4,800, -300*DEG,  0*DEG, 45*DEG,   375,  50,0,0,1,64,
       100/4,-2850/4,800, -325*DEG,  0*DEG, 45*DEG,   400,  50,0,0,1,64,
       500/4, -800/4,800,    0*DEG,  0*DEG, 45*DEG,   400,  50,0,0,1,64,
       500/4,    0/4,800,    0*DEG,  0*DEG,  0*DEG,   400,  50,0,0,1,64



//           26,110,96,   10*DEG,-3*DEG,0, 135,40,1,1,1,100,
//        214,1104,354,    8*DEG+(DEG/2),-3*DEG+(DEG/4),0, 135,50,1,1,1,100,
//       -257+214,1104+257,450,  -90*DEG,-3*DEG,45*DEG, 130,50,1,1,1,100,
//      -1075,1112,525, -170*DEG,0,10*DEG, 130,50,1,1,1,100,
//         -1325,0,600,       180*DEG,0,0, 130,50,1,1,1,100,
//     -1325,-1500,800,       180*DEG,0,0, 220,50,1,0,1, 60,
//      -600,-2350,800,   90*DEG,0,45*DEG, 275,50,1,0,1, 40,
//       375,-1500,800,        0,0,45*DEG, 300,50,0,0,1, 30,
//        500,1350,800,         2*DEG,0,0, 325,50,0,0,1, 25,
//


      };

struct landpt NiteLandPat[MAXNITEPAT]={

          26,    110, 96,   10*DEG, -3*DEG,      0,   135,  40,1,1,1,100,          /*  0  */
         120,    607,225,   10*DEG,-3*DEG+(DEG/4),0, 135,50,1,1,1,100,             /*  1  */
         167,    855,290,   10*DEG,-3*DEG+(DEG/4),0, 135,50,1,1,1,100,             /*  2  */
         214,   1104,354,   10*DEG,-3*DEG+(DEG/4),0, 135,50,1,1,1,100,             /*  3  */
         520,   2208,708,   10*DEG,-3*DEG+(DEG/4),  0*DEG,   140,  50,1,0,1,64,    /*  4  */
      };

EasyLanding()
{
	int V[3];
	int M[9];
	int TM[9];
   int   H;
	struct landpt *usedlpattern;
	struct landpt *usedlpatstart;

   if (landpt<0) return;   // dont fly again

	if(!(Night || (MissionWeather == 2)))
		usedlpattern = usedlpatstart = LandPat;
	else {
		usedlpattern = usedlpatstart = NiteLandPat;
		if(landpt > (MAXNITEPAT - 1))
			landpt = MAXNITEPAT - 1;
	}


   ONTHECAT=ONTHEHOOK=TimeOnHook=0;

	WireCaught=0;

	usedlpattern = usedlpatstart + landpt;

#if 0
   V[0]=LandPat[landpt].X+(carrier.wires[2].cx/100L);
   V[1]=LandPat[landpt].HT;
   V[2]=LandPat[landpt].Z+(carrier.wires[2].cz/100L);
#else
   V[0]=usedlpattern->X+(carrier.wires[2].cx/100L);
   V[1]=usedlpattern->HT;
   V[2]=usedlpattern->Z+(carrier.wires[2].cz/100L);
//   TXT("X val ");
//   TXN(usedlpattern->X);
//   Message(Txt, DEBUG_MSG);
#endif
   H= carrier.heading;

   MakeMatHPR(H,0,0,M);
	Rotate(V,M);
	PxLng =(carrier.x/100L)+V[0];
	PyLng =(0x100000L - ((carrier.z/100L)+V[2]));
   OurAlt=V[1];

#if 0
	OurHead=-(carrier.heading+LandPat[landpt].HEAD);
	OurPitch=-LandPat[landpt].PITCH;
	OurRoll=-LandPat[landpt].ROLL;
#else
	OurHead=-(carrier.heading+usedlpattern->HEAD);
	OurPitch=-usedlpattern->PITCH;
	OurRoll=-usedlpattern->ROLL;
#endif

	OurPitch= -OurPitch;

#if 0
   AirSpeed= LandPat[landpt].KNTS*27;
   THRUST=ThrustI= LandPat[landpt].THRUST;
   SetGear(LandPat[landpt].GEAR);
   SetDLC(LandPat[landpt].DLC);
   SetHook(LandPat[landpt].HOOK);
   TRIM= LandPat[landpt].TRIM;
#else
   AirSpeed= usedlpattern->KNTS*27;
   THRUST=ThrustI= usedlpattern->THRUST;
   SetGear(usedlpattern->GEAR);
   SetDLC(usedlpattern->DLC);
   SetHook(usedlpattern->HOOK);
//   TRIM= usedlpattern->TRIM;
   TRIM = Get1GTrim(AirSpeed);
#endif
   RUDDERVALS=0;
	InitScottFlight();

   if (--landpt<0) landpt=MAXPAT-1; // change to landpt--;


}
char  Coursestr[30]={0};
char  Slopestr[30]={0};
char  Rangestr[30]={0};
char  Decentstr[40]={0};

int   lastlso=0;


char *lsomsgptrs[11]={
      "",
      "COME RIGHT",
      "COME LEFT",
      "REDUCE POWER",
      "ADD POWER",
      "YOU'RE FAST",
      "YOU'RE SLOW",
      "DROP YOUR GEAR",
      "DROP YOUR FLAPS",
      "DROP YOUR HOOK",
      "LEFT THE PATTERN-SEE YA"
};


DoLSO()
{
   int   DHEAD,DPTCH,DST,DELHT;
   long  DX,DY,DZ,j;
   char  str[120];
   char  str2[40];
   char *strptr;
   int   HT,LFT,CLMBRATE,WVOFF,i;

   if ( (ONTHEHOOK)||ONTHECAT )
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=0;
      return;
      }

   if ((landingstatus&LANDPLAYERNOW)==0) return;



   SlopeCheck( FPARAMS.X,FPARAMS.Y,FPARAMS.Z,
               &DHEAD,&DPTCH, &DELHT, &DST );

   if ( (DST<2000) )
      {
      WVOFF=0;
      HT= ASinB(4*DST,637)+66;
      LFT= ASinB(DST,DHEAD);
      CLMBRATE= (int)(FPARAMS.VEL[1]/10L);

      if (Knots>145) WVOFF=5;
      if (Knots<125) WVOFF=6;
      if ((Status&WHEELSUP)) WVOFF=7;
//      if (flaps==0) WVOFF=8;
      if (HookDown==0) WVOFF=9;

      if ((DST<300)&&(DST>100)&&(abs(DHEAD)<0x4000))
         {
         if (LFT>25) WVOFF=1;
         if (LFT<-25) WVOFF=2;
         if (DELHT>20) WVOFF=3;
         if (DELHT<-15) WVOFF=4;
         }
      if (DST>=300)
         {
         if (DST>500)
            {
            if (LFT>35) WVOFF=1;
            if (LFT<-35) WVOFF=2;
            if (DELHT>40) WVOFF=3;
            if (DELHT<-25) WVOFF=4;
            }
         else
            {
            if (LFT>25) WVOFF=1;
            if (LFT<-25) WVOFF=2;
            if (DELHT>20) WVOFF=3;
            if (DELHT<-15) WVOFF=4;
            }
         if ( (abs(DHEAD)>(10*DEG))||(abs(DPTCH)>(5*DEG))||(Knots>160) )
            {
            WVOFF=10;
            BallCalled=0;
            }
         else
            {
            WavedOff=0;
            if (BallCalled==0) {
//			   Speech(L_PADDLES);
//               Message("PADDLES CONTACT",RIO_MSG);
			   Speech(L_CALL_BALL);
               Message("CALL THE BALL",RIO_MSG);

			}
            BallCalled=1;
            }
         }

	if ((abs(DST) > 100) && (abs(DHEAD) > 0x4000)) {
		WVOFF = 0;
		BallCalled=0;
	}

      if ( (WVOFF&&(DST<300)) )
         {

         if (WavedOff==0)
            {
            strcpy(str,"WAVEOFF");
            strcat(str,lsomsgptrs[WVOFF]);
			Speech(L_WAVE_OFF);
            Message(str,RIO_NOW_MSG);

            }
         WavedOff=1;
//         BallCalled=0;
         }

      if ( (WavedOff==0)&&BallCalled )
         {
         if ( (LSOMSGTIME-=Fticks)<0)
            {
            LSOMSGTIME=0;
            if (WVOFF)
               {
               lastlso=WVOFF;

			   switch(WVOFF) {
				  case 1:
					Speech(L_COME_RIGHT);
				  break;
				  case 2:
					Speech(L_COME_LEFT);
				  break;
				  case 3:
					Speech(L_REDUCE_POWER);
				  break;
				  case 4:
					Speech(L_ADD_POWER);
				  break;
			   }

               Message(lsomsgptrs[WVOFF],RIO_NOW_MSG);
               LSOMSGTIME=(2*60);
               }
            else
               {
               switch (lastlso)
                  {
                  case 1:
                  case 2:
                     Message("ON CENTERLINE",RIO_MSG);
                     break;
                  case 3:
                  case 4:
                     Message("ON GLIDESLOPE",RIO_MSG);
                     break;
                  }
               lastlso=0;
               }

            }
         }
      }
   else
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=WavedOff=BallCalled=0;
      }

   if (BallCalled)
      {
// print the realtime telemetry from carrier outside view only
      sprintf(Rangestr,"%i FT OUT",4*DST);
      if (LFT>=0)
         sprintf(Coursestr,"COME LEFT %i FT",LFT);
      else
         sprintf(Coursestr,"COME RIGHT %i FT",-LFT);
      if (DELHT>=0)
         sprintf(Slopestr,"%i FT TOO HIGH",DELHT);
      else
         sprintf(Slopestr,"%i FT TOO LOW",-DELHT);
      sprintf(Decentstr,"DESCENT %i",-CLMBRATE*6);
      }
   else
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=0;
      }
}
#if 0
DoLSO()
{
   int   DHEAD,DPTCH,DST,DELHT;
   long  DX,DY,DZ,j;
   char  str[120];
   char  str2[40];
   char *strptr;
   int   HT,LFT,CLMBRATE,WVOFF,i;

   if ( (ONTHEHOOK)||ONTHECAT )
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=0;
      return;
      }

   SlopeCheck( FPARAMS.X,FPARAMS.Y,FPARAMS.Z,
               &DHEAD,&DPTCH, &DELHT, &DST );
   if ( (DST<2000) )
      {
      WVOFF=0;
      HT= ASinB(4*DST,637)+66;
      LFT= ASinB(DST,DHEAD);
      CLMBRATE= (int)(FPARAMS.VEL[1]/10L);
      if ((DST<300)&&(DST>100))
         {
         if ( (abs(LFT)>30)||(Knots>145)||(CLMBRATE<-1000)  ) WVOFF=1;
//         if (abs(OurHead-(carrier.heading+carrier.wcheading))>(10*DEG)) WVOFF=1;
         if ( (DPTCH<-DEG)||(DPTCH>(2*DEG)) ) WVOFF=1;
//         if ( (DELHT<-5)||(DELHT>5) ) WVOFF=1;
         }
      else
         {
         if (DST>100)
            {
            if ( (abs(DHEAD)>(10*DEG))||(abs(DPTCH)>(5*DEG)) )
               {
               WVOFF=1;
               }
            else
               {
               WavedOff=0;
               if (BallCalled==0)
                  Message("PADDLES CONTACT",RIO_MSG);
               BallCalled=1;
               }
            }
         }
      if (WVOFF)
         {
         if ( (WavedOff==0)&&BallCalled )
            {
            Message("WAVEOFF",RIO_MSG);
            }
         WavedOff=1;
         BallCalled=0;
         }

      }
   else
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=WavedOff=0;
      }

   if (BallCalled)
      {
      if ( (LSOMSGTIME-=Fticks)< 0)
         {
         strptr=NULL;
// print the advice from lso

         if (DirectLiftControl==0) strptr="DLC";
         if (Knots>140)
            strptr="YOU'RE FAST";
         if (Knots<125)
            strptr="YOU'RE SLOW";
         if ( (abs(DHEAD)>(DEG))&&(abs(LFT)>15))
            {
            if (DHEAD<0)
               strptr="RIGHT FOR LINEUP";
            else
               strptr="COME LEFT";
            lastlso=1;
            }
         if (abs(DPTCH)>(DEG))
            {
            if (DPTCH>0)
               strptr="YOU'RE HIGH";
            else
               strptr="YOU'RE LOW";
            lastlso=2;
            }
         if ((Status&WHEELSUP)) strptr="DROP YOUR GEAR";
//         if (flaps==0) strptr="DROP YOUR FLAPS";
         if (HookDown==0) strptr="DROP YOUR HOOK";

         if (strptr)
            {
            LSOMSGTIME= 1*60;
            Message(strptr,RIO_MSG);
            }
         else
            {
            LSOMSGTIME= 0;
            if (lastlso)
               {
               switch (lastlso)
                  {
                  case 1:
                     Message("ON CENTERLINE",RIO_MSG);
                     break;
                  case 2:
                     Message("ON GLIDESLOPE",RIO_MSG);
                     break;
                  }
               lastlso=0;
               }
            }
         }
// print the realtime telemetry from carrier outside view only
      sprintf(Rangestr,"%i FT OUT",4*DST);
      if (LFT>=0)
         sprintf(Coursestr,"COME LEFT %i FT",LFT);
      else
         sprintf(Coursestr,"COME RIGHT %i FT",-LFT);
      if (DELHT>=0)
         sprintf(Slopestr,"%i FT TOO HIGH",DELHT);
      else
         sprintf(Slopestr,"%i FT TOO LOW",-DELHT);
      sprintf(Decentstr,"DESCENT %i",-CLMBRATE*6);
      }
   else
      {
      Rangestr[0]=Coursestr[0]=Slopestr[0]=0;
      }
}
#endif


/******************************************************************/
void MoveAIToCat(int planenum, int cat)
{
	int xval, yval;
	int tx, ty;
	int bottomy, topy;
	long ty2;

	if((cat < 0) || (cat > 3))
		return;

	if (landingstatus & LANDNOTAKEOFF) {
		carrmoveok = 0;
	}

	xval = (int)carrier.catpult[cat].x/100;
	yval = ((int)carrier.catpult[cat].z+350)/100;
	tx = (int)ailoc[planenum].xloc/100;
	ty = (int)ailoc[planenum].yloc/100;

	if(ty > 62) {
		if(!carrmoveok)
			return;
	}

	bottomy = ((int)carrier.catpult[2].z/100) + 24;  /*  For AMERICA should be -30  */
	topy = ((int)carrier.catpult[0].z/100) + 20;     /*  For AMERICA should be 62  */
   	if(((ty > bottomy) && (cat > 1)) || ((ty < topy) && (cat < 2) && (tx >= 2))) {
		if(!carrmoveok)
			return;
		else
			carrmoveok = 0;

		if(tx != 2) {
				if(tx > 2) {
					if(TurnTo(planenum, GetCatHeading(2) - 0x4000))
						ailoc[planenum].xloc -= 100;
				}
				else if(tx < 2) {
					if(TurnTo(planenum, GetCatHeading(2) + 0x4000))
				 		ailoc[planenum].xloc += 100;
				}
		}
		else {
			if(cat < 2) {
				if(TurnTo(planenum, GetCatHeading(2) + 0x8000))
					ailoc[planenum].yloc += 100;
			}
			else {
				if(TurnTo(planenum, GetCatHeading(2)))
			   		ailoc[planenum].yloc -= 100;
			}
			if(landingstatus & (LANDNEAREND|LANDPLAYERNOW)) {
				ty2 = (int)ailoc[planenum].yloc/100;
			   	if(!(((ty2 > bottomy) && (cat > 1)) || ((ty2 < topy) && (cat < 2) && (tx >= 2)))) {
					landingstatus |= LANDNOTAKEOFF;
				}
			}
		}
		bv[planenum].behaveflag &= ~AI_INVISIBLE;
		return;
	}
	else if(tx < xval) {
		if(TurnTo(planenum, GetCatHeading(2) + 0x4000))
			ailoc[planenum].xloc += 100;
	}
	else if(tx > xval) {
		if(TurnTo(planenum, GetCatHeading(2) - 0x4000))
			ailoc[planenum].xloc -= 100;
	}
	else if(ty < yval) {
		if(TurnTo(planenum, GetCatHeading(2) + 0x8000)) {
			ailoc[planenum].yloc += 100;
			if((ty + 1) == yval) {
				PreparePlane(cat, planenum);
			}
		}
	}
	else if(ty > yval) {
		if(TurnTo(planenum, GetCatHeading(2))) {
			ailoc[planenum].yloc -= 100;
			if((ty - 1) == yval) {
				PreparePlane(cat, planenum);
			}
		}
	}
	else {
		if (ReadyLaunch(cat,planenum)) {
			bv[planenum].threathead = cat;
			bv[planenum].varcnt = Gtime + 2;
		}
	}

	bv[planenum].behaveflag &= ~AI_INVISIBLE;

	if(ty > 62) {
		carrmoveok = 0;
		if(((ailoc[planenum].yloc/100) <= 62L) && (landingstatus & (LANDNEAREND|LANDPLAYERNOW)))
			landingstatus |= LANDNOTAKEOFF;
	}
}


int TurnTo(int planenum, int heading)
{
	int thead, thead2;

	thead = heading - ps[planenum].head;

	thead2 = rng2 (thead, -0x600,0x600);
	ps[planenum].head += thead2;

	if(thead2 == thead)
		return(1);

	return(0);
}


