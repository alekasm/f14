#include "library.h"

#include <stdlib.h>
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "armt.h"

extern  RPS     *RpCRT;
extern  UWORD   GRPAGE0[];
extern  COORD   Px,Py;
extern  UWORD   Alt;
extern  int     OurHead;
extern  int     OurRoll,OurPitch;
extern  UWORD   OurAlt;
extern  int     Knots;
extern  long    PxLng,PyLng;
extern  int     DeClutter;
extern  char    Txt[];
extern  int     NextWayPoint,ChangeWayPoint,WPMode;
extern  int     BingoFuel;
extern  int     Rtime;

extern RPS *Rp1;

int	hsdmode=0;

scaley(y)
{
	return(y*14/16);
}

fourpoints(char far *screen,int x,int y,int color) {                 // draw 4 symmetric points about a circle
   *(screen+x+y)=color;
   *(screen+x-y)=color;
   *(screen-x+y)=color;
   *(screen-x-y)=color;
}

circlepoints(char far *screen,int x,int y,int ox,int oy,int color) { // draw 8 symmetric points about a circle
   screen+=ox+oy*320;
   fourpoints(screen,x,(y*14)/16*320,color);
   fourpoints(screen,y,(x*14)/16*320,color);
}

drawcircle(char far *screen,int ox,int oy,int radius, int color) {   // draw circle using Bresenham method
   int x,y,d,de,dse;
   x=0;
   y=radius;
   d=1-radius;
   de=3;
   dse=-2*radius+5;

   circlepoints(screen,x,y,ox,oy,color);
   while (y>x++) {
      de+=2; dse+=2;
      if (d<0) {
	  	d+=de;
		circlepoints(screen,x,y  ,ox,oy,color);
	  }
      else {
	  	d+=dse;
		dse+=2;
		circlepoints(screen,x,y--,ox,oy,color);
	  }
   }
}


pitchbar(int w,int dist,int ox,int oy) {      // draw a bar according to pitch, dist away from center (ox,oy) of width/2 in length(w)
	int x,y,x2,y2;

	Rotate2D( w,dist,OurRoll,&x ,&y );
	x =x *22/30;
	y =y *22/30;
	y =scaley(y );
	Rotate2D(-w,dist,OurRoll,&x2,&y2);
	x2=x2*22/30;
	y2=y2*22/30;
	y2=scaley(y2);
	DrawLine(RpCRT, ox+x,oy+y,ox+x2,oy+y2, WHITE);
}

pitchnum(int w,int pitch,int dist,int off,int ox,int oy) { // draw numbers on a pitch bar where the left side is in ten's of degrees, the right side always zero ie: 1 - 0 is 10 degrees
   int x,y;

   x=abs(pitch-dist-off)/10;
   if (x>8) x=9-(x%8);               // max pitch 90 degrees
   itoa(x,Txt,10);
   Rotate2D( w,dist+off,OurRoll,&x,&y);
   x=x*22/30;
   y=y*22/30;  // rotate 0 into position, dist+off is distance from center of pitch bar
   RpPrint(RpCRT,ox+x-1,oy+scaley(y)-1,"0");
   Rotate2D(-w,dist+off,OurRoll,&x,&y);
   x=x*22/30;
   y=y*22/30;  // rotate 0 to 8 into position
   if (Txt[0]=='1') x+=2;                                      // if printing a '1' then shift over because not using proportional font, ie font 7
   RpPrint(RpCRT,ox+x-1,oy+scaley(y)-1,Txt);
}


int scaler(int x) { return(x*22/30); } // scale ladder's view, the z axis, um the magnification, do you get it yet?

