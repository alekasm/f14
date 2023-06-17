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
#include "cockpit.h"
#include "awg9.h"
#include "armt.h"
#include "planeai.h"
#include "setup.h"

extern  long	 PxLng,PyLng;
extern  UWORD	 MouseSprite;
extern	int      OurHead;
extern	int      sx,sy,sz;
extern	UWORD    Rtime;
extern  int	whichourcarr;
extern	UWORD	OurAlt;
extern  UWORD   Alt;
extern int 	lastmsgview;

extern confData ConfigData;

extern struct PlaneData far *GetPlanePtr(int PlaneNo);
extern struct MissileData far *GetMissilePtr(int MissileNo);
extern struct RadarData far *GetRadarPtr(int RadarNo);

extern   int AIWingMan;
extern  int     whichourcarr;     // holds value for which boat our carrier

extern	UWORD   GRPAGE0[];
extern   RPS   *Rp1;
extern   RPS   *Rp2;
extern   RPS   *Rp3;

//***************************************************************************

#define  RPLNCOLOR 1
#define  WPLNCOLOR 2
#define  WPTCOLOR 3
#define  CARCOLOR 4
#define  MSGCOLOR 3
#define  TXTCOLOR 4


#define  RESUMEXL 259
#define  RESUMEYL 172
#define  RESUMEXH 316
#define  RESUMEYH 188

#define  OBJXL 265
#define  OBJYL 136
#define  OBJXH 316
#define  OBJYH 148

#define  TELEXL 265
#define  TELEYL 152
#define  TELEXH 316
#define  TELEYH 164

#define  TXTXL 40
#define  TXTYL 64
#define  TXTXH 220
#define  TXTYH 164
#define  TXTWID TXTXH-TXTXL+1
#define  TXTHT TXTYH-TXTYL+1

#define  RPLNXL 267
#define  RPLNYL 28
#define  RPLNXH 277
#define  RPLNYH 38
#define  RPLNWID RPLNXH-RPLNXL+1
#define  RPLNHT RPLNYH-RPLNYL+1
#define  RPLNXMID ((RPLNXH+RPLNXL)/2)
#define  RPLNYMID ((RPLNYH+RPLNYL)/2)


#define  WPLNXL 267
#define  WPLNYL 42
#define  WPLNXH 277
#define  WPLNYH 52
#define  WPLNWID WPLNXH-WPLNXL+1
#define  WPLNHT WPLNYH-WPLNYL+1
#define  WPLNXMID ((WPLNXH+WPLNXL)/2)
#define  WPLNYMID ((WPLNYH+WPLNYL)/2)

#define  CARXL 268
#define  CARYL 56
#define  CARXH 275
#define  CARYH 67
#define  CARWID CARXH-CARXL+1
#define  CARHT CARYH-CARYL+1
#define  CARXMID ((CARXH+CARXL)/2)
#define  CARYMID ((CARYH+CARYL)/2)


//**********************************************************************
int NextRioTime=0;
int   iconmode=0;
BigRioDot(RPS *Rp,int x, int y, int color)
{
   RectFill(Rp, x-1, y-1, 3, 3, color);
}
WpsStar(RPS *Rp,int x, int y, int color)
{
   DrawLine(Rp, x-1,y,x+1,y,color);
   DrawLine(Rp, x,y-1,x,y+1,color);
}

extern    struct _path_ aipaths[];
DrawRioWayPoints(RPS *Rp,int type)
{
   int   cnt,i;
   int   Wpx,Wpy;

   cnt= aipaths[OurPath].numWPs;

   if (type==-1)
      {
      for (i=1;i<cnt;i++)
         {
         Wpx=((wps[i].xL) >> 12);
         Wpy=((wps[i].yL) >> 12);
         Wpy=MulDiv(Wpy,192,256);
         TXTN(i);
         SetAPen(Rp,WPTCOLOR);
         RpPrint (Rp, Wpx-2,Wpy-2,Txt);
         RectCopy(Rp,Wpx-2,Wpy-2,10,10,Rp1,Wpx-2,Wpy-2);
         WaitTicks(30);
         }
      return;
      }


   for (i=1;i<cnt;i++)
      {
      Wpx=((wps[i].xL) >> 12);
      Wpy=((wps[i].yL) >> 12);
      Wpy=MulDiv(Wpy,192,256);
      if (type)
         {
         TXTN(i);
         SetAPen(Rp,WPTCOLOR);
         RpPrint (Rp, Wpx-2,Wpy-2,Txt);
         }
      else
         {
         WpsStar(Rp,Wpx,Wpy,WPTCOLOR);
         }
      }
}

