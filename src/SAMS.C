/*  File:   Sams.c                                */
/*  Author: Sid Meier                             */
/*                                                */
/*  Game logic for Stealth: all weapons           */
/*  (Air-Air, Surface-Air, Air-Ground)            */

#include "Library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "sounds.h"
#include "F15defs.h"
#include "awg9.h"
#include "armt.h"
#include "stdlib.h"
#include "speech.h"
#include <string.h>
#include "event.h"
#include "proto.h"
#include "planeai.h"
#include "setup.h"

extern	int		 RIO_ASSIST;

#define  DEG      182         // 1 degree
#define  NM15     712         // 15 NM (15 BIT)
#define  NM10     475         // 10 NM (15 BIT)
#define  NM05     238         // 05 NM (15 BIT)
#define  NM01     48          // 01 NM (15 BIT)

void CheckProximityHit ();

int LastMisl;

struct CloseObj *TargetObj;
extern struct CloseObj *CheckTargetHit();
extern struct RastPort *Rp1, RpX1;

extern long     PxLng;
extern long     PyLng;
extern COORD    Px,Py;
extern int      OurHead,OurRoll,OurPitch;
extern unsigned AirSpeed;
extern int      sx,sy,sz;
extern unsigned Alt;
extern int      PovMisl;
extern UWORD    LastSlotNMount;
extern int      DMAX9;
extern int      Rtime;
extern int      IneffectiveHit;
extern int      CrtDisplays[];
extern int      CrtConfig[3][8];
extern int      DAMAGELITE[];
extern int      MASTER_CAUTION;
extern int      JAMMER[];

/* Detection variables */
extern  int     detected, atarg,gtarg,ImHit;
extern  COORD   xlast[],ylast[],chuteX,chuteY;
extern  int     zlast[],chuteZ,chuteDZ;
extern  int     vScale;
extern  int     CloseBase;
extern	int		LastCall;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);
extern struct PlaneData far *ptype;

extern int DesigTargCnt;
extern DTARGET DESIG_TARG[];
extern confData ConfigData;


// srctype: 0-ground
//   source: Rdrs list entry #

// srctype: 1-air
//   source: MAXPLANE - us, 0=WingMan(if any), otherwise ps array entry

// tgttype: 0-ground
//   target: tgtx, tgty are set

// tgttype: 1-air
//   target: MAXPLANE - us, 0=WingMan(if any), otherwise ps array entry


/* ---------------------------------------------------------*/
/* Process all enemy missiles (SAM and AAM) and our weapons */
/* ---------------------------------------------------------*/

static int homedist,homeangle, HomeHead, pangle;
static COORD xd,yd,dist,d;
static int homer,homing,zd,angl,stype;
static struct Sams *ssi;
// static struct sam *sst;
static struct MissileData far *sst;
static int EnemySam;
static int TargetsHit;
static int PrintedMiss;

//***************************************************************************
dosams()
{
    int i;

    TargetsHit  = 0;
    PrintedMiss = 0;

    for (i=0, ssi=ss; i<NSAM; i++, ssi++) {
        if (ssi->dist) {
            stype = ssi->type;
			sst = GetMissilePtr(ssi->type);
            homer = sst->homing;
            homing = FALSE;
            angl = FALSE;
            EnemySam = i<NESAM;
            WeaponHoming();
            SteerAndMoveMissile();
            CheckGroundHit();
            CheckProximityHit();
		}
	}
}


//***************************************************************************
// Sets dist, angl, homing
WeaponHoming()
{
	int j, x, y;

	if (ssi->tgttype) {				// target is a plane...
		All2AirHoming ();
	} else {						// ground target
		Air2GroundHoming ();
	}

	if (!homing) {
		ssi->losing++;
	} else {
		ssi->losing=0;
	}
}

