/************************************************************************
*
*    MENUS.C
*
*    In flight menu routines
*
*    Author: W. Becker
*
*    F-15 III
*    Microprose Software, Inc.
*    180 Lakefront Drive
*    Hunt Valley, Maryland  21030
*
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <bios.h>
#include <dos.h>
#include "types.h"
#include "library.h"
#include "keycodes.h"
#include "world.h"
#include "sdata.h"
#include "sounds.h"
#include "common.h"
#include "f15defs.h"

//************************************************************************

#define  MAXHS          20
#define  HS_ENABLED     1

#define  MTYPE_SELECT   0
#define  MTYPE_SUBMEN   1
#define  MTYPE_TOGGLE   2

#define  CLR_TEXT       WHITE

//************************************************************************

extern  RPS     *Rp1;
extern  RPS     *Rp2;
extern  char    SHOW_;
extern  int     GRPAGE0[];
extern  int     MenuSpr1;
extern  int     MenuSpr2[];
extern  int     MenuSpr3[];
extern  UBYTE   ExitCode;
extern  int     KbSense;
extern  int     SndPriority;
extern  UWORD   Joystick;
extern  UWORD   ThrustStick;
extern  UWORD   CoolyStick;
extern  UWORD   RudderStick;
extern  UWORD   MouseSprite;

extern	int	    MTNDETAIL;
extern	int	    GRNDDETAIL;
extern	int	    OBJECTDETAIL;
extern	int	    PLANEDETAIL;
extern	int	    XPLODETAIL;
extern  int     SKYDETAIL;
extern	int	    FASTROLL;
extern  int     StickType;
extern  int     Fencer;
extern	int		TVcolor;

extern  UWORD   MinStick[];
extern  UWORD   MaxStick[];
extern  UWORD   CenterPosition[];
extern  UWORD   LowRange[];
extern  UWORD   HighRange[];

//************************************************************************

static exit_to_dos   ();
static resume_game   ();
static end_mission   ();
static abort_mission ();

static set_sound     ();
static ped_on        ();
static joy_on        ();
static joy_off       ();
static joy_recal     ();
static joy_recal2    ();
static set_kb_sens   ();
static set_rollrate  ();

static set_mtn_detail  ();
static set_plane_detail();
static set_grnd_detail ();
static set_obj_detail  ();
static set_xplo_detail ();
static set_conditions  ();

//************************************************************************

typedef  struct {                      // MENU ENTRY
	int   x,y;                          // x,y offset of text
	char  *text;                        // entry text string
	char  type;                         // entry type
	union {
		int  (*func)();
		void  *data;
	} action;                           // pointer based on type
	int   altkey;                       // alternate key code
} ENTRY;

typedef  struct {                      // MENU
	int   x, y;                         // upper left corner of menu
	int   n;                            // number of entries
	ENTRY *entry;                       // pointer to menu entries
} MENU;

typedef  struct {                      // HOT SPOT
	int      xmin, ymin;                // x,y min
	int      xmax, ymax;                // x,y max
	char     status;                    // hot spot status
	MENU    *menu;                      // menu pointer
	int      entry;                     // entry number
} HS;

typedef  struct {                      // HOT SPOT CONTROL BLOCK
	int   max;                          // max size of stack
	int   n;                            // stack size
	int   cur;                          // current hot spot
	HS   *st;                           // stack
} HS_CB;

//************************************************************************

static int     MenuSpr1W;
static int     MenuSpr1H;
static int     MenuSpr2W;
static int     MenuSpr2H;
static int     MenuSpr3W;
static int     MenuSpr3H;

//************************************************************************

static RPS     MenuRX         = { 0, 0, 0, 319, 199, JAM1, WHITE, LGRAY, F_3x5B };
static RPS    *MenuRp         = &MenuRX;

static HS      hs_st[MAXHS];
static HS_CB   hs_cb          = { MAXHS, 0, -1, hs_st };

//************************************************************************

static char    P00[] = "\x0C6G\x08f\AME";
static char    P01[] = "\x0C6\C\x08fONTROL";
static char    P02[] = "O\x0C6P\x08fTIONS";

static char    P10[] = "\x0C6R\x08f\ESUME GAME";
static char    P12[] = "\x0C6\A\x08f\BORT MISSION";
static char    P13[] = "E\x0C6\X\x08fIT TO DOS";
static char    P14[] = "\x0C6\E\x08fND MISSION";

static char    P21[] = "\x0C6J\x08fOYSTICK";
static char    P22[] = "\x0C6K\x08f\EYBOARD SENSITIVITY";

static char    P36[] = "\x0C6N\x08FO JOYSTICK";
static char    P30[] = "\x0C6\O\x08fNE JOYSTICK";
static char    P31[] = "JOYSTICK \x0C6W\x08f/ THROTTLE";
static char    P32[] = "\x0C6T\x08fWO JOYSTICKS";
static char    P33[] = "THRUST\x0C6\M\x08f\ASTER";
static char    P34[] = "\x0C6\V\x08fIRT PILT, FLTSTK PRO"; /* CH PROD WITH COOLY */
static char    P35[] = "\x0C6\F\x08fOOT PEDALS";

