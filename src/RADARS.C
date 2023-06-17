/*  File:   Radars.c                              */
/*  Author: Sid Meier                             */
/*                                                */
/*  Game logic for Stealth: enemy radars          */
/*                                                */

#include "library.h"

#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "sounds.h"
#include "f15defs.h"
#include "awg9.h"
#include "speech.h"
#include "planeai.h"
#include "setup.h"

#define NM10 475

extern  int timetargetplayer;
extern  long    PxLng;
extern  long    PyLng;
extern  COORD   Px,Py;
extern  UWORD   Alt;
extern  int     CloseDist[];
extern  int     detected;
extern  COORD   xlast[],ylast[];
extern  int     zlast[];
extern  int     pulse,pulsee;
extern  int     Rtime;
extern  int     JAMMER[];
extern  int     FTicks;

        int     CloseGuy;
        int     DMAX9;

		int     LastSpike;
		int     LastMud;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);


//***************************************************************************
// Process enemy ground radar sites
doradar()
{
	int i;
	struct Radars *r;

    if (LastSpike) LastSpike -= FTicks;
    if (LastMud)   LastMud   -= FTicks;

	// determine active radar

// Make sure that the Rdrs array is set up properly before this
// is working

//	for (i=0; i<NERADAR; i++) {
	for (i=0; i<NRADAR; i++) {
		r = Rdrs + i;
		r->status &= ~RACTIVE;
		if (r->rclass != RL_NONE && !(r->status&DESTROYED)) {
			--r->pause;
			if (r->pause < 0) {
//				r->pause = 256*FrameRate/(32+(r->alert>>2))+i;
				r->pause = 512*FrameRate/(32+(r->alert>>4))+i;
			}
			else if (r->pause == 0) {
					r->status |= RACTIVE;
					detect (i);
			}
		}
	}
}

//***************************************************************************
// Radar detection check
detect(int i)
{
    COORD dist,xydist();
    int  signal,j,k,angl,a,m;
    long x,y;
	int targetnum;
	struct RadarData far *tempradar;
	struct MissileData far *tempmissile;
	long ldist;
	int gtnum;


    // check if returning a signal

	// SCOTT - The radar list needs to be read in like the new data list stuff
	// A radar list is built and the various types are stored

    j = Rdrs[i].rclass;
	tempradar = GetRadarPtr(j);
    signal = Rsignal (Rdrs[i].x,Rdrs[i].y,0, j,&angl,&dist, Rdrs[i].status & (FRIENDLY|NEUTRAL), &targetnum);

	if(targetnum < -1)
		return(0);

    if (!Night && dist <= 2) {   // visual
         signal = dist+1;
         Rdrs[i].alert = 255;
    }

    // check if detected

    if (signal>dist) {
        if (Rdrs[i].alert + (5<<Diff) <256) {
            Rdrs[i].alert  += (5<<Diff);
        } else {
            Rdrs[i].alert = 255;
        }

        if (Rdrs[i].alert>127) {
			tempgotcha = 1;
            gotcha(targetnum);
			tempgotcha = -999;
            if (!(Rdrs[i].status&RTRACK) && (tempradar->detectiontype==RADAR)) {
                if (!(Damaged&D_ECM)) {
					if(targetnum == MAXPLANE)
                    	Sound(N_RadarSearch, 1,0);
                    if (CloseGuy==0 && !LastMud) {
                        Message("Singer!",RIO_MSG);
						Speech(SINGER);
                        LastMud = 8*60;
                    }
                }
            }
            Rdrs[i].status |= RTRACK;
        }

        if (!(Rdrs[i].status&RTRACK) && (tempradar->detectiontype==RADAR)) {
            if (!(Damaged&D_ECM)) {
				if(targetnum == MAXPLANE)
                	Sound(N_RadarSearch, 1,0);
                if (CloseGuy==0) {
                    TEWS_SPOTTED (i);                   // set TEWS display
                    if (!(Rdrs[i].status&RSEEN) && !LastSpike) {
                        Message("Spike!",RIO_MSG);
						Speech(SPIKE);
                        LastSpike=8*60;
                    }
                }
            }
        }

        Rdrs[i].status |= RSEEN;

        // SAM WILL BE FIRED IF:
        // 1) MISSILE IS ASSOCIATED WITH RADAR INSTALLATION
        // 2) NOT OUT OF MISSILES
        // 3) RADAR ALERT LEVEL IS HIGH ENOUGH
        // 4) PLAYER IS NOT CLOSE TO FRIENDLY BASE
        // 5) PLAYER IS WITHIN MISSILE ENVELOPE

//        m = r[j].mtype;
		tempmissile = GetMissilePtr(tempradar->missiletype);
        a = (CloseGuy ? ps[targetnum].alt : Alt);

        if (tempradar->missiletype == ML_NONE) return 0;	// 1)
        if (Rdrs[i].status & NOAMMO) return 0;      	// 2)
        if (Rdrs[i].alert <= 250) return 0;      		// 3)
/*        if (CloseDist[CloseGuy]<NM10) return 0;  */   // 4)
        if (dist > tempmissile->range) return 0;    	// 5)
        if (dist < tempmissile->minrange) return 0; 	// 5)
        if (a > tempmissile->maxalt*3281L/4) return 0;  // 5)

        if (JAMMER[CloseGuy]) {
            if (tempradar->jam>(Rtime+i)%10) return 0;
        }

        x = (long)(Rdrs[i].x + 8)<<HTOV;
        y = (long) Rdrs[i].y<<HTOV;

        if (tempmissile->homing==IR_1 && !CheckTailAspect (x, y)) return 0;


//        k= (Diff ? i%Diff : 0) + 15;                   // planes use the first fifteen slots
		k = FindEmptyMissileSlot2 (NPSAM, NESAM, i);


		if(k == -1)
			return(0);

        if (ss[k].dist != 0)           return 0;      // NO OPEN SLOTS

        Rdrs[i].alert  -= 50<<(3-Diff);

        ss[k].x       = x;
        ss[k].y       = y;
        ss[k].alt     = 25;
        ss[k].speed   = 1;
        ss[k].head    = angl;
        ss[k].pitch   = 0x4000;
        ldist         = (long)tempmissile->range*(825/8);
        ss[k].dist    = (int)ldist;
        ss[k].type    = tempmissile->MissileEnumId;
        ss[k].source  = i;
        ss[k].srctype = 0;
//        ss[k].target  = CloseGuy?0:MAXPLANE;
        ss[k].target  = targetnum;
        ss[k].tgttype = 1;
        ss[k].losing  = 0;

		gtnum = GetRadarGTNum(i);

		LogIt (SLG_GROUND_MISSILE_LAUNCH, gtnum, x, y, 1, -999, tempmissile->MissileEnumId, 0L, LOG_GRND);

        if (CloseGuy==0) {
            Sound(N_RadarLock, 1,0);
            Message("Singer Launch!",RIO_MSG);
			Speech(SINGERLAUNCH);
        }

		if(CloseEnoughFor(SLOWSTUFF, (int)(x>>HTOV), (int)(y>>HTOV)))
        	NoAccel();

        if (rnd (tempradar->missilecount) == 0) {                   // OUT OF MISSILES?
            Rdrs[i].status |= NOAMMO;
            return 0;
        }
    } else {
        Rdrs[i].status &= ~RTRACK;
        Rdrs[i].status &= ~RSEEN;

        if (Rdrs[i].alert  >= (2<<(3-Diff))) {
            Rdrs[i].alert  -= (2<<(3-Diff));
        } else {
            Rdrs[i].alert   = 0;
        }

    }
}