pitchbar2(int w,int pitch,int dist,int ox,int oy,int dir,char *text) { // draw pitch bar with any text at both endpoints
   int x1=w,y1=(dir?-4:4),x2=w,y2=0,x3=6,y3=0;
   int a,b,c,d,p,t=pstrlen(RpCRT,text)/2-1;

   Rotate2D(0,dist,OurRoll,&a,&b);
   ox+=scaler(a);
   oy+=scaley(scaler(b));

   p=pitch*91*(dir?-1:1);
   Rotate2D(x1+8,y1,p,&a,&b);
   Rotate2D( a,b,OurRoll,&c,&d);
   c=scaler(c);
   d=scaler(d);
   RpPrint(RpCRT,ox+c-t,oy+d-1,text);
   Rotate2D(-a,b,OurRoll,&c,&d);
   c=scaler(c);
   d=scaler(d);
   RpPrint(RpCRT,ox+c-t,oy+d-1,text);

   Rotate2D(x1,y1,p,&x1,&y1);          // pitch upward the |-- --| lines
   Rotate2D(x2,y2,p,&x2,&y2);
   Rotate2D(x3,y3,p,&x3,&y3);

   Rotate2D( x1,y1,OurRoll,&a,&b);
   a=scaler(a);
   b=scaler(b);
   Rotate2D( x2,y2,OurRoll,&c,&d);
   c=scaler(c);
   d=scaler(d);
   DrawLine(RpCRT, ox+a,oy+b,ox+c,oy+d ,WHITE);
   Rotate2D( x3,y3,OurRoll,&a,&b);
   a=scaler(a);
   b=scaler(b);
   DrawLine(RpCRT, ox+c,oy+d,ox+a,oy+b ,WHITE);

   Rotate2D(-x1,y1,OurRoll,&a,&b);     a=scaler(a); b=scaler(b);
   Rotate2D(-x2,y2,OurRoll,&c,&d);     c=scaler(c); d=scaler(d);
   DrawLine(RpCRT, ox+a,oy+b,ox+c,oy+d ,WHITE);
   Rotate2D(-x3,y3,OurRoll,&a,&b);     a=scaler(a); b=scaler(b);
   DrawLine(RpCRT, ox+c,oy+d,ox+a,oy+b ,WHITE);
}

pitchnum2(int w,int pitch,int dist,int off,int ox,int oy) {
   int x,y,a;

   x=abs(a=(pitch-dist-off));
   dist=(dist+off)*3;
   if (x==90) {   // draw circle? at apex or bottom
      Rotate2D(0,dist,OurRoll,&x,&y);
	  x=scaler(x);
	  y=scaley(scaler(y));
      draw_circle(RpCRT,ox+x,oy+y,3,WHITE,0);
      return(1);
   } else
	if (x>85) x=95-(x%85);
   itoa(x,Txt,10);
   pitchbar2(w-3,x,dist,ox,oy,((a<0)&&(a>-90))||(a>85),(char *)Txt);
}


static char *degrees(int deg)
{
   static char s[3];

   itoa((deg=deg%36+1),s,10);
   if (deg<10) { s[2]='\0'; s[1]=s[0]; s[0]='0'; }
   return(s);
}

headingscale(int w,int color1,int color2) {
   unsigned deg=(unsigned)OurHead/91;
   int i,x,deg10=deg/20+33,xmin=(RpCRT->Width1/2)-w+1,xmax=(RpCRT->Width1/2)+w;

   RpCRT->APen=color1;
   DrawLine(RpCRT,xmin,5,xmax,5,color2);
   for (x=(RpCRT->Width1/2)-40-deg%20,i=0; x<(xmax+1); x+=4,i++) {
      if (x>=xmin) {
         DrawDot(RpCRT,x,4,color2);
         if (!(i%5)) {
		 	DrawDot(RpCRT,x,3,color2);
			RpPrint(RpCRT,x?x-3:x-4,0,degrees(deg10));
		 }
      } if (!(i%5)) deg10++;
   }
   x=rng(((AutoHead-OurHead)/182)*2+(RpCRT->Width1/2),(RpCRT->Width1/2)-w,(RpCRT->Width1/2)+w);
   DrawLine(RpCRT,x,5,x-2,7,color1); DrawLine(RpCRT,x,5,x+2,7,color1); // draw wedgy
}


