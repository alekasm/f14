//***************************************************************************
//*  File:   Planes3.c                              */
//*  Author: Scott Elson                             */
//*                                                */
//*  Game logic for Fleet Defender: plane behaviors          */
//
//
//
//
//***************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <bios.h>
#include <dos.h>
#include "library.h"
#include "planeai.h"
#include "world.h"
#include "behave.h"
#include "common.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "speech.h"
#include "armt.h"
#include "proto.h"
#include "setup.h"
#include "carr.h"


#define CHECKBASES	75
int targetplayer = -999;
int timetargetplayer = 0;
char spotflags[MAXPLANE + 1];

extern  int 	MissionWeather;
extern int     orgPlayernum;
extern UBYTE lastadded;
extern long oldFPX;
extern long oldFPZ;
extern UWORD oldAltval;
extern  int     olddog;
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
extern  char OurWingName[];

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
extern	int		LastCall;
extern  RPS     *Rp3D;
extern	int 	AIWingMan;
extern	int		closeplane;


extern struct PlaneData far *ptype;
extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);


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
extern    int lastcatused;
extern    long     AIBOMBX;     // 20-bit Scott-style X position of predicted Bomb impact
extern    long     AIBOMBY;     // 20-bit Scott-style Y position of predicted Bomb impact
extern    UWORD    AITTGT;      // 2 * Time to target in seconds
extern    UWORD    AITREL;      // 2 * Time to target intercept (bomb release) in seconds
extern    UWORD    AIRTGT;      // ground range to target in tenths of nautical miles
extern    int		 OtherAIDo;
extern    int		 OtherAITime;
extern    int		 WingID[];
extern    struct _path_ aipaths[];
extern    struct _wayPoint_ aiwaypoints[];
extern    struct _action_ aiactions[];
extern    int   PlanesLaunched;
extern	int	    Fticks;
extern  int     Speedy;
extern  int     whichourcarr;     // holds value for which boat our carrier
extern	 confData ConfigData;

extern struct FPARAM {
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

carriers far carrier;

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
   };

extern struct landpt LandPat[];

extern struct landpt NiteLandPat[];

/*************************************************************************
* void  InterceptControl(plane leader to intercept)                      *
*                                                                        *
*	The idea is that when an aircraft is detected we check to see if it  *
*	has been detected before.  If it has then we make sure that the      *
*	the plane that was set to intercept it is still active, and still    *
*   going after the detected group.  We will only check this once every  *
*   so often so we don't want to bog down the system.  We will want to   *
*   make sure that :
*			1.	The intercepting aircraft is still going after the group *
*			2.	The intercepting aircraft is alive						 *
*			3.	The intercepting aircraft is not returning to base.      *
*			4.	If the group is returning home and it is close to base   *
*				and the intercepting aircraft isn't close then recall    *
*				the interceptor.                                         *
*                                                                        *
*	If the group we have detected is not being intercepted (and should   *
*   be) then we must figure out which planes to have intercept.  Remember*
*	to keep the ---- PLAYER!!!!!! ----- in mind when you are choosing    *
*	which aircraft to vector to the group.  If no airborn aircraft are   *
*   relatively close by then also check the airbase and carrier lists.   *
*	Remember to see if the aircraft chosen to intercept CAN intercept.   *
*   Such things to look for are:                                         *
*			1.	Is the plane an enemy                                    *
*			2.	Does the plane have any Air to Air missiles              *
*			3.	Is the plane damaged                                     *
*			4.	Is the plane low on fuel (if we are using fuel)          *
*			5.	Is the plane returning home after just engaging this     *
*				plane.                                                   *
*			6.  Is the planes mission such that it cannot be used to     *
*				intercept aircraft(Such as bombing targets).             *
*			7.	If the plane is doing a mission such as CAP have other   *
*				aircraft take his place (these new planes will probably  *
*				be supplied from carriers and bases) if available.       *
*			8.  If this plane was already intercepting an aircraft make  *
*				sure he is going after the aircraft closest to him.  If  *
*				he switches which group he is going after remember to    *
*				assign a new interceptor after the group he was going    *
*				after.                                                   *
*                                                                        *
*	Another thing that must be done (somewhere else) is that if a group  *
*	hasn't been detected for a while then have the planes loose track of *
*	where he is.                                                         *
*	                                                                     *
*	In the same line, I must remember to put a thing in GetEnemyTarget   *
*	so that if the enemy hasn't detected you yet (and you don't have     *
*   him locked on with radar) then he won't pick you.                    *
*	                                                                     *
*                                                                        *
*	                                         SRE   7/8/93                *
**************************************************************************/

void InterceptControl(int planenum)
{
	int intercept;
	int prewing;
	struct PlaneData far *ptemp;

	if(Gtime < 7)
		return;

	if((planenum < -1) || (planenum > NPLANES))
		return;

//	TXT("Intercept Control ");
//	TXN(planenum);
//	Message(Txt);

	prewing = GetSquadLeader(planenum);

	if(prewing == -1) {
		intercept = targetplayer;
		if(Gtime < timetargetplayer) {
			return;
		}
	}
	else {
		intercept = ps[prewing].color;
		if(Gtime < ps[prewing].time) {
			return;
		}
		if(ps[prewing].status & CRASH)
			return;
		if(bv[prewing].Behavior == AIGoHome)
			return;
		if(ps[prewing].status & CIVILIAN) {
			ps[prewing].time = Gtime + 120;
			return;
		}
	}

/*	if(intercept == -1) {
		return;
	}  */

/*	TXT("Made it Here pw ");
	TXN(prewing);
	TXA(" int ");
	TXN(intercept);  */

	if((intercept >= 0) && (!(bv[intercept].behaveflag & ENEMYNOFIRE))) {
		if (!((ps[intercept].status & CRASH) || (!(ps[intercept].status & ALIVE)) || (!(ps[intercept].status&ACTIVE)))) {
/*			TXA(" itg ");
			TXN(GetSquadLeader(bv[intercept].target));  */

			if(GetSquadLeader(bv[intercept].target) == prewing)
				return;
		}
	}

/*	if(prewing == -1) {
		TXA(" a ");
		TXN(planenum);
		Message(Txt,WINGMAN_MSG);
	}  */

/*	TXT("INTERCEPT ");
	TXN(prewing);
	TXA(" ");
	TXN(tempgotcha);
	TXA(" ");
	TXN(planecnt);
	Message(Txt);  */

	GetInterceptPlane(planenum, prewing);

	if(prewing != -1) {
		if(ps[prewing].color == -999)
			ps[prewing].time = Gtime + 15;

/*		if(ps[prewing].color != -999) {
			TXT(" ");
			TXN(prewing);
			TXA(" int by ");
			TXN(ps[prewing].color);
			Message(Txt,DEBUG_MSG);
		}  */
	}
	else {
		if(targetplayer == -999)
			timetargetplayer = Gtime + 15;

/*		if(targetplayer != -999) {
			TXT("P ");
			TXN(prewing);
			TXA(" int by ");
			TXN(targetplayer);
			Message(Txt,DEBUG_MSG);
		}  */
	}

}

int CheckIfCanIntercept(int planenum, int tdist)
{
	int prewingt;
	COORD tdist2;

	if((bv[planenum].Behavior == FlyCruise) || (bv[planenum].Behavior == EngageTrain)) {
		return(0);
	}

	if((ps[planenum].status & TAKEOFF) && (ps[planenum].base < 0) && (bv[planenum].threathead < 0)) {
		return(0);
	}

	if((bv[planenum].Behavior == Ready5) && (bv[planenum].threathead < 0)) {
		return(0);
	}

	if(CheckAIMissiles(planenum) == -1) {  /*  No Missiles No Intercept  */
		return(0);
	}

	if(CheckAIBombs(planenum) != -1) {  /*  Bombs, Intercept  */
		return(0);
	}
	if(bv[planenum].behaveflag & (DAMAGERADAR|DAMAGEENGINE|DAMAGECONTROLSURFACE)) {
		return(0);
	}

	if(bv[planenum].Behavior == AIGoHome) {
		if(tdist < 1350)
			return(0);  /*  Could Be trying to disengage from planenum  */
	}

	prewingt = GetSquadLeader(bv[planenum].target);
	if(prewingt == -1) {
		if(targetplayer == planenum) {
			tdist2 = xydist(Px - ps[planenum].x, Py - ps[planenum].y);
			if(tdist2 > (tdist + 45))
				return(0);
		}
	}
	else {
		if(ps[prewingt].color == planenum) {
			tdist2 = xydist(ps[prewingt].x-ps[planenum].x, ps[prewingt].y-ps[planenum].y);
			if(tdist2 > (tdist + 45))
				return(0);
		}
	}

	return(1);
}

int GetCloseAirBase(int ainum, int friend, COORD *distval)
{
    int     b, r, j;
    COORD   d, tdist;

    tdist = 0x7fff;
    for (j=0; j<NRADAR; j++) {
        if (((Rdrs[j].status&(AIRBASE+NEUTRAL+DESTROYED))==AIRBASE)
				&& ((Rdrs[j].status & (FRIENDLY|NEUTRAL)) != (friend))){
            d =xydist (ps[ainum].x-Rdrs[j].x, ps[ainum].y-Rdrs[j].y);

            if (d<tdist) {
                tdist = d;
                b  = j;
            }
        }
    }

	*distval = tdist;
	if(tdist != 0x7fff) {
		return(b);
	}
	return(-999);
}

void UpdateInterceptGroup(int ainum, int dobehave)
{
	int prewing;
	int wingcnt;
	int radaron = 0;
	int enemynofire = 0;

	prewing = GetSquadLeader(ainum);

	if(prewing == -1)
		return;

	if(bv[ainum].behaveflag & RADARON)
		radaron = 1;
	if(bv[ainum].behaveflag & ENEMYNOFIRE)
		enemynofire = 1;
	wingcnt = prewing;
	while (prewing >= 0) {
		while (wingcnt >= 0) {

			if((CheckAIMissiles(wingcnt) != -1) && (CheckAIBombs(wingcnt) == -1)
					&& (!(bv[wingcnt].behaveflag & (DAMAGERADAR|DAMAGEENGINE|DAMAGECONTROLSURFACE)))) {
				if(dobehave) {
					if(bv[wingcnt].Behavior == Ready5) {
						ps[wingcnt].status |= TAKEOFF;
					}

					bv[wingcnt].Behavior = bv[ainum].Behavior;
					bv[wingcnt].horizontalsep = bv[ainum].horizontalsep;
					if(bv[ainum].behaveflag & COMBAT_SPREAD) {
						bv[wingcnt].behaveflag |= COMBAT_SPREAD;
					}
					else {
						bv[wingcnt].behaveflag &= ~COMBAT_SPREAD;
					}
				}
				bv[wingcnt].target = bv[ainum].target;
				bv[wingcnt].xloc = bv[ainum].xloc;
				bv[wingcnt].yloc = bv[ainum].yloc;
				ps[wingcnt].status &= ~BACKTOBASE;
				if(!(ps[wingcnt].status & TAKEOFF)) {
					bv[wingcnt].varcnt = 0;
					bv[wingcnt].groundt = bv[ainum].groundt;
				}

				if(radaron)
					bv[wingcnt].behaveflag |= RADARON;
				else
					bv[wingcnt].behaveflag &= ~RADARON;
				if(enemynofire)
					bv[wingcnt].behaveflag |= ENEMYNOFIRE;
				else
					bv[wingcnt].behaveflag &= ~ENEMYNOFIRE;
			}

			wingcnt = bv[wingcnt].wingman;
		}
		prewing = bv[prewing].nextsec;
		wingcnt = prewing;
	}
}

void VectorIntercept(int planenum, int targetnum)
{
	int replacecap;
	int whichbehave;
	int bogeyangle;
	COORD bogeydist;
	int dx, dy;
	int prewing;
	struct PlaneData far *ptemp;

/*	TXT("Vector ");
	TXA(" pn ");
	TXN(planenum);
	TXA(" after ");
	TXN(targetnum);
	TXA(" prev ");
//	TXN(bv[planenum].target);
	Message(Txt,DEBUG_MSG);  */


#if 0
	if(targetnum == -1) {
		TXT("Plane ");
		TXN(planenum);
		TXA(" AFTER PLAYER");
		Message(Txt,DEBUG_MSG);
	}
#endif

	if(planenum == -1) {
		prewing = GetSquadLeader(targetnum);

		if(ps[prewing].color == -1)
			return;

		ps[prewing].color = -1;
		dx = ps[targetnum].x - Px;
		dy = ps[targetnum].y - Py;
	    bogeyangle = angle(dx,-dy);
    	bogeydist = Dist2D(dx,dy);
		LocationPicture (AWACSSRC, ENEMYSRC, bogeyangle, bogeydist, targetnum, 0);
#if 0
		if(ps[targetnum].status & FRIENDLY)
			TXT("FRIENDLY ");
		else if(ps[targetnum].status & NEUTRAL)
			TXT("NEUTRAL ");
		else
			TXT("ENEMY ");
		TXN(targetnum);
		TXA(" ON ");
		TXN((int)bv[targetnum].orgnum);
		TXA(" STAT ");
		TXN(ps[targetnum].status);
		Message(Txt, DEBUG_MSG);
		sprintf(Txt, "P2 ORG D %d, D %ld LD %ld", bogeydist, LDist2D((long)dx, (long)dy), LDist2D((long)(ps[targetnum].xL - PxLng), (long)(ps[targetnum].yL - (0x100000-PyLng))));
		Message(Txt, DEBUG_MSG);
#endif
		return;
	}

	if(bv[planenum].target == targetnum)
	{
		if(targetnum != -1) {
			bv[planenum].behaveflag |= RADARON;
			bv[planenum].behaveflag &= ~ENEMYNOFIRE;
			return;
		}
	}

	if(IsCAPFlier(planenum)) {
		DoCAPReplace(planenum);

		bv[planenum].target = targetnum;
		if(bv[planenum].leader > -1) {
			if(!(FriendMatchType(planenum, FRIENDLY))) {
				bv[planenum].Behavior = Formation;
			}
			else
				bv[planenum].Behavior = FlyStraight;
			bv[planenum].behaveflag |= COMBAT_SPREAD;
			bv[bv[planenum].leader].target = bv[planenum].target;
			bv[bv[planenum].leader].Behavior = FlyStraight;
			bv[bv[planenum].leader].behaveflag |= RADARON;
			bv[bv[planenum].leader].behaveflag &= ~ENEMYNOFIRE;
		}
		else {
			bv[planenum].Behavior = FlyStraight;
			if(bv[planenum].wingman > -1) {
				bv[bv[planenum].wingman].Behavior = Formation;
				bv[bv[planenum].wingman].behaveflag |= COMBAT_SPREAD;
				bv[bv[planenum].wingman].target = bv[planenum].target;
				bv[bv[planenum].wingman].behaveflag |= RADARON;
				bv[bv[planenum].wingman].behaveflag &= ~ENEMYNOFIRE;
			}
		}
		bv[planenum].behaveflag |= RADARON;
		bv[planenum].behaveflag &= ~ENEMYNOFIRE;
	}
/*	if(!(bv[planenum].behaveflag & ENEMYNOFIRE)) {
		if((bv[planenum].target == -1) && (timetargetplayer) && (!(FriendMatchType(planenum, FRIENDLY)))) {
			GetInterceptPlane(bv[planenum].target, GetSquadLeader(bv[planenum].target));
		}
		else if((bv[planenum].target != -1) && (ps[bv[planenum].target].time)) {
			GetInterceptPlane(bv[planenum].target, GetSquadLeader(bv[planenum].target));
		}
	}  */
	bv[planenum].target = targetnum;
	ps[planenum].status &= ~BACKTOBASE;
	if(!(ps[planenum].status & TAKEOFF))
		bv[planenum].varcnt = 0;

	if(GetSquadLeader(targetnum) > -1)
		ps[GetSquadLeader(targetnum)].color = planenum;
	else
		targetplayer = planenum;

	bv[planenum].basespeed += 200;

	ptemp = GetPlanePtr(ps[planenum].type);

	if(bv[planenum].basespeed > ptemp->speed) {
		if(ptemp->speed > 450)
			bv[planenum].basespeed = ptemp->speed - 100;
		else
			bv[planenum].basespeed = ptemp->speed;
	}

    bogeydist = Dist2D((ps[targetnum].x - ps[planenum].x), (ps[targetnum].y - ps[planenum].y));
	bogeydist = (bogeydist * 2)/90;  /*  to get nm  */

	if(bv[planenum].Behavior == Ready5)
		ps[planenum].status |= TAKEOFF;

	if(((bogeydist > 30) || (ps[planenum].status & ENEMYNOFIRE)) && (!((bv[planenum].Behavior == Ladder) || (bv[planenum].Behavior == Wall) ||
			(bv[planenum].Behavior == Box) || (bv[planenum].Behavior == Formation) ||
			(bv[planenum].Behavior == Ladder2) || (bv[planenum].Behavior == FlyStraight)))) {

		if(FriendMatchType(planenum, FRIENDLY)) {
			bv[planenum].Behavior = Formation;
			bv[planenum].horizontalsep = COMBATSPREADDIST;
		}
		else {
			whichbehave = (Gtime) & 3;
			if(whichbehave == 0) {
				bv[planenum].Behavior = Box;
			}
			else if(whichbehave == 1) {
				bv[planenum].Behavior = Ladder;
			}
			else if(whichbehave == 2) {
				bv[planenum].Behavior = Wall;
			}
			else {
				bv[planenum].Behavior = Formation;
				bv[planenum].horizontalsep = PARADEDIST;
			}
		}
		bv[planenum].behaveflag |= RADARON;
		bv[planenum].behaveflag &= ~ENEMYNOFIRE;
		UpdateInterceptGroup(planenum, 1);
	}
	else if((!((bv[planenum].Behavior == Ladder) || (bv[planenum].Behavior == Wall) ||
			(bv[planenum].Behavior == Box) || (bv[planenum].Behavior == Formation) ||
			(bv[planenum].Behavior == Ladder2) || (bv[planenum].Behavior == FlyStraight)))) {

		bv[planenum].Behavior = FlyStraight;
		bv[planenum].behaveflag |= RADARON;
		bv[planenum].behaveflag &= ~ENEMYNOFIRE;
		UpdateInterceptGroup(planenum, 1);  /*  used to be (planenum, 0)  */
	}
	else {
		bv[planenum].behaveflag |= RADARON;
		bv[planenum].behaveflag &= ~ENEMYNOFIRE;
		UpdateInterceptGroup(planenum, 0);
	}
}