//***************************************************************************
// General radar detection algorithm for ground radars and planes
Rsignal(COORD x,COORD y,int z, int type,int *ang,int *dst,int isfriend,int *targetnum)
{
    COORD dist,dist1,xydist();
    int  dx,dy,dx1,dy1;
    int  signal, alt, DeadZone, cap;
	int	 founddist, cnt;
	int friendflag;
	struct RadarData far *tempradar;

	*targetnum = -999;
	founddist = 0;

	/* Calculate signal strength */
	if(isfriend != FRIENDLY) {
		founddist = 1;
    	dx = Px-x;
    	dy = Py-y;
    	dist = xydist(dx,dy);
    	CloseGuy=0;
		*targetnum = MAXPLANE;
	}

    for (cnt=0; cnt<NPLANES; cnt++) {
        if ((ps[cnt].status & ALIVE) && (ps[cnt].status&ACTIVE) && (!(ps[cnt].status&(CRASH|CIVILIAN))) && (ps[cnt].speed)) { /* Planes in the air */
			if(!FriendMatchType(cnt, isfriend)) {
       			dist1 = xydist(dx1=ps[cnt].x-x, dy1=ps[cnt].y-y);
       			if ((dist1<dist) || (!founddist)) {
					founddist = 1;
           			dist=dist1;
           			dx = dx1;
           			dy = dy1;
            		CloseGuy=1;
					*targetnum = cnt;
       			}
			}
		}
	}

    *dst = (int)((long)dist*32/825);   // convert to Km
    *ang = angle (dx,-dy);

	tempradar = GetRadarPtr(type);

    signal = tempradar->range;

	if(!founddist)
		*targetnum = -999;

    // dead zone for ground-based radar only

    if (z==0 && tempradar->detectiontype == RADAR) {
        alt = (CloseGuy?ps[*targetnum].alt:Alt);

        // Dead Zone is approx:        Capped at:
        // Level 3 - 1013 ft per 10NM              10000 ft
        // Level 2 - 1216 ft per 10NM              12000 ft
        // Level 1 - 1500 ft per 10NM              14000 ft
        // Level 0 - 2026 ft per 10NM              16000 ft

        if (dist<475) dist = 475;                       // min dist 10 NM
        DeadZone = ((long)dist<<HTOV)/(30+(Diff*10));
        cap = 4000 - (Diff*500);
        if (DeadZone > cap) DeadZone = cap;
        if (DeadZone > alt) signal = *dst;
    }

	if ((AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) && AWG9_SNIFF && Diff>=2) {
        signal -=signal*2/8;         // reduce range by Diff factor
    } else {
        signal -=signal*(3-Diff)/8;  // reduce range by Diff factor
    }

    return signal;
}

