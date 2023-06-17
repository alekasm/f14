#include <bios.h>
#include "library.h"
#include "f15defs.h"
#include "keycodes.h"

RPS RpJoyX  =  {0, 0,0, 319,199, JAM3, WHITE, LGREY, F_3x5B};
RPS *RpJoy  = &RpJoyX;

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


static  int     JoyX;
static  int     JoyY;
static  int     JoyW;
static  int     JoyZ;
static  int     JoyColor;
static  int     RudColor;
static  int     ThrstColor;

#define CoordX(a) (a/4)
#define CoordY(a) (a*3/16)

//***************************************************************************
JoyRecal (int x, int y)
{
    RpJoy->XBgn = x;
    RpJoy->YBgn = y;

    if (Joystick||RudderStick) {

        InitShowJoy ();

        JoyMessage (Joystick ? (RudderStick ? "Center Stick and Pedals"
                                            : "Center Joystick")
                             : "Center Pedals");
        do {
            WaitTicks(1);
            InitJoy2(Joystick|RudderStick); // purposely do not init the Cooly Hat or Throttle
            ShowJoy ();
        } while (!GetButtonDownUp());

        WaitTicks (10);

        if (Joystick) {
            JoyMessage ("Move Stick Upper Left");
            do {
                MinStick[0] = RawJoy[0];
                LowRange[0] = CenterPosition[0]-MinStick[0];

                MinStick[1] = RawJoy[1];
                LowRange[1] = CenterPosition[1]-MinStick[1];

                ShowJoy ();
            } while (!GetButtonDownUp());

            WaitTicks (10);

            JoyMessage ("Move Stick Lower Right");
            do {
                MaxStick[0]  = RawJoy[0];
                HighRange[0] = MaxStick[0]-CenterPosition[0];

                MaxStick[1]  = RawJoy[1];
                HighRange[1] = MaxStick[1]-CenterPosition[1];

                ShowJoy ();
            } while (!GetButtonDownUp());
        }

        if (RudderStick) {
            JoyMessage ("Move Pedals Left");
            do {
                MinStick[2] = RawJoy[2];
                LowRange[2] = CenterPosition[2]-MinStick[2];

                ShowJoy ();
            } while (!GetButtonDownUp());

            WaitTicks (10);

            JoyMessage ("Move Pedals Right");
            do {
                MaxStick[2]  = RawJoy[2];
                HighRange[2] = MaxStick[2]-CenterPosition[2];

                ShowJoy ();
            } while (!GetButtonDownUp());
        }

        if (ThrustStick) {
            JoyMessage ("Move Throttle Down");
            do {
                MaxStick[3]  = RawJoy[3];
                HighRange[3] = MaxStick[3]-CenterPosition[3];

                ShowJoy ();
            } while (!GetButtonDownUp());

            WaitTicks (10);

            JoyMessage ("Move Throttle Up");
            do {
                MinStick[3] = RawJoy[3];
                LowRange[3] = CenterPosition[3]-MinStick[3];

                ShowJoy ();
            } while (!GetButtonDownUp());
        }

        JoyMessage ("Adjust Dead Zone (+-)");
        do {
            ShowJoy ();
            if (kbhit()) {
                switch (_bios_keybrd (_KEYBRD_READ)) {
                    case PADPLUS:
                    case PLUS:
                    case INCPOWER:  if (DeadZone<63) DeadZone+=4;    break;

                    case PADMINUS:
                    case DECPOWER:  if (DeadZone>0)  DeadZone-=4;    break;
                }
                InitShowJoy ();
            }
        } while (!GetButtonDownUp());

    }
}

//***************************************************************************
TestJoy ()
{
    RpJoy->XBgn = 0;
    RpJoy->YBgn = 0;

    if (Joystick||RudderStick) {
        InitShowJoy ();
        do {
            ShowJoy ();
        } while (!kbhit());

        _bios_keybrd (_KEYBRD_READ);
    }
}

//***************************************************************************
static JoyMessage (char *str)
{
	JoyBox(0,57, 91,16);
	CenterPrint(RpJoy, 0,59, str, 91);
	CenterPrint(RpJoy, 0,66, "Press Fire Button", 91);
}

//***************************************************************************
static InitShowJoy ()
{
    struct STX j;

    JoyX = JoyY = JoyW = JoyZ = 0;
    JoyColor = RudColor = ThrstColor = DGREY;

    if (Joystick) {
        JoyBox   (0,0, 64,48);
        DrawLine (RpJoy, 32,0, 32,47, DGREY);
        DrawLine (RpJoy, 0,24, 63,24, DGREY);
        RectFill (RpJoy, 32-DeadZone/4,24-(DeadZone+2)*3/16, DeadZone/2+1, (DeadZone+2)*3/16*2+1, DGREY);
    }

    if (RudderStick) {
        JoyBox   (0,50, 64,5);
        RectFill (RpJoy, 32-DeadZone/4,51, DeadZone/4*2+1,3, DGREY);
    }

    if (ThrustStick) {
        JoyBox   (66,0, 5,48);
    }
}

//***************************************************************************
static ShowJoy()
{
    struct STX j;

    WaitTicks (1);
    j =ReadJoy2(Joystick|RudderStick|ThrustStick|CoolyStick);

    if (Joystick) {
        DrawDot (RpJoy, JoyX, JoyY, JoyColor);
        JoyX = (j.x+128)  /4;
        JoyY = (j.y+128)*3/16;
        JoyColor = GetRpPixel (RpJoy, JoyX, JoyY);
        DrawDot  (RpJoy, JoyX, JoyY, WHITE);
    }

    if (RudderStick) {
        DrawDot (RpJoy, JoyW, 52, RudColor);
        JoyW = (j.w+128) /4;
        RudColor = GetRpPixel (RpJoy, JoyW, 52);
        DrawDot  (RpJoy, JoyW, 52, WHITE);
    }

    if (ThrustStick) {
        DrawDot (RpJoy, 68, JoyZ, ThrstColor);
        JoyZ = j.z*3/8;
        ThrstColor = GetRpPixel (RpJoy, 68, JoyZ);
        DrawDot  (RpJoy, 68, JoyZ, WHITE);
    }
}

//***************************************************************************
static JoyBox(StartX, StartY, Width, Length)
{
    RectFill (RpJoy, StartX, StartY, Width, Length, LGREY);
    DrawLine (RpJoy, StartX,         StartY,          StartX+Width-1, StartY,          DGREY);
    DrawLine (RpJoy, StartX,         StartY+Length-1, StartX+Width-1, StartY+Length-1, DGREY);
    DrawLine (RpJoy, StartX,         StartY,          StartX,         StartY+Length-1, DGREY);
    DrawLine (RpJoy, StartX+Width-1, StartY,          StartX+Width-1, StartY+Length-1, DGREY);
}

//***************************************************************************
static GetButtonDownUp ()
{
    if (Trigger2(0)) {
        WaitTicks (10);     // debounce
        while (Trigger2(0));
        WaitTicks (10);     // debounce
        return (1);
    }
    return (0);
}

//***************************************************************************
CenterPrint (RPS *Rp, int x, int y, char *str, int width)
{
    x += (width -pstrlen(Rp, str) +1) /2;
    RpPrint (Rp, x,y, str);
}
