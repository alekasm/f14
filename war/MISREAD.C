/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          misread.c

    Author:             Ned Way

    File Date:          14:47:29  Thu  29-Jul-1993

    File Description:
                        mission file reader for F-14


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#include <ctype.h>
#include <io.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readfile.h"
#include "readdef.h"
#include "readmis.h"

#include "mission.h"    // contains mission header structure


static char *_ThisFile = __FILE__ ;

#define LOCATION        _ThisFile, __LINE__


char _misBuff[20480] ;


_MissionHeader __missionHeader ;

_MissionHeader *_missionHeader = &__missionHeader ; ;


char    _defName[64] = "basedef",
        _misName[64],
        _oName[64]   = "mission.f14" ;

_Object   *_object ;
_Path     *_path ;
_WayPoint *_wayPoint ;
_Action   *_action ;
_Station  *_station ;

char      *_briefing ;


int main (int argc, char **argv) {

char *defName=_defName, *misName="mission.f14" ;

FILE *iFile ;



        if (argc != 1)
                misName = argv[1] ;

        assert ( (iFile = fopen (defName, "rt") ) != NULL, LOCATION,
                 "Error opening Input File %s\r\n", defName) ;


        _lineIn = 0 ;
        ParseDefFile (iFile, _iBuff, defName) ;
        fclose (iFile) ;


        assert ( (iFile = fopen (misName, "rb") ) != NULL, LOCATION,
                 "Error opening Input File %s\r\n", misName) ;


        fread (_misBuff, 1, (size_t) _filelength (_fileno (iFile) ), iFile) ;
        fclose (iFile) ;


        _missionHeader = (_MissionHeader *) _misBuff ;

        _object   = (_Object *)   ( (long) _misBuff + _missionHeader->objectLoc) ;

        _path     = (_Path *)     ( (long) _misBuff + _missionHeader->pathLoc) ;
        _wayPoint = (_WayPoint *) ( (long) _misBuff + _missionHeader->wpLoc) ;
        _action   = (_Action *)   ( (long) _misBuff + _missionHeader->actionLoc) ;
        _station  = (_Station *)  ( (long) _misBuff + _missionHeader->stationLoc) ;

        _briefing = (char *)      ( (long) _misBuff + _missionHeader->briefingLoc) ;


        ReportFileName () ;
        ReportWorld () ;
        ReportBriefing (78) ;
        ReportDescription() ;
        ReportTime () ;
        ReportWeather () ;
        ReportCeiling() ;
        ReportLoadout () ;
        ReportStations() ;
        ReportPaths () ;
        ReportObjects () ;

}

int ReportFileName () {

        printf (";\r\n;        %s\r\n;\r\n\r\n", _missionHeader->name) ;

}


int ReportWorld () {

        printf ("[WORLD]\r\n"
                "%s\r\n\r\n",
                _obWorlds[_missionHeader->world].name) ;


}

int ReportBriefing (int lineLen) {

int len=0, wordLen ;
char *word ;

        printf ("[BRIEFING]\r\n") ;


        word = tok (_briefing, " ") ;

        while (word != NULL) {

                wordLen = strlen (word) ;

                if ( len + wordLen < 80) {
                        printf ("%s", word) ;
                        len += wordLen ;
                        }
                    else {
                        printf ("\r\n") ;
                        printf ("%s", word) ;
                        len = wordLen ;
                }

                if (len < 80) {
                        printf (" ") ;
                        len++ ;
                }


                word = tok (NULL, " ") ;
        }


        printf ("\r\n\r\n") ;

}

int ReportDescription () {

        printf ("[DESCRIPTION]\r\n"
                "%s\r\n\r\n",
                _missionHeader->description
               ) ;


}

int ReportTime () {

        printf ("[TIME]\r\n"
                "%2d:%2d\r\n\r\n",
                _missionHeader->time / 60 ? _missionHeader->time / 60 : 24,
                _missionHeader->time % 60
               ) ;

}

int ReportWeather () {

        printf ("[WEATHER]\r\n"
                "%s\r\n\r\n",
                _obWeathers[_missionHeader->weather].name) ;


}

int ReportCeiling () {

        printf ("[CEILING]\r\n"
                "%u\r\n\r\n",
                _missionHeader->ceiling
               ) ;

}

int ReportLoadout () {


        printf ("[LOADOUT]\r\n"
                "%s\r\n\r\n",
                _obLoadouts[_missionHeader->loadout].name) ;

}

