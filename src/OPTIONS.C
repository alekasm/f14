#include "library.h"
#include "f15defs.h"
#include "world.h"
#include "sdata.h"
#include "common.h"
#include "armt.h"
#include "awg9.h"
#include <dos.h>
#include <string.h>

UWORD   RealLandings;
UWORD   Joystick;
UWORD   ThrustStick;
UWORD   CoolyStick;
UWORD   RudderStick;
UWORD   StickType;
UWORD   TimeOfDay;

extern UBYTE JoyVals[];
extern UBYTE DeadZone;

extern  RPS     *Rp1;
extern  int     REALFLIGHT;
int     SKYDETAIL;

//***************************************************************************
InitOptions()
{
    Direction = 1;  /* Headed north */
	RealLandings = 1;
	REALFLIGHT = 1;
	seedme();
}

