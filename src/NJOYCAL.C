#include <stdlib.h>
#include <stdio.h>
#include <bios.h>
#include "library.h"
#include "f15defs.h"
#include "keycodes.h"

RPS RpJoyX  =  {0, 0,0, 319,199, JAM3, WHITE, LGREY, F_3x5B};
RPS *RpJoy  = &RpJoyX;
RPS RpJoyBackX  =  {1, 0,0, 319,199, JAM3, WHITE, LGREY, F_3x5B};
RPS *RpJoyBack  = &RpJoyBackX;

extern  UWORD	Joystick;
extern  UWORD   ThrustStick;
extern  UWORD   CoolyStick;
extern  UWORD   RudderStick;
extern  UWORD   MinStick[];
extern  UWORD   MaxStick[];
extern  UWORD   CenterPosition[];
extern  UWORD   LowRange[];
extern  UWORD   HighRange[];
extern  UWORD   RawJoy[];
extern  UBYTE   DeadZone;
extern UBYTE JoyVals[];

extern  int     KbSense;

extern	int	    GRNDDETAIL;
extern	int	    PLANEDETAIL;
extern  int     SKYDETAIL;
extern	int 	MISSILETRAIL;
extern  int 	MSGDETAIL;
extern  int 	SNDDETAIL;
extern  int     StickType;
extern  int     RudderType;

#define CoordX(a) (a/4)
#define CoordY(a) (a*3/16)


#define  JBXL  0
#define  JBXH  117
#define  JBYL  70
#define  JBYH  199
#define  JBWID  ((JBXH-JBXL))
#define  JBHT  ((JBYH-JBYL))
#define  JBDSTXC  210
#define  JBDSTYC  84


#define  JCRSXL   121
#define  JCRSXH   124
#define  JCRSYL   70
#define  JCRSYH   74
#define  JCRSXC   123
#define  JCRSYC   72
#define  JCRSWID  6
#define  JCRSHT   6

#define  JHSLDXL   121
#define  JHSLDXH   124
#define  JHSLDYL   78
#define  JHSLDYH   85
#define  JHSLDXC   122
#define  JHSLDYC   81
#define  JHSLDWID  5
#define  JHSLDHT   8

#define  JVSLDXL   121
#define  JVSLDXH   127
#define  JVSLDYL   90
#define  JVSLDYH   93
#define  JVSLDXC   124
#define  JVSLDYC   91
#define  JVSLDWID  8
#define  JVSLDHT   4


#define  JBXDSTXL 158
#define  JBXDSTXH 275
#define  JBXDSTYL 20
#define  JBXDSTYH 149
#define  JCRSDSTXC 210
#define  JCRSDSTYC 84
#define  JTHRDSTXC 261
#define  JTHRDSTYC 88
#define  JRUDDSTXC 210
#define  JRUDDSTYC 136


extern   int   ScaleRotate(UWORD SOURCE, int XSC, int YSC, int WID, int HT, RPS *Rp, int XDC, int YDC, int SCALE, int ROLL, int TRNSCLR, int MIRROR);
extern   int   GRPAGE0[];




//                    KB,   1 stick,  2 stx, stk&throt, ThrstMast, CH Pro &  VPilot sticks

static ThrustVals[] = {0,      0,       8,       8,       0,       8,       8  };
static CoolyVals [] = {0,      0,       0,       0,       8,       0,       0  };
static RudderVals[] = {0,      0,       4,       0,       0,       0,       0  };

int   KeyJoyInstalled=0;
//***************************************************************************
SetJoysticks(int i)
{

   StickType = i;
   Joystick = i ? 2+1 : 0;
   ThrustStick = ThrustVals[i];
   CoolyStick = CoolyVals[i];
   RudderStick |= RudderVals[i];

   if (i)
      {
      if (KeyJoyInstalled)
         {
         RemoveKeyJoy();
         KeyJoyInstalled=0;
         }
      }
   else
      {
      if (KeyJoyInstalled==0)
         {
         InstallKeyJoy();
         KeyJoyInstalled=1;
         }
      }
}
GetRidOfKeyJoy()
{
   if (KeyJoyInstalled)
      {
      RemoveKeyJoy();
      KeyJoyInstalled=0;
      }
}


//***************************************************************************
JoyRecal (int x, int y)
{
	int   b;

	LoadPicNoPal (1, "joypic.pic");

	if (Joystick||RudderStick) {
		DeadZone=12;
		InitJoy2(Joystick|RudderStick); // purposely do not init the Cooly Hat or ThrottleInitJoy2(Joystick|RudderStick); // purposely do not init the Cooly Hat or Throttle
		if (NWaitShowJoy(Joystick ? (RudderStick ?
			"Center Stick and Pedals":
			"Center Joystick"):
			"Center Pedals",-1)==-1   )
			return;

		if (Joystick) {
			if (NWaitShowJoy("Move Stick Upper Left",5)==-1)
				return;
			if (NWaitShowJoy("Move Stick Lower Right",10)==-1)
				return;
		}

		if (RudderStick) {
			if (NWaitShowJoy("Move Pedals Left",16)==-1 )
				return;
			if (NWaitShowJoy("Move Pedals Right",32)==-1 )
				return;
		}

		if (ThrustStick) {
			if (NWaitShowJoy("Move Throttle Down",64)==-1 )
				return;
			if (NWaitShowJoy("Move Throttle Up",128)==-1 )
				return;
		}
		NWaitShowJoy("Adjust Dead Zone (+-)",0);
	}
}

