/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          mission.h

    Author:             Ned Way

    File Date:          12:52:16  Mon  30-Aug-1993

    File Description:
                        mission file header structure def


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


typedef struct _missionHeader_ {


        char            name[16] ;      // name of input file (description ??)
        char            description[32];// short description of mission
        int             world ;
        int             time ;          // in minutes since midnight
        int             weather ;
        int             ceiling ;       // cloud deck level
        int             loadout ;

        int             numObjects ;
        int             numPaths ;
        int             numWPs ;
        int             numActions ;
        int             numStations ;

        long            objectLoc ;     // dword aligned offset into file
        long            pathLoc ;       //      of beginning of each section
        long            wpLoc ;
        long            actionLoc ;
        long            stationLoc ;


        long            briefingLoc ;   // dword aligned offset into file
                                        //      of briefing text - NOT Necessary
                                        //      for game - ONLY WRAPPERS
        int             briefingLen ;

        } _MissionHeader ;
