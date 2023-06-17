/*  File:   Planes.c                              */
/*  Author: Sid Meier                             */
/*                                                */
/*  Game logic for Stealth: enemy planes          */


#define ELFAI 1
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
#include "sounds.h"
#include "f15defs.h"
#include "speech.h"
#include "armt.h"
#include "proto.h"
#include "setup.h"

long lfpx, lfpy, lox, loy;

int lastcatused = 4;
int sotnum = -999;
int friendfire = 0;
int enemyfire = 0;
long oldFPX;
long oldFPZ;
UWORD oldAltval;
char OurWingName[20];
extern  UBYTE   ExitCode;
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
extern	int	    Fticks;
extern  int     Speedy;
extern  int     whichourcarr;     // holds value for which boat our carrier
extern	int		LastCall;
extern int  AvailCat[];
extern int OurLoadOut;
extern int  MyMSGTYP;
extern int msgtime;

int     olddog;

static struct Planes *psi;  // pointer into "ps" structure array for this plane

static    int   i;          // index of this plane in "ps" array

int		  planecnt;			// used since somebody desided to make i static
							// and after the planes file grew to more than
							// 5000 lines I got sick of it and desided to
							// kludge something so I could split the file.

#if 1
// #ifdef REMOVEDCODE
static    COORD xd,yd;      // Delta coords to plane's destination
static    int   zd;
static    COORD dist;       // distance to plane's destination
static    int   angl;       // angle to plane's destination
#endif

int   pdest;      // pitch angle to plane's destination

static    int   dpitch;     // delta pitch this frame
int   droll;      // delta roll this frame

int   PlanesLaunched=0;
int AIWingMan = -999;

int tempvar = 1;
int tempbehavevar = 0;
long     AIBOMBX;     // 20-bit Scott-style X position of predicted Bomb impact
long     AIBOMBY;     // 20-bit Scott-style Y position of predicted Bomb impact
UWORD    AITTGT;      // 2 * Time to target in seconds
UWORD    AITREL;      // 2 * Time to target intercept (bomb release) in seconds
UWORD    AIRTGT;      // ground range to target in tenths of nautical miles
int		 MyWingy = 0;
int		 OtherAIDo = 0;
int		 OtherAITime = 0;
int		 WingID[3];
int		 AIWingMenu = 0;
int		 AIWingMenuTime = 0;
extern struct _path_ aipaths[];
extern struct _wayPoint_ aiwaypoints[];
extern struct _action_ aiactions[];


extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);


struct PlaneData far *ptype;