GetToken(char *S, char *D)
{
   int l;
   l=0;


   while (isspace(*(S)))
      {
      *(D++)=*(S++);
      l++;
      }
   while (  isprint(*(S))&&(isspace(*(S))==0) )
      {
      *(D++)=*(S++);
      l++;
      }

   *(D)=0;
   return(l);
}

TextInTheBox(RPS *Rp,char *S, int xl, int yl, int xh, int yh, int color)
{
   int   i,j,l,tl;
   char T[120];
   int   Xp,Yp;
   int   oldcolor;
   int   fontht;

   oldcolor=Rp->APen;
   Rp->APen=color;

   xl+=6;
   yl+=6;
   xh-=6;
   yh-=6;

   fontht=FontHeight(Rp->FontNr);

   Yp= yl;
   i=0;
   while (1)
      {
      if ((Yp+fontht)>yh) break;
      Xp=xl;
      j=i;
      l=0;
      while (isspace(*(S+j)))
         j++;
      while (tl=GetToken(S+j,&T[l]))
         {
         if ( (Xp+pstrlen(Rp,T))>xh  ) break;
         j+=tl;
         l+=tl;
         }
      T[l]=0;
      SetAPen(Rp,TXTCOLOR);
      RpPrint(Rp,Xp,Yp,T);
      Yp+=fontht;
      i=j;
      if (tl==0) break;
      }
   Rp->APen=oldcolor;
}

//int   RioFont=0;

