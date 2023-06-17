/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          readmis.c

    Author:             Ned Way

    File Date:          10:42:07  Wed  28-Jul-1993

    File Description:
                        reads f14 mission files


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#include <ctype.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "readfile.h"
#include "readdef.h"
#include "readmis.h"




static char *_ThisFile = __FILE__ ;

#define LOCATION        _ThisFile, __LINE__



#define MI_WORLD               10
#define MI_TIME                11
#define MI_WEATHER             12
#define MI_LOADOUT             13
#define MI_BRIEFING            14
#define MI_PATHS               15
#define MI_OBJECTS             16
#define MI_DEBRIEF             17
#define MI_SCORING             18
#define MI_CEILING             19
#define MI_DESCRIPTION         20
#define MI_STATIONS            21



_commands _miCommands[] = {
        {"[world]",             MI_WORLD},
        {"[time]",              MI_TIME},
        {"[weather]",           MI_WEATHER},
        {"[ceiling]",           MI_CEILING},
        {"[description]",       MI_DESCRIPTION},
        {"[loadout]",           MI_LOADOUT},
        {"[briefing]",          MI_BRIEFING},
        {"[paths]",             MI_PATHS},
        {"[stations]",          MI_STATIONS},
        {"[objects]",           MI_OBJECTS},
        {"[debriefing]",        MI_DEBRIEF},
        {"[scoring]",           MI_SCORING},

        {"[end]",               CO_END},
        {"final",               CO_LIST_END}

        };


unsigned _world, _time, _weather, _loadout, _ceiling=-1 ;
char     *_briefing="", *_description = "" ;

/* THE ACTUAL NUMBERS FOR THESE ARE IN readmis.h

        THESE MAY VARY FROM THOSE


#define MAX_MI_OBJECTS   100
#define MAX_MI_PATHS     100
#define MAX_MI_WPS       500
#define MAX_MI_ACTIONS  1000

*/

//int       _objectCount, _pathCount=-1, _wpCount, _actionCount ;

_Path     _paths[MAX_MI_PATHS],     *_path=&_paths[-1] ;
_WayPoint _wayPoints[MAX_MI_WPS],   *_wayPoint=_wayPoints ;
_Action   _actions[MAX_MI_ACTIONS], *_action=_actions ;

_Object   _objects[MAX_MI_OBJECTS], *_object=&_objects[-1] ;

#define MAX_STATIONS    10

_list     _obStationNames[MAX_STATIONS], *_obStation=&_obStationNames[-1] ;
_Station  _stations[MAX_STATIONS], *_station=&_stations[-1] ;


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

char *GetBriefing (FILE *f, char *buff, char *iName) ;