static char    P50[] = "\x0C6H\x08fIGH";
static char    P51[] = "M\x0C6\E\x08f\DIUM";
static char    P52[] = "\x0C6L\x08fOW";

static char    P60[] = "\x0C6\E\x08fNABLE";
static char    P61[] = "\x0C6\D\x08fISABLE";
static char    P62[] = "\x0C6R\x08f\ECALIBRATE";

static char    P73[] = "\x0C6S\x08fOUND";
static char    P100[] = "\x0C6\M\x08f\OUNTAIN DETAIL";
static char    P101[] = "\x0C6\F\x08f\IGHTER DETAIL";
static char    P102[] = "O\x0C6\B\x08f\JECT DETAIL";
static char    P103[] = "G\x0C6\R\x08f\OUND DETAIL";
static char    P104[] = "E\x0C6\X\x08f\PLOSION DETAIL";
static char    P105[] = "S\x0C6\K\x08f\Y DETAIL";

static char    P106[] = "C\x0C6L\x08f\EAR";
static char    P107[] = "\x0C6\H\x08f\IGH CLOUDS";
static char    P108[] = "\x0C6O\x08f\VERCAST";

static char    P80[] = "\x0C6M\x08f\AXIMUM";
static char    P81[] = "\x0C6H\x08fIGH";
static char    P82[] = "M\x0C6\E\x08f\DIUM";
static char    P83[] = "\x0C6L\x08fOW";

static char    P90[] = "A\x0C6L\x08fL ON";
static char    P91[] = "\x0C6N\x08fO ENGINE";
static char    P92[] = "SP\x0C6\E\x08f\ECH & WARNINGS OFF";
static char    P93[] = "ALL \x0C6O\x08f\FF";

static char    P110[] = "ROLL R\x0C6\A\x08f\TE";
static char    P111[] = "\x0C6S\x08f\TANDARD";
static char    P112[] = "\x0C6\A\x08f\UTHENTIC";


//***************************************************************************
static ENTRY   SoundEntry[] = {
   { 7, 4, P90, MTYPE_TOGGLE, set_sound, LKEY },
   { 7, 4, P91, MTYPE_TOGGLE, set_sound, NKEY },
   { 7, 4, P92, MTYPE_TOGGLE, set_sound, EKEY },
   { 7, 4, P93, MTYPE_TOGGLE, set_sound, OKEY },
};
static MENU    SoundMenu = { 209, 11, 4, SoundEntry };

static ENTRY   MtnDetailEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_mtn_detail, HKEY },
   { 7, 4, P51, MTYPE_TOGGLE, set_mtn_detail, EKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_mtn_detail, LKEY },
};
static MENU    MtnDetailMenu = { 209, 24, 3, MtnDetailEntry };

static ENTRY   PlaneDetailEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_plane_detail, HKEY },
   { 7, 4, P51, MTYPE_TOGGLE, set_plane_detail, EKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_plane_detail, LKEY },
};
static MENU    PlaneDetailMenu = { 209, 37, 3, PlaneDetailEntry };

static ENTRY   ObjDetailEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_obj_detail, HKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_obj_detail, LKEY },
};
static MENU    ObjDetailMenu = { 209, 50, 2, ObjDetailEntry };


static ENTRY   GrndDetailEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_grnd_detail, HKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_grnd_detail, LKEY },
};
static MENU    GrndDetailMenu = { 209, 63, 2, GrndDetailEntry };

static ENTRY   XploDetailEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_xplo_detail, HKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_xplo_detail, LKEY },
};
static MENU    XploDetailMenu = { 209, 76, 2, XploDetailEntry };