//***************************************************************************
// Sets dist, angl, homing, pangle, zd
All2AirHoming()
{
	int j;

	j = ssi->target;

	if (j==-1) {                     	// no target designated
		homing = 0;
	} else {
		switch (j) {
			case MAXPLANE:     			// PLAYER IS TARGET...
				zd = Alt;
				HomeHead = OurHead;
				homing = home (PxLng, 0x100000-PyLng, Alt, homer);

				if((RIO_ASSIST == RIO_ASSIST_STANDARD) && (lastCounter < Gtime)) {
					if(homing) {
						if(((homer == IR_1) || (homer == IR_2)) && (homedist < 50)) {
							lastCounter = Gtime + 3;
							Message("(RIO) Dropping FLARE", RIO_MSG);
							dropdecoy(FLARE, MAXPLANE);
						}
						else if(((homer == RDR_1) || (homer == RDR_2)) && (homedist < 100)){
							lastCounter = Gtime + 3;
							Message("(RIO) Dropping CHAFF", RIO_MSG);
							dropdecoy(CHAFF, MAXPLANE);
						}
					}
				}
			break;
			default:             		// PLANE IN PLANE ARRAY IS TARGET...
				zd = ps[j].alt;
				HomeHead = ps[j].head;
				homing = home(ps[j].xL, ps[j].yL, ps[j].alt, homer);

				// Check to see if missile is a phoenix or is using a
				// designated track to find target

				if ((ssi->srctype==1) && (ssi->source==MAXPLANE) && (ssi->weapon==W_AIM_54) &&
					((CheckMode(AWG9_MODE,AWG9_TWSA)) || (CheckMode(AWG9_MODE,AWG9_TWSM)))) {
						if (homing && (homedist<(2<<BT2KM))) {
							ps[j].status |= PANIC;
						}
					} else {
						if (homing && (homedist<(6<<BT2KM))) {
							ps[j].status |= PANIC;
						}
					}
			break;
		}

		dist = homedist;
		angl = homeangle;
	}
//   if (EnemySam) decoyed ();

	decoyed();
}

//***************************************************************************
/* Sets dist, angl, homing, pangl, zd */
Air2GroundHoming()
{
	int boatnum;

	// Since there are NO LGBOMBs for us, I forced this to false
	if (ssi->type==ML_LGBOMB) {
		homing = false;
		return 0;
	}
	zd = 0;
	HomeHead = -1;
	if(ssi->tgtx <= (long)-1) {
		boatnum = GetPlaneOrgNum((int)ssi->tgty);
		if(ps[boatnum].status & CRASH) {
			ssi->tgtx = ps[boatnum].xL;
			ssi->tgty = ps[boatnum].yL;
			homing = home (ssi->tgtx, ssi->tgty,0, homer);
		}
		else
			homing = home (ps[boatnum].xL, ps[boatnum].yL,0, homer);
	}
	else
		homing = home (ssi->tgtx, ssi->tgty,0, homer);
	dist = homedist;
	angl = homeangle;
}

//***************************************************************************
static decoyed()
{
	int j,h,a;

	for (j=0; j<NDECOY; j++) {
		switch (homer) {
			case IR_1:
			case IR_2:
				if (ds[j].type!=FLARE) continue;
			break;
			case RDR_1:
			case RDR_2:
				if (ds[j].type!=CHAFF) continue;
			break;
			default:
				continue;
		}

		// can missile home on decoy?

		if (!home ((long)ds[j].x<<HTOV, (long)ds[j].y<<HTOV,zd, homer))
			continue;

		if (homedist > NM05) continue;   // inside max decoy range?
		if ((int)(dist-homedist) < -NM01)continue;   // decoy behind plane?

		a = abs(angl)+(sst->cmeffect*DEG);
		if (a<0) a=0;                                // adjust for cm effect
		if (abs(homeangle) >= a) continue;   // plane is easier to see?

		angl = homeangle;
		dist = homedist;
		homing = FALSE;
	}
}

//***************************************************************************
// Check if missile 'ssi' can home on location x,y,z
// Sets: homedist, homeangle, pangle
// Uses: HomeHead
static home(long x, long y, int z, int homer)
{
	long dx, dy;
	int  angl;

	dx = x-ssi->x;
	dy = y-ssi->y;

	if (labs(dx)>=23000L || labs(dy)>=23000L) {
		dx = dx>>HTOV;
		dy = dy>>HTOV;
		homedist = Dist2D((int)dx, (int)dy);
		if (homer==ML_AGM65) z = 75;                     // 300 feet cruise alt
		pangle = angle((z-ssi->alt)>>HTOV, homedist);
	} else {
		homedist = Dist2D ((int)dx, (int)dy);
		if (homedist>1520 && homer==ML_AGM65) z = 75;    // 300 feet cruise alt
		pangle = angle (z-ssi->alt, homedist);
		homedist >>= HTOV;
	}

	homeangle = angle((int)dx, (int)-dy);

	if(ssi->srctype == 1) {
		if((ssi->source < MAXPLANE) && (ssi->source >= 0)) {
			if(IsBoat(ps[ssi->source].type)) {
				return TRUE;
			}
		}
	}

	if (homer!=ML_FFBOMB && homer!=ML_RTBOMB && homer!=ML_LGBOMB && homer!=ML_AGM65
		&& homedist >= (ssi->speed*3 >>7) /FrameRate) { // (*3/8 /32 )*2 Terminal homing, last 4 frames

		// check if within homing cone...

		angl = abs(homeangle-ssi->head);

		if (angl > 0x1000) {
			return FALSE;
		}

		// check individual homer requirements

		switch (homer) {
			case IR_1:                    // TAIL ASPECT IR
			case RDR_2:                   // ACTIVE RADAR
			break;

			case RDR_1:                   // SEMI-ACTIVE RADAR
				if (!tracking())
					return FALSE;

			case IR_2:                    // FULL ASPECT IR
				if (homedist <= NM15 && homedist >= NM05) {
					angl = abs(ssi->head-HomeHead);
					angl = angl > 0x4000 ? 0x8000 - angl : angl;
					if (angl > 0x3800) return FALSE;
				}
			break;

			default:
				return FALSE;
		}
	}
	return TRUE;
}

