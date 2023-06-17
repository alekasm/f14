#include "library.h"
#include "f15defs.h"
#include "world.h"
#include "sdata.h"
#include "armt.h"
#include "event.h"
#include "proto.h"
#include "planeai.h"
#include "setup.h"
#include	<stdio.h>
#include	<stdlib.h>
#include <bios.h>
#include <dos.h>


extern  UWORD   QTimer;
extern  UWORD   Rtime;
extern  int     detected;
extern  UWORD   Alt;
extern  long    PxLng, PyLng;
extern  UWORD   Px, Py;

static	int	    MapX,MapZ;
static  UWORD   Dist2Obj;
static  UWORD   HitFlags;
static  int     NUM, k;


//***************************************************************************
void doaaa()
{
	int cnt;
	int pcnt;
	int pstart;
	long dx, dy;
	long ldistval;
	int thistime;
	int lastfire;

	pstart = Rtime & 7;
	for(cnt = 0; cnt < NGTARGETS; cnt ++) {
		if(gt[cnt].gflags & AAA_ACTIVE) {
			lastfire = (gt[cnt].gflags & 0xE)>>1;
			gt[cnt].gflags &= ~(0xE);
			thistime = gt[cnt].gflags & AAA_FIRING;
			gt[cnt].gflags &= ~AAA_FIRING;
			for(pcnt = pstart; pcnt < NPLANES; pcnt += 8) {
				if((!(ps[pcnt].status & CRASH)) && (ps[pcnt].status & ACTIVE) && (ps[pcnt].status & ACTIVE) && (ps[pcnt].alt <= 2000) && (((gt[cnt].gflags & (FRIENDLY|NEUTRAL)) != FriendStatus(pcnt)))) {
					dx = labs(gt[cnt].x - ps[pcnt].xL);
					dy = labs(gt[cnt].y - ps[pcnt].yL);
					if((dx < (long)(550<<HTOV)) && (dy < (long)(550<<HTOV))) {
						gt[cnt].gflags |= AAA_FIRING;
        			    ldistval = LDist2D (dx, dy) + ps[pcnt].alt;
						if(ldistval < 4101L) {
	        			    Dist2Obj = (int)ldistval;
							CheckAAAHit(pcnt, cnt);
						}
					}
				}
			}
			if(((pstart == 0) || (pstart == 4)) && (!(gt[cnt].gflags & FRIENDLY))) {
				dx = labs(gt[cnt].x - PxLng);
				dy = labs(gt[cnt].y - (0x100000-PyLng));
       			ldistval = LDist2D (dx, dy) + Alt;
				if((dx < (long)(550<<HTOV)) && (dy < (long)(550<<HTOV))) {
					gt[cnt].gflags |= AAA_FIRING;
        			ldistval = LDist2D (dx, dy) + Alt;
					if(ldistval < 4101L) {
	        			Dist2Obj = (int)ldistval;
						CheckAAAHit(MAXPLANE, cnt);
					}
				}
			}
			if(gt[cnt].gflags & AAA_FIRING) {
				lastfire = (gt[cnt].gflags & 0xE)>>1;
			}
			else if(thistime) {
				if(lastfire != 7) {
					gt[cnt].gflags |= AAA_FIRING;
					gt[cnt].gflags |= ((lastfire + 1) & 0x7)<<1;
					lastfire = (gt[cnt].gflags & 0xE)>>1;
				}
			}
		}
	}
}

//***************************************************************************
void CheckAAAHit(int planenum, int gtnum)
{
	int crashstat;
	int src;

	src = -gt[gtnum].orgnum;

    if (Dist2Obj<820) gotcha (planenum);  // 1 km

    if (Dist2Obj < 4101) {      // 5 km
        if (planenum == MAXPLANE) {
	        if ((Dist2Obj&127)==(Rtime&127)) {
				// We were hit by heavy guns
                DamageMe(3);

				LogIt (SLG_AAA_HIT, -1, PxLng, (0x100000-PyLng), Alt, src, -999, 0L, LOG_AIR);

                Message("Hit by AAA",RIO_MSG);
            }
		}
		else {
	        if ((Dist2Obj&255)==(Rtime&255)) {
				crashstat = ps[planenum].status & CRASH;
                DamageAI (planenum);
				if(crashstat != (ps[planenum].status & CRASH)) {
					LogIt (SLG_AAA_DESTROY, planenum, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, src, -999, 0L, LOG_AIR);
	                TXA (" destroyed by ");
        	        TXA ("AAA");
            	    Message (Txt,GAME_MSG);
				}
				else {
					LogIt (SLG_AAA_HIT, planenum, ps[planenum].xL, ps[planenum].yL, ps[planenum].alt, src, -999, 0L, LOG_AIR);
				}
			}
        }
    }
}



