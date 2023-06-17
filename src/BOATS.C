/*  File:   boats.c                                 */
/*  Author: Scott Elson                             */
/*                                                  */
/*  Game logic for Fleet Defender: boat behaviors   */


#include	<stdio.h>
#include	<stdlib.h>
#include <bios.h>
#include <dos.h>
#include "library.h"
#include "planeai.h"

#include "world.h"
#include "behave.h"

#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "speech.h"
#include "armt.h"
#include "proto.h"
#include "setup.h"
#include "carr.h"

#define CHECKBASES	75

int whichourcarr;     // holds value for which boat our carrier
int boatnum;          // index of this boat in "ps" array
int     junktime = -999;

extern  int		friendfire;
extern  int 	enemyfire;
extern  int 	firstbehaveshow;
extern  UBYTE 	lastadded;
extern  long 	oldFPX;
extern  long 	oldFPZ;
extern  UWORD 	oldAltval;
extern  int     olddog;
extern  char    Txt[];
extern  COORD   xydist ();
extern  long    LMul   ();

extern  COORD   Px,Py;
extern  long    PxLng, PyLng;
extern  int     OurHead,OurRoll, OurPitch;
extern   UWORD       AirSpeed;
extern  SWORD	Knots;
extern  UWORD   Alt;
extern  int     Demo;
extern  int     FIRST_ENEMY_TRACE,DMAX9;
extern  int     TSpeed;

extern  int     Rtime;
extern  int     PovMisl;
extern  int     detected, atarg;
extern  COORD   xlast[],ylast[];
extern  int     zlast[];
extern  int     Rcolor;
extern  int     CloseBase[],CloseDist[];
extern  int     CloseGuy;
extern  int     Iflew;
extern  int     LastRate;
extern  int     REALFLIGHT;
extern  int     RunwayLeftX;
extern  RPS     *Rp3D;


extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern	struct PlaneData far *ptype;


extern    int   planecnt;			// used since somebody desided to make i static
							// and after the planes file grew to more than
							// 5000 lines I got sick of it and desided to
							// kludge something so I could split the file.
							// I kept I static in this file so wouldn't
							// infect something else

extern    int   pdest;      // pitch angle to plane's destination

extern    int   droll;      // delta roll this frame

extern    int AIWingMan;

extern	  int tempvar;
extern    int tempbehavevar;
extern    struct _path_ aipaths[];
extern    struct _wayPoint_ aiwaypoints[];
extern    struct _action_ aiactions[];
extern    int   PlanesLaunched;
extern	int	    Fticks;
extern  int     Speedy;

extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

extern carriers far carrier;
extern   int   ONTHECAT;
extern   int   ONTHEHOOK;

//***************************************************************************
void doboats()
{
	int i;
	int maxrollval;
	struct Planes *psi;  // pointer into "ps" structure array for this boat
	long cdx, cdy;
	int orgbasespeed;

    int z, n;

//	return;

    for (i=LASTPLANE, psi=(ps + LASTPLANE); i<MAXPLANE; i++, psi++) {
		boatnum = i;
		planecnt = i;

        if (ps[boatnum].status & ALIVE) {
			UpdateI(planecnt);

//            ptype = p + ps[boatnum].type;

			ptype = GetPlanePtr(ps[boatnum].type);


            if (ps[boatnum].status&ACTIVE) { /* Planes in the air */

				orgbasespeed = bv[i].basespeed;

#ifdef SHOWBEHAVIORS
				if((tempvar) && (i >= firstbehaveshow) && (i < (firstbehaveshow + 4)))
					ShowWhichBehave(i);
#endif

				if((Rtime & 7) == (i & 7))
	   				bv[i].Behavior();
				else
					droll = 0;

				if(bv[i].Behavior == AIGoHome)
					droll = 0;

				if(bv[i].behaveflag & DAMAGECONTROLSURFACE)  /*  Control Surfaces Damaged  */
					droll /= 2;

                droll = rng2(droll, -ptype->maneuver<<9, ptype->maneuver<<9);

//                if (ps[boatnum].status & CRASH) droll = 4<<8;

                ps[boatnum].status &= ~PANIC;

				if(ps[i].status & PITCHUP)
					bv[i].basespeed = orgbasespeed;

               	AvoidLand();

               	UpdateBoatHeadPitchRoll (i);

               	UpdateBoatSpeedAndPosition (i);

                if ((Rtime%(FrameRate*4) == (i&3)*FrameRate)) {
                    if (!(ps[boatnum].status&CRASH)) {
//	                    boatsearching ();
						search();
                    }
                }

				if((!(bv[i].behaveflag & ENEMYNOFIRE)) && (ptype->gun != 0)) {
					CheckBoatGunFiring();
				}

				if(i == whichourcarr) {
#if 0
					if(tempvar)
						bv[whichourcarr].verticlesep ++;
#endif
					cdx = carrier.x;
					cdy = carrier.z;
					UpdateOurCarrier(i);
					cdx = carrier.x - cdx;
					cdy = carrier.z - cdy;
					UpdateAIsOnCarrier(i, cdx, cdy);
				}

            } else {
				ps[i].speed = 0;
            }
        }
		else {
			ps[i].speed = 0;
		}
	}
	if(tempvar)
		firstbehaveshow += 4;
	if(firstbehaveshow >= MAXPLANE)
		firstbehaveshow = 0;
}