DrawRioMapInfo(RPS *Rp,int force)
{
   int   Myx,Myy;
   int   Wngx,Wngy;
   int   Carx,Cary;
   int   time,i;
   long  TY;
   int   WngHead;
   int   CarHead;

   if (force==-1)
      {
      MouseHIDE();

      Myx=(PxLng >> 12);
      TY=(0x100000L-PyLng);
      TY>>=12;
      Myy=TY;
      Myy=MulDiv(Myy,192,256);

      // get wingman x,y, carrier x,y
	  if (AIWingMan != -999) {
      	if (ps[AIWingMan].status & ALIVE)
         	{
         	Wngx=(ps[AIWingMan].xL >> 12);
         	Wngy=(ps[AIWingMan].yL >> 12);
         	Wngy=MulDiv(Wngy,192,256);
         	WngHead=-ps[AIWingMan].head;
         	}
	  }

      Carx=(ps[whichourcarr].xL >> 12);
      Cary=(ps[whichourcarr].yL >> 12);
      Cary=MulDiv(Cary,192,256);

      RectCopy (Rp3, 0,0,256,200,Rp2,0,0);

      SetAPen(Rp,MSGCOLOR);
      RpPrint (Rp, 20,192,"CARRIER");
      RectCopy (Rp2, 0,0,320,200,Rp1,0,0);
      BigRioDot(Rp2,Carx, Cary, CARCOLOR);
      for (i=0;i<3;i++)
         {
         ScaleRotate(GRPAGE0[2],CARXMID,CARYMID,CARWID,CARHT,Rp1,Carx,Cary,0x100,0,0,0);
         WaitTicks(20);
         RectCopy(Rp2,Carx-10,Cary-10,20,20,Rp1,Carx-10,Cary-10);
         WaitTicks(20);
         }
      ScaleRotate(GRPAGE0[2],CARXMID,CARYMID,CARWID,CARHT,Rp2,Carx,Cary,0x100,0,0,0);

	  if (AIWingMan != -999) {
      	if (ps[AIWingMan].status & ALIVE)
         	{
         	RectCopy (Rp3, 0,192,320,8,Rp2,0,192);
         	BigRioDot(Rp2,Wngx, Wngy, WPLNCOLOR);
         	SetAPen(Rp,MSGCOLOR);
         	RpPrint (Rp, 20,192,"WINGMAN");
         	RectCopy (Rp2, 0,0,320,200,Rp1,0,0);
         	for (i=0;i<3;i++)
            	{
            	ScaleRotate(GRPAGE0[2],WPLNXMID,WPLNYMID,WPLNWID,WPLNHT,Rp1,Wngx,Wngy,0x100,WngHead,0,0);
            	WaitTicks(20);
            	RectCopy(Rp2,Wngx-10,Wngy-10,20,20,Rp1,Wngx-10,Wngy-10);
            	WaitTicks(20);
            	}
         	ScaleRotate(GRPAGE0[2],WPLNXMID,WPLNYMID,WPLNWID,WPLNHT,Rp2,Wngx,Wngy,0x100,WngHead,0,0);
         	}
	  }

      RectCopy (Rp3, 0,192,320,8,Rp2,0,192);
      BigRioDot(Rp2,Myx, Myy, RPLNCOLOR);
      SetAPen(Rp,MSGCOLOR);
      RpPrint (Rp, 20,192,"PLAYER");
      RectCopy (Rp2, 0,0,320,200,Rp1,0,0);

      for (i=0;i<3;i++)
         {
         ScaleRotate(GRPAGE0[2],RPLNXMID,RPLNYMID,RPLNWID,RPLNHT,Rp1,Myx,Myy,0x100,-OurHead,0,0);
         WaitTicks(20);
         RectCopy(Rp2,Myx-10,Myy-10,20,20,Rp1,Myx-10,Myy-10);
         WaitTicks(20);
         }
      ScaleRotate(GRPAGE0[2],RPLNXMID,RPLNYMID,RPLNWID,RPLNHT,Rp2,Myx,Myy,0x100,-OurHead,0,0);

      RectCopy (Rp3, 0,192,320,8,Rp2,0,192);
      SetAPen(Rp,MSGCOLOR);
      RpPrint (Rp, 20,192,"WAYPOINTS");
      RectCopy (Rp2, 0,0,320,200,Rp1,0,0);
      DrawRioWayPoints(Rp,-1);
      WaitTicks(20);
      RectCopy (Rp3, 0,192,320,8,Rp2,0,192);
      RectCopy (Rp2, 0,0,320,200,Rp1,0,0);

      MouseSHOW();

      return;

      }


   if ( force|| (NextRioTime>60)  )
      {
      NextRioTime=0;
      iconmode^=1;

      RectCopy (Rp3, 0,0,256,200,Rp2,0,0);

      Myx=(PxLng >> 12);
      TY=(0x100000L-PyLng);
      TY>>=12;
      Myy=TY;
      Myy=MulDiv(Myy,192,256);

// get wingman x,y, carrier x,y
	  if (AIWingMan != -999) {
      	if (ps[AIWingMan].status & ALIVE)
         	{
         	Wngx=(ps[AIWingMan].xL >> 12);
         	Wngy=(ps[AIWingMan].yL >> 12);
         	Wngy=MulDiv(Wngy,192,256);
         	}
	  }

      Carx=(ps[whichourcarr].xL >> 12);
      Cary=(ps[whichourcarr].yL >> 12);
      Cary=MulDiv(Cary,192,256);



// and waypoint info

      if (iconmode)
         {
         ScaleRotate(GRPAGE0[2],CARXMID,CARYMID,CARWID,CARHT,Rp2,Carx,Cary,0x100,0,0,0);

		if (AIWingMan != -999) {
	         if (ps[AIWingMan].status & ALIVE)
    	        ScaleRotate(GRPAGE0[2],WPLNXMID,WPLNYMID,WPLNWID,WPLNHT,Rp2,Wngx,Wngy,0x100,WngHead,0,0);
		}
         ScaleRotate(GRPAGE0[2],RPLNXMID,RPLNYMID,RPLNWID,RPLNHT,Rp2,Myx,Myy,0x100,-OurHead,0,0);
         }
      else
         {
         BigRioDot(Rp2,Carx, Cary, CARCOLOR);

		if (AIWingMan != -999) {
         if (ps[AIWingMan].status & ALIVE)
            BigRioDot(Rp2,Wngx, Wngy, WPLNCOLOR);
		}
         BigRioDot(Rp2,Myx, Myy, RPLNCOLOR);
         }
      DrawRioWayPoints(Rp,iconmode);

      MouseHIDE();
      RectCopy (Rp2, 0,0,320,200,Rp1,0,0);
      MouseSHOW();

      }

   if (force==0)
      {
      WaitTicks(1);
      NextRioTime++;
      }




}
//**********************************************************************