static ENTRY   ConditionsEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_conditions, HKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_conditions, LKEY },
};
static MENU    ConditionsMenu = { 209, 89, 2, ConditionsEntry };

static ENTRY   JoyEntry[] = {
   { 7, 4, P36, MTYPE_TOGGLE, joy_on, NKEY },
   { 7, 4, P30, MTYPE_TOGGLE, joy_on, OKEY },
   { 7, 4, P31, MTYPE_TOGGLE, joy_on, WKEY },
   { 7, 4, P32, MTYPE_TOGGLE, joy_on, TKEY },
   { 7, 4, P33, MTYPE_TOGGLE, joy_on, MKEY },
   { 7, 4, P34, MTYPE_TOGGLE, joy_on, VKEY },
   { 7, 4, P35, MTYPE_TOGGLE, ped_on, FKEY },
   { 7, 4, P62, MTYPE_SELECT, joy_recal2, RKEY },
};
static MENU    JoyMenu = { 153, 11, 8, JoyEntry };

static ENTRY   KbEntry[] = {
   { 7, 4, P50, MTYPE_TOGGLE, set_kb_sens, HKEY },
   { 7, 4, P51, MTYPE_TOGGLE, set_kb_sens, EKEY },
   { 7, 4, P52, MTYPE_TOGGLE, set_kb_sens, LKEY },
};
static MENU    KbMenu = { 153, 24, 3, KbEntry };


static ENTRY   RollEntry[] = {
   { 7, 4, P111, MTYPE_TOGGLE, set_rollrate, SKEY },
   { 7, 4, P112, MTYPE_TOGGLE, set_rollrate, AKEY },
};
static MENU    RollMenu = { 153, 37, 2, RollEntry };

static ENTRY   OptnEntry[] = {
   { 7, 4, P73,  MTYPE_SUBMEN, &SoundMenu,       SKEY },
   { 7, 4, P100, MTYPE_SUBMEN, &MtnDetailMenu,   MKEY },
   { 7, 4, P101, MTYPE_SUBMEN, &PlaneDetailMenu, FKEY },
   { 7, 4, P102, MTYPE_SUBMEN, &ObjDetailMenu,   BKEY },
   { 7, 4, P103, MTYPE_SUBMEN, &GrndDetailMenu,  RKEY },
   { 7, 4, P104, MTYPE_SUBMEN, &XploDetailMenu,  XKEY },
   { 7, 4, P105, MTYPE_SUBMEN, &ConditionsMenu,  KKEY },
};
static MENU    OptnMenu = { 132, 11, 7, OptnEntry };

static ENTRY   FileEntry[] = {
   { 7, 4, P10, MTYPE_SELECT,   resume_game, RKEY },
   { 7, 4, P12, MTYPE_SELECT, abort_mission, AKEY },
   { 7, 4, P13, MTYPE_SELECT,   exit_to_dos, XKEY },
};
static MENU    FileMenu = {  0, 11, 3, FileEntry };

static ENTRY   FileEntry2[] = {
   { 7, 4, P10, MTYPE_SELECT,   resume_game, RKEY },
   { 7, 4, P14, MTYPE_SELECT,   end_mission, EKEY },
   { 7, 4, P12, MTYPE_SELECT, abort_mission, AKEY },
   { 7, 4, P13, MTYPE_SELECT,   exit_to_dos, XKEY },
};
static MENU    FileMenu2 = {  0, 11, 4, FileEntry2 };

static ENTRY   CtrlEntry[] = {
   { 7, 4, P21,  MTYPE_SUBMEN,  &JoyMenu,  JKEY },
   { 7, 4, P22,  MTYPE_SUBMEN,  &KbMenu,   KKEY },
   { 7, 4, P110, MTYPE_SUBMEN,  &RollMenu, AKEY },
};
static MENU    CtrlMenu = { 63, 11, 3, CtrlEntry };

static ENTRY   BarEntry[] = {
   {   7, 3, P00, MTYPE_SUBMEN, &FileMenu, GKEY },
   {  70, 3, P01, MTYPE_SUBMEN, &CtrlMenu, CKEY },
   { 139, 3, P02, MTYPE_SUBMEN, &OptnMenu, PKEY },
};
static MENU    Bar = { 0, 0, 3, BarEntry };

