//***************************************************************************
//* File Name: carrai.c
//*
//* Author: M. McDonald
//*
//* Description:  	Carrier behaviors
//*
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

#define	DEG			182
#define	ANYCAT		-1
#define	NOCATSAVAIL	-1
#define MAXCATS		4

#define	WING_ANIM_STAT	0

extern  int     OurHead,OurRoll,OurPitch;
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
extern  UWORD   Region;
extern  int     THRUST;
extern	int		ThrustI;
extern  int     O[3][3];
extern	int		PlaneAnimVals[];

extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

/* Carrier specific var's */

/* Air Boss launch clearance */
int AvailCat[MAXCATS] = {-999,-999,-999,-999};
int BlastShield[MAXCATS] = {0,0,0,0};
int CatReady[MAXCATS] = {0,0,0,0};

int WingsReady=0;

extern int HookDown;

//***************************************************************************
int RequestCat(int cat, int planenum)
{
	int i;
	int catplane;

	if (cat==ANYCAT) {
		if(landingstatus & LANDREQUESTED)
			i=2;
		else
			i=0;
		while((AvailCat[i]!=-999) && (i<MAXCATS)) {
			i++;
		}
		if (i<MAXCATS) {
			AvailCat[i]=planenum;
			return(i);
		} else {
			if(planenum == -1) {
				catplane = AvailCat[2];
				AvailCat[2] = planenum;
				SetUpLaunch(catplane, bv[catplane].groundt);
#ifdef SRECATTEST
				TXT("Player replace ");
				TXN(catplane);
				Message(Txt);
#endif
				return(2);
			}
			return(-1);
		}
	} else {
		if ((cat>-1) && (cat<MAXCATS)) {
			if ((AvailCat[cat]==-999) && (!((cat >= 0) && (cat <=1) && (landingstatus & LANDREQUESTED)))) {
				AvailCat[cat]=planenum;
				return(cat);
			} else {
				if(planenum == -1) {
					if((cat <=1) && (landingstatus & LANDREQUESTED))
						cat = 2;
					catplane = AvailCat[cat];
					AvailCat[cat] = planenum;
					SetUpLaunch(catplane, bv[catplane].groundt);
#ifdef SRECATTEST
					TXT("Player replace ");
					TXN(catplane);
					Message(Txt);
#endif
					return(cat);
				}
				return(-1);
			}
		} else {
			return(-1);
		}
	}
}

//***************************************************************************
int PreparePlane(int cat, int PlaneId)
{
	//* Move plane into place - once into place set PlaneReady[cat] flag(1)

	/* Wings will now sweep back or fold down and blast shield will come up */
	/* little dudes should start doing there thing - cat should be put in */
	/* place */

	BlastShield[cat] = 1;
	return(1);
}

//***************************************************************************
int ReadyLaunch(int cat, int planeid)
{
//*	Check if animation is complete, if so return PLANE_READY
//* else return PLANE_NOTRED
	if (CatReady[cat])
		return(1);
	else
		return(0);
}

//***************************************************************************
int SaluteCatOfficer(int cat, int planeid)
{
//* Cat officer will do his dance, aircraft will pitch slightly and then
//* Launch
}

//***************************************************************************
int PlaneLaunched(int cat, int planeid)
{
//* Cat officer will do his dance, aircraft will pitch slightly and then
//* Launch
	if (OnTheDeck(FPARAMS.X, FPARAMS.Y, FPARAMS.Z))
		Sound(N_CatapultTakeOff,2,0);
	BlastShield[cat]=0;
	AvailCat[cat]=-999;
	if(cat <= 1) {
		landingstatus &= ~(LANDCAT0USED<<cat);
		LetNextPlaneLand();
	}
}

//***************************************************************************
LaunchAircraft()
{
#ifdef YEP
  	Knots=150;
	AirSpeed=300*27;
	ThrustI=THRUST=100;
	FPARAMS.VEL[2] = 350*100;
//	OurHead=(carrier.heading);
	OurHead=0;
	InitScottFlight();
#endif
	BlastShield[3]^=1;
}