//***************************************************************************
static tracking()
{
	// This use of Rdrs may cause a problem
	if (ssi->srctype == 0) {
		if (!(Rdrs[ssi->source].status&RTRACK)) return FALSE;
	} else if (ssi->source == MAXPLANE) {
		if (LOCKED != ssi->target) return FALSE;
	} else {
		if (!(ps[ssi->source].status&ALERTED)) return FALSE;
	}
	return TRUE;
}

//***************************************************************************
CheckIncomingWarning()
{

	if (homing && EnemySam) {
//      if (homer<=IR && (Rtime&2)!=0)  SetLight (IRWLight, LRED);
//      if (homer!=IR && (Rtime&2)==0)  SetLight (RWRLight, YELLOW);

		if (!(Rtime&3) && dist<(ssi->speed>>1) ) {
//			Sound(WARN3SND, 1);
        }
	}
}




//***************************************************************************
// Uses homing, angl, pangle, dist, zd
SteerAndMoveMissile()
{
	int dpitch, spd, turn;
	int dx, dy, dz, d;
	int lastspeed;
	long lx, ly;
	UWORD lz;
	int srclevel = 1;

	/* Steer the missile */

	if (homing) {
		turn = angl-ssi->head;
        if (EnemySam) {
			if(ssi->srctype == 1) {
				srclevel = bv[ssi->source].winglevel - 1;
				if(srclevel >= 4)
					srclevel = 3;
				else if(srclevel < 0)
					srclevel = 0;
				turn = rng2(turn, -(srclevel+1)<<8,(srclevel+1)<<8);
			}
			else {
				turn = rng2(turn, -(Diff+1)<<8,(Diff+1)<<8);
			}
		}
        turn = rng2 (turn, -(sst->maneuver<<7),sst->maneuver<<7);
        ssi->head += turn;
        ssi->roll = turn<<1;

        dpitch = pangle - ssi->pitch;   /* pangle calculated in "home" */
        dpitch = rng2 (dpitch, -(sst->maneuver<<11),sst->maneuver<<9);
        ssi->pitch += dpitch;
    } else {
        if (ssi->pitch>0) ssi->pitch -= (sgn(ssi->pitch)<<11)/FrameRate;
    }

#if 0
    if (ssi->speed < sst->speed) {
       ssi->speed += (sst->speed-ssi->speed + (4*FrameRate-1))/(4*FrameRate);
    } else if (ssi->speed > sst->speed) {
       ssi->speed += (sst->speed-ssi->speed - (4*FrameRate-1))/(4*FrameRate);
    }
#else
	if(ssi->srctype) {
		if(ssi->source == MAXPLANE) {
			lx = PxLng;
			ly = (0x100000 - PyLng);
			lz = Alt;
		}
		else {
			lx = ps[ssi->source].xL;
			ly = ps[ssi->source].yL;
			lz = ps[ssi->source].alt;
		}
	}

	if ((ssi->tgttype) && (ssi->speed < 75)) {
		ssi->speed += 20/FrameRate;
	}
	else if ((ssi->tgttype) && (ssi->speed < 175)) {
		ssi->speed += 50/FrameRate;
	}
	else if ((ssi->tgttype) && (ssi->srctype) && (labs(ssi->x - lx) < 720) && (labs(ssi->y - ly) < 720) && (abs(ssi->alt - lz) < 720)) {
		ssi->speed += 50/FrameRate;
	}
    if (ssi->speed < sst->speed) {
//		lastspeed = ssi->speed;
       ssi->speed += (sst->speed-ssi->speed + (4*FrameRate-1))/(FrameRate<<2);
/*	   if(ssi->speed <= lastspeed) {
			TXT("At MAX SPEED ");
			TXN(ssi->speed);
			TXA(" ");
			TXN(sst->speed);
			Message(Txt,DEBUG_MSG);
	   }  */
    } else if (ssi->speed > sst->speed) {
       ssi->speed += (sst->speed-ssi->speed - (4*FrameRate-1))/(FrameRate<<2);
    }
#endif

	/* Update weapon position */
	//
	// Miles     5280 ft   1 hour     1 bit
	// -----  x  ------- x ------   x -----  ==> 3/8 * MPH = Bits/Sec;
	// Hour      1 mile    3600 secs  4 feet
	//
    spd = ( (ssi->speed*3 >>3) +(FrameRate>>1) )/FrameRate;  // airspeed

//    if (ssi->dist > spd>>2) {
//        ssi->dist -= spd>>2;
    if (ssi->dist > spd>>3) {
        ssi->dist -= spd>>3;
    } else {
        ssi->dist = 0;
    }

    dz  = sinX(ssi->pitch, spd);
    spd = cosX(ssi->pitch, spd);           // ground speed
    dx  = sinX(ssi->head,spd);
    dy  = cosX(ssi->head,spd);

    if (ssi->alt < -dz) {
		if(dz == 0)
			dz = 1;
        d = (ssi->alt*32768L) /-dz;
        dx = TrgMul (dx, d);               /* interpolate for Z intercept */
        dy = TrgMul (dy, d);
        dz = -ssi->alt;
    }

    ssi->alt += dz;
    ssi->x   += dx;
    ssi->y   -= dy;

	if ( (ssi->srctype==1)&&(ssi->source==MAXPLANE) )
	{
		if ( AnimIsRunning(ssi->station)==0 )
         {
         dx=ASinB(-50,ssi->head);
         dy=-ACosB(-50,ssi->head);
			StartASmoke( ssi->x+(long)dx, ssi->alt, ssi->y+(long)dy, EXHAUST, 15 );
			StartASmoke( ssi->x+(long)((dx<<1)), ssi->alt, ssi->y+(long)((dy<<1)), EXHAUST, 6 );
         }

	}
	else
	{
		if(homer!=ML_FFBOMB)
         {
         dx=ASinB(-100,ssi->head);
         dy=-ACosB(-100,ssi->head);
			StartASmoke( ssi->x+(long)dx, ssi->alt, ssi->y+(long)dy, EXHAUST, 15 );
         }
	}
//	StartASmoke( ssi->x, ssi->alt, ssi->y, EXHAUST, 15 );
}