#define INTERCEPTDIST 3600
void GetInterceptPlane(int planenum, int prewing)
{
	int cnt, founddist, wingrear;
	int dx, dy, dx1, dy1, angle;
	int x, y, interceptnum;
	COORD dist, dist1, distnm;
	int wingnum, othertarget;
	int otherdist;
	int closebasenum;
	int basedist;
	int isfriend;
	int intps;
	int ready5num;

	if(planenum > -1) {
		x = ps[planenum].x;
		y = ps[planenum].y;
		isfriend = FriendStatus(planenum);

	}
	else {
		x = Px;
		y = Py;
		isfriend = FRIENDLY;
	}

	founddist = 0;
	if(!(isfriend==FRIENDLY)) {
		founddist = 1;
    	dx = Px-x;
    	dy = Py-y;
    	dist = xydist(dx,dy);
		interceptnum = -1;
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
//				&& (ps[cnt].speed) && (cnt != othertarget)) { /* Planes in the air */
			if((!FriendMatchType(cnt, isfriend)) && (GetSquadLeader(cnt) == cnt)) {
       			dist1 = xydist(dx1=ps[cnt].x-x, dy1=ps[cnt].y-y);
				if(CheckIfCanIntercept(cnt, dist1)) {
       				if ((dist1<dist) || (!founddist)) {
						founddist = 1;
           				dist=dist1;
           				dx = dx1;
           				dy = dy1;
						interceptnum = cnt;
       				}
					if((dist1 < 1800) && (bv[cnt].behaveflag & ENEMYNOFIRE)) {
						VectorIntercept(cnt, planenum);
					}
				}
			}
		}
	}

	closebasenum = GetCloseAirBase(planenum, isfriend, &basedist);
	if(basedist >= INTERCEPTDIST)
		closebasenum = -999;

	if((founddist) || (closebasenum != -999)) {
		if(!founddist) {
			ready5num = CheckForMobileR5(closebasenum, isfriend);
			if(ready5num != -999) {
				VectorIntercept(ready5num, planenum);
			}
			return;
		}
		else if(closebasenum == -999) {
			if(dist < INTERCEPTDIST) {
				VectorIntercept(interceptnum, planenum);
			}
			return;
		}
		else {
			if((basedist < (dist + 1800)) && (basedist < INTERCEPTDIST)) {
				ready5num = CheckForMobileR5(closebasenum, isfriend);
				if(ready5num != -999) {
					VectorIntercept(ready5num, planenum);
					return;
				}
			}
			if(dist < INTERCEPTDIST) {
				VectorIntercept(interceptnum, planenum);
			}
		}
	}
}

void LocationPicture (int whotoldorg, int whatis, int whatangle, COORD whatdist, int planedetect, int nochange)
{
    struct  Planes  *psi, *Mini;
    int i, dx, dy, dist, angl, MinDist, MinAngl, eyex, eyey, eyedist, bogeyi;
	int cnt;
	struct PlaneData far *ptemp;
	int carrx, carry, carrangle, carrrelangle;
	UWORD carrdist = 18000;
	int do_id = 0;
	int is_target = 0;
	int waynum, bogey_org_num;
	int whotold;

	whotold = whotoldorg;

	if(whotold == AWACSSRC) {
		for(cnt = 0; cnt < NPLANES; cnt ++) {
			if((ps[cnt].status & ALIVE) && (ps[cnt].status & ACTIVE) && (ps[cnt].status & FRIENDLY)) {
				ptemp = GetPlanePtr(ps[cnt].type);
				if(ptemp->bclass == RADAR_PLANE_OBJCLASS)
					break;
			}
		}

		if(cnt == NPLANES) {
			whotold = CARRIERSRC;
		}
		else {
    		TXT ("(HAWKEYE) ");
			eyex = ps[cnt].x;
			eyey = ps[cnt].y;
		}
	}
	if((whotold == CARRIERSRC) || (whotold == TOWERSRC)) {
		if(whichourcarr == -999) {
			whotold = WINGSRC;
		}
		else if((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status & ACTIVE)) {
			if(whotold == TOWERSRC)
				TXT("(TOWER) ");
			else
				TXT("(CARRIER) ");
			eyex = ps[whichourcarr].x;
			eyey = ps[whichourcarr].y;
		}
		else
			whotold = WINGSRC;
	}
	if(whotold == WINGSRC) {
		if(AIWingMan == -999) {
			return;
		}
		if((ps[AIWingMan].status & ALIVE) && (ps[AIWingMan].status & ACTIVE)) {
			if(whotoldorg != WINGSRC) {
				if(whatdist > 1800) {
					return;
				}
			}
			TXT(OurWingName);
			eyex = ps[AIWingMan].x;
			eyey = ps[AIWingMan].y;
		}
		else {
			return;
		}
	}


	TXA(OurCallSign);
	TXA(" ");

    if (nochange) {
        TXA ("Status unchanged");
    } else {
		bogeyi = planedetect;
		MinDist = whatdist;
		MinAngl = whatangle;

		eyedist = Dist2D ((ps[bogeyi].x - eyex), (ps[bogeyi].y - eyey));
		if((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status & ACTIVE)) {
			carrx = ps[whichourcarr].x;
			carry = ps[whichourcarr].y;
			carrdist = Dist2D ((carrx - ps[bogeyi].x), (carry - ps[bogeyi].y));
			carrangle = angle((carrx - ps[bogeyi].x), -(carry - ps[bogeyi].y));
			carrrelangle = carrangle - ps[bogeyi].head;
		}

        if ((MinDist==11250) && (eyedist >= 11250)) {
            TXA ("Clean ");
            Speech (A_CLEAN);

        } else {
			TXA("Contacts ");
            Speech (A_CONTACT_BRG);

            BearingToText (MinAngl,1);

            TXA (", ");
//	            Speech (V_EMPTY);

            MinDist = MinDist*2/95; // convert 15-bit COORD to nm

            if (MinDist>=160) {
                TXA ("160");
                Speech (A_160);
            } else if (MinDist>=120) {
                TXA ("120");
                Speech (A_120);
            } else if (MinDist>=80) {
                TXA ("80");
                Speech (A_80);
            } else if (MinDist>=40) {
                TXA ("40");
                Speech (A_40);
            } else {
                NSpeech (MinDist, 1);
            }
			if(warstatus == 1) {
				TXA(" Red and Free ");
            	Speech (A_RED_FREE);
			}
			else if(warstatus == 0) {
				/*  determin if no factor, otherwise  */
				/*  see if friendly's nearby for VID else  */
				if((carrdist < 11250) || ((abs(carrrelangle) < 1554) && (carrdist < 18000))) {
        			for (i=0; i<NPLANES; i++) {
            			if ((ps[i].status & ACTIVE) && (ps[i].status & ALIVE) && (ps[i].status&FRIENDLY) && !(ps[i].status&CRASH)) {
                			dx = ps[i].x - ps[bogeyi].x;
                			dy = ps[i].y - ps[bogeyi].y;
							if((abs(dx) < 1800) && (abs(dy) < 1800)) {
								if(GetSquadLeader(i) != -1) {
									do_id = 1;
									i = NPLANES;
								}
							}
            			}
        			}

					is_target = CheckIsWayTarget(bogeyi);

					if(is_target) {
						TXA(" Red and Free ");
        		    	Speech (A_RED_FREE);
					}
					else if(do_id) {
						TXA(" V.I.D ");
            			Speech (A_VID);
					}
					else {
						TXA(" Investigate ");
            			Speech (A_INVESTIGATE);
					}
				}
				else {

					is_target = CheckIsWayTarget(bogeyi);

					if(is_target) {
						TXA(" Red and Free ");
        		    	Speech (A_RED_FREE);
					}
					else {
						TXA(" No Factor");
    	        		Speech (A_NOFACTOR);
					}
				}
			}
        }
    }

    Message(Txt,GAME_MSG);
	return;
}

#if 0
CheckAirSpace(int x, int y, int radardistkm, int friendstat)
{
	int cnt, founddist, wingrear;
	int dx, dy, dx1, dy1, angle;
	int x, y, targetnum;
	COORD dist, dist1, distnm;
	int distflag = 0;

	founddist = 0;
	if(!(friendstat)) {
		founddist = 1;
    	dx = Px-x;
    	dy = Py-y;
    	dist = xydist(dx,dy);
    	CloseGuy=0;
		targetnum = -1;
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
				(ps[cnt].speed) && (cnt != othertarget)) { /* Planes in the air */
			if((psi->status & FRIENDLY) != (ps[cnt].status & FRIENDLY)) {
       			dist1 = xydist(dx1=ps[cnt].x-x, dy1=ps[cnt].y-y);
       			if ((dist1<dist) || (!founddist)) {
					founddist = 1;
           			dist=dist1;
           			dx = dx1;
           			dy = dy1;
            		CloseGuy=1;
					targetnum = cnt;
       			}
			}
		}
	}
}
#endif

void CheckE2C(int planenum)
{
	int targnum;
	COORD targdist;
	struct PlaneData far *ptmptype;
    COORD       dist;
    int         signal;
    int         angl;


//	targnum = Rtime & 0x7F;
	targnum = Rtime & 0xFF;
	if(targnum == MAXPLANE)
		targnum = -1;

	if(targnum > NPLANES)
		return;

	ptmptype = GetPlanePtr(ps[planenum].type);

    signal = RsignalOne (ps[planenum].x,ps[planenum].y,ps[planenum].alt, ptmptype->rclass, &angl,&dist,FriendStatus(planenum) , targnum);

	if(signal > dist) {
		tempgotcha = 2;
		gotcha (targnum);
		tempgotcha = -999;
	}
}


void TrooperDrop(int planenum)
{
	int troopernum;
	struct PlaneData far *ptmptype;
	int prewing;

	ptmptype = GetPlanePtr(ps[planenum].type);

	bv[planenum].basespeed = 150;

	troopernum = ps[planenum].mcnt[0];
	if((troopernum <= 0) || (troopernum > (NUMPTROOPS - 1))) {
		troopernum = ps[planenum].mcnt[0] = 0;
		return;
	}

	prewing = GetSquadLeader(planenum);

//	if((((Rtime & 3) == (planenum & 3)) && (!(ps[planenum].status & JUSTPUT))) || (((Rtime & 3) == (prewing & 3)) && (ps[planenum].status & JUSTPUT))) {
	if((Rtime & 3) == (prewing & 3)) {
		if(troopernum == ptmptype->mcnt[0])
			LogIt (SLG_AIR_TROOP_DROP, planenum, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, -999, ptmptype->misl[0], 0L, LOG_AIR);

		troopernum = ((planenum & 3) * 5) + troopernum;  /*   So that when a wing drops troops they won't cancel grab each others troops  */
		while(troopernum >= (NUMPTROOPS))
			troopernum -= NUMPTROOPS;

        paratroopers[troopernum].chuteX = ps[planenum].x;
        paratroopers[troopernum].chuteY = ps[planenum].y;
        paratroopers[troopernum].chuteZ = ps[planenum].alt;
        paratroopers[troopernum].chuteDZ = 0;

		ps[planenum].mcnt[0] = ps[planenum].mcnt[0] - 1;

#if 0
		else {
			bv[planenum].Behavior = FlyWayPoint;
			bv[planenum].basespeed = 250;
		}
#endif
	}

//	FlyWayPoint();
}

void LaunchCruise(int planenum)
{
	int planeadded;
	int wingcnt;
	int tprewing;
	int cnt, cnt2;
	int aistation;
	struct PlaneData far *tptype;
	int planeid;
	struct MissileData far *tempmissile;
	int sideid;
	int log_launch, log_src;

	if(GetSquadLeader(planenum) != planenum)
		return;

	if(planenum > NPLANES) {
		log_launch = SLG_BOAT_CRUISE_LAUNCH;
		log_src = LOG_BOAT;
	}
	else {
		log_launch = SLG_AIR_CRUISE_LAUNCH;
		log_src = LOG_AIR;
	}

	aistation = CheckAIBombs(planenum);
	if((aistation != -1) && (bv[planenum].Behavior != FlyCruise)) {
		tptype = GetPlanePtr(ps[planenum].type);
		tempmissile = GetMissilePtr(tptype->misl[aistation]);
		if((ps[planenum].mcnt[aistation] > 0) && (tempmissile->type < 0)) {
			if(planenum >= 0) {
				TXT(" ");
/*				TXN(planenum);
				TXA(" Cruise Launch ");
				TXN(bv[planenum].groundt);
				Message(Txt,DEBUG_MSG);  */
			}
		}
		else {
			return;
		}
	}
	else {
		return;
	}

	ps[planenum].mcnt[aistation]--;
	bv[planenum].behavecnt = Gtime + 3;

	planeid = PlaneSeek(-tempmissile->type);
	sideid = SS_ENEMY;
	if(ps[planenum].status & FRIENDLY)
		sideid = SS_FRIENDLY;
	else if(ps[planenum].status & NEUTRAL)
		sideid = SS_NEUTRAL;

	planeadded = AddSomePlanes2 (-1, planeid, SA_CRUISE_MISS, 0, 1, 0, sideid, ST_PLANE, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt - 5, ps[planenum].head, -999, bv[planenum].basespeed, 0);
	bv[planeadded].prevsec = planenum;
	LogIt (log_launch, planenum, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, -999, planeid, 0L, log_src);

	bv[planeadded].groundt = bv[planenum].groundt;

	if(warstatus == 0) {
		if((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status & ACTIVE)) {
			if((aiwaypoints[bv[planeadded].groundt].x <= (long)-1) && (aiwaypoints[bv[planeadded].groundt].y == (long)bv[whichourcarr].orgnum)) {
				warstatus = 1;
				LastCall = -4*60;
			}
		}
	}

	if(bv[planenum].Behavior == FlyWayPoint) {
		bv[planenum].groundt = bv[planenum].groundt + 1;
		return;
	}

	tprewing = planenum;
	wingcnt = bv[planenum].wingman;
	while (tprewing >= 0) {
		while (wingcnt >= 0) {
			if(bv[wingcnt].Behavior == bv[planenum].Behavior) {
				tptype = GetPlanePtr(ps[planenum].type);
				tempmissile = GetMissilePtr(tptype->misl[aistation]);
				if((ps[planenum].mcnt[aistation] > 0) && (tempmissile->type < 0)) {
					planeid = PlaneSeek(-tempmissile->type);
					planeadded = AddSomePlanes2 (-1, planeid, SA_CRUISE_MISS, 0, 1, 0, sideid, ST_PLANE, ps[wingcnt].xL, ps[wingcnt].yL, ps[wingcnt].alt - 5, ps[wingcnt].head, -999, bv[wingcnt].basespeed, 0);
					bv[planeadded].prevsec = wingcnt;
					LogIt (log_launch, wingcnt, ps[wingcnt].xL, ps[wingcnt].yL, ps[wingcnt].alt, -999, planeid, 0L, log_src);
					bv[planeadded].groundt = bv[planenum].groundt;
					ps[wingcnt].mcnt[aistation]--;
					bv[wingcnt].behavecnt = Gtime + 3;
					bv[wingcnt].groundt = bv[planenum].groundt + 1;
				}
			}
			wingcnt = bv[wingcnt].wingman;
		}
		tprewing = bv[tprewing].nextsec;
		wingcnt = tprewing;
	}

	bv[planenum].groundt = bv[planenum].groundt + 1;
}