extern struct FPARAM{
	long	X;
	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;

int firstbehaveshow;

// The Strategy:
//
// Planes have three overall plans:
//
//      o PATROL means to fly amongst nearby enemy entries in the Rdrs list,
//        quitting when "time" runs out.  When this occurs, the nearest enemy
//        base is chosen as destination and the plane flys home or just
//        disappears.  When near the base, landing is initiated.  When landed,
//        the plane is deactivated.  PATROL is the default plane type.
//
//      o LOITER is similar to PATROL, except that the target is an enemy
//        base and never changes.  This gives infinite touch-n-go's.
//
//
// Overriding factors include:
//
//      o If he "pings" you, he comes after you until he loses you.
//
//      o If "detected" planes will go after your last known position
//
// Slots in the planes array are as follows:
//
//      o Last four are for CloseBase touch-n-gos
//



/* -----------------------------*/
/* Process all planes except us */
/* -----------------------------*/
doplanes () {
    static int dog=0;
	int droll1, droll2;
	int maxrollval;
	int lastbvspeed;
	int maxspeedchng;
	struct PlaneData far *tempplane;
	int ldrollstat;
	int maxfspdchng;
	int numldroll = 0;
	int numnldroll = 0;
	int turnval, alteffectval, alteffectmax;
	int prewing;
	int fdroll;
	long ldroll;
	int minpassed;

    int z, n;

//	return(0);

    n = ((Rtime>>2)&3)+FIRST_ENEMY_TRACE;
    trs[n].x = 0;
	maxspeedchng = (int)(((long) 25 * Fticks) /60);
	maxfspdchng = (int)(((long) 10 * Fticks) /60);

//	if(tempvar)
//		mclear_screen();

	if(AIWingMan != -999) {
        if (!(ps[AIWingMan].status & ALIVE)) {
			AIWingMan = -999;
		}
	}
#if 0
    TakeoffNewBasePlanes ();
#endif

    olddog = dog;
    dog = 0;
	carrmoveok = 1;
/*	if(tempvar) {
		TXT("Friend ");
		TXN(friendfire);
		TXA(" Enemy ");
		TXN(enemyfire);
		TXA(" NP ");
		TXN(NPLANES);
		Message(Txt,DEBUG_MSG);
	}  */

#ifdef SHOWDETMAR
	if(tempvar)
		ShowDetmarMission();
#endif

	ProcessOtherComm();
	if(minTimeAloft) {
		minpassed = (int)(TotalGameTicks/3600L);
		if(minpassed >= minTimeAloft) {
			Message("(RIO)Cleared To Return", RIO_MSG);
			Speech(RTB);
			LogIt (SLG_MIN_TIME_REACHED, -1, PxLng, (0x100000 - PyLng), Alt, -999, -999, 0L, LOG_AIR);
			minTimeAloft = 0;
		}
	}

	if((Alt < KnockItOffStatus) && (KnockItOffStatus > 1)) {
		Message("(CARRIER) Knock It Off", DEBUG_MSG);
        Speech (L_KNOCK_IT_OFF);
		KnockItOffStatus = 1;
	}

	if(lastOClock < Gtime) {
		DoOClockCall();
	}

    for (i=0, psi=ps; i<NPLANES; i++, psi++) {
		planecnt = i;
        Txt[0]=0;
//		TXT(" ");

        if (psi->status & ALIVE) {
			ptype = GetPlanePtr(psi->type);

//            if (psi->status&ACTIVE && (psi->speed || (bv[i].Behavior == Ready5))) { /* Planes in the air */
            if (psi->status&ACTIVE) { /* Planes in the air */
				if(Gtime <= 5) {
					GLowDroll = 1;
				}
				else if((ps[i].status & LOWDROLL) && (abs(ps[i].pitch) < 0x200)){
					numldroll++;
					GLowDroll = 1;
				}
				else {
					numnldroll++;
					GLowDroll = 0;
				}

				ldrollstat = 0;
				lastbvspeed = bv[i].basespeed;
				ps[i].status &= ~JUSTPUT;

#ifdef SHOWBEHAVIORS
				if((tempvar) && (i >= firstbehaveshow) && (i < (firstbehaveshow + 4)))
					ShowWhichBehave(i);
#endif


				if(GetSquadLeader(i) != -1)
					bv[i].finhead = ps[i].head;

#if 1
//#ifdef REMOVEDCODE
                GetDestinationXYZ ();

                GetDestinationAngles ();
#endif

				ChooseBehavior();

#if 1
				if((ps[i].status & CRASH) || (ps[i].status & TAKEOFF)) {
					droll = 0;
					if(bv[i].Behavior == AICarrierLand)	{
						bv[i].Behavior = FlyStraight;
						GLowDroll = 0;
					}
				}
				else if(psi->status & LANDING) {
					if(bv[i].Behavior == AICarrierLand) {
						AICarrierLand();
					}
					else {
                		Landing ();
					}
					if(CheckDetection(i) != -999) {
						Jink2();
					}
                }
//				if(((Rtime & 7) == (i & 7)) || ((!(bv[i].behaveflag & ENEMYNOFIRE)) && ((Rtime & 3) == (i & 3)))) {
//				if((Rtime & 1) != (i & 1)) {
//				if((Rtime & 3) == (i & 3)) {
//				if((((Rtime & 3) == (i & 3))) || ((ps[i].status & LOWDROLL) && ((Rtime & 1) != (i & 1)))) {
				else if(((((Rtime + 1) & 3) == (i & 3))) || ((ps[i].status & LOWDROLL) && ((Rtime & 3) != (i & 3)))) {
					droll = 0;
					if(bv[i].behaveflag & ISHELICOPTER)
						pdest = 0;
					else
						pdest = ps[i].pitch;
					if(!(ps[i].status & LOWDROLL))
						ldrollstat = 1;
				}
//                if (psi->status&ALERTED && dist<1900) {  40 nmiles
                else if (!(ps[i].status&(LANDING|BACKTOBASE))) { // 40 nmiles
                    ++ dog;
					ps[i].status |= LOWDROLL;

					bv[i].Behavior();

					if(!(ps[i].status & LOWDROLL))
						ldrollstat = 1;

//					Attacker();
//                    Dogfight ();
//					psi->Behavior();
                }
				else {
					ps[i].status |= LOWDROLL;

					bv[i].Behavior();

					if(!(ps[i].status & LOWDROLL))
						ldrollstat = 1;
				}
#else
				droll = 0;
				pdest = 0;
//				Formation();
//				FlyWayPoint();
//				FlyStraight();
//				bv[i].Behavior();
#endif

                if (psi->status & TAKEOFF) {
					TakingOff ();
					 lastbvspeed = bv[i].basespeed;
				}

                if (psi->status & CRASH) droll = 4<<8;

                if ((psi->status & PANIC) && (!(psi->status & (TAKEOFF|CRASH|CIVILIAN))) && ((bv[i].Behavior != FlyCruise) && (ptype->bclass != CMISSILE_OBJCLASS) && (bv[i].Behavior != EngageTrain))) {
					EvadeMissile(i);
					ps[i].status &= ~JUSTPUT;
				}

                if (!(psi->status & (TAKEOFF|JUSTPUT))) {

					if(abs(bv[i].basespeed - lastbvspeed) > maxfspdchng) {
						ldrollstat = 1;
					}

					if(bv[i].basespeed > lastbvspeed) {
						if((bv[i].basespeed - lastbvspeed) > maxspeedchng)
							bv[i].basespeed = lastbvspeed + maxspeedchng;
					}
					else {
						if(bv[i].Behavior == AICarrierLand) {
							if((lastbvspeed - bv[i].basespeed) > ((maxspeedchng<<1) + maxspeedchng))
								bv[i].basespeed = lastbvspeed - ((maxspeedchng<<1) + maxspeedchng);
						}
						else if(((lastbvspeed - bv[i].basespeed) > maxspeedchng) && (!(bv[i].behaveflag & ISHELICOPTER)))
							bv[i].basespeed = lastbvspeed - maxspeedchng;
					}

					if(!((bv[i].behaveflag & ISHELICOPTER) || (bv[i].Behavior == Immelmann) || (bv[i].Behavior == SplitS))) {
						CheckTurnStuff(i);
					}

				}

				if(bv[i].behaveflag & DAMAGECONTROLSURFACE)  /*  Control Surfaces Damaged  */
					droll /= 2;

				if(bv[i].Behavior == AICarrierLand) {
					turnval = 4;
				}
				else if(ps[i].alt > 8750)
					turnval = ptype->maneuver - 3;
				else if(ps[i].alt > 7500)
					turnval = ptype->maneuver - 2;
				else if(ps[i].alt > 5000)
					turnval = ptype->maneuver - 1;
				else
					turnval = ptype->maneuver;

				if(turnval <= 0) {
					if(ptype->maneuver <= 1)
						alteffectval = 1<<8;
					else
						alteffectval = 1<<9;
					alteffectmax = 0;
				}
				else {
					if(ptype->maneuver <= 1)
						alteffectval = turnval<<9;
//						alteffectval = turnval<<10;
					else
						alteffectval = turnval<<10;
//						alteffectval = turnval<<11;
					alteffectmax = ((turnval- 1)<<11);
				}
				prewing = GetSquadLeader(i);

//				if((bv[i].Behavior != Immelmann) && (bv[i].Behavior != SplitS) && (bv[i].Behavior != Jink) && (bv[i].Behavior != Jink2) && (bv[i].Behavior != StraightAttack)) {
				if((bv[i].Behavior != Immelmann) && (bv[i].Behavior != SplitS)) {
					if((abs(droll) < (alteffectval>>1)) && (prewing != -1))
						alteffectval = alteffectval>>4;
					else if((abs(droll) < alteffectval) && (prewing != -1))
						alteffectval = alteffectval>>3;
					else if(abs(droll) < (alteffectval<<2)) {
						if(prewing == -1)
							alteffectval = alteffectval>>1;
						else
							alteffectval = alteffectval>>2;
					}
				}

				droll1 = droll;

                droll = rng2(droll, -alteffectval, alteffectval);
				maxrollval = 0x2000 + alteffectmax;

				if(turnval <= 0)
					maxrollval = 0x1800;

				if(maxrollval > 0x3A25)
					maxrollval = 0x3A25;

				ldroll = (long)droll*4/FrameRate;
				fdroll = (int)ldroll;
				if((abs(fdroll + psi->roll) > maxrollval) && ((bv[i].Behavior != Immelmann) && (bv[i].Behavior != SplitS))) {
//					if(psi->roll < 0)
					if(abs(ps[i].roll > 0x4000)) {
						if(ps[i].roll < 0)
							droll = -maxrollval - psi->roll;
						else
							droll = maxrollval - psi->roll;
					}
					else {
						if(droll < 0)
							droll = -maxrollval - psi->roll;
						else
							droll = maxrollval - psi->roll;
					}
					ldroll = (long)droll*FrameRate/4;
					droll = (int)ldroll;
	                droll = rng2(droll, -alteffectval, alteffectval);
				}

#if 0 /* AAA  */
				if(tempvar && (i == 3)) {
					sprintf(Txt, "dr 1 %x, 2 %x, 3 %x, r %x, pt %d", droll1, droll2, droll, psi->roll, ptype->maneuver);
					Message(Txt,DEBUG_MSG);
				}
#endif

				if(!((ps[i].status & JUSTPUT) || (bv[i].Behavior == Ready5) || ((psi->status & TAKEOFF) && (psi->base < 0) && (bv[i].threathead < 0)))) {
                	AvoidMountains ();

//                	Landing ();
					if(bv[i].behaveflag & ISHELICOPTER) {
                		UpdateHelicopterHPR_SandP ();
					}
					else {
                		UpdateHeadPitchRoll ();

                		UpdateSpeedAndPosition ();
					}
				}

				if(Gtime <= 5)
					bv[i].finhead = ps[i].head;

				if(ldrollstat)
					ps[i].status &= ~LOWDROLL;

                psi->status &= ~PANIC;

                if ((!(ps[i].status & TAKEOFF)) && (Rtime%(FrameRate*4) == (i&3)*FrameRate) && (bv[i].Behavior != FlyCruise)) {
                    if (!(psi->status&CRASH)) {
	                    search ();
/*                        if (!(bv[i].behaveflag & ENEMYNOFIRE)) {
	                        search ();
                        }  */
                    }
                }
				if(sotnum != -999) {
					if(trs[sotnum].x == 0L) {
						sotnum = -999;
					}
				}
            } else {
				ps[i].speed = 0;
            }
        }
		else {
			ps[i].speed = 0;
		}
	}
	OtherAIDo = 0;

	oldFPX = FPARAMS.X;
	oldFPZ = FPARAMS.Z;
	oldAltval = Alt;
	UpdateOurWPS(-999, 0);
#if 0
	if(tempvar) {
		TXT("LDR ");
		TXN(numldroll);
		TXA(" ");
		TXN(numnldroll);
		if(Night)
			TXA(" NIGHT ");
		Message(Txt,DEBUG_MSG);
//		sprintf(Txt, "X %ld, Y %ld", PxLng, (0x100000-PyLng));
//		Message(Txt,DEBUG_MSG);
		Sound(N_JetPassesBy, 1, 0);
	}
#endif
}


/*----------*/
int GetAltPitch(int orgalt, int destalt, COORD tdist) {
	int tpitch;

	if (orgalt > destalt) {
		tpitch = -angle((orgalt - destalt)>>(HTOV), tdist);
	}
	else {
		tpitch = angle((destalt - orgalt)>>(HTOV), tdist);
	}
	return(tpitch);
}

#if 1
// #ifdef REMOVEDCODE

/*----------*/
GetDestinationXYZ () {     // sets xd,yd,zd, mode, CloseGuy
    int b;
	COORD bxloc, byloc;
	UWORD bzloc;
	int   bhead;
	int   bret;
	int   heightval;

	CloseGuy = 0;

    if ((psi->status&BACKTOBASE) && (!(psi->status & TAKEOFF))) {

		bret = ReturnBaseInfo(psi->base, &bxloc, &byloc, &bzloc, &bhead);
      /* ... i'm going back to base */

        xd=bxloc;

//        if (i>=NPLANES-2) xd-=RunwayLeftX;   left side of double runways for touch n' go's

        if (psi->status&LANDING) {
			  heightval = (GetQHeight(((long)bxloc<<HTOV), ((long)byloc<<HTOV)) >> 2);
//            zd = (psi->x-xd);
            yd=byloc;
//            xd -= 2*zd;
//            zd = abs(zd) + (bret ? 17:6);  /*  was 38 : 6  */
            zd = (bret ? 17:6) + heightval;  /*  was 38 : 6  */
        } else {
            yd=byloc + 190*Direction; // (was 190) 4 nm North/South of base centerpoint
            zd= 100 + Dist2D(psi->x-xd,psi->y-yd);
        }

    } else {

        if (TRUE) {  /* .. plane is alerted (he pinged you) */
//        if (psi->status&ALERTED) {  /* .. plane is alerted (he pinged you) */

            xd = Px;
            yd = Py;
            zd = Alt;

        } else if (detected) {
            xd=xlast[CloseGuy];
            yd=ylast[CloseGuy];
            zd=zlast[CloseGuy];

        } else { /* ... patrolling */

/*            if ( !((Gtime+(i<<3))&0xbf) ) {
                GetNewPatrolDest ();
            }  */

			bret = ReturnBaseInfo(psi->base, &bxloc, &byloc, &bzloc, &bhead);
            xd=bxloc;
            yd=byloc;
            zd=rng ((CloseGuy?ps[0].alt:Alt)+1000, 5000,20000);
        }
    }
}


/*----------*/
GetDestinationAngles () {   // set dist, angl, and pdest
    int dx, dy;

    dx   = xd-psi->x;  /* Calculate deltas, angles and dist */
    dy   = yd-psi->y;
    angl = angle(dx,-dy);
    dist = Dist2D(dx,dy);

    pdest = angle ((zd-psi->alt)>>(HTOV), dist);
	if(ps[i].status & LANDING) {
		if((psi->alt- zd) > 250) {
			pdest -= 0x800;
		}
	}
#if 0
	if(ps[i].status & LANDING) {
		if((psi->alt- zd) > 50) {
			pdest -= 0x800;
		}
		else {
			if(pdest < -0x800) {
				pdest = -0x800;
			}
		}
	}
#endif
    pdest = rng2 (pdest, -32<<8, 16<<8);
// SRE    droll = rng2 (angl-psi->head, -0x3000,0x3000) <<1;
//    droll = rng2 (angl-psi->head, -0x3000,0x3000);
    droll = rng2 ((angl-psi->head) - psi->roll, -0x3000,0x3000);

#if 0
//	if((tempvar) && ((i == 1) || (i == 14))) {
	if((tempvar) && (i == 1)) {
		if(ps[i].status & LANDING) {
			if(ps[i].status & BACKTOBASE)
				sprintf(Txt, "%d LANDING d %d a %x pd %x", i, dist, angl- psi->head, pdest);
			else
				sprintf(Txt, "%d OPPS d %d a %x pd %x", i, dist, angl- psi->head, pdest);
			Message(Txt,DEBUG_MSG);
		}
		else if(ps[i].status & BACKTOBASE) {
			sprintf(Txt, "%d Back to base d %d a %x, pd %x", i, dist, angl- psi->head, pdest);
			Message(Txt,DEBUG_MSG);
		}
	}
#endif

}
#endif

/*----------*/
TakingOff () {
	COORD bxloc, byloc;
	UWORD bzloc;
	int   bhead;
	int   bret;
	int   prewing, wingcnt;
	int   takeoffOK = 0;
	int   cattemp;
	int   oldthreathead;
	int   oldbasespeed;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;


	if(Gtime < 5)
		return(0);

	if((ps[i].base < 0) && (bv[i].threathead < 0) && (bv[i].groundt != -1)) {
		if(aiwaypoints[bv[i].groundt].x <= (long)-1) {
			bv[i].basespeed = 0;
			ps[i].speed = 0;
			if(bv[i].threathead > -5) {
				cattemp = (bv[i].threathead * -1) - 1;
				MoveAIToCat(i, cattemp);
			}
			return(0);
		}
	}

	if((bv[i].Behavior == Ready5)) {
		bv[i].basespeed = 0;
		ps[i].speed = 0;
		return(0);
	}

	if(bv[i].behaveflag & ISHELICOPTER) {
		if(bv[planecnt].basespeed < 100) {
			if(bv[i].basespeed <= 30) {
				oldbasespeed = bv[i].basespeed;
				bv[i].Behavior();
				bv[i].basespeed = oldbasespeed;
			}
			bv[planecnt].basespeed += 3;
			ps[planecnt].speed += 3;
			droll = 0;
		}
		else {
			bv[planecnt].basespeed = ptype->speed - (ptype->speed>>2);
			GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
			if(speedval != -1)
				bv[i].basespeed = speedval;
			ps[i].status &= ~TAKEOFF;
			LogIt (SLG_AIR_TAKEOFF, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, -999, 0L, LOG_AIR);
		}
		if(ps[i].alt < 200)
			pdest = 0x2000;
		else
			pdest = 0;
		return(0);
	}

	bret = ReturnBaseInfo(psi->base, &bxloc, &byloc, &bzloc, &bhead);

#if 1
	if(bret) {  /*  do wingsweeps, door raising and stuff like that here  */
		prewing = GetSquadLeader(i);
		wingcnt = prewing;
		while(prewing >= 0) {
			while(wingcnt >= 0) {
				if(wingcnt == i) {
					if(!takeoffOK)
						takeoffOK = 1;
				}
				else {
					if(ps[wingcnt].status & TAKEOFF) {
						if(!takeoffOK) {
							takeoffOK = -1;
						}
					}

				}
				wingcnt = bv[wingcnt].wingman;
			}
			prewing = bv[prewing].nextsec;
			wingcnt = prewing;
		}
	}
	if(takeoffOK == -1) {
		psi->speed = 0;
		droll = 0;
		pdest = 0;
		return(0);
	}
#endif

	if(Gtime < bv[i].varcnt){
		psi->speed = 0;
		bv[i].basespeed = 0;
		pdest = 0;
		droll = 0;
		psi->roll = 0;
		return(0);
	}
	else if(bv[i].varcnt != 0){
		bv[i].basespeed = 10;
		bv[i].varcnt = 0;
		psi->speed = 10;

		if((ps[i].base < 0) && (bv[i].threathead >= 0) && (bv[i].threathead < 4)) {
#ifdef SRECATTEST
			TXT("Plane Launch ");
			TXN(i);
			TXA(" CAT ");
			TXN(bv[i].threathead);
			Message(Txt);
#endif
			PlaneLaunched(bv[i].threathead, i);
	  		GetNextLaunch(i);
		}

	}

    droll = 0;

	oldthreathead = bv[i].threathead;

	if(bv[i].basespeed <= 30) {
		oldbasespeed = bv[i].basespeed;
		bv[i].Behavior();
		bv[i].basespeed = oldbasespeed;
		psi->roll = 0;
		droll = 0;
	}

	bv[i].threathead = oldthreathead;

    if (psi->speed<340) {
        psi->pitch=0;
		psi->roll = 0;
		droll = 0;
    } else if (psi->pitch<1024) {
        psi->pitch += 512/FrameRate;
		if(psi->roll < 0x2000) {
			droll = 0x400;
		}
    }
/*	else {
		if(psi->roll < 0x2000) {
			droll = 0x400;
		}
	}  */

    pdest = psi->pitch;

    if (psi->speed<ptype->speed) {
		if(!bret) {
        	psi->speed += 20/FrameRate;
			bv[i].basespeed += 20/FrameRate;
		}
		else {
       		psi->speed += 150/FrameRate;
			bv[i].basespeed += 150/FrameRate;
		}
    } else {
       	if (psi->alt > ((GetQHeight(psi->xL, psi->yL) >> 2) + 300)) {
			bv[planecnt].basespeed = ptype->speed - (ptype->speed>>2);
			GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
			if(speedval != -1)
				bv[i].basespeed = speedval;
			ps[i].status &= ~TAKEOFF;
			LogIt (SLG_AIR_TAKEOFF, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, -999, 0L, LOG_AIR);

//******* Moved Up groundt getting reset by running behavior
//			if(ps[i].base < 0) {
//				GetNextLaunch(i);
//			}

			return(0);
		}
    }
	if(bv[i].basespeed > ptype->speed)
		bv[i].basespeed = ptype->speed;
}

/*----------*/
void AvoidMountains () {     // sets pdest
	int heightval;
	long dx, dy;
	int orgheightval;
	int dheight;
	int levelarea1 = 50;
	int levelarea2 = 200;
	int daltclimb = 50;

	if(ps[i].status & (TAKEOFF|LANDING))
		return;

    if (((i&7) == (Rtime&7)) || (ps[i].status & PITCHUP)) {
		if(!(ps[i].status & PITCHUP)) {
			if(IsLand(psi->xL, psi->yL) == -1)
				return;
		}
		else {
        	psi->status &= ~PITCHUP;
		}
        orgheightval = heightval = (GetQHeight(psi->xL, psi->yL)>>2);
        if (psi->alt< heightval) {
			if(psi->status & CRASH) {
        		psi->status = 0;

				if(CloseEnoughFor(SNDSTUFF, psi->x, psi->y))
	        		Sound (N_FarExplo, 2, 0);

				StartASmoke( psi->xL, (heightval + 3),psi->yL, GRND2XPLO, 17 );

				if(bv[i].Behavior == AICarrierLand)
					LetNextPlaneLand();

        		if (atarg==i)
					atarg=-1;
				return;
			}
            psi->status |= PITCHUP;
		    pdest = 0x3000;
        }
		else {
			if(ptype->bclass == CIVILIAN_OBJCLASS) {
				levelarea1 = 1000;
				daltclimb = 1000;
			}
        	if (orgheightval > 25) {
        		if (psi->alt < (heightval + levelarea1)) {
		            psi->status |= PITCHUP;
					if(pdest < 0) {
						pdest = 0;
					}
				}
			}
	    	dx = sinXL(psi->head,500L);
    		dy = -cosXL(psi->head,500L);
        	heightval = (GetQHeight(psi->xL + dx, psi->yL + dy) >> 2) + daltclimb;
	        if (psi->alt< heightval) {
    	        psi->status |= PITCHUP;
				if(pdest < 0x3000)
				    pdest = 0x3000;
			}
			else {
				if((psi->alt< (heightval + levelarea2)) && (!(psi->status & PITCHUP))) {
		            psi->status |= PITCHUP;
					if(pdest < 0)
						pdest = 0;
				}

				dx = dx<<1;
				dy = dy<<1;
	        	heightval = (GetQHeight(psi->xL + dx, psi->yL + dy) >> 2) + daltclimb;
		        if (psi->alt< heightval) {
					dheight = heightval - psi->alt;
    		        psi->status |= PITCHUP;
					if((dheight < 25) && (pdest < 0x1000)) {
					    pdest = 0x1000;
					}
					else if((dheight < 300) && (pdest < 0x2000)) {
					    pdest = 0x2000;
					}
					if((dheight >= 300) && (pdest < 0x3000)) {
					    pdest = 0x3000;
					}
				}
				else if((psi->alt< (heightval + levelarea2)) && (!(psi->status & PITCHUP))) {
		            psi->status |= PITCHUP;
					if(pdest < 0)
						pdest = 0;
				}
			}
        }
    }

//    if (psi->status&PITCHUP) pdest = 0x3000;
}

/*----------*/
UpdateHeadPitchRoll () {
    int n;
	int fdroll;
	long ldroll;
	int troll;
	int odp = 0;


/* Update ROLL and HEADING */
	ldroll = (long)droll*4/FrameRate;
	fdroll = (int)ldroll;
	if((bv[i].Behavior != AICarrierLand) || (bv[i].threathead != -1)) {
   		psi->roll += fdroll;
		if(abs(psi->roll) > 0x4000)
			troll = 0x8000 - psi->roll;
		else
			troll = psi->roll;
    	psi->head += (troll>>3)/FrameRate;

		if((bv[i].Behavior != AICarrierLand) || ((bv[i].threathead != -1) && (bv[i].threathead != 0)))
		    odp = dpitch = pdest - psi->pitch;
		else
    		dpitch = rng2 (0xA00 - psi->pitch, -32<<8, 16<<8);
	}
	else {
    	psi->head += ((fdroll)/FrameRate);
		droll = rng2(-psi->roll, -ptype->maneuver<<9, ptype->maneuver<<9);
		ldroll = (long)droll*4/FrameRate;
		psi->roll += (int)ldroll;

    	psi->pitch += (rng2 (0xA00 - psi->pitch, -4<<8, 4<<8)*4)/FrameRate;
		return(0);
	}

	if((abs(droll) > 0x200) || (abs(dpitch) > 0x200) || (!IsCAPFlier(i) && (!(bv[i].behaveflag & ENEMYNOFIRE))))
		ps[i].status &= ~LOWDROLL;
	else
		ps[i].status |= LOWDROLL;

/* Trail smoke if CRASHing */
    if (psi->status & CRASH) {
//        dpitch=-2<<8;
		if(psi->pitch > -0x3E00)
        	dpitch=-2<<6;
//		StartASmoke( psi->xL, psi->alt, psi->yL, SMKTRAIL, 5 );
    }

/* Update PITCH */
//    if (psi->pitch<0 && psi->alt<(200-sinX(psi->pitch,2000))
    if (psi->pitch<0 && psi->alt<(((GetQHeight(psi->xL, psi->yL) >> 2) + 20) - sinX(psi->pitch,1000))
    && !(psi->status&(CRASH+LANDING)) ) {
        dpitch = 4<<8;
    }

    dpitch = rng2 (dpitch, -4<<8, 4<<8);
    psi->pitch += dpitch*4/FrameRate;

    if (abs(psi->pitch)>0x4000) {
        psi->head += 0x8000;
        psi->roll += 0x8000;
        psi->pitch = 0x8000 - psi->pitch;
    }
}

//
//         naut. miles    1 hour     6080 feet      1 bit
// Knots = ----------  x -------   x ---------   x  ------ ==> 27/64 bits/sec
//           hour        3600 secs   1 n. mile      4 feet
//

//***************************************************************************
UpdateSpeedAndPosition ()
{
    int a;
	long spd, Gspd;
	long dx, dy;
	long spd2;
	int speed2;
	long tspd;
	long precval;
	long precval2;
	int abspeed;
	long alteffect;
	long rolleffect;
	long rolleffect1;
	long rolleffect2;
	long rolleffect3;
	long rolleffect4;
	int pitchval;

	if(((ailoc[i].xloc / AILOCDIV) != ps[i].xL) || ((ailoc[i].yloc / AILOCDIV) != ps[i].yL)) {
		ailoc[i].xloc = ps[i].xL * AILOCDIV;
		ailoc[i].yloc = ps[i].yL * AILOCDIV;
	}

	if(bv[i].behaveflag & DAMAGEENGINE) {
		if(bv[i].basespeed > (ptype->speed / 2))
			bv[i].basespeed = ptype->speed / 2;
	}
	if((!(bv[i].behaveflag & ENEMYNOFIRE)) || (ps[i].status & BACKTOBASE)) {
		if(ptype->speed > 500) {
			abspeed = ptype->speed - 200;
			if(abspeed < 450)
				abspeed = 450;
			if(bv[i].basespeed > abspeed) {
				if(bv[i].fueltank > 14000) {
					bv[i].fueltank -= (100 * Fticks) /60;
				}
				else {
					bv[i].basespeed = abspeed;
				}
			}
		}
	}

    if (!(psi->status&(TAKEOFF|LANDING|LANDED))) {
        spd = (long)((long)bv[i].basespeed*(0x8000-(psi->pitch/2))) >>15;

		rolleffect = 0;
		if(abs(psi->roll) > 0xA00) {
        	rolleffect = absL( sinXL(psi->roll,spd)) >>1;
//        	rolleffect = absL( sinXL(psi->roll,spd)) >>2;
        	spd -= rolleffect;
		}
//		else
//	        spd -= abs( sinXL(psi->roll,spd)) >>2;

#if 0
   	    if (REALFLIGHT) {
//     	    a = rng(psi->alt, 5000>>2, 30000>>2) - 5000>>2;
//           	spd -= LMul (a, spd) / (20*2500L);
			spd2 = spd;
           	spd -= ((long)a * spd) / (20*2500L);
//			sprintf(Txt, "SPD %ld a %d spd2 %ld", spd2, a, spd);
//			Message(Txt);
       	}
#else

#if 0
		if((ps[i].alt > 1250) && (!(ps[i].status & CRASH))) {
			spd -= (long)GetDAltSpeed(ps[i].alt);
		}
#else
		if(!(ps[i].status & CRASH)) {
			spd -= (long)GetDAltSpeed(ps[i].alt);
		}
#endif
#endif

/*		if(tempvar) {
			sprintf(Txt, "P2 %d, bs %d s %ld, r %x, rd %ld, as %d", i, bv[i].basespeed, spd, ps[i].roll, rolleffect);
			Message(Txt, DEBUG_MSG);
		}  */

		if(spd < 0L)
			spd = 0L;

        psi->speed = (int)spd;
		if((psi->speed <= 0) && (bv[i].basespeed >= 0))
			psi->speed = 1;
    }
	else {
		if(bv[i].Behavior == AICarrierLand) {
        	psi->speed = bv[i].basespeed;
		}
	}

//    spd = ((psi->speed*27 >>6)) /FrameRate;
//    tspd = (((long)psi->speed*27 >>6) * Fticks * AILOCDIV) /60;

    if (psi->alt>16000) {     /* Too high, level off */
		if(psi->pitch > 0)
        	psi->pitch = 0;
	}

/*
	if(tempvar) {
		TXT("Plane ");
		TXN(i);
		TXA(" ");
		TXN(psi->speed);
		TXA(" ");
		TXN(bv[i].basespeed);
		TXA(" ");
		TXN(psi->alt);
		TXA(" ");
		TXN(GetDAltSpeed(ps[i].alt));
		Message(Txt, DEBUG_MSG);
	}  */


	if((bv[i].Behavior != AICarrierLand) || ((bv[i].threathead != -1) && (bv[i].threathead != 0)))
		pitchval = psi->pitch;
	else
		pitchval = pdest;

	if(Speedy == 1) {
	    tspd = (((long)psi->speed*27 >>6) * Fticks * AILOCDIV) /60;
		spd = tspd;
		if(spd < 0L)
			spd = 0L;
    	speed2 = ((psi->speed*27 >>6) * Fticks) /60;
		Gspd = cosXL (pitchval,spd);
		if(Gspd < 0L)
			Gspd = 0L;
    	ailoc[i].xloc += sinXL(psi->head,Gspd);
    	ailoc[i].yloc -= cosXL(psi->head,Gspd);
		ps[i].xL = ailoc[i].xloc / AILOCDIV;
		ps[i].yL = ailoc[i].yloc / AILOCDIV;
	    psi->alt += sinX(pitchval,(spd/AILOCDIV));
	}
	else {
    	spd = (((long)psi->speed*27 >>6) * Fticks) /60;
		if(spd < 0L)
			spd = 0L;
	    Gspd = cosXL(pitchval,spd);
		if(Gspd < 0L)
			Gspd = 0L;
	    psi->xL += sinXL(psi->head,Gspd);
    	psi->yL -= cosXL(psi->head,Gspd);
	    psi->alt += (int)sinXL(pitchval, spd);
	}


	if(GetSquadLeader(i) != -1) {
		ailoc[i].xloc = ((long)ps[i].xL * AILOCDIV) + (AILOCDIV>>1);
		ailoc[i].yloc = ((long)ps[i].yL * AILOCDIV) + (AILOCDIV>>1);
	}
	else {
		precval = FPARAMS.X / AILOCDIV;
		precval = FPARAMS.X - (precval * AILOCDIV);
		ailoc[i].xloc = ((long)ps[i].xL * AILOCDIV) + precval;
		precval2 = FPARAMS.Z / AILOCDIV;
		precval2 = FPARAMS.Z - (precval2 * AILOCDIV);
		ailoc[i].yloc = ((long)ps[i].yL * AILOCDIV) + precval2;
		lfpx = FPARAMS.X;
		lfpy = FPARAMS.Z;
		lox = ailoc[i].xloc;
		loy = ailoc[i].yloc;
	}

    psi->x = (psi->xL+16) >> HTOV;
    psi->y = (psi->yL+16) >> HTOV;

    if (psi->status & CRASH) {
		StartASmoke( psi->xL, psi->alt, psi->yL, SMKTRAIL, 5 );
    }

	if((ps[i].alt < KnockItOffStatus) && (KnockItOffStatus > 1) && (!(ps[i].status & (TAKEOFF|LANDING|LANDED|CRASH)))) {
		Message("(CARRIER) Knock It Off", DEBUG_MSG);
        Speech (L_KNOCK_IT_OFF);
		KnockItOffStatus = 1;
		bv[i].Behavior = AIGoHome;
		ps[i].status |= BACKTOBASE;
		bv[i].behaveflag |= ENEMYNOFIRE;
	}

    if (psi->alt<0) {  /* Too low, hit the ground */
		if(!(psi->status & CRASH)) {
			IndirectWPCorrection(i);
			LogIt (SLG_AIR_CRASH, i, psi->xL, psi->yL, psi->alt, -999, -999, 0L, LOG_AIR);
		}

        psi->status = 0;

		if(CloseEnoughFor(SNDSTUFF, psi->x, psi->y))
	        Sound (N_FarExplo, 2, 0);

		StartASmoke( psi->xL, psi->alt,psi->yL, GRND2XPLO, 17 );

		if(bv[i].Behavior == AICarrierLand)
			LetNextPlaneLand();

        if (atarg==i)
			atarg=-1;
    }
#if 0 /* AAA  */
//	if((tempvar) && (GetSquadLeader(i) == -1))
	if((tempvar) && (i == 3))
	{
/*		TXT(" ");
		TXN(i);
		TXA(" bspd ");
		TXN(bv[i].basespeed);
		TXA(" pspd ");
		TXN(psi->speed);
		TXA(", p ");
		TXN(psi->pitch);
		TXA(", FR ");
		TXN(FrameRate);  */
	    dx = sinX(psi->head,Gspd);
    	dy = -cosX(psi->head,Gspd);
//		sprintf(Txt, "%d %d, bs %d, ps %d, gs %d, dx %ld, dy %ld", i, Rtime, bv[i].basespeed, psi->speed, spd, dx, dy);
//		sprintf(Txt, "%d a %x, gs %d, dx %ld, dy %ld", i, psi->head, spd, dx, dy);
//		sprintf(Txt, "x %d %d, y %d %d, s %d %d", xxx1, xxx2, yyy1, yyy2, sss1, sss2);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

//***************************************************************************
Landing ()
{
	int otherplane;
	int dx, dy, xd, yd, zd;
	COORD bxloc, byloc;
	UWORD bzloc;
	int   bhead;
	int   bret;
	int   heightval;


	if((bv[i].Behavior == AICarrierLand) || (bv[i].Behavior == AIGoHome)) {
		return(0);
	}

	bret = ReturnBaseInfo(psi->base, &bxloc, &byloc, &bzloc, &bhead);

    if (psi->status&BACKTOBASE) {
        if ( dist < 1<<(BT2KM-2) ) {

            if (psi->status&LANDING) {
                psi->status |= LANDED;
//				Message("Should Be Landing",DEBUG_MSG);
            } else {
				if(bv[i].Behavior != AIGoHome) {
                	psi->status |= LANDING;
				}
            }
        }
    }
	if(psi->status & LANDING) {
		if(bv[i].basespeed > 140) {
			bv[i].basespeed = 140;
		}
	}

#if 0
//	if((tempvar) && (i == 1)) {
		if(ps[i].status & LANDING) {
			if(ps[i].status & BACKTOBASE)
				sprintf(Txt, "%d LANDING2 d %d a %x pd %x, %d h %x", i, dist, angl- psi->head, pdest, 1<<(BT2KM-2), psi->head);
			else
				sprintf(Txt, "%d OPPS2 d %d a %x pd %x, %d", i, dist, angl- psi->head, pdest, 1<<(BT2KM-2));
			Message(Txt,DEBUG_MSG);
		}
//	}
#endif

    if (psi->status&LANDING) {
		otherplane = bv[i].wingman;
		if(otherplane >= 0) {
       		if ((!(ps[otherplane].status & LANDING)) && (ps[otherplane].status & ALIVE) && (ps[otherplane].status&ACTIVE) && (!(ps[otherplane].status & CRASH))) {
				bv[otherplane].Behavior = AIGoHome;
				ps[otherplane].status |= BACKTOBASE;
				ps[otherplane].status &= ~(LANDED|LANDING);
			}
		}
		otherplane = bv[i].nextsec;
		if(otherplane >= 0) {
       		if ((!(ps[otherplane].status & LANDING)) && (ps[otherplane].status & ALIVE) && (ps[otherplane].status&ACTIVE) && (!(ps[otherplane].status & CRASH))) {
				bv[otherplane].Behavior = AIGoHome;
				ps[otherplane].status |= BACKTOBASE;
				ps[otherplane].status &= ~(LANDED|LANDING);
			}
		}
	}



    if (psi->status&LANDED) {

/*		if(ps[i].alt > 3) {
			TXT("Landed  ");
			TXN(i);
			TXA(" at Alt ");
			TXN(ps[i].alt);
			TXA(" s ");
			TXN(psi->speed);
			TXA(" bs ");
			TXN(bv[i].basespeed);
			Message(Txt,DEBUG_MSG);
		}  */

        heightval = (GetQHeight(((long)bxloc<<HTOV), ((long)byloc<<HTOV)) >> 2);
        psi->roll = psi->pitch = 0;
        psi->head = bhead;
        psi->alt = (bret ? 15:2) + heightval;

        if (psi->speed>0) {
            psi->speed -= 120/FrameRate;
			bv[i].basespeed -= 120/FrameRate;
        } else {
            psi->status = 0;  /*  Might want to put LOG info here  */
			LogIt (SLG_AIR_LAND, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, -999, 0L, LOG_AIR);
			IndirectWPCorrection(i);
        }

#if 0
        if (i>=NPLANES-4 && psi->speed<100) {
            psi->status &= PERMANENTBITS;
            psi->status |= ACTIVE + TAKEOFF + BACKTOBASE;
        }
#endif
    }

}

#if 1
// #ifdef REMOVEDCODE
/*----------*/

/*----------*/
ReturnToBase () {
    int amin, j, n;

    psi->status |= BACKTOBASE;

    amin = 0x7fff;
    for (j=3; j<NTARGET; j++) {
        if ((Rdrs[j].status&(AIRBASE+NEUTRAL)) == AIRBASE) {
            n = xydist (psi->x-Rdrs[j].x, psi->y-Rdrs[j].y);
            if (n<amin) {
                psi->base = j;
                amin = n;
            }
        }
    }
}
#endif


/*----------*/
/* Airborne radar detection algorithm */
/*----------*/
search ()
{
    COORD       dist;
    int         signal;
    int         angl;
    int         j;
    int         mrng;
    struct Sams *ssk;
//    struct sam  *S;
	struct MissileData far *S;
	int tdistnm;
	int targetnum;
	int friendflag;
	int missilenum;
	int imaboat = 0;
	int malt, mhead, mpitch, mroll;
	int missileangle;
	COORD ddist;
	int enemypitch, enemyalt;
	int pwtargetnum;
	int organgl;

	i = planecnt;
	if(IsBoat(ps[planecnt].type)) {
		bv[planecnt].behaveflag &= ~ENEMYNOFIRE;
		bv[planecnt].behaveflag |= RADARON;
		imaboat = 1;
	}

	friendflag = FriendStatus(i);
	bv[i].behaveflag &= ~ENEMYLOCK;

	if((!imaboat) && ((bv[i].behaveflag & NO_SWITCH_TARGET) || ((GetSquadLeader(i) != i) && (bv[i].Behavior == Formation)))) {
		if(bv[i].Behavior == Formation)
			AIRadarForwardCheck(i, 0, 0);
	    signal = RsignalOne (ps[i].x,ps[i].y,ps[i].alt, ptype->rclass, &angl,&dist,friendflag, bv[i].target);
		if(bv[i].target == -1) {
			targetnum = MAXPLANE;
		}
		else
			targetnum = bv[i].target;
	}
	else {
	    signal = Rsignal (ps[i].x,ps[i].y,ps[i].alt, ptype->rclass, &angl,&dist,friendflag, &targetnum);
	}

	if(targetnum < -1)  /*  No Viable Target  */
		return(0);

	if(imaboat) {
		if(targetnum == MAXPLANE)
			bv[planecnt].target = -1;
		else
			bv[planecnt].target = targetnum;
	}

	organgl = angl;
   	angl = abs(angl-ps[i].head);

    ps[i].status&=~LOCK;
#ifdef ELFMMSG
	if((tempvar) && (!(bv[i].behaveflag & ENEMYNOFIRE)) && (GetSquadLeader(i) == -1)) {
		TXT("Bog ");
		TXN(i);
		TXA(", sig ");
		TXN(signal);
		TXA(", dist ");
		TXN(dist);
//		TXA(", angl ");
//		TXN(angl);
		TXA(" bs ");
		TXN(ps[i].status & BACKTOBASE);
		TXA(", alert ");
		TXN(ps[i].alert);
		if(ps[i].status & FRIENDLY)
			TXA(" F");
		else
			TXA(" E");
		Message(Txt,DEBUG_MSG);
	}
#endif
/* Check for detection */
    if (!(ps[i].status&BACKTOBASE)           // skip if going back to base
    &&( (signal>dist && (imaboat || (angl<10923)))         // in range and in radar cone (60) ?
      ||(dist<bv[i].winglevel*4 && !Night) )){         // got a visual tally? (max=12 Km)
		if(Gtime > 5)
        	ps[i].alert += (bv[i].winglevel+2)*16;          // accumulate alertness
		else
        	ps[i].alert = 193;          // accumulate alertness
        if (ps[i].alert>192) {
			if((!(bv[i].behaveflag & DAMAGERADAR)) || (dist<bv[i].winglevel*4 && !Night)) {
				pwtargetnum = GetSquadLeader(targetnum);
				if(pwtargetnum < MAXPLANE) {
					if((ps[pwtargetnum].time < Gtime) || (!((Gtime + i) & 0xF))) {
						tempgotcha = 3;
    	        		gotcha(targetnum);          // Detected! (I'm sure you are there now)
						tempgotcha = -999;
					}
				}
				else if (pwtargetnum == MAXPLANE) {
					tempgotcha = 3;
   	        		gotcha(targetnum);          // Detected! (I'm sure you are there now)
					tempgotcha = -999;
				}
			}

            if (bv[i].behaveflag & ENEMYNOFIRE) {
				return(0);
			}

			if(bv[i].behaveflag & RADARON) {
				bv[i].behaveflag |= ENEMYLOCK;
			}
			else if(dist<bv[i].winglevel*4 && !Night) {
				bv[i].behaveflag |= ENEMYLOCK;
			}

			if(DOINVISIBLE)
				return(0);

			if(Gtime < 13)
				return(0);

            for (j=0; j<3; j++) {           // try each missile type
				S=GetMissilePtr(ptype->misl[j]);
//                S=s+ptype->misl[j];
				if((S->homing != IR_1) && (S->homing != IR_2)
						&& (S->homing != RDR_1) && (S->homing != RDR_2))
					continue;

			    ddist = (int)((long)dist*825/32);   // convert from Km
				if(targetnum == MAXPLANE)
					enemyalt = Alt;
				else
					enemyalt = ps[targetnum].alt;

				enemypitch = GetAltPitch(ps[i].alt, enemyalt, ddist);
				enemypitch = abs(enemypitch - ps[i].pitch);

				missileangle = 4096 - ((MAXWINGLEVEL - bv[i].winglevel) * 205);
                if (ps[i].mcnt[j]>0          // any of these missiles left?
//                && ((angl<4096) || (imaboat)) // in missile cone? (+-22.5)
                && (((angl<missileangle) && (enemypitch<missileangle)) || (imaboat)) // in missile cone? (+-22.5)
                && (dist>S->minrange)         // check rmin
                && (dist<S->range)       	// check rmax
				&& (OurLoadOut != SL_GUNS)	// Not GUNS ONLY
				&& (!(bv[i].behaveflag & ENEMYNOFIRE))) {	// see if they are allowed to fire

                    if (((S->homing!=IR_1) && (bv[i].behaveflag & RADARON))
					||  (S->homing == IR_2)
                    ||  ((S->homing == IR_1) && CheckTailAspect(ps[i].xL, ps[i].yL))) {  // check for tail-aspect homers

                        ps[i].status|=LOCK;

                        if (dist < (S->range-(S->range>>3)) ) { // 7/8 range to fire
							if(targetnum == MAXPLANE)
								missilenum = FindEmptyMissileSlotP (0, NPSAM, i);
							else
								missilenum = FindEmptyMissileSlot2 (0, NPSAM, i);
							if(missilenum == -1) {
	                            ssk=ss;
							}
							else {
	                            ssk=ss + missilenum;
							}
                            if ((ssk->dist==0) && (missilenum != -1)) {         // is missile slot available?
								if(ps[i].status & FRIENDLY) {
//									Message("Friendly Fire",DEBUG_MSG);
									friendfire ++;
								}
								else {
//									Message("Enemy Fire",DEBUG_MSG);
									enemyfire ++;
								}

								if(imaboat) {
                                	malt     = 15;
                                	mhead    = organgl;
                                	mpitch   = 0x2000;
                                	mroll    = 0;
									LogIt (SLG_BOAT_MISSILE_LAUNCH, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, ptype->misl[j], 0L, LOG_BOAT);
								}
								else {
                                	malt     = ps[i].alt-2;
									if(malt <= 0) {
										malt = 1;
									}
                                	mhead    = ps[i].head;
                                	mpitch   = ps[i].pitch-0x400;
                                	mroll    = ps[i].roll;
									LogIt (SLG_AIR_MISSILE_LAUNCH, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, ptype->misl[j], 0L, LOG_AIR);
								}

						        firemissile2 (missilenum,
										0,     /* Offsets for drops from Modem guy?????? */
                            			ps[i].xL,
                            			ps[i].yL,
                            			malt,
                            			mhead,
                            			mpitch,
                            			mroll,
//                            			S->speed,
                            			ps[i].speed,
                            			targetnum,    // used for air-air only !
                            			i,
                            			0L,
                            			0L,
										ptype->misl[j],
										1);

                                if (targetnum == MAXPLANE) LaunchCall ();
								if(GetSquadLeader(i) == -1) {
									GetWingName(i, Txt);

				                    AIFoxCall (ptype->misl[j]);
									//Message("(Wingman) Missile Away",DEBUG_MSG);
								}
								if(targetnum != MAXPLANE)
									if(CheckAIBombs(targetnum) != -999)
										ShowJettison(targetnum);

								if(CloseEnoughFor(SLOWSTUFF, ps[i].x, ps[i].y))
                                	NoAccel();

								/*  Put in time until disengagement  */
#if 0
								if((CheckAIBombs(i) == -1) && (bv[i].varcnt == 0) && (!(ps[i].status & TAKEOFF)))
								{
									bv[i].varcnt = Gtime + 50 + (40 * bv[i].winglevel);
								}
#endif


                                --ps[i].mcnt[j];
                                if ((!imaboat) && (ps[i].mcnt[0]==0 && ps[i].mcnt[1]==0)) {
									if(GetSquadLeader(i) == -1) {
										GetWingName(i, Txt);
										TXA("Gone Winchester");
										Message(Txt,WINGMAN_MSG);
										Speech(WINCHESTER);
									}
									else {
                                    	ps[i].status |= BACKTOBASE;  // leave when ALL missiles gone!
										bv[i].Behavior = AIGoHome;
										GetAIBase(i);
									}
                                }
                            }
                        }
                    }
                }
				if((tempvar) && (GetSquadLeader(i) == -1)) {
					Message(Txt,WINGMAN_MSG);
				}
            }
        }
        ps[i].status |= ALERTED;
    } else {
        ps[i].status &= ~ALERTED;
        ps[i].alert -= 80;
    }
    ps[i].alert = rng(ps[i].alert, 0,255);
}

/*----------*/
CheckTailAspect (long x, long y) {
    int     dx, dy, hd;

    if (bv[i].target != -1) {
        dx = x-ps[bv[i].target].xL;
        dy = y-ps[bv[i].target].yL;
        hd =   ps[bv[i].target].head;
    } else {
        dx = x-PxLng;
        dy = y-(0x100000-PyLng);
        hd = OurHead;
    }

    return (abs(angle(dx, -dy)-hd)>21845); // rear 60 degrees
}
#ifdef YEP
/*----------*/
ActivatePlane (int pl, int base, int planeid)
{
    int t, i;
	struct PlaneData far *tempplane;


	if(base < 0)
		base = 0;


	t = SO_F-14FDA;
//    t = SO_F15E;
//	t = SO_F18;

    if (!(ps[pl].status&FRIENDLY))
		t = 8;

	if(planeid != -1) {
		t = planeid;
	}

    ps[pl].type = t;

	tempplane = GetPlanePtr(ps[pl].type);


    ps[pl].head = (Direction==1 ? 0:0x8000);

    if (Rdrs[base].status&CARRIER) {
        ps[pl].x = Rdrs[base].x+3*Direction;
        ps[pl].y = Rdrs[base].y-12*Direction;
        ps[pl].alt = 35;
        ps[pl].speed  = 100;
        ps[pl].head += -0x0400;

    } else {
        ps[pl].x = Rdrs[base].x;
        ps[pl].y = Rdrs[base].y+30*Direction;
        ps[pl].alt = 3;
        ps[pl].speed  = 10;
    }

    ps[pl].xL = (long) ps[pl].x<<HTOV;
    ps[pl].yL = (long) ps[pl].y<<HTOV;
	ailoc[pl].xloc = ps[pl].xL * AILOCDIV;
	ailoc[pl].yloc = ps[pl].yL * AILOCDIV;
    ps[pl].pitch = 0;
    ps[pl].roll = 0;
    ps[pl].status |= ALIVE+ACTIVE+TAKEOFF;
    ps[pl].base = base;
	ps[pl].time = 0;
	ps[pl].color = -999;
    ps[pl].mcnt[0] = tempplane->mcnt[0];
    ps[pl].mcnt[1] = tempplane->mcnt[1];

	bv[pl].Behavior = StraightAttack;
	bv[pl].leader = -999;
	bv[pl].wingman = -999;
	bv[pl].prevsec = -999;
	bv[pl].nextsec = -999;
	bv[pl].target = -1;
	bv[pl].groundt = -1;
	bv[pl].finhead = 0;
	bv[pl].basespeed = tempplane->speed / 2;
	bv[pl].behaveflag = RADARON;
	bv[pl].varcnt = 0;
	bv[pl].behavecnt = 0;
	bv[pl].verticlesep = 0;
	bv[pl].horizontalsep = PARADEDIST;
	bv[pl].fueltank = 20000;
	bv[pl].winglevel = 3;

}
#endif


//***************************************************************************
ActivatePlane(int pl, int base, int planeid)
{
    int t, i;
	struct PlaneData far *tempplane;

	if(base < 0)
		base = 0;

	// RIGHT NOW DEFAULT TO FIRST ENTRY
	t = 0;

    if (!(ps[pl].status&FRIENDLY))
	// RIGHT NOW DEFAULT TO FIRST ENTRY
		t = 0;

	if(planeid != -1) {
		t = planeid;
	}

	tempplane = GetPlanePtr(t);

    ps[pl].type = t;

    ps[pl].head = (Direction==1 ? 0:0x8000);

    if (Rdrs[base].status&CARRIER) {
        ps[pl].x = Rdrs[base].x+3*Direction;
        ps[pl].y = Rdrs[base].y-12*Direction;
        ps[pl].alt = 35;
        ps[pl].speed  = 100;
        ps[pl].head += -0x0400;

    } else {
        ps[pl].x = Rdrs[base].x;
        ps[pl].y = Rdrs[base].y+30*Direction;
        ps[pl].alt = 3;
        ps[pl].speed  = 10;
    }

    ps[pl].xL = (long) ps[pl].x<<HTOV;
    ps[pl].yL = (long) ps[pl].y<<HTOV;
	ailoc[pl].xloc = ps[pl].xL * AILOCDIV;
	ailoc[pl].yloc = ps[pl].yL * AILOCDIV;
    ps[pl].pitch = 0;
    ps[pl].roll = 0;
    ps[pl].status |= ALIVE+ACTIVE+TAKEOFF;
    ps[pl].base = base;
	ps[pl].time = 0;
	ps[pl].color = -999;

	ps[pl].mcnt[0] = tempplane->mcnt[0];
    ps[pl].mcnt[1] = tempplane->mcnt[1];
    ps[pl].mcnt[2] = tempplane->mcnt[2];

	bv[pl].Behavior = StraightAttack;
	bv[pl].leader = -999;
	bv[pl].wingman = -999;
	bv[pl].prevsec = -999;
	bv[pl].nextsec = -999;
	bv[pl].target = -1;
	bv[pl].groundt = -1;
	bv[pl].finhead = 0;
	bv[pl].basespeed = tempplane->speed / 2;
	bv[pl].behaveflag = RADARON;
	bv[pl].varcnt = 0;
	bv[pl].behavecnt = 0;
	bv[pl].verticlesep = 0;
	bv[pl].horizontalsep = PARADEDIST;
	bv[pl].fueltank = 20000;
	bv[pl].winglevel = 3;
}





/*----------*/

void GetTargADP(int planenum, int dx, int dy, COORD *tdist, int *tpitch, int *tangle, COORD *tdistnm, int *enemyalt, int deltalt)
{
	if(bv[planenum].target == -1) {
		*tdist = Dist2D(dx, dy);
		*tpitch = GetAltPitch(ps[planenum].alt, (Alt + deltalt), *tdist);
		*enemyalt = Alt;
	}
	else {
		*tdist = Dist2D(dx, dy);
		*tpitch = GetAltPitch(ps[planenum].alt, (ps[bv[planenum].target].alt + deltalt), *tdist);
		*enemyalt = ps[bv[i].target].alt;
	}
	*tangle = angle(dx, -dy);
//	*tpitch = rng2(*tpitch, -32<<8, 16<<8);
	*tpitch = rng2(*tpitch, -0x3800, 0x3800);
	*tdistnm = (*tdist * 2) / 95;
}

void CheckGunFiring(COORD tdistl, int tanglel, int tpitchl)
{
	int dbulpitch, dbulhead;
    int nn,tspd;
	int othertarget;
	int dxb, dyb, tpitchb, tangleb, enemyalt;
	COORD tdistb, tdistnm;
	long lx, ly;
	int ltangle;

	if(DOINVISIBLE)
		return;
#if 0
    if (Rtime&3) // .5 nm
		return;
#endif

	if(ptype->gun == 0)
		return;

	if((ptype->gun == 1) || (ptype->gun == 3)) {
		othertarget = bv[i].target;
		AIRadarRearCheck(i, 0, 1);
		if(bv[i].target == -1) {
			dxb = Px - psi->x;
			dyb = Py - psi->y;
			lx = FPARAMS.X - ailoc[i].xloc;
			ly = FPARAMS.Z - ailoc[i].yloc;
		}
		else {
			dxb = ps[bv[i].target].x - psi->x;
			dyb = ps[bv[i].target].y - psi->y;
			lx = ailoc[bv[i].target].xloc - ailoc[i].xloc;
			ly = ailoc[bv[i].target].yloc - ailoc[i].yloc;
		}
		GetTargADP(i, dxb, dyb, &tdistb, &tpitchb, &tangleb, &tdistnm, &enemyalt, 0);
		ltangle = Langle(lx, -ly);

		CoreCheckGunFiring(tdistb, ltangle, tpitchb, 0x8000, 0x1800, 0x1800, 48);

		if(ptype->gun == 3) {
			if(bv[i].target != -1)
				bv[i].target = othertarget;
			return;
		}
		bv[i].target = othertarget;
	}

	CoreCheckGunFiring(tdistl, tanglel, tpitchl, 0, (8<<8), 0x200, 24);
}

void CoreCheckGunFiring(COORD tdistl, int tanglel, int tpitchl, int turretangle, int fireangle, int dangle, int tdistfire)
{
	int dbulpitch, dbulhead;
    int nn,tspd;

    if (tdistl<tdistfire && (!(bv[i].behaveflag & ENEMYNOFIRE)) && // .5 nm
        abs((psi->head + turretangle)-tanglel)<(fireangle) && abs(psi->pitch-tpitchl)<(fireangle) ) {

		if(GetSquadLeader(i) == -1) {
			if(ps[i].time < Gtime) {
				ps[i].time = Gtime + 14;  //  time is usually used to indicate when the last
										 //  time the group was detected, and the
										 //  plane who's time is updated is the group leader.
										 //  since our wingman cannot be a group leader I am
										 //  using this as a delay so he doesn't go " GUNS GUNS GUNS..."
										 //  Trust me it will happen if I didn't do this.  SRE
				GetWingName(i, Txt);
				TXA("Guns");
				Message(Txt,WINGMAN_MSG);
				Speech(GUNS);
			}
		}

		if((warstatus == 0) && (GetSquadLeader(bv[i].target) == -1)) {
			warstatus = 1;
			LastCall = -4*60;
		}


		if((ptype->bclass != FIGHTER_OBJCLASS) || (ptype->bclass != HELICOPTER_OBJCLASS)) {
			ps[i].status |= LOCK;
			bv[i].behaveflag |= RADARON;
		}

        nn = ((Rtime>>2)&3)+FIRST_ENEMY_TRACE;

        if (CloseGuy) nn+=4;
        tspd = TSpeed * 2;
		dbulpitch = tpitchl - psi->pitch;
		dbulhead = tanglel - (psi->head + turretangle);
		if(dbulpitch >= 0) {
			if(dbulpitch > dangle)
				dbulpitch = dangle;
		}
		else {
			if(dbulpitch < -dangle)
				dbulpitch = -dangle;
		}
		if(dbulhead >= 0) {
			if(dbulhead > dangle)
				dbulhead = dangle;
		}
		else {
			if(dbulhead < -dangle)
				dbulhead = -dangle;
		}

//        trs[nn].dz = sinX (-(psi->pitch + dbulpitch), tspd);
        trs[nn].dz = sinX ((psi->pitch + dbulpitch), tspd);
        tspd       = cosX ( psi->pitch + dbulpitch, tspd);
        trs[nn].dx = sinX ( (psi->head + turretangle) + dbulhead,  tspd);
        trs[nn].dy = -cosX( (psi->head + turretangle) + dbulhead,  tspd);

        trs[nn].x = psi->xL;
        trs[nn].y = psi->yL;
        trs[nn].z = psi->alt;
		trs[nn].firedby = i;
    }
}

void GetEnemyTarget(int ainum)
{
	int cnt, founddist, wingrear;
	int dx, dy, dx1, dy1, angle;
	int x, y, targetnum;
	COORD dist, dist1, distnm;
	int wingnum, othertarget;
	int otherdist;
	int temppath, wayval;

	x = ps[ainum].x;
	y = ps[ainum].y;
	wingnum = -999;
	if(bv[ainum].leader != -999)
		wingnum = bv[ainum].leader;
	else if(bv[ainum].wingman != -999)
		wingnum = bv[ainum].wingman;

	if(bv[ainum].leader != -999)
		if(CheckWingmanSix(bv[ainum].leader)) {
			return;
		}

	targetnum = bv[ainum].target;

	if(bv[ainum].behaveflag & NO_SWITCH_TARGET) {
		if(targetnum == -1)
		{
			if(!(ps[ainum].status & FRIENDLY)) {
				return;
			}
		}
    	else if ((ps[targetnum].status & ALIVE) && (ps[targetnum].status&ACTIVE) &&
				(ps[targetnum].speed) && (!(ps[targetnum].status & (CRASH|CIVILIAN)))) { /* Planes in the air */
			return;
		}
		bv[ainum].behaveflag &= ~NO_SWITCH_TARGET;
	}
	othertarget = -999;
	if(((bv[ainum].leader == -1) || (bv[ainum].prevsec == -1)) && (DESIGNATED != -1) && (DESIGNATED != i)) {
		if(DESIGNATED != -1) {
			othertarget = DESIGNATED;
		}
	}
	if(bv[ainum].prevsec > -1) {
		othertarget = bv[bv[ainum].prevsec].target;
	}

	if (othertarget == -1) {
		if(ps[ainum].status & FRIENDLY) {
			othertarget = -999;
		}
	}
   	else if (!((ps[othertarget].status & ALIVE) && (ps[othertarget].status&ACTIVE) &&
			(!(ps[othertarget].status & (CRASH|CIVILIAN)))))
		othertarget = -999;

	founddist = 0;
	if(!(ps[ainum].status & FRIENDLY)) {
		founddist = 1;
    	dx = Px-x;
    	dy = Py-y;
    	dist = xydist(dx,dy);
    	CloseGuy=0;
		targetnum = -1;
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
				(ps[cnt].speed) && (cnt != othertarget) && (!(ps[cnt].status & (CRASH|CIVILIAN)))) { /* Planes in the air */
			if(!FriendMatch(ainum, cnt)) {
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

	distnm = (dist * 2) / 90;
	if(distnm > 200) {
		if(GetSquadLeader(ainum) != -1) {
#ifdef SHOWHOME
			TXT("Going Home ");
			TXN(ainum);
			Message(Txt,DEBUG_MSG);
#endif
			ps[ainum].status |= BACKTOBASE;
			bv[ainum].Behavior = AIGoHome;
			GetAIBase(ainum);
			if(bv[ainum].leader != -999)
				bv[ainum].Behavior = Formation;
		}
	}

	if((distnm > 20) && (othertarget > -1) && (bv[ainum].Behavior != StraightAttack)) {
		otherdist = xydist(ps[othertarget].x-x, ps[othertarget].y-y);
		if ((otherdist<dist)) {
			founddist = 1;
			targetnum = othertarget;
			if(targetnum == -1) {
				CloseGuy = 0;
			}
			else {
				CloseGuy = 1;
			}
		}
	}
	if(GetSquadLeader(ainum) == -1) {
		if((bv[ainum].Behavior == BracketLeft) || (bv[ainum].Behavior == BracketRight)
				|| (bv[ainum].Behavior == FlyStraight)) {
			if((distnm > 60) || (!founddist)) {
				bv[ainum].target == -1;
				return;
			}
		}
	}
	if(((distnm > 20) && (bv[ainum].Behavior == StraightAttack)) || (!founddist)) {
		if((distnm < 30) && (founddist)) {
			if(bv[ainum].leader == -1) {
				if(NearWingTarget(-1)) {
					bv[ainum].behaveflag &= ~NO_SWITCH_TARGET;  /*  Keep wingman near player, but not emergency  */
					return;
				}
			}
		}
		else if(othertarget != -999) {
			bv[ainum].target = othertarget;
		}
		else {
			if(bv[ainum].leader == -1) {
				if(NearWingTarget(-1)) {
					bv[ainum].behaveflag &= ~NO_SWITCH_TARGET;  /*  Keep wingman near player, but not emergency  */
					return;
				}
			}
#if 0
			TXT("Enemy Formation ");
			TXN(ainum);
			Message(Txt,DEBUG_MSG);
#endif
			bv[ainum].Behavior = Formation;
			bv[ainum].behaveflag &= ~RADARON;
			bv[ainum].behaveflag |= ENEMYNOFIRE;
			bv[ainum].target = -1;
//			if(bv[ainum].leader == -1)
			if(GetSquadLeader(ainum) == -1) {
				Speech(NOJOY);
				GetWingName(ainum, Txt);
				TXA("Reforming on wing");
				Message(Txt,WINGMAN_MSG);
				GetWingName(ainum, Txt);
			}
			else if(GetSquadLeader(ainum) == ainum) {
				wayval = bv[ainum].groundt;
				temppath = (int)bv[ainum].path;
				if((aipaths[temppath].wp0 + aipaths[temppath].numWPs) > wayval) {
					if(wayval >= 0)
						DoNextWayPt(ainum, wayval);
				}
			}
		}
	}

	if(founddist)
		bv[ainum].target = targetnum;
}

int IsCAPFlier(int ainum)
{

	if((bv[ainum].Behavior == SetUpCAP1) || (bv[ainum].Behavior == SetUpCAP2)
			|| (bv[ainum].Behavior == SetUpCAP3) || (bv[ainum].Behavior == CAPTurn1)
			|| (bv[ainum].Behavior == CAPTurn2) || (bv[ainum].Behavior == CAPLeg1)
			|| (bv[ainum].Behavior == CAPLeg2)  || (bv[ainum].Behavior == SARCAP0)
			|| (bv[ainum].Behavior == SARCAP1)  || (bv[ainum].Behavior == SARCAP2)
			|| (bv[ainum].Behavior == SARCAP3)) {
		return(1);
	}
	return(0);
}

int IsRefueling(int ainum)
{

#ifdef ELFREFUEL
	if((bv[ainum].Behavior == SetUpRefuel1) || (bv[ainum].Behavior == SetUpRefuel2)
			|| (bv[ainum].Behavior == RefuelTurn1) || (bv[ainum].Behavior == RefuelTurn2)
			|| (bv[ainum].Behavior == RefuelLeg1) || (bv[ainum].Behavior == RefuelLeg2)
			|| (bv[ainum].Behavior == ReleaseRefuel) || (bv[ainum].Behavior == AILockedRefuel)
			|| (bv[ainum].Behavior == AIWaitRefuel) || (bv[ainum].Behavior == AIRefuel)
			|| (bv[ainum].Behavior == HelpAIRefuel)
			|| (bv[ainum].Behavior == FlyRefuel) || (bv[ainum].Behavior == HoldSpeedRefuel)) {
		return(1);
	}
#endif
	return(0);
}

void FriendlyChooseBehavior()
{
	int behavechange, wingnum, leadernum, prewing;
	int dxl, dyl;
	int tanglel, tanglew, tpitchl, tpitchw, ranglel, ranglew, rranglel, rranglew;
	int enemyalt;
	COORD tdistl, tdistw, tdistnm;
	int temphold;
	long lx, ly;
	int ltangle;
	int friendval;

	if(ProcessAICommunications(i))
		return;

    if ((psi->status & TAKEOFF) || (psi->status & BACKTOBASE))
		return;

	wingnum = bv[i].wingman;
	leadernum = bv[i].leader;

	prewing = GetSquadLeader(i);

	behavechange = 0;
	if((bv[i].Behavior == Formation) || (bv[i].Behavior == FlySanitize) || (IsBombing(i))
			|| IsCAPFlier(i) || IsRefueling(i)) {
		return;
	}

	CheckAITarget(i);

#if 0
	else {
		if(bv[i].target != -1) {
			if ((ps[bv[i].target].status & CRASH) || (!(ps[bv[i].target].status & ALIVE)) || (!(ps[bv[i].target].status&ACTIVE))
					|| (ps[bv[i].target].speed == 0))
			{
				bv[i].target = -1;
				GetEnemyTarget(i);
			}
		}
	}
#endif

	if((bv[i].Behavior == BracketLeft) || (bv[i].Behavior == BracketRight)
			|| (bv[i].Behavior == FlyStraight)) {
		temphold = 0;
		if(Rtime & 0x10)
			temphold = 1;

		if(bv[i].target != -1) {
			if((ps[bv[i].target].status & CRASH) || (!(ps[bv[i].target].status & ALIVE)))
				temphold = 1;
		}


		if(temphold) {
			if(bv[i].leader != -999) {
				if(!(CheckWingmanSix(bv[i].leader))) {
					if(!(bv[i].behaveflag & NO_SWITCH_TARGET)) {
						AIRadarForwardCheck(i, 0, 0);
					}
				}
			}
			else {
				if(!(bv[i].behaveflag & NO_SWITCH_TARGET)) {
					AIRadarForwardCheck(i, 0, 0);
				}
			}
		}
	}

	if(bv[i].Behavior == Formation)
		return;

	if(bv[i].target == -1)
		return;

	if(bv[i].target == -1) {
		dxl = Px - psi->x;
		dyl = Py - psi->y;
		lx = FPARAMS.X - ailoc[i].xloc;
		ly = FPARAMS.Z - ailoc[i].yloc;
		friendval = FRIENDLY;
	}
	else {
		dxl = ps[bv[i].target].x - psi->x;
		dyl = ps[bv[i].target].y - psi->y;
		lx = ailoc[bv[i].target].xloc - ailoc[i].xloc;
		ly = ailoc[bv[i].target].yloc - ailoc[i].yloc;
		friendval = FriendStatus(bv[i].target);
	}
	GetTargADP(i, dxl, dyl, &tdistl, &tpitchl, &tanglel, &tdistnm, &enemyalt, 0);

	ltangle = Langle(lx, -ly);

#if 0
	if(tempvar) {
		TXT("ANGLE ");
		TXN(i);
		TXA(" ");
		TXN(tanglel);
		TXA(" ");
		TXN(ltangle);
		Message(Txt,DEBUG_MSG);
	}
#endif

//	CheckGunFiring(tdistl, tanglel, tpitchl);

	if((!FriendMatchType(i, friendval)) && ((Rtime & 7) != (i & 7)))
		CheckGunFiring(tdistl, ltangle, tpitchl);

	if((bv[i].Behavior == Immelmann) || (bv[i].Behavior == SplitS)) {
		return;
	}

	if(tdistnm < 20) {
		if((bv[i].Behavior == BracketLeft) || (bv[i].Behavior == BracketRight)
				|| (bv[i].Behavior == FlyStraight)) {
			bv[i].Behavior = StraightAttack;
		}
	}

	if(tdistnm <= 10)
		CloseInFight(tanglel, wingnum, leadernum);

}

int KeepBehavior(int tdistnm, int prewing)
{
	if((bv[i].Behavior == Wall) || (bv[i].Behavior == Wall2)) {
		return(1);
	}
	else if((bv[i].Behavior == Ladder) || (bv[i].Behavior == Ladder2) || (bv[i].Behavior == LadderCircle)) {
		return(1);
	}
	else if((bv[i].Behavior == AIBombTarget) || (bv[i].Behavior == AIGetBombDist) || (bv[i].Behavior == LowBombTarget)) {
		return(1);
	}
	else if(IsBombing(i)){
		return(1);
	}
	else if((bv[i].Behavior == AIGoHome) || (bv[i].Behavior == AIGoHome2) || (bv[i].Behavior == AICarrierLand) || (bv[i].Behavior == Ready5)) {
		return(1);
	}
	else if((bv[i].Behavior == GorillaBomb) || (bv[i].Behavior == GorillaCover) || (bv[i].Behavior == RearCover)) {
		return(1);
	}
	else if((bv[i].Behavior == FlyCruise) || (bv[i].Behavior == EngageTrain)) {
		return(1);
	}
	else if((bv[i].Behavior == FlyWayPoint) || (bv[i].Behavior == FlyWPFollowPlayer) || (bv[i].Behavior == FollowPlayer)) {
		return(1);
	}
	if((bv[i].Behavior == SARWait) || (bv[i].Behavior == SAR) || (bv[i].Behavior == DoAltHover)) {
		return(1);
	}
	if((bv[i].Behavior == Immelmann) || (bv[i].Behavior == SplitS) || (bv[i].Behavior == FlySanitize)) {
		return(1);
	}
/*	else if(bv[i].Behavior == FlyE2C) {
		return(1);
	}  */
	else if(bv[i].Behavior == Formation) {
		if(!(bv[i].behaveflag & ENEMYNOFIRE)) {
			if(!(ImNotaFighter(i, 0))) {
				if((tdistnm < 31) && (bv[i].prevsec == prewing))
					bv[i].Behavior = BracketLeft;
				else if(tdistnm < 31)
					bv[i].Behavior = BracketRight;
				else if(tdistnm < 20) {
					bv[i].Behavior = StraightAttack;
				}
			}
		}
		return(1);
	}
	else if((bv[i].Behavior == BracketLeft) || (bv[i].Behavior == BracketRight) || (bv[i].Behavior == FlyStraight)) {
		if(tdistnm < 23)
			bv[i].Behavior = StraightAttack;
		return(1);
	}
	else {
		return(0);
	}
	return(1);
}

/*----------*/
void ChooseBehavior() {
	int dxl, dyl, dxw, dyw;
	int tanglel, tanglew, tpitchl, tpitchw, ranglel, ranglew, rranglel, rranglew;
	COORD tdistl, tdistw, tdistnm;
	int changew = 0;
	int changel = 0;
	int prewing, wingcnt, secplace, tprewing;
	int wingnum, leadernum;
	int enemyalt;
	long lx, ly;
	int ltangle;
	int enemynum;
	int friendval;
	int checkarmed;
	int trainhead;
	int bandit;

	if(bv[i].Behavior == FlyCruise)
		return;

	if((Rtime & 7) != (i & 7)) {
		UpdateWing();
		if((KnockItOffStatus == 1) && (!(ps[i].status & (TAKEOFF|LANDING|LANDED)))) {
			bv[i].Behavior = AIGoHome;
			ps[i].status |= BACKTOBASE;
			bv[i].behaveflag |= ENEMYNOFIRE;
		}
	}

	if(ps[i].status & CRASH)
		return;

	prewing = GetSquadLeader(i);

//    if (ps[i].status&FRIENDLY) {
    if (prewing == -1) {
		FriendlyChooseBehavior();
		return;
	}

#if 1
    if ((prewing == i) && (!((Rtime + i) & 127))) {
		if((!(FriendMatchType(i, FRIENDLY))) && (prewing == GetSquadLeader(LOCKED))) {
			tempgotcha = 4;
			gotcha(LOCKED);
			tempgotcha = 5;
			gotcha(MAXPLANE);
			tempgotcha = -999;
		}
		enemynum = CheckDetection(i);
		if(enemynum != -999) {
			tempgotcha = 6;
			gotcha(enemynum);
			tempgotcha = -999;
		}
	}
#endif

//	if((Rtime & 7) != (i & 7))
	if((((Rtime & 15) != (i & 15)) && (bv[i].behaveflag & ENEMYNOFIRE))  || ((Rtime & 7) != (i & 7)))
		return;

	if(ImNotaFighter(i, 1)) {
		checkarmed = 0;
		if(prewing == GetSquadLeader(LOCKED)) {
			checkarmed = 1;
		}
		enemynum = CheckDetection(i);
		if(enemynum != -999) {
			checkarmed = 1;
		}
		if((bv[i].behaveflag & ENEMYNOFIRE) && (checkarmed)) {
			bv[i].behaveflag &= ~ENEMYNOFIRE;
			bv[i].behaveflag |= RADARON;
			ChangeGroupBehavior(i, 0);
//			UpdateInterceptGroup(i, 0)
		}
	}

	wingnum = bv[i].wingman;
	leadernum = bv[i].leader;

	bandit = CheckDetection(i);
	if(bandit != -999) {
		if(!(bv[i].behaveflag & DAMAGERADAR))
			bv[i].behaveflag |= RADARON;
		if(CheckAIBombs(i) != -1) {
			if(bandit == -1) {
    			dxl = Px- ps[i].x;
    			dyl = Py- ps[i].y;
    			tdistl = xydist(dxl,dyl);
			}
			else {
				tdistl = xydist(ps[bandit].x- ps[i].x, ps[bandit].y- ps[i].y);
			}
			if(tdistl < 450) { /*  10 nm  */
				ShowJettison(i);
				if((CheckAIMissiles(i) != -1) || (ptype->gun == FRONT_REAR_GUNS) || (ptype->gun == FRONT_GUNS)) {
					bv[i].Behavior = StraightAttack;
					bv[i].behaveflag &= ~ENEMYNOFIRE;
				}
				else {
					ps[i].status |= BACKTOBASE;
					bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
					TXT("Jettison Bombs, Going Home ");
					TXN(i);
					Message(Txt,DEBUG_MSG);
#endif
					GetAIBase(i);
				}
			}
		}
	}

	if((Gtime > bv[i].varcnt) && (bv[i].varcnt) && (!(ps[i].status & (BACKTOBASE|TAKEOFF))) && (!(DOINVISIBLE||ENEMYNODIE))) {
		if(leadernum != -999) {
			bv[i].Behavior = Formation;
		}
		else {
			psi->status |= BACKTOBASE;
			bv[i].Behavior = AIGoHome;
		}
		bv[i].varcnt = 0;
		if(!(bv[i].behaveflag & ENEMYNOFIRE)) {
			tempgotcha = 7;
			gotcha(bv[i].target);
			tempgotcha = -999;
		}

		/*  MIGHT WANT TO PUT CODE HERE SO THAT FIGHTERS WILL DISENGAGE THIS PLANE
		 			I.E. DON'T TRY TO CATCH UP WITH A PLANE THAT IS RUNNING AWAY  */

#ifdef SHOWHOME
		TXT("Out of Time, Going Home ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
#endif
		GetAIBase(i);

		if(wingnum >= 0) {
			if(bv[wingnum].Behavior == FreeFighter) {
				bv[wingnum].Behavior = StraightAttack;
			}
		}
		if(leadernum >= 0) {
			if(bv[leadernum].Behavior == FreeFighter) {
				bv[leadernum].Behavior = StraightAttack;
			}
		}
		return;
	}

	if(bv[i].target == -1) {
		dxl = Px - psi->x;
		dyl = Py - psi->y;
		lx = FPARAMS.X - ailoc[i].xloc;
		ly = FPARAMS.Z - ailoc[i].yloc;
		friendval = FRIENDLY;
		trainhead = OurHead;
	}
	else {
		dxl = ps[bv[i].target].x - psi->x;
		dyl = ps[bv[i].target].y - psi->y;
		lx = ailoc[bv[i].target].xloc - ailoc[i].xloc;
		ly = ailoc[bv[i].target].yloc - ailoc[i].yloc;
		friendval = FriendStatus(bv[i].target);
		trainhead = ps[bv[i].target].head;
	}

	GetTargADP(i, dxl, dyl, &tdistl, &tpitchl, &tanglel, &tdistnm, &enemyalt, 0);

	ltangle = Langle(lx, -ly);

//	CheckGunFiring(tdistl, tanglel, tpitchl);
	if(!FriendMatchType(i, friendval)) {
		if((!ENEMYNODIE) || (abs(tanglel + 0x8000 - trainhead) > 0x1000) || (abs(tanglel - ps[i].head) > 0x1000))
			CheckGunFiring(tdistl, ltangle, tpitchl);
	}

    if (psi->status & (TAKEOFF|BACKTOBASE))
		return;

	if(((IsCAPFlier(prewing)) || IsRefueling(prewing)) && (bv[i].Behavior != StraightAttack))
		return;

	CheckAITarget(i);

	if(KeepBehavior(tdistnm, prewing)) {
		return;
	}
#if 1
	else if(tdistnm > 33) {
		if((CheckAnyEnemyLock(i)) || (tdistnm > 40)) {
			if(bv[i].Behavior != Box) {
				bv[i].Behavior = Box;
#ifdef ELFDMSG
				TXT("Box ");
				TXN(i);
				Message(Txt,DEBUG_MSG);
#endif
			}
		}
		else {
			if(CheckDetection(i) != -999) {
				tprewing = prewing;
				wingcnt = i;
				while (tprewing >= 0) {
					while (wingcnt >= 0) {
						bv[wingcnt].Behavior = Champagne;
						secplace = (bv[wingcnt].behaveflag & 15);
						if(secplace <= 2)
							bv[wingcnt].behaveflag &= ~RADARON;
						wingcnt = bv[wingcnt].wingman;
					}
					tprewing = bv[tprewing].nextsec;
					wingcnt = tprewing;
				}
			}
		}
	}
	else if(tdistnm > 10){
		if((bv[i].Behavior == Box) && (tdistnm < 33)) {
#ifdef ELFDMSG
			TXT("Champagne ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			secplace = (bv[wingcnt].behaveflag & 15);
			if(secplace <= 2)
				bv[wingcnt].behaveflag &= ~RADARON;
			bv[i].Behavior = Champagne;
		}
//		else if((bv[i].Behavior == Champagne) && (((tdistnm < 17) && (prewing == i)) || ((tdistnm < 20) && (CheckDetection(prewing) != -999))))
		else if((bv[i].Behavior == Champagne) && ((tdistnm < 17) || ((tdistnm < 20) && (CheckDetection(prewing) != -999))))
		{
			wingcnt = prewing;
			while (prewing >= 0) {
				while (wingcnt >= 0) {
#ifdef ELFDMSG
					TXT("Straight Attack ");
					TXN(i);
					Message(Txt,DEBUG_MSG);
#endif
//					bv[wingcnt].Behavior = StraightAttack;
					bv[wingcnt].Behavior = FlyStraight;
					wingcnt = bv[wingcnt].wingman;
				}
				prewing = bv[prewing].nextsec;
				wingcnt = prewing;
			}
		}
		else if(tdistnm < 15) {
			if(CheckDetection(i) != -999) {
				bv[i].Behavior = Jink2;
				if(wingnum >= 0) {
					if(bv[wingnum].Behavior == FreeFighter) {
						bv[wingnum].Behavior = StraightAttack;
					}
				}
				if(leadernum >= 0) {
					if(bv[leadernum].Behavior == FreeFighter) {
						bv[leadernum].Behavior = StraightAttack;
					}
				}
			}
		}
	}
	else {
		if(bv[i].varcnt == 0)
		{
			bv[i].varcnt = Gtime + 120 + (40 * bv[i].winglevel);  /*  base was 50  */
		}
		CloseInFight(tanglel, wingnum, leadernum);
	}
#endif

	return;
}

void StraightAttack() {
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt, enemyroll;
	COORD tdist, distnm, distkm;
	long lx, ly;
	long ldist;
	int targetspeed;
	int endpitch;
	int alttemp;
	int tdist2;
	int orgspeed;
	int leadernum = -999;
	int orgrelangle;

	if((IsBoat(ps[planecnt].type)) || (ImNotaFighter(planecnt, 0))) {
		FlyWayPoint();
		return;
	}

	orgspeed = bv[i].basespeed;

	bv[i].basespeed = ptype->speed;

	if(bv[i].target == -1) {
		dx = Px - psi->x;
		dy = Py - psi->y;
		lx = FPARAMS.X - ailoc[i].xloc;
		ly = FPARAMS.Z - ailoc[i].yloc;
	}
	else {
		dx = ps[bv[i].target].x - psi->x;
		dy = ps[bv[i].target].y - psi->y;
		lx = ailoc[bv[i].target].xloc - ailoc[i].xloc;
		ly = ailoc[bv[i].target].yloc - ailoc[i].yloc;
	}

//	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);
	LGetTargADP(i, lx, ly, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0, &ldist);

	CheckRadarSet(tdist, tangle);  /*  Check to see if we should have radar on  */

	orgrelangle = relangle = tangle - psi->head;
	if(bv[i].target == -1) {
		enemyhead = OurHead;
		targetspeed = Knots + GetDAltSpeed(ps[i].alt);
		enemyalt = Alt;
		enemyroll = OurRoll;
	}
	else {
		enemyhead = ps[bv[i].target].head;
		targetspeed = bv[bv[i].target].basespeed;
		enemyalt = ps[bv[i].target].alt;
		enemyroll = ps[bv[i].target].roll;
	}
	rrelangle = (0x8000 + tangle) - enemyhead;
	pdest = tpitch;

	if(abs(relangle) < 0x3000) {
		if((abs(relangle) > 0x1800) && (distnm < 2)) {
			if(abs(rrelangle) > 0x2000) {
//				bv[i].basespeed = bv[i].basespeed>>2 + GetDAltSpeed(ps[i].alt) ;
				if(targetspeed > 300)
					bv[i].basespeed = targetspeed>>1;
				else
					bv[i].basespeed = 150;
			}
		}
		else if ((long)ldist < ((long)75 * AILOCDIV)) {
//			bv[i].basespeed = bv[i].basespeed>>2;
			bv[i].basespeed = targetspeed - (ldist / AILOCDIV);
		}
		else if ((long)ldist < ((long)200 * AILOCDIV)) {
			bv[i].basespeed = targetspeed;
		}
		else if ((long)ldist < ((long)850 * AILOCDIV)) {
			bv[i].basespeed = targetspeed + 50;
		}
		else if ((long)ldist < ((long)1500 * AILOCDIV)) {
			bv[i].basespeed = targetspeed + 100;
		}
		if(abs(relangle) < 0x1000)
			relangle = relangle<<2;
		else
			relangle = relangle<<1;
		if(distnm < 3) {
			if(rrelangle < 0) {
				if(abs(enemyroll + 0x4000) < 0x2000) {
					relangle -= 0x1000;
				}
			}
			else if (relangle > 0) {
				if(abs(enemyroll - 0x4000) < 0x2000) {
					relangle += 0x1000;
				}
			}
		}
	}
	else if(abs(relangle) > 0x5800) {
		if(abs(rrelangle) < 0x4000) {
			if(rrelangle < 0) {
				relangle = (enemyhead - 0x6000) - psi->head;
				if(relangle > 0)
					relangle *= -1;
			}
			else {
				relangle = (enemyhead + 0x6000) - psi->head;
				if(relangle < 0)
					relangle *= -1;
			}
		}
		if((abs(relangle) > 0x6000) && (abs(rrelangle) > 0x6800)) {
			if((((Gtime + i) & 0x1F) < (bv[i].winglevel<<1)) && (bv[i].winglevel > 2) && (!(bv[i].behaveflag & ISHELICOPTER)) && (ptype->maneuver > 1)) {
				if(ps[i].alt < 1000) {
					bv[i].Behavior = Immelmann;
//					Message("Immelmann1", DEBUG_MSG);
				}
				else if(ps[i].alt > 5250) {
					bv[i].Behavior = SplitS;
//					Message("SplitS1", DEBUG_MSG);
				}
				else if(Rtime & 1) {
					bv[i].Behavior = Immelmann;
/*					TXT("Immelmann2 ");
					TXN(i);
					TXA(" ");
					TXN(Rtime);
					Message(Txt, DEBUG_MSG);  */
				}
				else {
					bv[i].Behavior = SplitS;
/*					TXT("SplitS2 ");
					TXN(i);
					TXA(" ");
					TXN(Rtime);
					Message(Txt, DEBUG_MSG);  */
				}
			}
		}
	}
	else {
		if((abs(rrelangle) > 0x2000) && (distnm < 2)) {
			if(targetspeed > 300)
				bv[i].basespeed = targetspeed>>1;
			else
				bv[i].basespeed = 150;
		}
	}

#if 0
	tdist2 = tdist;
	if((ldist/AILOCDIV) < 32000L) {
/*		tdist2 = abs(cosX(relangle, (int)(ldist)));
		tdist2 /= AILOCDIV;  */
		tdist2 = (int)(ldist/AILOCDIV);
	}
	if((tdist2 - 100) < 0)
		tdist2 = 0;
	else
		tdist2 = tdist2 - 100;

	FormationSpeedAdjustUS(targetspeed, relangle, tdist2, bv[i].target);
#endif

	TurnAIPlaneBy(i, relangle);

	alttemp = ps[i].alt;

	leadernum = bv[i].leader;
	if(leadernum >= 0) {
		if((bv[leadernum].Behavior == StraightAttack) && (bv[leadernum].target == bv[i].target)) {
			if(tdist < 135) {
				if(ps[i].status & (FRIENDLY|NEUTRAL)) {
					droll = (-ps[leadernum].roll) - ps[i].roll;
				}
				enemyalt += 600;
			}
		}
	}

	if(alttemp < enemyalt) {
		if((alttemp < (enemyalt - 250)) && (enemyalt > 50))
			endpitch = 0x2000;
		else if((alttemp < (enemyalt - 5)) && (enemyalt > 5))
			endpitch = 0x800;
		else
			endpitch = 0x200;
		pdest = endpitch;
	}
	else if(alttemp > (enemyalt + 2)) {
		if(alttemp > (enemyalt + 250))
			endpitch = 0xD000;
		else if(alttemp > (enemyalt + 5))
			endpitch = 0xF800;
		else
			endpitch = 0xFE00;
		pdest = endpitch;
	}
	else
		pdest = 0;

	if(distnm < 4) {
		if((abs(rrelangle) < 0x4000) && (abs(orgrelangle) > 0x2000)) {
			pdest = 0x2800;
		}
		else if((abs(enemyroll + 0x4000) < 0x2000) || ((abs(enemyroll - 0x4000) < 0x2000))) {
			if((abs(rrelangle) > 0x5000) && (abs(orgrelangle) < 0x2000)) {
				if((ps[i].pitch < 0) && (ps[i].alt > 250) && ((enemyalt - ps[i].alt) < 400)) {
					if(pdest > -0x2000) {
						pdest = -0x2000;
					}
				}
			}
		}
	}

	bv[i].finhead = relangle + ps[i].head;

//#ifdef ELFBMSG
#if 0
	if(tempvar) {
		TXT("St At ");
		TXN(i);
		TXA(" targ ");
		TXN(bv[i].target);
		TXA(" d ");
		TXN(tdist);
//		TXA(" a ");
//		TXN(relangle);
		TXA(" s ");
		TXN(bv[i].basespeed);
		TXA(" ");
		TXN(targetspeed);
		sprintf(Txt, "ST AT %d targ %d d %d a %x oa %x s %d ts %d", i, bv[i].target, tdist, relangle, orgrelangle, bv[i].basespeed, targetspeed);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

//   #define ELFMSG 1

void Ladder() {
	int dx, dy, dz;
	int tx, ty, tz;
	int txl, tyl, tzl;
	int relangle, rrelangle;
	int tangle, tpitch, troll, leadhead, isleader, tanglel, enemyalt;
	COORD tdist, distkm, tdistnm;
	int prewing, prespeed, secplace, prewing2, wingcnt;
	int temphead, tsepangle;
	long lx, ly;
	int tempalt;
	int dpwhead;

	isleader = FALSE;
	prespeed = 0;
	secplace = (bv[i].behaveflag & 15) - 1;

/*	bv[i].behaveflag &= ~RADARON;
	bv[i].behaveflag |= ENEMYNOFIRE;  */

// **************************************************************************
//  Find out who is the flight leader
// **************************************************************************
//	prewing = GetSquadLeader(i);
	prewing = GetBehaviorLeader(i);

	if(i != prewing)
		bv[i].target = bv[prewing].target;

// **************************************************************************
//  Get the angle from leader to target
// **************************************************************************
#if 0
	if(bv[prewing].target == -1) {
		txl = Px;
		tyl = Py;
	}
	else {
		txl = ps[bv[prewing].target].x;
		tyl = ps[bv[prewing].target].y;
	}
	txl -= psi->x;
	tyl -= psi->y;
	tanglel = angle(txl, -tyl);
	if(bv[i].behaveflag & ENEMYNOFIRE)
		tanglel = ps[prewing].head;
#else
	tanglel = ps[prewing].head;
#endif

	dpwhead = bv[prewing].finhead - ps[prewing].head;

// **************************************************************************
//  Form the flight up along line extending from target through leader
// **************************************************************************
	if ((prewing != i) && (!(abs(dpwhead) > 0x600))) {
		temphead = tanglel + 0x8000;
		prespeed = bv[prewing].basespeed;
		lx = ailoc[prewing].xloc + sinXL(temphead, ((long)(125 * secplace) * AILOCDIV));
		ly = ailoc[prewing].yloc - cosXL(temphead, ((long)(125 * secplace) * AILOCDIV));

		tempalt = ps[prewing].alt - (125 * secplace);

		if(tempalt < 75)
			tempalt = 75;

		if((GLowDroll) && (!(ps[i].status & TAKEOFF))) {
			ps[i].status |= JUSTPUT;
			JustPutHere(i, lx, ly, tempalt, prewing);
			return;
		}

		LFlyTo(lx, ly, tempalt, prewing, prespeed);
		return;
	}
	else {
		isleader = TRUE;
		if(bv[i].target == -1) {
			dx = Px - psi->x;
			dy = Py - psi->y;

			GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
			rrelangle = (0x8000 + tangle) - OurHead;
			if(rrelangle < 0)
				tsepangle = -0x4000;
			else
				tsepangle = 0x4000;

			temphead = (OurHead + tsepangle);
			tx = Px + sinX(temphead, 270);
			ty = Py - cosX(temphead, 270);
//			tz = Alt + 225;
		}
		else {
			dx = ps[bv[i].target].x - psi->x;
			dy = ps[bv[i].target].y - psi->y;

			GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
			rrelangle = (0x8000 + tangle) - OurHead;
			if(rrelangle < 0)
				tsepangle = -0x4000;
			else
				tsepangle = 0x4000;

			temphead = (ps[bv[i].target].head + tsepangle);
			tx = ps[bv[i].target].x + sinX(temphead, 270);
			ty = ps[bv[i].target].y - cosX(temphead, 270);
//			tz = ps[bv[i].target].alt + 225;
		}
//		if(tz < 500)
//			tz = 500;
		tz = 2500;
	}

	if(tz < 50) {
		tz = psi->alt;
	}
	dx = tx - psi->x;
	dy = ty - psi->y;

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	if((prewing == i) && (!(psi->status & TAKEOFF))) {
		if((CheckAnyEnemyLock(i)) && (tdistnm < 44)) {
			wingcnt = prewing;
			prewing2 = prewing;
#if 0
			TXT("Switch to Ladder 2 ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			while (prewing2 >= 0) {
				while (wingcnt >= 0) {
						bv[wingcnt].Behavior = Ladder2;
						bv[wingcnt].behaveflag &= ~ENEMYNOFIRE;
						wingcnt = bv[wingcnt].wingman;
				}
				prewing2 = bv[prewing2].nextsec;
				wingcnt = prewing2;
			}
	 	}
		if(((!CheckAnyEnemyLock(i))) && (tdistnm < 4)) {
			wingcnt = prewing;
			prewing2 = prewing;
			while (prewing2 >= 0) {
				while (wingcnt >= 0) {
						bv[wingcnt].Behavior = LadderCircle;
						bv[wingcnt].basespeed = ptype->speed;
						if(!(bv[wingcnt].behaveflag & DAMAGERADAR))
							bv[wingcnt].behaveflag |= RADARON;
						bv[wingcnt].behaveflag &= ~ENEMYNOFIRE;
						wingcnt = bv[wingcnt].wingman;
				}
				prewing2 = bv[prewing2].nextsec;
				wingcnt = prewing2;
			}
	 	}
	}

//	tpitch = GetAltPitch(psi->alt, tz, tdist);
	tpitch = GetAltPitch(psi->alt, tz, 50);
	tpitch = rng2(tpitch, -32<<8, 16<<8);
	distkm = tdist*32/825;

	relangle = tangle - psi->head;
	if(abs(relangle) < 0x400)
		relangle = tanglel - psi->head;

	troll = relangle - psi->roll;
	pdest = tpitch;

	TurnAIPlaneBy(i, relangle);

	bv[i].finhead = relangle + ps[i].head;

	if(prespeed) {
//		FormationSpeedAdjustUS(prespeed, relangle, tdist, prewing);
		bv[i].basespeed = prespeed - 10;
	}

	if(((prewing == i) || (abs(dpwhead) > 0x600)) && (bv[i].behaveflag & ENEMYNOFIRE)) {
		if(prewing != i) {
			bv[i].groundt = bv[prewing].groundt;
			bv[i].xloc = bv[prewing].xloc;
			bv[i].yloc = bv[prewing].yloc;
		}
		if(bv[i].groundt != -1)
			FlyWayPoint();
	}

	if(prewing != i) {
		if(ps[i].alt > 50) {
			pdest = ps[prewing].pitch;
		}
	}

#if 0
//  #ifdef ELFBMSG
	if(tempvar) {
		TXT("LD ");
		TXN(i);
		TXA(" targ ");
		TXN(bv[i].target);
		TXA(", bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

#define SMBOXMIDDIST ((1 * 95) / 2)
#define SMBOXXDIST ((4 * 95) / 2)
#define SMBOXYDIST ((6 * 95) / 2)

#if 1
#define BOXMIDDIST 1500
#define BOXXDIST 3000
#define BOXYDIST 6000
#define BOXHDIST 2500
#else
#define BOXMIDDIST 15
#define BOXXDIST 30
#define BOXYDIST 60
#define BOXHDIST 25
#endif

void Box() {
	int dx, dy, dz;
	int tx, ty, tz, txl, tyl, tzl;
	int relangle;
	int tangle, tpitch, troll, leadhead, isleader, tanglel, enemyalt;
	COORD tdist, distkm, tdistnm;
	int prewing, prespeed, secplace;
	int txi, tyi;
	long lx, ly;
	int shw = 0;
	int dpwhead;
	int tsecplace, tsecp2;

	isleader = FALSE;
	prespeed = 0;
	secplace = (bv[i].behaveflag & 15);

// **************************************************************************
//  Find out who is the flight leader
// **************************************************************************
//	prewing = GetSquadLeader(i);
	prewing = GetBehaviorLeader(i);

	if(i != prewing)
		bv[i].target = bv[prewing].target;

	dpwhead = bv[prewing].finhead - ps[prewing].head;

	if(((secplace <= 1) || (prewing == i) || (abs(dpwhead) > 0x600)) && (bv[i].behaveflag & ENEMYNOFIRE)) {
		if(prewing != i) {
			bv[i].groundt = bv[prewing].groundt;
			bv[i].xloc = bv[prewing].xloc;
			bv[i].yloc = bv[prewing].yloc;
		}
		if(bv[i].groundt != -1) {
			FlyWayPoint();
			return;
		}
	}

	if((secplace <= 1) || (prewing == i) || (abs(dpwhead) > 0x600)) {
		txi = psi->x + sinX((psi->head + 0x4000), SMBOXMIDDIST);
		tyi = psi->y - cosX((psi->head + 0x4000), SMBOXMIDDIST);
		tz = 2500;
		if(bv[i].target == -1) {
			tx = Px;
			ty = Py;
//			tz = Alt + 500;
		}
		else {
			tx = ps[bv[i].target].x;
			ty = ps[bv[i].target].y;
//			tz = ps[bv[i].target].alt + 500;
		}
	}
	else {
		txi = psi->x;
		tyi = psi->y;
		tz = ps[prewing].alt;
//		tz = 2500;
/*		if(bv[i].target == -1) {
			tz = Alt;
		}
		else {
			tz = ps[bv[i].target].alt;
		}  */
		prespeed = bv[prewing].basespeed;
		if(secplace == 2) {
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			shw = 1;
		}
		else if(secplace == 3) {
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head), ((long)AILOCDIV * BOXYDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head), ((long)AILOCDIV * BOXYDIST));
			tz += BOXHDIST;
			shw = 2;
		}
		else if(secplace == 4) {
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			lx = lx + sinXL((ps[prewing].head), ((long)BOXYDIST * AILOCDIV));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			ly = ly - cosXL((ps[prewing].head), ((long)AILOCDIV * BOXYDIST));
			tz += BOXHDIST;
			shw = 3;
		}
		else if(secplace > 4) {
			tsecplace = secplace - 4;
			tsecp2 = tsecplace >> 1;
			if(tsecplace & 1)
				lx = ailoc[prewing].xloc;
			else
				lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			lx = lx + sinXL((ps[prewing].head), ((long)BOXYDIST * AILOCDIV * tsecp2));
			if(tsecplace & 1)
				ly = ailoc[prewing].yloc;
			else
				ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			ly = ly - cosXL((ps[prewing].head), ((long)AILOCDIV * BOXYDIST * tsecp2));
			while(tsecplace > 4)
				tsecplace = tsecplace >> 2;
			if(tsecplace > 2)
				tz += BOXHDIST;
			shw = 3;
		}
		if(tz < 50) {
			tz = psi->alt;
		}
		if((GLowDroll) && (!(ps[i].status & TAKEOFF))) {
			ps[i].status |= JUSTPUT;
			if(tz < 50)
				tz = 50;
			JustPutHere(i, lx, ly, tz, prewing);
			return;
		}

		LFlyTo(lx, ly, tz, prewing, prespeed);
		return;
	}

	if(tz < 50) {
		tz = psi->alt;
	}
	dx = tx - txi;
	dy = ty - tyi;

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	tpitch = GetAltPitch(psi->alt, tz, 50);
	tpitch = rng2(tpitch, -32<<8, 16<<8);
	distkm = tdist*32/825;

	relangle = tangle - psi->head;

	troll = relangle - psi->roll;
	pdest = tpitch;

	TurnAIPlaneBy(i, relangle);

	bv[i].finhead = relangle + ps[i].head;

	if(prespeed) {
//		FormationSpeedAdjust(prespeed, relangle, tdist, prewing);
		bv[i].basespeed = prespeed - 10;
	}

#if 0
	if(((secplace <= 1) || (secplace > 4) || (prewing == i) || (abs(dpwhead) > 0x600)) && (bv[i].behaveflag & ENEMYNOFIRE)) {
		if(prewing != i) {
			bv[i].groundt = bv[prewing].groundt;
			bv[i].xloc = bv[prewing].xloc;
			bv[i].yloc = bv[prewing].yloc;
		}
		FlyWayPoint();
	}
#endif

	if(prewing != i) {
		if(ps[i].alt > 50) {
			pdest = ps[prewing].pitch;
		}
	}

#ifdef ELFBMSG
	if(tempvar) {
		tanglel = angle((Px - psi->x), -(Py - psi->y));
		tanglel -= psi->head;
		TXT("Box ");
		TXN(i);
		TXA(" pw ");
		TXN(prewing);
		TXA(", ps ");
		TXN(prespeed);
		TXA(", bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void Champagne() {
	int dx, dy, dz;
	int tx, ty, tz, txl, tyl, tzl;
	int relangle;
	int tangle, tpitch, troll, leadhead, isleader, tanglel;
	COORD tdist, distkm, tdistnm;
	int prewing, prespeed, secplace;
	int txi, tyi;
	int winglead;
	long lx, ly;

	isleader = FALSE;
	prespeed = 0;
	secplace = (bv[i].behaveflag & 15);

// **************************************************************************
//  Find out who is the flight leader
// **************************************************************************
	prewing = GetSquadLeader(i);

	bv[i].basespeed = ptype->speed;

	if((secplace >= 3) && (secplace & 1)) {
		BracketLeft();
		return;
	}
	if((secplace >= 3) && (!(secplace & 1))) {
		BracketRight();
		return;
	}
	if(secplace < 3) {
		bv[i].basespeed -= 200;
		tz = 2500;
		if(secplace == 1) {
			txi = psi->x + sinX((psi->head + 0x4000), SMBOXMIDDIST);
			tyi = psi->y - cosX((psi->head + 0x4000), SMBOXMIDDIST);
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
		}
		else {
			txi = psi->x + sinX((psi->head - 0x4000), SMBOXMIDDIST);
			tyi = psi->y - cosX((psi->head - 0x4000), SMBOXMIDDIST);
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * BOXXDIST));
		}
		if(bv[i].target == -1) {
			tx = Px;
			ty = Py;
//			tz = Alt + 500;
		}
		else {
			tx = ps[bv[i].target].x;
			ty = ps[bv[i].target].y;
//			tz = ps[bv[i].target].alt + 500;
		}
	}

	if(tz < 50) {
		tz = psi->alt;
	}
	dx = tx - txi;
	dy = ty - tyi;
	tdist = Dist2D(dx, dy);
	tdistnm = (tdist * 2) / 95;

	if((CheckDetection(i) != -999) && (tdistnm < EVASIVEDIST)){
		winglead = bv[i].leader;
		if(winglead >= 0){
			winglead = CheckDetection(winglead);
		}
		else
			winglead = -999;
		if(winglead == -999) {
			if(!(bv[i].behaveflag & DAMAGERADAR))
				bv[i].behaveflag |= RADARON;
			PostHole();
			return;
		}
	}

//	tpitch = GetAltPitch(psi->alt, tz, tdist);
	tpitch = GetAltPitch(psi->alt, tz, 50);
	tangle = angle(dx, -dy);
	tpitch = rng2(tpitch, -32<<8, 16<<8);
//	distkm = tdist*32/825;

	relangle = tangle - psi->head;

	pdest = tpitch;

	TurnAIPlaneBy(i, relangle);

#if ELFDMSG
	if(tempvar) {
		tanglel = angle((Px - psi->x), -(Py - psi->y));
		tanglel -= psi->head;
		TXT("CP ");
		TXN(i);
/*		TXA(" pw ");
		TXN(prewing);
		TXA(" ps ");
		TXN(prespeed);  */
		TXA(", ra ");
		TXN(relangle);
		TXA(", bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

#define SMWALLMIDDIST 50
#define SMWALLXDIST 100

#if 1
#define WALLMIDDIST 1250
#define WALLXDIST 2500
#define WALLHDIST 750
#else
#define WALLMIDDIST 25
#define WALLXDIST 50
#define WALLHDIST 12
#endif

void Wall() {
	int dx, dy, dz;
	int tx, ty, tz, txl, tyl, tzl;
	int relangle;
	int tangle, tpitch, troll, leadhead, isleader, tanglel;
	COORD tdist, distkm, tdistnm;
	int prewing, prespeed, secplace;
	int txi, tyi;
	int wingcnt, prewing2;
	long lx, ly;
	int dpwhead;
	int dangle, tsecplace, tsecp2, dadd;

	isleader = FALSE;
	prespeed = 0;
	secplace = (bv[i].behaveflag & 15);

// **************************************************************************
//  Find out who is the flight leader
// **************************************************************************
//	prewing = GetSquadLeader(i);
	prewing = GetBehaviorLeader(i);

	dpwhead = bv[prewing].finhead - ps[prewing].head;

	if(i != prewing)
		bv[i].target = bv[prewing].target;

	if(((secplace <= 1) || (prewing == i) || (abs(dpwhead) > 0x600)) && (bv[i].behaveflag & ENEMYNOFIRE)) {
		if(prewing != i) {
			bv[i].groundt = bv[prewing].groundt;
			bv[i].xloc = bv[prewing].xloc;
			bv[i].yloc = bv[prewing].yloc;
		}
		if(bv[i].groundt != -1) {
			FlyWayPoint();
			return;
		}
	}

	if((secplace <= 1) || (prewing == i) || (abs(dpwhead) > 0x600)) {
		txi = psi->x + sinX((psi->head - 0x4000), SMWALLMIDDIST);
		tyi = psi->y - cosX((psi->head - 0x4000), SMWALLMIDDIST);
		tz = 3250;
		if(bv[i].target == -1) {
			tx = Px;
			ty = Py;
//			tz = Alt + 500;
		}
		else {
			tx = ps[bv[i].target].x;
			ty = ps[bv[i].target].y;
//			tz = ps[bv[i].target].alt + 500;
		}
	}
	else {
		txi = psi->x;
		tyi = psi->y;
		tz = ps[prewing].alt;
/*		if(bv[i].target == -1) {
			tz = Alt;
		}
		else {
			tz = ps[bv[i].target].alt;
		}  */
		prespeed = bv[prewing].basespeed;
		if(secplace == 2) {
			tx = ps[prewing].x + sinX((ps[prewing].head + 0x4000), WALLXDIST);
			ty = ps[prewing].y - cosX((ps[prewing].head + 0x4000), WALLXDIST);
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * WALLXDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + 0x4000), ((long)AILOCDIV * WALLXDIST));
			tz -= WALLHDIST;
		}
		else if(secplace == 3) {
			tx = ps[prewing].x + sinX((ps[prewing].head - 0x4000), WALLXDIST);
			ty = ps[prewing].y - cosX((ps[prewing].head - 0x4000), WALLXDIST);
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head - 0x4000), ((long)AILOCDIV * WALLXDIST));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head - 0x4000), ((long)AILOCDIV * WALLXDIST));
			tz -= WALLHDIST;
		}
		else if(secplace == 4) {
			tx = ps[prewing].x + sinX((ps[prewing].head - 0x4000), (WALLXDIST * 2));
			ty = ps[prewing].y - cosX((ps[prewing].head - 0x4000), (WALLXDIST * 2));
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head - 0x4000), ((long)AILOCDIV * WALLXDIST * 2));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head - 0x4000), ((long)AILOCDIV * WALLXDIST * 2));
			tz += WALLHDIST<<1;
		}
		else if(secplace > 4){
			tsecplace = secplace - 4;
			dadd = 2 + tsecplace >> 1;
			if(tsecplace & 1) {
				dangle = 0x4000;
			}
			else {
				dangle = -0x4000;
			}
			tx = ps[prewing].x + sinX((ps[prewing].head + dangle), (WALLXDIST * dadd));
			ty = ps[prewing].y - cosX((ps[prewing].head + dangle), (WALLXDIST * dadd));
			lx = ailoc[prewing].xloc + sinXL((ps[prewing].head + dangle), ((long)AILOCDIV * WALLXDIST * dadd));
			ly = ailoc[prewing].yloc - cosXL((ps[prewing].head + dangle), ((long)AILOCDIV * WALLXDIST * dadd));
			while(tsecplace > 4)
				tsecplace = tsecplace >> 2;
			if((tsecplace == 2) || (tsecplace == 3)) {
				tz -= WALLHDIST;
			}
			else if(tsecplace == 4)
				tz += WALLHDIST<<1;
		}
		if((GLowDroll) && (!(ps[i].status & TAKEOFF))) {
			ps[i].status |= JUSTPUT;
			if(tz < 50)
				tz = 50;
			JustPutHere(i, lx, ly, tz, prewing);
			return;
		}

		LFlyTo(lx, ly, tz, prewing, prespeed);
		return;
	}

	if(tz < 50) {
		tz = psi->alt;
	}
	dx = tx - txi;
	dy = ty - tyi;
	tdist = Dist2D(dx, dy);
//	tpitch = GetAltPitch(psi->alt, tz, tdist);
	tpitch = GetAltPitch(psi->alt, tz, 50);
	tangle = angle(dx, -dy);
	tpitch = rng2(tpitch, -32<<8, 16<<8);
	distkm = tdist*32/825;
	tdistnm = (tdist * 2)/95;

	if((prewing == i) && (!(psi->status & TAKEOFF))) {
		if((tdistnm < 34) || ((CheckAnyEnemyLock(i)) && (tdistnm < 41))) {
			wingcnt = prewing;
			prewing2 = prewing;
			while (prewing2 >= 0) {
				while (wingcnt >= 0) {
						bv[wingcnt].Behavior = Wall2;
						wingcnt = bv[wingcnt].wingman;
				}
				prewing2 = bv[prewing2].nextsec;
				wingcnt = prewing2;
			}
	 	}
	}

	relangle = tangle - psi->head;

	troll = relangle - psi->roll;
	pdest = tpitch;

	TurnAIPlaneBy(i, relangle);

	bv[i].finhead = relangle + ps[i].head;

	if(prespeed) {
//		FormationSpeedAdjust(prespeed, relangle, tdist, prewing);
		bv[i].basespeed = prespeed - 10;
	}

#if 0
	if(((secplace <= 1) || (prewing == i) || (abs(dpwhead) > 0x600)) && (bv[i].behaveflag & ENEMYNOFIRE)) {
		if(prewing != i) {
			bv[i].groundt = bv[prewing].groundt;
			bv[i].xloc = bv[prewing].xloc;
			bv[i].yloc = bv[prewing].yloc;
		}
		if(bv[i].groundt != -1)
			FlyWayPoint();
	}
#endif

	if(prewing != i) {
		if(ps[i].alt > 75) {
			pdest = ps[prewing].pitch;
		}
	}
#ifdef ELFBMSG
	if(tempvar) {
		TXT("Wall ");
		TXN(i);
		TXA(" bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void FreeFighter() {
	int dx, dy, dz;
	int relangle, rrelangle;
	int tangle, tpitch, troll;
	int enemyhead, enemyalt;
	COORD tdist, tdistnm;

	if(!(ps[i].status & (FRIENDLY|NEUTRAL))) {
		StraightAttack();
		return;
	}

	bv[i].basespeed = ptype->speed;
	if(bv[i].target == -1) {
		dx = Px - psi->x;
		dy = Py - psi->y;
		enemyhead = OurHead;
	}
	else {
		dx = ps[bv[i].target].x - psi->x;
		dy = ps[bv[i].target].y - psi->y;
		enemyhead = ps[bv[i].target].head;
	}

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 1250);

	relangle = tangle - psi->head;
	rrelangle = (0x8000 + tangle) - enemyhead;
	pdest = tpitch;

	if(tdistnm < 7) {
		if(abs(rrelangle) < 0x4000) {
			relangle = (enemyhead - 0x8000) - psi->head;
		}
		else {
			if(rrelangle < 0) {
				relangle = relangle + 0x4000;
			}
			else {
				relangle = relangle - 0x4000;
			}
			if(tdistnm < 2) {
				if(rrelangle < 0) {
					relangle = relangle + 0x1000;
				}
				else {
					relangle = relangle - 0x1000;
				}
			}
		}
	}

	TurnAIPlaneBy(i, relangle);

#ifdef ELFBMSG
	if(tempvar) {
		TXT("FF At ");
		TXN(i);
		TXA(" bs ");
		TXN(bv[i].basespeed);
		TXA(", d ");
		TXN(tdist);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void Wall2()
{
	int secplace;
	int dx, dy;
	COORD tdist, tdistnm;
	int tpitch, tangle, enemyalt;
	int prewing, wingcnt;
	int winglead;

	secplace = (bv[i].behaveflag & 15);

	if(bv[i].target == -1) {
		dx = Px - psi->x;
		dy = Py - psi->y;
	}
	else {
		dx = ps[bv[i].target].x - psi->x;
		dy = ps[bv[i].target].y - psi->y;
	}
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	if((CheckDetection(i) != -999) && ((secplace == 0) || (secplace == 3))) {
		if(tdistnm < EVASIVEDIST) {
			winglead = bv[i].leader;
			if(winglead >= 0){
				winglead = CheckDetection(winglead);
			}
			else
				winglead = -999;
			if(winglead == -999) {
				PostHole();
				return;
			}
		}
	}

	if(secplace > 4) {
		if(secplace & 1)
			BracketRight();
		else
			BracketLeft();
	}
	else if(secplace <= 2) {
		BracketRight();
	}
	else {
		BracketLeft();
	}

	prewing = GetSquadLeader(i);
	if(tdistnm < 21)
	{
		wingcnt = prewing;
		while (prewing >= 0) {
			while (wingcnt >= 0) {
				bv[wingcnt].Behavior = FlyStraight;
				wingcnt = bv[wingcnt].wingman;
			}
			prewing = bv[prewing].nextsec;
			wingcnt = prewing;
		}
	}
}

void Ladder2()
{
	int secplace;
	int dx, dy;
	COORD tdist, tdistnm;
	int tpitch, tangle, enemyalt;
	int prewing, wingcnt, tempalt;
	int angleinc, angleevade;
	int designatedsec, rrangle, enemyhead;
	int rangle;
	int winglead;

	secplace = (bv[i].behaveflag & 15);
	bv[i].basespeed = ptype->speed;

	prewing = GetSquadLeader(i);
	designatedsec = FirstTargInGroup(prewing);
	if(designatedsec != -1)
		designatedsec = bv[designatedsec].behaveflag & 15;

	if(bv[i].target == -1) {
		dx = Px - psi->x;
		dy = Py - psi->y;
		enemyhead = OurHead;
	}
	else {
		dx = ps[bv[i].target].x - psi->x;
		dy = ps[bv[i].target].y - psi->y;
		enemyhead = ps[bv[i].target].head;
	}
	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

	if(designatedsec == i) {
		if(tdistnm < EVASIVEDIST) {
			winglead = bv[i].leader;
			if(winglead >= 0){
				winglead = CheckDetection(winglead);
			}
			else
				winglead = -999;
			if(winglead == -999) {
				PostHole();
				return;
			}
		}
		else {
			StraightAttack();
			return;
		}
	}

	if((secplace <= 1) || ((secplace == 3) && (designatedsec == 1))) {
		rrangle = (0x8000 + tangle) - enemyhead;
//		if(LOCKED != -1) {
		if(LOCKED == i) {
			rangle = (enemyhead + 0x6800) - psi->head;

			TurnAIPlaneBy(i, rangle);

		}
		else {
		 	if(rrangle < -0x2000)
		 		angleinc = -0x1000;
		 	else
		 		angleinc = -0x1800;
		 	angleevade = 0x4000;
			BaseBracket(angleinc, angleevade);
		}
		tempalt = enemyalt - 1000;
		if(tempalt < 50)
			tempalt = 50;
		if(ps[i].alt > tempalt) {
//			pdest = -0x2000 - psi->pitch;
			pdest = -0x2000;
		}
		else
			pdest = 0;
	}
	else if((secplace == 2) || ((secplace == 4) && (designatedsec == 2))) {
		angleinc = 0x3000;
		angleevade = +0x4000;
		BaseBracket(angleinc, angleevade);
		if(ps[i].alt < (enemyalt + 750)) {
//			pdest = 0x2000 - psi->pitch;
			pdest = 0x2000;
		}
		else
			pdest = 0;
	}
//	else if((LOCKED != -1) || (secplace != 4)){
	else if((LOCKED == i) || (secplace != 4)){
		angleinc = -0x3000;
		angleevade = 0x4000;
		BaseBracket(angleinc, angleevade);
	}
	else {
		StraightAttack();
	}

	if(!((secplace <= 1) || ((secplace == 3) && (designatedsec == 1)))) {
		if(tdistnm < 24)
		{
			wingcnt = prewing;
			while (prewing >= 0) {
				while (wingcnt >= 0) {
					secplace = (bv[wingcnt].behaveflag & 15);
					if(!((secplace <= 1) || ((secplace == 3) && (designatedsec == 1))))
					{
						bv[wingcnt].Behavior = FlyStraight;
					}
					wingcnt = bv[wingcnt].wingman;
				}
				prewing = bv[prewing].nextsec;
				wingcnt = prewing;
			}
		}
	}
	else {
		if(abs(tangle) > 0x5000) {
			bv[i].Behavior = FlyStraight;
		}
	}
}

void LadderCircle ()
{
	int txi, tyi;
	int dx, dy, dz;
	int relangle;
	int tangle, tpitch, troll, leadhead, isleader, tanglel, enemyalt;
	COORD tdist, distkm, tdistnm;
	int prewing, prespeed, secplace;
	int whofollow, tmpcnt, temphead;
	int prewing2, wingcnt;

	if (psi->status & PANIC) {
		bv[i].Behavior = StraightAttack;
		ChangeGroupBehavior(i, 0);
	}
	bv[i].varcnt = 0;
	prewing = GetSquadLeader(i);
	whofollow = i;
	if(bv[i].leader >= 0)
		whofollow = bv[i].leader;
	else if(bv[i].prevsec >= 0) {
		tmpcnt = bv[i].prevsec;
		while(tmpcnt >= 0) {
			whofollow = tmpcnt;
			tmpcnt = bv[tmpcnt].wingman;
		}
	}

	tdist = tangle = 0;
	bv[i].basespeed = ptype->speed / 2;

	if((prewing == i) || (whofollow == i)) {
		droll = 0x3000 - psi->roll;   /*  was 0x2800  */
		pdest = 0;
		if(bv[i].target == -1) {
			dx = Px - psi->x;
			dy = Py - psi->y;
		}
		else {
			dx = ps[bv[i].target].x - psi->x;
			dy = ps[bv[i].target].y - psi->y;
		}

		GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

		if((tdistnm > 30) && (abs(tangle - temphead) < 0x4000)) {
			ps[i].status |= BACKTOBASE;
			bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
			TXT("LadderCircle, Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(i);
			prewing2 = prewing;
			wingcnt = prewing2;
			while (prewing2 >= 0) {
				while (wingcnt >= 0) {
					if(wingcnt != prewing2)
						bv[wingcnt].Behavior = Formation;
					ps[wingcnt].status |= BACKTOBASE;
					bv[wingcnt].Behavior = AIGoHome;
					GetAIBase(wingcnt);
					wingcnt = bv[wingcnt].wingman;
				}
				prewing2 = bv[prewing2].nextsec;
				wingcnt = prewing2;
			}
		}
	}
	else {
		temphead = (ps[whofollow].head + 0x8000);
//		txi = ps[whofollow].x + sinX(temphead, 20);
//		tyi = ps[whofollow].y - cosX(temphead, 20);
		txi = ps[whofollow].x;
		tyi = ps[whofollow].y;

		dx = txi - psi->x;
		dy = tyi - psi->y;

		GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &tdistnm, &enemyalt, 0);

		tpitch = GetAltPitch(psi->alt, ps[whofollow].alt, 50);
		tpitch = rng2(tpitch, -32<<8, 16<<8);

		relangle = tangle - psi->head;

		troll = relangle - psi->roll;
		pdest = tpitch;

		if(tdistnm < 1)
			bv[i].basespeed -= 100;

		TurnAIPlaneBy(i, relangle);

/*		if(abs(relangle) > 0x2000) {
			if(relangle < 0)
				droll = 0xB000 - psi->roll;
			else
				droll = 0x5000 - psi->roll;
		}
		else {
			droll = 2 * (relangle - ps[whofollow].roll);
		}    I commented this out because I can't figure out why I
 			 used whofollow at the end of this.   SRE*/
	}
#ifdef ELFBMSG
	if(tempvar) {
		TXT("LC ");
		TXN(i);
		TXA(" wf ");
		TXN(whofollow);
		TXA(", d ");
		TXN(tdist);
		TXA(", ra ");
		TXN(relangle);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

int CheckDetection(int planenum)
{
	int cnt;
	int returnval = -999;
	COORD tdist, tdist1;
	int dx, dy, x, y;

	x = ps[planenum].x;
	y = ps[planenum].y;

	if((LOCKED == planenum) && (!FriendMatchType(planenum, FRIENDLY))) {
    	dx = Px-x;
    	dy = Py-y;
    	tdist = xydist(dx,dy);
		returnval = -1;
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
		if((bv[cnt].target == planenum) && (bv[cnt].behaveflag & ENEMYLOCK) && (!FriendMatch(planenum, cnt))) {
    	    if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status&CRASH)) && (ps[cnt].speed)) { /* Planes in the air */
       			tdist1 = xydist(ps[cnt].x-x, ps[cnt].y-y);
				if(returnval == -999) {
					if(tdist1 < tdist) {
						tdist = tdist1;
						returnval = cnt;
					}
				}
				else {
					tdist = tdist1;
					returnval = cnt;
				}
			}
		}
	}
	return(returnval);
}

int CheckAnyEnemyLock(int planenum)
{
	int cnt;

	return(0);

	if(!(ps[planenum].status & FRIENDLY)) {
		if(LOCKED != -1)
			return(1);
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
		if((!FriendMatch(cnt, planenum)) && (bv[cnt].behaveflag & ENEMYLOCK)) {
    	    if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status&CRASH)) && (ps[cnt].speed)) { /* Planes in the air */
				return(1);
			}
		}
	}
	return(0);
}

void ShowAIWeaponStatus(int ainum)
{
    char Str[80];
	int cnt;
    struct sam  *S;
	static struct PlaneData far *aiptype;

	aiptype = GetPlanePtr(ps[ainum].type);

	GetWingName(ainum, Str);
	TXT(Str);
	if(CheckAIMissiles(ainum) == -1) {
		TXA("Out of missiles!");
	}
	else {
		TXA(" ");
		for(cnt = 0; cnt < 3; cnt++) {
			if(ps[ainum].mcnt[cnt] == 0)
				continue;
			if(cnt > 0)
				TXA(", ");
			TXN(ps[ainum].mcnt[cnt]);
			if(aiptype->misl[cnt] == ML_AIM7M)
				TXA(" Sparrows");
			else if(aiptype->misl[cnt] == ML_AIM9M)
				TXA(" Sidewinders");
			else if(aiptype->misl[cnt] == ML_AIM120)
				TXA(" AMRAAMs");
			else if(aiptype->misl[cnt] == ML_AIM54C)
				TXA(" Phoenixs");
			else
				TXA(" Bombs");
		}
	}
	Message(Txt,WINGMAN_MSG);
}

void ShowAIStatus(int ainum, int otherai)
{
	int isdamaged;
    char Str[80];
    char Str2[80];
	int rtemp;

	isdamaged = 0;
	GetWingName(ainum, Str);
    TXT (Str);

	if(bv[ainum].behaveflag & DAMAGERADAR) {
    	TXA ("Radar ");
		isdamaged = 1;
	}
	if(bv[ainum].behaveflag & DAMAGEENGINE) {
		TXA ("Engine ");
		isdamaged = 1;
	}
	if(bv[ainum].behaveflag & DAMAGECONTROLSURFACE) {
		TXA ("Controls ");
		isdamaged = 1;
	}

	if(isdamaged != 0) {
		TXA ("Damaged!");
	}
	else {
		rtemp = (Rtime + otherai) & 3;
		if(rtemp == 0)
			TXA ("A.O.K");
		else if(rtemp == 1)
			TXA ("Doing Fine");
		else if(rtemp == 2)
			TXA ("Systems Nominal");
		else
			TXA ("Good");
	}
	Message(Txt,WINGMAN_MSG);
	ShowAIWeaponStatus(ainum);
}

void ShowMenuStuff(int MenuID)
{
	switch(MenuID) {
		case GETWINGBRACKET:
//		    MsgBox (316, 2, 2);
//    	    RpPrint (Rp3D, 2,     2, "1 Bracket Left, 2 Bracket Right, 3 High, 4 Low, 5 Straight");
    	    Message("1 Bracket Left, 2 Bracket Right, 3 High, 4 Low, 5 Straight", WING_MENU_MSG);
		break;

		case GETWINGFORMATION:
//		    MsgBox (316, 2, 2);
//    	    RpPrint (Rp3D, 2,     2, "1 Parade, 2 Cruise, 3 Combat Spread");
    	    Message("1 Parade, 2 Cruise, 3 Combat Spread", WING_MENU_MSG);
		break;

		case GETWINGSANITIZE:
//		    MsgBox (316, 2, 2);
//    	    RpPrint (Rp3D, 2,     2, "Sanitize 1 Left, 2 Right, 3 Front, 4 Back");
    	    Message("Sanitize 1 Left, 2 Right, 3 Front, 4 Back", WING_MENU_MSG);
		break;

		case GETNEGVSEPERATION:
//		    MsgBox (316, 2, 2);
//    	    RpPrint (Rp3D, 2,     2, "Negative 1 1000, 2 2000, ... , 9 9000, 0 10000 Ft Verticle Seperation");
    	    Message("Negative 1 1000, 2 2000, ... , 9 9000, 0 10000 Ft Verticle Seperation", WING_MENU_MSG);
		break;

		case GETPOSVSEPERATION:
//		    MsgBox (316, 2, 2);
//    	    RpPrint (Rp3D, 2,     2, "Positive 1 1000, 2 2000, ... , 9 9000, 0 10000 Ft Verticle Seperation");
    	    Message("Positive 1 1000, 2 2000, ... , 9 9000, 0 10000 Ft Verticle Seperation", WING_MENU_MSG);
		break;
	}
}

int ProcessAICommunications(int ainum)
{
	int otherai;
    char Str[80];
    char Str2[80];
	int htemp;

    Rp3D->APen   = WHITE;
    Rp3D->FontNr = F_3x5B;

/*	if(behavecommands != 0) {
		TXT("behavecommands ");
		TXN(behavecommands);
		Message(Txt,DEBUG_MSG);
	}  */
	if(bv[ainum].prevsec == -1) {
		otherai = 1;
	}
	else if(bv[ainum].leader >= 0) {
		if(bv[bv[ainum].leader].prevsec == -1) {
			otherai = 2;
		}
		else
			otherai = 0;
	}
	else
		otherai = 0;

	if(((Rtime & 31) == 0) && (GetSquadLeader(ainum) == -1))
		CheckAISix(ainum);

	GetWingName(ainum, Str);

    TXT (Str);


	if((behavecommands == OTHERWINGAI) && (otherai)) {   /*  SHIFT F8  */
		OtherAITime = Gtime + 4;
	}

	if(Gtime > AIWingMenuTime) {
		AIWingMenuTime = 0;
		AIWingMenu = 0;
	}

	if(((bv[ainum].leader == -1) && (Gtime > OtherAITime) && (!OtherAIDo))
			|| ((otherai) && ((OtherAIDo) || (Gtime <= OtherAITime)))){
		if((otherai) && (behavecommands != 0)) {
			if(behavecommands != OTHERWINGAI)   /*  SHIFT F8  */
				OtherAITime = 0;
			OtherAIDo = behavecommands;
		}
		if(Gtime < 20) {
			if((behavecommands == AIRETURNBASE) && (MyWingy < 3)) {
				MyWingy ++;
			}
			else if((behavecommands == OTHERWINGAI) && (MyWingy == 3)) {
				MyWingy ++;
			}
			else if((behavecommands == AISHOWSTATUS) && (MyWingy == 4)) {
				MyWingy = -999;
				InitAddedWingmen(-999);
			}
		}

//		ShowMenuStuff(AIWingMenu);

		if(behavecommands >= AIBRACLEFT) {
			return(ProcessAICommMenu(ainum));
		}

		switch (behavecommands) {

#if 0
			case AIARMWEAPONSRADAR:     /*   ALT F1 KEY  */
				if(bv[ainum].behaveflag & ENEMYNOFIRE) {  /*  RADAR and WEAPONS HOLD/RELEASE  */
					if(!(bv[ainum].behaveflag & DAMAGERADAR))
						bv[ainum].behaveflag |= RADARON;
					bv[ainum].behaveflag &= ~ENEMYNOFIRE;
					if(CheckAIMissiles(ainum) != -1) {
						TXA("Weapons Armed");
						Message(Txt,WINGMAN_MSG);
					}
					else {
						TXA("No Missiles",WINGMAN_MSG);
						Message(Txt);
					}
				}
				else {
					bv[ainum].behaveflag &= ~RADARON;
					bv[ainum].behaveflag |= ENEMYNOFIRE;
					TXA("Weapons Disarmed");
					Message(Txt,WINGMAN_MSG);
				}
			break;
#endif

			case AIATTACKCLOSE:   /*  ALT F1 KEY  */
				bv[ainum].Behavior = StraightAttack;   /*  ATTACK CLOSE  */
				if(!(bv[ainum].behaveflag & DAMAGERADAR))
					bv[ainum].behaveflag |= RADARON;
				bv[ainum].behaveflag &= ~ENEMYNOFIRE;
				bv[ainum].behaveflag &= ~NO_SWITCH_TARGET;
				GetEnemyTarget(ainum);
				TXA("Engaging ");
//				TXN(bv[ainum].target);
//				Message(Txt,WINGMAN_MSG);
				Speech(ROGER);
				GetSortInfo(ainum);
				ShowAIWeaponStatus(ainum);
				return(1);
			break;

			case AIATTACKTARGET:    /*  ALT F2 KEY  */
				if(DESIGNATED != -1) {                        /*  ATTACK MY TARGET  */
					if(ps[DESIGNATED].status & FRIENDLY) {
						TXA("But Sir, That's a Friendly");
						Message(Txt,WINGMAN_MSG);
					}
					else {
						TXA("Bogey Targeted.");
						Message(Txt,WINGMAN_MSG);
						Speech(ROGER);
						Speech(TARGETING);
						if(bv[ainum].behaveflag & ENEMYNOFIRE) {
							TXT(Str);
							TXA("Requesting Permission to Fire");
							Message(Txt,WINGMAN_MSG);
						}
						bv[ainum].behaveflag |= NO_SWITCH_TARGET;
						bv[ainum].target = DESIGNATED;
						ShowAIWeaponStatus(ainum);
						return(1);
					}
				}
			break;

			case AIFORMUP:   /*  ALT F3 KEY  */
				AIWingMenu = GETWINGFORMATION;
				AIWingMenuTime = Gtime + 5;
				if(OtherAIDo)
					OtherAITime = AIWingMenuTime;
				ShowMenuStuff(AIWingMenu);
			break;

			case AIBRACKET: 	 /*  ALT F4  */
				AIWingMenu = GETWINGBRACKET;
				AIWingMenuTime = Gtime + 5;
				if(OtherAIDo)
					OtherAITime = AIWingMenuTime;
				ShowMenuStuff(AIWingMenu);
			break;

			case AIHELPME:  /*  ALT F5  */
				Speech(ROGER);
				if(CheckWingmanSix(-1)) {
					TXA("I'm on him.");    /*  Wingmen help us  */
					Message(Txt,WINGMAN_MSG);
				}
				else if(NearWingTarget(-1)) {
					TXA("Here I come.");    /*  Wingmen help us  */
					Message(Txt,WINGMAN_MSG);
				}
			break;

			case AISANITIZE:  /*  ALT F6  */
				if(bv[ainum].behaveflag & DAMAGERADAR) {
					TXA("My radar's out");
					Message(Txt,WINGMAN_MSG);
				}
				else {
					bv[ainum].behavecnt = 0;
					AIWingMenu = GETWINGSANITIZE;
					AIWingMenuTime = Gtime + 5;
					ShowMenuStuff(AIWingMenu);
				}
				if(OtherAIDo)
					OtherAITime = AIWingMenuTime;
			break;

			case AISORTAI:  /*  ALT F7  */
					Speech(ROGER);
					TXA("Sorted ");    /*  Wingmen id targets  */
					GetSortInfo(ainum);
			break;

			case AISHOWSTATUS:   /*  ALT F8 KEY  */
				Speech(ROGER);
				ShowAIStatus(ainum, otherai);
			break;

			case AIREJOIN:		 /*  ALT F9 KEY  */
				bv[ainum].Behavior = Formation;
				bv[ainum].behaveflag &= ~RADARON;
				bv[ainum].behaveflag |= ENEMYNOFIRE;
				bv[ainum].verticlesep = 0;
				TXA("Rejoining Wing");
				Message(Txt,WINGMAN_MSG);
				Speech(ROGER);
				return(1);
			break;

			case AIRETURNBASE:   /*  ALT F10 KEY  */
				if(ps[ainum].status & BACKTOBASE) {  /*  UNDO RETURN TO BASE  */
					ps[ainum].status &= ~BACKTOBASE;
					bv[ainum].Behavior = Formation;
					TXA("Roger ");
					Message(Txt,WINGMAN_MSG);
					Speech(ROGER);
					return(0);
				}
				ps[ainum].status |= BACKTOBASE;
				bv[ainum].Behavior = AIGoHome;
				TXA("Returning To Base");
				Message(Txt,WINGMAN_MSG);
				Speech(ROGER);
//				Speech(RTB);
				return(1);
#if 0
				if((bv[ainum].behaveflag & (DAMAGERADAR|DAMAGEENGINE|DAMAGECONTROLSURFACE))  /*  RETURN TO BASE  */
						|| ((ps[ainum].mcnt[0] == 0) && (ps[ainum].mcnt[1] == 0))) {
					ps[ainum].status |= BACKTOBASE;
					bv[ainum].Behavior = AIGoHome;
					TXA("Returning To Base");
					Message(Txt,WINGMAN_MSG);
					return(1);
				}
				else {
					TXA("I'm Fine ");  /*  Plane Is FINE  */
					Message(Txt,WINGMAN_MSG);
/*					bv[ainum].Behavior = AICarrierLand;
					bv[ainum].threathead = 100;
					ps[ainum].status |= LANDING;  */
					ps[ainum].status |= BACKTOBASE;
					bv[ainum].Behavior = AIGoHome;
				}
#endif
			break;

#ifdef ELFREFUEL
			case AIDOREFUEL:     /*   SHIFT F5 KEY  */
				GetRefuelPlane();
				bv[ainum].fueltank = 10000;
				TXA("Going To Refuel");
				Message(Txt,WINGMAN_MSG);
			break;
#endif

		}
	}
	return(0);
}

int ProcessAICommMenu(int ainum)
{
	int otherai;
    char Str[80];
    char Str2[80];
	int htemp;

    Rp3D->APen   = WHITE;
    Rp3D->FontNr = F_3x5B;

	GetWingName(ainum, Str);

    TXT (Str);


	switch (behavecommands) {
		case AIBRACLEFT:
			bv[ainum].Behavior = BracketLeft;   /*  Wingmen Bracket  */
			if(!(bv[ainum].behaveflag & NO_SWITCH_TARGET)) {
				bv[ainum].target = -1;
				AIRadarForwardCheck(ainum, 0, 0);
			}
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].behaveflag &= ~ENEMYNOFIRE;
			if(bv[ainum].target == -1) {
				bv[ainum].finhead = OurHead;
			}

			TXA("Bracketing Left.");
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			AIWingMenu = 0;
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
		break;

		case AIBRACRIGHT:
			bv[ainum].Behavior = BracketRight;   /*  Wingmen Bracket  */
			if(!(bv[ainum].behaveflag & NO_SWITCH_TARGET)) {
				bv[ainum].target = -1;
				AIRadarForwardCheck(ainum, 0, 0);
			}
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].behaveflag &= ~ENEMYNOFIRE;
			if(bv[ainum].target == -1) {
				bv[ainum].finhead = OurHead;
			}

			TXA("Bracketing Right.");
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			AIWingMenu = 0;
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
		break;

		case AIFLYSTRAIGHT:
			bv[ainum].Behavior = FlyStraight;   /*  Wingmen Bracket  */
			if(!(bv[ainum].behaveflag & NO_SWITCH_TARGET)) {
				bv[ainum].target = -1;
				AIRadarForwardCheck(ainum, 0, 0);
			}
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].behaveflag &= ~ENEMYNOFIRE;
			if(bv[ainum].target == -1) {
				bv[ainum].finhead = OurHead;
			}

			TXA("Heading Forward.");
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			AIWingMenu = 0;
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
		break;

		case AIFLYLOW:
			AIWingMenu = GETNEGVSEPERATION;
			AIWingMenuTime = Gtime + 5;
			if(OtherAIDo)
				OtherAITime = AIWingMenuTime;
			ShowMenuStuff(AIWingMenu);
		break;

		case AIFLYHIGH:
			AIWingMenu = GETPOSVSEPERATION;
			AIWingMenuTime = Gtime + 5;
			if(OtherAIDo)
				OtherAITime = AIWingMenuTime;
			ShowMenuStuff(AIWingMenu);
		break;

		case AIPARADE:
			bv[ainum].Behavior = Formation;
			bv[ainum].behaveflag &= ~RADARON;
			bv[ainum].behaveflag |= ENEMYNOFIRE;
			bv[ainum].behaveflag &= ~COMBAT_SPREAD;
			bv[ainum].horizontalsep = PARADEDIST;
			TXA("Parade Formation");
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			AIWingMenu = 0;
			bv[ainum].verticlesep = 0;
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			return(1);
		break;

		case AICOMBATSPREAD:
			bv[ainum].Behavior = Formation;
			bv[ainum].behaveflag |= COMBAT_SPREAD;
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].behaveflag &= ~ENEMYNOFIRE;
			bv[ainum].horizontalsep = COMBATSPREADDIST;
			TXA("Combat Spread");
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			AIWingMenu = 0;
			ShowAIWeaponStatus(ainum);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			return(1);
		break;

		case AICRUISE:
			bv[ainum].Behavior = Formation;
			bv[ainum].behaveflag &= ~RADARON;
			bv[ainum].behaveflag |= ENEMYNOFIRE;
			bv[ainum].behaveflag &= ~COMBAT_SPREAD;
			bv[ainum].horizontalsep = CRUISEDIST;
			TXA("Cruise Formation.");
			AIWingMenu = 0;
			bv[ainum].verticlesep = 0;
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			return(1);
		break;

		case AISANITIZELEFT:
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].finhead = OurHead - 0x4000;
			bv[ainum].Behavior = FlySanitize;
			TXA("Sanitizing Left.");    /*  SANITIZE DIRECTION  */
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			AIWingMenu = 0;
		break;

		case AISANITIZERIGHT:
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].finhead = OurHead + 0x4000;
			bv[ainum].Behavior = FlySanitize;
			TXA("Sanitizing Right.");    /*  SANITIZE DIRECTION  */
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			AIWingMenu = 0;
		break;

		case AISANITIZEFRONT:
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].finhead = OurHead;
			bv[ainum].Behavior = FlySanitize;
			TXA("Sanitizing Front.");    /*  SANITIZE DIRECTION  */
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			AIWingMenu = 0;
		break;

		case AISANITIZEBACK:
			if(!(bv[ainum].behaveflag & DAMAGERADAR))
				bv[ainum].behaveflag |= RADARON;
			bv[ainum].finhead = OurHead + 0x8000;
			bv[ainum].Behavior = FlySanitize;
			TXA("Sanitizing Back.");    /*  SANITIZE DIRECTION  */
			Message(Txt,WINGMAN_MSG);
			Speech(ROGER);
			if(MyMSGTYP == WING_MENU_MSG) {
				if(msgtime)
					msgtime=GETTIME() - 60;   // End Menu Msg if still there
			}
			AIWingMenu = 0;
		break;
	}
	if(behavecommands >= AINEGVSEP1000) {
		DoVSeperation(ainum);
		if(MyMSGTYP == WING_MENU_MSG) {
			if(msgtime)
				msgtime=GETTIME() - 60;   // End Menu Msg if still there
		}
		AIWingMenu = 0;
	}
	return(0);
}