//***************************************************************************
CheckGroundHit ()
{
    int targ, dx, dy, LastTgt, ObjNr;
	int NTargetHit;

    if ( ssi->alt <= (GetQHeight(ssi->x, ssi->y) >> 2)) {
        ssi->dist=0;

		if(ssi->weapon == -1)
			return(0);

		switch (ssi->weapon)
			{
			case W_CBU_87:
			case W_CBU_89:
				StartASmoke( ssi->x, ssi->alt, ssi->y, CLUSTERXPLO, 17 );
				break;
			}

/* Did we hit a ground object ? */
        if (ssi->tgttype!=0) {
			if(!EnemySam) {
            	Message("Ground Impact",GAME_MSG);
			}
        } else {
			LogIt (SLG_GROUND_BOMB_HIT, -999, ssi->x, ssi->y, 1, ssi->source, ssi->type, 0L, LOG_GRND);

			CheckPlaneBlast((int)(ssi->x>>HTOV), (int)(ssi->y>>HTOV), 1, 25, -999, ssi->source, ssi->type);
			NTargetHit = CheckBlastArea(ssi->x, ssi->y, ssi->weapon, -999);
            if (NTargetHit==0 &&!PrintedMiss) {
                PrintedMiss=1;
				if(!EnemySam) {
    	            Message(IneffectiveHit?"Ineffective Hit": "Target missed",GAME_MSG);
				}
				if(CloseEnoughFor(SNDSTUFF, (int)(ssi->x>>HTOV), (int)(ssi->y>>HTOV)))
                	Sound (N_FarExplo, 2,0);
//       	        StartASmoke( ssi->x, ssi->alt, ssi->y, AIR2XPLO, 17 );
            }
        }
    }
}