void FlyCruise()
{
	long dx, dy;
	UWORD dz;
	int tempalt;
	int tdx, tdy;
	COORD tdist, tdistnm;
	int wayval;

	i = planecnt;

	wayval = bv[planecnt].groundt;
	if(wayval <= -1) {
		ps[planecnt].status = 0;
		ps[planecnt].speed = 0;
		return;
	}

	GetWayXYZ(wayval, &dx, &dy, &dz);
	tdx = dx>>HTOV;
	tdy = dy>>HTOV;

	tdist = Dist2D((tdx - ps[planecnt].x), (tdy - ps[planecnt].y));

	if(tdist < 5) {
		CruiseDistruct(planecnt);
		ps[planecnt].status = 0;
		ps[planecnt].speed = 0;
		return;
	}
	tdistnm = (tdist * 2) / 90;  /*  convert to nm  */
	if(tdistnm > 60) {
		tempalt = ps[planecnt].alt;
		if(bv[planecnt].basespeed < ptype->speed)
//		if(bv[planecnt].basespeed < 800)
			bv[planecnt].basespeed += 5;
	}
	else {
		tempalt = 50;
		if(bv[planecnt].basespeed > 700)
			bv[planecnt].basespeed -= 10;
	}

	LFlyTo((dx * AILOCDIV), (dy * AILOCDIV), tempalt, -999, 0);

/*	if(tempvar) {
		TXT("CM d ");
		TXN(tdist);
		TXA(" spd ");
		TXN(bv[planecnt].basespeed);
		TXA(" psd ");
		TXN(ps[planecnt].speed);
		TXA(" alt ");
		TXN(tempalt);
		TXA(" dr ");
		TXN(droll);
		Message(Txt,DEBUG_MSG);
	}  */
}

void GetLastPosition (int planenum, long *aixloc, long *aiyloc, UWORD *palt)
{
    int a;
	long spd, Gspd;
	long dx, dy;
	long spd2;
	int speed2;
	long tspd;
	long precval;
	long precval2;
	long txL, tyL;
	long taixloc, taiyloc;
	UWORD talt;

	if(planenum == -1) {
		*aixloc = oldFPX;
		*aiyloc = oldFPZ;
		*palt = oldAltval;
		return;
	}

	talt = ps[planenum].alt;
	txL = ps[planenum].xL;
	tyL = ps[planenum].yL;
	taixloc = (ps[planenum].xL * AILOCDIV) + (AILOCDIV>>1);
	taiyloc = (ps[planenum].yL * AILOCDIV) + (AILOCDIV>>1);

	if(Speedy == 1) {
	    tspd = (((long)ps[planenum].speed*27 >>6) * Fticks * AILOCDIV) /60;
		spd = tspd;
    	speed2 = ((ps[planenum].speed*27 >>6) * Fticks) /60;
	    Gspd = cosXL (ps[planenum].pitch,spd);
    	taixloc -= sinXL(ps[planenum].head,Gspd);
    	taiyloc += cosXL(ps[planenum].head,Gspd);
		txL = taixloc / AILOCDIV;
		tyL = taiyloc / AILOCDIV;
	    talt -= sinX(ps[planenum].pitch,(spd/AILOCDIV));
	}
	else {
    	spd = (((long)ps[planenum].speed*27 >>6) * Fticks) /60;
	    Gspd = cosXL(ps[planenum].pitch,spd);
	    txL -= sinXL(ps[planenum].head,Gspd);
    	tyL += cosXL(ps[planenum].head,Gspd);
	    talt += (int)sinXL(ps[planenum].pitch, spd);
	}


	if(GetSquadLeader(i) != -1) {
		taixloc = ((long)txL * AILOCDIV) + (AILOCDIV>>1);
		taiyloc = ((long)tyL * AILOCDIV) + (AILOCDIV>>1);
	}
	else {
		precval = oldFPX / AILOCDIV;
		precval = oldFPX - (precval * AILOCDIV);
		taixloc = ((long)txL * AILOCDIV) + precval;
		precval2 = oldFPZ / AILOCDIV;
		precval2 = oldFPZ - (precval2 * AILOCDIV);
		taiyloc = ((long)tyL * AILOCDIV) + precval2;
	}
	*aixloc = taixloc;
	*aiyloc = taiyloc;
	*palt = talt;
}

void FlyWPFollowPlayer()
{
	COORD tdistnm;
	int prewing;
	struct PlaneData far *tempplane;

	i = planecnt;

	prewing = GetSquadLeader(planecnt);
	if(prewing != planecnt) {
		Formation();
		return;
	}

	FlyWayPoint();
	tdistnm = Dist2D((ps[planecnt].x - Px), (ps[planecnt].y - Py));
	tdistnm = (tdistnm * 2)/90;
	if((tdistnm < 3) && (abs(OurHead - ps[planecnt].head) < 0x2000)) {
		tempplane = GetPlanePtr(ps[planecnt].type);
		FTXT (tempplane->name);
		TXA (" Forming On Wing");
		Message(Txt,GAME_MSG);
		bv[planecnt].Behavior = FollowPlayer;
		bv[planecnt].varcnt = Gtime + 300;
		bv[planecnt].horizontalsep = BOMBERPARADEDIST;
	}
#if 0
	if(tempvar) {
		TXT("FlyWP ");
		TXN(tdistnm);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void FollowPlayer()
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
	struct PlaneData far *tempplane;

	i = planecnt;

	prewing = -1;

	pdest = 0;

	secplace = (bv[i].behaveflag & 15);

	prespeed = Knots + GetDAltSpeed(ps[planecnt].alt);

	temphead = OurHead - 0x5000;

	tempwing = -1;


	GetLastPosition (-1, &dx, &dy, &ourleadalt);
	dx = dx + sinXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));
	dy = dy - cosXL(temphead, ((long)bv[i].horizontalsep * AILOCDIV));

	tempalt = ourleadalt + 10;

	if(tempalt < 75)
		tempalt = 75;

	LFlyTo(dx, dy, tempalt, prewing, prespeed);

	if(bv[planecnt].varcnt) {
		if((bv[planecnt].varcnt - Gtime) < 2) {
			tempplane = GetPlanePtr(ps[planecnt].type);
			FTXT (tempplane->name);
			TXA (" Heading Home");
			Message(Txt,GAME_MSG);
			bv[planecnt].varcnt = Gtime - 1;
		}
	}

#if 0
	if(tempvar) {
		TXT("Follow ");
		TXN((bv[planecnt].varcnt - Gtime));
		Message(Txt,DEBUG_MSG);
	}
#endif

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

int GetWhoCover(int planenum)
{
	int cnt;
	int isfriend;
	int returnval = -999;
	COORD tdist, cdist;

	isfriend = FriendStatus(planenum);

	cdist = 1800;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if(FriendMatch(cnt, planenum) && (cnt != planenum) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)){
			if(cnt == GetSquadLeader(cnt)) {
				if(bv[cnt].Behavior == GorillaBomb) {
					if((bv[cnt].target == -1) || (FriendMatch(bv[cnt].target, planenum))) {
						tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
						if(tdist < cdist) {
							tdist = cdist;
							returnval = cnt;
						}
					}
				}
			}
		}
	}
	if(returnval != -999)
		return(returnval);

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if(FriendMatch(cnt, planenum) && (cnt != planenum) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)){
			if(cnt == GetSquadLeader(cnt)) {
				if((bv[cnt].target == -1) || (!FriendMatch(bv[cnt].target, planenum))) {
					tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
					if(tdist < cdist) {
						tdist = cdist;
						returnval = cnt;
					}
				}
			}
		}
	}
	return(returnval);
}

void GorillaCover()
{
	int tnextsec;
	int prewing;
	long dx, dy;
	int dz;
	int relangle;
	int tangle, tpitch, troll, enemyalt, temphead, secplace;
	int tempwing, prespeed, tempspeed;
	COORD tdist, distkm, tdistnm;
	int offangle;
	int hsep;
	int ourleadalt, tempalt;
	int dpwhead;
	int covernum;

	i = planecnt;

	prewing = GetSquadLeader(planecnt);

	if(bv[planecnt].target == -1) {
		bv[planecnt].target = GetWhoCover(planecnt);
		if(bv[planecnt].target == -999) {
			bv[planecnt].Behavior = AIGoHome;
    		ps[planecnt].status |= BACKTOBASE;
			bv[planecnt].target = -1;
			ChangeGroupBehavior(planecnt, 0);
		}
		if(prewing == planecnt)
			bv[bv[planecnt].target].target = planecnt;
	}
	covernum = bv[planecnt].target;

	if(prewing == planecnt) {
		tnextsec = bv[planecnt].nextsec;
		if(tnextsec != -999) {
			bv[tnextsec].Behavior = RearCover;
			bv[tnextsec].target = covernum;
			bv[tnextsec].prevsec = -999;
			bv[planecnt].nextsec = -999;
			ChangeGroupBehavior(tnextsec, 0);
		}
	}

	offangle = 0x4000;

	prespeed = 0;
	prespeed = bv[covernum].basespeed;

	if(bv[i].leader >= 0){
		tempwing = bv[i].leader;
		offangle = -0x4000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 150;  /*  600 ft put last to check formation  */
#else
		hsep = 4320;  /*  3 nm  */
#endif
		prespeed = bv[tempwing].basespeed;
	}
	else {
		tempwing = covernum;
		offangle = 0x4000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 25;  /*  100 ft put last to check formation */
#else
		hsep = 720;  /*  .5 nm  */
#endif
		prespeed = bv[tempwing].basespeed;
	}

//	dx = ps[tempwing].x + sinX(temphead, hsep);
//	dy = ps[tempwing].y - cosX(temphead, hsep);
	dx = ailoc[tempwing].xloc + sinXL(temphead, ((long)hsep * AILOCDIV));
	dy = ailoc[tempwing].yloc - cosXL(temphead, ((long)hsep * AILOCDIV));
	if(bv[i].leader < 0){
#ifdef ELFSHORT
		hsep = 300; /*  1200 ft put last to check  */
#else
		hsep = 8640;  /*  6 nm  */
#endif
		dx = dx + sinXL(ps[tempwing].head, ((long)hsep * AILOCDIV));
		dy = dy - cosXL(ps[tempwing].head, ((long)hsep * AILOCDIV));
	}
	ourleadalt = ps[tempwing].alt;

	if(tempwing == covernum)
		ourleadalt += 20;

	tempalt = ourleadalt + bv[i].verticlesep;

	if(tempalt < 75)
		tempalt = 75;

	LFlyTo(dx, dy, tempalt, tempwing, prespeed);

#if 0
	if(tempvar) {
		dx = LDist2D((ailoc[planecnt].xloc - dx), (ailoc[planecnt].xloc - dx));
		dx = dx / AILOCDIV;
		dx = dx >> HTOV;
		tdist = (int)dx;
		TXT("GCOV ");
		TXN(planecnt);
		TXA(" d ");
		TXN(tdist);
		TXA(" s ");
		TXN(bv[planecnt].basespeed);
		TXA(" ps ");
		TXN(prespeed);
		Message(Txt,DEBUG_MSG);
	}
#endif

}

void RearCover()
{
	int tnextsec;
	int prewing;
	long dx, dy;
	int dz;
	int relangle;
	int tangle, tpitch, troll, enemyalt, temphead, secplace;
	int tempwing, prespeed, tempspeed;
	COORD tdist, distkm, tdistnm;
	int offangle;
	int hsep;
	int ourleadalt, tempalt;
	int dpwhead;
	int covernum;

	i = planecnt;

	if(bv[planecnt].target == -1) {
		GetWhoCover(planecnt);
		if(bv[planecnt].target == -999) {
			bv[planecnt].Behavior = AIGoHome;
    		ps[planecnt].status |= BACKTOBASE;
			ChangeGroupBehavior(planecnt, 0);
		}
	}
	covernum = bv[planecnt].target;

	prewing = GetSquadLeader(planecnt);

	if(prewing == planecnt) {
		if(bv[bv[covernum].target].Behavior != GorillaCover) {
			if(bv[bv[covernum].target].Behavior == SetUpCAP1) {
				bv[planecnt].Behavior = SetUpCAP1;
				bv[planecnt].xloc = ps[planecnt].xL + 21600;
				bv[planecnt].yloc = ps[planecnt].yL;
				UpdateInterceptGroup(planecnt, 1);  /*  Might Need to do base here  */
			}
			else {
#if 0
				Message("Move Up Front",DEBUG_MSG);
#endif
				bv[planecnt].Behavior = GorillaCover;
				if(bv[planecnt].wingman >= 0) {
					bv[bv[planecnt].wingman].Behavior = GorillaCover;
				}
				bv[covernum].target = planecnt;
			}
		}
	}

	offangle = 0x4000;

	prespeed = 0;
	prespeed = bv[covernum].basespeed;

	if(bv[i].leader >= 0){
		tempwing = bv[i].leader;
		offangle = -0x4000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 150;  /*  600 ft put last to check formation  */
#else
		hsep = 4320;  /*  3 nm  */
#endif
		prespeed = bv[tempwing].basespeed;
	}
	else {
		if(bv[i].prevsec >= 0)
			tempwing = bv[i].prevsec;
		else
			tempwing = covernum;
		offangle = 0x4000;
		temphead = ps[tempwing].head + offangle;
#ifdef ELFSHORT
		hsep = 25;  /*  100 ft put last to check formation */
#else
		hsep = 720;  /*  .5 nm  */
#endif
		prespeed = bv[tempwing].basespeed;
	}

//	dx = ps[tempwing].x + sinX(temphead, hsep);
//	dy = ps[tempwing].y - cosX(temphead, hsep);
	dx = ailoc[tempwing].xloc + sinXL(temphead, ((long)hsep * AILOCDIV));
	dy = ailoc[tempwing].yloc - cosXL(temphead, ((long)hsep * AILOCDIV));
	if(bv[i].leader < 0){
#ifdef ELFSHORT
		hsep = 150; /*  600 ft put last to check  */
#else
		hsep = 4320;  /*  3 nm  */
#endif
		dx = dx + sinXL((ps[tempwing].head + 0x8000), ((long)hsep * AILOCDIV));
		dy = dy - cosXL((ps[tempwing].head + 0x8000), ((long)hsep * AILOCDIV));
	}
	ourleadalt = ps[tempwing].alt;

	if(tempwing == covernum)
		ourleadalt += 20;

	tempalt = ourleadalt + bv[i].verticlesep;

	if(tempalt < 75)
		tempalt = 75;

	LFlyTo(dx, dy, tempalt, tempwing, prespeed);
}

#define SARCENTER 16627
#define SARALT 2000

void SARCAP1()
{
	int wingnum;
	long xval, yval;
	UWORD zval;

	i = planecnt;

	CheckCAP();

	if(bv[i].groundt != -1) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}

	if(SARCAP0(0, (long)SARCENTER, SARALT)) {
		wingnum = -999;
		if(bv[i].leader > -1)
			wingnum = bv[i].leader;
		else if(bv[i].wingman > -1)
			wingnum = bv[i].wingman;
		bv[i].Behavior = SARCAP2;
		if(!(bv[i].behaveflag & DAMAGERADAR))
			bv[i].behaveflag |= RADARON;
		if(bv[i].wingman > -1) {
			if((bv[wingnum].Behavior == SARCAP1) || (bv[wingnum].Behavior == SARCAP2)
					|| (bv[wingnum].Behavior == Formation)) {
				bv[wingnum].basespeed = 450;
				bv[wingnum].Behavior = SARCAP3;
			}
			if(!(bv[wingnum].behaveflag & DAMAGERADAR))
				bv[wingnum].behaveflag |= RADARON;
			bv[wingnum].behaveflag &= ~ENEMYNOFIRE;
		}
		bv[i].basespeed = 400;
	}
}

void SARCAP2()
{
	int wingnum;
	long xval, yval;
	UWORD zval;

	i = planecnt;

	CheckCAP();

	if(bv[i].groundt != -1) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}

	if(SARCAP0(0xA000, (long)SARCENTER, SARALT)) {
		wingnum = -999;
		if(bv[i].leader > -1)
			wingnum = bv[i].leader;
		else if(bv[i].wingman > -1)
			wingnum = bv[i].wingman;
		bv[i].Behavior = SARCAP3;
		if(!(bv[i].behaveflag & DAMAGERADAR))
			bv[i].behaveflag |= RADARON;
		if(bv[i].wingman > -1) {
			if(bv[wingnum].Behavior == SARCAP3) {
				bv[wingnum].basespeed = 450;
				bv[wingnum].Behavior = SARCAP1;
			}
			if(!(bv[wingnum].behaveflag & DAMAGERADAR))
				bv[wingnum].behaveflag |= RADARON;
			bv[wingnum].behaveflag &= ~ENEMYNOFIRE;
		}
		bv[i].basespeed = 400;
	}
}