int ReportPaths () {

_Path *path=_path ;
_WayPoint *wayPoint ;
_Action *action ;
int actionIndex ;


int i, j, k ;

        printf ("[PATHS]\r\n") ;

        for (i=0; i<_missionHeader->numPaths; i++) {

                //printf ("%16.16s =", path->name) ;
                printf ("%8d =\t", i) ;

                wayPoint = &_wayPoint[path->wp0] ;

                for (j=0; j<path->numWPs; j++) {

                        if (wayPoint->x != -2) {

                                if (wayPoint->x % 4096 || wayPoint->y % 4096)
                                        printf ("( %8ld, %8ld ) ", wayPoint->x, wayPoint->y) ;
                                    else
                                        printf ("( %8ld, %8ld ) ", wayPoint->x>>12, wayPoint->y>>12) ;

                                }
                            else {      // waypoint is relative to object

                                wayPoint++ ;
                                printf ("{ %8ld, %8ld }:%d ", wayPoint->x, wayPoint->y, (wayPoint-1)->y) ;

                        }


                        action = &_action[wayPoint->actionIndex] ;

                        for (k=0; k<wayPoint->numActions; k++) {

                                actionIndex = FindValue (action->action, _obPathActions) ;
                                printf ("%s", _obPathActions[actionIndex].name) ;

                                if (_obPathActions[actionIndex].needEquals ||
                                    _obPathActions[actionIndex].needPathMatch
                                   ) {
                                        printf ("=%u ", action->value) ;

                                        }
                                    else
                                        printf (" ") ;

                                action++ ;

                        }


                        wayPoint++ ;
                        printf ("\r\n\t\t") ;

                }

                printf ("\r\n") ;

                path++ ;

        }

        printf ("\r\n\r\n") ;

}

int FindValue (int value, _list *list) {

int count=0 ;

        while (list->name != NULL)
                if (list->value == value)
                        return count ;
                    else {

                        list++ ;
                        count++ ;
                }


        return MA_ERROR ;

}

void UnpackLong (long num, int *a, int *b, int *c, int *d) ;

int ReportObjects () {

int i=0 ;
_Object *object=_object ;
int objectNum ;

int a, b, c, d ;

        for (i=0; i<_missionHeader->numObjects; i+= (object->number & 0xff), object+=(object->number & 0xff) ) {

                objectNum = FindValue(object->object, _obObjectNames) ;

                printf ("%s\t\t", _obObjectNames[objectNum].name) ;

                if (object->x & 4095 || object->y & 4095)
                        printf ("loc=( %8ld, %8ld )\r\n", object->x, object->y) ;
                    else
                        printf ("loc=( %8ld, %8ld )\r\n", object->x>>12, object->y>>12) ;

                if ((int) object->z != -1)
                        printf ("\t\televation=%u\r\n", object->z) ;
                    else
                        printf ("\t\televation=-1\r\n") ;

                printf ("\t\tspeed=%d\r\n", object->speed) ;

                if (object->type != -1)
                        printf ("\t\ttype=%s\r\n", _obTypes[FindValue(object->type, _obTypes)].name) ;
                    else
                        printf ("\t\ttype=-1\r\n") ;

                if (*object->name != ' ')
                        printf ("\t\tname=%s\r\n", object->name) ;

                if (*object->callSign != ' ')
                        printf ("\t\tcallSign=%s\r\n", object->callSign) ;


                if (object->formation != -1)
                        printf ("\t\tformation=%s\r\n", _obFormations[FindValue(object->formation, _obFormations)].name) ;
                    else
                        printf ("\t\tformation=-1\r\n") ;

                printf ("\t\tside=%s\r\n",      _obSides[FindValue(object->side, _obSides)].name) ;

                printf ("\t\tpath=%d\r\n",      object->path) ;

                UnpackLong (object->number, &a, &b, &c, &d) ;
                printf ("\t\tnumber=%d, %d, %d, %d\r\n", a, b, c, d) ;


                if (object->live_or_die)
                        UnpackLong (object->live_or_die, &a, &b, &c, &d) ;
                        if (strcmp (_obSides[FindValue(object->side, _obSides)].name, "enemy") == 0)
                                printf ("\t\tgoal=%d, %d, %d, %d\r\n", a, b, c, d) ;
                            else
                                printf ("\t\tsurvive=%d, %d, %d, %d\r\n", a, b, c, d) ;

                UnpackLong (object->quality, &a, &b, &c, &d) ;
                printf ("\t\tquality=%d, %d, %d, %d\r\n", a, b, c, d) ;


                if (object->stationNumber != -1)
                        printf ("\t\tstation = %d\r\n", object->stationNumber) ;


                printf ("\r\n") ;


        }



}

void UnpackLong (long num, int *a, int *b, int *c, int *d) {

        *a = num >> 24 ;
        *b = (num & 0x00ff0000) >> 16 ;
        *c = (num & 0x0000ff00) >>  8 ;
        *d = num & 0xff ;

}

int ReportStations () {


        printf ("[stations]\r\n"
                "; there are %d stations\r\n\r\n", _missionHeader->numStations) ;



}