//***************************************************************************
void CheckProximityHit()
{
	int n;
	int killed;
	int InProximity;
	int src;
	int crashstat;
	int gtnum;
	struct PlaneData far *tempplane;
	struct MissileData far *tempmissile;

    if (ssi->tgttype==0) return;   // don't do for air-to-ground
    if (ssi->target==-1) return;   // never had a target

    n = dist + (abs(zd-ssi->alt) >>HTOV); // dist in 15-bit scale

    InProximity = (n < ((ssi->speed*3 >>8) +FrameRate-1) /FrameRate); // (*3/8 /32 )

    if (homing) {
        if (!InProximity) return;
    } else {
        if (ssi->losing < FrameRate*5) return;
    }

	// Homing missile blows up

    ssi->dist = 0;

    if (!InProximity) return;

	// Now blow up the target

    killed = ssi->target;
	if(ssi->srctype == 0) {
		src = GetRadarGTNum(ssi->source);
		if(src != -999)
			src = -src;
//		src = -999;
	}
	else {
		src = ssi->source;
		if(src == MAXPLANE)
			src == -1;
	}

    if (killed == MAXPLANE) {
		tempmissile = GetMissilePtr(ssi->type);
		DamageMe(tempmissile->damage);
        Txt[0]=0;
		LogIt (SLG_AIR_MISSILE_HIT, -1, PxLng, (0x100000-PyLng), Alt, src, ssi->type, 0L, LOG_AIR);
	    StartASmoke (PxLng, Alt, 0x100000-PyLng, AIRXPLO, 10);

    } else {
		if(!(ps[killed].status & (ALIVE|ACTIVE)))
			return;

		if(ps[killed].status & CRASH) {
			ps[killed].status = 0;
			if(CloseEnoughFor(SNDSTUFF, ps[killed].x, ps[killed].y))
	    		Sound (N_FarExplo, 2,0);
			StartASmoke( ps[killed].xL, ps[killed].alt,ps[killed].yL, AIRXPLO, 10 );
			return;
		}
		if((!(DOINVISIBLE||ENEMYNODIE)) && (bv[killed].Behavior != EngageTrain)) {
			if((bv[killed].leader == -1) || (Rtime & 0x1)) {
				crashstat = ps[killed].status & CRASH;
				DamageAI(killed);
				if(crashstat != (ps[killed].status & CRASH))
					LogIt (SLG_AIR_DESTROYED_MISSILE, killed, ps[killed].xL, ps[killed].yL, ps[killed].alt, src, ssi->type, 0L, LOG_AIR);
				else
					LogIt (SLG_AIR_MISSILE_HIT, killed, ps[killed].xL, ps[killed].yL, ps[killed].alt, src, ssi->type, 0L, LOG_AIR);
			}
			else {
        		AirKill (killed);
				LogIt (SLG_AIR_MISSILE_HIT, killed, ps[killed].xL, ps[killed].yL, ps[killed].alt, src, ssi->type, 0L, LOG_AIR);
				LogIt (SLG_AIR_DESTROYED_MISSILE, killed, ps[killed].xL, ps[killed].yL, ps[killed].alt, src, ssi->type, 0L, LOG_AIR);
			}
		}
		else {
			StartASmoke( ps[killed].xL, ps[killed].alt,ps[killed].yL, AIRXPLO, 10 );
			LogIt (SLG_AIR_MISSILE_HIT, killed, ps[killed].xL, ps[killed].yL, ps[killed].alt, src, ssi->type, 0L, LOG_AIR);
			tempplane = GetPlanePtr(ps[killed].type);
		    FTXT (tempplane->name);
			TXA (" ");
		}
    }

	if(CloseEnoughFor(SNDSTUFF, ps[killed].x, ps[killed].y))
	    Sound (N_FarExplo, 2,0);

    TXA ("Hit by ");
	FTXA (sst->name);
    Message(Txt,GAME_MSG);

}

//***************************************************************************
// Enemy plane destroyed
AirKill (int killed)
{
    int LogWho;
	int arghtmp;
	char Str[80];
	int sarnum;
	struct PlaneData far *tempplane;

    if (!(ps[killed].status&CRASH)) {

		IndirectWPCorrection(killed);

        ps[killed].status |= CRASH;

		if((bv[killed].Behavior != FlyCruise) && (killed < NPLANES)) {
	        chuteX = ps[killed].x;
    	    chuteY = ps[killed].y;
        	chuteZ = ps[killed].alt;
        	chuteDZ = 128;
		}
		tempplane = GetPlanePtr(ps[killed].type);

		// Check and Make sure this is RIGHT!!!!
		LogWho = tempplane->PlaneEnumId;

//        LogWho = ps[killed].type;
        if (FriendMatchType(killed, FRIENDLY)) LogWho |= FRIENDKILL;
/*
#if OLDLOGDATA
        LogIt (PLANEHIT, LogWho);
#else
		LogIt(E_DESTROY, killed, 0, ps[killed].xL, ps[killed].yL);
#endif  */

        if (ps[killed].speed==0) ps[killed].status &= PERMANENTBITS;

		if((tempplane->bclass != CIVILIAN_OBJCLASS) && (tempplane->bclass != CARRIER_OBJCLASS)
				&& (tempplane->bclass != SHIP_OBJCLASS) && (tempplane->bclass != CMISSILE_OBJCLASS)
				&& (tempplane->bclass != GROUND_TARGETCLASS))
			GetSAR(killed);
    }

	tempplane = GetPlanePtr(ps[killed].type);
    FTXT (tempplane->name);
	TXA (" ");

	if(CloseEnoughFor(SNDSTUFF, ps[killed].x, ps[killed].y))
    	Sound (N_FarExplo, 2,0);
	StartASmoke( ps[killed].xL, ps[killed].alt,ps[killed].yL, AIRXPLO, 10 );
	if(GetSquadLeader(killed) == -1) {
		Speech(IM_HIT);
		arghtmp = Rtime & 3;
		GetWingName(killed, Str);
		if(arghtmp == 0)
			strcat(Str, "ARRRRRRRGGGGHHHHHHH!!!!!");
		else if(arghtmp == 1)
			strcat(Str, "EJECT...EJECT...EJECT!!!!!");
		else if(arghtmp == 2)
			strcat(Str, "BAILING OUT!!!!!");
		else if(arghtmp == 3)
			strcat(Str, "GOING IN!!!!!");
		Message(Str,WINGMAN_MSG);
	}

}

