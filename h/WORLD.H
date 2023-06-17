#define UBYTE unsigned char
#define UWORD unsigned int
#define SBYTE signed char
#define SWORD signed int
#define ULONG unsigned long int
#define SLONG signed long int

#define MAXPAT 21  /*  Max Number of Landing Pattern Gates  */
#define MAXNITEPAT 5 /*  Max Number of Landing Pattern Gates  */

// #define MAXRADAR 	110
#define MAXRADAR 	40
#define MAXPLANE 	70   /*  was 40  IS NUM PLANES PLUS BOATS  */
#define NUMPTROOPS 	20
#define MAXGROUNDTARGET 60

#define TILE(a,b,c) ((a<<11)+(b<<9)+(c<<7)+64)
#define TILE3(a,b)  ((a<<11)+(b<<9)+256)

/* Scale factors: bits, Km, Pixels */
/* Bt = 1 foot/HTOV */
#define BT2PX  	8
#define KM2PX  	2
#define BT2KM  	6  /* 1 km == 64 bits */

#define HTOV   	5  /* convert horizontal distance to vertical */
#define KH2BF  	(HTOV+1)  /* convert knots per hour to bits per frame */
#define COORD 	unsigned

/****************************************************************************/
/* ------ WayPoints ------ */

#define NWAYPOINTS 4

struct WayPoint {
      unsigned  x;
      unsigned  y;
      long      xL;     // 20-bit Scott style coord
      long      yL;
};

#ifdef _include_data
  struct WayPoint wps[NWAYPOINTS] =
  {     0,  0,  0,  0,
        0,  0,  0,  0,
        0,  0,  0,  0,
        0,  0,  0,  0,
  };
  int NextWayPoint=0;
  int ChangeWayPoint=0;
  int WPMode;
#else
  extern struct WayPoint wps[];
  extern int NextWayPoint,ChangeWayPoint,WPMode;
#endif

/****************************************************************************/

struct Radars {  /*  10 bytes  */
	COORD  x;
	COORD  y;
	char  rclass;     /* Radar class (see r structure) */
	UBYTE alert;      /* Current alert level 0-255 */
	int   status;     /* Various status bits */
	int   pause;      /* Time to next radar search */
};

#define DEBUG_MSG		0
#define SYSTEM_MSG		1
#define RIO_MSG			2
#define WINGMAN_MSG		3
#define GAME_MSG		4
#define RIO_NOW_MSG		5
#define WING_MENU_MSG   6

/* Radar Status bits */
#define AIRBASE  1
#define RACTIVE  2
#define RSEEN    4
#define SHIP     8
#define RTRACK  16
#define NOAMMO  32

#define DESTROYED   128
#define NEUTRAL     0x0100
#define CARRIER     0x0200
#define FRIENDLY    0x0400  // Same as planes!  Don't change
#define DOUBLE      0x0800
#define TANKER      0x1000

#define PERMANENTRBITS (AIRBASE+SHIP+NEUTRAL+CARRIER+FRIENDLY+DOUBLE)

//***************************************************************************
#define SPRTXPLSEG  	0
#define SPEECHSEG   	1
#define MISCSPRTSEG		2
#define HEAD1SEG		3
#define HEAD2SEG		4
#define NrEMSHandles 	5

//***************************************************************************
#define	SHADOWPX	21
#define	CHUTEPX		22
#define BOMB500PX   23
#define MISSILEPX   19
#define SIDEWPX     MISSILEPX
#define HARPOONPX   MISSILEPX

#define CARRIER_OBJ_1	1
#define CARRIER_OBJ_2	2
#define CARRIER_OBJ_3	3
#define CARRIER_OBJ_4	4
#define CARRIER_OBJ_5	5
#define CARRIER_OBJ_6	6
#define CARRIER_OBJ_7	7
#define CARRIER_OBJ_8	8
#define CARRIER_OBJ_9	9
#define CARRIER_OBJ_10	10
#define CARRIER_OBJ_11	11

#define	COOLF14_OBJ		30
#define	PLANES_OBJ		40
#define	CHUTE_OBJ		50
#define FLARE_OBJ		55
#define CHAFF_OBJ		56
#define	SHADOW_OBJ		60
#define MISSILE_OBJ		70
#define SPRTXPL_OBJ		80
#define REG_BOAT_OBJ	90

//***************************************************************************
#define OPTIC   1
#define RADAR   2