void DoVSeperation(int ainum)
{
	int htemp;

	if(behavecommands < 61) {
		bv[ainum].verticlesep = -250 * (behavecommands - 50);
		htemp = abs(bv[i].verticlesep)* 4;
		TXA("Neg. ");
	}
	else {
		bv[ainum].verticlesep = 250 * (behavecommands - 60);
		htemp = abs(bv[i].verticlesep)* 4;
		TXA("Pos. ");
	}
	TXN(htemp);
	TXA(" Ft. Verticle Seperation");
	Message(Txt,WINGMAN_MSG);
	Speech(ROGER);
}

void LowBombTarget()
{

	AIBombTarget();

	if(ps[i].alt < 95) {
		pdest = 0x2000 - psi->pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 125) {
//		pdest = 0xD000 - psi->pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

}

void AIGetBombDist()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	long xval, yval;
	UWORD zval;

	bv[i].basespeed = ptype->speed;

	if(bv[i].groundt == -1) {
		ps[i].status |= BACKTOBASE;
		bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
		TXT("No Bomb Target, Going Home ");
		TXN(i);
		Message(Txt,DEBUG_MSG);
#endif
		GetAIBase(i);
		return;
	}
	else {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		dx = (xval + 16) >> HTOV;
		dy = (yval + 16) >> HTOV;
		dx = dx - psi->x;
		dy = dy - psi->y;
	}

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	tangle += 0x8000;

	relangle = tangle - psi->head;

	if(distnm > 5) {
		bv[i].Behavior = LowBombTarget;
	}

	TurnAIPlaneBy(i, relangle);

	if(ps[i].alt < 95) {
//		pdest = 0x2000 - psi->pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > 125) {
//		pdest = 0xD000 - psi->pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if ELFBMSG
	if(tempvar) {
		TXT("GBD ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		TXA(", r ");
		TXN(relangle);
//		TXA(", x ");
//		TXN(aiwaypoints[bv[i].groundt].x);
//		TXA(", y ");
//		TXN(aiwaypoints[bv[i].groundt].y);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

void AIBombTarget()
{
	int dx, dy, dz;
	int relangle, rrelangle, enemyhead;
	int tangle, tpitch, troll, enemyalt;
	COORD tdist, distnm, distkm;
	int wingman;
	int bandit;
	int bombalt;
	int secplace;
	long xval, yval;
	UWORD zval;
	int coverplane;
	int makecap = 0;

	secplace = (bv[i].behaveflag & 15);

	bandit = CheckDetection(i);
	if(bandit != -999) {
		if(bandit == -1) {
    		dx = Px- ps[i].x;
    		dy = Py- ps[i].y;
    		tdist = xydist(dx,dy);
		}
		else {
			tdist = xydist(ps[bandit].x- ps[i].x, ps[bandit].y- ps[i].y);
		}
		if(tdist < 450) { /*  10 nm  */
			ShowJettison(i);
			if(CheckAIMissiles(i) != -1) {
				bv[i].Behavior = StraightAttack;
			}
			else {
				ps[i].status |= BACKTOBASE;
#if 0
				TXT("No Bomb Target2, Going Home ");
				TXN(i);
				Message(Txt,DEBUG_MSG);
#endif
				bv[i].Behavior = AIGoHome;
				GetAIBase(i);
			}
		}
	}

	bv[i].basespeed = ptype->speed / 2;

	if(bv[i].groundt == -1) {
		if(GetSquadLeader(i) != -1) {
			ps[i].status |= BACKTOBASE;
			bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
			TXT("No Bomb Target 3, Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(i);
		}
		return;
	}
	else {
		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);
		dx = ((long)xval + 16) >> HTOV;
		dy = ((long)yval + 16) >> HTOV;
		dx = dx - psi->x;
		dy = dy - psi->y;
	}

	GetTargADP(i, dx, dy, &tdist, &tpitch, &tangle, &distnm, &enemyalt, 0);

	relangle = tangle - psi->head;

	if((distnm > 10) && (distnm < 16)) {
		if(secplace < 3)
			relangle += 0x2000;
		else if(secplace < 5)
			relangle -= 0x2000;
	}

	if((distnm < 2) && (abs(relangle) > 0x1000)) {
		bv[i].Behavior = AIGetBombDist;
	}

	TurnAIPlaneBy(i, relangle);

	if(distnm < 20) {
		bv[i].basespeed = ptype->speed;
		wingman = bv[i].wingman;
		if(wingman >= 0) {
//			if(((bv[wingman].Behavior == Formation) || (bv[wingman].Behavior == GorillaBomb)) && (bv[wingman].xloc != bv[i].xloc)) {
			if(((CheckAIBombs(wingman) != -1)) && (!IsBombing(wingman)) && (bv[i].groundt == bv[wingman].groundt)) {
				bv[wingman].Behavior = AIBombTarget;
				bv[wingman].groundt = bv[i].groundt;
				bv[wingman].xloc = bv[i].xloc;
				bv[wingman].yloc = bv[i].yloc;
				makecap = 1;
			}
		}
		wingman = bv[i].nextsec;
		if(wingman >= 0) {
//			if(((bv[wingman].Behavior == Formation) || (bv[wingman].Behavior == GorillaBomb)) && (bv[wingman].xloc != bv[i].xloc)) {
//			if(((bv[wingman].Behavior == Formation)) && (!IsBombing(wingman)) && (bv[i].groundt == bv[wingman].groundt)) {
			if(((CheckAIBombs(wingman) != -1)) && (!IsBombing(wingman)) && (bv[i].groundt == bv[wingman].groundt)) {
				bv[wingman].Behavior = AIBombTarget;
				bv[wingman].groundt = bv[i].groundt;
				bv[wingman].xloc = bv[i].xloc;
				bv[wingman].yloc = bv[i].yloc;
				makecap = 1;
			}
		}
		coverplane = bv[i].target;
		if((coverplane >= 0) && (makecap)) {
			if(bv[coverplane].Behavior == GorillaCover) {
				bv[coverplane].Behavior = SetUpCAP1;
				bv[coverplane].xloc = ps[coverplane].xL;
				bv[coverplane].yloc = ps[coverplane].yL;
				bv[coverplane].groundt = -1;
				UpdateInterceptGroup(coverplane, 1);  /*  Might Need to do base here  */
			}
		}

		BOMB_TrackingAI();   /*  Moved Bombing stuff here because if before waypoint check above will be goofed  */
		DESIGNATED_TrackingAI(relangle);
	}
	bombalt = zval;
	if(ps[i].alt < bombalt) {
//		pdest = 0x2000 - psi->pitch;
		pdest = 0x2000;
	}
	else if(ps[i].alt > (bombalt + 10)) {
//		pdest = 0xD000 - psi->pitch;
		pdest = 0xD000;
	}
	else
		pdest = 0;

#if ELFBMSG
	if(tempvar) {
		TXT("BAI ");
		TXN(i);
		TXA(" d ");
		TXN(tdist);
		TXA(", r ");
		TXN(relangle);
//		TXA(", x ");
//		TXN(aiwaypoints[bv[i].groundt].x);
//		TXA(", y ");
//		TXN(aiwaypoints[bv[i].groundt].y);
		Message(Txt,DEBUG_MSG);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void BOMB_TrackingAI ()
{
	int  f;
	long  dist;
	int  dpitch;
	int  BombAngle;

/*	if(tempvar) {
		ps[i].x = Px;
		ps[i].y = Py;
	    ps[i].xL = (long) ps[i].x<<HTOV;
    	ps[i].yL = (long) ps[i].y<<HTOV;
		ps[i].pitch = OurPitch;
		ps[i].alt = Alt;
		ps[i].head = OurHead;
		ps[i].speed = AirSpeed / FPStoKNTS;
	}  */


	dpitch = abs(ps[i].pitch);

	f = 32767-Isin(dpitch);
	BombAngle = dpitch + ( (1500L + (ps[i].alt/8)) *TrgMul(f,f) >>15);

	dist = (long)(((long) ps[i].alt * Icos(BombAngle)) /Isin(BombAngle));

	AIBOMBX = ps[i].xL + sinXL (ps[i].head, dist);
	AIBOMBY = ps[i].yL - cosXL (ps[i].head, dist);
}


//////////////////////////////////////////////////////////////////////////
void DESIGNATED_TrackingAI (int relangle)
{
	long dx, dy;
	long  BOMBdist, DROPdist;
	int  TGTdist;
	int  angl;
	int  WentBy;
	static WasInFront=0;
	int tempspeed;
	int showpa;
	int behavenum, formnum, speedval, altval, headval, attack, launch, wpflag;

	tempspeed = ps[i].speed * FPStoKNTS;
//	dx = aiwaypoints[bv[i].groundt].x-AIBOMBX;
//	dy = aiwaypoints[bv[i].groundt].y-AIBOMBY;
	dx = bv[i].xloc-AIBOMBX;
	dy = bv[i].yloc-AIBOMBY;

	BOMBdist = LDist2D (dx, dy);

	angl = ps[i].head - Langle (dx, -dy);
	DROPdist = cosXL (angl, BOMBdist);
	WentBy = DROPdist<0L;
	if (WentBy) DROPdist = -DROPdist;

	AITREL = (long) DROPdist*2*4*16/(tempspeed+1); // 2x time to target intercept line in seconds
	showpa = 2;

	if(AITREL < 6) {
		if(abs(relangle) > 0x800) {
			bv[i].Behavior = LowBombTarget;
			return;
		}
		if(DropAIBomb()) {
			if(GetSquadLeader(i) == -1) {
				GetWingName(i, Txt);
				TXA("Bombs Away!");
				Message(Txt,WINGMAN_MSG);
			}
			GetWayPointChanges(bv[i].groundt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
			if(wpflag & DOWPGOAL) {
				LogIt (SLG_WAY_PT_GOAL, i, ps[i].xL, ps[i].yL, ps[i].alt, -(int)bv[i].path, bv[i].groundt, 0L, LOG_AIR);
//				DoGroupAction(i, DOWPGOAL, bv[i].groundt);
			}
			bv[i].Behavior = FlyWayPoint;
			bv[i].groundt ++;
			DoNextWayPt(i, bv[i].groundt);
			RestoreCover(i);
#if 0
			if(aiwaypoints[bv[i].groundt].actionIndex != -1) {
				TranslateBehavior(i, aiwaypoints[bv[i].groundt].actionIndex);
//				bv[i].groundt = -1;
//				ps[i].status |= BACKTOBASE;
//				if(bv[i].leader == -999)
//					bv[i].Behavior = StraightAttack;
//				else
//					bv[i].Behavior = Formation;
			}
			else {
				bv[i].Behavior = FlyWayPoint;
			}
#endif
#if 0
			else {
				bv[i].Behavior = AIBombTarget;
				bv[i].xloc = aiwaypoints[bv[i].groundt].x;
				bv[i].yloc = aiwaypoints[bv[i].groundt].y;
			}
#endif
		}
	}
}

int DropAIBomb()
{
	int type;
	int weaponnum;
	int aistation;
    struct Sams *ssk;
    struct sam  *S;
	int missilenum;
	long xval, yval;
	UWORD zval;

	aistation = CheckAIBombs(i);

	if(aistation == -1)
	{
		if(CheckAIMissiles(i) != -1) {
			bv[i].Behavior = StraightAttack;
			return(0);
		}
		if(GetSquadLeader(i) != -1) {
			ps[i].status |= BACKTOBASE;
			bv[i].Behavior = AIGoHome;
#ifdef SHOWHOME
			TXT("Out of Bombs, Going Home ");
			TXN(i);
			Message(Txt,DEBUG_MSG);
#endif
			GetAIBase(i);
		}
		bv[i].Behavior = Formation;
		return(0);
	}

	weaponnum = 1;
	if(ptype->misl[aistation] == ML_FFBOMB)
		weaponnum = 1;

    type = ORDNANCE[weaponnum].samtype;

	missilenum = FindEmptyMissileSlot2 (0, NPSAM, i);
    ssk=ss + missilenum;
	if(missilenum == -1) {
	    ssk=ss;
	}
	else {
	    ssk=ss + missilenum;
	}
    if ((ssk->dist==0) && (missilenum != -1)) {         // is missile slot available?
//		GetWayXYZ(bv[i].groundt, &xval, &yval, &zval);

		if(warstatus == 0) {
			if((ps[whichourcarr].status & ALIVE) && (ps[whichourcarr].status & ACTIVE)) {
				if((aiwaypoints[bv[i].groundt].x <= (long)-1) && (aiwaypoints[bv[i].groundt].y == (long)bv[whichourcarr].orgnum)) {
					warstatus = 1;
					LastCall = -4*60;
				}
			}
		}

		ps[i].mcnt[aistation] --;

		LogIt (SLG_AIR_BOMB_DROP, i, ps[i].xL, ps[i].yL, ps[i].alt, -999, ML_FFBOMB, 0L, LOG_AIR);

        firemissile2 (missilenum,
				0,     /* Offsets for drops from Modem guy?????? */
    			ps[i].xL,
       			ps[i].yL,
                ps[i].alt,
                ps[i].head,
                ps[i].pitch,
                ps[i].roll,
                (((ps[i].speed * FPStoKNTS)>>4)>>4) * 11,
                -1,    // used for air-air only !
                i,
                aiwaypoints[bv[i].groundt].x,
                aiwaypoints[bv[i].groundt].y,
				type,
				0);

#if 0
        ssk->x       = psi->xL;
        ssk->y       = psi->yL;
        ssk->alt     = psi->alt;
        ssk->head    = psi->head;
        ssk->pitch   = psi->pitch;
        ssk->roll    = psi->roll;
//        ssk->speed   = S->speed;           /* ACCEL THESE GUYS ?? */
        ssk->speed   = (((psi->speed * FPStoKNTS)>>4)>>4) * 11;           /* ACCEL THESE GUYS ?? */
        ssk->dist    = s[type].range * (825/4);
		ssk->weapon	 = weaponnum;
        ssk->type    = type;
        ssk->source  = i;
        ssk->srctype = 1;
        ssk->target  = 0;
        ssk->tgttype = 0;
        ssk->losing  = 0;
		ssk->tgtx	 = xval;
		ssk->tgty	 = yval;
#endif

		return(1);
	}
	return(0);
}

void TurnAIPlaneBy(int ainum, int relangle)
{

#if 0
	if(abs(relangle) > 0x2000) {
		if(relangle < 0)
			droll = 0xB000 - ps[ainum].roll;
		else
			droll = 0x5000 - ps[ainum].roll;
	}
	else {
		droll = relangle - ps[ainum].roll;
		droll = droll << 2;
	}
#endif

	droll = relangle - ps[ainum].roll;
	if(abs(droll) < 0x1000)
		droll = droll << 2;


#if 0
	if((tempvar) && (GetSquadLeader(ainum) == -1)) {
		sprintf(Txt, "i %d, ra %x, dr %x", ainum, relangle, droll);
		Message(Txt,DEBUG_MSG);
	}
#endif

}

void InitAddedWingmen (int planenum) {
	int cnt;
	int numwingmen;
	int wingspace;
	int lastplane;
	int lastsec;
	int otherplane = -999;
	int otherplane2 = -999;
	int wingnum;

	MyWingy = -999;

	if(planenum == -999)
		cnt = GetPlayerHelp();
	else
		cnt = planenum;

	if(cnt == -999)
		return;

	bv[cnt].prevsec = -1;
	WingID[1] = cnt;
	bv[cnt].behaveflag += 2;
	wingnum = bv[cnt].wingman;
	if(wingnum >= 0) {
		WingID[2] = wingnum;
		bv[wingnum].behaveflag += 2;
		otherplane2 = bv[wingnum].wingman;
		if(otherplane2 >= 0) {
			bv[wingnum].wingman = -999;
			bv[otherplane2].leader = -999;
			bv[otherplane2].Behavior = bv[cnt].Behavior;
		}
	}

	if(planenum == -999) {
		otherplane = bv[cnt].nextsec;
		if(otherplane >= 0) {
			bv[cnt].nextsec = -999;
			bv[otherplane].prevsec = -999;
			bv[otherplane].Behavior = bv[cnt].Behavior;
		}
	}
}

int CheckAIBombs(int ainum)
{
	int aistation;
//    struct sam  *S;
	struct MissileData far *S;
	struct PlaneData far *aiptype;

	aistation = -1;

	aiptype = GetPlanePtr(ps[ainum].type);
	S = GetMissilePtr(aiptype->misl[0]);
//    S=s+aiptype->misl[0];
	if((S->homing != IR_1) && (S->homing != IR_2)
			&& (S->homing != RDR_1) && (S->homing != RDR_2)
			&& (ps[ainum].mcnt[0]) > 0) {
		aistation = 0;
	}
	else {
		S = GetMissilePtr(aiptype->misl[1]);
		if((S->homing != IR_1) && (S->homing != IR_2)
				&& (S->homing != RDR_1) && (S->homing != RDR_2)
				&& (ps[ainum].mcnt[1]) > 0) {
			aistation = 1;
		}
		else {
			S = GetMissilePtr(aiptype->misl[2]);
			if((S->homing != IR_1) && (S->homing != IR_2)
					&& (S->homing != RDR_1) && (S->homing != RDR_2)
					&& (ps[ainum].mcnt[2]) > 0) {
				aistation = 2;
			}
		}
	}
	return(aistation);
}

int CheckAIMissiles(ainum)
{
	int aistation;
	struct PlaneData far *aiptype;
	struct MissileData far *S;
//  struct sam  *S;

	aistation = -1;

	aiptype = GetPlanePtr(ps[ainum].type);

	S = GetMissilePtr(aiptype->misl[0]);
	if(((S->homing == IR_1) || (S->homing == IR_2)
			|| (S->homing == RDR_1) || (S->homing == RDR_2))
			&& (ps[ainum].mcnt[0]) > 0) {
		aistation = 0;
	}
	else {
		S = GetMissilePtr(aiptype->misl[1]);
		if(((S->homing == IR_1) || (S->homing == IR_2)
				|| (S->homing == RDR_1) || (S->homing == RDR_2))
				&& (ps[ainum].mcnt[1]) > 0) {
			aistation = 1;
		}
		else {
			S = GetMissilePtr(aiptype->misl[2]);
			if(((S->homing == IR_1) || (S->homing == IR_2)
					|| (S->homing == RDR_1) || (S->homing == RDR_2))
					&& (ps[ainum].mcnt[2]) > 0) {
				aistation = 2;
			}
		}
	}
	return(aistation);
}

int NearWingTarget(int wingnum)
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1;

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

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
				(!(ps[cnt].status & (CRASH|CIVILIAN)))) { /* Planes in the air */
			if(!FriendMatch(i, cnt)) {
		    	dx = x- ps[cnt].x;
    			dy = y- ps[cnt].y;
				GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
				rangle = tangle - ps[cnt].head;
       			if ((tdist1<tdist) || ((tdistnm < 11) && (!founddist))) {
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
		bv[i].target = targetnum;
		bv[i].Behavior = FlyStraight;
		bv[i].behaveflag |= NO_SWITCH_TARGET;
		return(founddist);
	}

	bv[i].behaveflag &= ~NO_SWITCH_TARGET;
	return(0);
}

int AIRadarForwardCheck(int wingnum, int showinfo, int CAPrange)
{
	int otherrange;

	if(CAPrange)
		otherrange = 150;  /*  This is in KM 150 is about 85 nm...   was 40KM  */
	else
		otherrange = 0;

	return(AICoreRadarCheck(wingnum, showinfo, otherrange, 0x2000, 0));
}

int AIRadarRearCheck(int wingnum, int showinfo, int Bearrange)
{
	int otherrange;

	if(Bearrange)
		otherrange = 40;
	else
		otherrange = 0;

	return(AICoreRadarCheck(wingnum, showinfo, otherrange, 0, 0x5000));
}

int AICoreRadarCheck(int wingnum, int showinfo, int otherrange, int lessangle, int greaterangle)
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1, tangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1, distkm;
	int degreetmp;
	UWORD dangle;
	struct PlaneData far *wingptype;
	struct RadarData far *tempradar;

	int radarrangekm;

	if(wingnum == -1) {
		x = Px;
		y = Py;
		winghead = OurHead;
		radarrangekm = 150;
	} else {
		x = ps[wingnum].x;
		y = ps[wingnum].y;
		winghead = ps[wingnum].head;

		wingptype = GetPlanePtr(ps[wingnum].type);
		tempradar = GetRadarPtr(wingptype->rclass);
		radarrangekm = tempradar->range;
		/*  Could Put Diff Code here so CAP less effective  */
	}
	if((otherrange) && (radarrangekm > otherrange))
		radarrangekm = otherrange;  /*  intercept when enemy's in 60 nm  */

	if(!(bv[i].behaveflag & RADARON)) {
		radarrangekm = 10;
	}

	founddist = 0;
	targetnum = -999;
	tdist = 0;

	if(!(ps[wingnum].status & FRIENDLY)) {
    	dx = Px-x;
    	dy = Py-y;
		GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
		distkm = ((long)tdist1*32)/825;
		rangle = tangle - winghead;
		if((distkm < radarrangekm) && (((abs(rangle) < lessangle) || (!lessangle)) && ((abs(rangle) > greaterangle) || (!greaterangle)))){
			founddist = 1;
           	tdist=tdist1;
			targetnum = -1;
			rangle1 = rangle;
			rrangle1 = rrangle;
			tdistnm1 = tdistnm;
			tangle1 = tangle;
		}
	}


    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
				(ps[cnt].speed)) { /* Planes in the air */
			if(!FriendMatch(i, cnt)) {
		    	dx = ps[cnt].x - x;
    			dy = ps[cnt].y - y;
				GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
				distkm = (COORD)(((long)tdist1*32)/825);
				rangle = tangle - winghead;
       			if ((((tdist1<tdist) || ((distkm < radarrangekm) && (!founddist))) && (((abs(rangle) < lessangle) || (!lessangle)) && ((abs(rangle) > greaterangle) || (!greaterangle))))) {
					founddist = 1;
           			tdist=tdist1;
					targetnum = cnt;
					rangle1 = rangle;
					rrangle1 = rrangle;
					tdistnm1 = tdistnm;
					tangle1 = tangle;
       			}
			}
		}
	}
	if(founddist) {
		dangle = (UWORD)tangle1;
    	degreetmp = TrgMul (dangle>>1, 360);
		if(!(bv[wingnum].behaveflag & NO_SWITCH_TARGET)) {  /*  Check If we want to check for No switch  */
			bv[i].target = targetnum;
		}

		if(showinfo) {
			TXA("Bogeys ");
			TXN(degreetmp);
			TXA(", ");
			TXN(tdistnm1);
			Message(Txt,GAME_MSG);
		}
		return(founddist);
	}
	else {
		if(showinfo) {
			Speech(A_CLEAN);
			TXA("Clean");
			Message(Txt,GAME_MSG);
		}
	}

	return(0);
}


void GiveSanitizeMessage(int wingnum, int groupcheck[])
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1, tangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1;
	int degreetmp, groupcnt, groupalt;
	UWORD dangle;

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
	groupcnt = 0;

    for (cnt=0; cnt<NPLANES; cnt++) {
		if(groupcheck[cnt]) {
			groupcnt ++;
		    dx = ps[cnt].x - x;
    		dy = ps[cnt].y - y;
			GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
			rangle = tangle - winghead;
       		if ((((tdist1<tdist) || ((tdistnm < 80) && (!founddist))) && (abs(rangle) < 0x2000))) {
				founddist = 1;
           		tdist=tdist1;
				targetnum = cnt;
				rangle1 = rangle;
				rrangle1 = rrangle;
				tdistnm1 = tdistnm;
				tangle1 = tangle;
				groupalt = (ps[cnt].alt * 4) / 1000;
				if(groupalt < 1)
					groupalt = 1;
       		}
		}
	}
	if(founddist) {
		dangle = (UWORD)tangle1;
    	degreetmp = TrgMul (dangle>>1, 360);
		if(!(bv[wingnum].behaveflag & NO_SWITCH_TARGET)) {  /*  Check If we want to check for No switch  */
			bv[wingnum].target = targetnum;
		}

		Speech(CONTACT);
		NSpeech(groupcnt, 0);
//        BearingToText (dangle,0);
		Speech(TARGET_RANGE);
        if (tdistnm1>=160) {
            TXA ("160");
            Speech (C_160);
        } else if (tdistnm1>=120) {
            TXA ("120");
            Speech (C_120);
        } else if (tdistnm1>=80) {
            TXA ("80");
            Speech (C_80);
        } else if (tdistnm1>=40) {
            TXA ("40");
            Speech (C_40);
        } else {
            NSpeech (tdistnm1, 0);
        }
		Speech(ALTITUDE);
		NSpeech(groupalt,0);

		GetWingName(wingnum, Txt);
		if(groupcnt > 1)
			TXA("Contacts, ");
		else
			TXA("Contact, ");
		TXN(groupcnt);
		TXA(", ");
//		TXN(degreetmp);
//		TXA(", ");
		TXN(tdistnm1);
		TXA(", ");
		if(groupalt) {
			TXN(groupalt);
		}
		else {
			TXA("in the weeds");
		}
		Message(Txt,WINGMAN_MSG);
	}
	else {
		Speech(A_CLEAN);
		GetWingName(wingnum, Txt);
		TXA("Clean,");
		Message(Txt,WINGMAN_MSG);
	}
}

void AISanitizeGroupCheck(int wingnum, int enemynum, int planecheck[])
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle;
	int cnt, rangle1, rrangle1, tangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1;
	int degreetmp, cnt2;
	UWORD dangle;
	int groupcheck[MAXPLANE];

	for(cnt = 0; cnt < MAXPLANE; cnt++)
		groupcheck[cnt] = 0;

	groupcheck[enemynum] = 1;

	founddist = 1;
	targetnum = -999;

	while(founddist != 0) {
		founddist = 0;
		for(cnt2=0; cnt2<NPLANES; cnt2++) {
			if(groupcheck[cnt2]) {
				if(cnt2 == -1) {  /*  IF WE WANT ENEMY TO SANITIZE WILL HAVE TO ADD PLAYER INTO CHECK  */
					x = Px;			  /*  WE DON'T YET, PROBABLY WILL  */
					y = Py;
				}
				else {
					x = ps[cnt2].x;
					y = ps[cnt2].y;
				}

    			for (cnt=0; cnt<NPLANES; cnt++) {
        			if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
							(ps[cnt].speed) && (groupcheck[cnt] == 0)) { /* Planes in the air */
		    			dx = ps[cnt].x - x;
    					dy = ps[cnt].y - y;
						GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
       					if (tdistnm < 10) {
							groupcheck[cnt] = 1;
							planecheck[cnt] = 1;
							founddist = 1;
       					}
					}
				}
			}
		}
	}
	GiveSanitizeMessage(wingnum, groupcheck);
}

void AISanitizeCheck(int wingnum)
{
	int returnval, x, y, dx, dy, founddist, targetnum;
	int tpitch, tangle, enemyalt, rangle, rrangle, winghead;
	int cnt, rangle1, rrangle1, tangle1;
	COORD tdist, tdist1, tdistnm, tdistnm1;
	int degreetmp;
	UWORD dangle;
	int planecheck[MAXPLANE];

	for(cnt = 0; cnt < MAXPLANE; cnt++)
		planecheck[cnt] = 0;

	if(wingnum == -1) {  /*  IF WE WANT ENEMY TO SANITIZE WILL HAVE TO ADD PLAYER INTO CHECK  */
		x = Px;			 /*  WE DON'T YET, PROBABLY WILL  */
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

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) &&
				(ps[cnt].speed) && (planecheck[cnt] == 0)) { /* Planes in the air */
			if(!FriendMatch(wingnum, cnt)) {
		    	dx = ps[cnt].x - x;
    			dy = ps[cnt].y - y;
				GetTargADP(i, dx, dy, &tdist1, &tpitch, &tangle, &tdistnm, &enemyalt, 0);
				rangle = tangle - winghead;
       			if ((tdistnm < 80) && (abs(rangle) < 0x2000)) {
					planecheck[cnt] = 1;
					AISanitizeGroupCheck(wingnum, cnt, planecheck);
					founddist = 1;
       			}
			}
		}
	}
	if(founddist == 0) {
		Speech(A_CLEAN);
		GetWingName(i, Txt);
		TXA("Clean");
		Message(Txt,WINGMAN_MSG);
	}
}