int ParseMisFile (FILE *f, char *buff, char *iName) {

int len, command ;
char *name ;
char line[IBUFF_MAX] ;

        len = GetLine (f, buff, iName) ;

        while ( ( command = ParseCommand (buff, _miCommands) ) != CO_END) {

                switch (command) {

                        case MI_WORLD:

                                len = GetLine (f, buff, iName) ;
                                name = tok (buff, " \t=") ;

                                _world = MatchList (name, _obWorlds, VALUE) ;

                                assert (_world != MA_ERROR, LOCATION,
                                        "Illegal World Name entered\r\n"
                                        "File %s, line %d name:%s\r\n",
                                        iName, _lineIn, name) ;

                                len = GetLine (f, buff, iName) ;

                                break ;


                        case MI_TIME:
                                len = GetLine (f, buff, iName) ;
                                name = tok (buff, " \t=") ;
                                strcpy (line, buff) ;

                                _time  = atoi (tok (name, ":")) * 60 ;
                                _time += atoi (tok (NULL, ":")) ;

                                assert (_time > 60 && _time < 1500, LOCATION,
                                        "Illegally Entered Time: %s\r\n"
                                        "File %s, Line %d\r\n",
                                        iName, _lineIn) ;

                                len = GetLine (f, buff, iName) ;

                                break ;

                        case MI_WEATHER:

                                len = GetLine (f, buff, iName) ;
                                name = tok (buff, " \t=") ;

                                _weather = MatchList (name, _obWeathers, VALUE) ;

                                assert (_weather != MA_ERROR, LOCATION,
                                        "Illegal Weather entered\r\n"
                                        "File %s, line %d name:%s\r\n",
                                        iName, _lineIn, name) ;

                                len = GetLine (f, buff, iName) ;

                                break ;

                        case MI_CEILING:

                                len = GetLine (f, buff, iName) ;

                                _ceiling = (unsigned) atol (buff) ;

                                len = GetLine (f, buff, iName) ;

                                break ;

                        case MI_LOADOUT:

                                len = GetLine (f, buff, iName) ;
                                name = tok (buff, " \t=") ;

                                _loadout = MatchList (name, _obLoadouts, VALUE) ;

                                assert (_loadout != MA_ERROR, LOCATION,
                                        "Illegal loadout entered\r\n"
                                        "File %s, line %d name:%s\r\n",
                                        iName, _lineIn, name) ;

                                len = GetLine (f, buff, iName) ;

                                break ;


                        case MI_DESCRIPTION:

                                len = GetLine (f, buff, iName) ;

                                _description = AddBufferString (buff) ;

                                len = GetLine (f, buff, iName) ;

                                break ;


                        case MI_BRIEFING:

                                _briefing = GetBriefing (f, buff, iName) ;

                                break ;

                        case MI_PATHS:

                                GetPaths (f, buff, iName) ;

                                break ;


                        case MI_STATIONS:

                                GetStations (f, buff, iName) ;

                                break ;

                        case MI_OBJECTS:

                                GetObjects (f, buff, iName) ;

                                break ;

                        case MI_DEBRIEF:

                                break ;

                        case MI_SCORING:

                                break ;

                        default:
                                assert (FALSE, LOCATION,
                                        "Illegal command %s in file %s line %d\r\n",
                                        buff, iName, _lineIn) ;


                }




        }

        ResolveObjectsInPaths (iName) ;
        ResolveObjectsInObjects (iName) ;
        ResolveObjectLocations (iName) ;

        return 0 ;

}