//***************************************************************************
NShowJoy(char *msg,struct STX j)
{
   int   SEG;
   int   x,y,w,z,d,i;

   SEG= GRPAGE0[RpJoyBack->PageNr];

   x=j.x;
   y=j.y;
   w=j.w;
   z=j.z;

   x/= 4;
   y/= 4;
   w/= 4;
   z/= 4;

// fix for radial
   d= x*x+ y*y;
   for (i=1;i<65;i++) {
      if ((i*i)>d) break;
   }

   x= (x<0)? -((x*x)/i):((x*x)/i);
   y= (y<0)? -((y*y)/i):((y*y)/i);

// copy backgrnd to spare area
   RectCopy(RpJoyBack, JBXL, JBYL, JBXH-JBXL,JBYH-JBYL, RpJoyBack, JBXDSTXL, JBXDSTYL);

// set dead zone
   DrawLine(RpJoyBack, JCRSDSTXC-DeadZone/2, JCRSDSTYC-DeadZone/2, JCRSDSTXC+DeadZone/2, JCRSDSTYC-DeadZone/2,WHITE);
   DrawLine(RpJoyBack, JCRSDSTXC+DeadZone/2, JCRSDSTYC-DeadZone/2, JCRSDSTXC+DeadZone/2, JCRSDSTYC+DeadZone/2,WHITE);
   DrawLine(RpJoyBack, JCRSDSTXC+DeadZone/2, JCRSDSTYC+DeadZone/2, JCRSDSTXC-DeadZone/2, JCRSDSTYC+DeadZone/2,WHITE);
   DrawLine(RpJoyBack, JCRSDSTXC-DeadZone/2, JCRSDSTYC+DeadZone/2, JCRSDSTXC-DeadZone/2, JCRSDSTYC-DeadZone/2,WHITE);


// copy joy cursor
   ScaleRotate(SEG, JCRSXC, JCRSYC, JCRSWID, JCRSHT,RpJoyBack,JCRSDSTXC+x,JCRSDSTYC+y, 0x100, 0, 0, 0);

//   if (ThrustStick)
//      {
//   ScaleRotate( SEG, JSLDXC,JSLDYC, JSLDWID, JSLDHT, RpJoyBack, JTHRDSTXC, JTHRDSTYC-w, 0x100, 0, -1, 0);
   RectCopy (RpJoyBack, JVSLDXL, JVSLDYL, JVSLDWID,JVSLDHT, RpJoyBack, JTHRDSTXC-(JVSLDWID/2), JTHRDSTYC-z-(JVSLDHT/2));
//      }

//   if (RudderStick)
//      {
//   ScaleRotate( SEG, JSLDXC,JSLDYC, JSLDWID, JSLDHT,RpJoyBack, JRUDDSTXC+z,JRUDDSTYC, 0x100, 0, -1, 0);
   RectCopy (RpJoyBack, JHSLDXL, JHSLDYL, JHSLDWID,JHSLDHT, RpJoyBack, JRUDDSTXC+w-(JHSLDWID/2), JRUDDSTYC-(JHSLDHT/2));
//      }

   RpJoyBack->APen= BLACK;
   CenterPrint(RpJoyBack, JBDSTXC+1, JBXDSTYL+6+1, msg);
   CenterPrint(RpJoyBack, JBDSTXC+1, JBXDSTYL+12+1, "Then press Trigger");
   RpJoyBack->APen= 0x33;
   CenterPrint(RpJoyBack, JBDSTXC, JBXDSTYL+6, msg);
   CenterPrint(RpJoyBack, JBDSTXC, JBXDSTYL+12, "Then press Trigger");

   RectCopy(RpJoyBack, JBXDSTXL, JBXDSTYL, JBXDSTXH-JBXDSTXL,JBXDSTYH-JBXDSTYL, RpJoy, JBXDSTXL, JBXDSTYL);
}