#define IR_1 	1	// Tail Aspect IR
#define IR_2 	2	// Full Aspect IR
#define RDR_1	3	// Semi-Active Radar Homing
#define RDR_2	4	// Full-Active Radar Homing (AAMRAAM)
#define GUN		5


/****************************************************************************/

enum {
SO_AN_22a,
SO_AN_22b,
SO_AN_26a,
SO_AN_26b,
SO_BE_12a,
SO_F_5a,
SO_F_16a,
SO_F_16b,
SO_F_16c,
SO_G_4a,
SO_IL_38a,
SO_J_22a,
SO_MIG_21a,
SO_MIG_21b,
SO_MIG_21c,
SO_MIG_21d,
SO_MIG_23a,
SO_MIG_23b,
SO_MIG_23c,
SO_MIG_23d,
SO_MIG_25a,
SO_MIG_25b,
SO_MIG_25c,
SO_MIG_27a,
SO_MIG_27b,
SO_MIG_27c,
SO_MIG_29a,
SO_MIG_29b,
SO_MIG_29c,
SO_MIG_29d,
SO_MIG_29e,
SO_MIG_29f,
SO_MIG_31a,
SO_MIG_31b,
SO_MIG_31c,
SO_F_1a,
SO_F_1b,
SO_SU_17a,
SO_SU_17b,
SO_SU_17c,
SO_SU_22a,
SO_SU_22b,
SO_SU_22c,
SO_SU_22d,
SO_SU_24a,
SO_SU_24b,
SO_SU_24c,
SO_SU_24d,
SO_SU_24e,
SO_SU_25a,
SO_SU_25b,
SO_SU_25c,
SO_SU_25d,
SO_SU_25e,
SO_SU_27a,
SO_SU_27b,
SO_SU_27c,
SO_TU_16AA,
SO_TU_16DA,
SO_TU_16GA,
SO_TU_16GB,
SO_TU_16HA,
SO_TU_16JA,
SO_TU_16LA,
SO_TU_22a,
SO_TU_22b,
SO_TU_22c,
SO_TU_22MA,
SO_TU_95DA,
SO_TU_95GA,
SO_TU_126a,
SO_TU_142a,
SO_VIGGENA,
SO_VIGGENB,
SO_YAK_38a,
SO_YAK_38b,
SO_YAK_38c,
SO_A_6EA,
SO_A_6EB,
SO_A_6EC,
SO_A_6ED,
SO_A_7EA,
SO_A_7EB,
SO_A_7EC,
SO_B_52a,
SO_B_52b,
SO_B_52c,
SO_C_130a,
SO_E_2CA,
SO_EA_6BA,
SO_F_A_18a,
SO_F_A_18b,
SO_F_A_18c,
SO_F_A_18d,
SO_F_111a,
SO_F_111b,
SO_GR_1a,
SO_GR_1b,
SO_KA_6a,
SO_MK_3a,
SO_NIMRODA,
SO_P_3a,
SO_S_3a,
SO_KA_25a,
SO_KA_25b,
SO_KA_27a,
SO_KA_27b,
SO_MI_8a,
SO_MI_8b,
SO_MI_14a,
SO_MI_24a,
SO_SH_2a,
SO_SH_2b,
SO_SH_3a,
SO_SH_3b,
SO_SH_60a,
SO_MH_53a,
SO_MH_53b,
SO_NIMITZA,
SO_KHAWKA,
SO_FORSTALA,
SO_CGA,
SO_DDGA,
SO_FFGA,
SO_LSTA,
SO_SOVCVA,
SO_SOVCVHA,
SO_SOVCGHA,
SO_SOVBCGA,
SO_SOVCGA,
SO_SOVDDGA,
SO_SOVFFGA,
SO_SOVFFA,
SO_SOVPTMA,
SO_SOVLSTA,
SO_SOVAFSA,
SO_RAFTA,
SO_RAFTB,
SO_F_14FDA,
SO_F_14FDB,
SO_F_14FDC,
SO_F_14MCA,
SO_F_14MCB,
SO_AS_4,
SO_AS_6,
SO_F_14SPE,
SO_AS_5,
SO_AS_7,
SO_AS_9,
SO_AS_10,
SO_AS_12,
SO_EXOCET,
SO_HARM,
SO_HARPOON,
SO_PENQUIN,
SO_SEAEAGLE,
SO_TOMAHAWK,
SO_GT,
SO_DC10,
SO_EC130Q,
SO_EC130H,
SO_F_4,
SO_F_14TARPS,
SO_WALLEYE,
SO_F_14MCC,
SO_RESERV3,
SO_RESERV4,
SO_RESERV5,
SO_COUNT} ;