void SARCAP3()
{
	int wingnum;
	long xval, yval;
	UWORD zval;

	i = planecnt;

	CheckCAP();

	if(bv[i].groundt != -1) {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		bv[i].xloc = xval;
		bv[i].yloc = yval;
	}

	if(SARCAP0(0x6000, (long)SARCENTER, SARALT)) {
		wingnum = -999;
		if(bv[i].leader > -1)
			wingnum = bv[i].leader;
		else if(bv[i].wingman > -1)
			wingnum = bv[i].wingman;
		bv[i].Behavior = SARCAP1;
		if(!(bv[i].behaveflag & DAMAGERADAR))
			bv[i].behaveflag |= RADARON;
		if(bv[i].wingman > -1) {
			if(bv[wingnum].Behavior == SARCAP1) {
				bv[wingnum].basespeed = 450;
				bv[wingnum].Behavior = SARCAP2;
			}
			if(!(bv[wingnum].behaveflag & DAMAGERADAR))
				bv[wingnum].behaveflag |= RADARON;
			bv[wingnum].behaveflag &= ~ENEMYNOFIRE;
		}
		bv[i].basespeed = 400;
	}
}

int SARCAP0(int whatangle, long whatdist, int whatalt)
{
	int dx, dy, dalt;
	int temphead, temphead2;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int smalldist;
	int secplace;
	/*   Fly aircraft to one 'corner' of the CAP pattern  */

	i = planecnt;
	secplace = (bv[planecnt].behaveflag & 15);

	dx = bv[i].xloc>>HTOV;
	dy = bv[i].yloc>>HTOV;
	dalt = SARALT + (secplace * 125);
	temphead = whatangle;
	smalldist = (int)(whatdist>>HTOV);
	dx += sinX(whatangle, smalldist);
	dy -= cosX(whatangle, smalldist);

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

int GetWayPointChanges(int numorgwp, int *behavenum, int *formnum, int *nspeed, int *elevation, int *heading, int *attack, int *launch, int *wpflag)
{
	int cnt;
	int numactions;
	int startaction;
	int whataction;
	int temphead;
	UWORD temphead2;
	UWORD actionval;
	int numwp;
	int returnval = -999;

	*behavenum = -1;
	*formnum = -1;
	*nspeed = -1;
	*elevation = -1;
	*heading = -1;
	*attack = -1;
	*launch = -1;
	*wpflag = 0;

	numwp = numorgwp;
	if(numwp <= -1)
		return(returnval);

	if(aiwaypoints[numwp].x == (long)-2)
		numwp += 1;

	numactions = aiwaypoints[numwp].numActions;
	startaction = aiwaypoints[numwp].actionIndex;

	if(numactions == 0)
		return(returnval);

	for(cnt = startaction; cnt < (startaction + numactions); cnt ++) {
		whataction = aiactions[cnt].action;
		actionval = aiactions[cnt].value;

		if(whataction == SA_SPEED) {
			if(actionval != 0)
				*nspeed = actionval;
		}
		else if(whataction == SA_ELEVATION) {
			*elevation = actionval>>2;
		}
		else if(whataction == SA_HEADING) {
			temphead = actionval;
			while(temphead >= 360)
				temphead -= 360;
			while(temphead < 0)
				temphead += 360;

			temphead2 = 0;
			while(temphead >= 45) {
				temphead -= 45;
				temphead2 += 0x2000;
			}
			temphead2 += temphead * 182;

			*heading = temphead2;
		}
		else if(whataction == SA_HEADING) {
			*heading = actionval;
		}
		else if(whataction == SA_LAUNCH) {
			*launch = 1;
		}
		else if(whataction == SA_ATTACK) {
			*attack = 1;
		}
		else if(whataction == SA_DAMAGE) {
			*wpflag |= DOWPDAMAGE;
		}
		else if(whataction == SA_DITCH) {
			*wpflag |= DOWPDITCH;
		}
		else if(whataction == SA_EXPLODE) {
			*wpflag |= DOWPEXPLODE;
		}
		else if(whataction == SA_DEFENSIVEBFM) {
			*wpflag |= DOWPDBFM;
		}
		else if(whataction == SA_OFFENSIVEBFM) {
			*wpflag |= DOWPOBFM;
		}
		else if(whataction == SA_NEUTRALBFM) {
			*wpflag |= DOWPNBFM;
		}
		else if(whataction == SA_HARDDECK) {
			if(!KnockItOffStatus)
				KnockItOffStatus = actionval>>2;
		}
		else if(whataction == SA_PEACETIME) {
			warstatus = 0;
		}
		else if(whataction == SA_HAWKEYE) {
			*wpflag |= DOWPHAWKEYE;
			OurPath = actionval;
		}
		else if(whataction == SA_FORM_ON) {
			*wpflag |= DOWPFORMON;
		}
		else if(whataction == SA_TROOPERDROP) {
			if(Gtime != 0)
				TrooperDrop(planecnt);
			*wpflag |= DOWPTROOPDROP;
		}
		else if(whataction == SA_LAND) {
			*wpflag |= DOWPLAND;
		}
		else if(whataction == SA_FORMTRAIN) {
			DOINVISIBLE = 1;
		}
		else if(whataction == SA_IMMORTAL) {
			ENEMYNODIE = 1;
		}
		else if(whataction == SA_TRAINING) {
			Status |= TRAINING;
		}
		else if(whataction == SA_GOAL) {
			*wpflag |= DOWPGOAL;
		}
		else if(whataction == SA_FORMATION) {
			*formnum = actionval;
			if(*behavenum == -1) {
				*behavenum = SA_FORMATION;
			}
		}
		else if(whataction == SA_HOVER) {
			*wpflag |= DOWPHOVER;
			returnval = actionval;
		}
		else {
			*behavenum = whataction;
		}
	}
	return(returnval);
}

int DoNextWayPt(int planenum, int waypt)
{
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;
	int aistation;
	struct PlaneData far *tptype;
	int returnval = 0;
	int treturnval;
	int temppath;
	int basetemp;

	temppath = (int)bv[planenum].path;
	if((waypt < 0) || (temppath < 0))
		return(0);

	if((aipaths[temppath].wp0 + aipaths[temppath].numWPs) <= waypt) {
		if(IsBombing(planenum))
			bv[planenum].Behavior = Formation;
		return(0);
	}


	GetWayPointChanges(waypt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);

	if(behavenum != -1)
		returnval = TranslateBehavior(planenum, behavenum, formnum);

	if(attack != -1) {
		LaunchCruise(planenum);
	}
	if(wpflag & DOWPEXPLODE) {
		LogIt (SLG_AIR_DESTROYED_MISSILE, i, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, -999, -999, 0L, LOG_AIR);
		AirKill(planenum);
		returnval = 1;
	}
	if(wpflag & DOWPTROOPDROP) {
		DoGroupAction(planenum, DOWPTROOPDROP, 0);
		returnval = 1;
	}
	if(wpflag & DOWPHAWKEYE) {
		Message("Recieved New Waypoints",RIO_MSG);
	}
	if(wpflag & DOWPDAMAGE) {
		returnval = 1;
	}
	if(wpflag & DOWPDITCH) {
		returnval = 1;
	}
/*	if(wpflag & DOWPGOAL) {
		LogIt (SLG_WAY_PT_GOAL, planenum, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, -(int)bv[planenum].path, waypt, 0L, LOG_AIR);
		DoGroupAction(planenum, DOWPGOAL, waypt);
	}  */
	if((launch != -1) && (bv[planenum].Behavior != Ready5)) {
		SetUpLaunch(planenum, waypt);
	}
	if(wpflag & DOWPFORMON) {
		treturnval = FormOnWP(planenum);
		if(treturnval != 0)
			returnval = treturnval;
	}
	return(returnval);
}

void GetWayXYZ(int waynum, long *xval, long *yval, UWORD *zval)
{
	int cnt;
	int otherplane;

	if(aiwaypoints[waynum].x >= (long)0) {
		*xval = aiwaypoints[waynum].x;
		*yval = aiwaypoints[waynum].y;
		*zval = aiwaypoints[waynum].z;
		return;
	}

	otherplane = (int)aiwaypoints[waynum].y;

	GetPlanePointXYZ(otherplane, xval, yval, zval);

//	if((ps[otherplane].alt < 10) && (aiwaypoints[waynum].z > 10))
	if(aiwaypoints[waynum].z > (long)10)
		*zval = aiwaypoints[waynum].z;

	if(aiwaypoints[waynum].x == (long)-2) {
		*xval += aiwaypoints[waynum + 1].x;
		*yval += aiwaypoints[waynum + 1].y;
//		*zval += aiwaypoints[waynum + 1].z;  Would make relative WP alt relative to obj
											//  OK for boats (Low alt) but could be bad for planes
	}
}

void GetPlanePointXYZ(int planenum, long *xval, long *yval, UWORD *zval)
{
	int cnt;
	int otherplane;
	long tx, ty;
	UWORD tz;

	otherplane = planenum;

	cnt = GetPlaneOrgNum(planenum);

	if(cnt == -1) {
		*xval = PxLng;
		*yval = 0x100000 - PyLng;
		*zval = Alt;
		return;
	}
	if(cnt == MAXPLANE) {
		for(cnt = 0; cnt < NGTARGETS; cnt ++) {
			if(gt[cnt].orgnum == planenum) {
				*xval = gt[cnt].x;
				*yval = gt[cnt].y;
				*zval = 1;
				return;
			}
		}
		*xval = 200000;
		*yval = 200000;
		*zval = 66;
		TXT("No find loc for ");
		TXN(otherplane);
		Message(Txt,DEBUG_MSG);
		return;
	}

	*xval = tx = ps[cnt].xL;
	*yval = ty = ps[cnt].yL;
	*zval = tz = ps[cnt].alt;
}

void Ready5()
{
	i = planecnt;

	droll = 0;
	pdest = 0;
	bv[planecnt].basespeed = 0;
}

int ReturnBaseInfo(int basenum, COORD *xloc, COORD *yloc, UWORD *zloc, int *basehead)
{
	int tbnum;
	int cnt;
	int otherobj;
	struct PlaneData far *tempplane;


	if(basenum < 0) {
		tbnum = (basenum * -1) - 1;
		*xloc = ps[tbnum].x;
		*yloc = ps[tbnum].y;
		*zloc = 0;
		*basehead = ps[tbnum].head;
#if 0
		if((tbnum < NPLANES) || (tbnum > MAXPLANE))
			tempplane = GetPlanePtr(ps[tbnum].type);
#endif
		return(1);
	}
	else {
		*xloc = Rdrs[basenum].x;
		*yloc = Rdrs[basenum].y;
		*zloc = 0;
		*basehead = 0;
		return(0);
	}
	return(0);
}

#ifdef SHOWBEHAVIORS
void ShowWhichBehave(int planenum)
{
	TXT(" ");
	TXN(planenum);
	if(bv[planenum].Behavior == Box)
		TXA(" Box");
	else if(bv[planenum].Behavior == FlyWayPoint)
		TXA(" FlyWayPoint");
	else if(bv[planenum].Behavior == Champagne)
		TXA(" Champagne");
	else if(bv[planenum].Behavior == Wall)
		TXA(" Wall");
	else if(bv[planenum].Behavior == Wall2)
		TXA(" Wall2");
	else if(bv[planenum].Behavior == Ladder)
		TXA(" Ladder");
	else if(bv[planenum].Behavior == Ladder2)
		TXA(" Ladder2");
	else if(bv[planenum].Behavior == LadderCircle)
		TXA(" LadderCircle");
	else if(bv[planenum].Behavior == Formation)
		TXA(" Formation");
	else if(bv[planenum].Behavior == StraightAttack)
		TXA(" Straight Attack");
	else if(bv[planenum].Behavior == Jink)
		TXA(" Jinking");
	else if(bv[planenum].Behavior == Jink2)
		TXA(" Jinking2");
#if ELFREFUEL
	else if(IsRefueling(planenum))
		TXA(" Refueling");
#endif
	else if(IsCAPFlier(planenum))
		TXA(" CAP");
	else if(bv[planenum].Behavior == AIBombTarget)
		TXA(" Bomber");
	else if(bv[planenum].Behavior == GorillaBomb)
		TXA(" GorillaBomb");
	else if(bv[planenum].Behavior == AIGoHome) {
		TXA(" Going Home");
		if(ps[planenum].status & LANDING)
			TXA(" Landing");
	}
	else if(bv[planenum].Behavior == AIBombTarget)
		TXA(" Bomb Target");
	else if(bv[planenum].Behavior == AIGetBombDist)
		TXA(" Get Bomb Dist");
	else if(bv[planenum].Behavior == LowBombTarget)
		TXA(" Low Bomb");
	else if(bv[planenum].Behavior == DoHover)
		TXA(" Do Hover");
	else if(bv[planenum].Behavior == BracketLeft)
		TXA(" Bracket Left");
	else if(bv[planenum].Behavior == BracketRight)
		TXA(" BracketRight");
	else
		ShowWhichBehave2(planenum);

	TXA(" ");
	TXN(bv[planenum].target);
/*	TXA(" x ");
	TXN(ps[planenum].x);
	TXA(" y ");
	TXN(ps[planenum].y);  */
	TXA(" on ");
	TXN((int)bv[planenum].orgnum);
	TXA(" spd ");
	TXN(bv[planenum].basespeed);
	TXA(" pn ");
	TXN((bv[planenum].behaveflag & 15));
	if(bv[planenum].behaveflag & ENEMYNOFIRE) {
		TXA(" NOFIRE");
	}
	else {
		TXA(" FIRE");
	}
	TXA(" ");
	TXN(bv[planenum].groundt);
	TXA(" ");
	TXN(bv[planenum].varcnt);
	TXA(" ");
	TXN(ps[planenum].speed);
	TXA(" ");
	TXN(bv[planenum].horizontalsep);
	if(ps[planenum].status & TAKEOFF)
		TXA(" TAKEOFF");

	Message(Txt,DEBUG_MSG);
}

void ShowWhichBehave2(int planenum)
{
	if(bv[planenum].Behavior == FlyStraight)
		TXA(" Fly Straight");
	else if(bv[planenum].Behavior == FreeFighter)
		TXA(" FreeFighter");
	else if(bv[planenum].Behavior == SAR)
		TXA(" SAR");
	else if(bv[planenum].Behavior == SARWait)
		TXA(" SAR Wait");
	else if(bv[planenum].Behavior == GorillaCover)
		TXA(" GorillaCover");
	else if(bv[planenum].Behavior == RearCover)
		TXA(" RearCover");
	else if(bv[planenum].Behavior == FlyCruise)
		TXA(" Fly Cruise");
#if 0
	else if(bv[planenum].Behavior == FlySanitize)
		TXA(" Fly Sanitize");
	else if(bv[planenum].Behavior == FlyWPFollowPlayer)
		TXA(" FlyWPFollowPlayer");
	else if(bv[planenum].Behavior == FollowPlayer)
		TXA(" FollowPlayer");
//	else if(bv[planenum].Behavior == FlyE2C)
//		TXA(" FlyE2C");
#endif
	else
		TXA(" Other");
}

#endif

int GetPlayerHelp()
{
	int cnt;
	int returnval = -999;
	COORD tdist, cdist;

	cdist = 5400;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((FriendMatchType(cnt, FRIENDLY)) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((cnt == GetSquadLeader(cnt)) && ((IsCAPFlier(cnt)) || ((bv[cnt].Behavior == Ready5) && (bv[cnt].threathead >= 0)))) {
				if(CheckIfCanIntercept(cnt, 0) && (bv[cnt].target == -1)) {
					tdist = Dist2D((ps[cnt].x - Px), (ps[cnt].y - Py));
					if(tdist < cdist) {
						tdist = cdist;
						returnval = cnt;
					}
				}
			}
		}
	}
	return(returnval);
}

void ProcessOtherComm()
{
	int helpai;

	if(!behavecommands)
		return;

	switch (behavecommands) {
		case AIPLAYERHELP:     /*   SHIFT F1 KEY  */
			helpai = GetPlayerHelp();
			if(helpai != -999) {
				if(bv[helpai].Behavior == Ready5) {
					ps[helpai].status |= TAKEOFF;
				}
				else if(IsCAPFlier(helpai)) {
					DoCAPReplace(helpai);
				}
				bv[helpai].behaveflag |= ENEMYNOFIRE;
				bv[helpai].Behavior = Formation;
				bv[helpai].horizontalsep = CRUISEDIST;
				bv[helpai].behaveflag &= ~COMBAT_SPREAD;
				UpdateInterceptGroup(helpai, 1);
				bv[helpai].Behavior = FollowPlayer;
				TXT("(Carrier)Vectoring Assistance ");
				TXN(helpai);
				Message(Txt,GAME_MSG);
			}
			else {
				TXA("(Carrier)No Assistance Available");
				Message(Txt,GAME_MSG);
			}
		break;
	}

}