//***************************************************************************
DamageMe(int DamageType)
{
    int i, x, x2, NrHits;

//    Sound (N_LoudExplo, 2,0);
    Sound (N_WeGetHitExplo, 2,0);
	if((Status&TRAINING) && ENEMYNODIE && (DamageType > 9) && KnockItOffStatus)
		Speech(L_YOURE_A_MORT);
	else
		Speech(WERE_HIT);

	// We are in training - Do not damage me
    if (Status&TRAINING) return 0;

	// Determine amount of damage

	switch(DamageType) {
		case 0:		// No Damage
			NrHits = 0;
		break;
		case 1:		// Light Guns
			NrHits = 3;
		break;
		case 2:		// Med Guns
			NrHits = 5;
		break;
		case 3:		// Heavy Guns
			NrHits = 8;
		break;
		case 4:		// Real Heavy Guns
			NrHits = 10;
		break;
		case 10:	// Small warhead
			NrHits = 6;
		break;
		case 11:	// Med warhead
			NrHits = 12;
		break;
		case 12:	// large warhead
			NrHits = 15;
		break;
		case 13:	// very large warhead
			NrHits = 20;
		break;
		case 14:	// huge warhead
			NrHits = 25;
		break;
		case 15:	// very large warhead
			NrHits = 30;
		break;
		default:	// I'm not sure what hit us, but let's take some damage anyways
			NrHits = 3;
		break;
	}

	if (ConfigData.Damage==0) {
		if (NrHits==0)
			NrHits=0;
		else
			NrHits=3;
	}

    for (i=0; i<NrHits; i++) {
        x = rnd (16);
        x2 = 1<<x;
        if (!(Damaged&x2)) DamageIt(x2);
        Damaged |= x2;

        DAMAGELITE[x]  = 1;
        MASTER_CAUTION = 1;
    }
    ImHit = TRUE;
}


//***************************************************************************
DamageIt(x)
{
    switch (x) {
        case D_LEFTENGINE:
			Message("(RIO) Left Engine is on Fire!",RIO_MSG);
        break;

        case D_RIGHTENGINE:
			Message("(RIO) Right Engine is on Fire!",RIO_MSG);
		break;

		case D_HYDRAULICS:
			Message("(RIO) Hydraulics are damaged",RIO_MSG);
		break;


		case D_AFTERBURNER:
			Message("(RIO) Afterburner is damaged",RIO_MSG);
		break;

		case D_LFUELSYSTEM:
			Message("(RIO) Left fuel system is damaged",RIO_MSG);
		break;

		case D_RFUELSYSTEM:
			Message("(RIO) Right fuel system is damaged",RIO_MSG);
		break;

		case D_FIRECONTROL:
			Message("(RIO) AWG-15 (Weapon Control) is damaged",RIO_MSG);
		break;

		case D_AWG9:
			Message("(RIO) AWG-9 (Radar System) is damaged",RIO_MSG);
		break;

		case D_HUD:
			Message("(RIO) HUD damaged",RIO_MSG);
		break;

		case D_WINGSWEEP:
			Message("(RIO) Wing Sweep is damaged",RIO_MSG);
		break;

		case D_ECM:
			Message("(RIO) ECM systems are damaged",RIO_MSG);
		break;

		case D_GEAR:
			Message("(RIO) Landing Gear is damaged",RIO_MSG);
		break;

		case D_NAV:
			Message("(RIO) Navigation systems are damaged",RIO_MSG);
		break;
    }

    switch (x) {
        case D_NAV:
			AutoAlt = 0;
		break;

        case D_ECM:
			JAMMER[0] = 0;
		break;
	}
}

