/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          readmis.h

    Author:             Ned Way

    File Date:          11:36:37  Wed  28-Jul-1993

    File Description:
                        header file for readmis.c
                F14 mission generation

    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/



#define MAX_MI_OBJECTS   200
#define MAX_MI_PATHS     200
#define MAX_MI_WPS       800
#define MAX_MI_ACTIONS  1200



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

        long            dramaticEvent ; // what happened to him
        int             time ;          // time of dramatic event

        long            live_or_die ;   // flag indicating whether death of this
                                        //      thing is goal of mission
                                        // separated into 4 bit fields


        } _Object ;


typedef struct _path_ {

        char            *name ;
        unsigned        wp0 ;
        unsigned        numWPs ;

        } _Path ;

typedef struct _station_object {

        int     group ;
        int     number ;
        long    resupplyDelay ;         // packed long by levels


        } _StationObject ;

typedef struct _station_ {


        int     numObjects ;

        _StationObject objects[20] ;


        } _Station ;


extern unsigned _world, _time, _weather, _loadout, _ceiling ;
extern char     *_briefing, *_description ;


extern int       _objectCount, _pathCount, _wpCount, _actionCount ;

extern _Path     _paths[MAX_MI_PATHS],     *_path ;
extern _WayPoint _wayPoints[MAX_MI_WPS],   *_wayPoint ;
extern _Action   _actions[MAX_MI_ACTIONS], *_action ;

extern _Object   _objects[MAX_MI_OBJECTS], *_object ;


#define MAX_STATIONS    10

extern _list     _obStationNames[MAX_STATIONS], *_obStation ;
extern _Station  _stations[MAX_STATIONS], *_station ;
