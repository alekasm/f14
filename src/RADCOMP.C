#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>

FILE *tfile;
FILE *outfile;
char buf[15];

int RadarEnumId;
char name[10];
int range;
int detectiontype;
int missiletype;
int missilecount;
int jam;
int tewsnum;

struct cl {
	int RadarEnumId;
	char name[10];
	int range;
	int detectiontype;
	int missiletype;
	int missilecount;
	int jam;
	int tewsnum;
} radarrec;

int recsize = sizeof(radarrec);
int flstat;
int planecnt=0;
int i;
long recseek=0;

main()
{
	tfile = fopen("F14RD01.TXT","r");

	printf("\nFleet Defender - Radar Data File Compiler\n");
	printf("\nCompiling F14RD01.TXT...");
	if (tfile == NULL) {
		printf("\nError:  Cannot find file F14RD01.TXT\n");
		exit(-1);
	}

	outfile = fopen("F14RD01.DAT","wb");


	while ((flstat = fscanf(tfile,"%d %s %d %d %d %d %d %d\n",&RadarEnumId,name,&range,
		&detectiontype,&missiletype,&missilecount,&jam,&tewsnum)) != EOF) {
				radarrec.RadarEnumId = RadarEnumId;
				strcpy(radarrec.name,name);
				radarrec.range = range;
				radarrec.detectiontype = detectiontype;
				radarrec.missiletype = missiletype;
				radarrec.missilecount = missilecount;
				radarrec.jam = jam;
				radarrec.tewsnum = tewsnum;
				fwrite(&radarrec,recsize,1,outfile);
				planecnt++;
		}

	fclose(tfile);
	fclose(outfile);

	printf("Compile Complete.\n");

	outfile = fopen("F14RD01.DAT","rb");

	for (i=0; i<planecnt; i++) {
		recseek = (long)(i * recsize);
		fseek(outfile,recseek,SEEK_SET);
		fread(&radarrec,recsize,1,outfile);

		printf("%-3d %-12s %-3d %-3d %-3d %-3d %-3d %-3d\n",
			radarrec.RadarEnumId, radarrec.name,radarrec.range, radarrec.detectiontype,
			radarrec.missiletype,radarrec.missilecount,radarrec.jam,radarrec.tewsnum);
	}
	fclose(outfile);
}
