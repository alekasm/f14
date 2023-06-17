/*
      enum header file for F14 missions
      F-14 -- the definitive simulation

*/




// World Names Defines
enum {
SW_NORTHCAPE,
SW_AFRICA,
SW_TRAINING,
SW_COUNT} ;



// Sides Defines
enum {
SS_FRIENDLY,
SS_NEUTRAL,
SS_ENEMY,
SS_CIVILIAN,
SS_UNKNOWN,
SS_COUNT} ;



// Weather Types Defines
enum {
SWE_HIGHSKY,
SWE_HIGHCLOUDS,
SWE_OVERCAST,
SWE_COUNT} ;



// Weapon Loadout Names Defines
enum {
SL_FD_BRAVO,
SL_MC_BRAVO,
SL_FD_ALPHA,
SL_FD_CHARLIE,
SL_MC_CHARLIE,
SL_MC_ALPHA,
SL_GUNS,
SL_T_ALPHA,
SL_T_BRAVO,
SL_COUNT} ;



// Object Names Defines
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
SO_DC_10a,
SO_EC_130q,
SO_EC_130h,
SO_F_4a,
SO_F_14TARPS,
SO_RESERV1,
SO_RESERV2,
SO_RESERV3,
SO_RESERV4,
SO_RESERV5,
SO_COUNT} ;



// Object Types Defines
enum {
ST_PLAYER,
ST_WINGMAN,
ST_CAP,
ST_TARPS,
ST_ESCORT,
ST_READY_5,
ST_CARRIER,
ST_PLANE,
ST_BOAT,
ST_HOSTILE,
ST_DAMAGED,
ST_NOFIRE,
ST_GROUND_TARGET,
ST_ALIVE,
ST_DEAD,
ST_SA_2,
ST_SA_3,
ST_SA_5,
ST_SA_6,
ST_AIRPORT,
ST_AAASITE,
ST_NOAAA,
ST_COUNT} ;



// waypoint actions Defines
enum {
SA_CAP,
SA_FORMATION,
SA_WAYPOINT,
SA_GORILLABOMB,
SA_REFUEL,
SA_CRUISE_MISS,
SA_SAR,
SA_SAR_CAP,
SA_GORILLA_COVER,
SA_WAYPOINT_ESCORTABLE,
SA_AWACS,
SA_GET_FUEL,
SA_ELEVATION,
SA_SPEED,
SA_ATTACK,
SA_HAWKEYE,
SA_HEADING,
SA_FORM_ON,
SA_LAUNCH,
SA_DAMAGE,
SA_DITCH,
SA_EXPLODE,
SA_TROOPERDROP,
SA_LAND,
SA_FORMTRAIN,
SA_ENGAGETRAIN,
SA_TRAINING,
SA_HOVER,
SA_IMMORTALAI,
SA_DEFENSIVEBFM,
SA_OFFENSIVEBFM,
SA_NEUTRALBFM,
SA_HARDDECK,
SA_PEACETIME,
SA_GOAL,
SA_COUNT} ;



// Plane Formations Defines
enum {
SF_BOMBER_FORMATION,
SF_PARADE,
SF_CRUISE,
SF_COMBATSPREAD,
SF_LADDER,
SF_BOX,
SF_WALL,
SF_CAP,
SF_STRIKE,
SF_COUNT} ;



