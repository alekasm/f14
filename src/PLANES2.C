/*  File:   Planes2.c                              */
/*  Author: Scott Elson                             */
/*                                                */
/*  Game logic for Fleet Defender: plane behaviors          */


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

extern int lastcatused;
int whichdefensethreat[MAXPLANE];

extern	int		DESIGNATED;
extern	int		LOCKED;
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

static    int   i;          // index of this plane in "ps" array
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
extern    long     AIBOMBX;     // 20-bit Scott-style X position of predicted Bomb impact
extern    long     AIBOMBY;     // 20-bit Scott-style Y position of predicted Bomb impact
extern    UWORD    AITTGT;      // 2 * Time to target in seconds
extern    UWORD    AITREL;      // 2 * Time to target intercept (bomb release) in seconds
extern    UWORD    AIRTGT;      // ground range to target in tenths of nautical miles
extern    int		 OtherAIDo;
extern    int		 OtherAITime;
extern    int		 WingID[];
extern struct _path_ aipaths[];
extern struct _wayPoint_ aiwaypoints[];
extern struct _action_ aiactions[];
extern    int   PlanesLaunched;
extern    int targetplayer;
extern    int timetargetplayer;
extern  int     whichourcarr;     // holds value for which boat our carrier
extern	int		LastCall;
extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);

extern struct PlaneData far *ptype;
extern confData ConfigData;



void SetUpWingman(int leadnum, int b, int numadd, int planeid)
{
	int cnt;
	int numflight = 1;
	int lastleader;
	int wingspace;
	int whichbehave;
	int radaron = 0;
	int enemynofire = 0;
	int lastsec;
	int updatecnt = 0;
	int imaboat = 0;
	int startval;
	int endval;
	int wingdist = 1;

	lastleader = leadnum;
	lastsec = leadnum;

	if(bv[leadnum].behaveflag & RADARON)
		radaron = 1;
	if(bv[leadnum].behaveflag & ENEMYNOFIRE)
		enemynofire = 1;

	if(leadnum == (NPLANES - 1))
		updatecnt = 1;

	if(IsBoat(planeid)) {
		imaboat = 1;
		startval = LASTPLANE - numadd;
		endval = MAXPLANE;
		wingdist = 10;
	}
	else {
		startval = leadnum + 1;
		endval = (leadnum + 1 + numadd);
	}

	if(bv[leadnum].Behavior == StraightAttack) {
		whichbehave = b % 4;
		if(whichbehave == 0) {
			bv[leadnum].Behavior = Box;
		}
		else if(whichbehave == 1) {
			bv[leadnum].Behavior = Ladder;
		}
		else if(whichbehave == 2) {
			bv[leadnum].Behavior = Wall;
		}
		else if(whichbehave == 3) {
			bv[leadnum].Behavior = Formation;
		}
	}

/*	if(tempbehavevar == 0) {
		bv[leadnum].Behavior = Formation;
		tempbehavevar ++;
	}
	else
		bv[leadnum].Behavior = Wall;  */

//	bv[leadnum].behaveflag += 1;   I have know idea why I was doing this SRE
    wingspace = (Direction==1 ? wingdist:-wingdist);
	for(cnt = startval; cnt<endval; cnt++) {
        PlanesLaunched++;
		if((cnt >= NPLANES) && (updatecnt))
			NPLANES = cnt + 1;

		if(imaboat && (LASTPLANE > cnt))
			LASTPLANE = cnt;

		if(ps[leadnum].status & FRIENDLY)
			ps[cnt].status |= FRIENDLY;
		else
			ps[cnt].status &= ~FRIENDLY;

		if(ps[leadnum].status & NEUTRAL)
			ps[cnt].status |= NEUTRAL;
		else
			ps[cnt].status &= ~NEUTRAL;

		if(ps[leadnum].status & CIVILIAN)
			ps[cnt].status |= CIVILIAN;
		else
			ps[cnt].status &= ~CIVILIAN;

        ActivatePlane (cnt, b, planeid);
		bv[cnt].Behavior = bv[leadnum].Behavior;  /*  was Formation  */
		if(radaron)
			bv[cnt].behaveflag |= RADARON;
		else
			bv[cnt].behaveflag &= ~RADARON;
		if(enemynofire)
			bv[cnt].behaveflag |= ENEMYNOFIRE;
		else
			bv[cnt].behaveflag &= ENEMYNOFIRE;

		if(bv[leadnum].behaveflag & ISHELICOPTER) {
			bv[cnt].behaveflag |= ISHELICOPTER;
		}

		if(ps[leadnum].mcnt[0] == 0)
			ps[cnt].mcnt[0] = 0;
		if(ps[leadnum].mcnt[1] == 0)
			ps[cnt].mcnt[1] = 0;
		if((numflight & 1) == 0) {
			bv[cnt].Behavior = bv[leadnum].Behavior;  /*  Was Straight Attack  */
			bv[cnt].target = bv[leadnum].target;  /*  lastleader for line formation  */

			bv[lastsec].nextsec = cnt;
			bv[cnt].prevsec = lastsec;
			bv[cnt].leader = -999;
			lastleader = cnt;
			lastsec = cnt;
		}
		else {
			bv[cnt].leader = lastleader;
			bv[lastleader].wingman = cnt;
			lastleader = cnt;
		}

		ps[cnt].x = ps[leadnum].x;
		ps[cnt].xL = ps[leadnum].xL;
		if(ps[leadnum].xL <= (long)-1) {
			ps[cnt].y = ps[leadnum].y;
			ps[cnt].yL = ps[leadnum].yL;
		}
		else {
			ps[cnt].y = ps[leadnum].y + (numflight * wingspace);
			ps[cnt].yL = (long) ps[cnt].y<<HTOV;
		}
		ps[cnt].head = ps[leadnum].head;
		ps[cnt].speed = ps[leadnum].speed;
		bv[cnt].basespeed = bv[leadnum].basespeed;
		bv[cnt].behaveflag += numflight + 1;
		bv[cnt].path = bv[leadnum].path;
		bv[cnt].groundt = bv[leadnum].groundt;
		bv[cnt].xloc = bv[leadnum].xloc;
   		bv[cnt].yloc = bv[leadnum].yloc;

		bv[cnt].horizontalsep = bv[leadnum].horizontalsep;
		bv[cnt].threathead = bv[leadnum].threathead;
		if(bv[leadnum].behaveflag & COMBAT_SPREAD){
			bv[cnt].behaveflag |= COMBAT_SPREAD;
		}
		else {
			bv[cnt].behaveflag &= ~COMBAT_SPREAD;
		}

		ps[cnt].alt = ps[leadnum].alt;
		if(!(ps[leadnum].status & TAKEOFF)) {
			ps[cnt].status &= ~TAKEOFF;
			if(ps[cnt].xL > (long)-1) {
				ps[cnt].y += 5 * (numflight * wingspace);  /*  add separation since there is no takeoff delay  */
				ps[cnt].yL = (long) ps[cnt].y<<HTOV;
			}
			else {
				ps[cnt].alt += (cnt - leadnum) * 20;
			}
			bv[cnt].varcnt = 0;
			if(bv[cnt].basespeed == 0)
				bv[cnt].basespeed = 1;
		}
		else {
			bv[cnt].varcnt = Gtime + (4 * numflight);
			bv[cnt].basespeed = 1;
		}

		bv[cnt].orgnum = bv[leadnum].orgnum + numflight;
		if(bv[leadnum].Behavior == Ready5) {
			bv[cnt].Behavior = Ready5;
			SetUpLaunch(cnt, bv[cnt].groundt);
//			ps[cnt].status &= ~TAKEOFF;
		}
		else if(ps[leadnum].status & TAKEOFF) {
			SetUpLaunch(cnt, bv[cnt].groundt);
		}

		numflight ++;
		if(numflight > numadd)
			break;
	}
}

void UpdateWing()
{
	int bvl;

	i = planecnt;
	if(bv[i].leader >= 0) {
		if((ps[bv[i].leader].status & CRASH) || (!((ps[bv[i].leader].status & ALIVE) && (ps[bv[i].leader].status & ACTIVE)))) {
			bvl = bv[i].leader;
#ifdef ELFDMSG
			TXT("Replace Wing ");
			TXN(bvl);
			TXA(" with ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
//			bv[i].Behavior = bv[bvl].Behavior;
			bv[i].leader = bv[bvl].leader;
			bv[i].prevsec = bv[bvl].prevsec;
			bv[i].nextsec = bv[bvl].nextsec;
			if(bv[i].prevsec >=0)
				bv[bv[i].prevsec].nextsec = i;
			if(bv[bvl].nextsec >= 0)
				bv[bv[bvl].nextsec].prevsec = i;
			bv[i].behaveflag = bv[bvl].behaveflag;
			bv[bvl].behaveflag = 0;
			bv[bvl].wingman = -999;
			bv[bvl].leader = -999;
			bv[bvl].prevsec = -999;
			bv[bvl].nextsec = -999;
			return;
/*
		DO CODE HERE TO HANDLE WHEN A PLANE'S WINGMAN IS SHOT DOWN
		AND HE NEEDS TO FORM UP WITH ANOTHER WINGMAN
 			if((bv[i].leader == -999) && (bv[i].wingman == -999))
			{
				if(bv[bv[i].
				if(bv[bv[i].prevsec].wingman == -999) {
					bv[bv[i].prevsec].wingman == i;
					bv[i].leader = bv[i].prevsec;
					bv[i].prevsec = -999;
					bv[bv[i].leader].nextsec = bv[i].nextsec;
					bv[i].nextsec = -999;
				}
				else if (bv[bv[i].nextsec].leader == -999) {
					bv[bv[i].nextsec].wingman == i;
					bv[i].leader = bv[i].prevsec;
					bv[i].prevsec = -999;
					bv[bv[i].leader].nextsec = bv[i].nextsec;
					bv[i].nextsec = -999;
				}
			}  */
		}
	}
	if(bv[i].wingman >= 0) {
		if((ps[bv[i].wingman].status & CRASH) || (!((ps[bv[i].wingman].status & ALIVE) && (ps[bv[i].wingman].status & ACTIVE)))) {
#ifdef ELFDMSG
			TXT("Remove Wing ");
			TXN(bv[i].wingman);
			TXA(" From ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			bv[bv[i].wingman].leader = -999;
			bv[i].wingman = bv[bv[i].wingman].wingman;
			if(bv[i].wingman >= 0)
				bv[bv[i].wingman].leader = i;
		}
	}
	if((bv[i].prevsec >= 0) && (bv[bv[i].prevsec].wingman < 0)) {
		if((ps[bv[i].prevsec].status & CRASH) || (!((ps[bv[i].prevsec].status & ALIVE) && (ps[bv[i].prevsec].status & ACTIVE)))) {
			bvl = bv[i].prevsec;
#ifdef ELFDMSG
			TXT("Replace Leader ");
			TXN(bvl);
			TXA(" with ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
//			bv[i].Behavior = bv[bvl].Behavior;
			bv[i].prevsec = bv[bvl].prevsec;
			if(bv[i].prevsec >= 0)
				bv[bv[i].prevsec].nextsec = i;
			bv[i].behaveflag = bv[bvl].behaveflag;
			bv[bvl].prevsec = -999;
			bv[bvl].nextsec = -999;
			return;
		}
	}
	if((bv[i].nextsec >= 0) && (bv[bv[i].nextsec].wingman < 0)) {
		if((ps[bv[i].nextsec].status & CRASH) || (!((ps[bv[i].nextsec].status & ALIVE) && (ps[bv[i].nextsec].status & ACTIVE)))) {
#ifdef ELFDMSG
			TXT("Remove Follow ");
			TXN(bv[i].nextsec);
			TXA(" From ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			bv[bv[i].nextsec].prevsec = -999;
			bv[i].nextsec = bv[bv[i].nextsec].nextsec;
			if(bv[i].nextsec >= 0)
				bv[bv[i].nextsec].prevsec = i;
		}
	}
}

int GetSquadLeader(int planenum)
{
	int prewing;
	prewing = GetLeaderCore(planenum, 1);

	if((bv[prewing].leader == -1) || (bv[prewing].prevsec == -1)) {
		prewing = -1;
		return(prewing);
	}
}

//***********************************************************************
//  int GetBehaviorLeader(int planenum)
//
//  This function will return the first wingnum above plane num that
//  is either the section leader, or has a different behavior
//  This is so that you can have different behaviors leading a
//  bunch of planes, but have the other planes flying in different
//  formation.
//                                 SRE  6/22/93
//**********************************************************************
int GetBehaviorLeader(int planenum)
{
	return(GetLeaderCore(planenum, 0));
}

int GetLeaderCore(int planenum, int nobehave)
{
	int prewing;
	int iamdone;
	int otherplane;

	if(planenum == -1)
		return(-1);

	prewing = planenum;

	if(bv[prewing].leader > -1) {
		otherplane = bv[prewing].leader;
		if((bv[otherplane].Behavior == bv[planenum].Behavior) || (nobehave)) {
			iamdone = 0;
			prewing = otherplane;
			while((bv[prewing].leader > -1) && (!iamdone)) {
				otherplane = bv[prewing].leader;
				if((bv[otherplane].Behavior == bv[planenum].Behavior) || (nobehave))
					prewing = bv[prewing].leader;
				else
					return(otherplane);
			}
		}
		else
			return(otherplane);
	}
	if (bv[prewing].prevsec > -1) {
		otherplane = bv[prewing].prevsec;
		if((bv[otherplane].Behavior == bv[planenum].Behavior) || (nobehave)) {
			iamdone = 0;
			prewing = otherplane;
			while((bv[prewing].prevsec > -1) && (!iamdone)) {
				otherplane = bv[prewing].prevsec;
				if((bv[otherplane].Behavior == bv[planenum].Behavior) || (nobehave))
					prewing = bv[prewing].prevsec;
				else
					return(otherplane);
			}
		}
		return(otherplane);
	}

	return(prewing);
}

void CloseInFight(int tanglel, int wingnum, int leadernum)
{
	int ranglel, rranglel;

	i = planecnt;

	ranglel = tanglel - ps[i].head;

	if(bv[i].target == -1) {
		rranglel = (0x8000 + tanglel) - OurHead;
	}
	else {
		rranglel = (0x8000 + tanglel) - ps[bv[i].target].head;
	}

	if(CheckDetection(i) != -999)
	{
		bv[i].Behavior = Jink2;
		if(wingnum > -1) {
			if(bv[wingnum].Behavior == FreeFighter) {
				bv[wingnum].Behavior = StraightAttack;
			}
		}
		if(leadernum > -1) {
			if(bv[leadernum].Behavior == FreeFighter) {
				bv[leadernum].Behavior = StraightAttack;
			}
		}
	}
	else if((abs(ranglel) > 0x5000) && (abs(rranglel) < 0x2800)){
		bv[i].Behavior = Jink;
	}
	else {
		if(bv[i].Behavior == StraightAttack) {
			if(abs(ranglel) > 0x2000) {
				if(wingnum > -1) {
					if(bv[wingnum].Behavior == FreeFighter) {
#ifdef ELFDMSG
						TXT("Wingman Attack ");
						TXN(wingnum);
						Message(Txt,DEBUG_MSG);
#endif
						bv[wingnum].Behavior = StraightAttack;
					}
				}
				if(leadernum > -1) {
					if(bv[leadernum].Behavior == FreeFighter) {
#ifdef ELFDMSG
						TXT("Leader Attack ");
						TXN(leadernum);
						Message(Txt,DEBUG_MSG);
#endif
						bv[leadernum].Behavior = StraightAttack;
					}
				}
			}
			else if(abs(ranglel) <= 0x2000) {
				if(wingnum > -1) {
					if(bv[wingnum].Behavior == StraightAttack) {
#ifdef ELFDMSG
						TXT("Wingman Free ");
						TXN(wingnum);
						Message(Txt,DEBUG_MSG);
#endif
						bv[wingnum].Behavior = FreeFighter;
					}
				}
				if(leadernum > -1) {
					if(bv[leadernum].Behavior == StraightAttack) {
#ifdef ELFDMSG
						TXT("Leader Free ");
						TXN(leadernum);
						Message(Txt,DEBUG_MSG);
#endif
						bv[leadernum].Behavior = FreeFighter;
					}
				}
			}
		}
		else if(bv[i].Behavior == FreeFighter) {
			if(wingnum > -1) {
				if(bv[wingnum].Behavior == StraightAttack) {
					bv[i].target = bv[wingnum].target;
				}
			}
			if(leadernum > -1) {
				if(bv[leadernum].Behavior == StraightAttack) {
					bv[i].target = bv[leadernum].target;
				}
			}
		}
		else if(bv[i].Behavior != FreeFighter) {
			bv[i].Behavior = StraightAttack;
		}
	}
}

int CheckWingmanSix(int wingnum)
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1;

	i = planecnt;

	if(wingnum == -1) {
		x = Px;
		y = Py;
		winghead = OurHead;
	}
	else {
		x = ps[wingnum].x;
		y = ps[wingnum].y;
		winghead = ps[wingnum].head;
	}

	founddist = 0;
	targetnum = -999;
	tdist = 0;
	if((!(FriendMatchType(planecnt, FRIENDLY))) && (LOCKED == wingnum)) {
    	dx = x- Px;
    	dy = y- Py;
		GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
		rangle = tangle - OurHead;
		rrangle = (0x8000 + tangle) - winghead;
		if((tdistnm < 8) && (abs(rangle) < 0x2000) && (abs(rrangle) > 0x4000)) {
			targetnum = -1;
			rangle1 = rangle;
			rrangle1 = rrangle;
			tdistnm1 = tdistnm;
			tdist = tdist1;
			founddist = 1;
		}
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & (CRASH|CIVILIAN))) &&
				(ps[cnt].speed) && (bv[cnt].target == wingnum)) { /* Planes in the air */
			if(!FriendMatch(wingnum, cnt)) {
		    	dx = x- ps[cnt].x;
    			dy = y- ps[cnt].y;
				GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
				rangle = tangle - ps[cnt].head;
				rrangle = (0x8000 + tangle) - winghead;
       			if ((((tdist1<tdist) || ((tdistnm < 8) && (!founddist))) && (abs(rangle) < 0x2000)
						&& (abs(rrangle) > 0x4000))) {
					founddist = 1;
           			tdist=tdist1;
					targetnum = cnt;
					rangle1 = rangle;
					rrangle1 = rrangle;
					tdistnm1 = tdistnm;
       			}
			}
		}
	}
	if(founddist) {
		if((!(bv[i].behaveflag & NO_SWITCH_TARGET)) && (bv[i].leader == -1) &&
				(bv[i].target != targetnum)) {
			GetWingName(i, Txt);
			TXA("Engaging Rear Bogey ");
//			TXN(tdist);
			Message(Txt,WINGMAN_MSG);
		}
		bv[i].target = targetnum;
		bv[i].Behavior = StraightAttack;
		bv[i].behaveflag |= NO_SWITCH_TARGET;
		return(founddist);
	}

//	bv[i].behaveflag &= ~NO_SWITCH_TARGET;  Don't think I should do this.  I think it undoes Player Helping Us
	return(0);
}