static int        R_DISABLE=0;
static int        MOSspr   = 0;
static int        MOShotx;
static int        MOShoty;
static int        kb_sens[3] = { 0, 0, 1 };
static int        MosON      = 1;

//***************************************************************************
MainMenu()
{
   if (Fencer==2) BarEntry[0].action.func = &FileMenu2;
   SndSounds(N_Pause);
   bar_menu(&Bar);
   SndSounds(N_UnPause);
}

//***************************************************************************
static bar_menu(MENU *m)
{
	int lopt, fopt, last;
	int key, s;
	int done    = 0;
	int latched = 0;

	// change mouse pointer

	if (MOSspr != MouseSprite)
		MouseSETUP (MouseSprite,0,0);

	MouseHIDE();
	RectCopy(Rp1, 0,0, 320,200, Rp2, 0,0);

	menu_setup();
	bar_show(m);

	fopt = hs_find(m,0,HS_ENABLED);
	lopt = hs_find(m,m->n-1,HS_ENABLED);

	last = hs_cb.cur = -1;

	while (!done) {
		if (last != hs_cb.cur) {
			MouseHIDE ();

			if (last != -1)
            	ChangeColor(MenuRp,
					hs_cb.st[last].xmin,
					hs_cb.st[last].ymin,
					hs_cb.st[last].xmax - hs_cb.st[last].xmin + 1,
					hs_cb.st[last].ymax - hs_cb.st[last].ymin + 1,
					234,
					LGRAY);

			last = hs_cb.cur;

			if (last != -1)
				ChangeColor (MenuRp,
					hs_cb.st[last].xmin,
					hs_cb.st[last].ymin,
					hs_cb.st[last].xmax - hs_cb.st[last].xmin + 1,
					hs_cb.st[last].ymax - hs_cb.st[last].ymin + 1,
					LGRAY,
					234);

			MouseSHOW();
		}

		// run pull down menus

		key = menu_activate(m);

		// check inputs

		if (kbhit() || key) {
			// read keyboard

			if (kbhit()) key = _bios_keybrd (_KEYBRD_READ);

			switch (key) {
				case ESCKEY:
					done++;
				break;

				case LEFTARROW:
					if (hs_cb.cur <= fopt) hs_cb.cur = lopt + 1;
					hs_cb.cur--;
				break;

				case RIGHTARROW:
					if (++hs_cb.cur > lopt) hs_cb.cur = fopt;
				break;

				default:
					hs_cb.cur = hs_find_altkey (key, HS_ENABLED);
				break;
			}
		} else {
			// read mouse inputs
			if (Button) {
				latched = Button;
				hs_setcur (MouseX, MouseY);
			} else if (latched) {
				latched = 0;
				if (hs_setcur (MouseX, MouseY) == -1) {
					done++;
				}
			}
		}
	}

	bar_hide (m);

	if (s = CursorInCrt())
		MouseSETUP (s,5,5);

	ShowPage(1);
	MouseHITS();  // clear mouse hits
}

//***************************************************************************
static bar_show(MENU *m)
{
	int i, x, y;
	HS hs;
	ENTRY *e;

   MouseHIDE  ();
   ShowSprite (MenuRp, m->x, m->y, MenuSpr1);
   MouseSHOW  ();

   for (i = 0; i < m->n; i++) {

      e = m->entry + i;
      x = m->x     + e->x;
      y = m->y     + e->y;

      // draw text on bar


      SetAPen   (MenuRp, CLR_TEXT);
      MouseHIDE ();
      RpPrint   (MenuRp, x, y, e->text);
      MouseSHOW ();

      // setup and push hot spot

      hs.xmin   = x - 5;
      hs.xmax   = (x + pstrlen (MenuRp, e->text)) + 4;
      hs.ymin   = m->y;
      hs.ymax   = (m->y + MenuSpr1H) - 1;
      hs.status = HS_ENABLED;
      hs.menu   = m;
      hs.entry  = i;

      hs_push (&hs);

   }

}

//***************************************************************************
static bar_hide(MENU *m)
{
	int   i;

	// pop hot spots
	for (i = 0; i < m->n; i++)
		hs_pop ();
}