RioMap()
{
    int size,i,j,val,a,b;
	int key=0;
    struct Sams   *ssi;
    struct Radars *rri;
    struct Planes *psi;
	struct MissileData far *tempmissile;
	struct PlaneData far *tempplane;
	long MapX;
	long MapY;
	RPS Rpdef  =  {1, 0,0, 319,199, JAM3, GREEN, BLACK, 2};
	RPS *Rtemp  = &Rpdef;
	char far *DST;
	char far *SRC;
	int done=0;
	int MyMouseHits=0;
	int xpos;
	int ypos;
	int mseg;
	int GroundH;
   char S[255];
   FILE    *File;
	struct _missionHeader_ start_data;


   UpdateOurWPS(-999, 1);




   if ((File = fopen ("mission.f14", "rb")) != NULL)
      {
      fread (&start_data, sizeof (struct _missionHeader_), 1, File);

      fseek  (File, start_data.goalLoc, SEEK_SET) ;
      fread (S, sizeof(char),start_data.goalLen, File);
      fclose(File);
      for (i=0;i<start_data.goalLen;i++)
         S[i]=toupper(S[i]);
      S[start_data.goalLen]=0;
      S[254]=0;
      }
   else
      {
      S[0]=0;
      }

	MouseSETUP(MouseSprite,0,0);
	ScreenOff();
   	SndSounds(N_Pause);

//	RioTop(Rtemp, PxLng,(0x100000-PyLng), 0x100);

   LoadPicture(2,"map-sprt.PIC");

	switch(ConfigData.World) {
		case 0:
			LoadPicture(1,"map-cape.PIC");
		break;
		case 1:
			LoadPicture(1,"map-med.PIC");
		break;
		case 2:
			LoadPicture(1,"map-us.PIC");
		break;
		default:
			LoadPicture(1,"map-us.PIC");
		break;
	}
   RectCopy (Rp2, 0,0,256,200,Rp3,0,0);

   if (Status & TRAINING)
      {
      RectCopy (Rp3,TELEXL,TELEYL,TELEXH-TELEXL+1,TELEYH-TELEYL+1,
                  Rp2,TELEXL,TELEYL);
      }
   DrawRioMapInfo(Rtemp,1);

	ScreenOn();
	MouseSHOW();

	while (kbhit()) _bios_keybrd (_KEYBRD_READ); /* Flush kbd buffer */

   done=0;
	while (!done)
      {
		key=0;
		MyMouseHits=0;
		MyMouseHits = MouseHITS();

		if (kbhit())
			key = _bios_keybrd (_KEYBRD_READ);

      if (key==0)
         {
         if (MyMouseHits & 1)
            {
            if (InBox(MouseX,MouseY,RESUMEXL,RESUMEYL,RESUMEXH,RESUMEYH))
               key=ESCKEY;

            if (InBox(MouseX,MouseY,OBJXL,OBJYL,OBJXH,OBJYH))
               key=BKEY;

            if (InBox(MouseX,MouseY,TELEXL,TELEYL,TELEXH,TELEYH))
               key=PKEY;

            }
         }

      if (key)
         {
         switch (key)
            {
//            case FKEY:
//               RioFont++;
//               Rtemp->FontNr=RioFont;
//               DrawRioMapInfo(Rtemp,1);
//               break;
            case ESCKEY:
            case SLASH:
            case RKEY:
               done++;
               break;
            case BKEY:
               DrawRioMapInfo(Rtemp,-1);
               RectFill(Rp2, TXTXL, TXTYL,TXTWID,TXTHT,0);
               RectFill(Rp2, TXTXL+1, TXTYL+1,TXTWID-2,TXTHT-2,TXTCOLOR);
               RectFill(Rp2, TXTXL+2, TXTYL+2,TXTWID-4,TXTHT-4,0);
               TextInTheBox(Rtemp,S,TXTXL, TXTYL, TXTXH,TXTYH, 255);
               SetAPen(Rtemp,MSGCOLOR);
               CenterPrint (Rtemp, (TXTXL+TXTXH)/2,TXTYH-10,"PRESS ANY KEY TO CONTINUE");
               MouseHIDE();
               RectCopy (Rp2, TXTXL, TXTYL,TXTWID,TXTHT, Rp1,TXTXL, TXTYL);
               while (1)
                  {
		            MyMouseHits=0;
		            MyMouseHits = MouseHITS();
		            if (kbhit()) break;
                  if (MyMouseHits&3) break;
                  }
            	while (kbhit()) _bios_keybrd (_KEYBRD_READ); /* Flush kbd buffer */
               MouseHITS();   // flush mouse
	            MyMouseHits=0;
               DrawRioMapInfo(Rtemp,1);
               MouseSHOW();
               break;
            case PKEY:
               // if in training only
               if (Status & TRAINING)
                  {
                  MouseHIDE();
                  SetAPen(Rtemp,MSGCOLOR);
                  CenterPrint (Rtemp, 160,192,"USE LEFT MOUSE BUTTON TO RE-POSITION YOUR PLANE");
                  RectCopy (Rtemp, 0,192,320,8,Rp1,0,192);
                  MouseSHOW();
                  while (1)
                     {
		               key=0;
		               MyMouseHits=0;
		               MyMouseHits = MouseHITS();
		               if (kbhit())
			               key = _bios_keybrd (_KEYBRD_READ);
                     if (key==ESCKEY) break;
                     if (MyMouseHits&1)
                        {
                        if ((MouseX<256)&&(MouseY<192))
                           {
			                  PxLng = MouseX;
			                  PxLng = (PxLng << 12);
			                  PyLng = MulDiv(256,MouseY,192);
			                  PyLng = (PyLng << 12);
			                  PyLng = (0x100000 - (PyLng));

			                  GroundH = GetQHeight(PxLng,(0x100000-PyLng));

			                  if ((GroundH+500) > OurAlt)
                              {
				                  OurAlt = GroundH + 500;
				                  Alt = OurAlt >> 2;
				                  InitScottFlight();
   			                  }
                           }
                        break;
                        }
                     }
                  DrawRioMapInfo(Rtemp,1);
                  }
               break;
            }
         }
      DrawRioMapInfo(Rtemp,0);
      }

	MouseHIDE();
	ScreenOff();
	lastmsgview=-1;

	if (View<FRONTAL) {
    	LoadNewCockpit();
	} else {
       	ClearPage(0, BLACK);
		LoadPalNoPic("F14ORG01.PIC");
	}
	SndSounds(N_UnPause);
    return 1;


#if 0
		// Re-position our plane

		if (MyMouseHits & 2) {
			PxLng = MouseX;
			PyLng = MouseY;
			PyLng += PyLng/3;
			PxLng = (PxLng << 12);
			PyLng = (0x100000 - (PyLng << 12));

			GroundH = GetQHeight(PxLng,(0x100000-PyLng));

			if ((GroundH+500) > OurAlt) {
				OurAlt = GroundH + 500;
				Alt = OurAlt >> 2;
				InitScottFlight();
			}

			MapX = PxLng >> 12;
   			MapY = (0x100000 - PyLng) >> 12;
			MapY -= MapY/4;

			if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
				ScaleRotate(mseg,(214+5),(62+5),10,10,Rtemp,(int)MapX,
							(int)MapY,0x0100,0,0,0);
			}
		}
	}























	MapX = 0L;
	MapY = 0L;

	MapX = PxLng >> 12;
	MapY = (0x100000 - PyLng) >> 12;
	MapY -= (MapY/4);

	if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
		ScaleRotate(mseg,(214+5),(62+5),10,10,Rtemp,(int)MapX,
			(int)MapY,0x0100,0,0,0);
	}

    for (i=0, psi=ps; i<MAXPLANE; i++, psi++) {
        if ((psi->status&ACTIVE) && (psi->status&ALIVE)) {
			MapX = psi->xL >> 12;
			MapY = (0x100000 - psi->yL) >> 12;
			MapY -= MapY/4;
            a = (psi->head-OurHead) + 0x0800;
			tempplane = GetPlanePtr(psi->type);

			if (psi->status & FRIENDLY)
				b=2;
			else
				b=0;

			xpos = 154 + (((a>>12)&15)*10);

			switch(tempplane->bclass) {
				case FIGHTER_OBJCLASS:
					if (b==2)
						ypos=22;
					else
						ypos=82;
				break;
				case BOMBER_OBJCLASS:
					if (b==2)
						ypos=42;
					else
						ypos=102;
				break;
				case CARRIER_OBJCLASS:
					if (b==2) {
						ypos=52;
					} else {
						xpos=194;
						ypos=62;
					}
				break;
				case SHIP_OBJCLASS:
					if (b==2) {
						xpos=154;
						ypos=22;
					} else {
						xpos=194;
						ypos=62;
					}
				break;
				case CMISSILE_OBJCLASS:
					if (b==2)
						ypos=22;
					else
						ypos=72;
				break;
				default:
					if (b==2)
						ypos=22;
					else
						ypos=82;
				break;

			}
			if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
				ScaleRotate(mseg,(xpos+5),(ypos+5),10,10,Rtemp,(Rtemp->XBgn+(int)MapX),
							(Rtemp->YBgn+(int)MapY),0x0100,0,0,0);
			}
		}
    }

	for (i=0; i<4; i++) {
		MapX = wps[i].xL >> 12;
		MapY = wps[i].yL >> 12;
		MapY -= MapY/4;
		RectFill(Rtemp,(int)MapX-3,(int)MapY-3,2,2,6);
	}


	while (kbhit()) _bios_keybrd (_KEYBRD_READ); /* Flush kbd buffer */

	while (!done) {
		key=0;
		MyMouseHits=0;
		MyMouseHits = MouseHITS();

		if (kbhit())
			key = _bios_keybrd (_KEYBRD_READ);

		if (key) {
			switch (key) {
         		case ESCKEY:
				case SLASH:
					done++;
				break;
         		default:
            	break;
			}
		}

		// Re-position our plane

		if (MyMouseHits & 2) {
			PxLng = MouseX;
			PyLng = MouseY;
			PyLng += PyLng/3;
			PxLng = (PxLng << 12);
			PyLng = (0x100000 - (PyLng << 12));

			GroundH = GetQHeight(PxLng,(0x100000-PyLng));

			if ((GroundH+500) > OurAlt) {
				OurAlt = GroundH + 500;
				Alt = OurAlt >> 2;
				InitScottFlight();
			}

			MapX = PxLng >> 12;
   			MapY = (0x100000 - PyLng) >> 12;
			MapY -= MapY/4;

			if ((mseg=MapInEMSSprite(MISCSPRTSEG,1))) {
				ScaleRotate(mseg,(214+5),(62+5),10,10,Rtemp,(int)MapX,
							(int)MapY,0x0100,0,0,0);
			}
		}
	}


	MouseHIDE();
	ScreenOff();
	lastmsgview=-1;

	if (View<FRONTAL) {
    	LoadNewCockpit();
	} else {
       	ClearPage(0, BLACK);
		LoadPalNoPic("F14ORG01.PIC");
	}
	SndSounds(N_UnPause);
    return 1;
#endif

}


#define  HMAPBANK 0

RioTop(RPS *Rp, long X, long Z, int zoom )
{
   int   XP,ZP;
   int   WID,HT,XDC,YDC;
   char far *PTR;
   int   SEG;

   XP= (X>>12);
   ZP= (Z>>12);
   WID = Rp->Width1+1;
   HT  = Rp->Length1+1;
   XDC= Rp->XBgn+WID/2;
   YDC= Rp->YBgn+HT/2;

   FP_OFF(PTR)=0;
   FP_SEG(PTR)=SEG=MapInBank(HMAPBANK);

   TacScaleRotate256(SEG, (255/2),(191/2),255,191,Rp,(320/2),(200/2), 0x0100, 0,0x52,0);

}

