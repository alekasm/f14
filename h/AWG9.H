//***************************************************************************
//*
//*    AWG9.H
//*
//*    Author: Mike McDonald
//*
//*    Fleet Defender: F-14 Tomcat
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//***************************************************************************

// Radar Difficulty Levels

#define	 AWG9_EASY		0
#define	 AWG9_MODERATE	1
#define	 AWG9_AUTHENTIC	2


// RIO Assist Levels

#define RIO_ASSIST_STANDARD		0
#define RIO_ASSIST_MODERATE		1
#define RIO_ASSIST_AUTHENTIC	2

// AWG9 - RADAR MODES

#define  AWG9_BREAK		0
#define  AWG9_PDSRCH	20
#define  AWG9_TWSA		30
#define	 AWG9_TWSM      40
#define	 AWG9_RWS		50
#define  AWG9_SPECIAL	60

#define  AWG9_SRCH_WIDE			100
#define  AWG9_SRCH_MEDIUM		102
#define	 AWG9_SRCH_NARROW		104
#define  AWG9_SRCH_AUTHENTIC	106

#define  AWG9_RWS_WIDE			110
#define  AWG9_RWS_MEDIUM		112
#define	 AWG9_RWS_NARROW		114
#define  AWG9_RWS_AUTHENTIC		116

#define	 AWG9_TWSA_MEDIUM		122
#define	 AWG9_TWSA_NARROW		124
#define	 AWG9_TWSA_AUTHENTIC	126

#define	 AWG9_TWSM_MEDIUM		132
#define	 AWG9_TWSM_NARROW		134
#define	 AWG9_TWSM_AUTHENTIC	136

#define	 AWG9_VSL				150
#define	 AWG9_BORESIGHT			154
#define	 AWG9_AUTOACQ			156

#define	 AWG9_STANDARD_SRCH		200
#define	 AWG9_STANDARD_TRACK	202

#define	 AWG9_PDSTT				300

//***************************************************************************

#define	BAR_1		0
#define	BAR_2		2
#define	BAR_4		4
#define	BAR_8		8
#define	BAR_SIZE	419


#define	RNG200		5
#define	RNG100      4
#define	RNG50       3
#define	RNG20       2
#define	RNG10       1
#define RNG5		0

#define  DEG65    0
#define  DEG40    1
#define  DEG20    2
#define  DEG10    3
#define	 DEG05	  4
#define  DEG03    5

#define  BITP10NM		475L
#define  BIT_2_NM(n)	(10L*n/BITP10NM)

//***************************************************************************
#define  Xasp(x)		(x)
#define  Yasp(y)		(y-(y>>3))
#define  Xaspn(x)		(x)
#define  Yaspn(y)		(y+(y>>3))

#define  DEG			182
#define  MAXTARGFIL		24
#define  MAX_CLSR		15

#define  BACK_CLR		0
#define  DDD_CLR		0x15
#define  HORZ_CLR		0x14
#define  BLIP_CLR		0x13
#define  CIRC_CLR		0x13
#define  CARAT_CLR		0x14

#define  DDD_X			0
#define  DDD_Y			0
#define  DDD_W			83
#define  DDD_H			66
#define  DDD_CTR_X		DDD_W/2
#define  DDD_CTR_Y		DDD_H/2

#define  RNG_X			53
#define  RNG_Y			4
#define  ALT_MIN_X		10
#define  ALT_MAX_X		26
#define  ALT_Y			4

#define  CVG_CIRC_X		DDD_X
#define  CVG_CIRC_Y		DDD_Y+DDD_H

#define  EL_CARAT_X		0
#define  AZ_CARAT_Y		DDD_H
#define  RR_CARAT_X		DDD_W

#define  ASPECT_X    22
#define  ASPECT_Y    4
#define  ASPECT_L    12

#define  HDG_X       35
#define  HDG_Y       4
#define  HDG_L       14

#define  TARGALT_X   40
#define  TARGALT_Y   48

#define  SC_X        33
#define  SC_Y        48

#define  EL_DELTA    2

#define  NM200		 9500		 // 200 NM (IN 15 BIT COORDS)
#define  NM130		 6175		 // 130 NM (IN 15 BIT COORDS)
#define  NM100		 4750		 // 100 NM (IN 15 BIT COORDS)
#define  NM80        3800        // 80 NM (IN 15 BIT COORDS)
#define  NM10        475         // 10 NM (IN 15 BIT COORDS)
#define  NM5         238         //  5 NM (IN 15 BIT COORDS)
#define  NM2         95          //  2 NM (IN 15 BIT COORDS)

#define  FT2000      500         // IN 20 BIT COORDS
#define  FT500       125         // IN 20 BIT COORDS

#define  TID_ATTK	 0
#define  TID_NAV     1

//***************************************************************************
typedef struct {
	int conv;       // flag: TRUE - coords converted long to short
	int ox,oy,oz;   // obj position rel to player in world  coords
	int rx,ry,rz;   // obj position rel to player in player coords
	int bx,by,bz;   // obj position rel to radar  in player coords
	int d;          // distance from player
	int x,y;        // position in CRT
	int i;          // index in ps buffer
	int az,el;      // azimuth/elevation angles
	unsigned h;		// heading relative to player
	long lastdist;	// last distance measurement
	long targetclosure;	// target closure
	int lasttime;	// last time measurement
	int tx,ty;		// position in TID
	int id;			// Friend/Foe/Unkown IFF
} TARGET;

typedef struct {
	int status;
	int target;
	int ssi_index;
	int src_station;
} DTARGET;

typedef struct {
	int x;
	int y;
	int i;
} TIDTarg;

//***************************************************************************
extern 	 int	  SELECTED_BAR_NUM;
extern 	 int	  SELECTED_AZ;
extern	 int	  AWG9_REALITY_LEVEL;
extern   unsigned AWG9_MODE;
extern   int      MAXRNG;
extern   int      RANGES[];
extern   int      BRANGES[];
extern   int      RANGE;
extern   int      DESIGNATED;
extern   int      LOCKED;
extern   long     DESIGNATED_X;
extern   long     DESIGNATED_Y;
extern   unsigned PDT_ALT_1000;
extern   unsigned PDT_ALT_100;
extern   int      PDT_RNG;
extern   char     ASPECT_S[];
extern   int      SHOOT;
extern   int      ASE_RADIUS;
extern   int      PW;
extern   int      PDT_PROJ_X;
extern   int      PDT_PROJ_Y;
extern   int      PDT_PROJ;
extern   int      STEER_DOT_X;
extern   int      STEER_DOT_Y;
extern   int      STEER_DOT;
extern   int      TTI;
extern   int      CLOSURE;
extern   int      CLSR;
extern   int      AWG9_SNIFF;
extern   int      LOCKSHOOT;
extern   int      IFF_timer;
extern   int      proj_x,proj_y,proj_z;
extern	 int	  AZSWEEP;
extern	 int	  AZSCAN;
extern	 int	  NUM_BARS;
extern   int      TID_MODE;

