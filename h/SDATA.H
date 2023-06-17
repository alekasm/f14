/* sdata.h */

/* Data structures for stealth game logic. */
/* (non-scenario specific */

#define TRUE 1
#define FALSE 0

#define TXT(a) strcpy (Txt, a )
#define TXA(a) strcat (Txt, a )
#define TXTN(a) (char *) itoa (a, Txt,10)
#define TXN(a) strcat (Txt, (char *) itoa(a, ntmp, 10))

#if 1
#define FTXT(a) _fstrcpy ((char far *)Txt, (char far *)a )
#define FTXA(a) _fstrcat ((char far *)Txt, (char far *)a )

#else

#define FTXT(a) strcpy (Txt, (char far *)a )
#define FTXA(a) strcat (Txt, (char far *)a )

#endif

char ntmp[10];
extern char Txt[];

/* ------ All about our plane ------ */
#ifdef _include_data
    int Damaged=4,Status=0,Fuel=5000;
    int Stores[4] = {0,12,18,3};
    int Rounds=675;
    int AutoAlt=0,AutoHead;
    int Diff=1,Night=FALSE;
    int ejected,FrameRate=4,Gtime,Direction;
#else
	extern int Damaged,Status,Fuel,Rounds;
	extern int Stores[];
	extern AutoAlt,AutoHead;
	extern Diff,Night;
	extern ejected,FrameRate,Gtime,Direction;
#endif


/* ------ Status Bits --------- */
#define WHEELSUP        0x01
#define BRAKESON        0x08

#define DESIGNATE       0x0100
#define TRAINING        0x1000
#define SECONDHIT       0x2000
#define PRIMARYHIT      0x4000
#define SOUNDSOFF       0x8000

/* ------ Missile Decoys (chaff and flares) ------ */

struct Decoy {
	COORD  x;
	COORD  y;
	int   alt;
	int   type;
	int   time;
	int   color;
};

#define FLARE 1
#define CHAFF 2

#define MAXDECOY 33

#ifdef _include_data
    int NDECOY = 30;

struct Decoy ds[MAXDECOY] =
{
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
      0,0,0,0,0,0,
};
#else
extern struct Decoy ds[];
extern int NDECOY;
#endif

/* ------ Gunfire data: only tracers are tracked ------ */

  struct Tracer {  /*  18 bytes  */
    long    x;
    long    y;
    int     z;
    int     dx;
    int     dy;
    int     dz;
	int		firedby;
  };

#define NTRACERS 20*2
#ifdef _include_data
  struct Tracer trs[NTRACERS];
#else
extern struct Tracer trs[];
#endif

/* ------ Enemy plane decision tables ------ */

#define IML 0x0100
#define SPS    0x0200

   struct Sams {    /*  44  */
      long   x;
      long   y;     /* Scott style 20-bit coords (y is same sense as 15-bit) */
      long   tgtx;
      long   tgty;
      int   alt;
      int   speed;
      int   head;
      int   pitch;
      int   roll;
      int   dist; /* distance can fly in bits-per-sec/4, 825 bits-per-KM */
      int   type;
      int   weapon;
      int   station;
      int   source;
      int   srctype;
      int   target;
      int   tgttype;
      int   losing;
  };

#define NSAM 28
#define NESAM 20
#define NPSAM 15

#ifdef _include_data
struct Sams ss[NSAM];

#else
extern struct Sams ss[NSAM];
#endif

//  #define NLOG 64
#define NLOG 32

#if DETLOGDATA
struct log_data {    /*  16 bytes  */
  int event;
  int object;
  int code;
  int time;
  long x, y; };

#ifdef _include_data
    struct log_data gamelog[NLOG];
#else
extern struct log_data gamelog[];
#endif
#else
typedef struct _logObject_ {   /*  32 bytes  */


        int             objectNum ;     // index into object array passed
                                        // negative object # was added after
                                        //              game started

        int             objectType ;    // index into basedef list

        int             side ;          // who do they fly for
        long            x ;
        long            y ;
        long            z ;
        int             speed ;         // in knots

        long            dramaticEvent ; // what happened to him
                                        // negative # is config change

        int             missileOwner ;  // # of object that shot missile,
                                        //      dropped bomb, etc that hit this
                                        //      object

        int             missileType ;   //

        int             time ;          // time of dramatic event

        int             damageLevel ;   // 0-7  (see object above

        } _logObject ;


#ifdef _include_data
    struct _logObject_ gamelog[NLOG];
#else
extern struct _logObject_ gamelog[];
#endif
#endif



// "what" values
#define ENDOFLOG     0
#define OBJECTHIT    1
#define RADARHIT     2
#define PLANEHIT     3     /* who = index into p[] (not ps[]!) */
#define BOATHIT      4
#define BRIDGEHIT    5
#define AIRBASEHIT   6
#define HITBYMISSILE 7     /* who = index into s[] (not ps[]!) */
#define TAKINGOFF    8
#define INFLIGHT     9
#define GOODLANDING 10
#define TANKERSTART 11
#define TANKEREND   12
#define EJECTEND    13      // Ohmigod!  Too much excitement, gotta eject!
#define HITGROUND   14
#define CRASHLAND   15
#define BADEJECT    16
#define HITHILL     17
#define PRIMARYDONE 18
#define SECONDDONE  19
#define ENDMISSION  20
#define AAAHIT      21      // who = 1 for A/A, 0 for AAA

#define FRIENDKILL  0x80

