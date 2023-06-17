#include <dos.h>
#include "library.h"
#include "f15defs.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include <string.h>
#include "planeai.h"
#include "sounds.h"

#define MAXMESSAGES 5
#define MAXMSGLEN   60

char    HudMsg[MAXMESSAGES][MAXMSGLEN];
int     MsgCount[MAXMESSAGES];
int		MsgSrc[MAXMESSAGES];

int MsgOnScreen=0;

extern	RPS		*Rp3D, *Rp2,*Rp1,*Rp3;
extern  int     FrameRate;
extern  int     GettingMsg;
extern  int     View;

extern	UWORD	OurAlt;
extern  long	RadarAlt;
extern	int	    OurHead;
extern  int     Knots;
extern	UWORD	GRPAGE0[];
extern  int     BackSeat;
extern int MSGDETAIL;


//***************************************************************************
Messages()
{
	if ((View!=PADLOCK) && (View!=PILOT))
		MyMessages();
}

//***************************************************************************
Message(char *s,int src)
{
    int i, min, slot;

    if (ejected) return(1);

	mprintf(s);
	mprintf("\n\r");

	if (MSGDETAIL==3) return(0);
	if ((MSGDETAIL==2) && (src==GAME_MSG)) return(0);
	if ((MSGDETAIL==1) && (src==RIO_MSG)) return(0);
	MyMessage(s,src);
}

//***************************************************************************
MsgBox (Len, StartX, StartY)
{
    RectFill (Rp3D, StartX-1, StartY-1, Len+1+1, 7, LGREY);
    DrawLine (Rp3D, StartX-2, StartY-2, StartX+Len+1, StartY-2, WHITE);
    DrawLine (Rp3D, StartX-1, StartY+6, StartX+Len+1, StartY+6, DGREY);
    DrawLine (Rp3D, StartX-2, StartY-1, StartX-2,       StartY+5, WHITE);
    DrawLine (Rp3D, StartX+Len+1, StartY-1, StartX+Len+1, StartY+5, DGREY);
}




RPS MSGRp={1, 0,0, 319,199, JAM3, GREEN, BLACK, 1};
RPS *MRp=&MSGRp;

