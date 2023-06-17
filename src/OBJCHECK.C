#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>

FILE *tfile;
FILE *planefile;
FILE *radarfile;
FILE *missilefile;
char buf[15];

char name[7];
int speed;
int maneuver;
int rclass;
char filename[13];
int miss1;
int miss2;
int mcount1;
int mcount2;
int radarsig;
int gun;
int alignment;

struct cl {
	int plane_no;
	char name[7];
	int speed;
	int maneuver;
	int rclass;
	char filename[13];
	int radarsig;
	int gun;
	int alignment;
	int ecm;
	int misl[2];
	int mcnt[2];
	int behav;
} planerec;

int recsize = sizeof(planerec);
int i=0;
long recseek=0;

main()
{
	planefile = fopen("F14PD01.DAT","rb");
	missilefile = fopen("F14MD01.DAT","rb");
	radarfile = fopen("F14RD01.DAT","rb");

	if (planefile==NULL) {
		printf("Error:  Plane Data File (F14PD01.TXT) Not Found\n");
		exit(-1);
	}

	if (missilefile==NULL) {
		printf("Error:  Missile Data File (F14MD01.TXT) Not Found\n");
		exit(-1);
	}

	if (radarfile==NULL) {
		printf("Error:  Radar Data File (F14RD01.TXT) Not Found\n");
		exit(-1);
	}

	while (!feof(planefile)) {
		recseek = (long)(i * recsize);
		fseek(planefile,recseek,SEEK_SET);
		fread(&planerec,recsize,1,planefile);
		i++;
		printf("%d %s ",planerec.plane_no,planerec.name);
		tfile = fopen(planerec.filename,"rb");
		if (tfile == NULL) {
			printf("NO 3D OBJECT FOUND\n");
		} else {
			printf("%s\n",planerec.filename);
			fclose(tfile);
		}
	}

	fclose(planefile);
	fclose(missilefile);
	fclose(radarfile);
}
