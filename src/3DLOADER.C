//***************************************************************************
//	FILE: 3DLOADER.C
//
//	Fleet Defender - F14 Tomcat
//	Author: Mike McDonald
//
//	Date: 8/23/93
//
//  Files Used:
//				F14PD01.DAT   Planes/Boats/Objects characteristics file
//              F14MD01.DAT   Missiles Characteristics file
//              F14RD01.DAT   Radar Characteristics file
//
//***************************************************************************
#include <stdio.h>
#include <fcntl.h>
#include <dos.h>
#include "types.h"
#include "io.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"

extern  char Txt [];
extern int EMSHANDLE;
extern int NEMSSEG;

struct rrec {
	int RadarEnumId;
	char name[10];
	int range;
	int detectiontype;
	int missiletype;
	int missilecount;
	int jam;
	int tewsnum;
};

struct mrec {
	int MissileEnumId;
	char name[12];
	int range;
	int minrange;
	int maxalt;
	int speed;
	int homing;
	int maneuver;
	int cmeffect;
	int damage;
	char filename[12];
	int type;
};

struct prec {
	int PlaneEnumId;
	char name[7];
	int speed;
	int maneuver;
	int rclass;
	char filename[13];
	int radarsig;
	int gun;
	int alignment;
	int ecm;
	int misl[3];
	int mcnt[3];
	int bclass;
};


#define MAX_PLANE_LIST_CNT		40
#define MAX_RADAR_LIST_CNT		30
#define MAX_MISSILE_LIST_CNT	30

int PlaneDataSize = (sizeof(struct PlaneData) >> 4) + 1;
int MissileDataSize = (sizeof(struct MissileData) >> 4) + 1;
int RadarDataSize = (sizeof(struct RadarData) >> 4) + 1;

int planerecsize = sizeof(struct prec);
int missilerecsize = sizeof(struct mrec);
int radarrecsize = sizeof(struct rrec);

int EMS3DObjectSlot=0;
unsigned int PlaneListSeg;
unsigned int RadarListSeg;
unsigned int MissileListSeg;

extern int EMSHANDLE;
extern int NEMSSEG;
extern int ExitErrorCode;

extern confData ConfigData;


char far *fstrcpy(char far *D, char far *S);


//***************************************************************************
int Load3DPlaneInfo(int PlaneEnumId)
{
	int i;
	int PlaneId=-1;
	FILE *PlaneDataFile;
	long recseek;
	struct prec planerec;
	int missile0, missile1;
	int returnval;


	//************************************************************
	// CHANGE THIS YOU LOOSER!!!

	if ((PlaneDataListCnt+1)>MAX_PLANE_LIST_CNT) {
		sprintf(Txt,"PlaneData List exceeds MAX_PLANE: %d %d",PlaneDataListCnt,
				MAX_PLANE_LIST_CNT);
		Message(Txt,DEBUG_MSG);
		return(-1);
	}

	PlaneId = PlaneSeek(PlaneEnumId);

	if (PlaneId != -1) return PlaneId;

	PlaneDataFile = fopen("F14PD01.DAT","rb");

	// Make Sure this NEVER happens - This should be a FATAL_ERROR
	if (PlaneDataFile==NULL) {
		Message("Plane Data File Not Found",DEBUG_MSG);
		exit(-1);
	}

	recseek = (long)(PlaneEnumId * planerecsize);
	fseek(PlaneDataFile,recseek,SEEK_SET);
	if (feof(PlaneDataFile))
		return -1;
	fread(&planerec,planerecsize,1,PlaneDataFile);

	// Make Sure this NEVER happens - This should be a FATAL_ERROR
	if (!LoadEMSObject(planerec.filename,EMS3DObjectSlot)) {
		sprintf(Txt,"Plane Object %d Not Found: %s\n",planerec.PlaneEnumId,planerec.filename);
		Message(Txt,DEBUG_MSG);
	}
	PlaneDataList[PlaneDataListCnt].objslot = EMS3DObjectSlot;
	EMS3DObjectSlot++;


	PlaneDataList[PlaneDataListCnt].PlaneEnumId = planerec.PlaneEnumId;
	fstrcpy(PlaneDataList[PlaneDataListCnt].name,planerec.name);
	PlaneDataList[PlaneDataListCnt].speed = planerec.speed;
	PlaneDataList[PlaneDataListCnt].maneuver = planerec.maneuver;

	// Get Radar class and store radar - look up missile info also
	// when storing radar - store missile

	RadarAdd(planerec.rclass);

	PlaneDataList[PlaneDataListCnt].rclass = planerec.rclass;

	PlaneDataList[PlaneDataListCnt].radarsig = planerec.radarsig;
	PlaneDataList[PlaneDataListCnt].gun = planerec.gun;
	PlaneDataList[PlaneDataListCnt].alignment = planerec.alignment;
	PlaneDataList[PlaneDataListCnt].ecm = planerec.ecm;


	missile0 = MissileAdd(planerec.misl[0]);
	PlaneDataList[PlaneDataListCnt].misl[0] = planerec.misl[0];
	missile1 = MissileAdd(planerec.misl[1]);
	PlaneDataList[PlaneDataListCnt].misl[1] = planerec.misl[1];
	missile1 = MissileAdd(planerec.misl[2]);
	PlaneDataList[PlaneDataListCnt].misl[2] = planerec.misl[2];


	// Get Missile class and store missile(s)
	PlaneDataList[PlaneDataListCnt].mcnt[0] = planerec.mcnt[0];
	PlaneDataList[PlaneDataListCnt].mcnt[1] = planerec.mcnt[1];
	PlaneDataList[PlaneDataListCnt].mcnt[2] = planerec.mcnt[2];

	PlaneDataList[PlaneDataListCnt].bclass = planerec.bclass;

	PlaneDataListCnt++;

	fclose(PlaneDataFile);
	returnval = PlaneDataListCnt -1;

	if(missile0 < 0)   /*  Negative Value indicates Cruise Missile, number is index into planes array   SRE  */
		Load3DPlaneInfo(-missile0);
	if(missile1 < 0)
		Load3DPlaneInfo(-missile1);

	return (returnval);
}


