#define LB   0x01
#define PG   0x02
#define NC   0x04
#define CE   0x08

#define COLD 0x01
#define LIM  0x02
#define HOT  0x04

#define NIGHT   0x01
#define TIMED   0x02
#define LIGHTS  0x04
#define BAY4    0x08
#define BEACON  0x10
#define DISAPPEAR 0x20

#define M_AIRSTRIP    1
#define M_SAMRAD      2  
#define M_MOBSAM      3  
#define M_OTHRAD      4  
#define M_LPARRAD     5 
#define M_REFINERY    6
#define M_TANKFARM    7
#define M_HANGAR      8  
#define M_HQ          9      
#define M_CARRIER     10 
#define M_WARSHIP     11 
#define M_MBOAT       12   
#define M_SAM         13     
#define M_OILRIG      14  
#define M_NUKEP       15   
#define M_SUBPEN      16  
#define M_MSHIP       17   
#define M_SUB         18    
#define M_DEPOT       19   
#define M_BRIDGE      20  
#define M_AIRBASE     21
#define M_GPLANE      22
#define M_CAMP        23

#define O_SAMRAD    0
#define O_MOBSAM    1
#define O_OTHRAD    0
#define O_LPARRAD   0
#define O_REFINERY  9
#define O_TANKFARM  4
#define O_STEALTH   5
#define O_PLANES    6
#define O_AN72      6
#define O_TCAMP    11
#define O_KREMLIN   3
#define O_WARSHIP   7
#define O_MBOAT     7
#define O_SAM       1
#define O_OILRIG    8
#define O_NUKEP     2
#define O_MSHIP     0
#define O_DEPOT     0
#define O_HQ        0

#define PHOTO       1
#define STRIKE      2
#define AIRDROP     3
#define DELIVERY    4
#define INTERCEPT   5
#define RETURN      6
#define RAID        7
#define PENETRATE   8

struct MissionData {
        int where;
        int when;
        int type;
        int target;
        int special;
        int object;
    };

