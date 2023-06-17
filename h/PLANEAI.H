//#define ELFBMSG 1
//#define ELFDMSG 1
//#define ELFMMSG 1

#define DUNNOSRC	-1
#define AWACSSRC	0
#define TOWERSRC	1
#define CARRIERSRC  2
#define WINGSRC  3

#define ENEMYSRC	0
#define FRIENDLYSRC	1

#define SNDSTUFF 1
#define SLOWSTUFF 2
#define HITSTUFF 3

#define PLAYER160	0x1
#define PLAYER80	0x2
#define PLAYER40	0x4
#define AWACS160	0x8
#define AWACS80		0x10
#define CARRIER250	0x20
#define CARRIER150	0x40

//#define SHOWBEHAVIORS 1
//#define SHOWHOME 1
//#define SHOWDETMAR 1
//#define ELFREFUEL 1

#define MAXWINGLEVEL 5
#define PARADEDIST 16  /*  was 16 25  */
#define CRUISEDIST 53
#define COMBATSPREADDIST 88
#define BOMBERPARADEDIST 53
#define BOATDIST 14400  /*  was 14400  */
#define CARRIERGRPDIST 12800
#define AILOCDIV 100   /*  was 400  */

#define DOWPDITCH 0x10
#define DOWPDAMAGE 0x20
#define DOWPEXPLODE 0x40
#define DOWPHAWKEYE 0x80
#define DOWPFORMON  0x100
#define DOWPTROOPDROP 0x200
#define DOWPLAND 0x400
#define DOWPHOVER 0x800
#define DOWPDBFM  0x1000
#define DOWPOBFM  0x2000
#define DOWPNBFM  0x4000
#define DOWPGOAL  0x8000

/*  landing status flags  */
#define LANDPLAYERWAIT 0x01
#define LANDPLAYERNOW  0x02
#define LANDCAT0USED   0x10
#define LANDCAT1USED   0x20
#define LANDNEAREND    0x40
#define LANDREQUESTED  0x80
#define LANDNOTAKEOFF  0x100


#define EVASIVEDIST 32

#define DAMAGERADAR 0x20
#define DAMAGEENGINE 0X40
#define DAMAGECONTROLSURFACE 0x80
#define RADARON 0x100
#define ENEMYNOFIRE 0x200
#define CFDROPPED 0x400
#define AI_INVISIBLE 0x800
#define ENEMYLOCK 0x1000
#define NO_SWITCH_TARGET 0x2000
#define COMBAT_SPREAD 0x4000
#define ISHELICOPTER 0x8000

//  #define AIARMWEAPONSRADAR 1
#define AIATTACKCLOSE 1 	/*  ALT F1  */
#define AIATTACKTARGET 2	/*  ALT F2  */
#define AIFORMUP 3			/*  ALT F3  */
#define AIBRACKET 4			/*  ALT F4  */
#define AIHELPME 5			/*  ALT F5  */
#define AISANITIZE 6		/*  ALT F6  */
#define AISORTAI 7			/*  ALT F7  */
#define AISHOWSTATUS 8		/*  ALT F8  */
#define AIREJOIN 9			/*  ALT F9  */
#define AIRETURNBASE 10		/*  ALT F10  */

#define AIPLAYERHELP 11      /*  SHIFT F1  */
#define AIDOREFUEL  15      /*  SHIFT F5  */
#define OTHERWINGAI 18		/*  SHIFT F8  */

#define AIBRACLEFT 21
#define AIBRACRIGHT 22
#define AIFLYHIGH 23
#define AIFLYLOW 24
#define AIFLYSTRAIGHT 25
#define AIPARADE 31
#define AICRUISE 32
#define AICOMBATSPREAD 33
#define AISANITIZELEFT 41
#define AISANITIZERIGHT 42
#define AISANITIZEFRONT 43
#define AISANITIZEBACK 44
#define AINEGVSEP1000 50
#define AIPOSVSEP1000 60


#if 0
#define AIATTACKCLOSE 1 	/*  ALT F1  */
#define AIATTACKTARGET 2	/*  ALT F2  */
#define AIBOMBTARGET 8		/*  ALT F8  */
#define AIARMWEAPONSRADAR 9 /*  ALF F9  */
#define AISHOWSTATUS 11		/*  SHIFT F1  */
#define AIFORMUP 12			/*  SHIFT F2  */
#define OTHERWINGAI 18		/*  SHIFT F8  */
#define AIRETURNBASE 20		/*  SHIFT F10 */
#endif

#define GETWINGBRACKET 1
#define GETWINGFORMATION 2
#define GETWINGSANITIZE 3
#define GETNEGVSEPERATION 4
#define GETPOSVSEPERATION 5

/*  Ground Target Defines  */
#define ALIVE  		0x1
#define NEUTRAL     0x0100
#define FRIENDLY    0x0400  // Same as planes!  Don't change
#define IS_AIRBASE  0x1000
#define AAA_ACTIVE	0x2000
#define AAA_FIRING	0x4000