void FlySanitize()
{
	int relangle;
    char Str[80];

	if((Knots+ GetDAltSpeed(ps[i].alt)) > 300)
		bv[i].basespeed = (Knots + GetDAltSpeed(ps[i].alt)) - 100;
	else
		bv[i].basespeed = 200 + GetDAltSpeed(ps[i].alt);

	pdest = 0;
	relangle = bv[i].finhead - ps[i].head;
	TurnAIPlaneBy(i, relangle);

	if((abs(relangle) < 0x500) && (abs(droll < 0x500))) {
		if(bv[i].behavecnt == 0) {
			bv[i].behavecnt = Gtime + ((MAXWINGLEVEL - bv[i].winglevel) * 5);
		}
		else if(bv[i].behavecnt < Gtime) {
			AISanitizeCheck(i);
			bv[i].Behavior = Formation;
			if(bv[i].behaveflag & ENEMYNOFIRE)
				bv[i].behaveflag &= ~RADARON;
		}
	}
}

AIFoxCall (weapon)
{
    switch (weapon) {
        case ML_AIM7M:
			TXA ("Fox 1!");  Speech (FOX1);         break;
        case ML_AIM9M:
			TXA ("Fox 2!");  Speech (FOX2);         break;
        case ML_AIM120:
			TXA ("Fox 3!");  Speech (FOX3);         break;
        case ML_AIM54C:
			TXA ("Fox 3!");  Speech (FOX3);         break;
    }
    Message(Txt,WINGMAN_MSG);
}

