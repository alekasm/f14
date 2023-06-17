#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>

FILE *tfile;
FILE *outfile;
char buf[15];

int MissileEnumId;
char name[12];
int range;
int minrange;
int maxalt;
int speed;
int homing;
int maneuver;
int cmeffect;
int damage;
char filename[12];
int type;

struct cl {
	int MissileEnumId;
	char name[12];
	int range;
	int minrange;
	int maxalt;
	int speed;
	int homing;
	int maneuver;
	int cmeffect;
	int damage;
	char filename[12];
	int type;
} missilerec;

int recsize = sizeof(missilerec);
int flstat;
int planecnt=0;
int i;
long recseek=0;

main()
{
	tfile = fopen("F14MD01.TXT","r");

	printf("\nFleet Defender - Missile Data File Compiler\n");
	printf("\nCompiling F14MD01.TXT...");
	if (tfile == NULL) {
		printf("\nError:  Cannot find file F14MD01.TXT\n");
		exit(-1);
	}

	outfile = fopen("F14MD01.DAT","wb");


	while ((flstat = fscanf(tfile,"%d %s %d %d %d %d %d %d %d %d %s %d\n",&MissileEnumId,name,&range,
		&minrange,&maxalt,&speed,&homing,&maneuver,&cmeffect,&damage,filename,&type)) != EOF) {
				missilerec.MissileEnumId = MissileEnumId;
				strcpy(missilerec.name,name);
				missilerec.range = range;
				missilerec.minrange = minrange;
				missilerec.maxalt = maxalt;
				missilerec.speed = speed;
				missilerec.homing = homing;
				missilerec.maneuver = maneuver;
				missilerec.cmeffect = cmeffect;
				missilerec.damage = damage;
				strcpy(missilerec.filename,filename);
				missilerec.type = type;
				fwrite(&missilerec,recsize,1,outfile);
				planecnt++;
		}

	fclose(tfile);
	fclose(outfile);

	printf("Compile Complete.");

	outfile = fopen("F14MD01.DAT","rb");

	for (i=0; i<planecnt; i++) {
		recseek = (long)(i * recsize);
		fseek(outfile,recseek,SEEK_SET);
		fread(&missilerec,recsize,1,outfile);

		printf("%-3d %-12s %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-12s %-3d\n",
			missilerec.MissileEnumId, missilerec.name,missilerec.range, missilerec.minrange,
			missilerec.maxalt,missilerec.speed,missilerec.homing,missilerec.maneuver,
			missilerec.cmeffect,missilerec.damage,missilerec.filename,missilerec.type);
	}
	fclose(outfile);
}