//***************************************************************************
static menu_setup()
{
	static int  done=0;
	int far *s;

	if (done) return 0;
	done++;

	FP_OFF(s) = 0;
	FP_SEG(s) = MenuSpr1;
	MenuSpr1W = *s;
	MenuSpr1H = *(s+1);

	FP_SEG(s) = MenuSpr2[0];
	MenuSpr2W = *s;
	MenuSpr2H = *(s+1);

	FP_SEG(s) = MenuSpr3[0];
	MenuSpr3W = *s;
	MenuSpr3H = *(s+1);
}

//***************************************************************************
menu(MENU *m)
{
	int   last, lopt, fopt, cur;
	int   active  = 0;
	int   key     = 0;
	int   latched = 0;
	int   done    = 0;

	menu_show(m, 1);
	entry_draw (m, 0, 1, 0);

	fopt      =
	last      =
	hs_cb.cur = hs_find(m,      0, HS_ENABLED);
	lopt      = hs_find(m, m->n-1, HS_ENABLED);

	while (!done) {
      key = 0;

      // run menu

      if (hs_cb.st[hs_cb.cur].menu != m)

         done++;

      else if (hs_cb.cur != last) {

         entry_draw (m, hs_cb.st[     last].entry, 0, 0);   // restor old
         entry_draw (m, hs_cb.st[hs_cb.cur].entry, 1, 0);   // hilite new

      }

      if (active && !done) {

         R_DISABLE++;
//         if ((key = menu_activate (m)) == LEFTARROW) key=0;
         key = menu_activate (m);
         R_DISABLE--;
         menu_show  (m, 0);
         entry_draw (m, hs_cb.st[hs_cb.cur].entry, 1, 0);
         active=0;

      }

      last = hs_cb.cur;

      // check inputs

      if (kbhit() || key) {

         // read keyboard

         if (kbhit()) key = _bios_keybrd (_KEYBRD_READ);

         switch (key) {
         case ESCKEY:
         case LEFTARROW:
            if (R_DISABLE) key = 0;
            done++;
            break;

         case RIGHTARROW:

            if (R_DISABLE) break;

            done++;
            break;

         case ENTER:
            active++;
            break;

         case UPARROW:
            if (hs_cb.cur <= fopt) hs_cb.cur = lopt + 1;
            hs_cb.cur--;
            break;

         case DOWNARROW:
            if (++hs_cb.cur > lopt) hs_cb.cur = fopt;
            break;

         default:
            if ((hs_cb.cur = hs_find_altkey (key,HS_ENABLED)) == -1)
               hs_cb.cur = last;
            else {
               if (hs_cb.st[hs_cb.cur].menu != m) done++;
               active++;
            }
            break;
         }

      } else {

         // read mouse inputs

         if (Button) {
            latched = Button;
            if (hs_setcur (MouseX, MouseY) == -1) {
               hs_cb.cur = last;
            } else if (m->entry[hs_cb.st[hs_cb.cur].entry].type == MTYPE_SUBMEN) {
                active++;
                latched=0;
            }

         } else if (latched) {
            latched = 0;
            if (hs_setcur (MouseX, MouseY) == -1) {
                hs_cb.cur = last;
                done++;
                key = ESCKEY;
            } else {
                active++;
            }
         }

      }

   }

   menu_hide (m);

   return key;

}

//***************************************************************************
static menu_show(MENU *m, int hsflag)
{
	int   i;

	draw_dropshadow(m);

	for (i = 0; i < m->n; i++)
		entry_draw (m, i, 0, hsflag);
}

//***************************************************************************
static menu_hide(MENU *m)
{
	int   w, h, i;

	// erase menu

   w = MenuSpr2W + MenuSpr3W + 5;
   h = MenuSpr2H * m->n + 5;

   MouseHIDE ();
   RectCopy (Rp2, m->x, m->y, w, h, MenuRp, m->x, m->y);
   MouseSHOW ();

   // pop hot spots

   for (i = 0; i < m->n; i++)
      hs_pop ();

}

//***************************************************************************
static menu_activate(MENU *m)
{
	int      key = 0;
	int      cur;
	ENTRY   *e;


	if (hs_cb.cur != -1) {
		e = m->entry + hs_cb.st[hs_cb.cur].entry;

      switch (e->type) {

      case MTYPE_SELECT:

         e->action.func ();
         key = ESCKEY;
         break;

      case MTYPE_SUBMEN:

         cur = hs_cb.cur;
         hs_disable (cur);
         key = menu (e->action.data);
         hs_enable  (cur);
         hs_cb.cur = cur;
         break;

      case MTYPE_TOGGLE:

         e->action.func (1,hs_cb.st[hs_cb.cur].entry);
         key = LEFTARROW;
         break;

      default:
         break;

      }
   }

   return key;
}