#define ML_NONE		0
#define ML_AGM65	27
#define ML_HARM		26
#define ML_HARPOON	27
#define ML_AIM7M	46
#define ML_AIM9M	47
#define ML_AIM54C	48
#define ML_AIM120	49
#define ML_FFBOMB	50
#define ML_LGBOMB	51
#define ML_RTBOMB	52
#define ML_AS4      18
#define ML_AS6		20

#define RL_NONE		0

//***************************************************************************
// Behavior Classifications
#define FIGHTER_OBJCLASS	10
#define BOMBER_OBJCLASS		20
#define TRANSPORT_OBJCLASS  30
#define HELICOPTER_OBJCLASS 40
#define CIVILIAN_OBJCLASS	50
#define CARRIER_OBJCLASS	60
#define SHIP_OBJCLASS		70
#define CMISSILE_OBJCLASS   80
#define GROUND_TARGETCLASS  90
#define RADAR_PLANE_OBJCLASS 100


// Gun Classification
#define NO_GUN 0
#define FRONT_REAR_GUNS 1
#define FRONT_GUNS 2
#define REAR_GUNS 3

/* Status bits */
#define ALIVE  		0x1
#define ACTIVE 		0x2
#define BACKTOBASE 	0x4
#define ALERTED 	0x8
#define PANIC   	0x10
#define CRASH   	0x20
#define LOWDROLL  	0x40
#define JUSTPUT  	0x80
#define NEUTRAL 	0x0100
#define LANDING 	0x0200
#define FRIENDLY 	0x0400
#define CIVILIAN	0x0800
#define LANDED  	0x1000
#define PITCHUP 	0x2000
#define LOCK    	0x4000
#define TAKEOFF 	0x8000

//***************************************************************************
#define PERMANENTBITS (ALIVE+FRIENDLY)

struct Planes {   /*  38 bytes  */
      int   base;
      COORD  x;
      COORD  y;
      int   alt;
      long  xL;     /* Scott style 20-bit coords (y is same sense as 15-bit) */
      long  yL;
      int   head;
      int   pitch;
      int   roll;
      int   type;
      int   status;
      int   speed;
      int   time;      // not used
      int   color;     // not used
      int   alert;
      int   mcnt[3];   // missiles left
};

struct PlaneData {
	int PlaneEnumId;	// enum id
	char name[7];
	int speed;
	int maneuver;
	int rclass;		// Radar class
	int objslot;	// Slot of object in Scott's 3D mem.
	int radarsig;	// Radar Signature
	int gun;
	int alignment;
	int ecm;
	int misl[3];	// Missile Types
	int mcnt[3];	// # of each missile
	int bclass;		// Behavior class
};

struct RadarData {
	int RadarEnumId;
	char name[10];
	int range;
	int detectiontype;
	int missiletype;
	int missilecount;
	int jam;
	int tewsnum;
};

struct MissileData {
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
	int objslot;
	int type;
};


#ifdef _include_data
struct PlaneData far *PlaneDataList=NULL;
struct RadarData far *RadarDataList=NULL;
struct MissileData far *MissileDataList=NULL;
int PlaneDataListCnt=0;
int RadarDataListCnt=0;
int MissileDataListCnt=0;
#else
extern struct PlaneData far *PlaneDataList;
extern struct RadarData far *RadarDataList;
extern struct MissileData far *MissileDataList;
extern int PlaneDataListCnt;
extern int RadarDataListCnt;
extern int MissileDataListCnt;
#endif


//***************************************************************************
struct Mission {  /*  14 bytes  */
	int type;
	int target;
	int base;
	long x;       // 20-bit Scott style coords
	long y;
};

#ifdef _include_data
int  NRADAR,NERADAR,NTARGET,NPLANES;
struct Mission ms[3];
struct Radars Rdrs[MAXRADAR];
struct Planes ps[MAXPLANE];
#else
extern int  NRADAR,NERADAR,NTARGET,NPLANES;
extern struct Radars Rdrs[];
extern struct Planes ps[];
extern struct Mission ms[];
#endif

