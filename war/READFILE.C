/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          readfile.c

    Author:             Ned Way

    File Date:          10:39:00  Wed  28-Jul-1993

    File Description:
                        general read routines for F14 missions


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#include <ctype.h>
#include <direct.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readfile.h"

static char *_ThisFile = __FILE__ ;

#define LOCATION        _ThisFile, __LINE__

char _strBuffer[20480] ;
unsigned _strBuffLoc = 0 ;

#define IBUFF_MAX       255
char _iBuff[IBUFF_MAX] ;


/**** GLOBALS ****/

int _lineIn = 0 ;
int _errorOcurred = FALSE ;

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int GetLine (FILE *f, char *buff, char *iName) {

int len, whitesp, comment  ;

        for (_lineIn++;;_lineIn++) {

                assert (fgets (buff, IBUFF_MAX, f) != NULL, LOCATION,
                        "Unexpected end of input file %s \r\n", iName) ;

                len = strlen (buff) ;

                assert (buff[len] != '\n', LOCATION,
                        "Line %d too long in input file %s\r\n", _lineIn, iName) ;


                buff[--len] = '\0' ;

                if (len == 0)
                        continue ;

                len -= whitesp = strspn (buff, " \t") ;

                len -= StripComments (&buff[whitesp], len) ;

                len -= StripTrail (&buff[whitesp], len) ;

                if (len == 0)
                        continue ;

                strcpy (buff, &buff[whitesp]) ;

                return len ;

        }


}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

//      returns the number of characters removed from an input string due
//              to comments or explicit semi-colons