//***************************************************************************
static entry_draw(MENU *m, int i, int hiflg, int hsflg)
{
	int      x, y;
	HS       hs;
	ENTRY   *e;

	// draw menu entry

   e = m->entry + i;
   x = m->x;
   y = m->y + (i * MenuSpr2H);

   SetAPen    (MenuRp, CLR_TEXT);
   MouseHIDE  ();
   ShowSprite (MenuRp,             x,        y, MenuSpr2[hiflg]);
   ShowSprite (MenuRp, x + MenuSpr2W,        y, MenuSpr3[e->type]);
   RpPrint    (MenuRp,      e->x + x, e->y + y, e->text);
   MouseSHOW  ();

   if (e->type == MTYPE_TOGGLE && e->action.func (0, i)) {
         MouseHIDE  ();
         ShowSprite (MenuRp, x + MenuSpr2W, y, MenuSpr3[3]);
         MouseSHOW  ();
      }

   if (!hsflg) return 0;

   // setup and push hot spot

   hs.xmin   = x;
   hs.ymin   = y;
   hs.xmax   = (x + MenuSpr2W + MenuSpr3W) - 1;
   hs.ymax   = (y + MenuSpr2H) - 1;
   hs.status = HS_ENABLED;
   hs.menu   = m;
   hs.entry  = i;

   hs_push (&hs);

}

//***************************************************************************
static draw_dropshadow(MENU *m)
{

	MouseHIDE();
	RectFill(MenuRp,
              m->x+3,
              m->y+3,
              MenuSpr2W + MenuSpr2H,
              MenuSpr2H * m->n,
              BLACK);

	MouseSHOW();
}

//***************************************************************************
MouseSETUP(int MouseSeg, int MouseHotX, int MouseHotY)
{
   if (MouseSeg == MOSspr) return 0;

   MOShotx = MouseHotX;
   MOShoty = MouseHotY;
   MOSspr  = MouseSeg;

   MouseTYPE (MOShotx, MOShoty, MOSspr);

}

#define in_hot_spot(n,x,y) ((x >= (hs)->xmin) && (y >= (hs)->ymin) && \
									 (x <= (hs)->xmax) && (y <= (hs)->ymax))

//***************************************************************************
static hs_push(HS *hs)
{
	HS *next;

	if (hs_cb.n == hs_cb.max) return 0;

	next = hs_cb.st + hs_cb.n;
	hs_cb.n++;

	*next = *hs;
}

//***************************************************************************
static hs_pop()
{
	if (hs_cb.n) hs_cb.n--;

	if (hs_cb.cur == hs_cb.n)
      hs_cb.cur = -1;
}

//***************************************************************************
static hs_disable_all()
{
	int i;

	for (i = 0; i < hs_cb.n; i++)
		hs_disable (i);

}

//***************************************************************************
static hs_disable(int n)
{

	if (hs_cb.n == n) return 0;

 	hs_cb.st[n].status &= ~HS_ENABLED;

	if (hs_cb.cur == n)
      hs_cb.cur = -1;

}

//***************************************************************************
static hs_enable_all()
{
	int i;

	for (i = 0; i < hs_cb.n; i++)
	   hs_enable (i);

}

//***************************************************************************
static hs_enable (int n)
{
	if (hs_cb.n == n) return 0;
	hs_cb.st[n].status |= HS_ENABLED;
}

//***************************************************************************
static hs_setcur (int x, int y)
{
	int   i;
	HS    *hs;

	hs = hs_cb.st + hs_cb.cur;

	if (in_hot_spot (hs, x, y)) goto done;

	for (i = hs_cb.n - 1; i >= 0; i--) {

		hs = hs_cb.st + i;

		if (in_hot_spot(hs, x, y) && (hs->status & HS_ENABLED)) {

			hs_cb.cur = i;
         goto done;

		}
	}

	hs_cb.cur = -1;

done:

   return hs_cb.cur;
}

//***************************************************************************
static hs_find(MENU *menu, int entry, int status)
{
	int   i;

	for (i = 0; i < hs_cb.n; i++)
		if ((hs_cb.st[i].status & status) &&
			 (hs_cb.st[i].menu  == menu  ) &&
			 (hs_cb.st[i].entry == entry ))
			return i;

	return -1;
}