int lastmsgview=-1;
int LastMsgSeat=-1;
int newmsg=0;
char MyMSG[120]={0};
int  MyMSGTYP=0;
int msgtime=0;
int   Mupdate=0;
int   MsgEMS=0;
MyMessages()
{
   int   outside;
   char far *P;
   char far *SRC;
   int cnt, lastmsg, lastmsgnum;
   int msgplace;

	if (MsgEMS==0) {
	// try to get ems for messages
		if ((MsgEMS=MakeEMSFileBlock("MSG.EMS",(320*16/16) ))==0)
			MsgEMS=-1; // cant get it no inside messages !!!!
	}

	Rp1->Length1=199;

	switch (View) {
		case REARRIGHT:
		case REARLEFT:
		case FRONT:
		case DOWN:
		case RIGHT:
		case LEFT:
    		outside=0;
    		break;
		default:
    		outside=1;
	}

	if ((lastmsgview!=View)||(BackSeat!=LastMsgSeat)) {
		lastmsgview=View;
		LastMsgSeat=BackSeat;
		if (outside==0) {
    		// save the cockpit mask here
    		FP_SEG(P)=MapEmsFile(MsgEMS,0);
    		FP_OFF(P)=0;
    		FP_SEG(SRC)=GRPAGE0[0];
    		FP_OFF(SRC)=192*320;
    		fmemcpy(SRC,P,320*8);
		}
		Mupdate=1;
	}


	if (outside) {
		if (Mupdate) {
    		OutSideMSGBox(MRp);
    		OutSideHeading(MRp);
    		Mupdate=0;
			RectCopy(MRp,0,170,320,30,Rp1,0,170);
		} else {
			OutSideHeading(MRp);
			RectCopy(MRp,0,176,320,20,Rp1,0,176);
		}
	} else {
      if (MsgEMS<0) return(0);
      if (Mupdate) {
         Mupdate=0;
         if (MyMSG[0]) {
            InSideMSGBox(MRp);
            // copy to msg memory here
            FP_SEG(P)=MapEmsFile( MsgEMS,0);
            FP_OFF(P)=320*8;
            FP_SEG(SRC)= GRPAGE0[MRp->PageNr];
            FP_OFF(SRC)=192*320;
            fmemcpy(SRC,P,320*8);
            // then copy to real screen !!
            FP_SEG(SRC)= GRPAGE0[0];
            FP_OFF(SRC)=192*320;
            fmemcpy(P,SRC,320*8);
            }
      }

	  if (MyMSG[0]) {
/*
         // copy the msg memory here
         FP_SEG(P)=MapEmsFile( MsgEMS,0);
         FP_OFF(P)=320*8;
         FP_SEG(SRC)= GRPAGE0[MRp->PageNr];
         FP_OFF(SRC)=192*320;
         fmemcpy(P,SRC,320*8);
*/
	  }
	}

	if (msgtime) {
		if ((msgtime-GETTIME ())<0) {

			lastmsg = 0;
			lastmsgnum = 0;
			for(cnt = 0; cnt < MAXMESSAGES; cnt ++) {
				if(MsgCount[cnt] == 1) {
					lastmsgnum = cnt;
				}
				if(MsgCount[cnt] > lastmsg) {
					lastmsg = MsgCount[cnt];
				}
			}
			if(lastmsg == 0) {
	    		if (outside==0) {
    				// put back cockpit mask here
    				FP_SEG(P)=MapEmsFile( MsgEMS,0);
    				FP_OFF(P)=0;
    				FP_SEG(SRC)= GRPAGE0[0];
	    			FP_OFF(SRC)=192*320;
    				fmemcpy(P,SRC,320*8);
				}
    			MyMSG[0]=0;
	    		msgtime=0;
    			Mupdate=1;
			}
			else {
                Sound(N_RadarSearch, 1,0);
				msgplace = lastmsgnum;
	    		strncpy(MyMSG, HudMsg[msgplace], MAXMSGLEN);
    			MyMSG[MAXMSGLEN-1]='\0';
				MyMSGTYP=MsgSrc[msgplace];
				if(landingstatus & LANDPLAYERNOW)
					msgtime=GETTIME() + (2*60);      // two seconds per message
				else if(lastmsg > 2)
					msgtime=GETTIME() + (2*60);      // two seconds per message
				else
					msgtime=GETTIME() + (3*60);      // three seconds per message
    			Mupdate=1;
				HudMsg[msgplace][0] = 0;
				for(cnt = 0; cnt < MAXMESSAGES; cnt ++) {
					if(MsgCount[cnt] > 0)
						MsgCount[cnt] -= 1;
					else
						MsgCount[cnt] = 0;
				}
			}
		}
	}
	MsgOnScreen= (MyMSG[0])?1:0;

	if (MsgOnScreen) {
		Rp1->Length1=191;
	} else {
		Rp1->Length1=199;
	}
}