/*----------*/
void UpdateBoatHeadPitchRoll (int bnum)
{
    int n;
	int fdroll;
	int troll;


/* Update ROLL and HEADING */
//	fdroll = droll*4/FrameRate;
    ps[bnum].head += (droll>>3)/FrameRate;

	if(bnum == whichourcarr)
		ps[bnum].head = 0;

    if (abs(ps[bnum].pitch)>0x4000) {
        ps[bnum].head += 0x8000;
        ps[bnum].roll += 0x8000;
        ps[bnum].pitch = 0x8000 - ps[bnum].pitch;
    }
}

UpdateBoatSpeedAndPosition (int bnum)
{
    int a;
	long spd, Gspd;
	long dx, dy;
	long spd2;
	int speed2;
	long tspd;
	long precval;
	long precval2;

	if(((ailoc[bnum].xloc / AILOCDIV) != ps[bnum].xL) || ((ailoc[bnum].xloc / AILOCDIV) != ps[bnum].xL)) {
		ailoc[bnum].xloc = ps[bnum].xL * AILOCDIV;
		ailoc[bnum].yloc = ps[bnum].yL * AILOCDIV;
	}

	if(bv[bnum].behaveflag & DAMAGEENGINE) {
		if(bv[bnum].basespeed > (ptype->speed / 2))
			bv[bnum].basespeed = ptype->speed / 2;
	}

	if(bv[bnum].basespeed < 0)
		bv[bnum].basespeed = 0;

    spd = (long)((long)bv[bnum].basespeed*(0x8000-(ps[bnum].pitch/2))) >>15;
	ps[boatnum].speed = (int)spd;

	if(Speedy == 1) {
	    tspd = (((long)ps[bnum].speed*27 >>6) * Fticks * AILOCDIV) /60;
		spd = tspd;
    	speed2 = ((ps[bnum].speed*27 >>6) * Fticks) /60;
	    Gspd = cosXL (ps[bnum].pitch,spd);
    	ailoc[bnum].xloc += sinXL(ps[bnum].head,Gspd);
    	ailoc[bnum].yloc -= cosXL(ps[bnum].head,Gspd);
		ps[bnum].xL = ailoc[bnum].xloc / AILOCDIV;
		ps[bnum].yL = ailoc[bnum].yloc / AILOCDIV;
//	    ps[bnum].alt += sinX(ps[bnum].pitch,(spd/AILOCDIV));
	}
	else {
    	spd = (((long)ps[bnum].speed*27 >>6) * Fticks) /60;
	    Gspd = cosXL(ps[bnum].pitch,spd);
	    ps[bnum].xL += sinXL(ps[bnum].head,Gspd);
    	ps[bnum].yL -= cosXL(ps[bnum].head,Gspd);
//	    ps[bnum].alt += (int)sinXL(ps[bnum].pitch, spd);
	}

	ailoc[bnum].xloc = ((long)ps[bnum].xL * AILOCDIV) + (AILOCDIV>>1);
	ailoc[bnum].yloc = ((long)ps[bnum].yL * AILOCDIV) + (AILOCDIV>>1);
    ps[bnum].x = (ps[bnum].xL+16) >> HTOV;
    ps[bnum].y = (ps[bnum].yL+16) >> HTOV;

/* Trail smoke if CRASHing */
	if((bv[bnum].verticlesep) && (bv[bnum].varcnt > Gtime)) {
//	    if ((bv[bnum].verticlesep >= (Rtime>>4 & 3)) && (!(Rtime & 15))) {
	    if (!(Rtime & 15)){
			StartASmoke( ps[bnum].xL, ps[bnum].alt, ps[bnum].yL, CARRIERXPLO, 2 );
    	}
		if(!(Rtime & 31)) {
			StartASmoke( ps[bnum].xL, ps[bnum].alt, ps[bnum].yL, SMKPLUME, 35 );
		}
	}

}