void DoCAPReplace(int planenum)
{
	int cnt;
	int isfriend;
	COORD tdist;
	struct PlaneData far *tptype;

	isfriend = FriendStatus(planenum);

	for(cnt = 0; cnt < NPLANES; cnt ++ ) {
		if((FriendMatchType(cnt, isfriend)) && (cnt != planenum)){
			if((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && ((bv[cnt].Behavior == Ready5) && (bv[cnt].varcnt <= Gtime) && (bv[cnt].threathead >= 0))) {
				if(cnt == GetSquadLeader(cnt)) {
					tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
					if(tdist < 7250) {  /*  was 5400  */
						tptype = GetPlanePtr(ps[cnt].type);
						if(tptype->speed > 500) {
							bv[cnt].basespeed = tptype->speed - 100;
						}
						else {
							bv[cnt].basespeed = tptype->speed - 50;
						}
						bv[cnt].Behavior = SetUpCAP1;
						ps[cnt].status |= TAKEOFF;
						bv[cnt].groundt = bv[planenum].groundt;
						bv[cnt].path = bv[planenum].path;
						bv[cnt].xloc = bv[planenum].xloc;
						bv[cnt].yloc = bv[planenum].yloc;
						UpdateInterceptGroup(cnt, 1);
						return;
					}
				}
			}
		}
	}
}

void SARWait()
{
	i = planecnt;

	bv[planecnt].basespeed = 0;
	ps[planecnt].speed = 0;
	droll = -ps[planecnt].roll;
	pdest = 0;
}

void SAR()
{
	int dx, dy, dalt;
	COORD tdist;
	int covernum;

	i = planecnt;

	dx = bv[i].xloc>>HTOV;
	dy = bv[i].yloc>>HTOV;
	dalt = 200;

	if(FlyTo(dx, dy, dalt, 0)) {
		DoHover();
		if(ps[planecnt].alt > 13) {
			pdest = -0x1000;
		}
		else {
			pdest = 0;
			if(!(bv[planecnt].varcnt)) {
				bv[planecnt].varcnt = Gtime + 170;
			}
			else if((bv[planecnt].varcnt) - Gtime < 20) {
//				bv[planecnt].basespeed = ptype->speed;
				bv[planecnt].basespeed = 120;
				covernum = bv[planecnt].target;
				if(covernum >= 0) {
					if(IsCAPFlier(covernum)) {
						if(FriendMatch(planecnt, covernum)) {
							bv[covernum].Behavior = GorillaCover;
							bv[covernum].target = planecnt;
							UpdateInterceptGroup(covernum, 1);
							if(bv[covernum].path >= 0) {
								bv[covernum].groundt = aipaths[bv[covernum].path].wp0;
							}
#if 0
							TXT("Cover SAR ");
							TXN(covernum);
							Message(Txt,DEBUG_MSG);
#endif
						}
					}
				}
			}
		}
	}
	else {
		if(tdist > 40) {
			if(bv[planecnt].basespeed < 100) {
				bv[planecnt].basespeed += 3;
				droll = 0;
			}
			else {
				bv[planecnt].basespeed = ptype->speed;
			}

			if(ps[i].alt < dalt)
				droll = 0;
		}
		else {
			if(bv[planecnt].basespeed < 20) {
				bv[planecnt].basespeed += 2;
			}
		}

#if 0
		if(tempvar) {
			tdist = Dist2D((ps[planecnt].x - dx), (ps[planecnt].y - dy));
			TXT("SAR Dist MOVE ");
			TXN(tdist);
			Message(Txt,DEBUG_MSG);
		}
#endif
	}
}

void GetSAR(int planenum)
{
	int cnt;
	int returnval = -999;
	COORD tdist, cdist;
	int isfriend;

	isfriend = FriendStatus(planenum);

	cdist = 0;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((FriendMatchType(cnt, isfriend)) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((cnt == GetSquadLeader(cnt)) && (bv[cnt].Behavior == SARWait)) {
				tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
				if((tdist < cdist) || (returnval == -999)) {
					tdist = cdist;
					returnval = cnt;
				}
			}
		}
	}
	if(returnval == -999)
		return;

	ps[returnval].status |= TAKEOFF;
	bv[returnval].Behavior = SAR;
	bv[returnval].xloc = ps[planenum].xL;
	bv[returnval].yloc = ps[planenum].yL;
	bv[returnval].basespeed = 3;
	GetSARCover(returnval);
}

void GetSARCover(int planenum)
{
	int cnt;
	int returnval = -999;
	COORD tdist, cdist;
	int isfriend;

	isfriend = FriendStatus(planenum);

	cdist = 0;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((FriendMatchType(cnt, isfriend)) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((cnt == GetSquadLeader(cnt)) && ((bv[cnt].Behavior == Ready5) && (bv[cnt].varcnt <= Gtime)  && (bv[cnt].threathead >= 0))) {
				tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
				if((tdist < cdist) || (returnval == -999)) {
					tdist = cdist;
					returnval = cnt;
				}
			}
		}
	}
	if(returnval == -999)
		return;

	bv[returnval].Behavior = SARCAP1;
	bv[returnval].xloc = bv[planenum].xloc;
	bv[returnval].yloc = bv[planenum].yloc;
	bv[returnval].groundt = -1;
	ps[returnval].status |= TAKEOFF;
	bv[returnval].target = planenum;
	if(ps[returnval].base >= 0) {
		ps[returnval].base = -1;
	}
	UpdateInterceptGroup(returnval, 1);  /*  Might Need to do base here  */
	bv[returnval].varcnt = Gtime + 20;
	bv[planenum].target = returnval;
}

void RestoreCover(int planenum)
{
	int cnt;
	int returnval = -999;
	COORD tdist, cdist;
	int isfriend;
	int lastcap = -1;
	int prewing;

	isfriend = FriendStatus(planenum);
	prewing = GetSquadLeader(planenum);

	cdist = 2250;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((FriendMatchType(cnt, isfriend)) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((cnt == GetSquadLeader(cnt)) && (IsCAPFlier(cnt)) && (bv[cnt].target != -1)) {
				tdist = Dist2D((ps[cnt].x - ps[planenum].x), (ps[cnt].y - ps[planenum].y));
				if(tdist < cdist) {
#if 0
					TXT("Returning to Cover ");
					TXN(cnt);
					Message(Txt,DEBUG_MSG);
#endif
					returnval = cnt;
					bv[returnval].Behavior = GorillaCover;
					bv[returnval].xloc = bv[planenum].xloc;
					bv[returnval].yloc = bv[planenum].yloc;
					bv[returnval].groundt = -1;
					bv[returnval].target = prewing;
					if(lastcap == -1) {
						bv[prewing].target = returnval;
						lastcap = returnval;
					}
					else {
						bv[lastcap].nextsec = returnval;
						bv[returnval].prevsec = lastcap;
						lastcap = returnval;
					}
					UpdateInterceptGroup(returnval, 1);  /*  Might Need to do base here  */
				}
			}
		}
	}
}

int IsBombing(int planenum)
{
	if((bv[planenum].Behavior == AIBombTarget) || (bv[planenum].Behavior == AIGetBombDist) || (bv[planenum].Behavior == LowBombTarget)) {
		return(1);
	}
	return(0);
}

int FriendStatus(int planenum)
{
	if((planenum <= -1) || (planenum >= MAXPLANE))
		return(FRIENDLY);
	return((ps[planenum].status & (FRIENDLY|NEUTRAL)));
}

int FriendMatch(int plane1, int plane2)
{

	/*  NEED TO PUT CODE FOR NEUTRALS HERE  */

	if(FriendStatus(plane1) == FriendStatus(plane2))
		return(1);
	else
		return(0);
}

int FriendMatchType(int plane1, int isfriend)
{

	/*  NEED TO PUT CODE FOR NEUTRALS HERE  */

	if(FriendStatus(plane1) == isfriend)
		return(1);
	else
		return(0);
}

void UpdateOurWPS(int lastwp, int updateall)
{
	int waynum;
	long xval, yval;
	UWORD zval;
	int cnt, cnt2;
	int iscarr = 0;
	int NextNum = 1;
	int waystart = 0;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	if(NextWayPoint == 0)
		return;

//	waynum = aipaths[OurPath].wp0 + NextWayPoint - 1;

	cnt2 = 1;
	waynum = aipaths[OurPath].wp0;

	if((aiwaypoints[waynum].x == -1L) && ((GetPlaneOrgNum((int)aiwaypoints[waynum].y) == whichourcarr))) {
		waynum += 1;
		waystart = 1;
	}
	else if(aiwaypoints[waynum].x > -1L) {
		GetWayPointChanges(waynum, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
		if(behavenum != SA_CAP) {
			waynum += 1;
			waystart = 1;
		}
	}

	for(cnt = 0; cnt < (NextWayPoint - 1); cnt ++) {
		if(aiwaypoints[waynum].x == -2L)
			waynum += 1;
		waynum += 1;
		cnt2 += 1;
	}

	if((aiwaypoints[waynum].x == -1L) && ((GetPlaneOrgNum((int)aiwaypoints[waynum].y) == whichourcarr)))
		iscarr = 1;

	if((waynum >= (aipaths[OurPath].wp0 + aipaths[OurPath].numWPs))  ||
			((iscarr) && (waynum >= (aipaths[OurPath].wp0 + aipaths[OurPath].numWPs - 1)))){
		if(lastwp != -999) {
			if(lastwp == 0) {
				waynum = aipaths[OurPath].wp0 + aipaths[OurPath].numWPs - 1;
				NextWayPoint = cnt2;
			}
			else {
				NextWayPoint = 0;
				return;
			}
		}
		else
			waynum = aipaths[OurPath].wp0 + aipaths[OurPath].numWPs - 1;
	}

//	if((!(Rtime & 127)) || (Gtime < 2)) {
	if(updateall) {
		for(cnt = waystart; cnt < aipaths[OurPath].numWPs; cnt ++) {
			if(NextNum < 4) {
				waynum = aipaths[OurPath].wp0 + cnt;
				GetWayXYZ(waynum, &xval, &yval, &zval);
				wps[NextNum].xL = xval;
				wps[NextNum].yL = yval;
				wps[NextNum].x = wps[NextNum].xL>>HTOV;
				wps[NextNum].y = wps[NextNum].yL>>HTOV;
				if(aiwaypoints[waynum].x == -2L)
					cnt += 1;
			}
			NextNum += 1;
		}
	}
	else {
		GetWayXYZ(waynum, &xval, &yval, &zval);
		wps[NextWayPoint].xL = xval;
		wps[NextWayPoint].yL = yval;
		wps[NextWayPoint].x = wps[NextWayPoint].xL>>HTOV;
		wps[NextWayPoint].y = wps[NextWayPoint].yL>>HTOV;
	}

#if 0
	if(tempvar) {
		sprintf(Txt, "WAY %d, x %ld, y %ld", waynum, xval, yval);
		Message(Txt);
	}
#endif
}

int CloseEnoughFor(int whatcheck, int x, int y)
{
	int distval;

	switch(whatcheck) {
		case SNDSTUFF:
			distval = 1800;
			break;

		case SLOWSTUFF:
			distval = 3600;
			break;

		case HITSTUFF:
			distval = 4000;
			break;

		default:
			distval = 0;
			break;
	}

	if(abs(x - Px) > distval)
		return(0);

	if(abs(y - Py) > distval)
		return(0);

	return(1);
}

int FormOnWP(int planenum)
{
	int whofollow;
	int planeorg;
	int waynum;
	int wingcnt;
	int tmpcnt;
	int secplace = 0;
	int orgsecplace;
	int orgprewing;
	int cnt;

	orgprewing = GetSquadLeader(planenum);

	if(orgprewing != planenum)
		return(0);

	waynum = bv[planenum].groundt;
	planeorg = (int)aiwaypoints[waynum].y;
	if(aiwaypoints[waynum].x > (long)-1) {
#if 0
	if((aiwaypoints[waynum].x > (long)-1) || (planeorg == orgPlayernum)) {
		TXT("Can't Form On WP ");
		TXN(planenum);
		TXA(" org ");
		TXN((int)bv[planenum].orgnum);
		Message(Txt,DEBUG_MSG);
#endif

		bv[planenum].groundt += 1;
		return(DoNextWayPt(planenum, bv[planenum].groundt));
	}
#if 0
	else {
		TXT("Form ");
		TXN(orgprewing);
		TXA(" on ");
		TXN(planeorg);
		Message(Txt,DEBUG_MSG);
	}
#endif

	orgsecplace = bv[planenum].behaveflag & 15;
	whofollow = GetPlaneOrgNum(planeorg);
	if(whofollow == -1) {
		for(cnt = 0; cnt < NPLANES; cnt ++) {
			if(bv[cnt].prevsec == -1) {
				whofollow = cnt;
				break;
			}
		}
		if(cnt == NPLANES) {
			InitAddedWingmen (planenum);
			bv[planenum].groundt += 1;
			return(DoNextWayPt(planenum, bv[planenum].groundt));
		}
	}
	wingcnt = whofollow;
	bv[planenum].groundt = bv[whofollow].groundt;
	bv[planenum].path = bv[whofollow].path;
	while (wingcnt >= 0) {
		if((bv[wingcnt].wingman >= 0) || (bv[planenum].wingman >= 0)) {
			if(bv[wingcnt].nextsec >= 0) {
				wingcnt = bv[wingcnt].nextsec;
				secplace += 2;
			}
			else {
				secplace += 2;
				bv[wingcnt].nextsec = planenum;
				bv[planenum].prevsec = wingcnt;
				wingcnt = -999;
			}
		}
		else {
			secplace += 1;
			bv[wingcnt].wingman = planenum;
			bv[planenum].leader = wingcnt;
			wingcnt = -999;
		}
	}
	if(secplace > 15)
		secplace = 15;

	bv[planenum].behaveflag += secplace;
	orgsecplace = secplace;
	wingcnt = orgprewing;
	while (wingcnt >= 0) {
		tmpcnt = bv[wingcnt].wingman;
		if(tmpcnt >= 0) {
			if(tmpcnt != planenum)
				bv[tmpcnt].behaveflag += orgsecplace;
		}
		wingcnt = bv[wingcnt].nextsec;
		if(wingcnt >= 0) {
			if(wingcnt != planenum)
				bv[wingcnt].behaveflag += orgsecplace;
		}
	}
	return(DoNextWayPt(planenum, bv[planenum].groundt));
}

int GetPlaneOrgNum(int planenum)
{
	int cnt;
	int otherplane;

	otherplane = planenum;
	for(cnt = 0; cnt < MAXPLANE; cnt ++) {
    	if ((ps[cnt].status&ACTIVE) && (ps[cnt].status & ALIVE)) {
			if((int)bv[cnt].orgnum == otherplane) {
				return(cnt);
			}
		}
	}
	if(otherplane == orgPlayernum)
		return(-1);

	return(MAXPLANE);
}

int CheckForMobileR5(int basenum, int isfriend)
{
	int cnt;
	int returnval = -999;
	COORD tdist, cdist;
	int wingcnt;
	int otherplane;
	int xval, yval;

	cdist = 0;

	xval = Rdrs[basenum].x;
	yval = Rdrs[basenum].y;

	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((!FriendMatchType(cnt, isfriend)) && (ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((cnt == GetSquadLeader(cnt)) && (bv[cnt].Behavior == Ready5) && (ps[cnt].base >= 0)) {
				tdist = Dist2D((ps[cnt].x - xval), (ps[cnt].y - yval));
				if((tdist < cdist) || (returnval == -999)) {
					tdist = cdist;
					returnval = cnt;
				}
			}
		}
	}
	if(returnval == -999)
		return(returnval);

	yval += 30;
	MovePlane(returnval, xval, yval);
	yval += 1;

	wingcnt = returnval;
	while(wingcnt >= 0) {
		otherplane = bv[wingcnt].wingman;
		if(otherplane >= 0) {
			MovePlane(otherplane, xval, yval);
			yval = yval + 1;
		}
		otherplane = bv[wingcnt].nextsec;
		if(otherplane >= 0) {
			MovePlane(otherplane, xval, yval);
			yval = yval + 1;
		}
		wingcnt = bv[wingcnt].nextsec;
	}
#if 0
	Message("Planes Moved",DEBUG_MSG);
#endif
	return(returnval);
}

int MovePlane(int planenum, int xval, int yval)
{
	ps[planenum].x = xval;
	ps[planenum].y = yval;
	ps[planenum].xL = ((long)ps[planenum].x)<<HTOV;
	ps[planenum].yL = ((long)ps[planenum].y)<<HTOV;
	ailoc[i].xloc = ((long)ps[planenum].xL * AILOCDIV) + (AILOCDIV>>1);
	ailoc[i].yloc = ((long)ps[planenum].yL * AILOCDIV) + (AILOCDIV>>1);
	ps[planenum].head = 0;
}

void CruiseDistruct(int planenum)
{
	if(ps[planenum].alt < 300) {
		LogIt (SLG_GROUND_CRUISE_HIT, -999, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, bv[planenum].prevsec, ps[planenum].type, 0L, LOG_GRND);
		CheckBlastArea(ps[planenum].xL, ps[planenum].yL, W_MK_84, planenum);
	}

	CheckPlaneBlast(ps[planenum].x, ps[planenum].y, ps[planenum].alt, 10, planenum, bv[planenum].prevsec, ps[planenum].type);
}

void CheckPlaneBlast(int xval, int yval, UWORD zval, int blastrad, int iscruise, int camefrom, int missiletype)
{
	int cnt;
	struct PlaneData far *tempplane;
	int doKill = 0;
	int logtype;
	long lxval, lyval;
	int hitmsg, destroymsg;
	int crashstat;

	lxval = (long)xval<<HTOV;
	lyval = (long)yval<<HTOV;

	if(xydist((Px - xval), (Py - yval)) < blastrad) {
		if(abs(Alt - zval) < 300) {
	        DamageMe(13);
    	    Txt[0]=0;
	    	StartASmoke (PxLng, Alt, 0x100000-PyLng, AIRXPLO, 10);
			TXT("Damaged By ");
			if(iscruise > -1) {
    			FTXA (ptype->name);
				LogIt (SLG_AIR_CRUISE_HIT, -1, lxval, lyval, zval, camefrom, missiletype, 0L, LOG_AIR);
			}
			else {
				TXA("BOMB");
				LogIt (SLG_AIR_BOMB_HIT, -1, lxval, lyval, zval, camefrom, missiletype, 0L, LOG_AIR);
			}
			Message(Txt,RIO_MSG);
		}
	}

	for(cnt = (MAXPLANE - 1); cnt >= 0; cnt--) {
    	Txt[0]=0;
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & CRASH))) {
			if((xydist((ps[cnt].x - xval), (ps[cnt].y - yval)) < blastrad) && (bv[cnt].Behavior != FlyCruise)) {
				if(abs(ps[cnt].alt - zval) < 300) {
					if(cnt > NPLANES) {
						logtype = LOG_BOAT;
						if(iscruise > -1) {
							hitmsg = SLG_BOAT_CRUISE_HIT;
							destroymsg = SLG_BOAT_DESTROYED_CRUISE;
						}
						else {
							hitmsg = SLG_BOAT_BOMB_HIT;
							destroymsg = SLG_BOAT_DESTROYED_BOMB;
						}
					}
					else {
						logtype = LOG_AIR;
						if(iscruise > -1) {
							hitmsg = SLG_AIR_CRUISE_HIT;
							destroymsg = SLG_AIR_DESTROYED_CRUISE;
						}
						else {
							hitmsg = SLG_AIR_BOMB_HIT;
							destroymsg = SLG_AIR_DESTROYED_BOMB;
						}
					}
					if((cnt > NPLANES) && (bv[cnt].verticlesep < 5)) {
						bv[cnt].verticlesep += 1;
						if(bv[cnt].varcnt > Gtime)
							bv[cnt].varcnt = bv[cnt].varcnt + 120;
						else
							bv[cnt].varcnt = Gtime + 120;
	    				StartASmoke (ps[cnt].xL, ps[cnt].alt, ps[cnt].yL, AIRXPLO, 10);
						tempplane = GetPlanePtr(ps[cnt].type);
			    		FTXT (tempplane->name);
						TXA (" Hit By ");
						if(iscruise > -1) {
    						FTXA (ptype->name);
							LogIt (SLG_BOAT_CRUISE_HIT, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, LOG_BOAT);
						}
						else {
							TXA("BOMB");
							LogIt (SLG_BOAT_BOMB_HIT, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, LOG_BOAT);
						}
						Message(Txt,GAME_MSG);
					}
					else {
						doKill = 0;
						if((bv[cnt].leader == -1) || (Rtime & 0x1) || (cnt == whichourcarr)) {
							crashstat = ps[cnt].status & CRASH;
							DamageAI(cnt);
							if(crashstat != (ps[cnt].status & CRASH))
								doKill = 1;
						}
						else {
	        				AirKill (cnt);
							doKill = 1;
						}
				    	TXA ("Hit by ");
						if(iscruise > -1) {
   							FTXA (ptype->name);
							if(doKill)
								LogIt (destroymsg, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, logtype);
							else
								LogIt (hitmsg, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, logtype);
						}
						else {
							TXA("BOMB");
							if(doKill)
								LogIt (destroymsg, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, logtype);
							else
								LogIt (hitmsg, cnt, lxval, lyval, zval, camefrom, missiletype, 0L, logtype);
						}
   						Message (Txt,WINGMAN_MSG);
					}
				}
			}
		}
	}
}