//***************************************************************************
int PlaneSeek(int PlaneEnumId)
{
	int i;
	int cnt=-1;

	for (i=0; i<PlaneDataListCnt; i++) {
		if (PlaneDataList[i].PlaneEnumId == PlaneEnumId) {
			cnt=i;
			break;
		}
	}

	return (cnt);
}

//***************************************************************************
int RadarAdd(int RadarEnumId)
{
	int i;
	int cnt=-1;
	FILE *RadarDataFile;
	long radarrecseek;
	struct rrec radarrec;

	// RadarAdd will search the loaded radar characteristics for the RadarEnumId
	// If the radar is found, true is returned
	// If the radar is not found, the characteristics is loaded and true returned
	// If a radar is not found in the file, then return false

	if ((RadarDataListCnt+1) > MAX_RADAR_LIST_CNT) {
		Message("RadarData List exceeded MAX_RADAR_LIST_CNT",DEBUG_MSG);
		return(-1);
	}

	for (i=0; i<RadarDataListCnt; i++) {
		if (RadarDataList[i].RadarEnumId == RadarEnumId) {
			cnt=i;
			break;
		}
	}

	if (cnt != -1) return(1);

	// Search file for Radar Entry
	RadarDataFile = fopen("F14RD01.DAT","rb");

	if (RadarDataFile==NULL) {
		Message("Radar Data File Not Found",DEBUG_MSG);
		exit(-1);
	}

	radarrecseek = (long)(RadarEnumId * radarrecsize);
	fseek(RadarDataFile,radarrecseek,SEEK_SET);
	// If EOF, then we have a problem, invalid ID, return error
	if (feof(RadarDataFile)) {
		Message("Radar Data File Read Error",DEBUG_MSG);
		fclose(RadarDataFile);
		return (0);
	}

	fread(&radarrec,radarrecsize,1,RadarDataFile);

	RadarDataList[RadarDataListCnt].RadarEnumId = radarrec.RadarEnumId;
	fstrcpy(RadarDataList[RadarDataListCnt].name,radarrec.name);
	RadarDataList[RadarDataListCnt].range = ((radarrec.range * 16)/10);
	RadarDataList[RadarDataListCnt].detectiontype = radarrec.detectiontype;

	// Check if missile is there, if not add type
	MissileAdd(radarrec.missiletype);
	RadarDataList[RadarDataListCnt].missiletype = radarrec.missiletype;

	RadarDataList[RadarDataListCnt].missilecount = radarrec.missilecount;
	RadarDataList[RadarDataListCnt].jam = radarrec.jam;
	RadarDataList[RadarDataListCnt].tewsnum = radarrec.tewsnum;

	RadarDataListCnt++;
	fclose(RadarDataFile);
	return (1);
}