/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int GetStations (FILE *f, char *buff, char *iName) {


int len, bLen, count=0, value=0 ;
char section[36] ;
char *name ;

        strcpy (section, buff) ;

        len = GetLine (f, buff, iName) ;

        name = tok (buff, " \t=") ;

        while (*buff != '[') {

                assert (stricmp (name, "station") == 0, LOCATION,
                        "Station definition must start off with \"station =\"\r\n",
                        "File %s, line %d\r\n",
                        iName, _lineIn) ;


                _obStation++ ;

                name = tok (NULL, " \t=") ;

                strlwr (name) ;

                _obStation->name  = AddBufferString (name) ;
                _obStation->value = _obStation - _obStationNames ;


                len = GetLine (f, buff, iName) ;
                name = tok (buff, " \t=") ;
        }

        _obStation++ ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


char *GetBriefing (FILE *f, char *buff, char *iName) {


int len, bLen, count=0, value=0 ;
char *briefing=NULL ;
char section[36] ;

        strcpy (section, buff) ;

        bLen = GetLine (f, buff, iName) ;

        if (*buff != '[')
                briefing = AddBufferString (buff) ;

        len = GetLine (f, buff, iName) ;

        while (*buff != '[') {

                briefing[bLen] = ' ' ;
                AddBufferString (buff) ;
                bLen += len + 1 ;

                len = GetLine (f, buff, iName) ;
        }

        return briefing ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#define MI_FORMATION            0
#define MI_WAYPOINT             1
#define MI_GORILLABOMB          2
#define MI_REFUEL               3
#define MI_CRUISE_MISS          4
#define MI_BOMBER_FORMATION     5
#define MI_SAR                  6
#define MI_SAR_CAP              7
#define MI_GORILLA_COVER        8
#define MI_WAYPOINT_ESCORTABLE  9
#define MI_AWACS               10
#define MI_GET_FUEL            11
#define MI_ATTACK              12

#define MI_HAWKEYE             13
#define MI_ELEVATION           14
#define MI_SPEED               15



/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int GetPaths (FILE *f, char *buff, char *iName) {

int len, count=0, value=0 ;
char *name ;
char section[36] ;


        strcpy (section, buff) ;

        len = GetLine (f, buff, iName) ;
        name = tok (buff, " \t=") ;

        while (*buff != '[') {
                if (*name != '(') {
                        _path++ ;
                        strlwr (name) ;
                        _path->name = AddBufferString (name) ;
                        _path->wp0  = _wayPoint - _wayPoints ;

                        name = tok (NULL, " \t=") ;

                        if (*name == '('  || *name == '{') {
                                AddWayPoint (iName, name) ;
                                AddActions (iName) ;
                        }

                        }

                    else if (*name == '('  || *name == '{') {

                        AddWayPoint (iName, name) ;
                        AddActions (iName) ;

                }

                _wayPoint++ ;
                _path->numWPs++ ;

                assert ( (_wayPoint - _wayPoints) < MAX_MI_WPS, LOCATION,
                        "Number of way points entered in file %s exceeds max of %d\r\n",
                        iName, MAX_MI_WPS) ;

                len = GetLine (f, buff, iName) ;
                name = tok (buff, " \t=") ;
        }

        return 0 ;
}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int ResolveObjectsInPaths (char *iName) {

_Path *path=_paths ;
int i, value ;

        while (path <= _path) {

                for (i=path->wp0; i< path->wp0+path->numWPs; i++) {

                        if (_wayPoints[i].x == -1 ||    // location same as an object
                            _wayPoints[i].x == -2) {    // location relative to object
                                value = FindObjectName ( (char *) _wayPoints[i].y) ;

                                assert (value != MA_ERROR, LOCATION,
                                        "Unknown object name entered as alias in path list\r\n"
                                        "File %s, path name %s, object name %s\r\n",
                                        iName, path->name, (char *) _wayPoints[i].y ) ;

                                _wayPoints[i].y = value ;
                        }
                }

                path++ ;

        }

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int ResolveObjectsInObjects (char *iName) {

_Object *object=_objects ;
int value ;

        while (object <= _object) {

                if (object->x == -1) {
                        value = FindObjectName ( (char *) object->y) ;

                        assert (value != MA_ERROR, LOCATION,
                                "Unknown object name entered as alias for location in object list\r\n"
                                "File %s, object name %s, alias object name %s\r\n",
                                iName, object->name, (char *) object->y ) ;

                        object->y = value ;

                        }

                object++ ;

        }

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int ResolveObjectLocations (char *iName) {

_Object *object=_objects ;
int value ;

        while (object <= _object) {

                if (object->x == -1) { // loc must come from another object def

                        assert (_objects[object->y].x != -1 && _objects[object->y].x != 0,
                                LOCATION,
                                "Object location indirected to object with no location specified\r\n"
                                "file %s, object type %s, indirected to object #%d\r\n",
                                iName, _obObjectNames[object->object], _objects[object->y]) ;

                        object->x = _objects[object->y].x ;
                        object->y = _objects[object->y].y ;
                        object->z = _objects[object->y].z ;

                        }
                    else if (object->x == 0) {  // loc must come from path

                        assert (object->path != -1, LOCATION,
                                "Object with no specific location or path defined\r\n"
                                "file %s, object type %s, object name %s, object number in file %d\r\n",
                                iName, _obObjectNames[object->object], object-_objects) ;

                        object->x = _wayPoints[_paths[object->path].wp0].x ;
                        object->y = _wayPoints[_paths[object->path].wp0].y ;

                }

                object++ ;

        }





}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int FindObjectName (char *name) {

_Object *object=_objects ;


        while (object <= _object) {

                if (stricmp (name, object->name) == 0)
                        return (object - _objects) ;

                object++ ;

        }


        return MA_ERROR ;

}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

#define FT_PER_NMI      6076.12

int AddWayPoint (char *iName, char *buff) {

char *name, *pos ;
int numIn ;
int relativeWP = (*buff == '{' ? TRUE : FALSE) ;
double x, y ;

        if (relativeWP) {
                _wayPoint->x = -2 ;
                _wayPoint++ ;
                _path->numWPs++ ;

        }       // must adjust y with object number later



        //assert (strlen(buff) == 1, LOCATION,
        //        "Opening paren must be followed by at least one space\r\n"
        //        "File %s, line %d\r\n",
        //        iName, _lineIn) ;

        if (strlen (buff) == 1) {

                name = tok (NULL, ")}") ;
                name++ ;        // ?????????????

                }
            else {
                name = buff + 1 ;
                tok (NULL, ")}") ;
        }

// must remove leading & trailing blanks for object name
        name += strspn (name, " \t") ;

        while (name[strlen(name)-1] == ' ' || name[strlen(name)-1] == '\t')
               name[strlen(name)-1] = '\0' ;

        if ( (isdigit (*name)) || (*name == '-')) {

                if (! relativeWP)
                        numIn = sscanf (name, "%d, %d", &_wayPoint->x, &_wayPoint->y) ;
                    else
                        numIn = sscanf (name, "%lf, %lf", &x, &y) ;

                assert (numIn == 2, LOCATION,
                        "Improperly entered waypoint\r\n"
                        "File %s, line %d\r\n",
                        iName, _lineIn) ;

                if (! relativeWP) {

                        if (_wayPoint->x < 256)
                                _wayPoint->x *= 4096 ;

                        if (_wayPoint->y < 256)
                                _wayPoint->y *= 4096 ;

                        }
                    else {

                        _wayPoint->x = (long) (x * FT_PER_NMI / 4.) ;
                        _wayPoint->y = (long) (y * FT_PER_NMI / 4.) ;

                }

                }

            else {

                if ( (pos = strchr (name, ')' ) ) != NULL)
                        *pos = '\0' ;

                assert (! relativeWP, LOCATION,
                        "A relative waypoint must be specified as a distance relative to a object:\r\n"
                        "       { x, y }:home\r\n"
                        "File %s, line %d\r\n",
                        iName, _lineIn
                       ) ;


                _wayPoint->x = -1 ;
                _wayPoint->y = (long) AddBufferString (name) ;

                //assert (strchr(name, ')') == NULL, LOCATION,
                //        "An Object Name in a waypoint must be followed by a space\r\n"
                //        "File %s, line %d\r\n",
                //        iName, _lineIn) ;

        }


        if (relativeWP) {

                _wayPoint-- ;

                tok (NULL, ":") ;
                name = tok (NULL, " \t") + 1 ;

                assert (strlen (name) != 0, LOCATION,
                        "Error entering relative waypoint\r\n"
                        "probably caused by a space after the colon\r\n"
                       ) ;

                _wayPoint->x = -2 ;
                _wayPoint->y = (long) AddBufferString (name) ;

                _wayPoint++ ;
        }

}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int AddActions (char *iName) {

char *name ;
int  actionNum ;
int first=TRUE ;

        name = tok (NULL, " })\t=") ;

        while (name != NULL) {

                actionNum = MatchList (name, _obPathActions, INDEX) ;

                assert (actionNum != MA_ERROR, LOCATION,
                        "Unknown action specified\r\n"
                        "File %s line %d action %s\r\b\n",
                        iName, _lineIn, name) ;



                _action->action = _obPathActions[actionNum].value ;

                if (first) {
                        _wayPoint->actionIndex = _action - _actions ;
                        first = FALSE ;
                }

                if (_obPathActions[actionNum].needEquals == EQUALS_NEC) {

                        name = tok (NULL, " \t=") ;

                        if (isdigit (*name) )
                                _action->value = atoi (name) ;
                            else
                                _action->value = MatchList (name, _obFormations, VALUE) ;
                        }

                    else if (_obPathActions[actionNum].needPathMatch) {
                        name = tok (NULL, " \t=") ;
                        _action->value = MatchPaths (name, _paths) ;

                        assert (_action->value != MA_ERROR, LOCATION,
                                "Unknown path name entered\r\n"
                                "File %s line %d Path Name %s\r\n",
                                iName, _lineIn, name) ;

                        }

                    else
                        _action->value = -1 ;


                _wayPoint->numActions++ ;
                _action++ ;

                assert ( (_action - _actions) < MAX_MI_ACTIONS, LOCATION,
                        "Number of actions at waypoints entered in file %s exceeds max of %d\r\n",
                        iName, MAX_MI_ACTIONS) ;


                name = tok (NULL, " \t=") ;

        }

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int MatchPaths (char *match, _Path *list) {

int count=0 ;

        strlwr (match) ;

        for (;list->name != NULL; list++, count++) {

                if ( *( (int *) match) == *( (int *) list->name) )
                        if (strcmp (match, list->name) == 0)
                                return count ;

        }

        return MA_ERROR ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

long PackLong (char *str, char *iName) {

int numIn, a, b, c, d ;

        //numIn = sscanf (tok (NULL, " \t="), "%u, %u, %u, %u",
        //                &a, &b, &c, &d) ;

        numIn = sscanf (str, "%u, %u, %u, %u",
                        &a, &b, &c, &d) ;

        assert (numIn == 4, LOCATION,
                "Improper object number definition - should be\r\n"
                "number = #, #, #, #\r\n"
                "File %s, line %d\r\n",
                 iName, _lineIn) ;

        assert ( (a <= b) && (b <= c) && (c <= d), LOCATION,
                "entered 4 number string must have lowest numbers on left\r\n"
                "File %s, line %d\r\n"
                "errant line: %s\r\n",
                 iName, _lineIn, str) ;



        return ( ((long) a << 24) | ( (long) b << 16) | (c << 8) | d) ;


}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

#define MI_OB_CALLSIGN  0
#define MI_OB_ELEVATION 1
#define MI_OB_FORMATION 2
#define MI_OB_GOAL      3
#define MI_OB_HEADING   4
#define MI_OB_LOCATION  5
#define MI_OB_NAME      6
#define MI_OB_NUMBER    7
#define MI_OB_PATH      8
#define MI_OB_SIDE      9
#define MI_OB_SPEED    10
#define MI_OB_TYPE     11
#define MI_OB_SKILL    12
#define MI_OB_SURVIVE  13
#define MI_OB_STATION  14

_commands _miObCommands[] = {

        {"callsign",     MI_OB_CALLSIGN},
        {"elevation",    MI_OB_ELEVATION},
        {"elev",         MI_OB_ELEVATION},
        {"formation",    MI_OB_FORMATION},
        {"form",         MI_OB_FORMATION},
        {"goal",         MI_OB_GOAL},
        {"heading",      MI_OB_HEADING},
        {"location",     MI_OB_LOCATION},
        {"loc",          MI_OB_LOCATION},
        {"name",         MI_OB_NAME},
        {"number",       MI_OB_NUMBER},
        {"path",         MI_OB_PATH},
        {"side",         MI_OB_SIDE},
        {"speed",        MI_OB_SPEED},
        {"type",         MI_OB_TYPE},
        {"skill",        MI_OB_SKILL},
        {"survive",      MI_OB_SURVIVE},
        {"station",      MI_OB_STATION},
        {"final",        CO_LIST_END}

        };


_Object _defaultObject = {

        0,                      // object mig23a
        -1,                     // will default to plane in Scott's code
        -1,                     // not assigned to a station
        "                ",     // name
        "                ",     // callSign
        0,                      // squad member
        0x02020202,             // low quality (0 - 4 is range)
        2,                      // enemy side
        0, -1, -1,              // x, y & z
        -1,                     // speed
        24576,                  // heading South East
        0x01010101,             // number
        -1,                     // default will be cruise
        -1,                     // no assigned path

        0, 0,                   // no dramatic event or time

        0                       // not the goal, neither must it survive

        } ;


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int GetObjects (FILE *f, char *buff, char *iName) {

int len, count=0, value=0 ;
char *name ;
char section[36] ;
//_Object a ;
int i, numIn ;


        strcpy (section, buff) ;

        len = GetLine (f, buff, iName) ;

        while (*buff != '[') {

                name = tok (buff, " \t=") ;

                if ( (value = MatchList (name, _obObjectNames, INDEX) ) != MA_ERROR) {


                 // must make copies of previous object up to number in object

                        if (_object>_objects) {

                                for (i=1; i<(_object->number & 0xff); i++) {

                                        *(_object+i) = *_object ;
                                        (_object+i)->number = 1 ;
                                        (_object+i)->squadMember = i ;

                                }

                                _object += (_object->number & 0xff) ;

                                }
                            else
                                _object++ ;

                        *_object = _defaultObject ;
                        _object->side   = _obObjectNames[value].side ;
                        _object->object = _obObjectNames[value].value ;


                        if ( (name = tok (NULL, " \t=") ) == NULL) {

                                len = GetLine (f, buff, iName) ;

                        }

                }

                value = ParseCommand (name, _miObCommands) ;


                switch (value) {


                        case MI_OB_CALLSIGN:

                                strncpy (_object->callSign, tok (NULL, " \t="), sizeof (_object->callSign)-1 ) ;

                                break ;

                        case MI_OB_ELEVATION:

                                _object->z = atoi (tok (NULL, " \t=") ) ;

                                break ;

                        case MI_OB_FORMATION:

                                _object->formation = MatchList (name = tok (NULL, " \t="), _obFormations, VALUE) ;

                                assert (_object->formation != MA_ERROR, LOCATION,
                                        "Unknown formation entered\r\n"
                                        "File %s, line %d, formation %s\r\n",
                                         iName, _lineIn, name) ;

                                break ;

                        case MI_OB_GOAL:        // should check here to ensure is enemy

                                //_object->goal = TRUE ;

                                tok (NULL, "=") ;
                                _object->live_or_die = PackLong (tok (NULL, NULL)+1, iName) ;

                                break ;

                        case MI_OB_HEADING:

                                _object->heading = (unsigned) atol (tok (NULL, " \t=") ) ;

                                break ;

                        case MI_OB_LOCATION:

                                name = tok (NULL, "(") ;
                                name = tok (NULL, ")") ;
                                name++ ;
                                name += strspn (name, " \t") ;

                                if (isdigit (*name) ) {
                                        numIn = sscanf (name, "%ld, %ld", &_object->x, &_object->y) ;
					
                                        assert (numIn == 2, LOCATION,
                                                "Improperly entered object location\r\n"
                                                "File %s, line %d\r\n",
                                                iName, _lineIn) ;

                                        if (_object->x < 256)
                                                _object->x *= 4096 ;

                                        if (_object->y < 256)
                                                _object->y *= 4096 ;

                                        }

                                    else {
                                        _object->x = -1 ;
                                        _object->y = (long) AddBufferString (name) ;

                                }

                                break ;

                        case MI_OB_NAME:

                                strncpy (_object->name, tok (NULL, " \t="), sizeof (_object->name)-1 ) ;

                                break ;

                        case MI_OB_NUMBER:

                                //_object->number = atoi (tok (NULL, " \t=") ) ;

                                tok (NULL, "=") ;
                                _object->number = PackLong (tok (NULL, NULL)+1, iName) ;

                                break ;




                        case MI_OB_PATH:

                                _object->path = MatchPaths (name = tok (NULL, " \t="), _paths ) ;

                                assert (_object->path != MA_ERROR, LOCATION,
                                        "Unknown path entered\r\n"
                                        "File %s, line %d, path %s\r\n",
                                         iName, _lineIn, name) ;


                                //if (_object->x == 0) {
                                //        _object->x = -1 ;
                                //        _object->y = _object->path ;
                                //
                                //}

                                break ;

                        case MI_OB_SIDE:

                                _object->side = MatchList (name = tok (NULL, " \t="), _obSides, VALUE ) ;

                                assert (_object->side != MA_ERROR, LOCATION,
                                        "Unknown side entered\r\n"
                                        "File %s, line %d, side %s\r\n",
                                         iName, _lineIn, name) ;


                                break ;

                        case MI_OB_SPEED:

                                _object->speed = atoi (tok (NULL, " \t=") ) ;

                                break ;

                        case MI_OB_TYPE:

                                _object->type = MatchList (name = tok (NULL, " \t="), _obTypes, VALUE ) ;

                                assert (_object->type != MA_ERROR, LOCATION,
                                        "Unknown object type entered\r\n"
                                        "File %s, line %d, type %s\r\n",
                                         iName, _lineIn, name) ;



                                _object->side = _obTypes[_object->type].side ;

                                break ;

                        case MI_OB_SKILL:

                                tok (NULL, "=") ;
                                _object->quality = PackLong (tok (NULL, NULL)+1, iName) ;

                                break ;

                        case MI_OB_SURVIVE:     // should check here to ensure is friend

                                tok (NULL, "=") ;
                                _object->live_or_die = PackLong (tok (NULL, NULL)+1, iName) ;

                                break ;

                        case MI_OB_STATION:

                                _object->stationNumber = MatchList (name = tok (NULL, " \t="), _obStationNames, INDEX) ;


                                assert (_object->stationNumber != MA_ERROR, LOCATION,
                                        "Bad Station Name entered in object list\r\n"
                                        "File %s, line %d, section %s: bad entry %s\r\n",
                                        iName, _lineIn, section, name) ;


                                break ;

                        default:


                                assert (FALSE, LOCATION,
                                        "Either a bad object name or command has been entered\r\n"
                                        "File %s, line %d, section %s: bad entry %s\r\n",
                                        iName, _lineIn, section, name) ;


                                break ;


                }

                len = GetLine (f, buff, iName) ;

        }

        return 0 ;
}