#ifdef YEP
AdiCrt(int CrtNr)
{    // Artificial Horizon
	int h,i,j,line,pitch,x,y,x2,y2,color=RED;
	int ox=12;
	int oy=15;
	int adigroundcolor=5;
	int adiskycolor=15;
	char far *screen;
	static char radius=14;
	static int stupid[]={ 14,14, 12, 9, 8, 6, 3, 2 };

	screen=(char far *)((ULONG)GRPAGE0[1]<<16L);
	RectFill(RpCRT,0,0,30,34,BLACK);
	screen+=(RpCRT->XBgn+ox-radius)+(RpCRT->YBgn+oy-radius+4)*320;
	x2=0;
	y2=((long)OurPitch*radius)/0x1555;  // 0x1555=30 degree pitch view
	Rotate2D(x2,y2,OurRoll,&x2,&y2);
	y2=scaley(y2);
	Rotate2D(radius,0,OurRoll,&x,&y);
	y=scaley(y);

	for (i=2;i<(radius-1);i++) {
		line=abs(i-radius/2);        // what line (y value) are we working with
		j=(i-y2/2-radius/2)*2;
		if (y)
			h=stupid[line]+((x+x)*j/(y+y))+x2;  // x=(dx/dy)*y solve for x, x2 and y2 are pitch
		else {
			h=radius*2;                         // make color change point off display
			if (j<0) h=-h;                      // reverse color change position reverses colors
			if (!(OurRoll&0x4000)) h=-h;        // if in lower right or upper left half reverse color change
			if ((OurRoll>-182)&&(OurRoll<0)) h=-h; // cheap fix, display would flip colors from 0 to -182
		}
		screen+=radius-stupid[line];
		if ((OurRoll/182)>=0) {
			for (j=0;(j<h)&&(j<(stupid[line]*2));j++) *(screen++)=adiskycolor;
		 		while (j++<(stupid[line]*2)) *(screen++)=adigroundcolor;
		}
		else {
			for (j=0;(j<h)&&(j<(stupid[line]*2));j++)
		  		*(screen++)=adigroundcolor;
			while (j++<(stupid[line]*2)) *(screen++)=adiskycolor;
		}
		screen+=radius-stupid[line];
		screen+=640-radius*2; // skip down 2 lines
	}
	return 1;
}
#endif


extern RPS        *Rp1, RpX2, *Rp2, *Rp3D, RpX3D, *RpHud, RpHudX;