//***************************************************************************
NWaitShowJoy(char *msg, int vals)
{
   int   i,v;
   struct STX j;

   while (Trigger2(0)==0)
      {
      if (kbhit())
         {
         switch (_bios_keybrd (_KEYBRD_READ))
            {
            case PADPLUS:
            case PLUS:
            case INCPOWER:
               if (DeadZone<63) DeadZone+=4;
               break;

            case PADMINUS:
            case DECPOWER:
               if (DeadZone>7)  DeadZone-=4;
               break;
            case ESCKEY:
               return(-1);
            case ENTER:
               return(0);
            }
         }

      WaitTicks (1);
      j =ReadJoy2(Joystick|RudderStick|ThrustStick|CoolyStick);

      if (vals==-1)
         {
         for (i=0;i<4;i++)
            {
            MinStick[i] = RawJoy[i];
            LowRange[i] = CenterPosition[i]-MinStick[i];
            MaxStick[i]  = RawJoy[i];
            HighRange[i] = MaxStick[i]-CenterPosition[i];
            }
         j.x=j.y=j.w=j.z=0;
         }
      else
         {
         for (i=0,v=vals;i<4;i++,(v=(v>>2)))
            {
            if (v&1)
               {
               MinStick[i] = RawJoy[i];
               LowRange[i] = CenterPosition[i]-MinStick[i];
               }
            if (v&2)
               {
               MaxStick[i]  = RawJoy[i];
               HighRange[i] = MaxStick[i]-CenterPosition[i];
               }
            }
         }

      WaitTicks(10);
      NShowJoy(msg,j);
      }

   if (vals==-1)
      InitJoy2(Joystick|RudderStick); // purposely do not init the Cooly Hat or Throttle


   while (Trigger2(0))
      {
      if (kbhit())
         {
         switch (_bios_keybrd (_KEYBRD_READ))
            {
            case ESCKEY:
               return(-1);
            }
         }
      WaitTicks(10);
      NShowJoy(msg,j);
      }
   return(0);
}
//**********************************************************************
RedrawJoyBox()
{
	RectCopy(RpJoyBack, JBXDSTXL, JBXDSTYL, JBXDSTXH-JBXDSTXL,JBXDSTYH-JBXDSTYL, RpJoy, JBXDSTXL, JBXDSTYL);

}

//***************************************************************************
CenterPrint (RPS *Rp, int x, int y, char *str )
{
   x -= ((pstrlen(Rp, str)+1)/2);
    RpPrint (Rp, x,y, str);
}

//***************************************************************************
int GetJoyConfig()
{

	FILE *f ;
	int offset;

	f = fopen ("JOY.F14", "rb") ;

    fread(&offset,			sizeof (int), 1, f);
    fread(&KbSense,			sizeof (int), 1, f);
	if (KbSense<0 || KbSense>2) KbSense=0;
    fread(&SNDDETAIL,		sizeof (int), 1, f);
	if (SNDDETAIL<0 || SNDDETAIL>3) SNDDETAIL=0;
    fread(&MSGDETAIL,		sizeof (int), 1, f);
	if (MSGDETAIL<0 || MSGDETAIL>3) MSGDETAIL=0;
    fread(&SKYDETAIL,		sizeof (int), 1, f);
	if (SKYDETAIL<0 || SKYDETAIL>3) SKYDETAIL=0;
    fread(&GRNDDETAIL,		sizeof (int), 1, f);
	if (GRNDDETAIL<0 || GRNDDETAIL>3) GRNDDETAIL=0;
    fread(&PLANEDETAIL,		sizeof (int), 1, f);
	if (PLANEDETAIL<0 || PLANEDETAIL>3) PLANEDETAIL=3;
    fread(&MISSILETRAIL,	sizeof (int), 1, f);
	if (MISSILETRAIL<0 || MISSILETRAIL>2) MISSILETRAIL=0;

	fread(&StickType, 		sizeof(int),1,f);
	fread(&RudderType,		sizeof(int),1,f);
    fread(&Joystick,    	sizeof (unsigned), 1, f) ; // 3 when enabled
    fread(&ThrustStick, 	sizeof (unsigned), 1, f) ; // 8 when enabled
    fread(&CoolyStick,  	sizeof (unsigned), 1, f) ; // 8 when enabled
    fread(&RudderStick, 	sizeof (unsigned), 1, f) ; // 4 when enabled

    fread(&JoyVals[0], sizeof (unsigned char), 52, f) ;  // 56 bytes long

    fread(&DeadZone, sizeof (char), 1, f) ;

	fclose(f) ;
}

//***************************************************************************
int SaveJoyConfig()
{
	FILE *f ;
	int offset=16;

    f = fopen ("JOY.f14", "wb");

    fwrite(&offset,			sizeof (int), 1, f);
    fwrite(&KbSense,		sizeof (int), 1, f);
    fwrite(&SNDDETAIL,		sizeof (int), 1, f);
    fwrite(&MSGDETAIL,		sizeof (int), 1, f);
    fwrite(&SKYDETAIL,		sizeof (int), 1, f);
    fwrite(&GRNDDETAIL,		sizeof (int), 1, f);
    fwrite(&PLANEDETAIL,	sizeof (int), 1, f);
    fwrite(&MISSILETRAIL,	sizeof (int), 1, f);

	fwrite(&StickType, 		sizeof (int),1,f);
	fwrite(&RudderType,		sizeof (int),1,f);

    fwrite (&Joystick,		sizeof (unsigned), 1, f) ; // 3 when enabled
    fwrite (&ThrustStick,	sizeof (unsigned), 1, f) ; // 8 when enabled
    fwrite (&CoolyStick,	sizeof (unsigned), 1, f) ; // 8 when enabled
    fwrite (&RudderStick,	sizeof (unsigned), 1, f) ; // 4 when enabled
    fwrite (&JoyVals[0],	sizeof (unsigned char), 52, f) ;  // 56 bytes long
    fwrite (&DeadZone,		sizeof (char), 1, f) ;
	fclose (f) ;
}