void CheckAITarget(int ainum)
{
	int targetdead = 0;
	int orgt, preorgt, prenewt;

	i = planecnt;

	orgt = bv[ainum].target;

	if(orgt != -1) {
		if ((ps[orgt].status & (CRASH|CIVILIAN)) || (!(ps[orgt].status & ALIVE)) || (!(ps[orgt].status&ACTIVE))) {
			targetdead = 1;
		}
		if(orgt == ainum) {
			targetdead = 1;
		}
	}

	if((bv[ainum].Behavior == StraightAttack) || (targetdead) || ((!(bv[ainum].behaveflag & ENEMYNOFIRE)) && (ImNotaFighter(ainum, 1)))) {
		if(((Rtime & 31) == (ainum & 31))) {
			GetEnemyTarget(ainum);
		}
		else {
			if(targetdead){
				bv[ainum].target = -1;
				GetEnemyTarget(ainum);
			}
		}
	}

#ifdef ELFTARGSWITCH
	if(ainum == GetSquadLeader(ainum)) {
		preorgt = GetSquadLeader(orgt);
		prenewt = GetSquadLeader(bv[ainum].target);

		if(preorgt != prenewt) {
			ps[prenewt].color = ainum;
			TXT("Target Change ");
			TXN(ainum);
			TXA(" from ");
			TXN(orgt);
			TXA(" to ");
			TXN(bv[ainum].target);
			TXA(" po ");
			TXN(preorgt);
			TXA(" pn ");
			TXN(prenewt);
			Message(Txt,DEBUG_MSG);
		}
	}
#endif

}

void FormationSpeedAdjustUS(int prespeed, int relangle, int tdist, int prewing)
{
	int temphead1;
	int temphead2;
	long junkthis;
	int oldbasespeed;
	int whatmaneuver;
	int speed1, speed2, speed3;
	int ppspd;
	struct PlaneData far *tempplane;

	i = planecnt;

#if 0
	if(tdist > 15) {
		ps[i].status &= ~LOWDROLL;
	}
#endif

	if(IsBoat(ps[planecnt].type)) {
		FormationSpeedAdjustBoat(prespeed, relangle, tdist, prewing);
		return;
	}

	if(prewing != -1) {
		if(ps[i].type != ps[prewing].type) {
			tempplane = ptype;
			ptype = GetPlanePtr(ps[prewing].type);
			prespeed -= GetDAltSpeed(ps[prewing].alt);
			ptype = tempplane;
			prespeed += GetDAltSpeed(ps[prewing].alt);
		}
	}

	tempplane = GetPlanePtr(ps[i].type);
	whatmaneuver = tempplane->maneuver;

	oldbasespeed = bv[i].basespeed;
	if((abs(relangle) > 0x4000) || ((abs(relangle) > 0x2000) && (whatmaneuver < 2))) {
		if(tdist < 960) {
			if(tdist < 156)
				if(prewing == -1)
					bv[i].basespeed = prespeed - (tdist+ 44);
				else
					bv[i].basespeed = prespeed - (tdist+ 4);
			else
				bv[i].basespeed = prespeed - 160;
			if(relangle < 0)
				droll = 0xE800 - ps[i].roll;
			else
				droll = 0x1800 - ps[i].roll;
		}
		else {
			if(prespeed > 400)
				bv[i].basespeed = prespeed - 200;  /*  was 200  */
			else if(prespeed > 200)
				bv[i].basespeed = 200;
			else
				bv[i].basespeed = prespeed;
		}
	}
	else if (prewing == -1){
		if(tdist > 4) {
			if(tdist > 3200) {
				bv[i].basespeed = ptype->speed;  /*  was 250  */
			}
			else if(tdist > 1000) {  /*  was 480  */
				bv[i].basespeed = prespeed + 200;  /*  was 350 150  */
			}
			else if(tdist > 25) {  /*  was 480  */
//				bv[i].basespeed = prespeed + 25;
				bv[i].basespeed = prespeed + 5;  /*  was 350 150  */
			}
			else {
				bv[i].basespeed = prespeed + (tdist - 45);  /*  was 100  */
			}
		}
		else {
			bv[i].basespeed = prespeed + (tdist - 45);  /*  was 20  */
		}
	}
	else {
		if(tdist > 4) {
			if(tdist > 3200) {
				bv[i].basespeed = ptype->speed;  /*  was 250  */
			}
			else if(tdist > 1000) {  /*  was 480  */
				bv[i].basespeed = prespeed + 200;  /*  was 350 150  */
			}
			else if(tdist > 25) {  /*  was 480  */
				bv[i].basespeed = prespeed + 25;  /*  was 350 150  */
			}
			else {
				bv[i].basespeed = prespeed + tdist;  /*  was 100  */
			}
		}
		else {
			bv[i].basespeed = prespeed;
		}
	}
/*	if(abs(bv[i].basespeed - oldbasespeed) < 60) {
		if(bv[i].basespeed < oldbasespeed)
			bv[i].basespeed = oldbasespeed - 1;
		else if(bv[i].basespeed > oldbasespeed)
			bv[i].basespeed = oldbasespeed + 1;
	}  */

#if 1
//	if((tempvar) && (i == 3))
//	if((tempvar) && (ptype->PlaneEnumId == SO_TU_16GB))
//	if((tempvar) && ((i == 17) || (i == 18)))
	if((tempvar) && (i == 62))
	{
		TXT(" ");
		TXN(i);
		TXA(" Speed ");
		TXN(bv[i].basespeed);
		TXA(" prespeed ");
		TXN(prespeed);
		TXA(", tdist ");
		TXN(tdist);
		TXA(", ra ");
		TXN(relangle);
		TXA(", PE ");
		TXN(ptype->PlaneEnumId);
		Message(Txt,DEBUG_MSG);
	}
#endif

	if(bv[i].basespeed < 150) {
		if(!(bv[i].behaveflag & ISHELICOPTER)) {
			bv[i].basespeed = 150;
		}
		else if(bv[i].basespeed < 0) {
			bv[i].basespeed = 0;
		}
	}

	if(bv[i].basespeed > ptype->speed)
		bv[i].basespeed = ptype->speed;
}

void Jink()
{
	COORD distkm, tdist, tdistnm;
	int relangle, dojinkangle;
	int dx, dy, tpitch, tangle;
	int enemyroll, enemyhead, enemyalt, enemypitch, e2uspitch;
	int enemynum;
	int targetspeed;

	i = planecnt;

	enemynum = CheckDetection(i);
	if(enemynum == -999)
		enemynum = bv[i].target;

	if(enemynum == -1) {
		dx = Px - ps[i].x;
		dy = Py - ps[i].y;
		enemyroll = OurRoll;
		enemyhead = OurHead;
		enemypitch = OurPitch;
		targetspeed = Knots;
	}
	else {
		dx = ps[enemynum].x - ps[i].x;
		dy = ps[enemynum].y - ps[i].y;
		enemyroll = ps[enemynum].roll;
		enemyhead = ps[enemynum].head;
		enemypitch = ps[enemynum].pitch;
		targetspeed = bv[enemynum].basespeed;
	}
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
	relangle = tangle - ps[i].head;
	dojinkangle = (0x8000 + tangle) - enemyhead;
	e2uspitch = -tpitch;

	if((abs(enemyroll) < 0x1800) && (abs(enemyroll) < abs(ps[i].roll))) {
		if(ps[i].roll < 0) {
			droll = 0xC000 - ps[i].roll;
		}
		else {
			droll = 0x4000 - ps[i].roll;
		}
	}
	else if(enemyroll > 0) {
		droll = 0xC000 - ps[i].roll;
	}
	else{
		droll = 0x4000 - ps[i].roll;
	}

#if 0
	if(tdist < 30){
		if(enemypitch > 0x1000) {
			pdest = 0xC800;
		}
		else {
			pdest = 0x2800;
		}
	}
	else {
		if(enemypitch < -0x1000) {
			pdest = 0x2800;
		}
		else {
			pdest = 0xC800;
		}
	}

#else

	if(tdist < 105){  /* was 30  */
		if((((Gtime + planecnt) & 0x1F) < (bv[i].winglevel<<1)) && (bv[i].winglevel > 2) && (!(bv[i].behaveflag & ISHELICOPTER)) && (ptype->maneuver > 1)) {
			if((abs(ps[i].pitch - enemypitch) > 0x1000) && (abs(ps[i].roll) < 0x1000)) {
				if(((ps[i].pitch > 0) || (ps[i].alt < 1000)) && (ps[i].alt < 5250)) {
					bv[i].Behavior = Immelmann;
//					Message("Immelmann", DEBUG_MSG);
				}
				else {
					bv[i].Behavior = SplitS;
//					Message("SplitS", DEBUG_MSG);
				}
			}
		}

//		if((abs(enemypitch - e2uspitch) < 0x1000) && ((abs(enemypitch - ps[i].pitch) < 0x800))) {
		if(abs(enemypitch - e2uspitch) < 0x1000) {
			if(enemypitch < e2uspitch) {
				pdest = 0x2800;
			}
			else {
				pdest = 0xD000;
			}
		}
//		else if(e2uspitch > 0x1000) {
		else if(e2uspitch > ps[i].pitch) {
			pdest = 0xD000;
		}
		else {
			pdest = 0x2800;
		}
	}
	else {
//		if(e2uspitch < -0x1000) {
		if(e2uspitch < ps[i].pitch) {
			pdest = 0x2800;
		}
		else {
			pdest = 0xD000;
		}
	}
//	if(e2uspitch < ps[i].pitch) {
/*	if(e2uspitch > enemypitch) {
		pdest = 0x2800;
	}
	else {
		pdest = 0xD000;
	}  */
#endif

	if(ps[i].alt > 6000) {
//		pdest = 0xC800 - ps[i].pitch;
		pdest = 0xD000;
	}
	else {
		if((ps[i].alt < 500) && (ps[i].pitch > 0xF800)) {
			pdest = 0x3000;
		}
	}

	if(tdistnm < 2) {
		if(targetspeed > 300)
			bv[i].basespeed = targetspeed>>1;
		else
			bv[i].basespeed = 150;
	}
	else
		bv[i].basespeed = ptype->speed;

	if((ps[i].alt < 20) && (ps[i].pitch < 0))
	{
//		pdest = 0 - ps[i].pitch;
		pdest = 0;
	}

//	sprintf(Txt, "J2 1 a %x rra %x bs %d ebs %d", relangle, rrelangle, bv[i].basespeed, enemyspeed);
//	sprintf(Txt, "J1 a %x bs %d ebs %d, r %x, er %x, dr %x, d %d n %d", relangle, bv[i].basespeed, targetspeed, ps[i].roll, enemyroll, droll, tdist, tdistnm);
//	sprintf(Txt, "J1 pdest %x, e %x, tp %x, e2 %x, p %x", pdest, enemypitch, tpitch, e2uspitch, ps[i].pitch);
//	Message(Txt, DEBUG_MSG);

#ifdef ELFBMSG
	if(tempvar) {
		TXT("JK ");
		TXN(i);
		TXA(" bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		TXA(" nm ");
		TXN(tdistnm);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void Formation() {
	long dx, dy;
	int dz;
	int tx, ty;
	int relangle;
	int tangle, tpitch, troll, enemyalt, prewing, temphead, secplace;
	int tempwing, prespeed, tempspeed;
	COORD tdist, distkm, tdistnm;
	int offangle;
	int tempalt;
	int ourleadalt;
	int dpwhead;  /*  is prewing turning alot  */
	int craps;
	long xval, yval;
	UWORD zval;
	struct PlaneData far *tempplane;

	i = planecnt;

	prewing = GetSquadLeader(i);
	if(prewing == -1) {
		FormationUS();
		return;
	}

	tempplane = GetPlanePtr(ps[i].type);

	if(tempplane->maneuver < 2) {
		if(bv[i].horizontalsep < BOMBERPARADEDIST)
			bv[i].horizontalsep = BOMBERPARADEDIST;
	}

	prewing = GetBehaviorLeader(i);

	secplace = (bv[i].behaveflag & 15);

	offangle = 0x6000;
	if(bv[i].behaveflag & COMBAT_SPREAD) {
		offangle = 0x4000;
	}

	prespeed = 0;
	if(prewing != i) {
		prespeed = bv[prewing].basespeed;
	}

	dpwhead = bv[prewing].finhead - ps[prewing].head;

	if((prewing == i) || (secplace <= 1) || (abs(dpwhead) > 0x600)) {
		tempspeed = bv[i].basespeed;
		if((bv[i].behaveflag & ENEMYNOFIRE) || (ImNotaFighter(i, 0))) {
			if(prewing != i) {
				bv[i].groundt = bv[prewing].groundt;
				GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
				bv[i].xloc = xval;
				bv[i].yloc = yval;
			}
			FlyWayPoint();

			if(prewing != i)
				if((bv[i].Behavior != Formation) && (bv[prewing].behaveflag & ENEMYNOFIRE)) {
					bv[i].Behavior = Formation;
				}

//			if(!(bv[prewing].behaveflag & (ENEMYNOFIRE)))
/*			if(i > 10) {
				TXT("wing ");
				TXN(i);
				TXA(" pw ");
				TXN(prewing);
				TXA(" ri ");
				craps = (bv[i].behaveflag & RADARON);
				TXN(craps);
				TXA(" r ");
				craps = (bv[prewing].behaveflag & RADARON);
				TXN(craps);
				TXA(" efi ");
				craps = (bv[i].behaveflag & ENEMYNOFIRE);
				TXN(craps);
				TXA(" efp ");
				craps = (bv[prewing].behaveflag & ENEMYNOFIRE);
				TXN(craps);
				Message(Txt,DEBUG_MSG);
			}  */
		}
		else {
			StraightAttack();
			bv[i].basespeed = tempspeed;
		}
/*		ps[i].head = 0x2000;
		ps[i].alt = 2500;
		ps[i].pitch = 0;
		ps[i].roll = 0;
		droll = 0;  */
		if(prespeed)
			bv[i].basespeed = prespeed - 10;

		if(prewing != i) {
			if(ps[i].alt > 75) {
				pdest = ps[prewing].pitch;
			}
		}
		return;
	}
	if(IsBoat(ps[i].type)) {
		FormationBoat();
		return;
	}
	else if(secplace == 2) {
		if(bv[i].prevsec >= 0)
			tempwing = bv[i].prevsec;
		else if(bv[i].leader >= 0)
			tempwing = bv[i].leader;
		else
			tempwing = prewing;
		temphead = ps[tempwing].head + offangle;
		prespeed = bv[tempwing].basespeed;
	}
	else {
		if(bv[i].horizontalsep == PARADEDIST) {
			if(bv[i].prevsec >= 0) {
				tempwing = bv[i].prevsec;
				if(tempwing == prewing)
					temphead = ps[tempwing].head - offangle;
				else
					temphead = ps[tempwing].head + offangle;
			}
			else if(bv[i].leader >= 0) {
				tempwing = bv[i].leader;
				temphead = ps[tempwing].head - offangle;
			}
			else {
				tempwing = prewing;
				temphead = ps[tempwing].head - offangle;
			}
		}
		else {
			if(bv[i].prevsec >= 0) {
				tempwing = bv[i].prevsec;
				if(tempwing != prewing) {
					if(bv[tempwing].wingman >= 0)
						tempwing = bv[tempwing].wingman;
				}
			}
			else if(bv[i].leader >= 0) {
				tempwing = bv[i].leader;
			}
			else {
				tempwing = prewing;
			}
			temphead = ps[tempwing].head - offangle;
		}
		prespeed = bv[tempwing].basespeed;
	}

//	dx = ps[tempwing].xL + sinX(temphead, bv[i].horizontalsep);
//	dy = ps[tempwing].yL - cosX(temphead, bv[i].horizontalsep);
//	if(i > tempwing) {

//	FlyWayPoint();
//	return;

	bv[i].groundt = bv[prewing].groundt;

	if(i > tempwing) {
		GetLastPosition (tempwing, &dx, &dy, &ourleadalt);
	}
	else {
		dx = ailoc[tempwing].xloc;
		dy = ailoc[tempwing].yloc;
		ourleadalt = ps[tempwing].alt;
	}

	dx = dx + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
	dy = dy - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));

	tempalt = ourleadalt + bv[i].verticlesep;

//	if((tempalt < 75) && (prespeed < 70))
	if(tempalt < 50)
		tempalt = 50;

	if((GLowDroll) && (!(ps[i].status & TAKEOFF))) {
		ps[i].status |= JUSTPUT;
		JustPutHere(i, dx, dy, tempalt, tempwing);
		return;
	}

//	LFlyTo(dx, dy, tempalt, prewing, prespeed);
	LFlyTo(dx, dy, tempalt, tempwing, prespeed);

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
		sprintf(Txt, "1h %x, lh %x, th %x, ti %d", ps[1].head, ps[tempwing].head, temphead, tempwing);
		Message(Txt,DEBUG_MSG);
/*		sprintf(Txt, "PW sx %d, lx %ld, nx %ld, sy %d, ly %ld, ny %ld", ps[tempwing].x, ailoc[tempwing].xloc, dx, ps[tempwing].y, ailoc[tempwing].yloc, dy);
		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "AI sx %d, lx %ld, nx %ld, sy %d, ly %ld, ny %ld", ps[i].x, ailoc[i].xloc, ps[tempwing].xL, ps[i].y, ailoc[i].yloc, ps[tempwing].yL);
		Message(Txt,DEBUG_MSG);  */
	}
#endif
}

