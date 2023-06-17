/*  File:   BULLETS.C                             */
/*  Author: Sid Meier / Andy Hollis               */
/*                                                */
/*  Start, Track and Render bullet tracers        */

#include <stdlib.h>
#include	<stdio.h>
#include <bios.h>
#include <dos.h>
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "sounds.h"
#include "f15defs.h"
#include "keycodes.h"
#include "armt.h"
#include "event.h"
#include "proto.h"
#include "setup.h"

#define S2L(a) ((long) a<<HTOV)
//#define GUNTEST 1

struct CloseObj *TargetObj, *CheckTargetHit ();

extern  int     sotnum;
extern  COORD   Px,Py;
extern  long    PxLng,PyLng;
extern  SWORD	Knots;
extern  RPS     *Rp3D;
extern  int     OurHead,OurPitch;
extern  UWORD   Alt;
extern  int     Rtime;
extern  char    Txt[50];
extern  int     sx,sy,sz;
extern  UWORD   KEYPRINT;
extern  int 	LastCall;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);

int     Firing;
int     NTRACE,NTRACE2,FIRST_ENEMY_TRACE;
int     LastRate;
int     TSpeed;

extern struct FPARAM{
	long	X;
  	long	Y;
	long	Z;
	long	VEL[3];
	int	MAT[9];
} FPARAMS;


//****************************************************************************
//
//  HOW IT ALL WORKS:
//
//  One-player game - Elements of the "trs" array below FIRST_ENEMY_TRACE are
//  the player's bullets and track against enemy planes or ground targets.
//  Those equal to and above the limit, up to NTRACE2 are enemy firing at you
//
//  Front/Back Seat - same as above
//
//  Two-player WingMan - Now we two sets of each.  The bullets from 0 to
//  NTRACE are your bullets.  Those from NTRACE to 2*NTRACE are your wingy's
//  bullets.  The next four, starting at FIRST_ENEMY_TRACE are shots at you.
//  The last four are shots at your wingman.
//
//  Head-to-Head -
//

// NTRACE is set according to the framerate so that there are enough tracer
// slots for each one to be active for one second.
// "docannon" cycles through the slots every frame.
//
//****************************************************************************

//***************************************************************************
InitCannonRate ()
{

    LastRate = FrameRate;

    NTRACE =  rng (LastRate, 3,16);

    NTRACE2 = (NTRACE+4);

    FIRST_ENEMY_TRACE = NTRACE;

    TSpeed = ((3-Diff)*1000 + 3000)/4/LastRate;
}

//***************************************************************************
docannon ()
{
    int n,i,tspd;

    if ((KEYPRINT==ENTER) && (Rounds>0) && !ejected) {
        Rounds = rng(Rounds-40/LastRate, 0,1000);
        Sound(N_VulcanCannon,2,0);
        firecannon (PxLng, 0x100000-PyLng, Alt, OurHead, OurPitch, 1);
        Firing = TRUE;
    } else {
        n = (Rtime % NTRACE);
        trs[n].x = 0;
        Firing = FALSE;
    }
}

//***************************************************************************
firecannon (long x, long y, int z, int Head, int Pitch, int local)
{
    int n, tspd;

    n = Rtime % NTRACE;
    if (!local) n+= NTRACE;

    tspd = TSpeed;
    trs[n].dz = sinX (Pitch, tspd);

    tspd = cosX (Pitch, tspd);
    trs[n].dx = sinX (Head, tspd);
    trs[n].dy = -cosX (Head, tspd);

    trs[n].x = x /*+ (trs[n].dx)*/;
    trs[n].y = y /*+ (trs[n].dy)*/;
    trs[n].z = z /*+ (trs[n].dz)*/;
	trs[n].firedby = -1;
}

