//************************************************************************
//*
//*    ARMT.H
//*
//*    Header file for ARMT.C
//*
//*    Author: W. Becker
//*
//*    F-15 III
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//*************************************************************************

#define  MASTER_NAV        0
#define  MASTER_AA         1
#define  MASTER_N          3

#define  NAV_NAV           0
#define  NAV_TACAN         1
#define  NAV_ILSNAV        2
#define  NAV_ILSTCN        3
#define  NAV_N             4

#define  AA_MRM            0
#define  AA_SRM            1
#define  AA_GUN            2
#define  AA_N              3
#define  AA_LRM            4

#define  W_NONE            	0
#define  W_MK_82           	1
#define  W_BSU_49          	2
#define  W_MK_20           	3
#define  W_AGM_65G         	4
#define  W_MK_84           	5
#define  W_BSU_50          	6
#define  W_GBU_10          	7
#define  W_GBU_12          	8
#define  W_GBU_15          	9
#define  W_CBU_87          	10
#define  W_CBU_89          	11
#define  W_AGM_88          	12
#define  W_AGM_84A         	13
#define  W_AGM_84E         	14
#define  W_AIM_54			15
#define  W_AXQ_14          	16
#define  W_FUEL_TANK       	17
#define  W_AIM_9           	18
#define  W_AIM_120         	19
#define  W_AIM_7           	20
#define  W_CANNON          	21
#define  W_TYPES           	22

#define  W_DUD             0
#define  W_AAW             2
#define  W_AGW             4
#define  W_SYM             8
#define  W_IND             16

#define F14_STATION_NUM		12

//*************************************************************************
typedef struct {
   char  name[10];
   char  model[2];
   int   samtype;
   int   wtype;
   int   rmax;
   int   rmin;
   UBYTE area;      // in 8 foot units
   UBYTE HardDamage;
   UBYTE SoftDamage;
} WEAPON;

#define WPN_AREA_TO_BITS 2  // 8 foot area units to 4 foot system units

typedef struct {
	int weapon;
	int pos[3];
} LAUNCHPOS;

//*************************************************************************
extern   int         MODE_MASTER;
extern   int         MODE_NAV;
extern   int         MODE_AA;
extern   int         F14_STATION[];
extern   WEAPON      ORDNANCE[];
LAUNCHPOS            *ARMT_release();