void FormationUS()
{
	long dx, dy;
	long dx2, dy2;
	int dz;
	int tx, ty;
	int relangle;
	int tangle, tpitch, troll, enemyalt, prewing, temphead, secplace;
	int tempwing, prespeed;
	COORD tdist, distkm, tdistnm;
	int tempalt;
	int ourleadalt;
	long txt, tyt;

	i = planecnt;

	prewing = -1;

	if(landingstatus & (LANDPLAYERWAIT|LANDPLAYERNOW)) {
		if(whichourcarr >= 0) {
	    	if ((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status&ACTIVE)) {
				if((abs(ps[whichourcarr].x - Px) < 1800) && (abs(ps[whichourcarr].y - Py) < 1800)) {
					ps[planecnt].status |= BACKTOBASE;
					bv[planecnt].Behavior = AIGoHome;
				}
			}
		}
	}
	pdest = 0;

	secplace = (bv[i].behaveflag & 15);

	prespeed = Knots + GetDAltSpeed(ps[i].alt);

	if(bv[i].behaveflag & COMBAT_SPREAD) {
		if(bv[i].behaveflag & ENEMYNOFIRE) {
			bv[i].behaveflag &= ~ENEMYNOFIRE;
			if(!(bv[i].behaveflag & DAMAGERADAR))
				bv[i].behaveflag |= RADARON;
		}
		if(secplace == 2) {
			temphead = OurHead + 0x4000;
		}
		else {
			temphead = OurHead - 0x4000;
		}
	}
	else {
		if(secplace == 2) {
			temphead = OurHead + 0x6000;
		}
		else {
			temphead = OurHead - 0x6000;
		}
	}

	tempwing = -1;

	if(bv[i].leader != -999) {
		tempwing = bv[i].leader;
	}
	else if(bv[i].prevsec != -999) {
		tempwing = bv[i].prevsec;
	}

	if(tempwing == -1) {
//		dx = FPARAMS.X + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
//		dy = FPARAMS.Z - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
		GetLastPosition (-1, &dx, &dy, &ourleadalt);
		dx = dx + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
		dy = dy - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
	}
	else {
		if(i > tempwing) {
			GetLastPosition (tempwing, &dx, &dy, &ourleadalt);
		}
		else {
			dx = ailoc[tempwing].xloc;
			dy = ailoc[tempwing].yloc;
			ourleadalt = ps[tempwing].alt;
		}
		dx = dx + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
		dy = dy - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
//		dx = ps[tempwing].xL + sinX(temphead, bv[i].horizontalsep);
//		dy = ps[tempwing].yL - cosX(temphead, bv[i].horizontalsep);
	}

	tx = ((dx+16)/AILOCDIV) >> HTOV;
	ty = ((dy+16)/AILOCDIV) >> HTOV;
	tdist = Dist2D((tx - ps[i].x), (ty - ps[i].y));

	if(tdist < 128) {
		if(Alt == 0) {
			droll = 0x4000 - ps[i].roll;
			if(bv[i].basespeed < 180)
				bv[i].basespeed = 180;
			return;
		}
	}

	if(bv[i].verticlesep == 0)
		tempalt = ourleadalt + bv[i].verticlesep;
	else
		tempalt = Alt + 4 + bv[i].verticlesep;
	if(tempalt < 75)
		tempalt = 75;

	LFlyTo(dx, dy, tempalt, prewing, prespeed);

#ifdef ELFBMSG
	if(tempvar) {
		TXT("FM ");
		TXN(i);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void BracketLeft() {
	int angleinc, angleevade;

	angleinc = -0x2800;
	angleevade = 0x4000;
	BaseBracket(angleinc, angleevade);
}

void BracketRight() {
	int angleinc, angleevade;

	angleinc = 0x2800;
	angleevade = -0x4000;
	BaseBracket(angleinc, angleevade);
}

void FlyStraight() {
	int angleinc, angleevade;

	angleinc = 0;
	angleevade = 0;
	BaseBracket(angleinc, angleevade);
}

void CheckRadarSet(int tdist, int tangle)
{
	COORD distkm;
	int j;
	int radaron;
//    struct sam  *S;
	struct MissileData far *S;
    struct Sams *ssk;
	int missileinair, misslcnt;

	i = planecnt;

	if(bv[i].behaveflag & DAMAGERADAR) {
		bv[i].behaveflag &= ~RADARON;
		return;
	}

	radaron = 1;

	missileinair = 0;
    distkm = (long)tdist*32/825;   // convert to Km
	for (j=0; j<3; j++) {           // try each missile type
		if(ps[i].mcnt[j] > 0) {
			S = GetMissilePtr(ptype->misl[j]);
//			S=s+ptype->misl[j];
			if((S->minrange < distkm) && ((S->homing == RDR_1) || (S->homing == RDR_2))) {
				radaron = 1;
				if((S->homing == RDR_1) && (bv[i].behaveflag & RADARON)) {
    				for (misslcnt=0, ssk=ss; misslcnt<NSAM; misslcnt++, ssk++) {
        				if (ssk->dist) {
            				if(ssk->source == i) {
								missileinair = 1;
								break;
							}
        				}
    				}
					if(missileinair)
						break;
				}
			}
			else if((S->homing == IR_2) && (distkm>S->minrange) && (distkm<S->range)) {
				if((!Night) || ((tdist < 90) && (abs(tangle) < 0x2000))) {
					radaron = 0;  /*  If in all aspect heat seeker range  */
					break;        /*  turn off radar and use these  */
				}
				else if(Night) {
					radaron = 1;
				}
			}
		}
	}
	if(radaron) {
		bv[i].behaveflag |= RADARON;
	}
	else {
		bv[i].behaveflag &= ~RADARON;
	}
}

void Jink2() {
	int misslcnt;
	int dx, dy, dz, dxw, dyw;
	int relangle, rrelangle;
	int tangle, tpitch, troll, twangle;
	int enemyhead;
	COORD tdist, tdistnm;
    struct Sams *ssk;
	int missileinair;
	int wingnum, enemyalt;
	int enemynum;
	int enemyspeed;

	i = planecnt;

	if(CheckDetection(i) == -999) {
		bv[i].Behavior = StraightAttack;
		StraightAttack();
		return;
	}

	enemynum = CheckDetection(i);
	if(enemynum == -999)
		enemynum = bv[i].target;

	missileinair = 0;
	bv[i].basespeed = ptype->speed;
	if(enemynum == -1) {
		dx = Px - ps[i].x;
		dy = Py - ps[i].y;
		enemyspeed = Knots;
	}
	else {
		dx = ps[enemynum].x - ps[i].x;
		dy = ps[enemynum].y - ps[i].y;
		enemyspeed = bv[enemynum].basespeed;
	}
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	relangle = tangle - ps[i].head;
	if(enemynum == -1) {
		enemyhead = OurHead;
	}
	else {
		enemyhead = ps[enemynum].head;
	}
	rrelangle = (0x8000 + tangle) - enemyhead;
	pdest = tpitch;

#if ELFBMSG
	if(tempvar) {
		TXT("JK2 ");
		TXN(i);
		TXA(" ra ");
		TXN(relangle);
		TXA(" rra ");
		TXN(rrelangle);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif

	if(abs(relangle) < 0x2000) {
    	for (misslcnt=0, ssk=ss; misslcnt<NSAM; misslcnt++, ssk++) {
        	if (ssk->dist) {
            	if(ssk->source == i) {
					missileinair = 1;
					break;
				}
        	}
    	}
		if(missileinair == 0) {
#ifdef ELFBMSG
			if(tempvar) {
				TXT("JK2 ");
				TXN(i);
				TXA(", No Missile");
				Message(Txt,DEBUG_MSG);
			}
#endif
			StraightAttack();
			if((abs(relangle) > 0x800) && (tdist < 90)) {
				if(ps[i].alt > 250) {
					pdest = 0xE400;
				}
			}
			return;
		}
	}
	if((abs(relangle) < 0x5000) && (abs(rrelangle) < 0x2800) && (tdist < 60)) {
		StraightAttack();
		if(abs(relangle) > 0x4000) {
			if(enemyspeed > 300)
				bv[i].basespeed = enemyspeed>>1;
			else
				bv[i].basespeed = 150;
		}
		else
			bv[i].basespeed = ptype->speed;
//		pdest = 0x3000 - ps[i].pitch;
		pdest = 0x3000;
#if ELFBMSG
		if(tempvar) {
			TXT("JKS ");
			TXN(i);
			TXA(" ra ");
			TXN(relangle);
			TXA(" rra ");
			TXN(rrelangle);
			TXA(", d ");
			TXN(tdist);
			Message(Txt,DEBUG_MSG);
		}
#endif
		return;
	}
	wingnum = -999;
	if(bv[i].leader >= 0)
		wingnum = bv[i].leader;
	else if(bv[i].wingman >= 0)
		wingnum = bv[i].wingman;

	if(wingnum != -999) {
		if(bv[wingnum].target == -1) {
			dxw = Px - ps[wingnum].x;
			dyw = Py - ps[wingnum].y;
			tdist = Dist2D(dxw, dyw);
		}
		else {
			dxw = ps[bv[wingnum].target].x - ps[wingnum].x;
			dyw = ps[bv[wingnum].target].y - ps[wingnum].y;
			tdist = Dist2D(dxw, dyw);
		}
		twangle = angle(dxw, -dyw);
		twangle = (0x8000 + twangle) - enemyhead;
		if(twangle < 0) {
			relangle -= 0x2000;
		}
		else {
			relangle += 0x2000;
		}
	}
	else {
		if((abs(relangle) > 0x5000) && (abs(rrelangle) < 0x2800) && (tdist < 200)){
			Jink();
			return;
		}
		else {
			if(relangle > 0) {
				if(rrelangle < 0)
					relangle += 0x2000;
				else
					relangle -= 0x2000;
			}
			else {
				if(rrelangle > 0)
					relangle -= 0x2000;
				else
					relangle += 0x2000;
			}
		}

	}

	TurnAIPlaneBy(i, relangle);

	if((tpitch < 0) && (ps[i].alt > 20) && (enemyalt > 300)) {
//		pdest = 0xC800 - ps[i].pitch;
		pdest = 0xC800;
	}
	else {
//		pdest = 0x2800 - ps[i].pitch;
		pdest = 0x2800;
	}

	if(ps[i].alt > 6000) {
//		pdest = 0xC800 - ps[i].pitch;
		pdest = 0xC800;
	}


#ifdef ELFBMSG
	if(tempvar) {
		TXT("JK2 ");
		TXN(i);
		TXA(" bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void PostHole()
{
	int dx, dy, dz;
	int relangle, rrelangle;
	int tangle, tpitch, troll;
	int enemyhead, enemyalt;
	COORD tdist, tdistnm;
	int enemynum;

	i = planecnt;

	enemynum = CheckDetection(i);
	if(enemynum == -999)
		enemynum = bv[i].target;

	if(enemynum == -1) {
		dx = Px - ps[i].x;
		dy = Py - ps[i].y;
		enemyhead = OurHead;
	}
	else {
		dx = ps[enemynum].x - ps[i].x;
		dy = ps[enemynum].y - ps[i].y;
		enemyhead = ps[enemynum].head;
	}
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	rrelangle = (0x8000 + tangle) - enemyhead;
	if(rrelangle < 0) {
		droll = 0x5000 - ps[i].roll;
	}
	else {
		droll = 0xB000 - ps[i].roll;
	}
	if(ps[i].alt > 250) {
//		pdest = -0x1000 - ps[i].pitch;
		pdest = -0x1000;
	}
	else
		pdest = 0;
#ifdef ELFBMSG
	if(tempvar) {
		TXT("PH ");
		TXN(i);
		TXA(" bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void BaseBracket(int angleinc, int angleevade)
{
	int dx, dy, dz;
	int relangle, tempalt;
	int tangle, tpitch, troll, tangle1, enemyalt;
	COORD tdist, distkm, tdistnm, ttdist, ttdistnm, ttdistkm;
	int winglead;

	i = planecnt;

	if(bv[i].target == -1) {
		dx = Px - ps[i].x;
		dy = Py - ps[i].y;
	}
	else {
		dx = ps[bv[i].target].x - ps[i].x;
		dy = ps[bv[i].target].y - ps[i].y;
	}

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

#if 1
	if((CheckDetection(i) != -999) && (tdistnm < EVASIVEDIST)){
		winglead = bv[i].leader;
		if(winglead >= 0){
			winglead = CheckDetection(winglead);
		}
		else
			winglead = -999;
		if(winglead == -999) {
			if(bv[i].target == -1)
				relangle = (OurHead + angleevade) - ps[i].head;
			else
				relangle = (ps[bv[i].target].head + angleevade) - ps[i].head;

			TurnAIPlaneBy(i, relangle);
			return;
		}
	}
#endif

	tpitch = 0;
	if(GetSquadLeader(i) == -1) {
		tempalt = Alt + bv[i].verticlesep;
		if(tempalt < 75)
			tempalt = 75;
		if(ps[i].alt < tempalt)
			tpitch = 0x2000 - ps[i].pitch;
		else if(ps[i].alt > (tempalt + 50)) {
			tpitch = 0xD000 - ps[i].pitch;
		}
		else
			tpitch = 0;
	}
	tangle1 = tangle;
	tangle += angleinc;
	distkm = tdist*32/825;

	if((bv[i].target == -1) && (GetSquadLeader(i) == -1)) {
		if(distkm > 10) {
			bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt);
			tangle = OurHead;
		}
		else {
			tangle = bv[i].finhead + angleinc;
		}
	}
	else if(bv[i].leader == -1) {
		ttdist = Dist2D((Px - ps[i].x), (Py - ps[i].y));
		ttdistkm = ttdist*32/825;
		if(ttdistkm > 10) {
			bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt);
			tangle = OurHead;
		}
	}
	else if(bv[i].leader >= 0) {
		ttdist = Dist2D((ps[bv[i].leader].x - ps[i].x), (ps[bv[i].leader].y - ps[i].y));
		ttdistnm = (ttdist*2)/90;
		if(ttdistnm > 15) {             /*  Switch to NM so that we could still get good seperation for Chamagne  */
			bv[i].basespeed = bv[bv[i].leader].basespeed;    /*  Might want to make 10 bigger  */
			tangle = ps[bv[i].leader].head;
		}
	}

/**********************************************************************/
//  NEED TO MODIFY UPDATE SPEED AND POSITION FUNCTION before this
//  can be used
//
//  I think it is usable now.  SRE  4/19/93
/**********************************************************************/

	relangle = tangle - ps[i].head;
	tangle1 = tangle1 - ps[i].head;
	pdest = tpitch;

	TurnAIPlaneBy(i, relangle);

#ifdef ELFDMSG
	if(tempvar)  {
		TXT("BB ");
		TXN(i);
		TXA(" ta1 ");
		tangle1 = TrgMul (tangle1>>1, 360);
		TXN(tangle1);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif

}

#ifdef SREOLDVERS
void DoAICounters(int ainum, int distnm, struct Sams *enemymissile)
{
//	struct sam *emsltype;
	struct MissileData far *emsltype;

	if (!(bv[ainum].behaveflag & CFDROPPED)) {
		emsltype = GetMissilePtr(enemymissile->type);
//    	emsltype = s+ enemymissile->type;
		if (((emsltype->homing == IR_1) || (emsltype->homing == IR_2))
				&& (((!(Rtime & 0x110)) && (distnm == 1) && (!(FriendMatchType(ainum, FRIENDLY))))
				|| ((FriendMatchType(ainum, FRIENDLY)) && (distnm == 2)))) {
			if(bv[ainum].leader == -1) {
				GetWingName(ainum, Txt);
				TXA("Dropping Flare");
				Message(Txt,WINGMAN_MSG);
			}
			dropdecoy(FLARE, ainum);
			bv[ainum].behaveflag |= CFDROPPED;
		}
		else if (((emsltype->homing == RDR_1) || (emsltype->homing == RDR_2))
				&& (((distnm == 3) && (!(Rtime & 0x110)) && (!(FriendMatchType(ainum, FRIENDLY))))
				|| ((distnm == 2) && (FriendMatchType(ainum, FRIENDLY))))) {
			if((bv[ainum].leader == -1) && (distnm == 2)) {
				GetWingName(ainum, Txt);
				TXA("Dropping Chaff");
				Message(Txt,WINGMAN_MSG);
			}
			dropdecoy(CHAFF, ainum);
			bv[ainum].behaveflag |= CFDROPPED;
		}
	}
	else if (((distnm != 1) && (distnm != 3) && (!(FriendMatchType(ainum, FRIENDLY)))) || ((distnm != 2) && (FriendMatchType(ainum, FRIENDLY)))) {
		bv[ainum].behaveflag &= ~CFDROPPED;
	}
}
#else

void DoAICounters(int ainum, int distnm, struct Sams *enemymissile)
{
//	struct sam *emsltype;
	struct MissileData far *emsltype;
	UWORD difftime;

	difftime = 0x008;  /*  Might Want to tweek this  */
	if(bv[ainum].winglevel <= 1)
		difftime = 0xFFFF;
	else if(bv[ainum].winglevel == 2)
		difftime = 0x00E;
	else if(bv[ainum].winglevel == 3)
		difftime = 0x00C;

	if (!(bv[ainum].behaveflag & CFDROPPED)) {
		emsltype = GetMissilePtr(enemymissile->type);
//    	emsltype = s+ enemymissile->type;
		if (((emsltype->homing == IR_1) || (emsltype->homing == IR_2))
				&& (((!(Gtime & difftime)) && (distnm == 1) && (bv[ainum].leader != -1))
				|| ((bv[ainum].leader == -1) && (distnm == 2)))) {
			if(bv[ainum].leader == -1) {
				GetWingName(ainum, Txt);
				TXA("Dropping Flare");
				Message(Txt,WINGMAN_MSG);
			}
			dropdecoy(FLARE, ainum);
			bv[ainum].behaveflag |= CFDROPPED;
		}
		else if (((emsltype->homing == RDR_1) || (emsltype->homing == RDR_2))
				&& (((distnm == 3) && (!(Gtime & difftime)) && (bv[ainum].leader != -1))
				|| ((distnm == 2) && (bv[ainum].leader == -1)))) {
			if((bv[ainum].leader == -1) && (distnm == 2)) {
				GetWingName(ainum, Txt);
				TXA("Dropping Chaff");
				Message(Txt,WINGMAN_MSG);
			}
			dropdecoy(CHAFF, ainum);
			bv[ainum].behaveflag |= CFDROPPED;
		}
	}
	else if (((distnm != 1) && (distnm != 3) && (bv[ainum].leader != -1)) || ((distnm != 2) && (bv[ainum].leader == -1))) {
		bv[ainum].behaveflag &= ~CFDROPPED;
	}
}
#endif

void EvadeMissile(int ainum)
{
	struct Sams *enemymissile;
	struct sam *emsltype;
	int cnt;
	long dx, dy;
	int tempangle, relangle;
	COORD homedist, distnm;
	struct Sams *tenemymissile;
	COORD thomedist;

	tenemymissile = 0;
    for (cnt=0, enemymissile=ss; cnt<NSAM; cnt++, enemymissile++) {
        if ((enemymissile->dist) && (enemymissile->target == ainum)) {
			bv[ainum].basespeed = ptype->speed;
   			dx = ps[ainum].xL- enemymissile->x;
   			dy = ps[ainum].yL- enemymissile->y;
#if 0
   			if (absL(dx)>=23000L || absL(dy)>=23000L) {
      			dx = dx>>HTOV;
      			dy = dy>>HTOV;
      			homedist = Dist2D ((int)dx, (int)dy);

   			} else {
      			homedist = Dist2D ((int)dx, (int)dy);
      			homedist >>= HTOV;
   			}
#else
			homedist = (int)(LDist2D(dx, dy)>>HTOV);
#endif
			tempangle = enemymissile->head- ps[ainum].head;
			if(tempangle < 0)
				tempangle = enemymissile->head + 0x4000;
			else
				tempangle = enemymissile->head - 0x4000;

			relangle = tempangle - ps[ainum].head;

			TurnAIPlaneBy(ainum, relangle);

			if(!(FriendMatchType(ainum, FRIENDLY))){
				distnm = (homedist*2) / 95;
				DoAICounters(ainum, distnm, enemymissile);
				return;
			}
			else {
				if(tenemymissile == 0) {
					tenemymissile = enemymissile;
					thomedist = homedist;
				}
				else {
					if(homedist < thomedist) {
						tenemymissile = enemymissile;
						thomedist = homedist;
					}
				}
			}
		}
	}

	if((tenemymissile != 0) && ((ConfigData.WeaponEffect==1) || (ConfigData.WeaponEffect==2))) {
		distnm = (thomedist*2) / 95;
		DoAICounters(ainum, distnm, tenemymissile);
	}
}

void DamageAI(int planenum)
{
	int damagenum;
	int damageeffect;
	char Str[80];
	struct PlaneData far *tempplane;

	if(ps[planenum].status & CRASH)
		return;

	if(bv[planenum].Behavior == FlyCruise) {
		AirKill(planenum);
		return;
	}

	damagenum = Rtime & 3;
	damageeffect = 0;
	if(damagenum == 0) {
		damageeffect = DAMAGERADAR;
		if((Night) && (bv[planenum].leader != -1) && (bv[planenum].Behavior != FlyCruise)) {
    		ps[planenum].status |= BACKTOBASE;
			bv[planenum].Behavior = AIGoHome;
			LetWingmenContinue(planenum);
#ifdef SHOWHOME
			TXT("Bad Damage, Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(planenum);
		}
	}
	else if(damagenum == 1) {
		damageeffect = DAMAGEENGINE;
	}
	else if(damagenum == 2) {
		damageeffect = DAMAGECONTROLSURFACE;
		if((bv[planenum].leader != -1) && (bv[planenum].Behavior != FlyCruise)) {
    		ps[planenum].status |= BACKTOBASE;
			bv[planenum].Behavior = AIGoHome;
			LetWingmenContinue(planenum);
#ifdef SHOWHOME
			TXT("Bad Damage 2, Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(planenum);
		}
	}
	else {
		if(ps[planenum].mcnt[0] || ps[planenum].mcnt[1]) {
			ps[planenum].mcnt[0] = 0;
			ps[planenum].mcnt[1] = 0;
			if((bv[planenum].leader != -1) && (bv[planenum].Behavior != FlyCruise)) {
    			ps[planenum].status |= BACKTOBASE;
				bv[planenum].Behavior = AIGoHome;
				LetWingmenContinue(planenum);
#ifdef SHOWHOME
				TXT("Bad Damage3, Going Home ");
				TXN(i);
				Message(Txt,DEBUG_MSG);
#endif
				GetAIBase(planenum);
			}
		}
		else {
			AirKill(planenum);
			return;
		}
	}

	if(bv[planenum].behaveflag & damageeffect) {
		AirKill(planenum);
		return;
	}
	else {
		bv[planenum].behaveflag |= damageeffect;
		StartASmoke( ps[planenum].xL, ps[planenum].alt,ps[planenum].yL, AIR2XPLO, 20 );
	}

	if(bv[planenum].leader == -1) {
		Speech(IM_HIT);
		GetWingName(planenum, Str);
		if(damagenum == 0)
			strcat (Str, "I'm Hit, Radar Out!");
		else if(damagenum == 1)
			strcat (Str, "I'm Hit, Engine Damaged!");
		else if(damagenum == 2)
			strcat (Str, "I'm Hit, Control Surfaces Damaged!");
		else
			strcat (Str, "I'm Hit, Missiles Out");
		Message(Str,WINGMAN_MSG);
	}

	tempplane = GetPlanePtr(ps[planenum].type);
    FTXT (tempplane->name);
	TXA (" ");
}

void GetSortInfo(int ainum)
{
	int prewing;
	int x, y, dx, dy;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int rangle1, rrangle1, tangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1, targnm;
	int aihead;
	int targetnum;
	int rangarray[5];
	int distarray[5];
	int numarray[5];
	int rangtemp, disttemp, numtemp;
	int cnt, wingcnt, cnt2;
	int tempsort, tempother, tempme;

	if(bv[ainum].target == -1) {
		TXA("none");
		Message(Txt,WINGMAN_MSG);
		return;
	}
	x = ps[ainum].x;
	y = ps[ainum].y;
	aihead = ps[ainum].head;
	targetnum = bv[ainum].target;
   	dx = ps[targetnum].x - x;
	dy = ps[targetnum].y - y;
	GetTargADP(ainum, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
	if(tdistnm <= 5) {
		TXA("unknown");
		Message(Txt,WINGMAN_MSG);
		return;
	}
	rangle = tangle - aihead;
	if(((abs(rangle) > 0x3800) && (tdistnm > 20)) || (tdistnm > 80) ||
			(!((ps[targetnum].status & ALIVE) && (ps[targetnum].status&ACTIVE) && (ps[targetnum].speed)))) {
		TXA("none");
		Message(Txt,WINGMAN_MSG);
		return;
	}
	targnm = tdistnm;
	for(cnt = 0; cnt < 5; cnt ++) {
		rangarray[cnt] = -1;
		distarray[cnt] = -1;
		numarray[cnt] = -1;
	}
	targetnum = bv[ainum].target;
	prewing = GetSquadLeader(bv[ainum].target);
	wingcnt = prewing;
	cnt = 0;
	while (prewing >= 0) {
		while (wingcnt >= 0) {
   			dx = ps[wingcnt].x - x;
			dy = ps[wingcnt].y - y;
			GetTargADP(ainum, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
			if(abs(tdistnm - targnm) < 30) {
				rangle = tangle - aihead;
				rangarray[cnt] = rangle;
				distarray[cnt] = tdist;
				numarray[cnt] = wingcnt;
				cnt ++;
			}
			wingcnt = bv[wingcnt].wingman;
		}
		prewing = bv[prewing].nextsec;
		wingcnt = prewing;
	}
	if(cnt <= 1) {
		TXA("locked");
		Message(Txt,WINGMAN_MSG);
		return;
	}
	for(cnt2 = 0; cnt2 < 4; cnt2 ++) {
		cnt = 0;
		while((cnt < 4) && (numarray[cnt + 1] != -1)) {
			if(rangarray[cnt + 1] < rangarray[cnt]) {
				rangtemp = rangarray[cnt];
				disttemp = distarray[cnt];
				numtemp = numarray[cnt];
				rangarray[cnt] = rangarray[cnt+1];
				distarray[cnt] = distarray[cnt+1];
				numarray[cnt] = numarray[cnt+1];
				rangarray[cnt+ 1] = rangtemp;
				distarray[cnt+ 1] = disttemp;
				numarray[cnt+ 1] = numtemp;
			}
			cnt ++;
		}
	}
	GiveSortInfo(rangarray, numarray, distarray, targetnum);
}

void GiveSortInfo(int rangarray[], int numarray[], int distarray[], int targetnum)
{
	int tempme, tempother, tempsort;

	if(numarray[2] == -1) {
		if(abs(distarray[0] - distarray[1]) < 10) {
			if(targetnum == numarray[0]) {
				TXA("Left");
				Message(Txt,WINGMAN_MSG);
				return;
			}
			else {
				TXA("Right");
				Message(Txt,WINGMAN_MSG);
				return;
			}
		}
		else {
			if(targetnum == numarray[0]) {
				tempme = 0;
				tempother =1;
			}
			else {
				tempme = 1;
				tempother =0;
			}
			if(distarray[tempme] < distarray[tempother]) {
				TXA("Lead");
				Message(Txt,WINGMAN_MSG);
				return;
			}
			else {
				TXA("Trail");
				Message(Txt,WINGMAN_MSG);
				return;
			}
		}
	}
	else if(numarray[3] == -1) {
		if(abs(rangarray[0] - rangarray[1]) < abs(rangarray[2] - rangarray[1]))
			tempsort = 0;
		else
			tempsort = 1;
		if(targetnum != numarray[1]) {
			if(targetnum == numarray[0]) {
				TXA("Left ");
				tempme = 0;
			}
			else {
				TXA("Right ");
				tempme = 2;
			}
			if(!tempsort) {
				if(distarray[tempme] < distarray[1]) {
					TXA("Lead");
				}
				else {
					TXA("Trail");
				}
			}
			Message(Txt,WINGMAN_MSG);
			return;
		}
		else {
			if(tempsort) {
				TXA("Left ");
				tempother = 0;
			}
			else {
				TXA("Right ");
				tempother = 1;
			}
			if(distarray[1] < distarray[tempother]) {
				TXA("Lead");
			}
			else {
				TXA("Trail");
			}
			Message(Txt,WINGMAN_MSG);
			return;
		}
	}
	else {
		if(targetnum == numarray[0]) {
			tempme = 0;
			tempother = 1;
			TXA("Left ");
		}
		else if(targetnum == numarray[1]) {
			tempme = 1;
			tempother = 0;
			TXA("Left ");
		}
		else if(targetnum == numarray[2]) {
			tempme = 2;
			tempother = 3;
			TXA("Right ");
		}
		else if(targetnum == numarray[3]) {
			tempme = 3;
			tempother = 2;
			TXA("Right ");
		}
		if(distarray[tempme] < distarray[tempother]) {
			TXA("Lead");
			Message(Txt,WINGMAN_MSG);
			return;
		}
		else {
			TXA("Trail");
			Message(Txt,WINGMAN_MSG);
			return;
		}
	}
	TXA("?");
	Message(Txt,WINGMAN_MSG);
	return;
}

#define CAPL 338  /*  half CAP length of 7.5 nm  */
#define CAPW 90   /*  half CAP width of 2 nm  */
#define CAPD 450
#define CAPALT 6250
void SetUpCAP1()
{
	int othersec;
	int prewing;
	long xval, yval;
	UWORD zval;

	i = planecnt;

	prewing = GetSquadLeader(i);

	CheckCAP();

	if(bv[i].groundt != -1) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}

	if(SetUpOval1(CAPALT, CAPW, CAPL)) {
		if(bv[i].nextsec > -1) {
			othersec = bv[i].nextsec;
			bv[othersec].xloc += (long)sinXL((bv[i].threathead + 0x4000), (long)(900<<HTOV));
			bv[othersec].yloc -= (long)cosXL((bv[i].threathead + 0x4000), (long)(900<<HTOV));
			if(bv[othersec].wingman > -1) {
				bv[bv[othersec].wingman].xloc += sinXL((bv[i].threathead + 0x4000), (long)(900<<HTOV));
				bv[bv[othersec].wingman].yloc -= cosXL((bv[i].threathead + 0x4000), (long)(900<<HTOV));
			}
			bv[i].nextsec = -999;
			bv[othersec].prevsec = -999;
			bv[othersec].Behavior = SetUpCAP1;
		}
		bv[i].Behavior = SetUpCAP2;
	}

	if((prewing != i) && (prewing != -1)) {
		bv[i].Behavior = Formation;
		bv[i].behaveflag |= COMBAT_SPREAD;
		bv[i].horizontalsep = COMBATSPREADDIST;
	}

}

int SetUpOval1(int ovalheight, int ovalwidth, int ovallength)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;
	/*   Fly aircraft to one 'corner' of the CAP pattern  */

	i = planecnt;

	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);

	if(headval != -1)
		bv[i].threathead = headval;

	if(altval != -1)
		dalt = altval;
	else
		dalt = ovalheight;

	if(speedval != -1)
		bv[i].basespeed = speedval;

	temphead = bv[i].threathead;
	temphead2 = temphead + 0x4000;
	dx += sinX(temphead2, ovalwidth);
	dy -= cosX(temphead2, ovalwidth);
	temphead2 = temphead + 0x8000;
	dx += sinX(temphead2, ovallength);
	dy -= cosX(temphead2, ovallength);

	if(FlyTo(dx, dy, dalt, 0)) {
		return(1);
	}

#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("SetUPOVAL1 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(0);

}

void SetUpCAP2()
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;

	i = planecnt;

	CheckCAP();

	wingnum = -999;
	if(bv[i].leader > -1)
		wingnum = bv[i].leader;
	else if(bv[i].wingman > -1)
		wingnum = bv[i].wingman;
	if(SetUpOval2(CAPALT, CAPW, CAPL)) {
  		bv[i].Behavior = CAPLeg1;
		if(GetSquadLeader(i) == -1) {
  			bv[i].Behavior = SetUpCAP3;
			GetWingName(i, Txt);
			TXA("Going Cold");
			Message(Txt,WINGMAN_MSG);
		}
		bv[i].basespeed = 300;
		if(!(bv[i].behaveflag & DAMAGERADAR))
			bv[i].behaveflag |= RADARON;
		bv[i].behaveflag &= ~ENEMYNOFIRE;
		if(bv[i].wingman > -1) {
			bv[wingnum].Behavior = SetUpCAP3;
			bv[wingnum].basespeed = 400;
			if(!(bv[wingnum].behaveflag & DAMAGERADAR))
				bv[wingnum].behaveflag |= RADARON;
			bv[wingnum].behaveflag &= ~ENEMYNOFIRE;
		}
	}


}

int SetUpOval2(int ovalheight, int ovalwidth, int ovallength)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	i = planecnt;

	/*   Fly aircraft to one 'corner' of the CAP pattern  */
	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if(altval != -1)
		dalt = altval;
	else
		dalt = ovalheight;

	temphead = bv[i].threathead;
	temphead2 = temphead + 0x4000;
	dx += sinX(temphead2, ovalwidth);
	dy -= cosX(temphead2, ovalwidth);
	if(FlyTo(dx, dy, dalt, 0)) {
		return(1);
	}

#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("SetUPOVAL2 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(0);
}

void SetUpCAP3()
{
	int dx, dy, dalt;
	int temphead, temphead2;

	i = planecnt;

	CheckCAP();
	if(SetUpOval3(CAPALT, CAPW, CAPL))
		bv[i].Behavior = CAPTurn2;
}

int SetUpOval3(int ovalheight, int ovalwidth, int ovallength)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	i = planecnt;

	/*   Fly aircraft to one 'corner' of the CAP pattern  */
	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if(altval != -1)
		dalt = altval - 500;
	else
		dalt = ovalheight - 500;

	if(dalt < 5)
		dalt = 5;

	temphead = bv[i].threathead;
	temphead2 = temphead - 0x4000;
	dx += sinX(temphead2, ovalwidth);
	dy -= cosX(temphead2, ovalwidth);
	temphead2 = temphead + 0x8000;
	dx += sinX(temphead2, ovallength);
	dy -= cosX(temphead2, ovallength);
	if(FlyTo(dx, dy, dalt, 0)) {
		return(1);
	}

#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("SetUPCAP3 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(0);
}

void CAPTurn1()
{
	int dx, dy, dalt;
	int temphead, temphead2;
	long xval, yval;
	UWORD zval;

	i = planecnt;

	if((bv[i].groundt != -1) && (bv[i].Behavior != AICarrierLand) && (!(Rtime & 63))) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}

	CheckCAP();

	if(OvalTurn1(CAPALT, CAPW, CAPL)) {
		bv[i].Behavior = CAPLeg2;
	}
}

int OvalTurn1(int ovalheight, int ovalwidth, int ovallength)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	i = planecnt;

	/*   Fly aircraft to one 'corner' of the CAP pattern  */
	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if((altval != -1) && (bv[i].Behavior != AICarrierLand))
		dalt = altval;
	else
		dalt = ovalheight;

	if(bv[i].leader > -1)
		dalt -= 500;

	if(dalt < 5)
		dalt = 5;

	temphead = bv[i].threathead;
	temphead2 = temphead - 0x4000;
	dx += sinX(temphead2, ovalwidth);
	dy -= cosX(temphead2, ovalwidth);
	temphead2 = temphead;
	dx += sinX(temphead2, ovallength);
	dy -= cosX(temphead2, ovallength);
	if(FlyTo(dx, dy, dalt, 0)) {
		return(1);
	}

#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPTurn1 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(0);
}

void CAPTurn2()
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;

	i = planecnt;

	CheckCAP();

	wingnum = -999;
	if(bv[i].leader > -1)
		wingnum = bv[i].leader;
	else if(bv[i].wingman > -1)
		wingnum = bv[i].wingman;
	/*   Fly cold end turn of CAP Pattern  */
	/*   If we get to hot side of pattern  */
	/*   And otherwingman still hot then   */
	/*   Tell him to speed up and we go a   */
	/*   little slower.                    */

	if(OvalTurn2(CAPALT, CAPW, CAPL)) {
		bv[i].Behavior = CAPLeg1;
		if(wingnum > -1) {
			bv[wingnum].basespeed = 450;
			if(bv[wingnum].Behavior == CAPLeg1) {
				bv[i].basespeed = 225;
			}
			else
				bv[i].basespeed = 275;
		}
		else
			bv[i].basespeed = 275;

		if(GetSquadLeader(i) == -1) {
			GetWingName(i, Txt);
			TXA("Going Hot");
			Message(Txt,WINGMAN_MSG);
		}
	}

}

int OvalTurn2(int ovalheight, int ovalwidth, int ovallength)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	i = planecnt;

	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if((altval != -1) && (bv[i].Behavior != AICarrierLand))
		dalt = altval;
	else
		dalt = ovalheight;

	if(bv[i].leader > -1)
		dalt -= 500;

	if(dalt < 5)
		dalt = 5;

	temphead = bv[i].threathead;
	temphead2 = temphead + 0x4000;
	dx += sinX(temphead2, ovalwidth);
	dy -= cosX(temphead2, ovalwidth);
	temphead2 = temphead + 0x8000;
	dx += sinX(temphead2, ovallength);
	dy -= cosX(temphead2, ovallength);
	if(FlyTo(dx, dy, dalt, 0)) {
		return(1);
	}

#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPTurn2 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(0);
}

void CAPLeg1()
{
	int wingnum;

	i = planecnt;

	CheckCAP();

	if(CAPAI(0, CAPALT, CAPW, CAPL, CAPD)) {
		wingnum = -999;
		if(bv[i].leader > -1)
			wingnum = bv[i].leader;
		else if(bv[i].wingman > -1)
			wingnum = bv[i].wingman;

		bv[i].Behavior = CAPTurn1;

		if(wingnum > -1) {
			if(bv[wingnum].Behavior != CAPLeg1) {
				bv[wingnum].basespeed = 550;
			}
		}
		if(GetSquadLeader(i) == -1) {
			GetWingName(i, Txt);
			TXA("Going Cold");
			Message(Txt,WINGMAN_MSG);
		}
	}
#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPLeg1 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void CAPLeg2()
{
	int wingnum;

	i = planecnt;

	CheckCAP();

	if(CAPAI(1, CAPALT, CAPW, CAPL, CAPD)) {
		bv[i].Behavior = CAPTurn2;
	}
#if 0
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPLeg2 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void CheckCAP()
{
	int targnum;
	COORD targdist;
	int bogeyangle;
	COORD bogeydist;
	int oldtarget, newtarget;
	int dx, dy;
	struct PlaneData far *tempplane;

	i = planecnt;

	tempplane = GetPlanePtr(ps[i].type);

	if(tempplane->bclass == RADAR_PLANE_OBJCLASS)
		CheckE2C(planecnt);

//	return;

	if(!(Gtime & 3)) {
		oldtarget = GetSquadLeader(bv[i].target);
		if(AIRadarForwardCheck(i, 0, 1)) {
			tempgotcha = 8;
			gotcha (bv[i].target);
			tempgotcha = -999;
/*			Message("CAP Intercept",DEBUG_MSG);
			TXT(" ");
			TXN(i);
			TXA(" engaging ");
			TXN(bv[i].target);
			Message(Txt,DEBUG_MSG);  */
			if((GetSquadLeader(bv[i].target) == -1) && (targetplayer < 0) && (timetargetplayer < Gtime)) {
				Message("(Bandit)Unidentified Aircraft Turn Back Now!!!",GAME_MSG);
//				Message("(Bandit)Turn Back Now!!!",GAME_MSG);
				timetargetplayer = Gtime + 8;
			}

			if(GetSquadLeader(i) == -1) {
				newtarget = bv[i].target;
				if(oldtarget != GetSquadLeader(newtarget)) {
					dx = ps[newtarget].x - ps[i].x;
					dy = ps[newtarget].y - ps[i].y;
	    			bogeyangle = angle(dx,-dy);
    				bogeydist = Dist2D(dx,dy);
					LocationPicture (WINGSRC, ENEMYSRC, bogeyangle, bogeydist, bv[i].target, 0);
				}
			}
#if 0
			targnum = bv[i].target
			if(targnum >= 0)
				targdist = xydist(ps[targnum].x-ps[i].x, ps[targnum].y-ps[i].y);
			else
				targdist = xydist(Px-ps[i].x, Py-ps[i].y);
			if(targdist < 1800) {   /*  40 nm  */
				if(bv[i].leader > -1) {
					if(!(FriendMatchType(i, FRIENDLY))) {
						bv[i].Behavior = Formation;
					}
					else
						bv[i].Behavior = FlyStraight;
					bv[i].behaveflag |= COMBAT_SPREAD;
					bv[bv[i].leader].target = bv[i].target;
					bv[bv[i].leader].Behavior = FlyStraight;
					bv[bv[i].leader].behaveflag |= RADARON;
					bv[bv[i].leader].behaveflag &= ~ENEMYNOFIRE;
				}
				else {
					bv[i].Behavior = FlyStraight;
					if(bv[i].wingman > -1) {
						bv[bv[i].wingman].Behavior = Formation;
						bv[bv[i].wingman].behaveflag |= COMBAT_SPREAD;
						bv[bv[i].wingman].target = bv[i].target;
						bv[bv[i].wingman].behaveflag |= RADARON;
						bv[bv[i].wingman].behaveflag &= ~ENEMYNOFIRE;
					}
				}
				bv[i].behaveflag |= RADARON;
				bv[i].behaveflag &= ~ENEMYNOFIRE;
			}
#endif
		}
	}
}

int CAPAI(int side, int ovalheight, int ovalwidth, int ovallength, int ovaldist)
{

	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	int alttemp;
	int hangle;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	i = planecnt;

	dx = (int)(bv[i].xloc>>HTOV);
	dy = (int)(bv[i].yloc>>HTOV);
	dx = dx - ps[i].x;
	dy = dy - ps[i].y;

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	hangle = (tangle + 0x8000) - bv[i].threathead;
	if(side) {
		relangle = (bv[i].threathead + 0x8000) - ps[i].head;
	}
	else {
		relangle = bv[i].threathead - ps[i].head;
	}

	if(tdist > ovaldist)
		return(1);

	TurnAIPlaneBy(i, relangle);

	GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if((altval != -1) && (bv[i].Behavior != AICarrierLand))
		alttemp = altval;
	else
		alttemp = ovalheight;

	if(bv[i].leader > -1)
		alttemp -= 500;

	if(ps[i].alt < alttemp) {
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > (alttemp + 500)) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

	return(0);
}

int FlyTo(int xdim, int ydim, int altdim, int ignoredist)
{

	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	int alttemp;

	i = planecnt;

	dx = xdim;
	dy = ydim;
	dx = dx - ps[i].x;
	dy = dy - ps[i].y;

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	relangle = tangle - ps[i].head;

	TurnAIPlaneBy(i, relangle);

	alttemp = ps[i].alt;

	if(alttemp < altdim) {
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(alttemp > (altdim + 50)) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

	if((tdist < 20) && !ignoredist) {
		return(1);
	}

#if 1
	if((tempvar) && (bv[i].Behavior == AIGoHome)) {
		TXT("FT ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
//		TXA(" b ");
//		TXN(ps[i].base);
		Message(Txt,DEBUG_MSG);
	}
#endif
	return(0);
}

//void TranslateBehavior(int planenum, int behavenum)
int TranslateBehavior(int planenum, int behavenum, int formnum)
{
	int returnval = 0;

	switch(behavenum) {

		case SA_FORMATION:
			returnval = 1;
			switch(formnum) {
			case SF_PARADE:
				bv[planenum].Behavior = Formation;
				bv[planenum].behaveflag &= ~COMBAT_SPREAD;
				bv[planenum].horizontalsep = PARADEDIST;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("PARADE",DEBUG_MSG);
#endif
				break;

			case SF_CRUISE:
				bv[planenum].Behavior = Formation;
				bv[planenum].behaveflag &= ~COMBAT_SPREAD;
				bv[planenum].horizontalsep = CRUISEDIST;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("CRUISE",DEBUG_MSG);
#endif
				break;

			case SF_COMBATSPREAD:
				bv[planenum].Behavior = Formation;
				bv[planenum].behaveflag |= COMBAT_SPREAD;
				bv[planenum].horizontalsep = COMBATSPREADDIST;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("COMBAT_SPREAD",DEBUG_MSG);
#endif
				break;

			case SF_BOMBER_FORMATION:
				bv[planenum].Behavior = Formation;
				bv[planenum].behaveflag &= ~COMBAT_SPREAD;
				bv[planenum].horizontalsep = BOMBERPARADEDIST;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("BOMBER_FORMATION",DEBUG_MSG);
#endif
				break;

			case SF_LADDER:
				bv[planenum].Behavior = Ladder;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("LADDER",DEBUG_MSG);
#endif
				break;

			case SF_BOX:
				bv[planenum].Behavior = Box;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("BOX",DEBUG_MSG);
#endif
				break;

			case SF_WALL:
				bv[planenum].Behavior = Wall;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("WALL",DEBUG_MSG);
#endif
				break;

			case SF_STRIKE:
				bv[planenum].Behavior = GorillaBomb;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("STRIKE",DEBUG_MSG);
#endif
				break;

			default:
				bv[planenum].Behavior = Formation;
				bv[planenum].behaveflag &= ~COMBAT_SPREAD;
				bv[planenum].horizontalsep = CRUISEDIST;
				bv[planenum].behaveflag &= ~RADARON;
				bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
				Message("DEFAULT FORM ",DEBUG_MSG);
#endif
				break;

			}
			break;

		case SA_CAP:
			bv[planenum].Behavior = SetUpCAP1;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("CAP",DEBUG_MSG);
#endif
			break;

		case SA_WAYPOINT:
			returnval = 1;
			bv[planenum].Behavior = FlyWayPoint;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("WAYPOINT",DEBUG_MSG);
#endif
			break;

		case SA_GORILLABOMB:
			bv[planenum].Behavior = AIBombTarget;
#ifdef SHOWBEHAVE
			Message("GORILLABOMB",DEBUG_MSG);
#endif
			break;

#ifdef ELFREFUEL
		case SA_REFUEL:
			returnval = 1;
			bv[planenum].Behavior = SetUpRefuel1;
//			bv[planenum].behaveflag &= ~RADARON;
//			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("Refuel",DEBUG_MSG);
#endif
			break;
#endif

		case SA_CRUISE_MISS:
			returnval = 1;
			bv[planenum].Behavior = FlyCruise;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("Cruise",DEBUG_MSG);
#endif
			break;

		case SA_WAYPOINT_ESCORTABLE:
			bv[planenum].Behavior = FlyWPFollowPlayer;
			bv[planenum].horizontalsep = BOMBERPARADEDIST;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("WAYPOINT",DEBUG_MSG);
#endif
			break;

		case SA_GORILLA_COVER:
			returnval = 1;
			bv[planenum].Behavior = GorillaCover;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("Gorilla Cover",DEBUG_MSG);
#endif
			break;

		case SA_SAR_CAP:
			bv[planenum].Behavior = SARCAP1;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("SARCAP ",DEBUG_MSG);
#endif
			break;

		case SA_AWACS:
//			bv[planenum].Behavior = FlyE2C;
			bv[planenum].Behavior = SetUpCAP1;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("FlyE2C ",DEBUG_MSG);
#endif
			break;

		case SA_SAR:
			returnval = 1;
			bv[planenum].Behavior = SARWait;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
			bv[planenum].behaveflag |= ISHELICOPTER;
			bv[planenum].threathead = 5;
			ps[planenum].status &= ~TAKEOFF;
#ifdef SHOWBEHAVE
			Message("SARCAP ",DEBUG_MSG);
#endif
			break;

		case SA_ENGAGETRAIN:
			bv[planenum].Behavior = EngageTrain;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("Engage Train ",DEBUG_MSG);
#endif
			break;

		default:
			bv[planenum].Behavior = FlyWayPoint;
			bv[planenum].behaveflag &= ~RADARON;
			bv[planenum].behaveflag |= ENEMYNOFIRE;
#ifdef SHOWBEHAVE
			Message("DEFAULT ",DEBUG_MSG);
#endif
			break;


	}
	return(returnval);
}

#ifdef ELFREFUEL
void FlyRefuel()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;

	/*  Check to see if player is near our plane */
	/*  If so adjust speed accordingly and do level */
	/*  flight                                   */

	i = planecnt;

	dx = Px - ps[i].x;
	dy = Py - ps[i].y;
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	if(tdist < 30) {
#if 0
		if(tempvar) {
			TXT("N ");
			TXN(i);
			TXA(" d ");
			TXN(tdist);
			TXA(", r ");
			TXN(tangle);
			Message(Txt,DEBUG_MSG);
		}
#endif
		HelpPlayerRefuel(tdist, tangle);
		return;
	}

	bv[i].basespeed = 250;

	dx = bv[i].xloc - ps[i].x;
	dy = bv[i].yloc - ps[i].y;

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	relangle = (tangle - ps[i].head) + 0x4000;

	if(distnm < 8)
		relangle += 0x2000;
	else if(distnm > 12)
		relangle -= 0x2000;

	TurnAIPlaneBy(i, relangle);

	if(ps[i].alt < 1250) {
		bv[i].basespeed = ptype->speed / 2;
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 1350) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if 0
	if(tempvar) {
/*		TXT("RFL ");
		TXN(i);
		TXA(" d ");
		TXN(distnm);
		TXA(", r ");
		TXN(relangle);  */
		sprintf(Txt, "RFL %x d %d, r %x, dr %x", i, distnm, relangle, droll);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void HelpPlayerRefuel(int pdist, int pangle)
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;

	i = planecnt;

	relangle = pangle - ps[i].head;
	if(((Knots + GetDAltSpeed(ps[i].alt)) > 200) && ((Knots + GetDAltSpeed(ps[i].alt)) < 300)) {
		if(abs(relangle) < 0x4000)
			bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) + 25;
		else {
			if(pdist < 3)
				bv[i].Behavior = HoldSpeedRefuel;
			else if(pdist < 7)
				bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 5;
			else if(abs(relangle) > 0x7000) {
				if(pdist < 12)
					bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 10;
				else
					bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 25;
			}
			else
				bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) + 25;
		}
	}


	droll = -ps[i].roll;

	if(ps[i].alt < 1250) {
		bv[i].basespeed = ptype->speed / 2;
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 1350) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if 0
	if(tempvar) {
		TXT("HPR ");
		TXN(i);
		TXA(" d ");
		TXN(pdist);
		TXA(", r ");
		TXN(relangle);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void HoldSpeedRefuel()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;

	i = planecnt;

	dx = Px - ps[i].x;
	dy = Py - ps[i].y;
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	if(tdist > 20)
		bv[i].Behavior = HelpPlayerRefuel;

	relangle = tangle - ps[i].head;

	droll = -ps[i].roll;

	if(ps[i].alt < 1250) {
		bv[i].basespeed = ptype->speed / 2;
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 1350) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if 0
	if(tempvar) {
		TXT("HSR ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		TXA(", r ");
		TXN(relangle);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void HelpAIRefuel()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;

	i = planecnt;

	bv[i].basespeed = 250;

	droll = -ps[i].roll;

	if(ps[i].alt < 1250) {
		bv[i].basespeed = ptype->speed / 2;
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 1350) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if 0
	if(tempvar) {
		TXT("HAI ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}
#endif

void FlyWayPoint()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	int wayval = 0;
	int wayval2 = 0;
	int temppath;
	long xval, yval;
	UWORD zval;
	long xval2, yval2;
	UWORD zval2;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;
	int aistation;
	struct PlaneData far *tptype;
	int timeval;


//	bv[i].basespeed = ptype->speed / 2;

	i = planecnt;

	if(bv[i].behavecnt < Gtime) {
		bv[i].behavecnt = 0;
	}

	if(AIRadarForwardCheck(i, 0, 1)) {
		tempgotcha = 10;
		gotcha (bv[i].target);
		tempgotcha = -999;
	}

	if((bv[i].groundt != -1) && ((int)bv[i].path != -1)) {
		temppath = (int)bv[i].path;
		wayval = bv[i].groundt;

		if((aipaths[temppath].wp0 + aipaths[temppath].numWPs) <= wayval) {
			GetWayPointChanges((aipaths[temppath].wp0 + aipaths[temppath].numWPs - 1), &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);

			if(speedval != -1) {
				bv[i].basespeed = speedval;
			}
			if(headval != -1) {
				relangle = headval - ps[i].head;
				TurnAIPlaneBy(i, relangle);
				if(!IsBoat(ps[i].type))
					bv[i].finhead = headval;
				else
					bv[i].finhead = ps[i].head;
			}
			else {
				droll = -ps[i].roll;
				bv[i].finhead = ps[i].head;
			}

			if(altval > 5) {
				if(ps[i].alt < altval) {
					if(ps[i].alt < (altval - 3)) {
//						pdest = 0x2000 - ps[i].pitch;
						pdest = 0x2000;
					}
					else {
//						pdest = 0x200 - ps[i].pitch;
						pdest = 0x200;
					}
				}
				else if(ps[i].alt > (altval + 25)) {
					if(ps[i].alt > (altval + 30)) {
						pdest = 0xD000 - ps[i].pitch;
						pdest = 0xD000;
					}
					else {
						pdest = 0xFE00 - ps[i].pitch;
						pdest = 0xFE00;
					}
				}
				else
					pdest = 0;
			}
			else
				pdest = 0;
			return;
		}
	}

	if((bv[i].groundt == -1) || (bv[i].path == -1)) {
		if(GetSquadLeader(i) != -1) {
			ps[i].status |= BACKTOBASE;
			bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
			TXT("No Waypoint , Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(i);
		}
		else
			bv[i].Behavior = Formation;
		return;
	}
	else {
		wayval = bv[i].groundt;

		GetWayXYZ(wayval, &xval, &yval, &zval);
		dx = (xval + 16) >> HTOV;
		dy = (yval + 16) >> HTOV;
		dx = dx - ps[i].x;
		dy = dy - ps[i].y;
	}

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	relangle = tangle - ps[i].head;

	if(tdist < 10) {
		temppath = (int)bv[i].path;
		timeval = GetWayPointChanges(wayval, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
		if(wpflag & DOWPLAND) {
			GetAIBase(planecnt);
			bv[planecnt].Behavior = AIGoHome;
			ps[planecnt].status |= BACKTOBASE;
		}
		if(wpflag & DOWPGOAL) {
			LogIt (SLG_WAY_PT_GOAL, planecnt, ps[planecnt].xL, ps[planecnt].yL, ps[planecnt].alt, -(int)bv[planecnt].path, wayval, 0L, LOG_AIR);
			DoGroupAction(planecnt, DOWPGOAL, wayval);
		}
		if(wpflag & DOWPHOVER) {
			bv[planecnt].Behavior = DoAltHover;
			bv[planecnt].varcnt = Gtime + timeval;
			return;
		}

		if((aipaths[temppath].wp0 + aipaths[temppath].numWPs) <= (wayval + 1)) {
#ifdef GOHOMEATEND
			if(GetSquadLeader(i) != -1) {
#ifdef SHOWHOME
				TXT("Last WP, Go Home ");
				TXN(i);
				TXA(" ");
				TXN(aipaths[temppath].wp0);
				TXA(" ");
				TXN(aipaths[temppath].numWPs);
				TXA(" ");
				TXN(wayval);
				Message(Txt,DEBUG_MSG);
#endif
				ps[i].status |= BACKTOBASE;
				bv[i].Behavior = AIGoHome;
				GetAIBase(i);
			}
			else {
				bv[i].Behavior = Formation;
			}
#else
			bv[i].groundt = wayval + 1;
			droll = -ps[i].roll;
			pdest = 0;
#endif
			return;
		}
		else {

			if(aiwaypoints[wayval].x == (long)-2)
				wayval2 = bv[i].groundt + 2;
			else
				wayval2 = bv[i].groundt + 1;

			bv[i].groundt = wayval2;
//			wayval2 = wayval + 1;
//			bv[i].groundt = wayval + 1;
			GetWayXYZ(wayval2, &xval2, &yval2, &zval2);
			bv[i].xloc = xval2;
			bv[i].yloc = yval2;
			if(DoNextWayPt(i, wayval2)) {
				if(GetSquadLeader(planecnt) == planecnt)
					ChangeGroupBehavior(planecnt, 1);
			}
		}
	}
	else {
		GetWayPointChanges(wayval, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
		if(speedval != -1) {
			bv[i].basespeed = speedval;
		}
		if((attack != -1) && (bv[i].behavecnt == 0)) {
			LaunchCruise(planecnt);
		}
		if(wpflag & DOWPDITCH) {
			if((ps[i].alt < 125) || (abs(relangle) > 0x4000)) {
        		ps[i].status |= CRASH;
			}
			else {
//				pdest = 0xD000 - ps[i].pitch;
				pdest = 0xD000;
				return;
			}
		}
		if(wpflag & DOWPEXPLODE) {
			LogIt (SLG_AIR_DESTROYED_MISSILE, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, -999, 0L, LOG_AIR);
			AirKill(i);
		}
		if(wpflag & DOWPTROOPDROP) {
			DoGroupAction(planecnt, DOWPTROOPDROP, 0);
		}
	}

	TurnAIPlaneBy(i, relangle);

	if(ps[i].alt < zval) {
		if(ps[i].alt < (zval - 5)) {
//			pdest = 0x2000 - ps[i].pitch;
			pdest = 0x2000;
		}
		else {
//			pdest = 0x200 - ps[i].pitch;
			pdest = 0x200;
		}
	}
	else if(ps[i].alt > (zval + 25)) {
		if(ps[i].alt > (zval + 30)) {
//			pdest = 0xD000 - ps[i].pitch;
			pdest = 0xD000;
		}
		else {
//			pdest = 0xFE00 - ps[i].pitch;
			pdest = 0xFE00;
		}
	}
	else
		pdest = 0;

	if(!IsBoat(ps[i].type))
		bv[i].finhead = tangle;
	else
		bv[i].finhead = ps[i].head;

#if 0
//#ifdef ELFBMSG
/*
	if((ps[i].type == 38) && (wayval != bv[i].groundt)) {
		TXT("New Way ");
		TXN(bv[i].groundt);
		Message(Txt,DEBUG_MSG);
	}
	if((ps[i].type == 38) && (tdist < 10)) {
		TXT("Should New Way ");
		TXN(bv[i].groundt);
		TXA(" ");
		TXN(wayval2);
		TXA(" ");
		Message(Txt,DEBUG_MSG);
	}
*/
	if((tempvar) && (i> LASTPLANE)) {
		TXT("FWP ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		TXA(", wp ");
		TXN(bv[i].groundt);
//		TXN(wayval2);
		TXA(", pd ");
		TXN(pdest);
		TXA(", alt ");
		TXN(ps[i].alt);
		TXA(", wa ");
		TXN(zval);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void ShowJettison(int planenum)
{
	int type;
	int weaponnum;
	int aistation;
    struct Sams *ssk;
    struct sam  *S;
	int  f;
	long dist;
	int  dpitch;
	int  BombAngle;
	int  otherplane;
	struct PlaneData far *ptargettype;

	int  doshowdrop = 0;
	int missilenum;

	if(bv[planenum].Behavior == FlyCruise)
		return;

	ptargettype = GetPlanePtr(ps[planenum].type);

	if((ps[planenum].alt > 5000) && ((ptargettype->gun == NO_GUN) || (ptargettype->gun == REAR_GUNS)))
		return;

	aistation = CheckAIBombs(planenum);

	if(aistation == -1)
	{
		return;
	}

	// Need to check this SCOTT

	doshowdrop = 1;

	dpitch = abs(ps[planenum].pitch);

	f = 32767-Isin(dpitch);
	BombAngle = dpitch + ( (1500L + (ps[planenum].alt/8)) *TrgMul(f,f) >>15);

	dist = (long)(((long) ps[planenum].alt * Icos(BombAngle)) /Isin(BombAngle));

	AIBOMBX = ps[planenum].xL + sinXL (ps[planenum].head, dist);
	AIBOMBY = ps[planenum].yL - cosXL (ps[planenum].head, dist);

	ps[planenum].mcnt[aistation] = 0;
	aistation = CheckAIBombs(planenum);
	if(aistation != -1)
		ps[planenum].mcnt[aistation] = 0;


	LetWingmenContinue(planenum);

	if((CheckAIMissiles(planenum) != -1) || (ptargettype->gun == FRONT_REAR_GUNS) || (ptargettype->gun == FRONT_GUNS)) {
		bv[planenum].Behavior = StraightAttack;
		bv[planenum].behaveflag &= ~ENEMYNOFIRE;
		GetEnemyTarget(planenum);
	}
	else {
		ps[planenum].status |= BACKTOBASE;
		bv[planenum].Behavior = AIGoHome;
#ifdef SHOWHOME
		TXT("Jettison No Missile, Going Home ");
		TXN(planenum);
		Message(Txt,DEBUG_MSG);
#endif
		GetAIBase(planenum);
	}

	weaponnum = 0;

	if(doshowdrop) {
    	type = ORDNANCE[1].samtype;

//		Message("Jettison ",DEBUG_MSG);

		missilenum = FindEmptyMissileSlot2 (0, NPSAM, planenum);
        firemissile2 (missilenum,
				-1,     /* So that it doesn't explode on Ground */
    			ps[planenum].xL,
       			ps[planenum].yL,
                ps[planenum].alt,
                ps[planenum].head,
                ps[planenum].pitch,
                ps[planenum].roll,
                (((ps[planenum].speed * FPStoKNTS)>>4)>>4) * 11,
                -1,    // used for air-air only !
                planenum,
                AIBOMBX,
                AIBOMBY,
				type,
				0);
	}
}

void SlewOurWingmen(long xslew, long yslew)
{
	int cnt;

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && ps[cnt].speed) { /* Planes in the air */
			if(GetSquadLeader(cnt) == -1) {
				ps[cnt].xL += xslew;
				ps[cnt].yL += yslew;
			}
		}
	}
}

#ifdef ELFREFUEL
int CheckDisengage(int planenum)
{
	if((CheckDetection(planenum) != -999) || (CheckDetection(bv[planenum].target) != -999)) {
		bv[planenum].Behavior = ReleaseRefuel;
		return(1);
	}
	return(0);
}

void GetRefuelPlane()
{
	int cnt;

	i = planecnt;

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((bv[cnt].Behavior == FlyRefuel) || (bv[cnt].Behavior == HoldSpeedRefuel)) {
			bv[i].target = cnt;
			bv[i].Behavior = AIRefuel;
			break;
		}
	}
	if(cnt == NPLANES)
		return;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((bv[cnt].Behavior == AIRefuel) && (bv[cnt].target == bv[i].target) && (cnt != i)) {
			if(cnt == bv[i].leader) {
				bv[i].Behavior = AIRefuel;
				bv[cnt].Behavior = AIWaitRefuel;
			}
			else
				bv[i].Behavior = AIWaitRefuel;
			break;
		}
	}
}

void AIRefuel()
{
	int dx, dy, dalt;
	int refuelplane;
	int tangle;
	COORD tdist;

	i = planecnt;

	if(CheckDisengage(i))
		return;

	refuelplane = bv[i].target;
	dx = ps[refuelplane].x;
	dy = ps[refuelplane].y;
	dalt = ps[refuelplane].alt - 30;

	if(dalt < 5)
		dalt = 5;

	FlyTo(dx, dy, dalt, 1);

	tdist = Dist2D((ps[i].x - dx), (ps[i].y - dy));

	if(ps[i].alt < dalt) {
//		pdest = 0x2000 - ps[i].pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > (dalt + 2)) {
//		pdest = 0xD000 - ps[i].pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

	tangle = angle((ps[i].x - dx), -(ps[i].y - dy)) - ps[i].head;


	if(tdist > 30) {
		bv[i].basespeed = 350;
	}
	else {
		if(abs(tangle) < 0x4000)
			bv[i].basespeed = bv[refuelplane].basespeed - 15;
		else if(tdist > 7) {
			bv[i].basespeed = bv[refuelplane].basespeed + 35;
			bv[refuelplane].Behavior = HelpAIRefuel;
		}
		else
			bv[i].basespeed = bv[refuelplane].basespeed + 20;
	}

	if((tdist < 4) && (abs(dalt - ps[i].alt) < 5))
		bv[i].Behavior = AILockedRefuel;

#if 0
	if(tempvar) {
//		TXT("AIR ");
//		TXN(i);
//		TXA(" d ");
//		TXN(tdist);
		sprintf(Txt, "AIR %d, d %d, droll %x, bs %d", i, tdist, droll, bv[i].basespeed);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void AIWaitRefuel()
{
	int cnt;
	int linepos = 1;
	int dx, dy, dalt;
	int refuelplane;
	int refuelhead;
	int holddist;
	COORD tdist;

	i = planecnt;

	if(CheckDisengage(i))
		return;

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if(cnt == i)
			break;
		else if(bv[cnt].Behavior == AIWaitRefuel) {
			linepos ++;
		}
	}

	refuelplane = bv[i].target;
	refuelhead = ps[refuelplane].head;
	refuelhead -= 0x6000;
	holddist = linepos * 20;

	dx = ps[refuelplane].x + sinX(refuelhead, holddist);
	dy = ps[refuelplane].y - cosX(refuelhead, holddist);
	dalt = ps[refuelplane].alt;

	FlyTo(dx, dy, dalt, 1);

	tdist = Dist2D((ps[i].x - dx), (ps[i].y - dy));
	if(tdist > 30)
		bv[i].basespeed = 350;
	else if(tdist > 3)
		bv[i].basespeed = bv[refuelplane].basespeed + 25;
	else
		bv[i].basespeed = bv[refuelplane].basespeed;

#if 0
	if(tempvar) {
		TXT("RW ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

#if 0
#define REFUELACC 3;
void AILockedRefuel()
{
	int dx, dy, dalt;
	int refuelplane;
	int refuelhead;
	int probedist;
	COORD tdist;
	int fueladd;

	i = planecnt;

	if(CheckDisengage(i))
		return;

	refuelplane = bv[i].target;
	refuelhead = ps[refuelplane].head;
	probedist = 3;

	ps[i].head = refuelhead;
	refuelhead += 0x8000;

	ps[i].x = ps[refuelplane].x + sinX(refuelhead, probedist);
	ps[i].y = ps[refuelplane].y - cosX(refuelhead, probedist);
	ps[i].xL = (long)ps[i].x<<HTOV;
	ps[i].yL = (long)ps[i].y<<HTOV;
	ps[i].alt = ps[refuelplane].alt - 15;
	ps[i].roll = ps[refuelplane].roll;

	bv[i].basespeed = 1;
	ps[i].speed = 1;
	fueladd = (REFUELACC * 16) / FrameRate;
	if(fueladd <= 0)
		fueladd = 1;
	bv[i].fueltank += fueladd;
	if(bv[i].fueltank > 18000)
		bv[i].Behavior = ReleaseRefuel;
#if 0
	if(tempvar) {
		TXT("LR ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}
#else
void AILockedRefuel()
{
	int dx, dy, dalt;
	int refuelplane;
	int tangle;
	COORD tdist;
	int fueladd;

	i = planecnt;

	if(CheckDisengage(i))
		return;

	refuelplane = bv[i].target;
	dx = ps[refuelplane].x;
	dy = ps[refuelplane].y;
	dalt = ps[refuelplane].alt - 30;

	if(dalt < 5)
		dalt = 5;

	FlyTo(dx, dy, dalt, 1);

	tdist = Dist2D((ps[i].x - dx), (ps[i].y - dy));

	pdest = 0;

	tangle = angle((ps[i].x - dx), -(ps[i].y - dy)) - ps[i].head;

	if(tdist > 30)
		bv[i].basespeed = 350;
	else {
		if(abs(tangle) < 0x4000)
			bv[i].basespeed = bv[refuelplane].basespeed - 50;
		else if(tdist > 7)
			bv[i].basespeed = bv[refuelplane].basespeed + 30;
		else if(tdist > 3)
			bv[i].basespeed = bv[refuelplane].basespeed + 20;
		else if(tdist > 1)
			bv[i].basespeed = bv[refuelplane].basespeed;
		else
			bv[i].basespeed = bv[refuelplane].basespeed - 3;
	}

	fueladd = (REFUELACC * 16) / FrameRate;
	if(fueladd <= 0)
		fueladd = 1;
	bv[i].fueltank += fueladd;
	if(bv[i].fueltank > 18000) {
		bv[i].Behavior = ReleaseRefuel;
	}
#if 0
	if(tempvar) {
		TXT("LR ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}
#endif

void ReleaseRefuel()
{
	int dx, dy, dalt;
	int refuelplane;
	COORD tdist;
	int ourleader;

	i = planecnt;

	refuelplane = bv[i].target;
	dx = ps[refuelplane].x;
	dy = ps[refuelplane].y;
	dalt = ps[refuelplane].alt;

	droll = 0;
	pdest = 0;
	bv[i].basespeed = bv[refuelplane].basespeed - 25;
	tdist = Dist2D((ps[i].x - dx), (ps[i].y - dy));
	if(tdist > 30) {
		if(CheckDisengage(i)) {
			bv[i].Behavior = StraightAttack;
		}
		else {
			ourleader = bv[i].leader;
			if(ourleader == -999) {
				bv[i].Behavior = FlyWayPoint;
				if(bv[i].groundt == -1)
					bv[i].groundt = 0;
				NextRefuel();
			}
			else {
				TranslateBehavior(i, SA_FORMATION, SF_CRUISE);
				if(bv[ourleader].Behavior == AIWaitRefuel)
					bv[ourleader].Behavior = AIRefuel;
				else
					NextRefuel();
			}
		}
	}
	else if((tdist < 7) && (tdist > 4))
		bv[refuelplane].Behavior = FlyRefuel;

#if 0
	if(tempvar) {
		TXT("RR ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void NextRefuel()
{
	int cnt;
	int wingpos;

	i = planecnt;

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((bv[cnt].Behavior == AIWaitRefuel) && (bv[cnt].target == bv[i].target)) {
			wingpos = cnt;
			while(bv[wingpos].wingman >= 0)
				wingpos = bv[wingpos].wingman;

			if(bv[wingpos].Behavior != AIWaitRefuel);
				wingpos = cnt;
			bv[wingpos].Behavior = AIRefuel;
			break;
		}
	}
}
#endif

void DoHover()
{
	droll = -ps[i].roll;
	bv[i].basespeed = 0;
	pdest = 0;
}

void GetAIBase(int ainum)
{
    int     b, r, j;
    COORD   d, tdist;

	if(ps[ainum].base < 0)
		return;

    tdist = 0x7fff;
    for (j=0; j<NRADAR; j++) {
        if (((Rdrs[j].status&(AIRBASE+DESTROYED))==AIRBASE)
				&& ((Rdrs[j].status & (FRIENDLY|NEUTRAL)) == (FriendStatus(ainum)))){
            d =xydist (ps[ainum].x-Rdrs[j].x, ps[ainum].y-Rdrs[j].y);

            if (d<tdist) {
                tdist = d;
                b  = j;
            }
        }
    }
	if(tdist != 0x7fff)
		ps[ainum].base = b;
}

void AIGoHome()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	COORD bxloc, byloc;
	UWORD bzloc;
	int   bhead;
	int   bret;
	int   dalt;
	int   dist;

	i = planecnt;

	bret = ReturnBaseInfo(ps[i].base, &bxloc, &byloc, &bzloc, &bhead);
	if(bret) {
		if(ps[planecnt].base == ((whichourcarr + 1) * -1))
			dist = 225;
		else
			dist = 500;
		dx = bxloc + sinX((bhead + 0x8000), dist);
		dy = byloc - cosX((bhead + 0x8000), dist);
	}
	else {
		dx = bxloc;
		dy = byloc + 500;
	}

	dalt = ps[i].alt;
	if(dalt < 500) {
		dalt = 500;
	}

	if(FlyTo(dx, dy, dalt, 0)) {
		if(ps[planecnt].base == ((whichourcarr + 1) * -1)) {
#ifdef AILANDDEBUG
			TXT("Requesting Landing ");
			TXN(i);
			TXA(" ");
			TXN(Gtime);
			Message(Txt, DEBUG_MSG);
#endif
	        ps[i].status |= LANDING;
			CheckPlaneLand(i);
			bv[planecnt].Behavior = AICarrierLand;
			GetPlaneLanding();
		}
		else {
			bv[i].Behavior = AIGoHome2;
		}
	}

}

void AIGoHome2()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	COORD bxloc, byloc;
	UWORD bzloc;
	int   bhead;
	int   bret;
	int   dalt;

	i = planecnt;

	bret = ReturnBaseInfo(ps[i].base, &bxloc, &byloc, &bzloc, &bhead);
	if(bret) {
		dx = bxloc + sinX((bhead + 0x8000) ,225);
		dy = byloc - cosX((bhead + 0x8000), 225);
	}
	else {
		dx = bxloc;
		dy = byloc + 225;
	}

	dalt = ps[i].alt;
	if(dalt < 500) {
		dalt = 500;
	}

	if(FlyTo(dx, dy, dalt, 0)) {
		bv[i].Behavior = FlyWayPoint;
        ps[i].status |= LANDING;
	}

}

void CheckAISix(int wingnum)
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1, threatdist;
	int threatcone;
    char Str[80];

	i = planecnt;

	x = ps[wingnum].x;
	y = ps[wingnum].y;
	winghead = ps[wingnum].head;
	threatcone = 0x1000 + ((MAXWINGLEVEL - bv[i].winglevel) * 545);
	threatdist = 5 + (MAXWINGLEVEL - bv[i].winglevel);  /*  Do we want to have this ramped on level?  */


	founddist = 0;
	targetnum = -999;
	tdist = 0;

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & CRASH)) &&
				(ps[cnt].speed) && (bv[cnt].target == wingnum)) { /* Planes in the air */
			if(!FriendMatch(planecnt, cnt)) {
		    	dx = x- ps[cnt].x;
    			dy = y- ps[cnt].y;
				GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
				rangle = tangle - ps[cnt].head;
				rrangle = (0x8000 + tangle) - winghead;
       			if ((((tdist1<tdist) || ((tdistnm < threatdist) && (!founddist))) && (abs(rangle) < 0x2000)
						&& (abs(rrangle) > threatcone))) {
					founddist = 1;
           			tdist=tdist1;
					targetnum = cnt;
					rangle1 = rangle;
					rrangle1 = rrangle;
					tdistnm1 = tdistnm;
       			}
			}
		}
	}
	if(founddist) {
		if(whichdefensethreat[wingnum] != targetnum) {
			whichdefensethreat[wingnum] = targetnum;
			GetWingName(wingnum, Str);
			strcat (Str, "Engaged Defensive");
			Message(Str,WINGMAN_MSG);
			Speech(ENGAGE_DEF);
		}
	}

}

void GorillaBomb()
{
	long dx, dy;
	int dz;
	int relangle;
	int tangle, tpitch, troll, enemyalt, prewing, temphead, secplace;
	int tempwing, prespeed, tempspeed;
	COORD tdist, distkm, tdistnm;
	int offangle;
	int hsep;
	int ourleadalt, tempalt;
	int dpwhead;
	long xval, yval;
	UWORD zval;
	int coverplane;

	i = planecnt;

//	prewing = GetSquadLeader(i);
	prewing = GetBehaviorLeader(i);

	secplace = (bv[i].behaveflag & 15);

	offangle = 0x4000;

	prespeed = 0;
	if(prewing != i) {
		prespeed = bv[prewing].basespeed;
	}

	dpwhead = bv[prewing].finhead - ps[prewing].head;

	if((prewing == i) || (secplace <= 1) || (abs(dpwhead) > 0x600)|| ((bv[i].prevsec < 0) && (bv[i].leader < 0))) {
/*		if(ptype->speed > 450)
			tempspeed = 400;
		else
			tempspeed = 300;  I can't remember why I did this  SRE  */

		if((bv[i].behaveflag & ENEMYNOFIRE) || (CheckAIBombs(i) != -1)) {
			if(prewing != i) {
				bv[i].groundt = bv[prewing].groundt;
				GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
				bv[i].xloc = xval;
				bv[i].yloc = yval;
			}
			FlyWayPoint();
			if(prewing != i)
				if((bv[i].Behavior != GorillaBomb) && (bv[prewing].behaveflag & ENEMYNOFIRE)) {
					bv[i].Behavior = GorillaBomb;
				}
		}
		else
			StraightAttack();

		if(prespeed)
			bv[i].basespeed = prespeed - 20;

		return;
	}
	else if(bv[i].prevsec >= 0) {
		tempwing = bv[i].prevsec;
		offangle = 0x8000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 150;  /*  600 ft put last to check formation */
#else
		hsep = 4320;  /*  3 nm  */
#endif
	}
	else if(bv[i].leader >= 0){
		tempwing = bv[i].leader;
		offangle = -0x4000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 100;  /*  400 ft put last to check formation  */
#else
		hsep = 2880;  /*  2 nm  */
#endif
	}
	else {
		FlyWayPoint();
		return;
	}

#if 0
	if(bv[tempwing].Behavior == AIBombTarget) {
		bv[i].Behavior = AIBombTarget;
		bv[i].groundt = bv[prewing].groundt;
		bv[i].xloc = bv[prewing].xloc;
		bv[i].yloc = bv[prewing].yloc;
		coverplane = bv[tempwing].target;
		if(coverplane >= 0) {
			if(bv[coverplane].Behavior == GorillaCover) {
				bv[coverplane].Behavior = SetUpCAP1;
				bv[coverplane].xloc = ps[coverplane].xL;
				bv[coverplane].yloc = ps[coverplane].yL;
				bv[coverplane].groundt = -1;
				UpdateInterceptGroup(coverplane, 1);  /*  Might Need to do base here  */
			}
		}
	}
	else
#endif

	if((prewing != i) && (bv[tempwing].groundt >= 0))
		bv[i].groundt = bv[tempwing].groundt;

	if((bv[tempwing].Behavior != GorillaBomb) && (bv[tempwing].Behavior != AIBombTarget) && (!(bv[i].behaveflag & ENEMYNOFIRE))) {
		if((bv[i].behaveflag & ENEMYNOFIRE) || (CheckAIBombs(i) != -1)) {
			if((prewing != i) && (bv[tempwing].groundt >= 0)) {
				bv[i].groundt = bv[tempwing].groundt;
				GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
				bv[i].xloc = xval;
				bv[i].yloc = yval;
			}
			bv[i].Behavior = FlyWayPoint;
			DoNextWayPt(i, bv[i].groundt);
		}
	}

//	dx = ps[tempwing].x + sinX(temphead, hsep);
//	dy = ps[tempwing].y - cosX(temphead, hsep);
	dx = ailoc[tempwing].xloc + sinXL(temphead, ((long)hsep * AILOCDIV));
	dy = ailoc[tempwing].yloc - cosXL(temphead, ((long)hsep * AILOCDIV));
	ourleadalt = ps[tempwing].alt;

	tempalt = ourleadalt + bv[i].verticlesep;

	if(tempalt < 75)
		tempalt = 75;

	if((GLowDroll) && (!(ps[i].status & TAKEOFF))) {
		ps[i].status |= JUSTPUT;
		JustPutHere(i, dx, dy, tempalt, tempwing);
		return;
	}

	LFlyTo(dx, dy, tempalt, tempwing, prespeed);

#if 0
//    #ifdef ELFBMSG
	if(tempvar) {
		TXT("GB ");
		TXN(i);
		TXA(" pw ");
		TXN(tempwing);
		TXA(", ps ");
		TXN(bv[prewing].basespeed);
		TXA(", os ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

int Langle(long x, long y)
{

	if((absL(x) + absL(y)) > 0x3FFF) {
//		if(tempvar)
//			Message("Down Shift",DEBUG_MSG);
		x >>= 8;
		y >>= 8;
	}
	if((absL(x) + absL(y)) > 0x3FFF) {
//		if(tempvar)
//			Message("Down Shift2",DEBUG_MSG);
		x >>= 8;
		y >>= 8;
	}
	return(ARCTAN((int) x, (int) y));
}

void LGetTargADP(int planenum, long dx, long dy, COORD *tdist, int *tpitch, int *tangle, COORD *tdistnm, int *enemyalt, int deltalt, long *ldist)
{
	int holddist;
	int tdx, tdy;
	int junkangle, junkangle2;
	long holddist2;

	tdx = (int)(((dx+16)/AILOCDIV) >> HTOV);
	tdy = (int)(((dy+16)/AILOCDIV) >> HTOV);

	*ldist = 0L;
	*tdist = holddist = Dist2D(tdx, tdy);
	holddist2 = *ldist = LDist2D(dx, dy);

	if(bv[planenum].target == -1) {
		*tpitch = GetAltPitch(ps[planenum].alt, (Alt + deltalt), *tdist);
		*enemyalt = Alt;
	}
	else {
		*tpitch = GetAltPitch(ps[planenum].alt, (ps[bv[planenum].target].alt + deltalt), *tdist);
		*enemyalt = ps[bv[planenum].target].alt;
	}
	junkangle = *tangle = Langle(dx, -dy);
	junkangle2 = ARCTAN(tdx, tdy);
	*tpitch = rng2(*tpitch, -32<<8, 16<<8);
	*tdistnm = (*tdist * 2) / 95;
#if 0
	if((tempvar) && (GetSquadLeader(planenum) == -1)) {
		sprintf(Txt, "LGT a %x, la %x, d %d, ld %ld", junkangle2, junkangle, holddist, holddist2);
		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "tx %d lx %ld, ty %d ly %ld", tdx, dx, tdy, dy);
		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "%d x %ld, y %ld", planenum, ailoc[planenum].xloc, ailoc[planenum].xloc);
		Message(Txt,DEBUG_MSG);
//		sprintf(Txt, "LGTs tx %d, dx %ld, ty %d, dy %ld", tdx, dx, tdy, dy);
//		Message(Txt,DEBUG_MSG);
	}
#endif
}

void LFlyTo(long xdim, long ydim, int altdim, int prewing, int prespeed)
{

	long dx, dy;
	int dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm, tdist2;
	long ldist;
	int wingman;
	int alttemp;
	int leadhead;
	long prespeedL;
	int showwww;
	int leadpitch;
	int endpitch;

	relangle = -999;
	rrelangle = -999;
	i = planecnt;

	dx = xdim;
	dy = ydim;

	dx = dx - ailoc[i].xloc;
	dy = dy - ailoc[i].yloc;

	LGetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0, &ldist);

	relangle = tangle - ps[i].head;

	TurnAIPlaneBy(i, relangle);

	alttemp = ps[i].alt;

	if(prewing == -999)
		leadpitch = 0;
	else if(prewing == -1)
		leadpitch = OurPitch;
	else
		leadpitch = ps[prewing].pitch;

	if(alttemp < (altdim - 3)) {
		if(alttemp < (altdim - 250))
			endpitch = 0x2000 + leadpitch;
		else if(alttemp < (altdim - 15))
			endpitch = 0x800 + leadpitch;
		else
			endpitch = 0x100 + leadpitch;
		if(endpitch > 0x3800)
			endpitch = 0x3800;
//		pdest = endpitch - ps[i].pitch;
		pdest = endpitch;
	}
	else if(alttemp > (altdim + 3)) {
		if(alttemp > (altdim + 250))
			endpitch = 0xD000 + leadpitch;
		else if(alttemp > (altdim + 15))
			endpitch = 0xF800 + leadpitch;
		else
			endpitch = 0xFF00 + leadpitch;
		if(endpitch < -0x3800)
			endpitch = -0x3800;
//		pdest = endpitch - ps[i].pitch;
		pdest = endpitch;
	}
	else
//		pdest = 0;
		pdest = leadpitch;

	if(prespeed) {
		tdist2 = tdist;
		if((ldist/AILOCDIV) < 32000L) {
#if 0
//			tdist2 = ldist;
			tdist2 = abs(cosX(relangle, (int)(ldist)));
			tdist2 /= AILOCDIV;
#endif
			tdist2 = (int)(ldist/AILOCDIV);
			if(tdist2 > 15) {
				ps[i].status &= ~LOWDROLL;
			}
#if 0 /* AAA */
			if((tempvar) && (prewing == -1)) {
				sprintf(Txt, "i %d, rax %x, ld %d, dy %d", i, relangle, ldist, tdist2);
				Message(Txt,DEBUG_MSG);
			}
#endif
		}
		else {
			tdist2 = 4000;
		}
		FormationSpeedAdjustUS(prespeed, relangle, tdist2, prewing);
	}

	if((tdist < 5) && (prewing != -999)) {
		if(prewing == -1)
			leadhead = OurHead;
		else
			leadhead = ps[prewing].head;
		dx = xdim + sinX(leadhead,5000);
   		dy = ydim - cosX(leadhead,5000);
		dx = dx - ailoc[i].xloc;
		dy = dy - ailoc[i].yloc;
		tangle = Langle(dx, -dy);
		rrelangle = relangle;
		relangle = tangle - ps[i].head;
		TurnAIPlaneBy(i, relangle);
	}

#if 0 /* AAA */
	if(tempvar) {
/*		TXT("FT ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		TXA(" b ");
		TXN(ps[i].base);  */
		sprintf(Txt, "LFT i %d, d %d, ta %x, ra %x, rra %x", i, tdist, tangle, relangle, rrelangle);
//		sprintf(Txt, "LFT i %d, at %d, ad %d, A %d, pd %x", i, alttemp, altdim, Alt, pdest);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

long LDist2D(long x, long y)
{
	int shifthold = 0;
	long returnval;
	long ox, oy;
	long doshift = 0x3FFFL;

	ox = x;
	oy = y;
	if((absL(x) + absL(y)) > doshift) {
		x >>= 8;
		y >>= 8;
		shifthold = 8;
	}
	if((absL(x) + absL(y)) > doshift) {
		x >>= 8;
		y >>= 8;
		shifthold = 16;
	}
	if(shifthold) {
		returnval = (long)(Dist2D((int) x, (int) y));
		returnval <<= shifthold;
	}
	else
		returnval = (long)(Dist2D((int) x, (int) y));

#if 0
	if((tempvar) && (i == 3)) {
		sprintf(Txt, "HHH x %ld %ld, y %ld %ld, rv %ld", x, ox, y, oy, returnval);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(returnval);
}

long absL(long x)
{
    return(x>0 ? x:-x);

}

#if 1
long sinXL(int tangle, long idist)
{
	long holdval;
	long tdist;
	int cnt = 0;
	int shwnum;

	i = planecnt;

	holdval = 0L;
	tdist = idist;
	if(idist < 0) {
//		Message("Sin Goof",DEBUG_MSG);
		sprintf(Txt, "%d Sin Goof d %ld, a %x", planecnt, idist, tangle);
		Message(Txt,DEBUG_MSG);
	}
	while(tdist > 16000L) {
		holdval += (long)sinX(tangle, 16000);
		tdist -= 16000L;
		cnt ++;
	}
	holdval += (long)sinX(tangle, (int)tdist);
#if 0
//	if((i == 2) && tempvar) {
	shwnum = Rtime & 0x7F;
	if(i == shwnum) {
		TXT("Sin ");
		TXN(planecnt);
		TXA(" cnt ");
		TXN(cnt);
		Message(Txt,DEBUG_MSG);
	}
#endif
	return(holdval);
}

long cosXL(int tangle, long idist)
{
	long holdval;
	long tdist;
	int cnt = 0;

	i = planecnt;

	holdval = 0L;
	tdist = idist;

	if(idist < 0) {
//		Message("Cos Goof",DEBUG_MSG);
		sprintf(Txt, "%d Cos Goof d %ld, a %x", planecnt, idist, tangle);
		Message(Txt,DEBUG_MSG);
	}

	while(tdist > 16000L) {
		holdval += (long)cosX(tangle, 16000);
		tdist -= 16000L;
		cnt ++;
	}
	holdval += (long)cosX(tangle, (int)tdist);
#if 0
	if((i == 2) && tempvar) {
//		TXT("HereC ");
//		TXN(cnt);
//		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "c %d ta %x, id %ld, td %ld, hv %ld", cnt, tangle, idist, tdist, holdval);
		Message(Txt,DEBUG_MSG);
	}
#endif
#if 0
	if(cnt > 1) {
		TXT("Cos ");
		TXN(planecnt);
		TXA(" cnt ");
		TXN(cnt);
		Message(Txt,DEBUG_MSG);
	}
#endif

	return(holdval);
}
#else
long sinXL(int tangle, long idist)
{
	long holdval;
	long tdist;
	int cnt = 0;

	i = planecnt;

	holdval = 0L;
	tdist = idist;
	while(tdist > 16000L) {
		holdval += sinX(tangle, 16000);
		tdist -= 16000L;
		cnt ++;
	}
	holdval += sinX(tangle, (int)tdist);
#if 0
	if((i == 2) && tempvar) {
		TXT("Here2 ");
		TXN(cnt);
		Message(Txt,DEBUG_MSG);
	}
#endif
	return(holdval);
}

long cosXL(int tangle, long idist)
{
	long holdval;
	long tdist;
	int cnt = 0;

	i = planecnt;

	holdval = 0L;
	tdist = idist;
	while(tdist > 16000L) {
		holdval += cosX(tangle, 16000);
		tdist -= 16000L;
		cnt ++;
	}
	holdval += cosX(tangle, (int)tdist);
#if 0
	if((i == 2) && tempvar) {
//		TXT("HereC ");
//		TXN(cnt);
//		Message(Txt,DEBUG_MSG);
		sprintf(Txt, "c %d ta %x, id %ld, td %ld, hv %ld", cnt, tangle, idist, tdist, holdval);
		Message(Txt,DEBUG_MSG);
	}
#endif
	return(holdval);
}
#endif

#ifdef ELFREFUEL
#define REFUELL 676  /*  half CAP length of 7.5 nm  */
#define REFUELW 180   /*  half CAP width of 2 nm  */
#define REFUELD 900
#define REFUELALT 2500
void SetUpRefuel1()
{
	long xval, yval;
	UWORD zval;

	i = planecnt;

	if(bv[i].groundt != -1) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}
	if(SetUpOval1(REFUELALT, REFUELW, REFUELL))
		bv[i].Behavior = SetUpRefuel2;
}

void SetUpRefuel2()
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;

	i = planecnt;

	wingnum = -999;
	if(bv[i].leader > -1)
		wingnum = bv[i].leader;
	else if(bv[i].wingman > -1)
		wingnum = bv[i].wingman;
	if(SetUpOval2(REFUELALT, REFUELW, REFUELL)) {
		bv[i].Behavior = RefuelLeg1;
		bv[i].basespeed = 250;
	}
	CheckHelpPlayer();
}

void RefuelTurn1()
{
	int dx, dy, dalt;
	int temphead, temphead2;

	i = planecnt;

	if(OvalTurn1(REFUELALT, REFUELW, REFUELL))
		bv[i].Behavior = RefuelLeg2;
	CheckHelpPlayer();
}

void RefuelTurn2()
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int wingnum;

	i = planecnt;

	wingnum = -999;
	if(bv[i].leader > -1)
		wingnum = bv[i].leader;
	else if(bv[i].wingman > -1)
		wingnum = bv[i].wingman;

	if(OvalTurn2(REFUELALT, REFUELW, REFUELL)) {
		bv[i].Behavior = RefuelLeg1;
		bv[i].basespeed = 250;
	}
	CheckHelpPlayer();

}

void RefuelLeg1()
{
	int wingnum;

	i = planecnt;

	if(CAPAI(0, REFUELALT, REFUELW, REFUELL, REFUELD)) {
		wingnum = -999;
		if(bv[i].leader > -1)
			wingnum = bv[i].leader;
		else if(bv[i].wingman > -1)
			wingnum = bv[i].wingman;

		bv[i].Behavior = RefuelTurn1;
		bv[i].basespeed = 250;
	}
	bv[i].basespeed = 250;
	CheckHelpPlayer();
#if 1
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPLeg1 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void RefuelLeg2()
{
	int wingnum;

	i = planecnt;

	if(CAPAI(1, REFUELALT, REFUELW, REFUELL, REFUELD)) {
		bv[i].Behavior = RefuelTurn2;
		bv[i].basespeed = 250;
	}
	CheckHelpPlayer();

#if 1
//#ifdef ELFBMSG
//	if((tempvar) && (i < 9)) {
	if(tempvar) {
		TXT("CAPLeg2 ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void CheckHelpPlayer()
{
	long dx, dy;
	int dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	long ldist;

	/*  Check to see if player is near our plane */
	/*  If so adjust speed accordingly and do level */
	/*  flight                                   */

	i = planecnt;

	dx = FPARAMS.X - ailoc[i].xloc;
	dy = FPARAMS.Z - ailoc[i].yloc;
	LGetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0, &ldist);

	if(tdist > 30) {
		return;
	}

	relangle = tangle - ps[i].head;
	if(((Knots + GetDAltSpeed(ps[i].alt)) > 200) && ((Knots + GetDAltSpeed(ps[i].alt)) < 300)) {
		if(abs(relangle) < 0x4000)
			bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) + 25;
		else {
			if(ldist < (20 * AILOCDIV))
//				bv[i].Behavior = HoldSpeedRefuel;
				bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) + (ldist / AILOCDIV) - (5 + 20);
			else if(ldist < (224 * AILOCDIV))
				bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 5;
			else if(abs(relangle) > 0x7000) {
				if(ldist < ((long)1920 * AILOCDIV))
					bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 10;
				else
					bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) - 25;
			}
			else
				bv[i].basespeed = Knots + GetDAltSpeed(ps[i].alt) + 25;
		}
	}

#if 0
	if(tempvar) {
		TXT("HPR ");
		TXN(i);
		TXA(" d ");
		TXN((int)(ldist/AILOCDIV));
		TXA(", r ");
		TXN(relangle);
		TXA(" ");
		TXN(bv[i].basespeed);
		TXA(" ");
		TXN(Knots);
		Message(Txt,DEBUG_MSG);
	}
#endif
}
#endif

int FirstTargInGroup(int prewing)
{
	int wingcnt;
	int prewing2;

	wingcnt = prewing;
	prewing2 = prewing;
	while (prewing2 >= 0) {
		while (wingcnt >= 0) {
				if(CheckAnyEnemyLock(wingcnt)) {
					return(wingcnt);
				}
				wingcnt = bv[wingcnt].wingman;
		}
		prewing2 = bv[prewing2].nextsec;
		wingcnt = prewing2;
	}
	return(-1);
}