//***************************************************************************
dotracers ()
{
    int i, d;

    if (Rtime==4) InitCannonRate ();    // re-init after a couple frames

    for (i=0; i<NTRACE2; i++) {
/*		if(i == sotnum) {
			sprintf(Txt, "NX1 %d %ld", i, trs[i].x);
			Message(Txt,DEBUG_MSG);
		}  */
        if (trs[i].x) {
            if (trs[i].z<-trs[i].dz) {
                d = (trs[i].z*32768L) /-trs[i].dz;

                trs[i].dx = TrgMul (trs[i].dx, d);    /* interpolate for Z intercept */
                trs[i].dy = TrgMul (trs[i].dy, d);
                trs[i].dz = -trs[i].z;
            }

            trs[i].x += trs[i].dx;
            trs[i].y += trs[i].dy;
            trs[i].z += trs[i].dz;
        }
    }
    TrackBullets ();
}

//***************************************************************************
TrackBullets()
{
    int size,i,j,n;
    int r,angl, cnt;
    int dx,dy,targt,tsize;
    int Radii[] = {50, 25, 17, 13};  // 50/(Diff+1)
	struct PlaneData far *planetemp;
	int src;
	int crashstat;

	/* Process bullets */

    ORDNANCE[W_CANNON].area = Radii[Diff];

    tsize = TSpeed + Radii[Diff];   // 50 feet @ diff==3

    for (i=0; i<NTRACE2; i++) {
        if (trs[i].x) {
            if (i<FIRST_ENEMY_TRACE) {
                for (j=0; j<NPLANES; j++) {
                    if ((ps[j].status&(ACTIVE+CRASH))==ACTIVE) {
                        n = BulletDist (ps[j].xL, ps[j].yL, ps[j].alt, trs+i);
                        if (n<(tsize* 2)) {
                            ps[j].status |= PANIC;

//                            if (n<(tsize>>2)) {
                            if (n<tsize) {
								if(CheckBulletBetter(ps[j].xL, ps[j].yL, ps[j].alt, trs+i, j)) {
									src = trs[i].firedby;
									if((!(DOINVISIBLE||ENEMYNODIE)) && (bv[j].Behavior != EngageTrain)) {
										if(Rtime & 0x1) {
											crashstat = ps[j].status & CRASH;
											DamageAI(j);
											if(crashstat != (ps[j].status & CRASH)) {
												LogIt (SLG_AIR_DESTROYED_BULLET, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
	                    	                	TXA (" destroyed by ");
    	                    	            	if (i>=NTRACE) TXA ("wingman ");
        	                    	        	TXA ("gunfire");
            	                    	    	Message (Txt,GAME_MSG);
                	                    		trs[i].x = 0;
											}
											else {
												LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
											}
										}
										else {
							        		AirKill (j);
//											LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
											LogIt (SLG_AIR_DESTROYED_BULLET, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
                    	                	TXA (" destroyed by ");
                        	            	if (i>=NTRACE) TXA ("wingman ");
                            	        	TXA ("gunfire");
                                	    	Message (Txt,GAME_MSG);
                                    		trs[i].x = 0;
										}
									}
									else {
										LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
										StartASmoke( ps[j].xL, ps[j].alt,ps[j].yL, AIRXPLO, 10 );
    									Sound(N_FarExplo,2,0);
									}
								}
                            }
                        }
                    }
                }
            } else {

                n = BulletDist (PxLng, 0x100000-PyLng, Alt, trs+i);

                if (n<(tsize * 2)) {
					if(CheckBulletBetter(PxLng, 0x100000-PyLng, Alt, trs+i, -1)) {
	                    TXT ("Hit by gunfire");
    	                Message (Txt,GAME_MSG);
						src = trs[i].firedby;
						// We were hit by med guns
        	            DamageMe(2);
						LogIt (SLG_AIR_BULLET_HIT, -1, PxLng, (0x100000-PyLng), Alt, src, -999, 0L, LOG_AIR);
/*
#if OLDLOGDATA
            	        LogIt (AAAHIT, 1);
#else
						LogIt(E_HIT, 0, EHC_GUN, PxLng, PyLng);
#endif  */
	                    trs[i].x = 0;
    	            }
				}
                for (j=0; j<NPLANES; j++) {
                    if ((ps[j].status&(ACTIVE+CRASH))==ACTIVE) {
                        n = BulletDist (ps[j].xL, ps[j].yL, ps[j].alt, trs+i);
                        if (n<(tsize* 2)) {
                            ps[j].status |= PANIC;

                            if ((n<tsize) && (trs[i].firedby != j)) {
								if(CheckBulletBetter(ps[j].xL, ps[j].yL, ps[j].alt, trs+i, j)) {

//									Message("Plane Hit By Bullets",DEBUG_MSG);
									src = trs[i].firedby;
									if((!(DOINVISIBLE||ENEMYNODIE)) && (bv[j].Behavior != EngageTrain)) {
										if((bv[j].leader == -1) || (Rtime & 0x1)) {
											crashstat = ps[j].status & CRASH;
											DamageAI(j);
											if(crashstat != (ps[j].status & CRASH)) {
												LogIt (SLG_AIR_DESTROYED_BULLET, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
												if(GetSquadLeader(trs[i].firedby) == -1) {
													GetWingName(trs[i].firedby, Txt);
//													TXT("(Wingman)Splash ");

													if(ps[j].status & FRIENDLY)
														TXA("OPPS! ");

													TXA("Splash ");
													planetemp = GetPlanePtr(ps[j].type);
										    		FTXA (planetemp->name);
													TXA ("!");
													Message(Txt,WINGMAN_MSG);
												}
											}
											else {
												LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
											}
										}
										else {
						        			AirKill (j);
//											LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
											LogIt (SLG_AIR_DESTROYED_BULLET, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
											if(GetSquadLeader(trs[i].firedby) == -1) {
												TXT("(Wingman)Splash ");
												planetemp = GetPlanePtr(ps[j].type);
									    		FTXA (planetemp->name);
												TXA ("!");
												Message(Txt,WINGMAN_MSG);
											}
										}
									}
									else {
										LogIt (SLG_AIR_BULLET_HIT, j, ps[j].xL, ps[j].yL, ps[j].alt, src, -999, 0L, LOG_AIR);
										StartASmoke( ps[j].xL, ps[j].alt,ps[j].yL, AIRXPLO, 10 );
    									Sound(N_FarExplo, 2,0);
									}
                                	trs[i].x = 0;
								}
                            }
                        }
                    }
                }
            }

/* Did bullet hit the ground ? */
            if (trs[i].z==0) {
#if 0
                while ((TargetObj=CheckTargetHit (trs[i].x, trs[i].y, ORDNANCE+W_CANNON)) !=NULL) {
                    GroundKill ();
                    TXA (" destroyed by ");
                    if (i>=NTRACE) TXA (" wingman ");
                    TXA ("gunfire");
                    Message (Txt,GAME_MSG);
                }
#endif

                trs[i].x = 0;
            }
        }
    }
}


//***************************************************************************
DrawBullets()
{
    int i, ssx,ssy;

    for (i=0; i<NTRACE2; i++) {
        if (trs[i].x) {
            lproject (trs[i].x, trs[i].y, trs[i].z);
            ssx=sx; ssy=sy;
            lproject ((trs[i].x+(trs[i].dx>>2))&~3, (trs[i].y+(trs[i].dy>>2))&~3, (trs[i].z+(trs[i].dz>>2))&~3);

            if (sx!=-1  && ssx!=-1) {
                DrawLine (Rp3D, sx,sy, ssx,ssy, i<FIRST_ENEMY_TRACE ? ORANGEFLAME:BLUEFLAME);
            }
        }
    }
}

//***************************************************************************
BulletDist(long x, long y, int z, struct Tracer *t)
{
    long    dx, dy;
    int     dz;

    dx = labs (x-t->x);
    dy = labs (y-t->y);
    dz = abs (z-t->z);

    if (dx+dy+dz >= 23000L) {
        return 32767;
    } else {
        return (Dist3D ((int)dx, (int)dy, dz));
    }
}

//***************************************************************************
int CheckBulletBetter(long x, long y, int z, struct Tracer *t, int targnum)
{
    long    dx, dy;
    int     dz;
	long	xydval;
	long	xyzdval;
	int		xyangle;
	int		xyzpitch;
	int		trshead;
	int		trspitch;
	int		trsxydist;
	long	conewidth;
	long	coneheight;
	long	dwidth;
	long	dheight;
	int     whofired;
//	struct plane  far *ptargettype;
	struct PlaneData far *ptargettype;



	whofired = t->firedby;
	if(whofired >= 0) {
		ptargettype = GetPlanePtr(ps[t->firedby].type);

		if((ptargettype->gun == 1) || (ptargettype->gun == 3))
			return (1);
	}

	conewidth = 6;
	coneheight = 6;

	if(whofired != -1) {
		if (targnum>=0) {
			ptargettype = GetPlanePtr(ps[targnum].type);
		}
		if(whofired > NPLANES) {
			if(IsDeckPlane(whofired, targnum))
				return(0);

			conewidth = conewidth<<5;
			coneheight = coneheight<<5;
		}
		else {
			conewidth = conewidth<<3;    /*  was 3  */
			coneheight = coneheight<<3;  /*  was 3  */
		}
		if (targnum>=0) {
			if(ptargettype->maneuver < 2) {
				conewidth = conewidth<<1;
				coneheight = coneheight<<1;
			}
		}
	} else {
		ptargettype = GetPlanePtr(ps[targnum].type);
		if (targnum>=0) {
			if(ptargettype->maneuver < 2) {
				conewidth *= 5;
				coneheight *= 5;
			} else {
				conewidth = conewidth<<2;
				coneheight = coneheight<<2;
			}
		}
		else {
			conewidth = conewidth<<2;
			coneheight = coneheight<<2;
		}
	}

    dx = labs (x-t->x);
    dy = labs (y-t->y);
    dz = abs (z-t->z);

	trshead = angle(t->dx, t->dy);
	trsxydist = Dist2D(t->dx, t->dy);
	xydval = LDist2D(dx, dy);
	xyangle = Langle(dx, -dy);
	xyangle = xyangle - trshead;
    dwidth = sinXL(xyangle, xydval);

	if(labs(dwidth) < conewidth) {
		trspitch = angle(t->dz, trsxydist);
		xyzpitch = Langle((long)dz, xydval);
		xyzpitch = xyzpitch - trspitch;
		xyzdval = LDist2D((long)dz, xydval);
	    dheight = sinXL(xyzpitch, xyzdval);
		if(labs(dheight) < coneheight) {
			return(1);
		}
		else {
#ifdef GUNTEST
			if(GetSquadLeader(whofired) == -1) {
				sprintf(Txt, "CH %ld, %ld, T %d", coneheight, dheight, targnum);
				Message(Txt,DEBUG_MSG);
			}
#endif
			return(0);
		}
	}
	else {
#ifdef GUNTEST
		if(GetSquadLeader(whofired) == -1) {
			sprintf(Txt, "CW %ld, %ld, T %d, a %x, d %ld", conewidth, dwidth, targnum, xyangle, xydval);
			Message(Txt,DEBUG_MSG);
		}
#endif
		return(0);
	}

	return(0);
}

//***************************************************************************
int IsDeckPlane(int whofired, int targnum)
{
	if(!FriendMatch(whofired, targnum))
		return(0);

	if(targnum == -1) {
		if (OnTheDeck(FPARAMS.X, FPARAMS.Y, FPARAMS.Z))
			return(1);
	}
	else {
		if((ps[targnum].status & TAKEOFF) || (bv[targnum].basespeed == 0))
			return(1);
	}
	return(0);
}