//***************************************************************************
firemissile()
{
	LAUNCHPOS *lp;
    int     i,k,type,weapon,cnt;
    long    TgtX, TgtY;
	struct MissileData far *tempmissile;


    if (ejected || (Damaged&D_FIRECONTROL)) return 0;

	// Check to see if Radar Mode == TWS and currently in Phoenix

	if (CheckMode(AWG9_MODE,AWG9_TWSA) || CheckMode(AWG9_MODE,AWG9_TWSM)) {
		if (AAGetPriority() != W_AIM_54)
			return(0);
	}

    cnt =0;

    while ((lp = ARMT_release ()) != NULL) {
        weapon = lp->weapon;
        type   = ORDNANCE[weapon].samtype;

		LogIt (SLG_AIR_MISSILE_LAUNCH, -1, PxLng, (0x100000 - PyLng), Alt, -999, type, 0L, LOG_AIR);



        k = firemissile2 (-1,
							weapon,
                            PxLng,
                            (0x100000 - PyLng),
                            Alt,
                            OurHead,
                            OurPitch,
                            OurRoll,
                            ((AirSpeed>>4)>>4) * 11,
                            SHOOT ? LOCKED : -1,    // used for air-air only !
                            MAXPLANE,
                            TgtX,
                            TgtY,
							-1,
							-1);

        if (k != -1) {
            PovMisl = k;
            if (ORDNANCE[weapon].wtype&W_AAW) LastMisl = k;
			tempmissile = GetMissilePtr(type);
            Sound(tempmissile->range ? N_LoudMissile:N_DropBomb, 2,0); /* CHANGE THIS !!! */
        }
    }
}

//***************************************************************************
firemissile2(int misnum, int weapon, long x,long y,int z, int h, int p, int r, int spd,
              int gtarg, int source, long TgtX, long TgtY, int stype, int targettype)
{
    int k, type,tcnt;
	struct MissileData far *tempmissile;
	long ldist;

	if(misnum == -1) {
	    k = (source!=MAXPLANE) ? FindEmptyMissileSlot2 (0, NESAM, source):
    	                                     FindEmptyMissileSlot (NESAM, NSAM);
	}
	else {
		k = misnum;
	}

    if (k!=-1) {

		if(stype == -1)
        	type = ORDNANCE[weapon].samtype;
		else
			type = stype;

		tempmissile = GetMissilePtr(type);

        ss[k].x        = x;
        ss[k].y        = y;
        ss[k].alt      = z;
        ss[k].speed    = spd;      // was * 11 to convert FPS/16 to MPH
        ss[k].head     = h;
        ss[k].pitch    = p;
        ss[k].roll     = r;
//        ldist     = tempmissile->range * (825/4);
		if(weapon == -1)
        	ldist     = 3 * (825/8);
		else
        	ldist     = tempmissile->range * (825/8);
        ss[k].dist     = (int)ldist;
//        ss[k].dist     = tempmissile->range * (825/4);
        ss[k].type     = type;
        ss[k].weapon   = weapon;
        ss[k].station  = LastSlotNMount;
        ss[k].srctype  = 1;
        ss[k].source   = source;
        ss[k].losing   = 0;

		if(weapon == -1)
			weapon = 0;

		if ((ss[k].srctype==1) && (ss[k].source==MAXPLANE) && (ss[k].weapon==W_AIM_54) &&
			((CheckMode(AWG9_MODE,AWG9_TWSA)) || (CheckMode(AWG9_MODE,AWG9_TWSM)))) {
		// Track-While-Scan launch - Update Missile as fired and update the
		// Current track information
				for (tcnt=0; tcnt<6; tcnt++) {
					if (DESIG_TARG[tcnt].src_station==LastSlotNMount) {
						DESIG_TARG[tcnt].status=2;  // Missile Fired - Still Designated
						DESIG_TARG[tcnt].ssi_index=k;
						gtarg = DESIG_TARG[tcnt].target;
					}
				}
			UpdateTIDTargs();
			}


        if ((ORDNANCE[weapon].wtype&W_AAW) || (targettype == 1)) {
            ss[k].target   = gtarg;
            ss[k].tgttype  = 1;

			if((warstatus == 0) && (gtarg == MAXPLANE)) {
				warstatus = 1;
				LastCall = -4*60;
			}

			if ((ss[k].srctype==1) && (ss[k].source==MAXPLANE) && (ss[k].weapon==W_AIM_54) &&
				((CheckMode(AWG9_MODE,AWG9_TWSA)) || (CheckMode(AWG9_MODE,AWG9_TWSM)))) {
						// Don't do anything
			} else {
				if((gtarg != MAXPLANE) && (gtarg >= 0) && (gtarg < NPLANES)) {
					if(CheckAIBombs(gtarg) != -999) {
						ShowJettison(gtarg);
					}
					if((warstatus == 0) && (GetSquadLeader(gtarg) == -1)) {
						warstatus = 1;
						LastCall = -4*60;
					}
				}
			}
        } else {
            ss[k].tgtx = TgtX;
            ss[k].tgty = TgtY;
            ss[k].tgttype = 0;
        }
    }
    return k;
}