//***************************************************************************
int MissileAdd(int MissileEnumId)
{
	int i;
	int cnt=-1;
	FILE *MissileDataFile;
	long missilerecseek;
	struct mrec missilerec;

	if ((MissileDataListCnt+1) > MAX_MISSILE_LIST_CNT) {
		Message("MissileData List exceeded MAX_MISSILE_LIST_CNT",DEBUG_MSG);
		return(-1);
	}


	for (i=0; i<MissileDataListCnt; i++) {
		if (MissileDataList[i].MissileEnumId == MissileEnumId) {
			cnt=i;
			break;
		}
	}

	if (cnt != -1) return(1);

	// Search file for Missile Entry
	MissileDataFile = fopen("F14MD01.DAT","rb");

	if (MissileDataFile==NULL) {
		Message("Missile Data File Not Found",DEBUG_MSG);
		exit(-1);
	}

	missilerecseek = (long)(MissileEnumId * missilerecsize);
	fseek(MissileDataFile,missilerecseek,SEEK_SET);
	// If EOF, then we have a problem, invalid ID, return error
	if (feof(MissileDataFile)) {
		fclose(MissileDataFile);
		return (0);
	}
	fread(&missilerec,missilerecsize,1,MissileDataFile);

	MissileDataList[MissileDataListCnt].MissileEnumId = missilerec.MissileEnumId;
	fstrcpy(MissileDataList[MissileDataListCnt].name,missilerec.name);
	MissileDataList[MissileDataListCnt].range = ((missilerec.range*16)/10);  // KLUDE - nmi-to-km
	MissileDataList[MissileDataListCnt].minrange = ((missilerec.minrange*16)/10);
	MissileDataList[MissileDataListCnt].maxalt = missilerec.maxalt;
	MissileDataList[MissileDataListCnt].speed = missilerec.speed;
	MissileDataList[MissileDataListCnt].homing = missilerec.homing;
	MissileDataList[MissileDataListCnt].maneuver = missilerec.maneuver;
	MissileDataList[MissileDataListCnt].cmeffect = missilerec.cmeffect;
	MissileDataList[MissileDataListCnt].damage = missilerec.damage;
	MissileDataList[MissileDataListCnt].objslot = EMS3DObjectSlot;
	MissileDataList[MissileDataListCnt].type = missilerec.type;

	// Make Sure this NEVER happens - This should be a FATAL_ERROR
	if (!LoadEMSObject(missilerec.filename,EMS3DObjectSlot)) {
		sprintf(Txt,"Missile Object %d Not Found: %s\n",missilerec.MissileEnumId,missilerec.filename);
		Message(Txt,DEBUG_MSG);
	}

	EMS3DObjectSlot++;
	MissileDataListCnt++;
	fclose(MissileDataFile);

	if(missilerec.type < 0)
		return(missilerec.type);

	return(1);
}



//***************************************************************************
int GetPicType(int PlaneNo)
{
	if (PlaneNo <0) {
		TXT("Bad Plane Pic Type < 0");
		TXN(PlaneNo);
		Message(Txt,DEBUG_MSG);
		return -1;
	}

	if (PlaneNo > PlaneDataListCnt) {
		Message("Bad Plane Pic Type > PDLC",DEBUG_MSG);
		return -1;
	}

	return(PlaneDataList[PlaneNo].objslot);
}

//***************************************************************************
struct PlaneData far *GetPlanePtr(int PlaneNo)
{

