/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          loadmiss.c

    Author:             Ned Way

    File Date:          16:25:26  Wed  23-Sep-1992

    File Description:
                        external load mission source for f15iii


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#include "library.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "string.h"
#include "setup.h"
#include "planeai.h"
#include "proto.h"
#include "speech.h"

#include <bios.h>
#include <dos.h>

#include <stdio.h>
#include <process.h>

UBYTE lastadded;
int orgPlayernum;

extern int OurLoadOut;
int MissionWeather;
int CloudCeiling;

extern char OurWingName[];
extern int whichourcarr;     // holds value for which boat our carrier
extern UWORD GRPAGE0[];

extern RPS *Rp1;
#define NUM_AI_PATHS 40   /*  was 15  */
struct _path_ aipaths[NUM_AI_PATHS];
#define NUM_AI_WAYPTS 85  /*  was 70 30 40 */
struct _wayPoint_ aiwaypoints[NUM_AI_WAYPTS];
#define NUM_AI_ACTIONS 130  /*  was 120 100  */
struct _action_ aiactions[NUM_AI_ACTIONS];
extern  long    PxLng, PyLng;
extern  int     OurHead,OurRoll, OurPitch;
extern  UWORD   OurAlt;
extern  UWORD   TimeOfDay;
extern  UWORD   Alt;
extern  UWORD   AirSpeed;
extern  SWORD   ThrustI;
extern  int     THRUST;
extern   int   ONTHECAT;
extern   int   ONTHEHOOK;
extern   int   TRIM;

int World;

void AddGroundTarget(struct _object_ one_object, int cnt);
extern struct RadarData far *GetRadarPtr(int RadarNo);
extern struct PlaneData far *GetPlanePtr(int PlaneNo);

