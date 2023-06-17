#include <stdlib.h>
#include "library.h"
#include "world.h"
#include "sdata.h"
#include "f15defs.h"
#include "awg9.h"
#include "armt.h"

extern RPS  *RpCRT;
extern int   OurHead;
extern long  PxLng;
extern long  PyLng;
extern UWORD Px, Py;
extern UBYTE FlirTbl[];
extern UWORD NOASPECT;
extern int   CrtDisplays [];
extern int   CrtConfig [3][8];
extern int   ag_msg_cnt;
extern int   ag_msg;
extern UWORD RadarCursorSprite[];
extern int   MouseCrtX;
extern int   MouseCrtY;
extern int   MouseDsp;
extern int   ACQ_PX;
extern int   ACQ_PY;
extern int   CurCRT;
extern char  Txt[];
extern long  RadarAlt;

extern UWORD far CrtMask[];

extern struct CloseObj *TargetObj;
extern char  *HRM_RANGES[];

extern struct OBJ3D far OBJECTS[96][16];
extern char	        far MAPBUF[256][256];

       int   HRMscale   =  1;      // range of values is 0-7
       int   HRMChanged =  0;
       int   HRMTime    =  0;
       int   MapRangeBad=  0;
       int   CheckingHRMLimits=1;

static int   Mapx       = -1;
static int   Mapy       = -1;
static int   MapHead    =  0;
static int   MapScale   =  0;
static int   MapTime    =  0;
static int   MapTgt     = -1;

/*----------*/
HiResMap (i) {
    int         x, y;
    long        dx, dy;
    UWORD       ZP;
    UWORD far  *M;

    if (Damaged&D_RADAR) {
        RectFill (RpCRT, 0,0, CRTXSIZ, CRTYSIZ, BLACK);
        RpCRT->APen = 0x13;
        RpCRT->FontNr = F_3x5B;
        RpPrint (RpCRT, CRTXCENT-14, CRTYCENT, "DAMAGED");
        return 1;
    }

    if (!HRMChanged) return 0;

    if (CheckingHRMLimits && MapTime>Gtime) {
        RectFill (RpCRT, 0,0, CRTXSIZ, CRTYSIZ, BLACK);
        TXTN (MapTime-Gtime);
        RpCRT->FontNr=F_3x5A;
        RpPrint (RpCRT, CRTXCENT-1, 2, Txt);
        RpPrint (RpCRT, CRTXSIZ/2-6, CRTYSIZ-8, HRM_RANGES[HRMscale]);
        HRMChanged=2;
        return 1;
    }

    RpCRT->Width1 -= 3;
    RpCRT->XBgn++;
    RotatingMap (RpCRT, (long) Mapx<<HTOV, (long) Mapy<<HTOV, MapScale+4, -MapHead, (char far *) 0L, 0, 0);
    RpCRT->Width1 += 3;
    RpCRT->XBgn--;

    if (MapScale<=1) {
        ZP = 4096<<MapScale;
        NOASPECT = 1;

        Bgn3D (RpCRT, -MapHead, 0xC000, 0, 0,0, ZP, 1, 0x100);
        Draw3dGroundObjects ((long) Mapx<<HTOV, ZP, (long) Mapy<<HTOV, 1);

        NOASPECT = 0;
    }

    if (DESIGNATED_AG) {
        dx = DESIGNATED_X-((long)Mapx<<HTOV);
        dy = DESIGNATED_Y-((long)Mapy<<HTOV);

        if (labs(dx)<23000L && labs(dy)<23000L) {
            Rotate2D ((int)dx, (int)dy, MapHead, &x, &y);
            x = ( x >>(MapScale-1+HTOV) ) + CRTXCENT;
            y = ( y >>(MapScale-1+HTOV) ) + CRTYCENT;

            ClipSprite (RpCRT, x-4, y-4, RadarCursorSprite[0]);
        }
    }

    OverlaySequencePoints ((long)Mapx<<HTOV, (long)Mapy<<HTOV, MapHead, 1<<(4+MapScale), HRM);

    clip_rotmap     ();

    CrtMask[MASKX] = RpCRT->XBgn;
    CrtMask[MASKY] = RpCRT->YBgn;

    FlirCopy (CrtMask);

    RpCRT->APen = TSD_MONO_COLOR;
    RpCRT->FontNr = F_3x5A;
    RpPrint (RpCRT, CRTXSIZ/2-6, CRTYSIZ-8, HRM_RANGES[HRMscale]);

    ag_err_msgs ();

    return 1;

}

/*----------*/
SetHiResMap (X, Y, Disp) {

    Mapx     = X;
    Mapy     = Y;
    MapHead  = OurHead;
    MapScale = HRMscale;
    MapTime  = HRMTime;
    MapTgt   = -1;

    HRMChanged  = 1;

    CurCRT = Disp;

    if (CrtDisplays[Disp]==RDR) {               // Flip display if no HRM visible

        if ( Disp<=2 && (CrtDisplays[0]==HRM
                      || CrtDisplays[1]==HRM
                      || CrtDisplays[2]==HRM) ) return;

        if ( Disp>2  && (CrtDisplays[3]==HRM
                      || CrtDisplays[4]==HRM
                      || CrtDisplays[3]==HRM
                      || CrtDisplays[3]==HRM) ) return;

        CrtDisplays  [Disp] = HRM;
        CrtConfig [2][Disp] = HRM;
    }

}