int StripComments (char *buff, int len) {

int semiLoc=0, numOut=0  ;

        if (len == 0)
                return 0 ;

        if (*buff == ';')
                return len ;

        while ( (semiLoc = strcspn (&buff[semiLoc], ";") ) != len) {

                if (buff[semiLoc-1] == '\\') {   // explicit semicolon in text
                        strcpy (&buff[semiLoc-1], &buff[semiLoc]) ;
                        numOut++ ;

                        }

                    else {
                        buff[semiLoc] = '\0' ;

                        return (len - semiLoc + numOut) ;
                }

        }

        return numOut ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int StripTrail (char *buff, int len) {

int numOut=0 ;

        if (len == 0)
                return 0 ;

        while (buff[len-1-numOut] == ' '  ||
               buff[len-1-numOut] == '\t'
              ) {

                numOut++ ;

        }

        buff[len-numOut] = '\0' ;

        return numOut ;

}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

char *AddBufferString (char *s) {

char *retVal ;

        retVal = &_strBuffer[_strBuffLoc] ;

        strcpy (retVal, s) ;
        _strBuffLoc += strlen (s) + 1 ;

        return retVal ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int ParseCommand (char *buff, _commands *commands) {

char *s ;

        //if (*buff != '[')
        //        return CO_NOT_COMMAND ;

        strlwr (buff) ;

        while (commands->value != CO_LIST_END) {

                if (*( (int *) buff) == *( (int *) commands->command) ) {
                        if (strncmp (buff, commands->command, strlen (commands->command) ) == 0)
                                return commands->value ;
                            else
                                commands++ ;

                        }
                    else
                        commands++ ;

        }

        return CO_ERROR ;

}



/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int ReadList (FILE *f, char *buff, char *iName,
              _list *list, int maxList, int equalsFlagNec) {

int len, count=0, value=0 ;
char *name ;
char section[36] ;

        strcpy (section, buff) ;

        len = GetLine (f, buff, iName) ;

        while (*buff != '[') {

                name = tok (buff, " \t=") ;

                if (equalsFlagNec) {

                        if (name[strlen(name)-1] == '*') {
                                name[strlen(name)-1] = '\0' ;
                                list->needEquals = FALSE ;

                                }

                            else if (name[strlen(name)-1] == '+') {
                                name[strlen(name)-1] = '\0' ;
                                list->needPathMatch = TRUE ;

                                }
                            else if (name[strlen(name)-1] == '#') {
                                name[strlen(name)-1] = '\0' ;

                                }
                            else
                                list->needEquals = TRUE ;

                        }
                    else
                        list->needEquals = FALSE ;

                list->name = AddBufferString (name) ;
                strlwr (list->name) ;
                list->value = value ;
                list++ ;

                assert (count++ < maxList, LOCATION,
                        "Adding too many specifiers to list type %s in file %s\r\n",
                        section, iName) ;

                while ( (name = tok (NULL, " \t,=") ) != NULL) {
                        strlwr (name) ;

                        if (strcmp (name, "syn") == 0) {
                                name = tok (NULL, " \t,=") ;

                                list->needEquals = (list-1)->needEquals ;
                                list->name = AddBufferString (name) ;
                                strlwr (list->name) ;
                                list->value = value ;
                                list++ ;

                                assert (count++ < maxList, LOCATION,
                                        "Adding too many specifiers to list type %s in file %s\r\n",
                                        section, iName) ;

                                }
                            else
                                assert (FALSE, LOCATION,
                                        "Improper synonym definition in file %s, line %d def %s\r\n",
                                        iName, _lineIn, (list-1)->name) ;


                }

                len = GetLine (f, buff, iName) ;
                value++ ;

        }


}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int MatchList (char *match, _list *list, int returnType) {

int count=0 ;

        strlwr (match) ;

        for (;list->name != NULL; list++, count++) {

                if ( *( (int *) match) == *( (int *) list->name) )
                        if (strcmp (match, list->name) == 0)
                                if (returnType == VALUE)
                                        return list->value ;
                                    else
                                        return count ;

        }

        return MA_ERROR ;

}



/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


void assert (int assertion, char *FileName, int Line,
	     char *fString, ...) {

va_list args ;


	if (! assertion) {
		args = va_start (args, fString) ;

                //fprintf (stderr, "File-> %s, Line-> %d\r\n", FileName, Line) ;
                //vfprintf (stderr, fString, args) ;

                //exit (0xff) ;

                fprintf (stdout, "File-> %s, Line-> %d\r\n", FileName, Line) ;
                vfprintf (stdout, fString, args) ;

                _errorOcurred = TRUE ;

	}

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

char _tokBuf[10240], *_tokPtr = _tokBuf ;

char *tok (char *str, char *search) {

static char *sStr ;
char *s, *s1 ;

        if (str != NULL)
                sStr = str ;


        if (*sStr == '\0')
                return NULL ;

        while (strchr (search, *sStr) != NULL && *sStr != '\0')
                sStr++ ;


        if (*sStr == '\0')
                return NULL ;

        s = sStr ;

        while (strchr (search, *sStr) == NULL && *sStr != '\0')
                sStr++ ;


        if (_tokPtr - _tokBuf + (sStr - s + 1) > sizeof (_tokBuf) )
                _tokPtr = _tokBuf ;


        strncpy (_tokPtr, s, sStr - s) ; // -1 ?

        _tokPtr[sStr - s] = '\0' ;

        s1 = _tokPtr ;

        _tokPtr += (sStr - s + 1) ;

        return s1 ;

}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

char *MakeBaseDir (char *specDir, char *baseDir) {

char curDrive[_MAX_DRIVE] ;
char baseDrive[_MAX_DRIVE], basePath[_MAX_PATH] ;

char curFullPath[65] ;



        if (*specDir == '\\') {
                strcpy (baseDir, specDir) ;

                }
            else {

                _splitpath (specDir, baseDrive, basePath, NULL, NULL) ;

                if (*baseDrive != '\0') { // must get current dir of another drive

                        _getdcwd (toupper (*baseDrive) - 'A' + 1, curFullPath, sizeof (curFullPath) ) ;

                        }
                    else {

                        _getcwd (curFullPath, sizeof (curFullPath) ) ;

                }


                if (*basePath != '\\') {

                        strcpy (baseDir, curFullPath) ;

                        if (baseDir[strlen(baseDir)-1] != '\\')
                                strcat (baseDir, "\\") ;

                        strcat (baseDir, specDir) ;

                        }
                    else {

                        _splitpath (curFullPath, curDrive, NULL, NULL, NULL) ;

                        strcpy (baseDir, curDrive) ;
                        strcat (baseDir, specDir) ;

                }

        }

        if (baseDir[strlen(baseDir)-1] != '\\')
                strcat (baseDir, "\\") ;

        strlwr (baseDir) ;


        return baseDir ;
}