#ifdef _mission_data
struct MissionData md[] = {
/* 0*/  LB+PG+NC+CE, COLD+LIM+HOT, DELIVERY, M_AIRSTRIP, NIGHT+TIMED+LIGHTS+BAY4+DISAPPEAR, 0,
        LB+PG+NC+CE, COLD+LIM+HOT, DELIVERY, M_AIRSTRIP, NIGHT+TIMED+LIGHTS+BAY4+DISAPPEAR, 0,
        LB+PG+NC+CE, COLD,         AIRDROP,  M_AIRSTRIP, TIMED+BEACON+BAY4+DISAPPEAR, 0,
        LB+PG+NC+CE, LIM+HOT,      AIRDROP,  M_AIRSTRIP, TIMED+BEACON+BAY4+DISAPPEAR,  0,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_SAMRAD,   0, O_SAMRAD,
        LB+PG+NC+CE,      LIM+HOT, STRIKE,   M_SAMRAD,   0, O_SAMRAD,
        LB+PG+NC+CE, LIM+HOT,      STRIKE,   M_MOBSAM,   0, O_MOBSAM,
        LB+PG+NC+CE, COLD+LIM+HOT, PHOTO,    M_OTHRAD,   0, O_OTHRAD,
        LB+PG+NC+CE, LIM+HOT,      STRIKE,   M_OTHRAD,   0, O_OTHRAD,
        LB+PG+NC+CE, COLD,         PHOTO,    M_LPARRAD,  0, O_LPARRAD,
/*10*/  LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_LPARRAD,  0, O_LPARRAD,
        LB+PG+NC+CE, COLD+LIM+HOT, PHOTO,    M_REFINERY, 0, O_REFINERY,
        LB+PG+NC+CE, HOT,          STRIKE,   M_REFINERY, 0, O_REFINERY,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_TANKFARM, 0, O_TANKFARM,
        LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_TANKFARM, 0, O_TANKFARM,
        LB+PG+NC+CE, COLD+LIM+HOT, PHOTO,    M_GPLANE,   0, O_STEALTH,
        LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_GPLANE,   0, O_STEALTH,
        LB+PG+NC+CE, COLD+LIM+HOT, PHOTO,    M_GPLANE,   0, O_PLANES,
        LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_GPLANE,   0, O_PLANES,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_GPLANE,   TIMED+DISAPPEAR, O_AN72,
/*20*/  LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_GPLANE,   TIMED+DISAPPEAR, O_AN72,
        LB+PG,       COLD+LIM,     PHOTO,    M_CAMP,     0,     O_TCAMP,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_CAMP,     TIMED, O_TCAMP,
        LB+PG+NC+CE, COLD,         PHOTO,    M_CARRIER,  0,     O_KREMLIN,
        LB+PG+NC+CE, LIM+HOT,      STRIKE,   M_CARRIER,  0,     O_KREMLIN,
        LB+PG+NC+CE, COLD+LIM+HOT, PHOTO,    M_MBOAT,    0,     O_WARSHIP,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_MBOAT,    TIMED, O_MBOAT,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_MBOAT,    0,     O_MBOAT,
        LB+PG,       COLD+LIM+HOT, PHOTO,    M_SAM,      0,     O_SAM,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_SAM,      0,     O_SAM,
/*30*/  PG,          COLD+LIM,     PHOTO,    M_OILRIG,   0,     O_OILRIG,
        PG,          COLD+LIM+HOT, STRIKE,   M_OILRIG,   0,     O_OILRIG,
        LB+PG,       COLD+LIM,     PHOTO,    M_NUKEP,    0,     O_NUKEP,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_NUKEP,    0,     O_NUKEP,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_SUBPEN,   0,     0,
        LB+PG+NC+CE, LIM+HOT,      STRIKE,   M_SUBPEN,   0,     0,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_MSHIP,    TIMED+DISAPPEAR, 0,
        LB+PG,       COLD+LIM+HOT, STRIKE,   M_MSHIP,    TIMED+DISAPPEAR, 0,
        LB+PG+NC+CE, COLD+LIM,     PHOTO,    M_SUB,      0,     0,
        LB+PG+NC+CE, LIM+HOT,      STRIKE,   M_SUB,      0,     0,
/*40*/  LB+PG+NC+CE, COLD,         PHOTO,    M_DEPOT,    0,     O_DEPOT,
        LB+PG+NC+CE, COLD+LIM+HOT, STRIKE,   M_DEPOT,    0,     O_DEPOT,
        LB+PG+CE,    COLD+LIM+HOT, PHOTO,    M_HQ,       0,     O_HQ,
        LB+PG+CE,    COLD+LIM+HOT, STRIKE,   M_HQ,       0,     O_HQ,
        NC+CE,            LIM+HOT, STRIKE,   M_HQ,       TIMED, O_HQ,            
        LB+PG+NC+CE, LIM+HOT,      PHOTO,    M_BRIDGE,   0,     0,
        LB+PG+NC+CE,      LIM+HOT, STRIKE,   M_BRIDGE,   0,     0,
        NC+CE,       LIM+HOT,      INTERCEPT,M_AIRBASE,  0,  0,
        LB+PG,       COLD+LIM+HOT, INTERCEPT,M_AIRBASE,  0,  0,
        NC+CE,       LIM+HOT,      RAID,     M_AIRBASE,  0,  0,
/*50*/  LB+PG,       COLD+LIM,     RAID,     M_AIRBASE,  0,  0,
        LB+PG+CE,    COLD,         RAID,     M_AIRBASE,  0,  0,
        LB+PG+NC+CE, COLD+LIM,     RETURN,   M_AIRBASE,  0,  -TU95,
        NC+CE,       LIM+HOT,      PENETRATE,M_AIRBASE,  0,  -TU95,
        LB+NC+CE,    LIM+HOT,      PENETRATE,M_AIRBASE,  0,  -IL76,
        LB+PG+NC+CE, COLD+LIM+HOT, PENETRATE,M_AIRBASE,  0,  -MIG29,

    }; 

#define NMISSIONS (sizeof(md)/sizeof(struct MissionData))

#else
extern struct MissionData md[];
#endif




typedef struct _MissionPoint {

        int     x ;
        int     y ;

        } MissionPoint ;


typedef struct _Objects {
        int     offset ;        // 1 based, 0 for end of list, 
                                //     neg for crit targs, 
                                //     pos for all inclusive (including point targs)
        int     crit_flag ;     // bits set for critical targets
        int     non_crit_flag ; // bits set for non-critical targets
        int     num_non_crit ;  // # non-crits needed
        
        } Objects ;



#ifdef _TESTING

        typedef struct Mission_ {

                UBYTE   stamp ;
                UBYTE   class ;
                int     priority ;

                Objects Objects[10] ;

                int      num_non_crit ;
                MissionPoint location ;
                unsigned StampLoc ;
                unsigned accomplished ;


                char    *BriefingText ;

                } Mission ;

#else


        typedef struct Mission_ {

                UBYTE   stamp ;
                UBYTE   class ;
                int     priority ;

                Objects Objects[10] ;

                int      num_non_crit ;
                MissionPoint location ;
                unsigned StampLoc ;
                unsigned accomplished ;


                char    BriefingText[64] ;

                } Mission ;


#endif

#define CSOFT_A  1
#define CHARD_A  2
#define CAREA_S  3
#define CAREA_H  4
#define NAVAL_S  5
#define NAVAL_H  6



extern Mission AllMissions[] ;


//#define _NDEBUG