void GetWingName(int ainum, char *Str)
{
	strcpy (Str, OurWingName);

	if(WingID[0] == ainum) {
		return;
	}
	if(WingID[1] == ainum) {
		strcpy(Str,"(Elf)");
	}
	else if(WingID[2] == ainum) {
		strcpy(Str,"(Bigmac)");
	}
}

void ChangeGroupBehavior(int ainum, int matchenemyfire)
{
	int prewing;
	int wingcnt;
	int radaron = 0;
	int enemynofire = 0;
	int combatspread = 0;
	int pitchup = 0;

	prewing = GetSquadLeader(ainum);
	if(bv[ainum].behaveflag & RADARON)
		radaron = 1;
	if(bv[ainum].behaveflag & ENEMYNOFIRE)
		enemynofire = 1;
	if(bv[ainum].behaveflag & COMBAT_SPREAD)
		combatspread = 1;
	if(ps[ainum].status & PITCHUP)
		pitchup = 1;

	wingcnt = prewing;
	while (prewing >= 0) {
		while (wingcnt >= 0) {
			if((!matchenemyfire) || ((bv[wingcnt].behaveflag & ENEMYNOFIRE) == (bv[ainum].behaveflag & ENEMYNOFIRE))) {
				bv[wingcnt].Behavior = bv[ainum].Behavior;

				if(radaron)
					bv[wingcnt].behaveflag |= RADARON;
				else
					bv[wingcnt].behaveflag &= ~RADARON;
				if(enemynofire)
					bv[wingcnt].behaveflag |= ENEMYNOFIRE;
				else
					bv[wingcnt].behaveflag &= ~ENEMYNOFIRE;
				if(combatspread)
					bv[wingcnt].behaveflag |= COMBAT_SPREAD;
				else
					bv[wingcnt].behaveflag &= ~COMBAT_SPREAD;

				if(pitchup)
					ps[wingcnt].status |= PITCHUP;

				bv[wingcnt].horizontalsep = bv[ainum].horizontalsep;
				if(!(ps[wingcnt].status & TAKEOFF))
					bv[wingcnt].groundt = bv[ainum].groundt;
			}

			wingcnt = bv[wingcnt].wingman;
		}
		prewing = bv[prewing].nextsec;
		wingcnt = prewing;
	}
}

