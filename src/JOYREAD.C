extern unsigned int   Joystick     ; // 3 when enabled
extern unsigned int   ThrustStick  ; // 8 when enabled
extern unsigned int   CoolyStick   ; // 8 when enabled
extern unsigned int   RudderStick  ; // 4 when enabled

extern char *JoyVals ;  // 56 bytes long
extern char DeadZone ;

int GetJoyConfig (char *name) 
{

	FILE *f ;

	f = fopen (name, "rb") ;

    fread (&Joystick,    sizeof (unsigned), 1, f) ; // 3 when enabled
    fread (&ThrustStick, sizeof (unsigned), 1, f) ; // 8 when enabled
    fread (&CoolyStick,  sizeof (unsigned), 1, f) ; // 8 when enabled
    fread (&RudderStick, sizeof (unsigned), 1, f) ; // 4 when enabled

    fread (JoyVals, sizeof (char), 56, f) ;  // 56 bytes long

    fread (&DeadZone, sizeof (char), 1, f) ;


    fclose (f) ;
}


int SaveJoyConfig (char *name) 
{
	FILE *f ;

    f = fopen (name, "wb") ;

    fwrite (&Joystick,    sizeof (unsigned), 1, f) ; // 3 when enabled
    fwrite (&ThrustStick, sizeof (unsigned), 1, f) ; // 8 when enabled
    fwrite (&CoolyStick,  sizeof (unsigned), 1, f) ; // 8 when enabled
    fwrite (&RudderStick, sizeof (unsigned), 1, f) ; // 4 when enabled
    fwrite (JoyVals, sizeof (char), 56, f) ;  // 56 bytes long
    fwrite (&DeadZone, sizeof (char), 1, f) ;

	fclose (f) ;
}