HsdCrt(int CrtNr)
{             // Compass thing
	static int color=DGREY;
	int radius=26;
	int ox=(RpCRT->Width1/2)+RpCRT->XBgn;
	int oy=(RpCRT->Length1/2)+RpCRT->YBgn;
	int i,x,y,x2,y2,bx,by;
	char heading[5];
	char far *screen;
	char s[10];
	int off,font;

	if (Damaged&D_PDISPLAYS) {
		RectFill(RpCRT,0,0,RpCRT->Width1+1,RpCRT->Length1+1, BLACK);
		return(1);
	}

	font = GetFont(RpCRT,0);
	screen=(char far *)((ULONG)GRPAGE0[1]<<16L);
	RectFill(RpCRT,0,0,RpCRT->Width1,RpCRT->Length1,BLACK);

	circlepoints(screen,0,radius+4,ox,oy,LGREY);     ///        ticks on the
	circlepoints(screen,0,radius+3,ox,oy,LGREY);        ////     compass
	circlepoints(screen,radius-4,radius-4,ox,oy,LGREY);     ////
	circlepoints(screen,radius-5,radius-5,ox,oy,LGREY);         ////
	screen+=ox+oy*320;                                              ///
	for (i=0;i++<18;) {                                                //
		Rotate2D(0,-radius-1,OurHead+i*910,&x,&y);                        //
		*(screen+x+scaley(y)*320)=color;
		*(screen-x-scaley(y)*320)=color;  //
		*(screen+y-scaley(x)*320)=color;
		*(screen-y+scaley(x)*320)=color;   //
		if (i%2) {                                                           //
			Rotate2D(0,-radius,OurHead+i*910,&x,&y);                          //
			*(screen+x+scaley(y)*320)=color;
			*(screen-x-scaley(y)*320)=color; //
			*(screen+y-scaley(x)*320)=color;
			*(screen-y+scaley(x)*320)=color; //
		}
	}

	DrawLine(RpCRT,(RpCRT->Width1/2) ,(RpCRT->Length1/2)-26,(RpCRT->Width1/2) ,(RpCRT->Length1/2)-18,LGREY);  // lubber line

	SetFont(RpCRT,F_3x5B);
	RpCRT->APen=WHITE;
	sprintf(s,"WPD %d",SteerRange());
	RpPrint(RpCRT,(RpCRT->Width1/2)-12,(RpCRT->Length1/2)-6,s);

	sprintf(s,"TAS%d",Knots);
	RpPrint (RpCRT, (RpCRT->Width1/2)-10,(RpCRT->Length1/2)+2,s);
	sprintf(s,"GS%d",TAStoIAS(Knots));
	RpPrint (RpCRT, (RpCRT->Width1/2)-9,(RpCRT->Length1/2)+10,s);

	/* Put labels/Numbers on */
	SetFont(RpCRT,F_3x5B);
	RpCRT->APen=WHITE;
	Rotate2D(0,-radius-1,OurHead,&x,&y);
	RpPrint(RpCRT, x+(RpCRT->Width1/2)-2, scaley(y)+(RpCRT->Length1/2)-2,"N");
	RpPrint(RpCRT,-y+(RpCRT->Width1/2)-2, scaley(x)+(RpCRT->Length1/2)-2,"E");
	RpPrint(RpCRT, y+(RpCRT->Width1/2)-2,-scaley(x)+(RpCRT->Length1/2)-2,"W");
	RpPrint(RpCRT,-x+(RpCRT->Width1/2)-2,-scaley(y)+(RpCRT->Length1/2)-2,"S");
	SetFont(RpCRT,F_3x4);
	RpCRT->APen=LGREY;
	Rotate2D(0,-radius-1,OurHead-0x1555,&x,&y);
	RpPrint(RpCRT, x+(RpCRT->Width1/2)  , scaley(y)+(RpCRT->Length1/2)-1,"3");
	RpPrint(RpCRT, y+(RpCRT->Width1/2)-3,-scaley(x)+(RpCRT->Length1/2)-1,"30");
	RpPrint(RpCRT,-y+(RpCRT->Width1/2)-2, scaley(x)+(RpCRT->Length1/2)-1,"12");
	RpPrint(RpCRT,-x+(RpCRT->Width1/2)-2,-scaley(y)+(RpCRT->Length1/2)-1,"21");
	Rotate2D(0,-radius-1,OurHead-0x2aaa,&x,&y);
	RpPrint(RpCRT, x+(RpCRT->Width1/2)  , scaley(y)+(RpCRT->Length1/2)-1,"6");
	RpPrint(RpCRT, y+(RpCRT->Width1/2)-3,-scaley(x)+(RpCRT->Length1/2)-1,"33");
	RpPrint(RpCRT,-y+(RpCRT->Width1/2)-2, scaley(x)+(RpCRT->Length1/2)-1,"15");
	RpPrint(RpCRT,-x+(RpCRT->Width1/2)-3,-scaley(y)+(RpCRT->Length1/2)-1,"24");

	/* Draw heading indicator */
	i=OurHead-angle(wps[NextWayPoint].x-Px,-(wps[NextWayPoint].y-Py)); // airport arrow indicator
	Rotate2D(0,-radius-6,i,&x,&y);
	y=scaley(y);
	Rotate2D(-2,-radius-3,i,&x2,&y2);
	y2=scaley(y2);
	DrawLine(RpCRT, (RpCRT->Width1/2)+x+1,(RpCRT->Length1/2)+y+1,(RpCRT->Width1/2)+x2+1,(RpCRT->Length1/2)+y2+1, WHITE);
	Rotate2D( 2,-radius-3,i,&x2,&y2);
	y2=scaley(y2);
	DrawLine(RpCRT, (RpCRT->Width1/2)+x+1,(RpCRT->Length1/2)+y+1,(RpCRT->Width1/2)+x2+1,(RpCRT->Length1/2)+y2+1, WHITE);

	SetFont(RpCRT,font);

	/* MakeItGreen(); */

	return 1;
}

//***************************************************************************
int SteerRange()
{
	UWORD    dx;
	UWORD    dy;

	dx = labs (wps[NextWayPoint].xL-PxLng) >>5;
	dy = labs (wps[NextWayPoint].yL-(0x100000-PyLng)) >>5;
	return (MDiv (Dist2D (dx, dy), 2, 95));  // nautical miles
}