//***************************************************************************
// We've been detected !! (Arghhhh)
gotcha (int targetnum)
{
	int i;

    detected = FrameRate<<8;
    DMAX9 = FrameRate<<6;

    if (targetnum != MAXPLANE) {
		if((ps[targetnum].status & CRASH) || (!((ps[targetnum].status & ALIVE) && (ps[targetnum].status & ACTIVE))))
			return(0);
        xlast[1]=ps[targetnum].x;
        ylast[1]=ps[targetnum].y;
        zlast[1]=ps[targetnum].alt;
		InterceptControl(targetnum);
		ps[targetnum].time = Gtime;  /*  Us this to tell us last time plane detected  */

    } else {
        xlast[0]=Px;
        ylast[0]=Py;
        zlast[0]=Alt;
		InterceptControl(-1);
//		bv[0].behaveflag &= ~ENEMYNOFIRE;
		timetargetplayer = Gtime;
    }

//  Make sure the Rdrs array is setup properly before turning this on
//	for (i=0; i<NTARGET; i++) {
    for (i=0; i<0; i++) {
        if (Rdrs[i].rclass!=RL_NONE) {
            Rdrs[i].alert = rng (Rdrs[i].alert, 10<<Diff,255);
        }
    }
}

//***************************************************************************
// General radar detection algorithm for ground radars and planes
// When looking at only one specific plane
int RsignalOne (COORD x,COORD y,int z, int type,int *ang,int *dst,int isfriend,int targetnum)
{
    COORD dist,dist1,xydist();
    int  dx,dy,dx1,dy1;
    int  signal, alt, DeadZone, cap;
	int friendflag;
	struct RadarData far *tempradar;

	if(targetnum < -1){
		tempradar = GetRadarPtr(type);
    	signal = tempradar->range;

	    *dst = signal + 100;
    	*ang = 0;

		return(signal);
	}

	/* Calculate signal strength */
	if((!isfriend) && (targetnum == -1)) {
    	dx = Px-x;
    	dy = Py-y;
    	dist = xydist(dx,dy);
    	CloseGuy=0;
	}

	if(targetnum >= 0) {
    	if ((ps[targetnum].status & ALIVE) && (ps[targetnum].status&ACTIVE)
				&& (!(ps[targetnum].status&(CRASH|CIVILIAN))) && (ps[targetnum].speed)) { /* Planes in the air */
			if(!FriendMatchType(targetnum, isfriend)) {
       			dist1 = xydist(dx1=ps[targetnum].x-x, dy1=ps[targetnum].y-y);
       			dist=dist1;
       			dx = dx1;
       			dy = dy1;
          		CloseGuy=1;
			}
		}
	}

    *dst = (int)((long)dist*32/825);   // convert to Km
    *ang = angle (dx,-dy);

    signal = tempradar->range;

    // dead zone for ground-based radar only

    if (z==0 && tempradar->detectiontype == RADAR) {
        alt = (CloseGuy?ps[targetnum].alt:Alt);

        // Dead Zone is approx:        Capped at:
        // Level 3 - 1013 ft per 10NM              10000 ft
        // Level 2 - 1216 ft per 10NM              12000 ft
        // Level 1 - 1500 ft per 10NM              14000 ft
        // Level 0 - 2026 ft per 10NM              16000 ft

        if (dist<475) dist = 475;                       // min dist 10 NM
        DeadZone = ((long)dist<<HTOV)/(30+(Diff*10));
        cap = 4000 - (Diff*500);
        if (DeadZone > cap) DeadZone = cap;
        if (DeadZone > alt) signal = *dst;
    }

	if ((AWG9_REALITY_LEVEL==AWG9_AUTHENTIC) && AWG9_SNIFF && Diff>=2) {
        signal -=signal*2/8;         // reduce range by Diff factor
    } else {
        signal -=signal*(3-Diff)/8;  // reduce range by Diff factor
    }

    return signal;
}