//***************************************************************************
int IsBoat(int itemid)
{
	struct PlaneData far *tempplane;

	tempplane = GetPlanePtr(itemid);

	if ((tempplane->bclass == CARRIER_OBJCLASS) ||
	    (tempplane->bclass == SHIP_OBJCLASS)) {
		return (1);
	}
	return(0);
}

//***************************************************************************
void UpdateOurCarrier(int boatnum)
{
	int cnt;

	carrier.x = ailoc[boatnum].xloc;
	carrier.z = ailoc[boatnum].yloc;
	carrier.y = 0;

	if(!(landingstatus & LANDPLAYERWAIT)) {
		wps[0].xL = ailoc[boatnum].xloc/AILOCDIV;
		wps[0].yL = ailoc[boatnum].yloc/AILOCDIV;
		wps[0].x = wps[0].xL>>HTOV;
		wps[0].y = wps[0].yL>>HTOV;
	}

//	if((Knots == 0) && (Alt < 17)) {
#if 0
	if(ONTHECAT || ONTHEHOOK) {
		KeepPlaneOnCarrier(PlayerCat, -1);
	}
#endif
}

void UpdateAIsOnCarrier(int boatnum, long cdx, long cdy)
{
	int cnt;

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((ps[cnt].status & ALIVE) && (ps[cnt].status & ACTIVE) && (ps[cnt].base < 0) &&
				((bv[cnt].Behavior == Ready5) || (bv[cnt].Behavior == SARWait)
				|| ((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead <= -3)) || ((ps[cnt].status & TAKEOFF) && (ps[cnt].speed <= 1)))) {
#if 1  /*  (THIS LINE IS WRONG) THIS WON'T WORK FOR NORMAL GUYS WAITING ON DECK TO LAUNCH TO NEXT WAYPOINT  */
	   /*  WE NEED THIS SO THAT PLANES NOT ON CARRIER (IE WAYPOINT WON'T HAVE X == -1) WON'T  */
	   /*  BE PUT ON THE CARRIER  */
			if(bv[cnt].Behavior == AICarrierLand) {
				KeepPlaneOnCarrier(bv[cnt].threathead, cnt);
			}
			else if(bv[cnt].groundt >= 0) {
				if(aiwaypoints[bv[cnt].groundt].x == (long)-1) {
					KeepPlaneOnCarrier(bv[cnt].threathead, cnt);
				}
			}
			else if(bv[cnt].Behavior == SARCAP1) {
				KeepPlaneOnCarrier(bv[cnt].threathead, cnt);
			}
#else
		 	KeepPlaneOnCarrier(bv[cnt].threathead, cnt);
#endif
		}
		else if((ps[cnt].status & ALIVE) && (ps[cnt].status & ACTIVE)
				&& ((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead < 1))) {
	    	ailoc[cnt].xloc += cdx;
    		ailoc[cnt].yloc += cdy;
			ps[cnt].xL = ailoc[cnt].xloc / AILOCDIV;
			ps[cnt].yL = ailoc[cnt].yloc / AILOCDIV;
    		ps[cnt].x = (ps[cnt].xL+16) >> HTOV;
    		ps[cnt].y = (ps[cnt].yL+16) >> HTOV;
		}
	}
}

