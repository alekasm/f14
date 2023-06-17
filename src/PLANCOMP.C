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
char filename[13];
int miss1;
int miss2;
int miss3;
int mcount1;
int mcount2;
int mcount3;
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
	int misl[3];
	int mcnt[3];
	int behav;
} planerec;

int recsize = sizeof(planerec);
int flstat;
int planecnt=0;
int i;
long recseek=0;
int planeno;
int behav;
int ecm;
int status;

main()
{
	tfile = fopen("F14PD01.TXT","r");

	printf("\nFleet Defender - Plane Data File Compiler\n");
	printf("\nCompiling F14PD01.TXT...");
	if (tfile == NULL) {
		printf("\nError:  Cannot find file F14PD01.TXT\n");
		exit(-1);
	}

	outfile = fopen("F14PD01.DAT","wb");


	while ((flstat = fscanf(tfile,"%d %s %d %d %d %s %d %d %d %d %d %d %d %d %d %d %d\n",&planeno,name,&speed,
		&maneuver,&rclass,filename,&radarsig,&gun,&alignment,&ecm,&miss1,&miss2,&miss3,&mcount1,&mcount2,&mcount3,&behav)) != EOF) {
				planerec.plane_no = planeno;
				strcpy(planerec.name,name);
				planerec.speed = speed;
				planerec.maneuver = maneuver;
				planerec.rclass = rclass;
				strcpy(planerec.filename,filename);
				planerec.radarsig = radarsig;
				planerec.gun = gun;
				planerec.alignment = alignment;
				planerec.ecm = ecm;
	   			planerec.misl[0] = miss1;
				planerec.misl[1] = miss2;
				planerec.misl[2] = miss3;
				planerec.mcnt[0] = mcount1;
				planerec.mcnt[1] = mcount2;
				planerec.mcnt[2] = mcount3;
				planerec.behav = behav;
				fwrite(&planerec,recsize,1,outfile);
				planecnt++;
		}

	fclose(tfile);
	fclose(outfile);
	printf("Compile Complete.\n");

	outfile = fopen("F14PD01.DAT","rb");

	for (i=0; i<planecnt; i++) {
		recseek = (long)(i * recsize);
		fseek(outfile,recseek,SEEK_SET);
		fread(&planerec,recsize,1,outfile);

		printf("%-3d %-8s %-4d %-3d %-3d %-12s %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d\n",
			planerec.plane_no,planerec.name,planerec.speed,
			planerec.maneuver,planerec.rclass,planerec.filename,
			planerec.radarsig,planerec.gun,planerec.alignment,planerec.ecm,
			planerec.misl[0],planerec.misl[1],planerec.misl[2],planerec.mcnt[0],
			planerec.mcnt[1],planerec.mcnt[2],planerec.behav);
	}
	fclose(outfile);
}