int AddSomePlanes2 (int planenum, int planeid, int AIBehavior, int formnum, int numadd, int numwingadd, int isfriend, int whattype, long xval, long yval, int altval, int headval, int path, int speedval, int howgood)
{
	int cnt;
	int numwingmen;
	int wingspace;
	int lastplane;
	int lastsec;
	int returnval = -999;
	int startval, endval;
	int imaboat;
	int incval;
	int notdone = 1;
	struct PlaneData far *tempplane;

	numwingmen = 0;
    wingspace = (Direction==1 ? 5:-5);

	tempplane = GetPlanePtr(planeid);

	if(IsBoat(planeid)) {
		startval = MAXPLANE - 1;
		endval = NPLANES;
		imaboat = 1;
		incval = -1;
	}
	else {
		startval = 0;
		endval = LASTPLANE;
		imaboat = 0;
		incval = 1;
	}

	for(cnt = startval; notdone != 0; cnt += incval) {  /*  Had been cnt = 1 for a while  */
		if(imaboat) {
			if(cnt <= endval) {
				notdone = 0;
			}
		}
		else {
			if(cnt >= endval) {
				notdone = 0;
			}
		}

    	if ((notdone) && !((ps[cnt].status&ACTIVE) && (ps[cnt].status & ALIVE))) { /* Planes in the air */
    		ps[cnt].status = ALIVE+ACTIVE;
			if(tempplane->bclass == CIVILIAN_OBJCLASS)
    			ps[cnt].status |= CIVILIAN;
			if((cnt >= NPLANES) && (!imaboat))
				NPLANES = cnt + 1;
			if((cnt <= LASTPLANE) && (imaboat))
				LASTPLANE = cnt;

			if(isfriend == SS_FRIENDLY) {
				ps[cnt].status |= FRIENDLY;
				ps[cnt].status &= ~NEUTRAL;
			}
			else if(isfriend == SS_NEUTRAL) {
				ps[cnt].status |= NEUTRAL;
				ps[cnt].status &= ~FRIENDLY;
			}

    		ActivatePlane (cnt, ms[0].base, planeid);
			if(whattype == ST_DAMAGED) {
				ps[cnt].mcnt[0] = 0;
				ps[cnt].mcnt[1] = 0;
			}

			if(tempplane->bclass ==  HELICOPTER_OBJCLASS) {
				bv[cnt].behaveflag |= ISHELICOPTER;
			}

			if(altval > 6)
        		ps[cnt].alt = altval;
			else
				altval = 6;
			ps[cnt].head = headval;
			bv[cnt].threathead = headval;
			if(speedval > 0) {
	    		ps[cnt].speed = speedval;
				bv[cnt].basespeed = speedval;
			}
			else {
	    		ps[cnt].speed = 0;
				bv[cnt].basespeed = 0;
			}

			if(whattype == ST_WINGMAN) {
				if(numwingmen < 3)
					WingID[numwingmen] = cnt;
				AIWingMan = cnt;
				bv[cnt].leader = -1;
				bv[cnt].behaveflag += 2 + numwingmen;
				if(ps[cnt].alt < 25) {
					ps[cnt].alt = 25;
				}
				if(bv[cnt].basespeed < 200) {
					bv[cnt].basespeed = 200;
					ps[cnt].speed = 200;
				}
			}
			else
				bv[cnt].behaveflag += 1 + numwingmen;

			if(altval != 0)
				ps[cnt].status &= ~TAKEOFF;

			TranslateBehavior(cnt, AIBehavior, formnum);
			if(isfriend == SS_NEUTRAL)
				bv[cnt].behaveflag |= ENEMYNOFIRE;
			bv[cnt].path = path;
			if(path != -1) {
				bv[cnt].groundt = aipaths[path].wp0;
			}
			else {
				bv[cnt].groundt = -1;
			}

//			DoNextWayPt(cnt, bv[cnt].groundt);

			bv[cnt].xloc = Rdrs[ms[0].base].x;
    		bv[cnt].yloc = Rdrs[ms[0].base].y;
			ps[cnt].xL = xval;
			ps[cnt].x = xval>>HTOV;
			ps[cnt].yL = yval;
			ps[cnt].y = yval>>HTOV;
			bv[cnt].orgnum = (char)planenum;
			GetAIBase(cnt);
			bv[cnt].winglevel = howgood;

			if(whattype == ST_READY_5)
				bv[cnt].Behavior = Ready5;

			if(AIBehavior != SA_CRUISE_MISS) {
				if(numwingadd > 0)
					bv[cnt].wingman = 0;
				DoNextWayPt(cnt, bv[cnt].groundt);
				bv[cnt].wingman = -999;
			}

#ifdef SEEIFWENEEDTHIS
			if(tempplane->PlaneEnumId == SO_KHAWKA) {
				whichourcarr = cnt;     // holds value for which boat our carrier
				wps[0].xL = xval;
				wps[0].yL = yval;
				wps[0].x = xval>>HTOV;
				wps[0].y = yval>>HTOV;
				UpdateOurCarrier(cnt);
			}
#endif

			if(returnval == -999)
				returnval = cnt;

			if(whattype == ST_READY_5) {
				bv[cnt].Behavior = Ready5;
				SetUpLaunch(cnt, bv[cnt].groundt);
//				ps[cnt].status &= ~TAKEOFF;
			}

			if(numwingadd > 0)
				SetUpWingman(cnt, ps[cnt].base, numwingadd, planeid);
		/*			if(AIBehavior < B_CAP) {
				bv[cnt].Behavior = FlyWayPoint;
			}  */
			numwingmen ++;
			if(numwingmen > (numadd - 1)) {
				break;
			}
		}
	}

	return(returnval);
}