//***************************************************************************
FindEmptyMissileSlot(int Start, int End)
{
    int i;

    for (i=Start; i<End; i++) {
        if (ss[i].dist==0) {
            return (i);
        }
    }
    return (-1);
}

//***************************************************************************
FindEmptyMissileSlot2 (int Start, int End, int shooter)
{
    int i;
	int returnval;
	int playertarget;

	returnval = -1;
	playertarget = 0;
    for (i=Start; i<End; i++) {
        if (ss[i].dist==0) {
            returnval = i;
		}
		else {
			if((ss[i].source == shooter) && (ss[i].tgttype)) {
				return(-1);
			}
        }
    }
    return (returnval);
}

//***************************************************************************
FindEmptyMissileSlotP (int Start, int End, int shooter)
{
    int i;
	int returnval;
	int playertarget;

	returnval = -1;
	playertarget = 0;
    for (i=Start; i<End; i++) {
        if (ss[i].dist==0) {
            returnval = i;
		}
		else {
			if(ss[i].target == MAXPLANE) {
				playertarget++;
				if(playertarget >= (2 << Diff))
					return(-1);
			}
			if((ss[i].source == shooter) && (ss[i].tgttype))
				return(-1);
        }
    }
    return (returnval);
}

//***************************************************************************
static HomeOnClosestPlane (int k)   // Sets dist, angl, homing, pangl, zd
{
	int j, homee;
	struct MissileData far *tempmissile;

    homee = -1;
	tempmissile = GetMissilePtr(ss[k].type);
    homer = tempmissile->homing;

    for (j = 0, dist = 0x7fff; j < NPLANES; j++) {
        if (ps[j].status&ACTIVE && ps[j].speed) {
            HomeHead = ps[j].head;
            if (home (ps[j].xL, ps[j].yL, ps[j].alt, homer) ) {
                if (homedist < dist) {
                    dist   = homedist;
                    homee = j;
                }
            }
        }
    }

    return (homee);
}

//***************************************************************************
MissilesInAir ()
{
    int i;

    for (i=0, ssi=ss; i<NSAM; i++, ssi++) {
        if (ssi->dist) return 1;
    }

    return 0;
}

//***************************************************************************
FoxCall(weapon)
{
    TXT ("(WingMan) ");
    switch (weapon) {
        case W_AIM_7:
			TXA ("Fox 1!");
			Speech (FOX1);
		break;
        case W_AIM_9:
			TXA ("Fox 2!");
			Speech (FOX2);
		break;
        case W_AIM_120:
		case W_AIM_54:
			TXA ("Fox 3!");
			Speech (FOX3);
		break;
    }
    Message(Txt,WINGMAN_MSG);
}

//***************************************************************************
LaunchCall()
{
	Message("Launch, Launch!",RIO_NOW_MSG);
	Speech(LAUNCH);
	Sound(N_RadarLock, 1,0);
}

//***************************************************************************
int CheckBlastArea(long xval, long yval, int weapon, int cruise)
{
    int targ, dx, dy, LastTgt, ObjNr;
	int TargetHit;
	int cnt;
	long gdx, gdy;
	int radarnum;

	TargetHit = 0;

	for(cnt = 0; cnt < NGTARGETS; cnt ++) {
		if(gt[cnt].gflags != 0) {
			gdx = labs(gt[cnt].x - xval);
			gdy = labs(gt[cnt].y - yval);
			if((gdx < (long)2048) && (gdy < (long)2048)) {
				Message("Ground Target Destroyed", RIO_MSG);
				gt[cnt].gflags = 0;
				StartASmoke( xval, 120, yval, GRNDXPLO, 30 );

				radarnum = GetGTRadarNum(cnt);
				if(radarnum != -999)
					Rdrs[radarnum].status |= DESTROYED;

				if(cruise > -1) {
					LogIt (SLG_GROUND_DESTROYED_CRUISE, gt[cnt].orgnum, gt[cnt].x, gt[cnt].y, 1, bv[cruise].prevsec, ps[cruise].type, 0L, LOG_GRND);
				}
				else {
					LogIt (SLG_GROUND_DESTROYED_BOMB, gt[cnt].orgnum, gt[cnt].x, gt[cnt].y, 1, ssi->source, ssi->type, 0L, LOG_GRND);
				}

				if(CloseEnoughFor(SNDSTUFF, (int)(xval>>HTOV), (int)(yval>>HTOV)))
	    			Sound (N_FarExplo, 2,0);

			}
		}
	}
	return(TargetHit);
}