void EngageTrain()
{
	droll = 0x2000 - ps[planecnt].roll;
	if(ps[planecnt].alt > 2500)
		pdest = -0x800;
	else
		pdest = 0;

}

void JustPutHere(int planenum, long xval, long yval, UWORD altval, int prewing)
{
	ps[planenum].xL = xval / AILOCDIV;
	ps[planenum].yL = yval / AILOCDIV;
	ailoc[planenum].xloc = ((long)ps[planenum].xL * AILOCDIV) + (AILOCDIV>>1);
	ailoc[planenum].yloc = ((long)ps[planenum].yL * AILOCDIV) + (AILOCDIV>>1);
    ps[planenum].x = (ps[planenum].xL+16) >> HTOV;
    ps[planenum].y = (ps[planenum].yL+16) >> HTOV;
	ps[planenum].alt = altval;
	if(prewing == -1) {
		ps[planenum].roll = OurRoll;
		ps[planenum].pitch = OurPitch;
		ps[planenum].head = OurHead;
		FormationSpeedAdjustUS(Knots, 0, 1, prewing);
	}
	else {
		ps[planenum].roll = ps[prewing].roll;
		ps[planenum].pitch = ps[prewing].pitch;
		ps[planenum].head = ps[prewing].head;
//		bv[planenum].basespeed = bv[prewing].basespeed;
//		ps[planenum].speed = ps[prewing].speed;
		FormationSpeedAdjustUS(bv[prewing].basespeed, 0, 1, prewing);
	}
	droll = 0;
	pdest = ps[planenum].pitch;
}

void SetUpLaunch(int planenum, int waypt)
{
	int catval;
	int numondeck[]   = {  10,   9,   9};
	int numinfront[]  = {   8,   7,   5};
	int xplace[]      = {  25,  25,  25};
	int yplacefront[] = { -84, -65, -77};
	int yplacerear[]  = { -47, -26, -41};
	int whichcarr;


	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	ps[planenum].status |= TAKEOFF;
	bv[planenum].basespeed = 0;
	ps[planenum].speed = 0;
	ps[planenum].alt = 1;
	if(bv[planenum].behaveflag & ISHELICOPTER)
		return;
	if((aiwaypoints[waypt].x <= (long)-1) && (ps[planenum].status & FRIENDLY)) {
#if 0
		TXT("LAST CAT USED ");
		TXN(lastcatused);
		Message(Txt,DEBUG_MSG);
#endif

		if(bv[planenum].Behavior != Ready5) {
			catval = RequestCat(-1, planenum);
			if(catval <= 1)
				landingstatus |= (LANDCAT0USED<<catval);
		}
		else {
			catval = RequestCat(3, planenum);
		}

		if(catval != -1) {
			PreparePlane(catval, planenum);
			bv[planenum].threathead = catval;
			if(bv[planenum].leader == -1)
				bv[planenum].varcnt = Gtime + 30 + (catval * 5);
			else
				bv[planenum].varcnt = Gtime + 10 + (catval * 5);
#ifdef SRECATTEST
			TXT("PLANE ");
			TXN(planenum);
			TXA(" CAT ");
			TXN(catval);
			Message(Txt);
#endif
		}
		else {
			bv[planenum].threathead = -5;
			if((bv[planenum].Behavior == Ready5) && (lastcatused == 4))
				lastcatused += 1;
			if(lastcatused < numondeck[whichcarr] ) {
				ailoc[planenum].xloc = (long)xplace[whichcarr] * AILOCDIV;
				if(lastcatused < numinfront[whichcarr])
					ailoc[planenum].yloc = ((long)yplacefront[whichcarr] + (lastcatused * 14)) * AILOCDIV;
				else
					ailoc[planenum].yloc = ((long)yplacerear[whichcarr] + (lastcatused * 14)) * AILOCDIV;
#ifdef SRECATTEST
				TXT("ON DECK ");
				TXN(planenum);
				TXA(" slot ");
				TXN(lastcatused);
				Message(Txt);
#endif
			}
			else {
//				ailoc[planenum].xloc = (long)-28 * AILOCDIV;
//				ailoc[planenum].yloc = (long)78 * AILOCDIV;
//				ailoc[planenum].xloc = (long)25 * AILOCDIV;
//				ailoc[planenum].yloc = (long)7 * AILOCDIV;
				ailoc[planenum].xloc = (long)-28 * AILOCDIV;
				ailoc[planenum].yloc = (long)-32 * AILOCDIV;

				bv[planenum].behaveflag |= AI_INVISIBLE;
#ifdef SRECATTEST
				TXT("INVISIBLE!!!!!!!! ");
				TXN(planenum);
				Message(Txt);
#endif
			}

			lastcatused ++;
		}
	}
#if 0
	else {
		bv[planenum].varcnt = Gtime + 5 + (planenum * 5);
	}
#endif
}

void GetNextLaunch(int planenum) {
	int cnt;
	int catval;
	int basenum;
	int elevX[3][4] = {{25, 25, 25, -27},
	                       {25, 25, 25, -27},
	                       {25, 25, 25, -27}};
	int elevY[3][4] = {{-28, 7, 76, 77},
	                   {-17, 21, 82, 89},
	                   {-13, 35, 69, -33}};
	int elevnum;
	int cnt2;
	int whichcarr;


	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	if(planenum == -1) {
		catval = PlayerCat;
		basenum = (whichourcarr + 1) * -1;
	}
	else {
		catval = bv[planenum].threathead;
		basenum = ps[planenum].base;
		if(bv[planenum].groundt != -1) {
			if(aiwaypoints[bv[planenum].groundt].x >= (long)0) {
				return;
			}
		}
		else if (bv[planenum].Behavior != SARCAP1)
			return;

	}
	if((catval < 0) || (catval > 3))
		return;

	if((landingstatus & LANDREQUESTED) && (catval < 2))
		return;

	for(cnt2 = 0; cnt2 < 2; cnt2 ++) {
		for(cnt = 0; cnt < NPLANES; cnt ++) {
        	if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
				if((ps[cnt].status & TAKEOFF) && (ps[cnt].base == basenum) && (bv[cnt].threathead == -5) && ((bv[cnt].Behavior != Ready5) || ((cnt2 == 1) && (catval > 1)))) {
#ifdef SRECATTEST
					TXT("ACTIVATE ");
					TXN(cnt);
					TXA(" CAT ");
					TXN(catval);
					Message(Txt);
#endif
					bv[cnt].threathead = (catval + 1) * -1;
					if(bv[cnt].behaveflag & AI_INVISIBLE) {
						switch (whichcarr) {
							case 1:
								elevnum = GetWhichElevator(1, 2, 3, 0, 1);
								break;
							case 2:
								elevnum = GetWhichElevator(1, 2, 0, 0, 1);
								break;
							default:
								elevnum = GetWhichElevator(1, 2, 3, 0, 1);
								break;
						}
						ailoc[cnt].xloc = (long)elevX[whichcarr][elevnum] * 100L;
						ailoc[cnt].yloc = (long)elevY[whichcarr][elevnum] * 100L;
						if(elevnum < 3) {
							ps[cnt].head = carrier.heading - 0x4000;
						}
						else {
							ps[cnt].head = carrier.heading + 0x4000;
						}
					}
	//				bv[cnt].behaveflag &= ~AI_INVISIBLE;
					if(catval <= 1)
						landingstatus |= (LANDCAT0USED<<catval);
					return;
				}
			}
		}
	}
}