// // group definitions Defines
#define MAX_GROUPS 100
enum {GR_An_22,
GR_An_26,
GR_Be_12a,
GR_F_5a,
GR_F_16,
GR_G_4a,
GR_Il_38a,
GR_J_22a,
GR_MiG_21,
GR_MiG_23,
GR_MiG_25,
GR_MiG_27,
GR_Mig_29,
GR_MiG_31,
GR_F_1,
GR_Su_17,
GR_Su_22,
GR_Su_24,
GR_Su_25,
GR_Su_27,
GR_Tu_16,
GR_Tu_22,
GR_Tu_95,
GR_Tu_126a,
GR_Tu_142a,
GR_Viggena,
GR_Viggenb,
GR_Yak_38,
GR_A_6,
GR_A_7,
GR_B_52,
GR_C_130a,
GR_E_2Ca,
GR_EA_6Ba,
GR_F_A_18,
GR_F_111,
GR_GR_1,
GR_KA_6a,
GR_MK_3a,
GR_Nimroda,
GR_P_3a,
GR_S_3a,
GR_Ka_25,
GR_Ka_27,
GR_Mi_8,
GR_Mi_14a,
GR_Mi_24a,
GR_SH_2,
GR_SH_3,
GR_SH_60a,
GR_MH_53,
GR_carrier,
GR_CGa,
GR_DDGa,
GR_FFGa,
GR_LSTa,
GR_SovCVa,
GR_SovCVHa,
GR_SovCGHa,
GR_SovBCGa,
GR_SovCGa,
GR_SovDDGa,
GR_SovFFGa,
GR_SovFFa,
GR_SovPTMa,
GR_SovLSTa,
GR_SovAFSa,
GR_Rafta,
GR_Raftb,
GR_F_14,
GR_AS_4,
GR_AS_6,
GR_F_14SPE,
GR_AS_5,
GR_AS_7,
GR_AS_9,
GR_AS_10,
GR_AS_12,
GR_Exocet,
GR_HARM,
GR_Harpoon,
GR_Penquin,
GR_SeaEagle,
GR_Tomahawk,
GR_gt,
GR_dc_10a,
GR_ec_130q,
GR_ec_130h,
GR_f_4a,
GR_F_14tarps,
GR_reserv1,
GR_reserv2,
GR_reserv3,
GR_reserv4,
GR_reserv5,
} ;