int LoadScottMission()
{

    int     cnt, cnt2;
    FILE    *File;
	struct _missionHeader_ start_data;
	struct _object_ one_object;
	int planeid, objtype, objside, behavenum, numwingadd;
	long xval, yval;
	long nxval, nyval;
	long txval, tyval;
	UWORD nzval, tzval;
	int altval;
	int headval, path, speedval;
	int numobjects;
	int numGTs;
	int numpaths;
	int numwaypoints;
	int numactions;
	int formnum;
	int attack;
	int launch;
	int whatside, howgood;
	int numjump = 0;
	int otherplane;
	int firstwp;
	int lastalt = 1;
	int wpflag;
	int planeslot;
	int otherplane2;
	int bigplaneid;
	int tpath;
	struct PlaneData far *tempplane;
	int setediff = 0;

	whichourcarr = -999;     // holds value for which boat our carrier
	PlayerCat = -999;

    if ((File = fopen ("mission.f14", "rb")) == NULL) {
		Message("No Mission Data",SYSTEM_MSG);
        return 0;
    }

	NPLANES = 0;
	LASTPLANE = MAXPLANE;
	NGTARGETS = 0;
	warstatus = 2;
	DOINVISIBLE = 0;
	ENEMYNODIE = 0;
	KnockItOffStatus = 0;
	planelanding = -999;
	lastCounter = 0;
	lastOClock = 0;

    fread (&start_data, sizeof (struct _missionHeader_), 1, File);
	TimeOfDay = start_data.time;

	numobjects = start_data.numObjects;
	numGTs = start_data.numGTs;
	OurLoadOut = start_data.loadout;
	MissionWeather = start_data.weather;
	CloudCeiling = start_data.ceiling;
	World = start_data.world;
	minTimeAloft = start_data.minTimeAloft;

	numpaths = start_data.numPaths;
	numwaypoints = start_data.numWPs;
	numactions = start_data.numActions;

	if(numpaths >= NUM_AI_PATHS) {
		sprintf(Txt,"More paths %d than allocated space: %d",numpaths,NUM_AI_PATHS);
		Message(Txt,SYSTEM_MSG);
	}

	if(numwaypoints >= NUM_AI_WAYPTS) {
		sprintf(Txt,"More waypoints %d than allocated space: %d",numwaypoints,NUM_AI_WAYPTS);
		Message(Txt,SYSTEM_MSG);
	}

	if(numactions >= NUM_AI_ACTIONS) {
		sprintf(Txt,"More actions %d than allocated space: %d",numactions,NUM_AI_ACTIONS);
		Message(Txt,SYSTEM_MSG);
	}

#ifdef YEP
	if(numpaths >= NUM_AI_PATHS)
		Message("More paths than allocated space",SYSTEM_MSG);
	if(numwaypoints >= NUM_AI_WAYPTS)
		Message("More waypoints than allocated space",SYSTEM_MSG);
	if(numactions >= NUM_AI_ACTIONS)
		Message("More actions than allocated space",SYSTEM_MSG);
#endif

    fseek  (File, start_data.pathLoc, SEEK_SET) ;
    fread (&aipaths, sizeof (struct _path_), numpaths, File);

    fseek  (File, start_data.wpLoc, SEEK_SET) ;
    fread (&aiwaypoints, sizeof (struct _wayPoint_), numwaypoints, File);

    fseek  (File, start_data.actionLoc, SEEK_SET) ;
    fread (&aiactions, sizeof (struct _action_), numactions, File);

	for(cnt = 0; cnt < numwaypoints; cnt ++) {
		if(aiwaypoints[cnt].z <= 0) {
			GetWayPointChanges(cnt, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
			if(altval != -1) {
				aiwaypoints[cnt].z = altval;
				lastalt = altval;
			}
			else {
				aiwaypoints[cnt].z = lastalt;
			}
		}
	}

    fseek  (File, start_data.objectLoc, SEEK_SET) ;

#if 1
	for(cnt = 0; cnt < numobjects; cnt ++) {
		lastadded = cnt;
    	fread (&one_object, sizeof (struct _object_), 1, File);
		if(numjump > 0) {
			numjump --;
			continue;
		}
		if((one_object.object == SO_GT) || (one_object.type == ST_GROUND_TARGET) ||
			(one_object.type == ST_AIRPORT) || (one_object.type == ST_SA_2) ||
			(one_object.type == ST_SA_3) || (one_object.type == ST_SA_5) ||
			(one_object.type == ST_SA_6) || (one_object.type == ST_AAASITE) ||
			(one_object.type == ST_NOAAA)) {

			AddGroundTarget(one_object, cnt);
			continue;
		}
		if(((int)one_object.number & 0xF) == 0) {
			continue;
		}

		firstwp = aipaths[one_object.path].wp0;
		GetWayPointChanges(firstwp, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
/*		if(wpflag & DOWPLAND) {
			landingstatus |= LANDREQUESTED;
		}  */
/*		if(aiwaypoints[firstwp].x <= (long)-1) {
			// WHAT DO WE WANT TO DO ABOUT GUYS WHO ARE REFERENCED AFTER US
			// CAN CHECK Y VALUE TO SEE IF GUY HAS ALREADY BEEN DONE
			// REMEMBER WE CAN'T CHECK THIS UNTIL WE ARE ACTIVELY ADDING PLANES
		}  */

		if(one_object.formation != -1)
			formnum = one_object.formation;
		else if(formnum == -1)
			formnum = SF_CRUISE;

		if(behavenum == -1) {
			behavenum = SA_FORMATION;
		}

		if(headval == -1) {
			headval = one_object.heading;
		}

		if((altval == -1) || (one_object.z > -1)) {
			altval = (int)(one_object.z>>2);
			if(altval == -1) {
				altval = 0;
			}
		}
		if((altval < 25) && (!launch))
			altval = 2500;

		if((speedval == -1) || (one_object.speed > 0)) {
			speedval = one_object.speed;
		}
		if((speedval <= 0) && ((!launch) || (altval > 50))) {
			speedval = 300;
		}
		//***************************************************************
		// Planeid MUST be setup this way.
		// THIS ONLY SHOULD BE USED AT THE START OF THE GAME.  CONSULT
		// MIKE FOR FURTHER INFO.
		bigplaneid = one_object.object;
		if(one_object.type == ST_WINGMAN) {
			switch(OurLoadOut) {
				case SL_FD_ALPHA:
					bigplaneid = SO_F_14FDA;
					break;
				case SL_FD_BRAVO:
					bigplaneid = SO_F_14FDB;
					break;
				case SL_FD_CHARLIE:
					bigplaneid = SO_F_14FDC;
					break;
				case SL_MC_ALPHA:
					bigplaneid = SO_F_14MCA;
					break;
				case SL_MC_BRAVO:
					bigplaneid = SO_F_14MCB;
					break;
				case SL_MC_CHARLIE:
					bigplaneid = SO_F_14MCC;
					break;
			}
		}

		planeid = Load3DPlaneInfo(bigplaneid);

		objtype = one_object.type;
		xval = one_object.x;
		yval = one_object.y;
		if(wpflag & DOWPDBFM) {
	    	xval += sinXL((headval + 0x8000),2500L);
    		yval -= cosXL((headval + 0x8000),2500L);
		}
		else if(wpflag & DOWPOBFM) {
	    	xval += sinXL(headval,2500L);
    		yval -= cosXL(headval,2500L);
		}
		else if(wpflag & DOWPNBFM) {
	    	xval += sinXL((headval + 0x4000),2500L);
    		yval -= cosXL((headval + 0x4000),2500L);
		}

		numwingadd = ((int)one_object.number & 0xF) - 1;

		if(numwingadd <= 0)
			numwingadd = 0;

		numjump = numwingadd;

		path = one_object.path;
		whatside = one_object.side;
		howgood = ((int)one_object.quality & 0xF) + 1;
		if(path != -1) {
			firstwp = aipaths[path].wp0;
		}
		else {
			firstwp = -1;
		}

		if(one_object.type == ST_PLAYER) {
  			PxLng = xval;
			if(PxLng > (long)-1)
				PyLng = 0x100000 - yval;
			else
				PyLng = yval;
			OurAlt = altval<<2;
			if(launch == -1) {
				ONTHECAT = 0;
				TRIM = 0;
				Alt = altval;
				if(Alt > 70) {
		        	AirSpeed = 400*27;
		        	ThrustI = THRUST = 100;
		        	Status |= WHEELSUP;
	            	Status &= ~BRAKESON;
		        	EngineAlreadyOn ();
				}
			}
			else {
				PlayerCat = RequestCat((cnt & 3), -1);
				if(PlayerCat == 0)
					landingstatus |= LANDCAT0USED;
				else if(PlayerCat == 1)
					landingstatus |= LANDCAT1USED;
				if(PlayerCat >= 0)
					PreparePlane(PlayerCat, -1);
			}
			OurHead = headval;
			orgPlayernum = cnt;
			OurPath = path;

			if(strcmp(one_object.callSign, "                ")) {
				strcpy (OurCallSign, one_object.callSign);
			}
			else
				strcpy (OurCallSign, " ");

//			sprintf(Txt, "%d Player x %ld, y %ld, alt %d, head %x", cnt, PxLng, PyLng, OurAlt, OurHead);
//			Message(Txt);
		}
		else {
			if(one_object.type == ST_WINGMAN) {
				if(strcmp(one_object.callSign, "                ")) {
					strcpy (OurWingName, "(");
					strcat (OurWingName, one_object.callSign);
					strcat (OurWingName, ")");
				}
				else {
					strcpy (OurWingName, "(Wingman)");
				}
			}
			if(!setediff) {
				Diff = howgood -1;
				if(Diff >= 4)
					Diff = 3;
				else if (Diff < 0)
					Diff = 0;
				if(whatside != SS_FRIENDLY)
					setediff = 1;
			}
			planeslot = AddSomePlanes2 (cnt, planeid, behavenum, formnum, 1, numwingadd, whatside, objtype, xval, yval, altval, headval, path, speedval, howgood);
			bv[planeslot].groundt = firstwp;
			bv[planeslot].xloc = aiwaypoints[firstwp].x;
    		bv[planeslot].yloc = aiwaypoints[firstwp].y;
			if(planeslot > NPLANES) {
				bv[planeslot].verticlesep = one_object.damageLevel;
			}
		}
    }

    fseek  (File, start_data.gtLoc, SEEK_SET) ;

	for(cnt = 0; cnt < numGTs; cnt ++) {
    	fread (&one_object, sizeof (struct _object_), 1, File);

		AddGroundTarget(one_object, (cnt + numobjects));
	}


	for(cnt = 0; cnt < MAXPLANE; cnt ++) {
		if(ps[cnt].xL <= (long)-1) {

			otherplane = (int)ps[cnt].yL;

			otherplane2 = GetPlaneOrgNum(otherplane);
			GetPlanePointXYZ(otherplane, &nxval, &nyval, &nzval);
			if(ps[cnt].xL == (long)-2) {
				firstwp = bv[cnt].groundt + 1;
				txval = aiwaypoints[firstwp].x;
				nxval += aiwaypoints[firstwp].x;
				tyval = aiwaypoints[firstwp].y;
				nyval += aiwaypoints[firstwp].y;
				tzval = aiwaypoints[firstwp].z;
				nzval += aiwaypoints[firstwp].z;
			}

			if(nzval == 0) {
				ps[cnt].xL = nxval;
				ps[cnt].yL = nyval;
				ps[cnt].alt = 66;
			}
			else {
				ps[cnt].xL = nxval;
				ps[cnt].yL = nyval - (cnt * CRUISEDIST);
				ps[cnt].alt = nzval;
			}

			if(bv[cnt].Behavior == Ready5) {
				KeepPlaneOnCarrier(bv[cnt].threathead, cnt);
			}
			ps[cnt].x = ps[cnt].xL>>HTOV;
			ps[cnt].y = ps[cnt].yL>>HTOV;
			if((otherplane2 >= 0) && (otherplane2 < MAXPLANE)) {
				if(IsBoat(ps[otherplane2].type)) {
					ps[cnt].base = -(otherplane2 + 1);
				}
			}
		}
	}

	if(PxLng <= (long)-1) {
		otherplane = (int)PyLng;
		otherplane2 = GetPlaneOrgNum(otherplane);
		GetPlanePointXYZ(otherplane, &nxval, &nyval, &nzval);
		if(PxLng == (long)-2) {
			firstwp = aipaths[OurPath].wp0 + 1;
			nxval += aiwaypoints[firstwp].x;
			nyval += aiwaypoints[firstwp].y;
			nzval += aiwaypoints[firstwp].z;
		}
		PxLng = nxval;
		PyLng = 0x100000 - nyval;

		if((otherplane2 >= 0) && (otherplane2 < MAXPLANE)) {
			OurHead = ps[otherplane2].head;
			if(ps[otherplane2].alt > 66)
				Alt = ps[otherplane2].alt + 66;  /*  Don't Want Planes on top of each other  */
			if(IsBoat(ps[otherplane2].type)) {
				whichourcarr = otherplane2;     // holds value for which boat our carrier
				wps[0].xL = ps[otherplane2].xL;
				wps[0].yL = ps[otherplane2].yL;
				wps[0].x = ps[otherplane2].xL>>HTOV;
				wps[0].y = ps[otherplane2].yL>>HTOV;
				UpdateOurCarrier(otherplane2);
			}
		}
		OurAlt = Alt<<2;
	}

	if(whichourcarr == -999) {
		tpath = OurPath;
		if(tpath >= 0) {
			firstwp = aipaths[tpath].wp0 + aipaths[tpath].numWPs - 1;
			if(aiwaypoints[firstwp].x <= (long)-1) {
				otherplane2 = (int)aiwaypoints[firstwp].y;
				otherplane2 = GetPlaneOrgNum(otherplane2);
				if((otherplane2 >= 0) && (otherplane2 < MAXPLANE)) {
					if(IsBoat(ps[otherplane2].type)) {
						whichourcarr = otherplane2;
						wps[0].xL = ps[otherplane2].xL;
						wps[0].yL = ps[otherplane2].yL;
						wps[0].x = ps[otherplane2].xL>>HTOV;
						wps[0].y = ps[otherplane2].yL>>HTOV;
						UpdateOurCarrier(otherplane2);
					}
				}
			}
		}
	}

	if(whichourcarr == -999) {
		for(cnt2 = NPLANES; cnt2 < MAXPLANE; cnt2 ++) {
			if((ps[cnt2].status & ACTIVE) && (ps[cnt2].status & ALIVE) && (ps[cnt2].status & FRIENDLY)) {
				tempplane = GetPlanePtr(ps[cnt2].type);
				if(tempplane->bclass == CARRIER_OBJCLASS) {
					whichourcarr = cnt2;
				}
			}
		}
	}

	for(cnt2 = 0; cnt2 < MAXPLANE; cnt2 ++) {
		tpath = (int)bv[cnt2].path;
		if(tpath >= 0) {
			firstwp = aipaths[tpath].wp0 + aipaths[tpath].numWPs - 1;
			if(aiwaypoints[firstwp].x <= (long)-1) {
				otherplane2 = (int)aiwaypoints[firstwp].y;
				otherplane2 = GetPlaneOrgNum(otherplane2);
				if((otherplane2 >= 0) && (otherplane2 < MAXPLANE)) {
					if(IsBoat(ps[otherplane2].type)) {
						ps[cnt2].base = -(otherplane2 + 1);
					}
				}
			}
		}

#ifdef CHECKTHISOUT
		if ((ps[cnt2].status&ACTIVE) && (ps[cnt2].status & ALIVE)) {
			if((bv[cnt2].Behavior == Ready5) && (whichourcarr != -999) && (ps[cnt2].base < 0)) {
				ps[cnt2].base = (whichourcarr + 1) * -1;
			}
		}
#endif
	}

#endif

	fclose(File);

	MissileAdd(ML_FFBOMB);

	if((Status&TRAINING) && ENEMYNODIE && KnockItOffStatus)
		Speech(L_FIGHTS_ON);

    return 1 ;
}

#ifdef SHOWSCOTT
int pathshow = 0;
int ShowScottMission()
{

    int     cnt;
    FILE    *File;
	struct _missionHeader_ start_data;
	struct _object_ one_object;
	int planeid, objtype, objside, behavenum, numwingadd;
	long xval, yval;
	int altval;
	int headval, path, speedval;
	int numobjects;
	int numpaths;
	int numwaypoints;
	int numactions;
	int formnum;
	int attack;
	int launch;
	int firstwp;
	int whatside, howgood;
	int wpflag;

    if ((File = fopen ("mission.f14", "rb")) == NULL) {
		Message("No Mission Data",SYSTEM_MSG);
        return 0;
    }

    fread (&start_data, sizeof (struct _missionHeader_), 1, File);
	TimeOfDay = start_data.time;
	numobjects = start_data.numObjects;
	numpaths = start_data.numPaths;
	numwaypoints = start_data.numWPs;
	numactions = start_data.numActions;

#if 0
    fseek  (File, start_data.pathLoc, SEEK_SET) ;
    fread (&aipaths, sizeof (struct _path_), numpaths, File);

    fseek  (File, start_data.wpLoc, SEEK_SET) ;
    fread (&aiwaypoints, sizeof (struct _wayPoint_), numwaypoints, File);

    fseek  (File, start_data.actionLoc, SEEK_SET) ;
    fread (&aiactions, sizeof (struct _action_), numactions, File);
#endif


#if 0
	TXT(" ");
	TXN(numobjects);
	TXA(" ");
	TXN(numpaths);
	TXA(" ");
	TXN(numwaypoints);
	TXA(" ");
	TXN(numactions);
	TXA(" ");
	TXN(start_data.time);
	TXA(" ");
	TXN(start_data.weather);
	TXA(" ");
	Message(Txt);
#endif

#if 0
    fseek  (File, start_data.objectLoc, SEEK_SET) ;
	for(cnt = 0; cnt < numobjects; cnt ++) {
    	fread (&one_object, sizeof (struct _object_), 1, File);
		firstwp = aipaths[one_object.path].wp0;
		GetWayPointChanges(firstwp, &behavenum, &formnum, &speedval, &altval, &headval, &attack, &launch, &wpflag);
/*		if(aiwaypoints[firstwp].x <= (long)-1) {
			// WHAT DO WE WANT TO DO ABOUT GUYS WHO ARE REFERENCED AFTER US
			// CAN CHECK Y VALUE TO SEE IF GUY HAS ALREADY BEEN DONE
			// REMEMBER WE CAN'T CHECK THIS UNTIL WE ARE ACTIVELY ADDING PLANES
		}  */

		if(behavenum == -1) {
			behavenum = SA_FORMATION;
			formnum = one_object.formation;
		}

		if(headval == -1) {
			headval = one_object.heading;
		}

		if((altval == -1) || (one_object.z > -1)) {
			altval = (int)(one_object.z>>2);
		}
		if((altval < 25) && (!launch))
			altval = 2500;

		if((speedval == -1) || (one_object.speed > 0)) {
			speedval = one_object.speed;
		}
		if((speedval <= 0) && (!launch)) {
			speedval = 300;
		}

		//***************************************************************
		// Planeid MUST be setup this way.
		// THIS ONLY SHOULD BE USED AT THE START OF THE GAME.  CONSULT
		// MIKE FOR FURTHER INFO.

		planeid = Load3DPlaneInfo(one_object.object);

//		planeid = one_object.object;

		objtype = one_object.type;
		xval = one_object.x;
		yval = one_object.y;
		numwingadd = (int)one_object.number & 0xF;
		path = one_object.path;
		whatside = one_object.side;
		howgood = (int)one_object.quality & 0xF;

		if((cnt == -1) && (one_object.type == ST_PLAYER)) {
/*  			PxLng = one_object.x;
			PyLng = 0x100000 - one_object.y;
			OurAlt = one_object.altitude<<2;
			Alt = one_object.altitude;
			if(Alt > 0) {
		        AirSpeed = 400*27;
		        ThrustI = THRUST = 100;
		        Status |= WHEELSUP;
	            Status &= ~BRAKESON;
		        EngineAlreadyOn ();
			}
			OurHead = one_object.heading;   */

			sprintf(Txt, "%d Player x %ld, y %ld, alt %d, head %x", cnt, PxLng, PyLng, OurAlt, OurHead);
			Message(Txt);
		}
		else {
/*			planeid = one_object.object;
			objtype = one_object.type;
			behavenum = one_object.formation;
			numwingadd = (int)one_object.number & 0xF;
			xval = one_object.x;
			yval = one_object.y;
			altval = one_object.z;
			headval = one_object.heading;
			path = one_object.path;
			speedval = one_object.speed;  */
			if((cnt >= pathshow) && (cnt < pathshow + 2)) {
				sprintf(Txt, "%d NP id %d, t %d, bn %d, wa %d, q %d", cnt, planeid, objtype, one_object.formation, numwingadd, howgood);
				Message(Txt);
				sprintf(Txt, "%d xv %ld, yv %ld, zv %d, h %x, p %d, s %d", cnt, xval, yval, altval, headval, path, speedval);
				Message(Txt);
			}
		}
    }
	pathshow += 2;
	if(pathshow > numobjects)
		pathshow = 0;
#endif

#if 1
    for (cnt=0; cnt<numpaths; cnt++) {
		if((cnt >= pathshow) && (cnt < pathshow + 4)) {
			sprintf(Txt, "%d path wp0 %d, numpts %d", cnt, aipaths[cnt].wp0, aipaths[cnt].numWPs);
			Message(Txt);
		}
	}
	pathshow += 4;
	if(pathshow > numpaths)
		pathshow = 0;
#endif

#if 0
    for (cnt=0; cnt<numwaypoints; cnt++) {
		if((cnt >= pathshow) && (cnt < pathshow + 4)) {
			sprintf(Txt, "%d way x %ld, y %ld, z %d, t %d, d %d", cnt, aiwaypoints[cnt].x, aiwaypoints[cnt].y, aiwaypoints[cnt].z, aiwaypoints[cnt].actionIndex, aiwaypoints[cnt].numActions);
			Message(Txt);
		}
	}
	pathshow += 4;
	if(pathshow > numwaypoints)
		pathshow = 0;
#endif

#if 0
    for (cnt=0; cnt<numactions; cnt++) {
		if((cnt >= pathshow) && (cnt < pathshow + 4)) {
			sprintf(Txt, "%d actions action %d, value %d", cnt, aiactions[cnt].action, aiactions[cnt].value);
			Message(Txt);
		}
	}
	pathshow += 4;
	if(pathshow > numactions)
		pathshow = 0;
#endif

	fclose(File);

	InitScottFlight();

    return 1 ;
}
#endif

void AddGroundTarget(struct _object_ one_object, int cnt)
{
	int radarnum = 27;
	struct RadarData far *tempradar;

	if(NGTARGETS >= MAXGROUNDTARGET) {
		TXT("More ground targets ");
		TXN(NGTARGETS);
		TXT(" than allocated space: ");
		TXN(MAXGROUNDTARGET);
		Message(Txt,SYSTEM_MSG);
		return;
	}
	gt[NGTARGETS].x = one_object.x + 2048;
	gt[NGTARGETS].y = one_object.y + 2048;
	gt[NGTARGETS].orgnum = cnt;
	gt[NGTARGETS].gflags = (ALIVE|AAA_ACTIVE);

	if(one_object.side == SS_FRIENDLY) {
		gt[NGTARGETS].gflags |= FRIENDLY;
	}
	else if(one_object.side == SS_NEUTRAL) {
		gt[NGTARGETS].gflags |= NEUTRAL;
	}

	if((!strcmp("SA-2", one_object.name)) || (one_object.type == ST_SA_2))
		radarnum = 1;
	else if((!strcmp("SA-3", one_object.name)) || (one_object.type == ST_SA_3))
		radarnum = 2;
	else if((!strcmp("SA-5", one_object.name)) || (one_object.type == ST_SA_5))
		radarnum = 3;
	else if((!strcmp("SA-6", one_object.name)) || (one_object.type == ST_SA_6))
		radarnum = 4;
	else if((!strcmp("Crotale", one_object.name)) || (one_object.type == ST_CROTALE))
		radarnum = 12;
	else if((!strcmp("Hawk", one_object.name)) || (one_object.type == ST_HAWK))
		radarnum = 13;
	else if((!strcmp("Nike-Herc", one_object.name)) || (one_object.type == ST_NIKE_HERCULES))
		radarnum = 47;
	else if(one_object.type == ST_AIRPORT)
		radarnum = 27;
	else {
		if(one_object.type == ST_NOAAA) {
			gt[NGTARGETS].gflags &= ~AAA_ACTIVE;
		}
		NGTARGETS++;
		return;
	}
	NGTARGETS++;

	if(!RadarAdd(radarnum))
		return;

	if(NRADAR >= MAXRADAR) {
		TXT("More radars ");
		TXN(NRADAR);
		TXA(" than allocated space: ");
		TXN(MAXRADAR);
		Message(Txt,SYSTEM_MSG);
		return;
	}
	tempradar = GetRadarPtr(radarnum);
	if(!MissileAdd(tempradar->missiletype))
		return;

	Rdrs[NRADAR].x = (int)(((long)one_object.x + 2048)>>HTOV);
	Rdrs[NRADAR].y = (int)(((long)one_object.y + 2048)>>HTOV);
	Rdrs[NRADAR].rclass = (char)radarnum;
	if(one_object.type == ST_AIRPORT)
		Rdrs[NRADAR].status = AIRBASE;
	else
		Rdrs[NRADAR].status = 0;
	if(one_object.side == SS_FRIENDLY) {
		Rdrs[NRADAR].status |= FRIENDLY;
	}
	else if(one_object.side == SS_NEUTRAL) {
		Rdrs[NRADAR].status |= NEUTRAL;
	}
	NRADAR += 1;
}
