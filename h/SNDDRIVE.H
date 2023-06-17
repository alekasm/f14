/*
**  SNDDRIVE.H - sound driver function call prototypes
**
**  update: 15.Oct.1993
*/

extern int _far SndSysSetup(
	int BoardAddr,
	int BoardType,
	int BoardIRQ,

	int DigAddr,
	int Digtype,
	int DigIRQ,
	int DigDRQ,
	long sizeLoaded,
	unsigned EMSsegment,
	unsigned EMShandle
);
extern void far SndSounds(int sound, ...);
extern void far SndShutDown(void);
extern void far SndManager(void);
extern void far MakeNoise(void);
extern void unsigned GetTime(void);