struct Behavevars {  /*  51 bytes  */
	void (*Behavior)();	  /* Pointer to function used for AI behavior */
	int  leader;    /*  points to wing leader  */
	int  wingman;   /*  points to wingman  */
	int  prevsec;   /*  points to previous section, usally a pair.  Allows larger group tactics  */
	int  nextsec;   /*  points to next section, usally a pair.  Allows larger group tactics  */
	int  target;    /*  int to target usually air,  assumes array  */
	char path;      /*  int to path, use groundt for waypoint  */
	int  groundt;   /*  int to ground target   */
	int  finhead;   /*  final heading  */
	int	 basespeed; /*  speed at level flight, used so that planes won't always be going max speed */
	int  behaveflag;/*  can be used to set flags for behaviors  */
	int  varcnt;    /*  counter to be used when needed  */
	int  behavecnt; /*  counter that can be used to vary behaviors in the same situation  */
	int	 verticlesep;  /*  verticle seperation for formations and stuff like that  */
	int	 horizontalsep;  /*  horizontal seperation for formations and stuff like that  */
	long xloc;     /*  holds x location of target or area to be protected  */
	long yloc;     /*  holds y location of target or area to be protected  */
	int  threathead;  /*  holds the head heading for CAP flying  */
	int	 fueltank;	/*  holds how much fuel is in the fuel tank  */
	int	 winglevel;  /*  holds the experience level of ai  */
	char orgnum;     /*  holds what object of Neds this is  */
	char squadnum;   /*  holds what squad member he is  */
};

struct AILoc{  /*  8 bytes  */
	long xloc;
	long yloc;
};

struct ChuteStuff {
	COORD chuteX;
	COORD chuteY;
	int	chuteZ;
	int	chuteDZ;
};

struct GroundTarget {
	long x;
	long y;
	int orgnum;
	int gflags;
};

#ifdef ELFAI
    struct Behavevars bv[MAXPLANE];
	struct AILoc ailoc[MAXPLANE];
	struct ChuteStuff paratroopers[NUMPTROOPS];
	int behavecommands;
	int PlayerCat;
	char OurCallSign[20];
	int OurPath;
	int LASTPLANE;   /*  Separation between boats and planes  */
	int DOINVISIBLE;
	int GLowDroll;
	int carrmoveok;
	struct GroundTarget gt[MAXGROUNDTARGET];
	int NGTARGETS;
	int warstatus;
	int ENEMYNODIE;
	int planelanding;
	int landingstatus;
	UWORD KnockItOffStatus;
	int lastCounter;
	int lastOClock;
	int tempgotcha;
	int minTimeAloft;
	unsigned long TotalGameTicks;
#else
    extern struct Behavevars bv[];
	extern struct AILoc ailoc[];
	extern struct ChuteStuff paratroopers[];
	extern int behavecommands;
	extern int PlayerCat;
	extern char OurCallSign[];
	extern int OurPath;
	extern int LASTPLANE;   /*  Separation between boats and planes  */
	extern int DOINVISIBLE;
	extern int GLowDroll;
	extern int carrmoveok;
	extern struct GroundTarget gt[];
	extern int NGTARGETS;
	extern int warstatus;
	extern int ENEMYNODIE;
	extern int planelanding;
	extern int landingstatus;
	extern UWORD KnockItOffStatus;
	extern int lastCounter;
	extern int lastOClock;
	extern int tempgotcha;
	extern int minTimeAloft;
	extern unsigned long TotalGameTicks;
#endif


/****************************************************************************/
/*                                                                          */
/*                      TARGET INFO                                         */
/*                                                                          */
/****************************************************************************/
/******* NOTE: These offsets are in 20-bit "Andy" style coordinates *********/

struct Target {      /*  12 bytes  */
    UBYTE   Type;
    UBYTE   Hardness;
    int     Points;
    int     Xsiz;       /* 1/2 the X dimension of the box */
    int     Ysiz;       /* 1/2 the Y dimension of the box */
    int     Xoff;       /* Group targets must NOT have an offset! */
    int     Yoff;
};

#define AAATGT       1
#define NAVALTGT     2
#define RADARTGT     4
#define RUNWAYTGT    8
#define HARDTGT      16  /* if not HARD, then it is SOFT */
#define SACREDTGT    32  /* cannot blow these up */
#define GROUPTGT     128 /* If GROUP target, "Hardness" is index of element
                         /* in GTargets array for first target in group */
#define LASTTGT      128
#define MAXTARGETS      64  /* Maximum # of unique 3D objects in a world */
#define MAXGROUPTARGETS 56

