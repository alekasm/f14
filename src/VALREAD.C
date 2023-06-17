#include <stdlib.h>
#include <stdio.h>
#include <bios.h>


int offset;

int     KbSense;
unsigned int Joystick;
unsigned int ThrustStick;
unsigned int CoolyStick;
unsigned int RudderStick;
unsigned int MouseSprite;
int	    GRNDDETAIL;
int	    PLANEDETAIL;
int     SKYDETAIL;
int 	MISSILETRAIL;
int 	MSGDETAIL;
int 	SNDDETAIL;
int 	RudderType;
int     StickType;
unsigned char  DeadZone;
unsigned char  JoyVals[52];


main()
{
	GetJoyConfig();
	PrintJoyConfig();
}



//***************************************************************************
int GetJoyConfig()
{

	FILE *f ;
	int offset;

	f = fopen ("JOY.F14", "rb") ;

    fread(&offset,			sizeof (int), 1, f);
    fread(&KbSense,			sizeof (int), 1, f);
    fread(&SNDDETAIL,		sizeof (int), 1, f);
    fread(&MSGDETAIL,		sizeof (int), 1, f);
    fread(&SKYDETAIL,		sizeof (int), 1, f);
    fread(&GRNDDETAIL,		sizeof (int), 1, f);
    fread(&PLANEDETAIL,		sizeof (int), 1, f);
    fread(&MISSILETRAIL,	sizeof (int), 1, f);

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
PrintJoyConfig()
{
	int i;
	printf("Offset: %d\n",offset);
	printf("Keyboard Sens: %d\n",KbSense);
	printf("Sound Detail: %d\n",SNDDETAIL);
	printf("Message Detail: %d\n",MSGDETAIL);
	printf("Sky Detail: %d\n",SKYDETAIL);
	printf("Ground Detail: %d\n",GRNDDETAIL);
	printf("Plane Detail: %d\n",PLANEDETAIL);
	printf("Missile Detail: %d\n\n",MISSILETRAIL);
	printf("StickType: %d\n",StickType);
	printf("RudderType: %d\n",RudderType);
	printf("Joystick: %d\n",Joystick);
	printf("ThrustStick: %d\n",ThrustStick);
	printf("CoolyStick: %d\n",CoolyStick);
	printf("RudderStick: %d\n",RudderStick);
	printf("JoyVals[");
	for (i=0; i<52; i++)
		printf("%i",JoyVals[i]);
	printf("]\n");
	printf("DeadZone: %d\n",DeadZone);
}