#define MAXHELOPITCH 0x1200
void UpdateHelicopterHPR_SandP () {
    int a;
    long spd, Gspd;
	long dx, dy;
	long spd2;
	long speed2;
	long tspd;
	long precval;
	long precval2;
    int n;
	int fdroll;
	int temppitch;
	int temppitch2;
	int dpitchrange;
	long temppitchl;
	long temppitchl2;


/* Update ROLL and HEADING */
	fdroll = droll*4/FrameRate;
   	psi->roll += fdroll;
    psi->head += (psi->roll>>3)/FrameRate;

    dpitch = pdest;  /*  In this case pitch will show speed so */
					 /*  pdest will be used instead of pitch to change alt  */

	if((abs(dpitch) > 0x200) || (abs(dpitch) > 0x200) || (!IsCAPFlier(i) && (!(bv[i].behaveflag & ENEMYNOFIRE))))
		ps[i].status &= ~LOWDROLL;
	else
		ps[i].status |= LOWDROLL;

	/* Update Speed and Position */
	if(((ailoc[i].xloc / AILOCDIV) != ps[i].xL) || ((ailoc[i].xloc / AILOCDIV) != ps[i].xL)) {
		ailoc[i].xloc = ps[i].xL * AILOCDIV;
		ailoc[i].yloc = ps[i].yL * AILOCDIV;
	}

	if(bv[i].behaveflag & DAMAGEENGINE) {
		if(bv[i].basespeed > (ptype->speed / 2))
			bv[i].basespeed = ptype->speed / 2;
	}

    if (!(psi->status&(TAKEOFF|LANDING|LANDED))) {
//        spd = ((long)ptype->speed*(0x8000-(psi->pitch/2))) >>15;
//        spd = ((long)bv[i].basespeed*(0x8000-(psi->pitch/2))) >>15;

		temppitchl2 = -((long)ps[i].pitch * 1000);
		temppitchl = temppitchl2 / MAXHELOPITCH;
//		sprintf(Txt, "tpl %ld, tpl2 %ld, p %d", temppitchl, temppitchl2, ps[i].pitch);
        spd = ((long)ptype->speed*temppitchl)/1000;

		if(spd < 0L)
			spd = 0L;

		if(abs(psi->roll) > 0xA00)
	        spd -= labs( sinXL(psi->roll,spd)) >>1;

        if (REALFLIGHT) {
            a = rng(psi->alt, 5000/4, 30000/4) - 5000/4;
            spd -= ((long)a * spd) / (20*2500L);
        }

		if(spd < 0L)
			spd = 0L;

        psi->speed = (int)spd;
		if(psi->speed == 0)
			psi->speed = 1;
    }

//    spd = (psi->speed*27 >>6) /FrameRate;

    if (psi->alt>2500) {     /* Too high, level off */
		if(psi->pitch > 0)
	        dpitch = 0;
    }

/* Trail smoke if CRASHing */
    if (psi->status & CRASH) {
//        dpitch=-2<<8;
		dpitch = 0;
		psi->head += 0x1000;
		psi->alt -= 40;
    }

#if 0
    Gspd = cosX (dpitch,spd);

    psi->xL += sinX(psi->head,Gspd);
    psi->yL -= cosX(psi->head,Gspd);
    psi->alt += sinX(dpitch,spd);

    psi->x = (psi->xL+16) >> HTOV;
    psi->y = (psi->yL+16) >> HTOV;
#endif
	if(Speedy == 1) {
	    tspd = (((long)psi->speed*27 >>6) * Fticks * AILOCDIV) /60;
		if(tspd >= 0)
			spd = tspd;
		else
			spd = 0L;
    	speed2 = (((long)psi->speed*27 >>6) * Fticks) /60;
	    Gspd = cosXL (dpitch, spd);
		if(Gspd < 0L)
			Gspd = 0L;
    	ailoc[i].xloc += sinXL(psi->head,Gspd);
    	ailoc[i].yloc -= cosXL(psi->head,Gspd);
		ps[i].xL = ailoc[i].xloc / AILOCDIV;
		ps[i].yL = ailoc[i].yloc / AILOCDIV;
		if(bv[i].basespeed > 3)
		    psi->alt += sinXL(dpitch,(spd/AILOCDIV));
		else {
	    	psi->alt += sinXL(dpitch,(spd/AILOCDIV)) + sinX(dpitch, 7);
		}
	}
	else {
    	speed2 = (((long)psi->speed*27 >>6) * Fticks) /60;
		if(speed2 < 0L)
			speed2 = 0L;
	    Gspd = cosXL (dpitch, speed2);
		if(Gspd < 0L)
			Gspd = 0L;
	    psi->xL += sinXL(psi->head,Gspd);
    	psi->yL -= cosXL(psi->head,Gspd);
		if(bv[i].basespeed > 3)
	    	psi->alt += sinXL(dpitch, speed2);
		else {
	    	psi->alt += sinXL(dpitch, speed2) + sinX(dpitch, 7);
		}
	}

	if(GetSquadLeader(i) != -1) {
		ailoc[i].xloc = ((long)ps[i].xL * AILOCDIV) + (AILOCDIV>>1);
		ailoc[i].yloc = ((long)ps[i].yL * AILOCDIV) + (AILOCDIV>>1);
	}
	else {
		precval = FPARAMS.X / AILOCDIV;
		precval = FPARAMS.X - (precval * AILOCDIV);
		ailoc[i].xloc = ((long)ps[i].xL * AILOCDIV) + precval;
		precval2 = FPARAMS.Z / AILOCDIV;
		precval2 = FPARAMS.Z - (precval2 * AILOCDIV);
		ailoc[i].yloc = ((long)ps[i].yL * AILOCDIV) + precval2;
		lfpx = FPARAMS.X;
		lfpy = FPARAMS.Z;
		lox = ailoc[i].xloc;
		loy = ailoc[i].yloc;
	}

    psi->x = (psi->xL+16) >> HTOV;
    psi->y = (psi->yL+16) >> HTOV;


/* Update PITCH */

	temppitchl = -(((long)psi->speed * 1000) / ptype->speed);
	temppitchl = (temppitchl * MAXHELOPITCH) / 1000;
	temppitchl2 = -(((long)bv[i].basespeed * 1000) / ptype->speed);
	temppitchl2 = (temppitchl2 * MAXHELOPITCH) / 1000;
	temppitch = (temppitchl2 - temppitchl);
	dpitchrange = 0x1000/FrameRate;
    temppitch = rng2 (temppitch, -dpitchrange, dpitchrange);
    psi->pitch += temppitch;
    psi->pitch = rng2 (psi->pitch, -MAXHELOPITCH, 0x800);

    if (abs(psi->pitch)>0x4000) {
        psi->head += 0x8000;
        psi->roll += 0x8000;
        psi->pitch = 0x8000 - psi->pitch;
    }

    if (psi->status & CRASH) {
		StartASmoke( psi->xL, psi->alt, psi->yL, SMKTRAIL, 5 );
    }

	if (psi->alt<0) {  /* Too low, hit the ground */
        psi->status &= PERMANENTBITS;

		if(CloseEnoughFor(SNDSTUFF, psi->x, psi->y))
        	Sound (N_FarExplo, 2, 0);
		StartASmoke( psi->xL, psi->alt,psi->yL, GRND2XPLO, 17 );

        if (atarg==i) atarg=-1;
    }
}


void UpdateI(int newi)
{
	i = newi;
	psi = ps + i;
}

int ImNotaFighter(int planenum, int checkguns)
{
	struct PlaneData far *tempplane;

	tempplane = GetPlanePtr(ps[planenum].type);

	if((tempplane->bclass == FIGHTER_OBJCLASS) || (tempplane->bclass == HELICOPTER_OBJCLASS))
		return(0);

	if(tempplane->bclass == BOMBER_OBJCLASS) {
		if((CheckAIMissiles(planenum) != -1) && (CheckAIBombs(planenum)== -1))
			return(0);
	}

	if(!checkguns)
		return(1);

	if(tempplane->gun != NO_GUN)
		return(1);

	return(0);
}