void DoAltHover()
{
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;
	int wayval = 0;

	if(bv[planecnt].varcnt <= Gtime) {
		bv[planecnt].varcnt = 0;
		wayval = bv[i].groundt;
		if(wayval < 0)
			return;
		if(aiwaypoints[wayval].x == (long)-2)
			wayval = bv[i].groundt + 2;
		else
			wayval = bv[i].groundt + 1;

		bv[planecnt].groundt = wayval;
		bv[planecnt].Behavior = FlyWayPoint;
		GetWayPointChanges(wayval, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
		if(speedval == -1)
			bv[planecnt].basespeed = 90;
		else
			bv[planecnt].basespeed = speedval;
		DoNextWayPt(planecnt, bv[planecnt].groundt);
		return;
	}
	DoHover();
	wayval = bv[planecnt].groundt;

	if(wayval < 0)
		return;

	GetWayPointChanges(wayval, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
	if(altval == -1)
		altval = 1;

	if(ps[planecnt].alt > altval) {
		if(ps[planecnt].alt > (altval + 10)) {
			pdest = -0x1000;
		}
		else {
			pdest = -0x200;
		}
	}
	else if(ps[planecnt].alt < altval) {
		if(ps[planecnt].alt < (altval - 10)) {
			pdest = 0x1000;
		}
		else {
			pdest = 0x200;
		}
	}
}

int GetRadarGTNum(int radarnum)
{
	int cnt;
	for(cnt = 0; cnt < NGTARGETS; cnt ++) {
		if(((int)(gt[cnt].x>>HTOV) == Rdrs[radarnum].x) && ((int)(gt[cnt].y>>HTOV) == Rdrs[radarnum].y))
			return(gt[cnt].orgnum);
	}
	return(-999);
}

int GetGTRadarNum(int gtnum)
{
	int cnt;
	for(cnt = 0; cnt < NRADAR; cnt ++) {
		if(((int)(gt[gtnum].x>>HTOV) == Rdrs[cnt].x) && ((int)(gt[gtnum].y>>HTOV) == Rdrs[cnt].y))
			return(cnt);
	}
	return(-999);
}

#define NUM_AI_WAYPTS 70  /*  was 30 40 */

void IndirectWPCorrection(int planenum)
{
	long newx, newy;
	int cnt;
	long orgnum;
	int otherplane;
	int done = 0;

	orgnum = (long)bv[planenum].orgnum;
	newx = -1L;

	otherplane = bv[planenum].wingman;
	if(otherplane != -999) {
		if((ps[otherplane].status & ACTIVE) && (ps[otherplane].status & ALIVE)) {
			newy = (long)bv[otherplane].orgnum;
			done = 1;
		}
	}

	otherplane = bv[planenum].nextsec;
	if((otherplane != -999) && (!done)) {
		if((ps[otherplane].status & ACTIVE) && (ps[otherplane].status & ALIVE)) {
			newy = (long)bv[otherplane].orgnum;
			done = 1;
		}
	}

	if(!done) {
		newx = ps[planenum].xL;
		newy = ps[planenum].yL;
	}

	for(cnt = 0; cnt < NUM_AI_WAYPTS; cnt ++) {
		if((aiwaypoints[cnt].x <= -1L) && (aiwaypoints[cnt].y == orgnum)) {
			if(newx == -1L) {
				aiwaypoints[cnt].y = newy;
				if(aiwaypoints[cnt].x == -2L)
					cnt += 1;
			}
			else {
				if((aiwaypoints[cnt].x == -2L) && (cnt < (NUM_AI_WAYPTS - 1))) {
					aiwaypoints[cnt].x = newx + aiwaypoints[cnt + 1].x;
					aiwaypoints[cnt].y = newy + aiwaypoints[cnt + 1].y;
					cnt += 1;
					aiwaypoints[cnt].x += newx;
					aiwaypoints[cnt].y += newy;
				}
				else {
					aiwaypoints[cnt].x = newx;
					aiwaypoints[cnt].y = newy;
				}
			}
		}
	}
}

void LetWingmenContinue(int planenum)
{
	int otherplane;

	otherplane = bv[planenum].wingman;
	if(otherplane >= 0) {
		if(bv[otherplane].Behavior == Formation) {
			bv[otherplane].groundt = bv[planenum].groundt;
			bv[otherplane].Behavior = FlyWayPoint;
			DoNextWayPt(otherplane, bv[otherplane].groundt);
		}
	}
	otherplane = bv[planenum].nextsec;
	if(otherplane >= 0) {
		if(bv[otherplane].Behavior == Formation) {
			bv[otherplane].groundt = bv[planenum].groundt;
			bv[otherplane].Behavior = FlyWayPoint;
			DoNextWayPt(otherplane, bv[otherplane].groundt);
		}
	}
}

void ShowOurWPS()
{
	int waynum;
	int wayobj;
	int cnt;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

//	waynum = aipaths[OurPath].wp0 + NextWayPoint - 1;

	waynum = aipaths[OurPath].wp0;

	if((aiwaypoints[waynum].x == -1L) && ((GetPlaneOrgNum((int)aiwaypoints[waynum].y) == whichourcarr))) {
		waynum += 1;
	}
	else if(aiwaypoints[waynum].x > -1L) {
		GetWayPointChanges(waynum, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
		if(behavenum != SA_CAP) {
			waynum += 1;
		}
	}

	for(cnt = 0; cnt < (NextWayPoint - 1); cnt ++) {
		if(aiwaypoints[waynum].x == -2L)
			waynum += 1;
		waynum += 1;
	}

	if(waynum >= (aipaths[OurPath].wp0 + aipaths[OurPath].numWPs)) {
		waynum = aipaths[OurPath].wp0 + aipaths[OurPath].numWPs - 1;
	}

	GetWayPointChanges(waynum, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);

    TXT ("Next Sequence Point: ");

	if(NextWayPoint == 0) {
		if(landingstatus & LANDPLAYERWAIT)
			TXA ("Landing Marshall Point ");
		else
			TXA ("Carrier ");
	}
	else if(behavenum == SA_CAP) {
		TXA("Center of CAP area");
	}
	else if(aiwaypoints[waynum].x >= 0L) {
		TXA("Way Point ");
		TXN(NextWayPoint);
	}
	else {
		TXN(NextWayPoint);
		if(aiwaypoints[waynum].x == -2L)
			TXA(" Near");
		wayobj = GetPlaneOrgNum((int)aiwaypoints[waynum].y);
		if(wayobj < NPLANES) {
			TXA(" Plane ");
		}
		else if(wayobj == MAXPLANE) {
			TXA(" Ground Target ");
		}
		else if(wayobj == -1) {
			TXA(" US ");
		}
		else if(wayobj == whichourcarr) {
			TXA(" Carrier ");
		}
		else {
			TXA(" BOAT ");
		}
	}

    Message (Txt, RIO_NOW_MSG);
}

int GetDAltSpeed(UWORD alt)
{
	long alteffect;

#if 1
	if(alt > 2500) {
		alteffect = (((((long)alt - 2500)/25) + 100) * ptype->speed)/1000;
		return((int)alteffect);
	}
	else if(alt > 1250) {
		alteffect = ((((long)alt - 1250)/25) * ptype->speed)/500;
		return((int)alteffect);
	}
#else
	alteffect = (((long)alt/25) * ptype->speed)/500;
	return((int)alteffect);
#endif

	return(0);
}

void CheckTurnStuff(int planenum)
{
	int planeG, tempg;
	int tdroll, cnt, altspd, tempspd;
	int i, maxspeed;
	int wingkludge = 0;
	long spd;
	int MinSpdG[] = {150, 150, 225, 250, 275, 300, 325, 350, 5000, 5000, 5000};
	int GAngle[] = {0, 0, 0x2AA8, 0x3224, 0x35B1, 0x37C8, 0x3929, 0x3A25, 0x3AE1, 0x3B72, 0x3BE7};


	i = planenum;

	if(ps[planenum].status & (CRASH|LANDING|LANDED))
		return;

	if(planenum != GetSquadLeader(planenum))
		wingkludge = -20;

	tempg = abs(cosX(ps[planenum].roll,100));
	if(tempg != 0) {
		planeG = 100 / tempg;
		if(planeG > 9)
			planeG = 9;
	}
	else
		planeG = 9;

	maxspeed = ptype->speed;

	if(bv[planenum].behaveflag & DAMAGEENGINE)
		maxspeed = maxspeed>>1;

	if((!(bv[planenum].behaveflag & ENEMYNOFIRE)) || (ps[planenum].status & BACKTOBASE)) {
		if((maxspeed > 500) && (bv[planenum].fueltank <= 14000)) {
			maxspeed = maxspeed - 200;
			if(maxspeed < 450)
				maxspeed = 450;
		}
	}

	if(planeG < 2) {
		if(ps[planenum].speed < (MinSpdG[planeG] + wingkludge)) {
			if(maxspeed > bv[planenum].basespeed) {
				altspd = GetDAltSpeed(ps[planenum].alt);
				tempspd = MinSpdG[planeG] + altspd + wingkludge;
				if(ps[planenum].pitch > 0) {
		        	spd = (long)((long)tempspd*(0x8000-(ps[planenum].pitch/2))) >>15;
        			tempspd += tempspd - (int)spd;
				}
				tempspd += (abs( sinX(ps[planenum].roll,tempspd)) >>1);
//				tempspd += (abs( sinX(ps[planenum].roll,tempspd)) >>2);
				if(tempspd > bv[planenum].basespeed)
					bv[planenum].basespeed = tempspd;
				if(maxspeed >= bv[planenum].basespeed)
					return;
			}
			bv[planenum].basespeed = ptype->speed;
			if(pdest > -0x1000)
				pdest = -0x1000;
		}
		else {
			if(bv[planenum].basespeed > maxspeed)
				bv[planenum].basespeed = ptype->speed - wingkludge;
		}
		return;
	}

	if(ps[planenum].speed < MinSpdG[planeG]) {
		if((maxspeed > bv[planenum].basespeed) && (ps[planenum].alt < 1250)) {
			altspd = GetDAltSpeed(ps[planenum].alt);
			tempspd = MinSpdG[planeG] + altspd;
			if(ps[planenum].pitch > 0) {
		        spd = (long)((long)tempspd*(0x8000-(ps[planenum].pitch/2))) >>15;
        		tempspd += tempspd - (int)spd;
			}
			tempspd += (abs( sinX(ps[planenum].roll,tempspd)) >>1);
//			tempspd += (abs( sinX(ps[planenum].roll,tempspd)) >>2);
			if(tempspd > bv[planenum].basespeed)
				bv[planenum].basespeed = tempspd;
			if(maxspeed < bv[planenum].basespeed) {
				bv[planenum].basespeed = ptype->speed;
				if(pdest > -0x1000)
					pdest = -0x1000;
			}
			else {
				return;
			}
		}
		else if(pdest > -0x1000)
			pdest = -0x1000;

		tdroll = 0;
		for(cnt = 0; cnt < 10; cnt ++) {
			if(ps[planenum].speed >= MinSpdG[planeG])
				tdroll = cnt;
		}
		tdroll = GAngle[tdroll];
		if(ps[planenum].roll < 0)
			tdroll = -tdroll;
		droll = tdroll - ps[planenum].roll;
	}

}

#define DBEHINDCARR 14400
void AICarrierLand()
{
	int V[3];
	int M[9];
	int TM[9];
	int   H;
	int landpnt;
	int i;
	long dx, dy;
	long dx2, dy2;
	long dx3, dy3;
	int dz;
	int rearhead;
	int ptdist;
	long ptdistl;
	int tempalt;
	int ptangle;
	int landangle;
	int droll1;
	int otherplane;
	int whichcarr;
	struct landpt *usedlpattern;
	struct landpt *usedlpatstart;

	if(!(Night || (MissionWeather == 2)))
		usedlpattern = usedlpatstart = LandPat;
	else {
		usedlpattern = usedlpatstart = NiteLandPat;
	}

	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	i = planecnt;

	landpnt = bv[planecnt].threathead;
	if(landpnt > (MAXPAT-1)) {
		if(GetSquadLeader(planecnt) == -1) {
			if(landingstatus & (LANDPLAYERWAIT|LANDPLAYERNOW)) {
				if(whichourcarr >= 0) {
	    			if ((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status&ACTIVE)) {
						if((abs(ps[whichourcarr].x - Px) >= 1800) && (abs(ps[whichourcarr].y - Py) >= 1800)) {
							ps[planecnt].status &= ~(BACKTOBASE|LANDING);
							bv[planecnt].Behavior = Formation;
						}
					}
				}
			}
		}
		rearhead = carrier.heading + 0x8000;
		dx = carrier.x + sinXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
		dy = carrier.z - cosXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
		if(landpnt > 100) {
			bv[planecnt].basespeed = 250;
			DoLandWait(dx, dy, rearhead);
			return;
		}
		if(bv[planecnt].basespeed < 400)
			bv[planecnt].basespeed = 400;
		dx2 = ailoc[planecnt].xloc - dx;
		dy2 = ailoc[planecnt].yloc - dy;
		ptdist = (int)((long)(LDist2D(dx2, dy2)/100L)>>HTOV);
		if(ptdist < 20) {
			bv[planecnt].threathead = (MAXPAT-1);
  			otherplane = bv[planecnt].wingman;
			if(otherplane >= 0) {
		        if ((bv[otherplane].Behavior != AICarrierLand) && (ps[otherplane].status & ALIVE) && (ps[otherplane].status&ACTIVE) && (!(ps[otherplane].status & CRASH))) {
					bv[otherplane].Behavior = AIGoHome;
					ps[otherplane].status |= BACKTOBASE;
					ps[otherplane].status &= ~(LANDED|LANDING);
				}
			}
			otherplane = bv[planecnt].nextsec;
			if(otherplane >= 0) {
        		if ((bv[otherplane].Behavior != AICarrierLand) && (ps[otherplane].status & ALIVE) && (ps[otherplane].status&ACTIVE) && (!(ps[otherplane].status & CRASH))) {
					bv[otherplane].Behavior = AIGoHome;
					ps[otherplane].status |= BACKTOBASE;
					ps[otherplane].status &= ~(LANDED|LANDING);
				}
			}
		}
		tempalt = 1250;
		LFlyTo(dx, dy, tempalt, -1, 0);
		return;
	}
	else {
		if(landpnt < 0) {
			V[0]=(carrier.wires[2].cx/100L);
			V[1]=(int)(carrier.deckht/100L) + 2;
			V[2]=(carrier.wires[2].cz/100L);
//			bv[planecnt].basespeed = LandPat[0].KNTS;
			bv[planecnt].basespeed = usedlpatstart->KNTS;
		}
		else {
			if((Night || (MissionWeather == 2)) && (landpnt > (MAXNITEPAT - 1)))
				landpnt = bv[planecnt].threathead = (MAXNITEPAT - 1);
			usedlpattern = usedlpatstart + landpnt;
//			V[0]=LandPat[landpnt].X+(carrier.wires[2].cx/100L);
//			V[1]=LandPat[landpnt].HT;
//			V[2]=LandPat[landpnt].Z+(carrier.wires[2].cz/100L);
//			bv[planecnt].basespeed = LandPat[landpnt].KNTS;
			V[0]=usedlpattern->X+(carrier.wires[2].cx/100L);
			V[1]=usedlpattern->HT;
			V[2]=usedlpattern->Z+(carrier.wires[2].cz/100L);
			bv[planecnt].basespeed = usedlpattern->KNTS;
		}
	}
	H= carrier.heading;

	MakeMatHPR(H,0,0,M);
	Rotate(V,M);
	dx =carrier.x + (V[0] * 100L);
	dy =(carrier.z + (V[2] * 100L));
	tempalt = V[1]>>2;
	dx2 =  dx - ailoc[planecnt].xloc;
	dy2 =  dy - ailoc[planecnt].yloc;
	ptdistl = LDist2D(dx2, dy2);
	ptangle = Langle(dx2, -dy2);
	ptangle = ptangle - ps[planecnt].head;
	if(abs(ptangle) < 0x1000)
		ptangle = (ptangle << 2);
	else if(ptangle < 0)
		ptangle = -0x4000;
	else
		ptangle = 0x4000;

	if(landpnt < ((MAXPAT-1)>>1))  /*  was (landpnt < 5) when MAXPAT == 8  */
		landingstatus |= LANDNEAREND;

	if(landpnt <= -1) {
		if(landpnt == -2) {
//			V[0]=-LandPat[0].X+(carrier.wires[2].cx/100L);
/*			V[0]=-13L;
			V[1]=LandPat[0].HT;
			V[2]=-LandPat[0].Z+(carrier.wires[2].cz/100L);  */
			V[0]=-13L;
			V[1]=usedlpatstart->HT;
			V[2]=-usedlpatstart->Z+(carrier.wires[2].cz/100L);
			MakeMatHPR(H,0,0,M);
			Rotate(V,M);
			dx3 =carrier.x + (V[0] * 100L);
			dy3 =(carrier.z + (V[2] * 100L));
			landangle = Langle(dx3 - ailoc[planecnt].xloc, -(dy3  - ailoc[planecnt].yloc));
			ps[planecnt].head = landangle;
			bv[planecnt].basespeed = 0;
			pdest = 0;
			droll = 0;
			ps[planecnt].alt = tempalt;
			if(ps[planecnt].speed == 0) {
				ailoc[planecnt].xloc = ailoc[planecnt].xloc - carrier.x;
				ailoc[planecnt].yloc = ailoc[planecnt].yloc - carrier.z;
				ps[planecnt].status |= JUSTPUT;
//				bv[planecnt].threathead = -6;
				switch (whichcarr) {
					case 1:
						bv[planecnt].threathead = -6 - GetWhichElevator(1, 0, 2, 3, 1);
						break;
					case 2:
						bv[planecnt].threathead = -6 - GetWhichElevator(1, 0, 3, 2, 1);
						break;
					default:
						bv[planecnt].threathead = -6 - GetWhichElevator(1, 0, 2, 3, 1);
						break;
				}
				bv[planecnt].varcnt = Gtime + 5;
				LogIt (SLG_AIR_LAND, planecnt, ps[planecnt].xL, ps[planecnt].yL, ps[planecnt].alt, -999, -999, 0L, LOG_AIR);
				LetNextPlaneLand();
				GetPlaneLanding();
			}
			return;
		}
		else if(landpnt <= -3) {
			MoveAIToElevator(planecnt, bv[planecnt].threathead);
			ps[planecnt].status |= JUSTPUT;
			return;
		}
		if((ptdistl < 1000L) || ((ptdistl < 8000L) && (abs(ptangle) > 0x3000))) {
			bv[planecnt].threathead -= 1;
			bv[planecnt].basespeed = 0;
			ps[planecnt].alt = tempalt;
			pdest = 0;
			droll = 0;
			return;
		}
		droll = ptangle;
	}
	else {
		if((ptdistl < 12000L) || ((ptdistl < 25000L) && (abs(ptangle) > 0x3000))) {
			bv[planecnt].threathead -= 1;
			if((bv[planecnt].threathead == (MAXPAT-2)) || (bv[planecnt].threathead == ((MAXPAT-1)>>1))) {
				if(planecnt == planelanding)
					LetNextPlaneLand();
			}
//			if(bv[planecnt].threathead == 5)
//				bv[planecnt].threathead -= 1;
		}
		droll = ptangle - ps[planecnt].roll;
	}

	droll1 = droll;

	if(abs(droll) < 0x2000)
		droll = (droll << 2);

	pdest = Langle ((long)(tempalt-ps[planecnt].alt), ptdistl/100L);
	if((landpnt == 0) || (landpnt == -1)) {
		if((pdest > -0x222) && (pdest < 0)) {
			pdest = 0;
		}
	}
}

void MoveAIToElevator(int planenum, int elevator)
{
	int xval, yval;
	int tx, ty;
	int elevX[3][4] = {{25, 25, 25, -27},
	                       {25, 25, 25, -27},
	                       {25, 25, 25, -27}};
	int elevY[3][4] = {{-28, 7, 76, 77},
	                   {-17, 21, 82, 89},
	                   {-13, 35, 69, -33}};
	int whichcarr;
	int elevindx;


	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	elevindx = abs(elevator) - 6;
	if((elevindx < 0) || (elevindx > 3))
		elevindx = 1;

	if(bv[planecnt].varcnt > Gtime)
		return;

	xval = elevX[whichcarr][elevindx];
	yval = elevY[whichcarr][elevindx];

	tx = (int)ailoc[planenum].xloc/100;
	ty = (int)ailoc[planenum].yloc/100;

   	if(ty != yval) {
		if(ty < yval) {
			if(TurnTo(planenum, GetCatHeading(2) - 0x8000)) {
					ailoc[planenum].yloc += 100;
			}
		}
		else {
			if(TurnTo(planenum, GetCatHeading(2))) {
					ailoc[planenum].yloc -= 100;
			}
		}
	}
	else if(tx != xval) {
		if(tx < xval) {
			if(TurnTo(planenum, GetCatHeading(2) - 0x4000)) {
					ailoc[planenum].xloc += 100;
			}
		}
		else {
			if(TurnTo(planenum, GetCatHeading(2) + 0x4000)) {
					ailoc[planenum].xloc -= 100;
			}
		}
		if((ailoc[planenum].xloc/100L) == (long)xval)
			bv[planenum].varcnt = Gtime + 3;
	}
	else {
		if(bv[planenum].behaveflag & AI_INVISIBLE) {
			IndirectWPCorrection(planenum);
			ps[planenum].status = 0;
			landingstatus &= ~LANDNEAREND;
			GetPlaneLanding();
			if(!(landingstatus & (LANDNEAREND|LANDPLAYERNOW)))
				landingstatus &= ~LANDNOTAKEOFF;
			bv[planenum].behaveflag &= ~AI_INVISIBLE;
		}
		else {
			bv[planenum].behaveflag |= AI_INVISIBLE;
			bv[planenum].varcnt = Gtime + 1 + (Rtime & 3);
		}
	}
}

#define LANDL 225  /*  half LAND length of 5 nm  */
#define LANDW 90   /*  half CAP width of 2 nm  */
#define LANDD 338
void DoLandWait(long dx, long dy, int rearhead)
{
	int whichwait;
	int i;
	int whichside;
	long dx2, dy2;
	int newangle;
	int returnval;

	i = planecnt;

	whichwait = bv[planecnt].threathead;
	if(whichwait >= 700) {
		whichside = whichwait - 700;
		whichwait = 700;
		newangle = bv[planecnt].threathead = rearhead + 0x2000;
	}
	else if(whichwait >= 600) {
		whichside = whichwait - 600;
		whichwait = 600;
		newangle = bv[planecnt].threathead = rearhead;
	}
	else {
		whichside = whichwait - 500;
		whichwait = 500;
		newangle = bv[planecnt].threathead = rearhead - 0x2000;
	}
	dx2 = dx + sinXL(newangle, ((long)DBEHINDCARR * AILOCDIV));
	dy2 = dy - cosXL(newangle, ((long)DBEHINDCARR * AILOCDIV));
	bv[i].xloc = dx2/100L;
	bv[i].yloc = dy2/100L;
	switch (whichside) {
		case 0:
			returnval = CAPAI(0, 1250 + bv[planecnt].verticlesep, LANDW, LANDL, LANDD);
			break;
		case 1:
			returnval = OvalTurn1(1250 + bv[planecnt].verticlesep, LANDW, LANDL);
			break;
		case 2:
			returnval = CAPAI(1, 1250 + bv[planecnt].verticlesep, LANDW, LANDL, LANDD);
			break;
		case 3:
			returnval = OvalTurn2(1250 + bv[planecnt].verticlesep, LANDW, LANDL);
			break;
		default :
			returnval = CAPAI(0, 1250 + bv[planecnt].verticlesep, LANDW, LANDL, LANDD);
			break;
	}
	if(returnval) {
		whichside += 1;
		if(whichside > 3)
			whichside = 0;
	}
	bv[planecnt].threathead = whichwait + whichside;
}

void CheckPlaneLand(int planenum)
{
	int cnt;
	int altplus = 0;
	int startval = 100;
	int rearhead;
	long dx, dy;
	int angels;

	landingstatus |= LANDREQUESTED;
	if(landingstatus & (LANDCAT0USED | LANDCAT1USED))
		startval = 500;

	if(landingstatus & (LANDPLAYERWAIT|LANDPLAYERNOW)) {
		if(planenum == -1)
			return;
		startval = 500;
	}
	if((Night || (MissionWeather == 2)) && (startval == 500)) {
		startval = 600;
	}
	for(cnt = 0; cnt < NPLANES; cnt ++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & CRASH))) {
			if((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead > (MAXPAT-1))) {
				if(startval == 100)
					startval = 400;
				if(!(Night || (MissionWeather == 2))) {
					startval += 100;
					if(startval == 800) {
						altplus += 250;
						startval = 500;
					}
				}
				else {
					startval = 600;
					altplus += 250;
				}
			}
		}
	}
	if(planenum >= 0) {
		bv[planenum].threathead = startval;
		bv[planenum].verticlesep = altplus;
#ifdef AILANDDEBUG
		if(startval == 100) {
			TXT("I can Land ");
			TXN(planenum);
			TXA(" ");
			TXN(Gtime);
			Message(Txt, DEBUG_MSG);
		}
#endif
	}
	else {
		planelanding = -1;
		TXT("(CARRIER) ");
		TXA(OurCallSign);
		if(startval == 100) {
			TXA(" Cleared To Land");
			landingstatus |= LANDPLAYERNOW;
			Speech(L_CLRD_LAND);
		}
		else {
			Speech(NEGATIVE);
			landingstatus |= LANDPLAYERWAIT;
			angels = (1250 + altplus) / 250;
			rearhead = carrier.heading + 0x8000;
			dx = carrier.x + sinXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
			dy = carrier.z - cosXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
			TXA(" Go To Marshall Point ");
			if(startval == 700) {
				rearhead = bv[planecnt].threathead = rearhead + 0x2000;
				TXA("CHARLIE, Angels ");
			}
			else if(startval == 600) {
				rearhead = bv[planecnt].threathead = rearhead;
				if(!(Night || (MissionWeather == 2)))
					TXA("BRAVO, Angels ");
				else
					TXA(", Angels ");
			}
			else {
				rearhead = bv[planecnt].threathead = rearhead - 0x2000;
				TXA("ALPHA, Angels ");
			}
			TXN(angels);
			Message(Txt, GAME_MSG);
			TXT("(RIO) Waypoint Updated");
			dx = dx + sinXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
			dy = dy - cosXL(rearhead, ((long)DBEHINDCARR * AILOCDIV));
			NextWayPoint = 0;
			wps[NextWayPoint].xL = dx/(long)AILOCDIV;
			wps[NextWayPoint].yL = dy/(long)AILOCDIV;
			wps[NextWayPoint].x = wps[NextWayPoint].xL>>HTOV;
			wps[NextWayPoint].y = wps[NextWayPoint].yL>>HTOV;
		}
		Message(Txt, GAME_MSG);
	}
#ifdef AILANDDEBUG
	TXT("Plane ");
	TXN(planenum);
	TXA(" sv ");
	TXN(startval);
	TXA(" ap ");
	TXN(altplus);
	if(!SHOWFIRST) {
		TXA(" F1 ");
		TXN(firstone);
	}
	Message(Txt, DEBUG_MSG);
#endif
}

