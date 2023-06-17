#include <dos.h>
#include <stdio.h>

#include "library.h"
#include "world.h"
#include "sdata.h"
#include "Cockpit.h"
#include "f15defs.h"
#include "common.h"
#include "event.h"

extern int      nextlog;

//***************************************************************************
InitLogFile()
{
    FILE    *File;

	File = fopen ("log.f14", "wb");
	fclose(File);
}

//***************************************************************************
DumpLogFile()
{
    FILE    *File;

	File = fopen ("log.f14", "ab");

	fwrite (gamelog, sizeof(struct _logObject_), nextlog, File);
	fclose (File);
	nextlog=0;
}


//***************************************************************************
farfread(void far *B, unsigned int size, unsigned int count, FILE *handle)
{
	unsigned char far *S;
	unsigned int i,j;
	unsigned char C[400];

	S=(char far *)B;
	for(i=0;i<count;i++) {
		fread(C, size, 1, handle);
		for(j=0;j<size;j++) {
			*(S++)=*(C+j);
		}
	}
}