	if (PlaneNo <0) {
		TXT("Bad Plane Pic Type < 0: ");
		TXN(PlaneNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}

	if (PlaneNo > PlaneDataListCnt) {
		TXT("Bad Plane Ptr type > PDLC");
		TXN(PlaneNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}
	return &(PlaneDataList[PlaneNo]);
}

//***************************************************************************
struct MissileData far *GetMissilePtr(int MissileNo)
{
	int i;
	int FoundNo=-1;

	if (MissileNo <0) {
		TXT("Bad Missile Type < 0: ");
		TXN(MissileNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}

	for (i=0; i<MissileDataListCnt; i++) {
		if (MissileDataList[i].MissileEnumId == MissileNo) {
			FoundNo = i;
			break;
		}
	}

	if (FoundNo == -1) {
		TXT("Bad Missile ID - Missile ID Not Found: ");
		TXN(MissileNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}
	return &(MissileDataList[FoundNo]);
}

//***************************************************************************
struct RadarData far *GetRadarPtr(int RadarNo)
{
	int i;
	int FoundNo=-1;

	if (RadarNo <0) {
		TXT("Bad Radar Type < 0: ");
		TXN(RadarNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}

	for (i=0; i<RadarDataListCnt; i++) {
		if (RadarDataList[i].RadarEnumId == RadarNo) {
			FoundNo = i;
			break;
		}
	}

	if (FoundNo == -1) {
		TXT("Bad Radar ID - Radar ID Not Found: ");
		TXN(RadarNo);
		Message(Txt,DEBUG_MSG);
		return (NULL);
	}
	return &(RadarDataList[FoundNo]);
}


//***************************************************************************
int Release3DMemory()
{
	if (EMSHANDLE)
		EMSReleasePages(EMSHANDLE);
}

//***************************************************************************
int Init3DPlanes()
{
	// FATAL_ERROR
	if (_dos_allocmem((PlaneDataSize*MAX_PLANE_LIST_CNT),&PlaneListSeg)) {
		ExitErrorCode=-12;
		InitAbort();
	}
	FP_SEG(PlaneDataList) = PlaneListSeg;
	FP_OFF(PlaneDataList) = 0;

	if (_dos_allocmem((RadarDataSize*MAX_RADAR_LIST_CNT),&RadarListSeg)) {
		ExitErrorCode=-16;
		InitAbort();
	}
	FP_SEG(RadarDataList) = RadarListSeg;
	FP_OFF(RadarDataList) = 0;

	if (_dos_allocmem((MissileDataSize*MAX_MISSILE_LIST_CNT),&MissileListSeg)) {
		ExitErrorCode=-14;
		InitAbort();
	}
	FP_SEG(MissileDataList) = MissileListSeg;
	FP_OFF(MissileDataList) = 0;
}

//***************************************************************************
int LoadSpecialObjects()
{
	int i,j,squad;

	char *CarrierAmerica[14] = {
		"AMERB14.3DZ",		// Base - Level 1
		"AMERT9.3DZ",		// Tower - Level 1 & 2
		"AMERT108.3DZ",		// Deck - Level 1 & 2
		"AMERWIR.3DZ",		// Wires
		"AMERLIT.3DZ",		// Ball
		"ABL1.3DZ",	  		// Blast Door 1
		"ABL2.3DZ",	   		// Blast Door 2
		"ABL3.3DZ",	   		// Blast Door 3
		"ABL4.3DZ",			// Blast Door 4
		"AMERTMB5.3DZ",		// Base - Level 2
		"AMERTSB2.3DZ",		// Base - Level 3
		"AMERTST1.3DZ",		// Tower - Level 3
		"AMERTSS4.3DZ",		// Carrier - Level 4
		"AMERWAKE.3DZ"		// Wake
	};

	char *CarrierNimitz[14] = {
		"NIMB08.3DZ",		// Base - Level 1
		"NIMNT06.3DZ",		// Tower - Level 1 & 2
		"NIMT109.3DZ",		// Deck - Level 1 & 2
		"WIREN.3DZ",		// Wires
		"LIGHTN.3DZ",		// Ball
		"NBL1.3DZ",			// Blast Door 1
		"NBL2.3DZ",			// Blast Door 2
		"NBL3.3DZ",			// Blast Door 3
		"NBL4.3DZ",	 		// Blast Door 4
		"NIMBM02.3DZ",		// Base - Level 2
		"NIMBS02.3DZ",		// Base - Level 3
		"NIMST02.3DZ",		// Tower - Level 3
		"NIMBSS01.3DZ",		// Carrier - Level 4
		"NIMWAKE.3DZ"		// Wake
	};

	char *CarrierSaratoga[14] = {
		"SARAB09.3DZ",		// Base - Level 1
		"SARATOW.3DZ",		// Tower - Level 1 & 2
		"SARAT108.3DZ",		// Deck - Level 1 & 2
		"WIRESR.3DZ",		// Wires
		"LIGHTSR.3DZ",		// Ball
		"SBL1.3DZ",			// Blast Door 1
		"SBL2.3DZ",			// Blast Door 2
		"SBL3.3DZ",			// Blast Door 3
		"SBL4.3DZ",			// Blast Door 4
		"SARABM03.3DZ",		// Base - Level 2
		"SARABS02.3DZ",		// Base - Level 3
		"SARATS01.3DZ",		// Tower - Level 3
		"SARABSS1.3DZ",		// Carrier - Level 4
		"SARAWAKE.3DZ"		// Wake
	};

	char *CoolF14Objects[9] = {
		"F14L09B.3DZ",
		"F14MNS1H.3DZ",
		"F14MPS1H.3DZ",
		"F14R09B.3DZ",
		"F14SMLO2.3DZ",
		"F14SMC01.3DZ",
		"F14SMR01.3DZ",
		"F14SMM03.3DZ",
		"F14SMX01.3DZ",
	};

	char *CoolF14Tails[22] = {
//		"F14LTSAN.3DZ",
//		"F14RTSAN.3DZ",
		"F14LT014.3DZ",	// Tophatters VF-14
		"F14RT014.3DZ",
		"F14LT032.3DZ", // Swordsman  VF-32
		"F14RT032.3DZ",
		"F14LT033.3DZ",	// Starfighters VF-33
		"F14RT033.3DZ",
		"F14LT041.3DZ",	// Black Aces	VF-41
		"F14RT041.3DZ",
		"F14LT074.3DZ",	// Be-Devilers	VF-74
		"F14RT074.3DZ",
		"F14LT084.3DZ",	// Jolly Rogers VF-84
		"F14RT084.3DZ",
		"F14LT102.3DZ",	// Diamondbacks	VF-102
		"F14RT102.3DZ",
		"F14LT103.3DZ",	// Sluggers VF-103
		"F14RT103.3DZ",
		"F14LT142.3DZ",	// Ghostriders VF-142
		"F14RT142.3DZ",
		"F14LT143.3DZ",	// Pukin' Dogs VF-143
		"F14RT143.3DZ",
		"F14LT101.3DZ",
		"F14RT101.3DZ"
	};

	char *Missiles[7] = {
		"SDWNDR3.3DZ",
		"SPARROW3.3DZ",
		"PHOENIX.3DZ",
		"FUELTANK.3DZ",
		"HARPOON.3DZ",
		"SDWNDR.3DZ",
		"MK82.3DZ"
	};

	char *MiscSpecial[9] = {
		"SHADOW.3DZ",
		"AMERCHUT.3DZ",
		"RAMP02.3DZ",
		"TRPLAAA.3DZ",
		"CANOPY06.3DZ",
		"CHAFF.3DZ",
		"FLARE.3DZ"
	};

	// Load Carrier
	for (i=0; i<14; i++) {
		switch(ConfigData.Carrier) {
			case 0:
				if (!LoadEMSObject(CarrierAmerica[i],i)) {
					sprintf(Txt,"Special Object Not Found: %s\n",CarrierAmerica[i]);
					Message(Txt,DEBUG_MSG);
				}
			break;
			case 1:
		   		if (!LoadEMSObject(CarrierNimitz[i],i)) {
					sprintf(Txt,"Special Object Not Found: %s\n",CarrierNimitz[i]);
					Message(Txt,DEBUG_MSG);
				}
			break;
			case 2:
				if (!LoadEMSObject(CarrierSaratoga[i],i)) {
					sprintf(Txt,"Special Object Not Found: %s\n",CarrierSaratoga[i]);
					Message(Txt,DEBUG_MSG);
				}
			break;
			default:
   				if (!LoadEMSObject(CarrierAmerica[i],i)) {
					sprintf(Txt,"Special Object Not Found: %s\n",CarrierAmerica[i]);
					Message(Txt,DEBUG_MSG);
				}
			break;
		}
	}

	// Load F-14

	for (j=0,i=14; i<23; j++,i++) {
   		if (!LoadEMSObject(CoolF14Objects[j],i)) {
			sprintf(Txt,"Special Object Not Found: %s\n",CoolF14Objects[j]);
			Message(Txt,DEBUG_MSG);
		}
	}

	// Load F-14 Squadron

	squad = ConfigData.Squadron;
	if (squad<0) squad = 20;
	if (squad>9) squad = 20;
	squad = squad * 2;

   	if (!LoadEMSObject(CoolF14Tails[squad],23)) {
		sprintf(Txt,"Special Object Not Found: %s\n",CoolF14Tails[squad]);
		Message(Txt,DEBUG_MSG);
	}


   	if (!LoadEMSObject(CoolF14Tails[squad+1],24)) {
		sprintf(Txt,"Special Object Not Found: %s\n",CoolF14Tails[squad+1]);
		Message(Txt,DEBUG_MSG);
	}

	// Load Missiles

	for (j=0,i=25; i<32; j++,i++) {
   		if (!LoadEMSObject(Missiles[j],i)) {
			sprintf(Txt,"Special Object Not Found: %s\n",Missiles[j]);
			Message(Txt,DEBUG_MSG);
		}
	}

	// Misc.

	for (j=0,i=32; i<39; j++,i++) {
   		if (!LoadEMSObject(MiscSpecial[j],i)) {
			sprintf(Txt,"Special Object Not Found: %s\n",MiscSpecial[j]);
			Message(Txt,DEBUG_MSG);
		}
	}

	EMS3DObjectSlot=39;
	MissileAdd(ML_AIM54C);
	MissileAdd(ML_AIM7M);
	MissileAdd(ML_AIM9M);
}