void CheckBoatGunFiring()
{
	int dbulpitch, dbulhead;
    int nn,tspd;
	int othertarget;
	int dxb, dyb, tpitchb, tangleb, enemyalt;
	COORD tdistb, tdistnm;
	long lx, ly;
	int ltangle;
	int dxl, dyl;
	int friendval;
	COORD tdistl;
	int tanglel, tpitchl;

	if(Rtime & 3)
		return;

	if(bv[boatnum].target == -1) {
		dxl = Px - ps[boatnum].x;
		dyl = Py - ps[boatnum].y;
		lx = FPARAMS.X - ailoc[boatnum].xloc;
		ly = FPARAMS.Z - ailoc[boatnum].yloc;
		friendval = FRIENDLY;
	}
	else {
		if(!(ps[bv[boatnum].target].status & ACTIVE))
			return;
		dxl = ps[bv[boatnum].target].x - ps[boatnum].x;
		dyl = ps[bv[boatnum].target].y - ps[boatnum].y;
		lx = ailoc[bv[boatnum].target].xloc - ailoc[boatnum].xloc;
		ly = ailoc[bv[boatnum].target].yloc - ailoc[boatnum].yloc;
		friendval = FriendStatus(bv[boatnum].target);
	}

	GetTargADP(boatnum, dxl, dyl, &tdistl, &tpitchl, &tanglel, &tdistnm, &enemyalt, 0);

	ltangle = Langle(lx, -ly);

	if(FriendMatchType(boatnum, friendval))
		return;

/*	if((boatnum != whichourcarr) && (tempvar)) {
		TXT("Other Boats guns ");
		TXN(tdistl);
		TXA(" T ");
		TXN(bv[boatnum].target);
		Message(Txt,DEBUG_MSG);
	}  */

    if (tdistl<24 && (!(bv[boatnum].behaveflag & ENEMYNOFIRE))) { // .5 nm
/*
		TXT("Carrier Guns ");
		TXN(bv[boatnum].target);
		Message(Txt,DEBUG_MSG);  */
        nn = ((Rtime>>2)&3)+FIRST_ENEMY_TRACE;
        if (CloseGuy) nn+=4;
        tspd = TSpeed;

//        trs[nn].dz = sinX (-(ps[boatnum].pitch + dbulpitch), tspd);
        trs[nn].dz = sinX (tpitchl, tspd);
        tspd       = cosX (tpitchl, tspd);
        trs[nn].dx = sinX (ltangle ,  tspd);
        trs[nn].dy = -cosX(ltangle,  tspd);

        trs[nn].x = ps[boatnum].xL;
        trs[nn].y = ps[boatnum].yL;
        trs[nn].z = ps[boatnum].alt;
		trs[nn].firedby = boatnum;
    }
}

void FormationSpeedAdjustBoat(int prespeed, int relangle, int tdist, int prewing)
{
	int temphead1;
	int temphead2;
	long junkthis;
	int oldbasespeed;
	int whatmaneuver;
	int i;
	struct PlaneData far *tempplane;

	i = planecnt;

	if(tdist > 15) {
		ps[i].status &= ~LOWDROLL;
	}

	tempplane = GetPlanePtr(ps[i].type);

	whatmaneuver = tempplane->maneuver;

//	whatmaneuver = p[ps[i].type].maneuver;

	oldbasespeed = bv[i].basespeed;
	if(abs(relangle) > 0x800) {
		if(tdist < 960) {
			if(tdist < 200)
				bv[i].basespeed = prespeed - ((tdist+ 4)>>3);
			else
				bv[i].basespeed = prespeed - 10;

			if(relangle < 0)
				droll = 0xE800 - ps[i].roll;
			else
				droll = 0x1800 - ps[i].roll;
		}
		else {
			bv[i].basespeed = prespeed;
		}

		if(bv[i].basespeed <= 0)
			bv[i].basespeed = 5;

		if(bv[i].basespeed > prespeed)
			bv[i].basespeed = prespeed;
	}
	else {
		if(tdist > 4) {
			if(tdist > 200) {
				bv[i].basespeed = ptype->speed + 30;  /*  was 250  */
			}
			else {
				bv[i].basespeed = prespeed + (tdist>>3);  /*  was 100  */
			}
		}
		else {
			bv[i].basespeed = prespeed;
		}
	}

#if 0
	if(tempvar)
	{
		TXT(" ");
		TXN(i);
		TXA(" Spd ");
		TXN(bv[i].basespeed);
		TXA(" pspd ");
		TXN(prespeed);
		TXA(", tdist ");
		TXN(tdist);
		TXA(", ra ");
		TXN(relangle);
		TXA(" s ");
		TXN((bv[i].behaveflag & 15));
		Message(Txt,DEBUG_MSG);
	}
#endif

	if(bv[i].basespeed > (ptype->speed + 30))
		bv[i].basespeed = (ptype->speed + 30);
}

