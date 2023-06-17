#define HUDX        48
#define HUDY        15
#define HUDXSIZ     (320-2*48)
#define HUDYSIZ     74
#define HUDFRAMEX   62
#define HUDFRAMEY   1
#define HUDFRAMELEN 92
#define HUDFRAMESIZ 197

#define CRTXSIZ         82
#define CRTYSIZ         57
#define CRTXCENT        (CRTXSIZ/2)
#define CRTYCENT        (CRTYSIZ/2)

#define LFTFRTMONOCRTX  119
#define LFTFRTMONOCRTY  138
#define RGTFRTMONOCRTX  119
#define RGTFRTMONOCRTY  138

#define RGTBAKMONOCRTX  172
#define RGTBAKMONOCRTY  124
#define RGTBAKCOLRCRTX  269
#define RGTBAKCOLRCRTY  129

#define BAKCOLRCRTPAN   (CRTXSIZ-(320-RGTBAKCOLRCRTX))

/* Left and right front cockpit masked blast areas (screen corners) */

#define LOOKDOWNDY  		76
#define SIZE3D              150
#define SIZE3DBACK          99
#define SIZE3DBACKBACK      109
#define SIZE3DFRONTBACK     111
#define FPStoKNTS 			27
#define S2L(a) ((long) a<<HTOV)


/* CRT Displays */
#define BLANK       0
#define HRM         1
#define RDR         2
#define VDI			3
#define TGTFLIR     4
#define HUDREP      5
#define WEAPONS     6
#define CAUTIONS    7
#define ADI         8
#define HSD         9
#define TSD         10
#define BLANKING    11
#define	TEWS		15
#define	DDD			16
#define TID			17

#define HUDCRT      14  // Faked up entry for mouse stuff

#define LASTDISPLAY BLANKING

/* Data structure defs */
struct	OBJ3D {
	int	NUM;
	int	X;
	int	Y;
};


struct CloseObj {
    int              ObjNr;
    long             X;
    long             Y;
    int              Dist;
    int              STAMPLOC;
    int              Index;
    UWORD            HitFlags;
    struct OBJ3D far *OP;
};

typedef struct _configData_ {
	int DiffLevel;
    int WeaponEffect;
	int FightModel;
	int Damage;
	int CarrierLanding;
	int RioAssist;
	int BackSeat;
	int GunCamera;
	int Radar;
	int CrashMode;
	int EnemySkill;
	int Carrier;
	int Squadron;
	int World;
} confData ;


/****************************************************************************/
#define ORANGEFLAME 	0x5C
#define BLUEFLAME   	0x5D
#define HISMISSLCOLOR 	0xE0
#define	HUD_COLOR		0x06
#define TSD_COLOR   	0x0a
#define TSD_MONO_COLOR 	0x97

/****************************************************************************/

#define COOLYCENTER 0
#define COOLYLEFT   1
#define COOLYDOWN   2
#define COOLYRIGHT  3
#define COOLYUP     4

/****************************************************************************/

#define angle(a,b) ARCTAN(a,b)

/****************************************************************************/

#define MASKX 0 /* Positions within copy mask of X,Y start offset */
#define MASKY 1

/****************************************************************************/

#define KBStatus (*(UBYTE far *) 0x0417)
#define KBType   (*(UBYTE far *) 0x0496)

/****************************************************************************/
//          SPRITE EXPLOSION TYPES

#define	AIRXPLO	    	0       /* grandiose air explosion with secondaries */
#define	GRNDXPLO		1       /* grandiose ground explosion with secondaries */
#define	SMKTRAIL		2       /* single puff-o-smoke */
#define	SMKPLUME		3       /* ground-based column of smoke with secondaries */
#define	GRND2XPLO		4       /* secondary ground explosion (single) */
#define	AIR2XPLO		5       /* secondary air explosion (single) */
#define CLUSTERXPLO 	6       /* single cluster puff */
#define CLUSTER2XPLO 	7      /* other cluster puffs */
#define CARRIERXPLO 	8
#define EXHAUST 		9          /* missile exhaust in flight */

//***************************************************************************
// DAMAGE TO OUR PLANE

#define D_LEFTENGINE    0x0001
#define D_RIGHTENGINE   0x0002
#define D_HYDRAULICS    0x0004
#define D_AFTERBURNER   0x0008
#define D_LFUELSYSTEM   0x0010
#define D_RFUELSYSTEM   0x0020
#define D_FIRECONTROL   0x0040
#define D_AWG9          0x0080
#define D_HUD           0x0100
#define D_PDISPLAYS     0x0200
#define D_HOOK			0x0400
#define D_NAV           0x0800
#define D_GEAR			0x1000
#define D_WINGSWEEP     0x2000
#define D_ECM           0x4000
#define D_COCKPIT       0x8000

//****************************************************************************
#define F_3x5A      1   /* old 1 */
#define F_3x5B      2   /* old 9 */
#define F_3x6       3   /* old 8 */
#define F_3x4       4   /* old 7 */

/****************************************************************************/
#define J_KBONLY    0
#define J_ONESTICK  1
#define J_STKNTHROT 2
#define J_TWOSTICKS 3
#define J_THMASTER  4
#define J_CHPRO     5
