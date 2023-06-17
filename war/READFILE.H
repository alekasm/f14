/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          readfile.h

    Author:             Ned Way

    File Date:          11:16:22  Wed  28-Jul-1993

    File Description:
                        header file for readfile.c
                        F14 mission generator

    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

#define FALSE   0
#define TRUE    (! FALSE)


#define VALUE   0
#define INDEX   1

#define CO_END                 -1
#define CO_ERROR               -2
#define CO_NOT_COMMAND         -3
#define CO_LIST_END            -4


#define NO_EQUALS_NEC   0
#define EQUALS_NEC      1
#define MA_ERROR       -1

#define IBUFF_MAX       255
extern  char _iBuff[IBUFF_MAX] ;

#undef assert


typedef struct _commands_ {

        char    *command ;
        int     value ;

        } _commands ;



typedef struct _list_ {

        char    *name ;
        int     value ;

        union {
                int     needEquals ;    // for command lists
                int     side ;          // for object lists
        } ;

        int     needPathMatch ;

        } _list ;




extern int _lineIn, _errorOcurred ;


int  GetLine (FILE *f, char *buff, char *iName) ;

int  StripComments (char *buff, int len) ;
int  StripTrail (char *buff, int len) ;

char *AddBufferString (char *s) ;

int  ParseCommand (char *buff, _commands *commands) ;
int  ReadList (FILE *f, char *buff, char *iName,
              _list *list, int maxList, int equalsFlagNec) ;
int  MatchList (char *match, _list *list, int returnType) ;     // returnType is INDEX or VALUE

void assert (int assertion, char *FileName, int Line,
             char *fString, ...) ;

char *tok (char *, char *) ;


char *MakeBaseDir (char *specDir, char *baseDir) ;