void FormationBoat() {
	long dx, dy;
	int dz;
	int tx, ty;
	int relangle;
	int tangle, tpitch, troll, enemyalt, prewing, temphead, secplace;
	int prespeed;
	COORD tdist, distkm, tdistnm;
	int offangle;
	int ourleadalt;
	int dpwhead;  /*  is prewing turning alot  */
	int craps;
	long xval, yval;
	UWORD zval;
	int i;

	i = planecnt;

	prewing = GetSquadLeader(i);

	if(bv[i].horizontalsep < BOATDIST)
		bv[i].horizontalsep = BOATDIST;

	prewing = GetBehaviorLeader(i);

	secplace = (bv[i].behaveflag & 15);


	switch(secplace) {
		case 1:
			offangle = -0x1000;
			break;
		case 2:
			offangle = 0x1000;
			break;
		case 3:
			offangle = 0x2000;
			break;
		case 4:
			offangle = -0x2000;
			break;
		case 5:
			offangle = 0x6000;
			break;
		case 6:
			offangle = -0x6000;
			break;
		case 7:
			offangle = 0x4000;
			break;
		case 8:
			offangle = -0x4000;
			break;
		case 9:
			offangle = 0x8000;
			break;
		case 10:
			offangle = 0;
			break;
		case 11:
			offangle = 0x3000;
			break;
		default:
			offangle = -0x7000 + ((secplace - 8) * 0x2000);
			break;
	}

	prespeed = 0;
	if(prewing != i) {
		prespeed = bv[prewing].basespeed;
	}

	temphead = ps[prewing].head + offangle;

	if(i > prewing) {
		GetLastPosition (prewing, &dx, &dy, &ourleadalt);
	}
	else {
		dx = ailoc[prewing].xloc;
		dy = ailoc[prewing].yloc;
		ourleadalt = ps[prewing].alt;
	}

	dx = dx + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
	dy = dy - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));

	if(GLowDroll) {
		ps[i].status |= JUSTPUT;
		JustPutHere(i, dx, dy, ps[i].alt, prewing);
		return;
	}

	LFlyTo(dx, dy, ps[i].alt, prewing, prespeed);

#if 0 /* AAA  */
//#ifdef ELFBMSG
	if((tempvar) && (i == 3)) {
		TXT("FM ");
		TXN(i);
		TXA(" pw ");
		TXN(prewing);
		TXA(", ps ");
		TXN(bv[prewing].basespeed);
		TXA(", os ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "1h %x, lh %x, th %x, ti %d", ps[1].head, ps[prewing].head, temphead, prewing);
		Message(Txt,DEBUG_MSG);
/*		sprintf(Txt, "PW sx %d, lx %ld, nx %ld, sy %d, ly %ld, ny %ld", ps[prewing].x, ailoc[prewing].xloc, dx, ps[prewing].y, ailoc[prewing].yloc, dy);
		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "AI sx %d, lx %ld, nx %ld, sy %d, ly %ld, ny %ld", ps[i].x, ailoc[i].xloc, ps[prewing].xL, ps[i].y, ailoc[i].yloc, ps[prewing].yL);
		Message(Txt,DEBUG_MSG);  */
	}
#endif
}

void AvoidLand()
{

	if(Gtime < 3)
		return;

    if ((boatnum&7) == (Rtime&7)) {
		if(ps[boatnum].status & PITCHUP) {
			if(bv[boatnum].basespeed > 0) {
				if((boatnum&15) == (Rtime&15)) {
/*					TXT("Breaking ");
					TXN(bv[boatnum].basespeed);
					Message(Txt, DEBUG_MSG);  */
					bv[boatnum].basespeed -= 1;
				}
			}
			else
				bv[boatnum].basespeed = 0;
			return;
		}
		if(IsLand(ps[boatnum].xL, ps[boatnum].yL) != -1) {
			ps[boatnum].status |= PITCHUP;
			ChangeGroupBehavior(boatnum, 0);
/*			sprintf(Txt, "LAND HO %d X %ld, Y %ld, IL %d", boatnum, ps[boatnum].xL, ps[boatnum].yL, IsLand(ps[boatnum].xL, ps[boatnum].yL));
			Message(Txt,DEBUG_MSG);
			TXT("Leader ");
			TXN(GetSquadLeader(boatnum));
			Message(Txt,DEBUG_MSG);  */
		}
	}
	return;
}