MyMessage(char *s,int src)
{
    int i, min, slot;
	int cnt, lastmsg, lastmsgnum;
	int msgplace;

    if (ejected) return(1);

	if((!msgtime) || ((src == RIO_NOW_MSG) || (src == WING_MENU_MSG))) {
        Sound(N_RadarSearch, 1,0);
    	strncpy(MyMSG, s, MAXMSGLEN);
    	MyMSG[MAXMSGLEN-1]='\0';
		if(src == RIO_NOW_MSG) {
			MyMSGTYP= RIO_MSG;
			msgtime=GETTIME() + (2*60);      // four seconds per message
		}
		else if(src == WING_MENU_MSG) {
			MyMSGTYP=src;
			msgtime=GETTIME() + (5*60);      // four seconds per message
		}
		else {
			MyMSGTYP=src;
			msgtime=GETTIME() + (3*60);      // three seconds per message
		}
    	Mupdate=1;
	}
	else {
		lastmsg = 0;
		lastmsgnum = 0;
		for(cnt = 0; cnt < MAXMESSAGES; cnt ++) {
			if(MsgCount[cnt] > lastmsg) {
				lastmsg = MsgCount[cnt];
				lastmsgnum = cnt;
			}
		}
		if(lastmsg < MAXMESSAGES) {
			if(lastmsg == 0)
				msgplace = 0;
			else {
				msgplace = lastmsgnum + 1;
				if(msgplace >= MAXMESSAGES)
					msgplace = 0;
			}
    		strncpy(HudMsg[msgplace], s, MAXMSGLEN);
			MsgCount[msgplace] = lastmsg + 1;
			MsgSrc[msgplace] = src;
		}
		else {
        	Sound(N_RadarSearch, 1,0);
			msgplace = lastmsgnum + 1;
			if(msgplace >= MAXMESSAGES)
				msgplace = 0;
	    	strncpy(MyMSG, HudMsg[msgplace], MAXMSGLEN);
    		MyMSG[MAXMSGLEN-1]='\0';
			MyMSGTYP=MsgSrc[msgplace];
			msgtime=GETTIME() + (2*60);      // two seconds per message
    		Mupdate=1;
    		strncpy(HudMsg[msgplace], s, MAXMSGLEN);
			MsgCount[msgplace] = 6;
			MsgSrc[msgplace] = src;
			for(cnt = 0; cnt < MAXMESSAGES; cnt ++) {
				MsgCount[cnt] -= 1;
			}
		}
	}
}

MBox (RPS *Rp,int StartX, int StartY, int EndX, int EndY)
{
    RectFill (Rp, StartX, StartY, EndX-StartX+1,EndY-StartY+1, 17);

    DrawLine (Rp, StartX, StartY, EndX,StartY,18);
    DrawLine (Rp, EndX,StartY, EndX,EndY,18);
    DrawLine (Rp, EndX,EndY, StartX,EndY,18);
    DrawLine (Rp, StartX,EndY, StartX,StartY,18);
}

MsgPrnt(RPS *Rp,int x, int y, char *S,int typ)
{
	int   msgcolor;

	msgcolor=15;

	Rp->DrawMode= JAM1;
	Rp->APen= msgcolor;
	Rp->FontNr = F_3x5B;
	RpPrint(Rp,x, y,S);
}


InSideMSGBox(RPS *Rp)
{
   MBox (Rp,0,192,319,199);
   MsgPrnt(Rp,10, 193,MyMSG,MyMSGTYP);
}

OutSideMSGBox(RPS *Rp)
{
   MBox (Rp,0,170,319,199);
   MsgPrnt(Rp,10, 170+16,MyMSG,MyMSGTYP);
}

extern  int     Speedy;
extern   char Rangestr[];
extern   char Coursestr[];
extern   char Slopestr[];

OutSideHeading(RPS *Rp)
{
	char	S[80];
	char	N[90];
	UWORD V;

   S[0]=0;
	strcpy(S,"HEADING     ALTITUDE       AIRSPEED     ");
	V=(UWORD)OurHead/(UWORD)182;
	itoa(V,N,10);
	strcpy(&S[11-strlen(N)],N);
	S[11]=' ';

	V=(UWORD)OurAlt;
	ltoa((unsigned long)V,N,10);
	strcpy(&S[26-strlen(N)],N);
	S[26]=' ';

	itoa(Knots,N,10);
	strcpy(&S[39-strlen(N)],N);
	S[39]=0;

   if (Speedy>1)
      {
      itoa(Speedy,N,10);
      strcat(N,"X ");
      }
   else
      N[0]=0;

   strcat(N,S);


	Rp->DrawMode= JAM1;
	Rp->APen= 15;
   Rp->FontNr = F_3x5B;
   RectFill (Rp, 10, 176, 300,8, 17);
	RpPrint(Rp,Rp->Width1/2-78, 170+6,N);

/*

   if (Rangestr[0])
      {
      RectFill (Rp, 10, 184, 300,8, 17);
   	RpPrint(Rp,20,184,Rangestr);
   	RpPrint(Rp,120,184,Coursestr);
   	RpPrint(Rp,220,184,Slopestr);
      }
*/

}
