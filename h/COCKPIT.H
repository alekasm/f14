/*      Cockpit.h

    Constants for Cockpit display
                                 */
#ifdef _include_code
	int View;
#else
	extern int View;
#endif

/* Main View Types */

#define VOFFSET 0x80
#define VFULL   0x40
#define VNUM    (~(VOFFSET|VFULL))


#define FRONT   		0
#define LEFT			1
#define	RIGHT			2
#define DOWN			3
#define REARLEFT 		4
#define	REARRIGHT		5
#define FRONTAL 		6  + VFULL			
#define PADLOCK 		7  + VFULL        // from here down, full view screen is adjustable in size
#define PILOT			8  + VFULL

#define STEADY      	9  + VFULL+VOFFSET
#define WINGMAN     	10 + VFULL+VOFFSET
#define MAGICCAM1   	11 + VFULL+VOFFSET
#define MAGICCAM2   	12 + VFULL+VOFFSET
#define POVVIEW     	13 + VFULL+VOFFSET
#define EJECTVIEW   	14 + VFULL+VOFFSET
#define	TOWERVIEW		15 + VFULL+VOFFSET
#define	LSOVIEW			16 + VFULL+VOFFSET

struct light {
	int x;
	int y;
	int x2;
	int y2;
	int color;
};

#ifdef _include_code

struct light ls[] = {
	163,191, 169,197, CYAN,
	180,191, 186,197, CYAN,
	214,191, 220,197, CYAN,
	197,191, 203,197, CYAN,
	2,133,  16,138, BLACK,
	287,133, 297,138, BLACK,
	287,144, 297,149, BLACK,
	287,155, 297,160, BLACK,
	152,114, 168,117, CYAN,
	152,180, 168,183, CYAN,

	10,116,  16,120, LGREEN,
	19,116,  25,120, LGREEN,
	28,116,  35,120, LGREEN,
	10,121,  16,124, LGREEN,
	19,121,  25,124, LGREEN,
	28,121,  35,124, LGREEN,
	10,125,  16,128, LGREEN,
	19,125,  25,128, LGREEN,
	28,125,  35,128, LGREEN,
};

#else
extern struct light ls[];
#endif


#define LSTALL_LIGHT		0
#define	GEAR_LIGHT			1
#define	BRAKE_LIGHT			2
#define HOOK_LIGHT      	3
#define	RSTALL_LIGHT		4
#define SAM_LIGHT			5
#define	AAA_LIGHT			6
#define AI_LIGHT			7
#define	SEAMLOCK_LIGHT		8
#define	COLLISION_LIGHT		9
#define	HOTTRIG_LIGHT		10
#define	LEFTFIRE_LIGHT		11
#define	ADJ_LIGHT			12
#define LANDINGCHK_LIGHT	13
#define ACLREADY_LIGHT		14
#define APCPLR_LIGHT		15
#define	CMDCONTROL_LIGHT	16
#define	SECONDS_LIGHT		17
#define	TILT_LIGHT			18
#define	RIGHTFIRE_LIGHT		19
#define	WAVEOFF_LIGHT		20
#define	WINGSWEEP_LIGHT		21
#define	REDUCESPEED_LIGHT	22
#define	ALTLOW_LIGHT		23
#define	GRHIGH_LIGHT		24
#define	GRLOW_LIGHT			25
#define	SWCOOLON_LIGHT		26
#define	SWCOOLOFF_LIGHT		27
#define MSLPREPON_LIGHT		28
#define	MSLPREPOFF_LIGHT	29
#define	MSLMODENORM_LIGHT	30
#define MSLMODEBRSIT_LIGHT	31
#define	STORES1				32
#define	STORES2				33
#define	STORES3				34
#define	STORES4				35
#define	STORES5				36
#define STORES6				37
#define	STORES7				38
#define	STORES8				39



