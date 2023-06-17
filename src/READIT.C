#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <dos.h>


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
int mcount1;
int mcount2;

struct cl {
	int plane_no;
	char name[7];
	int speed;
	int maneuver;
	int rclass;
	char filename[13];
	int misl[2];
	int mcnt[2];
	int behav;
	int ecm;
	int status;
};

struct cl planerec;

struct PlaneData {
	int plane_no;
	char name[7];
	int speed;
	int maneuver;
	int rclass;
	int misl[2];
	int mcnt[2];
	int behav;
	int ecm;
	int status;
};


int recsize = sizeof(struct cl);
int flstat;
int planecnt=0;
int i;
long recseek=0;
int planeno;
int behav;

struct PlaneData far *MasterList;
struct PlaneData far *templist;

unsigned newseg;
int PlaneCnt=0;

char far *fstrcpy( char far *D, char far *S);
struct PlaneData far *GetPtr(int num);

unsigned memsize = ((sizeof(struct PlaneData)>>4)+1);
int planecount;
unsigned int maxsize;
unsigned maxseg;
unsigned newmem;

main()
{
	outfile = fopen("F14PD01.DAT","rb");

	_dos_allocmem(memsize,&newseg);
	FP_SEG(MasterList) = newseg;
	FP_OFF(MasterList) = 0;

	while (!feof(outfile)) {
		recseek = (long)(PlaneCnt * recsize);
		fseek(outfile,recseek,SEEK_SET);
		fread(&planerec,recsize,1,outfile);
		MasterList[PlaneCnt].speed = planerec.speed;
		MasterList[PlaneCnt].rclass = planerec.rclass;
		fstrcpy(MasterList[PlaneCnt].name,planerec.name);
		PlaneCnt++;
	}

	fclose(outfile);

	PrintList();
	templist = GetPtr(20);
	printf("\n\nName: %Fs\n",templist->name);
	_dos_freemem(newseg);
}

struct PlaneData far *GetPtr(int num)
{
	return &(MasterList[num]);
}

PrintList()
{
	int i;

	for (i=0; i<(PlaneCnt-1); i++)
		printf("%d %d %Fs\n",MasterList[i].speed,MasterList[i].rclass,
				MasterList[i].name);
}

char far *fstrcpy( char far *D, char far *S)
{
   while ( (*(D++)=*(S++))!=0 );
   return(D);
}