/*----------*/
RefreshHRM () {
    HRMChanged=1;
}

/*----------*/
HrmLock () {
    int     x, y, rx, ry;

    x = ( MouseCrtX-(CRTXCENT) ) <<MapScale >>1;
    y = ( MouseCrtY-(CRTYCENT) ) <<MapScale >>1;

    Rotate2D (x, y, -MapHead, &rx, &ry);

    rx += Mapx;
    ry += Mapy;

    DESIGNATED_X = (long) rx<<HTOV;
    DESIGNATED_Y = (long) ry<<HTOV;
    LockAG ();

    HRMChanged = 1;

}

/*----------*/
HrmRemap () {
    int     x, y, rx, ry;

    if (APG_SNIFF) return;

    if (MapRangeBad) {
       ag_msg_cnt = 3*60;        // 3 seconds
       ag_msg = MapRangeBad-1;
       HRMChanged = 1;
       return 0;
    }

    ag_msg_cnt = 0;              // remove existing message

    x = ( MouseCrtX-(CRTXCENT) ) <<MapScale >>1;
    y = ( MouseCrtY-(CRTYCENT) ) <<MapScale >>1;

    Rotate2D (x, y, -MapHead, &rx, &ry);

    rx += Mapx;
    ry += Mapy;

    ACQ_PX = rx;
    ACQ_PY = ry;

    SetHiResMap (ACQ_PX, ACQ_PY, MouseDsp);

    DESIGNATED_X = (long) rx<<HTOV;
    DESIGNATED_Y = (long) ry<<HTOV;
    LockAG ();

    HRMChanged = 1;

}

/*----------*/
CheckHrmRemap () {
    int     x, y, rx, ry;

    x = ( MouseCrtX-(CRTXCENT) ) <<MapScale >>1;
    y = ( MouseCrtY-(CRTYCENT) ) <<MapScale >>1;

    Rotate2D (x, y, -MapHead, &rx, &ry);

    rx += Mapx;
    ry += Mapy;

    return (CheckHRMLimits (rx, ry));
}

/*----------*/
CheckHRMLimits (int X, int Y) {
    static int HRMMinRange [] = {143,  162,  209,  247,  523, 1045, 2090};
    static int HRMMaxRange [] = {935, 1868, 2311, 3688, 7363, 7125, 6650};

    int     dx, dy;
    int     dist;
    int     angl;

    MapRangeBad = 0;

    if (CheckingHRMLimits) {
        dx = X-Px;
        dy = Y-Py;

        dist = Dist2D (X-Px, Y-Py);
        angl = abs (OurHead-angle (dx, -dy));

        if (dist<HRMMinRange[HRMscale] || dist>HRMMaxRange[HRMscale]) {
            MapRangeBad = 1;
        } else if (angl>9102) {     // +-50 degree view angle
            MapRangeBad = 2;
        } else if (angl<1456) {     // +-8 degree blind zone
            MapRangeBad = 3;
        }

        HRMTime = 10L*dist/HRMMaxRange[HRMscale] + 10L*(9102-angl)/9102 + Gtime + 3;

        if ( (RadarAlt<500L  && dist>475) ||        // 10 nm
             (RadarAlt<1000L && dist>950) ||        // 20 nm
             (RadarAlt<2000L && dist>1900)    ) {   // 40 nm

                MapRangeBad = 1;
        }

    }
            
    return (MapRangeBad!=0);
}

/*----------*/
HRM_kb_acquire () {

	int	    MapX,MapZ;
	int	    XOFF,ZOFF,i,j,k,t,NUM;
    int     dX, dZ;
    int     sx, sy;
    int     STAMP;
    int     cnt;
    long    X, Z;
    struct  OBJ3D far  *OP;

    X = (long)Mapx<<HTOV;
    Z = (long)Mapy<<HTOV;

	MapX= (X>>12)&0xff;
	MapZ= (Z>>12)&0xff;

	XOFF= (X&0xfff);
	ZOFF= (Z&0xfff);


    for (t=0; t<2; t++) {
        cnt=0;
        MapTgt++;

	    for (i=-1;i<=1;i++) {
		    for (j=-1;j<=1;j++) {

			    STAMP = MAPBUF[MapZ+i][MapX+j];

	            for (k=0;k<16;k++) {

                    OP  = &OBJECTS[STAMP][k];
                    NUM = OP->NUM;

                    if (NUM!=-1) {

                        dX = (4096*j)+(OP->X) - XOFF;
                        dZ = (4096*i)+(OP->Y) - ZOFF;

                        Project2D ((long)dX, (long)dZ, MapHead, 1<<(4+MapScale), &sx, &sy);

                        if (sx>=0 && sx<CRTXSIZ && sy>=0 && sy<CRTYSIZ) {
                            if (cnt++==MapTgt) {
                                DESIGNATED_X = X+dX;
                                DESIGNATED_Y = Z+dZ;
                                LockAG ();

                                HRMChanged = 1;
                                return;
                            }

                        }

		            }
			    }
		    }
        }

        MapTgt = -1;
    }

}

