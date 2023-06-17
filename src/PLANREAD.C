#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>

FILE *tfile;
FILE *outfile;
char buf[15];

char name[7];
int speed;
int maneuver;
int rclass;
char filename[12];
int miss1;
int miss2;
int mcount1;
int mcount2;

struct cl {
	char name[7];
	int speed;
	int maneuver;
	int rclass;
	char filename[12];
	int misl[2];
	int mcnt[2];
} planerec;

int recsize = sizeof(planerec);

main()
{
	tfile = fopen("F14PD01.TXT","r");
	fscanf(tfile,"%s %d %d %d %s %d %d %d %d\n",name,&speed,&maneuver,
			&rclass,filename,&miss1,&miss2,&mcount1,&mcount2);
	printf("%s %d %d %d %s %d %d %d %d\n",name,speed,maneuver,rclass,
			filename,miss1,miss2,mcount1,mcount2);
	fclose(tfile);

	strcpy(planerec.name,name);
	planerec.speed = speed;
	planerec.maneuver = maneuver;
	planerec.rclass = rclass;
	strcpy(planerec.filename,filename);
	planerec.misl[0] = miss1;
	planerec.misl[1] = miss2;
	planerec.mcnt[0] = mcount1;
	planerec.mcnt[1] = mcount2;

	outfile = fopen("F14PL01.DAT","wb");
	fwrite(&planerec,recsize,1,outfile);
	fclose(outfile);

	outfile = fopen("F14PL01.DAT","rb");
	fread(&planerec,recsize,1,outfile);
	printf("Bin Read: %s %d %d %d %s %d %d %d %d\n",planerec.name,
			planerec.speed,planerec.maneuver,planerec.rclass,planerec.filename,
			planerec.misl[0],planerec.misl[1],planerec.mcnt[0],planerec.mcnt[1]);

	fclose(outfile);
}
