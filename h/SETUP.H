#ifndef SETUP_H
#define SETUP_H

// Setup Objects
// World Names Defines
enum {
SW_NORTHCAPE,
SW_MED,
SW_OCEANA,
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
SWE_RANDOM,
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
ST_CROTALE,
ST_HAWK,
ST_NIKE_HERCULES,
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
SA_IMMORTAL,
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

typedef struct _path_ {

        char far        *name ;
        unsigned        wp0 ;
        unsigned        numWPs ;

        } _Path ;



typedef struct _wayPoint_ {

        long            x ;             // if x is -1, then y contains index
                                        //      into object list to vector towards
                                        // if x is -2, then following waypoint coords
                                        //      are relative to object in y
        long            y ;
        unsigned        z ;
        unsigned        actionIndex ;   // index into action list
        unsigned        numActions ;    // # actions to take @ this waypoint

        } _WayPoint ;

typedef struct _action_ {


        int             action ;        // index into actions list
        unsigned        value ;         // value associated w/action (knots for speed, etc)

        } _Action ;

typedef struct _object_ {

        int             object ;        // index into object list
        int             type ;          // etc, etc
        int             stationNumber ; //
        char            name[16] ;      //
        char            callSign[16] ;  //
        int             squadMember ;   // index into squadron names
        long            quality ;       // how good
                                        // separated into 4 bit fields

        int             side ;          // who do they fly for
        long            x ;
        long            y ;
        long            z ;
        int             speed ;         // in knots
        int             heading ;       // ?
        long            number ;        // Scott must turn these to 1
                                        // separated into 4 bit fields

        int             formation ;     // index into ...
        int             path ;          // index into paths in file

        int             damageLevel ;   // how much damage is this bitch carrying
                                        // 0 -  no damage
                                        // 1-2  slight damgage
                                        // 2-3  moderate damage
                                        // 4-5  heavy damage
                                        // 6    severe damage
                                        // 7    fatal damage

        long            dramaticEvent ; // what happened to him
        int             time ;          // time of dramatic event

        long            live_or_die ;   // flag indicating whether death of this
                                        //      thing is goal of mission
                                        // separated into 4 bit fields


        } _Object ;



typedef struct _missionHeader_ {


        char            name[16] ;      // name of input file (description ??)
        char            description[64];// short description of mission
        int             world ;
        int             time ;          // in minutes since midnight
        int             weather ;
        int             ceiling ;       // cloud deck level
        int             loadout ;
        int             minTimeAloft ;  // for survive  only missions

        //int             overallDiff ;   // overall difficulty level

        int             numObjects ;    // total objects - GTs
        int             numGTs ;
        int             numPaths ;
        int             numWPs ;
        int             numActions ;
        int             numStations ;

        long            objectLoc ;     // dword aligned offset into file
        long            gtLoc ;         //      of beginning of each section
        long            pathLoc ;       //      of beginning of each section
        long            wpLoc ;
        long            actionLoc ;
        long            stationLoc ;


        long            briefingLoc ;   // dword aligned offset into file
                                        //      of briefing text - NOT Necessary
                                        //      for game - ONLY WRAPPERS
        int             briefingLen ;


        long            goalLoc ;       // text that defines what to kill
        int             goalLen ;


        int             numMustLive ;   // # must Live Objects
        int             numMustDie ;
        int             numMustEscort ; //


        long            mLiveObjectsLoc ; // must live object locs
        long            mDieObjectsLoc ;
        long            mEscObjectsLoc ;


        } _MissionHeader ;

// LOG Defines
enum {
SLG_AIR_MISSILE_LAUNCH,
SLG_BOAT_MISSILE_LAUNCH,
SLG_GROUND_MISSILE_LAUNCH,
SLG_AIR_CRUISE_LAUNCH,
SLG_BOAT_CRUISE_LAUNCH,
SLG_GROUND_CRUISE_LAUNCH,
SLG_AIR_MISSILE_HIT,
SLG_BOAT_MISSILE_HIT,
SLG_GROUND_MISSILE_HIT,
SLG_AIR_BULLET_HIT,
SLG_BOAT_BULLET_HIT,
SLG_GROUND_BULLET_HIT,
SLG_AIR_CRUISE_HIT,
SLG_BOAT_CRUISE_HIT,
SLG_GROUND_CRUISE_HIT,
SLG_AIR_DESTROYED_MISSILE,
SLG_BOAT_DESTROYED_MISSILE,
SLG_GROUND_DESTROYED_MISSILE,
SLG_AIR_DESTROYED_BULLET,
SLG_BOAT_DESTROYED_BULLET,
SLG_GROUND_DESTROYED_BULLET,
SLG_AIR_DESTROYED_CRUISE,
SLG_BOAT_DESTROYED_CRUISE,
SLG_GROUND_DESTROYED_CRUISE,
SLG_AIR_BOMB_DROP,
SLG_AIR_BOMB_HIT,
SLG_BOAT_BOMB_HIT,
SLG_GROUND_BOMB_HIT,
SLG_AIR_DESTROYED_BOMB,
SLG_BOAT_DESTROYED_BOMB,
SLG_GROUND_DESTROYED_BOMB,
SLG_AIR_TROOP_DROP,
SLG_AIR_TAKEOFF,
SLG_AIR_LAND,
SLG_AIR_CRASH,
SLG_AIR_REFUEL,
SLG_CONFIG_CHANGE,
SLG_AIR_BAD_EJECT,
SLG_AIR_GOOD_EJECT,
SLG_TRAINING_TURNED_ON,
SLG_TRAINING_TURNED_OFF,
SLG_AAA_HIT,
SLG_AAA_DESTROY,
SLG_WAY_PT_GOAL,
SLG_MIN_TIME_REACHED,
SLG_COUNT} ;

enum {
LOG_AIR,
LOG_BOAT,
LOG_GRND,
LOG_COUNT};

#endif  // #ifndef SETUP_H