// // group definitions Defines
#define MAX_GROUPS 100
int _numGroups = 95,
    _groups[MAX_GROUPS][20] = {
		{SO_AN_22a, SO_AN_22b},                                   	// An_22
		{SO_AN_26a, SO_AN_26b},                                   	// An_26
		{SO_BE_12a},                                              	// Be_12a
		{SO_F_5a},                                                	// F_5a
		{SO_F_16a, SO_F_16b, SO_F_16c},                           	// F_16
		{SO_G_4a},                                                	// G_4a
		{SO_IL_38a},                                              	// Il_38a
		{SO_J_22a},                                               	// J_22a
		{SO_MIG_21a, SO_MIG_21b, SO_MIG_21c, SO_MIG_21d},         	// MiG_21
		{SO_MIG_23a, SO_MIG_23b, SO_MIG_23c, SO_MIG_23d},         	// MiG_23
		{SO_MIG_25a, SO_MIG_25b, SO_MIG_25c},                     	// MiG_25
		{SO_MIG_27a, SO_MIG_27b, SO_MIG_27c},                     	// MiG_27
		{SO_MIG_29a, SO_MIG_29b, SO_MIG_29c, SO_MIG_29d, SO_MIG_29e, SO_MIG_29f}, 	// Mig_29
		{SO_MIG_31a, SO_MIG_31b, SO_MIG_31c},                     	// MiG_31
		{SO_F_1a, SO_F_1b},                                       	// F_1
		{SO_SU_17a, SO_SU_17b, SO_SU_17c},                        	// Su_17
		{SO_SU_22a, SO_SU_22b, SO_SU_22c, SO_SU_22d},             	// Su_22
		{SO_SU_24a, SO_SU_24b, SO_SU_24c, SO_SU_24d, SO_SU_24e},  	// Su_24
		{SO_SU_25a, SO_SU_25b, SO_SU_25c, SO_SU_25d, SO_SU_25e},  	// Su_25
		{SO_SU_27a, SO_SU_27b, SO_SU_27c},                        	// Su_27
		{SO_TU_16AA, SO_TU_16DA, SO_TU_16GA, SO_TU_16GB, SO_TU_16HA, SO_TU_16JA, SO_TU_16LA}, 	// Tu_16
		{SO_TU_22a, SO_TU_22b, SO_TU_22c, SO_TU_22MA},            	// Tu_22
		{SO_TU_95DA, SO_TU_95GA},                                 	// Tu_95
		{SO_TU_126a},                                             	// Tu_126a
		{SO_TU_142a},                                             	// Tu_142a
		{SO_VIGGENA},                                             	// Viggena
		{SO_VIGGENB},                                             	// Viggenb
		{SO_YAK_38a, SO_YAK_38b, SO_YAK_38c},                     	// Yak_38
		{SO_A_6EA, SO_A_6EB, SO_A_6EC, SO_A_6ED},                 	// A_6
		{SO_A_7EA, SO_A_7EB, SO_A_7EC},                           	// A_7
		{SO_B_52a, SO_B_52b, SO_B_52c},                           	// B_52
		{SO_C_130a},                                              	// C_130a
		{SO_E_2CA},                                               	// E_2Ca
		{SO_EA_6BA},                                              	// EA_6Ba
		{SO_F_A_18a, SO_F_A_18b, SO_F_A_18c, SO_F_A_18d},         	// F_A_18
		{SO_F_111a, SO_F_111b},                                   	// F_111
		{SO_GR_1a, SO_GR_1b},                                     	// GR_1
		{SO_KA_6a},                                               	// KA_6a
		{SO_MK_3a},                                               	// MK_3a
		{SO_NIMRODA},                                             	// Nimroda
		{SO_P_3a},                                                	// P_3a
		{SO_S_3a},                                                	// S_3a
		{SO_KA_25a, SO_KA_25b},                                   	// Ka_25
		{SO_KA_27a, SO_KA_27b},                                   	// Ka_27
		{SO_MI_8a, SO_MI_8b},                                     	// Mi_8
		{SO_MI_14a},                                              	// Mi_14a
		{SO_MI_24a},                                              	// Mi_24a
		{SO_SH_2a, SO_SH_2b},                                     	// SH_2
		{SO_SH_3a, SO_SH_3b},                                     	// SH_3
		{SO_SH_60a},                                              	// SH_60a
		{SO_MH_53a, SO_MH_53b},                                   	// MH_53
		{SO_NIMITZA, SO_KHAWKA, SO_FORSTALA},                     	// carrier
		{SO_CGA},                                                 	// CGa
		{SO_DDGA},                                                	// DDGa
		{SO_FFGA},                                                	// FFGa
		{SO_LSTA},                                                	// LSTa
		{SO_SOVCVA},                                              	// SovCVa
		{SO_SOVCVHA},                                             	// SovCVHa
		{SO_SOVCGHA},                                             	// SovCGHa
		{SO_SOVBCGA},                                             	// SovBCGa
		{SO_SOVCGA},                                              	// SovCGa
		{SO_SOVDDGA},                                             	// SovDDGa
		{SO_SOVFFGA},                                             	// SovFFGa
		{SO_SOVFFA},                                              	// SovFFa
		{SO_SOVPTMA},                                             	// SovPTMa
		{SO_SOVLSTA},                                             	// SovLSTa
		{SO_SOVAFSA},                                             	// SovAFSa
		{SO_RAFTA},                                               	// Rafta
		{SO_RAFTB},                                               	// Raftb
		{SO_F_14FDA, SO_F_14FDB, SO_F_14FDC, SO_F_14MCA, SO_F_14MCB}, 	// F_14
		{SO_AS_4},                                                	// AS_4
		{SO_AS_6},                                                	// AS_6
		{SO_F_14SPE},                                             	// F_14SPE
		{SO_AS_5},                                                	// AS_5
		{SO_AS_7},                                                	// AS_7
		{SO_AS_9},                                                	// AS_9
		{SO_AS_10},                                               	// AS_10
		{SO_AS_12},                                               	// AS_12
		{SO_EXOCET},                                              	// Exocet
		{SO_HARM},                                                	// HARM
		{SO_HARPOON},                                             	// Harpoon
		{SO_PENQUIN},                                             	// Penquin
		{SO_SEAEAGLE},                                            	// SeaEagle
		{SO_TOMAHAWK},                                            	// Tomahawk
		{SO_GT},                                                  	// gt
		{SO_DC_10a},                                              	// dc_10a
		{SO_EC_130q},                                             	// ec_130q
		{SO_EC_130h},                                             	// ec_130h
		{SO_F_4a},                                                	// f_4a
		{SO_F_14TARPS},                                           	// F_14tarps
		{SO_RESERV1},                                             	// reserv1
		{SO_RESERV2},                                             	// reserv2
		{SO_RESERV3},                                             	// reserv3
		{SO_RESERV4},                                             	// reserv4
		{SO_RESERV5},                                             	// reserv5
	} ;