void LetNextPlaneLand()
{
	int cnt;
	int altplus = 0;
	int startval = 100;
	int gotone = 0;
	int letland = -1;

	if(landingstatus & (LANDCAT0USED | LANDCAT1USED)) {
		return;
	}
	if(landingstatus & LANDPLAYERNOW) {
		return;
	}

	if(landingstatus & LANDPLAYERWAIT) {
		letland = MAXPLANE;
	}

	for(cnt = 0; cnt < NPLANES; cnt ++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & CRASH))) {
			if((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead > 100)) {
				if(!gotone) {
					letland = cnt;
					gotone = 1;
				}
			}
			if((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead == 100)) {
				bv[cnt].basespeed = 700;
#ifdef AILANDDEBUG
				TXT("Plane At 100 ");
				TXN(cnt);
				Message(Txt, DEBUG_MSG);
#endif
				return;
			}
		}
	}
	if(letland >= 0) {
		if(landingstatus & LANDPLAYERWAIT) {
			TXT("(CARRIER) ");
			TXA(OurCallSign);
			TXA(" Cleared To Land");
			Message(Txt, GAME_MSG);
			Speech(L_CLRD_LAND);
			landingstatus &= ~LANDPLAYERWAIT;
			landingstatus |= LANDPLAYERNOW;
			planelanding = -1;
			return;
		}
		altplus = bv[letland].verticlesep;
		startval = bv[letland].threathead % 100;
		bv[letland].threathead = 100;
		bv[letland].verticlesep = 0;
#ifdef AILANDDEBUG
		TXT("LET LAND ");
		TXN(letland);
		TXA(" at ");
		TXN(Gtime);
		Message(Txt, DEBUG_MSG);
#endif
	}
	for(cnt = 0; cnt < NPLANES; cnt ++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE)) {
			if((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead > 100)) {
				if((bv[cnt].threathead >= startval) && (bv[cnt].threathead < (startval + 100))) {
					if(bv[cnt].verticlesep > altplus)
						bv[cnt].verticlesep -= 250;
				}
			}
		}
	}
}

void GetPlaneLanding()
{
	int cnt;
	int landval = 1000;

	if(landingstatus & LANDPLAYERNOW)
		planelanding = -1;

	if(planelanding == -1)
		return;

	planelanding = -999;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status & CRASH))) {
			if((bv[cnt].Behavior == AICarrierLand) && (bv[cnt].threathead < landval)) {
				if(bv[cnt].threathead > -2) {
					planelanding = cnt;
					landval = bv[cnt].threathead;
					if(bv[cnt].threathead < ((MAXPAT-1)>>1))
						landingstatus |= LANDNEAREND;
				}
			}
		}
	}
}

int GetWhichElevator(int try0, int try1, int try2, int try3, int yesinvis)
{
	int cnt, cnt2;
	int elevorder[4];
//	int elevX[] = {25, 25, 25, -27};
//	int elevY[] = {-29, 7, 76, 78};
	int elevX[3][4] = {{25, 25, 25, -27},
	                       {25, 25, 25, -27},
	                       {25, 25, 25, -27}};
	int elevY[3][4] = {{-28, 7, 76, 77},
	                   {-17, 21, 82, 89},
	                   {-13, 35, 69, -33}};
	int elevnum;
	int elevOK;
	int whichcarr;


	whichcarr = ConfigData.Carrier;
	if((whichcarr > 2) || (whichcarr < 0))
		whichcarr = 0;

	elevorder[0] = try0;
	elevorder[1] = try1;
	elevorder[2] = try2;
	elevorder[3] = try3;
	for(cnt = 0; cnt < 4; cnt ++ ) {
		elevnum = elevorder[cnt];
		elevOK = 1;
		for(cnt2 = 0; cnt2 < NPLANES; cnt2 ++) {
			if((ps[cnt2].status & ALIVE) && (ps[cnt2].status & ACTIVE) && (ps[cnt2].base == ((whichourcarr + 1) * -1)) && ((!(bv[cnt2].behaveflag & AI_INVISIBLE)) || yesinvis)) {
				if(((bv[cnt2].Behavior == Ready5) || (bv[cnt2].Behavior == SARWait)
						|| ((bv[cnt2].Behavior == AICarrierLand) && (bv[cnt2].threathead <= -3)) || ((ps[cnt2].status & TAKEOFF) && (ps[cnt2].speed <= 1)))) {
					if((labs((ailoc[cnt2].xloc/100L) - (long)elevX[whichcarr][elevnum]) < 12) && (labs((ailoc[cnt2].yloc/100L) - (long)elevY[whichcarr][elevnum]) < 12)) {
						elevOK = 0;
					}
				}
			}
		}
		if(elevOK) {
			return(elevnum);
		}
	}
	return(elevnum);
}

int CheckIsWayTarget(int bogeyi)
{
	int bogey_org_num, waynum, cnt;
	int is_target = 0;

	bogey_org_num = GetSquadLeader(bogeyi);

	if(bogey_org_num == -1)
		bogey_org_num = (int)orgPlayernum;
	else
		bogey_org_num = (int)bv[bogey_org_num].orgnum;

	for(cnt = 0; cnt < aipaths[OurPath].numWPs; cnt ++) {
		waynum = aipaths[OurPath].wp0 + cnt;
		if(aiwaypoints[waynum].x <= -1L) {
			if(aiwaypoints[waynum].y == (long)bogey_org_num) {
				is_target = 1;
			}
		}
		if(aiwaypoints[waynum].x == -2L)
			cnt += 1;
	}
	return(is_target);
}

void DoGroupAction(int planenum, int actionflag, int waypt)
{
	int wingcnt;
	int tprewing;

	if(bv[planenum].Behavior == FlyWayPoint)
		return;

	tprewing = planenum;
	wingcnt = bv[planenum].wingman;
	while (tprewing >= 0) {
		while (wingcnt >= 0) {
			if(bv[wingcnt].Behavior == bv[planenum].Behavior) {
				if(actionflag == DOWPTROOPDROP) {
					if(Gtime != 0)
						TrooperDrop(wingcnt);
				}
				if(actionflag == DOWPGOAL) {
					LogIt (SLG_WAY_PT_GOAL, wingcnt, ps[wingcnt].xL, ps[wingcnt].yL, ps[wingcnt].alt, -(int)bv[planenum].path, waypt, 0L, LOG_AIR);
				}
			}
			wingcnt = bv[wingcnt].wingman;
		}
		tprewing = bv[tprewing].nextsec;
		wingcnt = tprewing;
	}
}

void DoOClockCall()
{
	int cnt;
	int targUS = 0;
	long ldist, tldist;
	long dx, dy;
	UWORD aoclock;
	int noclock;
	int tpitch;

	closeplane = -1;
	ldist = -1;
	for(cnt = 0; cnt < NPLANES; cnt ++) {
		if((ps[cnt].status & ALIVE) && (ps[cnt].status & ACTIVE) && (!(ps[cnt].status & (FRIENDLY|CRASH|CIVILIAN)))) {
			if((abs(ps[cnt].x - Px) < 450) && (abs(ps[cnt].y - Py) < 450)) {
				if((GetSquadLeader(bv[cnt].target) == -1) || (targUS == 0)) {
					tldist = LDist2D((ps[cnt].xL - PxLng), (ps[cnt].yL - (0x100000-PyLng)));
					if((tldist < ldist) || (ldist < 0) || ((GetSquadLeader(bv[cnt].target) == -1) && (targUS == 0))) {
						ldist = tldist;
						closeplane = cnt;
					}
				}
			}
		}
	}
	if(closeplane == -1) {
		lastOClock = Gtime + 45;
		return;
	}
	else if ((int)(ldist << HTOV) > 90) {
		lastOClock = Gtime + 20;
	}
	else {
		lastOClock = Gtime + 5;
	}
	if(closeplane == LOCKED) {
		return;
	}
	aoclock = Langle((ps[closeplane].xL - PxLng), -(ps[closeplane].yL - (0x100000-PyLng)));
	aoclock = aoclock - OurHead;
	aoclock = aoclock + 2730;
	noclock = aoclock / 5461;
	tpitch = GetAltPitch(Alt, ps[closeplane].alt, (int)(ldist>>HTOV));
	TXT("(RIO) BANDIT ");
	if(noclock == 0)
		TXA("12");
	else
		TXN(noclock);
	TXA(" O'clock ");
	if(tpitch > 0x1200){
		TXA("HIGH");
	}
	else if(tpitch < -0x1200){
		TXA("LOW");
	}
	Message(Txt, RIO_MSG);
}

void Immelmann()
{
	int i;

	i = planecnt;
	bv[planecnt].basespeed = ptype->speed;

	if(abs(ps[planecnt].roll) > 0x4000) {
		bv[i].Behavior = StraightAttack;
		StraightAttack();
/*		sprintf(Txt, "Immelmann Off p %x, r %x", ps[i].pitch, ps[i].roll);
		Message(Txt, DEBUG_MSG);  */
		return;
	}
	if(ps[i].speed < 150) {
		if(ps[i].alt > 1000) {
//			Message("Switch to SplitS", DEBUG_MSG);
			bv[i].Behavior = SplitS;
			SplitS();
			return;
		}
		else {
//			Message("Switch to Straight Attack", DEBUG_MSG);
			bv[i].Behavior = StraightAttack;
			StraightAttack();
			return;
		}
	}
	if(ps[planecnt].roll < -0x1000) {
		droll = -0x1000 - ps[planecnt].roll;
	}
	else if(ps[planecnt].roll > 0x1000) {
		droll = 0x1000 - ps[planecnt].roll;
	}
	if(ps[planecnt].pitch > 0)
		pdest = 0x6000;
	else
		pdest = 0x3000;
}

void SplitS()
{
	COORD distkm, tdist, tdistnm;
	int relangle, dojinkangle;
	int dx, dy, tpitch, tangle;
	int enemyroll, enemyhead, enemyalt, enemypitch, e2uspitch;
	int enemynum;
	int targetspeed;

	i = planecnt;
	if((ps[i].alt < 500) || ((abs(ps[planecnt].roll) < 0x1800) && (ps[planecnt].pitch < -0x2A00))) {
		bv[i].Behavior = StraightAttack;
		StraightAttack();
//		Message("Split S Off", DEBUG_MSG);
		return;
	}
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

	if(abs(relangle) > 0x7000) {
		droll = 0x8000 - ps[planecnt].roll;
	}
	else if(relangle < 0) {
		droll = -0x7000 - ps[planecnt].roll;
	}
	else {
		droll = 0x7000 - ps[planecnt].roll;
	}

	if(abs(ps[planecnt].roll) >= 0x6800) {
		if(ps[planecnt].pitch < 0)
			pdest = -0x6000;
		else
			pdest = -0x3000;
	}
	else {
		if(ps[planecnt].pitch < 0)
			pdest = ps[planecnt].pitch;
		else
			pdest = -0x3000;
	}
}

void ReArmWingman()
{
	int cnt;
	struct PlaneData far *ptemp;

	for(cnt = 0; cnt < NPLANES; cnt++) {
		if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (bv[cnt].leader == -1) && (ps[cnt].status & FRIENDLY) && (!(ps[cnt].status & CRASH))) {
			ptemp = GetPlanePtr(ps[cnt].type);
			ps[cnt].mcnt[0] = ptemp->mcnt[0];
    		ps[cnt].mcnt[1] = ptemp->mcnt[1];
			bv[cnt].behaveflag &= ~(DAMAGERADAR|DAMAGEENGINE|DAMAGECONTROLSURFACE);
		}
	}
}