//***************************************************************************
static hs_find_altkey(int key, int status)
{
	int   i;

	for (i = 0; i < hs_cb.n; i++)
	   if ((hs_cb.st[i].status & status) &&
			 (hs_cb.st[i].menu->entry[hs_cb.st[i].entry].altkey == key))
         return i;
	return -1;
}

//***************************************************************************
pstrlen (RPS *rp, char *s)
{
	char  *t = s;
	int   fn, n;

   fn = rp->FontNr;
   n = 0;

   while (*t != 0) n += CharWidth (fn, *t++);
   if (LibCanDo (TextIs16Bit))  n /= 2;

   return n;

}

//***************************************************************************
static exit_to_dos()
{
	EndGame (ENDOFLOG);
	ExitCode = C_QUIT;
}

//***************************************************************************
static resume_game()
{
}

//***************************************************************************
static abort_mission()
{
	EndGame(ENDOFLOG);
	ExitCode = C_ALTERNATE;
}

//***************************************************************************
static end_mission()
{
   EndGame(ENDMISSION);
}

//***************************************************************************
static set_sound(int tog, int i)
{
	if (tog) {
		SndPriority = i;
		UpdateJetSnd ();
   }
   return SndPriority == i;
}

//***************************************************************************
static ped_on(int tog, int i)
{
   if (tog) {
      if (RudderStick && StickType!=J_TWOSTICKS) {
         RudderStick = 0;
      } else {
         RudderStick = 4;
         joy_recal ();
      }

   }
   return (RudderStick && StickType!=J_TWOSTICKS);
}

//***************************************************************************
static joy_on(int tog, int i)
{
   if (tog) {
      if (i) {
         if (StickType==0) RemoveKeyJoy  ();
      } else {
         if (StickType!=0) InstallKeyJoy ();
      }

      SetJoysticks (i);

      joy_recal ();

   }
   return (StickType==i);
}

//***************************************************************************
static joy_recal2()
{
   int i, j;

   InitJoy2 (Joystick|RudderStick); // purposely do not init the Cooly Hat or Throttle
   WaitTicks (1);

   ReadJoy2 (Joystick|RudderStick|ThrustStick|CoolyStick);
   WaitTicks (1);

   for(i=0;i<2;i++) {
      j=CenterPosition[i];

      MinStick[i]  = j-j/2;
      MaxStick[i]  = j+j/2;
      LowRange[i]  = j/2;
      HighRange[i] = j/2;
   }
}

//***************************************************************************
static joy_recal()
{
    JoyRecal (153, 120);
    RectCopy (Rp2, 153,120, 91,73, Rp1, 153,120);

}

//***************************************************************************
static set_kb_sens(int tog, int i)
{
	if (tog) {
		kb_sens[KbSense] = 0;
		KbSense          = 2-i;
		kb_sens[KbSense] = 1;
	} else return kb_sens[2-i];
}

//***************************************************************************
static set_mtn_detail(int tog, int i)
{
	if (tog)
		MTNDETAIL=(2-i);
	else
		return( (MTNDETAIL==(2-i)) );
}

//***************************************************************************
static set_plane_detail(int tog, int i)
{
	if (tog)
		PLANEDETAIL=(2-i);
	else
		return( (PLANEDETAIL==(2-i)) );
}

//***************************************************************************
static set_grnd_detail(int tog, int i)
{
	if (tog)
		GRNDDETAIL=(1-i);
	else
		return( (GRNDDETAIL==(1-i)) );
}

//***************************************************************************
static set_obj_detail(int tog, int i)
{
	if (tog)
		OBJECTDETAIL=(1-i);
	else
		return( (OBJECTDETAIL==(1-i)) );
}

//***************************************************************************
static set_xplo_detail(int tog, int i)
{
	if (tog)
		XPLODETAIL=(1-i);
	else
		return( (XPLODETAIL==(1-i)) );
}

//***************************************************************************
static set_conditions(int tog, int i)
{
	if (tog)
        SKYDETAIL = (1-i);
	else
		return( SKYDETAIL==(1-i) );
}

//***************************************************************************
static set_rollrate(int tog, int i)
{
	if (tog)
		FASTROLL=i;
	else
		return( (FASTROLL==i) );
}


