#include <io.h>
#include <fcntl.h>
#include "stdio.h"
#include "stdlib.h"
#include <Dos.H>

#define  HMAPBANK 0
#define  BMAPBANK 1
#define  SDEFBANK 2
#define  STMPBANK 3
#define  TILEBANK 11
#define  NUMBANKS 19

extern	char Txt[];

extern	int	GRPAGE0[];




struct	OBJ3D {
	int	NUM;
	int	X;
	int	Y;
};
extern   struct OBJ3D far OBJECTS[96][16];
extern   int   IdentInvMat[];
extern   int   GroundObjectScale[200];
extern   char  GroundObjectNames[32][20];
extern   int   LandObjectBase;



struct emsfile{
   char name[13];
   int   Page;
   int   Seg;
   unsigned int paras;
   };
struct emsfile far emsfiles[513];


int   far emspageuse[513];

char far *fstrcpy( char far *D, char far *S)
{
   while ( (*(D++)=*(S++))!=0 );
   return(D);
}

fstrcmp(char far *D, char far *S)
{
   char str[200];
   char str2[200];

   fstrcpy( str, D);
   fstrcpy( str2, S);
   return( strcmp(str,str2) );
}

unsigned int NEMSSEG=0;
unsigned int EMSHANDLE=0;
int   EMSPagesFree=0;
int   EMSPagesGrabbed=0;

int   NOTEXTURE=0;

//***************************************************************************
NewEmsFunction()
{
   int   free;

	if (EMMThere()) {
      if (NEMSSEG=EMSPageFrame()) {
         EMSPagesFree=EMSPagesLeft();
         EMSPagesGrabbed= 100+(NUMBANKS*4);     // over 2 meg !!!
         if (EMSPagesGrabbed>EMSPagesFree)
            EMSPagesGrabbed=EMSPagesFree;
         EMSHANDLE=EMSAllocatePages( EMSPagesGrabbed );
         EMSPagesGrabbed-= (4*NUMBANKS);

	  }
	}
}

//***************************************************************************
LoadTheWorld(char *name )
{
	char  PBlock[3*256];
	int	DHNDL;
	unsigned 	len;
	char far *PTR;
	int   i;

	FP_OFF(PTR)=0;

	if( _dos_open( name,O_RDONLY,&DHNDL)==0) {
		_dos_read(DHNDL,PBlock,3*256,&len);
		for (i=0;i<NUMBANKS;i++) {
			FP_SEG(PTR)=MapInBank(i);
			_dos_read(DHNDL,PTR,0x8000,&len);
			_dos_read(DHNDL,PTR+0x8000,0x8000,&len);
		}

      _dos_read(DHNDL,OBJECTS,sizeof(OBJECTS),&len);
      _dos_read(DHNDL,GroundObjectScale,sizeof(GroundObjectScale),&len);
      _dos_read(DHNDL,GroundObjectNames,sizeof(GroundObjectNames),&len);

      _dos_close(DHNDL);
	}
}

LoadGroundObjects()
{
   int   i;

   LandObjectBase=200;

   for (i=0;i<32;i++)
      {
      if (GroundObjectNames[i][0]==0) return;
	LoadEMSObject(GroundObjectNames[i],i+LandObjectBase);
	  }
}

//***************************************************************************
GetEmsMem( int paras, int *page, int *seg )
{
   int   i,j,size;
   int   lstpg;

// entry 0 is never used !!!!!!

   lstpg=paras/1024;

   for (i=1;i<EMSPagesGrabbed;i++)
      {
      if ( ((emspageuse[i]==0)&&((i+lstpg-1)<EMSPagesGrabbed))
                  || ((emspageuse[i]+paras)<=1024)  )
         {
         *(page)=i+(NUMBANKS*4);
         *(seg)=emspageuse[i];
         size=emspageuse[i]+paras;
         for (j=i;(j<EMSPagesGrabbed)&&(size>0);j++,size-=1024)
            {
            emspageuse[j]= (size>1024)? 1024:size;
            }
         return(i);
         }
      }
   return(0);
}

ResizeEmsMem(int i,int oldparas, int paras)
{
   int   size,j;

   for (j=(i+1);oldparas>1024;j++,oldparas-=1024)
      {
      emspageuse[j]=0;
      }
   emspageuse[i]-= oldparas;
   size=emspageuse[i]+paras;
   for (j=i;(j<EMSPagesGrabbed)&&(size>0);j++,size-=1024)
      {
      emspageuse[j]= (size>1024)? 1024:size;
      }
   return(i);
}

MapEmsFile( int hndl, int PhysPage)
{
   int   i,lastpara;
   int   pg,pgnum;

   if (hndl<1) return(0);
   i=hndl;

   if (EMSHANDLE)
      {
      lastpara=emsfiles[i].Seg+emsfiles[i].paras;
      pg=emsfiles[i].Page;
      pgnum=1;
      if (lastpara>1024) pgnum++;
      if (lastpara>2048) pgnum++;
      if (lastpara>3072) pgnum++;

      if ((pgnum+PhysPage)>4) pgnum=4-PhysPage;


      EMMMapMultiPage(EMSHANDLE,PhysPage,pgnum,pg );

/*
      EMMMapPage(EMSHANDLE,emsfiles[i].Page,PhysPage);
      if (lastpara>1024)
         EMMMapPage(EMSHANDLE,emsfiles[i].Page+1,PhysPage+1);
      if (lastpara>2048)
         EMMMapPage(EMSHANDLE,emsfiles[i].Page+2,PhysPage+2);
      if (lastpara>3072)
         EMMMapPage(EMSHANDLE,emsfiles[i].Page+3,PhysPage+3);
*/

      return(emsfiles[i].Seg+NEMSSEG+(1024*PhysPage));
      }
   return(0);
}

MapInBank(int BANK)
{
   int   BNK;

   if (EMSHANDLE)
      {
      BNK=4*BANK;
      EMMMapMultiPage(EMSHANDLE,0,4,BNK );
/*
      EMMMapPage(EMSHANDLE,4*BANK,0);
      EMMMapPage(EMSHANDLE,4*BANK+1,1);
      EMMMapPage(EMSHANDLE,4*BANK+2,2);
      EMMMapPage(EMSHANDLE,4*BANK+3,3);
*/
      return(NEMSSEG);
      }
   return(0);
}


FileInEms(char *Name)
{
   int   i;
   for (i=1;i<513;i++)
      {
      if ( fstrcmp(emsfiles[i].name,Name)==0)
         return(i);
      }
   return(0);

}

GetNextEMSfile()
{
   int   i;
   for (i=1;i<513;i++)
      {
      if (emsfiles[i].name[0]==0) return(i);
      }
   return(0);

}

MakeEMSFileBlock(char *name, unsigned paras)
{
   int   i,bank,page,seg;

   i=FileInEms(name);
   if (i) return(i);
   if ( (i=GetNextEMSfile()) ==0) return(0);

// get bank of paras
   if ( (bank=GetEmsMem( paras, &page, &seg ))==0) return(0);
   emsfiles[i].Page=page;
   emsfiles[i].Seg=seg;
   emsfiles[i].paras=paras;
   fstrcpy( emsfiles[i].name,name);
   return(i);
}

EMSFileUsage()
{
   int   i;

   for (i=1;i<513;i++)
      {
      if (emspageuse[i]==0) break;
      }
   return(i-1);
}






ReadEMSPCX( char *name, int SETPAL)
{
   return( ReadEMSPCXSize( name,SETPAL, 320,200) );
}

ReadEMSPCXSize( char *name, int SETPAL, int XCLP, int YCLP)
{
   int   i,size,page,seg,cnt,bank;
   char far *DST;
   long t;

   i=FileInEms(name);
   if (i) return(i);
   if ( (i=GetNextEMSfile()) ==0) return(0);


   t= XCLP;
   t*=YCLP;
   t+=15;
   t>>=4;
   size= (int)t;

// get bank of size
   if ( (bank=GetEmsMem( size, &page, &seg ))==0) return(0);
   emsfiles[i].Page=page;
   emsfiles[i].Seg=seg;
   emsfiles[i].paras=size;
// map it in
   FP_SEG(DST)=MapEmsFile(i,0);
   FP_OFF(DST)=0;
// read pcx to bank
   cnt=ReadPCXSize( name, DST, SETPAL, XCLP, YCLP);
// set the name
   fstrcpy( emsfiles[i].name,name);
// resize the bank if necessary
   if (size>cnt) {
      ResizeEmsMem(bank,size,cnt);
      emsfiles[i].paras=cnt;
   }
// return the ems file index
   return(i);
}






LoadEmsFile(char *Name)
{
	FILE	*stream;
	char	string[10];
	unsigned int	paras;
	unsigned long	fsize;
	unsigned long	psize;
	unsigned	loadseg;
	char	far *tload;
	char	ch[1];
	int	handle;
	void far * B;

   int   Page,Seg;

   int   i,bank;

   if ( (i=FileInEms(Name)) ) return(i);
   if ( (i=GetNextEMSfile()) ==0) return(0);

//open file and return if open problem
	if ( (stream=fopen(Name,"r+b")) ==0 ) return(0);
//get the file size
	fsize=filelength(fileno(stream));
  	fclose( stream);
   if (fsize==-1) return(0);
	psize=(fsize+15)>>4;
   paras=psize;

   if ( (bank=GetEmsMem( paras, &Page, &Seg ))==0 ) return(0);
   emsfiles[i].Page=Page;
   emsfiles[i].Seg=Seg;
   emsfiles[i].paras= paras;
   fstrcpy(emsfiles[i].name,Name);

   if ( (FP_SEG(B)=MapEmsFile( i, 0))==0 ) return(0);
	FP_OFF(B)=0;
	_dos_open(Name,0,&handle);
	_dos_read(handle,B,(int)fsize,(unsigned int *)&fsize);
	_dos_close(handle);
   return(i);

}



struct Zsheader
	{    char Mfg;           /* ID for ZSoft files, 0A hex */
        char Ver;           /* Version, 5 */
        char Encode;        /* Encoded pic data if 1 */
        char Bpp;           /* Bits/pixel */
        int  X1;            /* Left col */
        int  Y1;            /* Top row */
        int  X2;            /* Right col */
        int  Y2;            /* Bottom row */
        int  Xres;          /* Horiz size of screen */
        int  Yres;          /* Vert size of screen */
        char zpal[48];      /* 3 bytes/palette reg */
        char Vmode;         /* Video mode, ignored */
        char Planes;        /* # color planes in pic */
        int  Bpl;           /* Bytes/line in pic */
			char pad[60];
}ZHEAD={
	0xa,5,1,8,0,0,640,480,640,480,
		0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
		0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
	0,1,640

};
ReadPCX( char *name, char far *DST, int SETPAL)
{
   return(ReadPCXSize( name, DST, SETPAL, 320, 200 ));
}


#if 1
ReadPCXSize( char *name, char far *DST, int SETPAL, int XCLP, int YCLP)
{
	FILE	*Handle;
	int	i,j;

	char Block[3*256];

	char	S[80];

	long	sz;

	unsigned int last,cnt;
   long  numread;

	unsigned int k,m;
	unsigned int d;
	unsigned int vs,hs;



		if( (Handle=fopen(name,"rb"))!=NULL)
			{



			fread(&ZHEAD, sizeof(ZHEAD), 1, Handle );
			hs=ZHEAD.X2+1;
			vs=ZHEAD.Y2+1;


			last= 0x8000;
			cnt=numread=0;

			for (i=0;i<vs;i++)
				{
				for (k=0;k<hs;k++)
					{
					if (cnt==0)
						{
                  cnt= getc(Handle);
//						fread(&cnt, sizeof(char), 1, Handle );

						d=(cnt);
						if ((cnt&0xc0)==0xc0)
							{
							cnt&=63;
                     d= getc(Handle);
//							fread(&d, sizeof(char), 1, Handle );
							}
						else
							{
							cnt=1;
							}
						}

               if ( (k<XCLP)&&(i<YCLP))
                  {
                  *(DST++)=d;
                  numread++;
                  }
					cnt--;
					}
            for (;(k<XCLP)&&(i<YCLP);k++)
               {
               *(DST++)=0;
               numread++;
               }
				}

			fread(&d, sizeof(char), 1, Handle );
			if ( ((d&0xff)==12) && SETPAL )
				{
				fread(Block, sizeof(Block), 1, Handle );
				for (i=0;i<3*256;i++)
					Block[i]>>=2;
//				SetPallet( Block );
				}
			fclose( Handle );
			}
      return((int)(numread>>4));

}

#endif


#if 0
ReadPCXSize( char *name, char far *DST, int SETPAL, int XCLP, int YCLP)
{
	FILE	*Handle;
	int	i,j;

	char Block[3*256];

	char	S[80];

	long	sz;

	unsigned int last,cnt;
   long  numread;

	unsigned int k,m;
	unsigned int d;
	unsigned int vs,hs;



		if( (Handle=fopen(name,"rb"))!=NULL)
			{



			fread(&ZHEAD, sizeof(ZHEAD), 1, Handle );
			hs=ZHEAD.X2+1;
			vs=ZHEAD.Y2+1;


			last= 0x8000;
			cnt=numread=0;

			for (i=0;i<vs;i++)
				{
				for (k=0;k<hs;k++)
					{
					if (cnt==0)
						{
						fread(&cnt, sizeof(char), 1, Handle );

						d=(cnt);
						if ((cnt&0xc0)==0xc0)
							{
							cnt&=63;
							fread(&d, sizeof(char), 1, Handle );
							}
						else
							{
							cnt=1;
							}
						}

               if ( (k<XCLP)&&(i<YCLP))
                  {
                  *(DST++)=d;
                  numread++;
                  }
					cnt--;
					}
            for (;(k<XCLP)&&(i<YCLP);k++)
               {
               *(DST++)=0;
               numread++;
               }
				}

			fread(&d, sizeof(char), 1, Handle );
			if ( ((d&0xff)==12) && SETPAL )
				{
				fread(Block, sizeof(Block), 1, Handle );
				for (i=0;i<3*256;i++)
					Block[i]>>=2;
//				SetPallet( Block );
				}
			fclose( Handle );
			}
      return((int)(numread>>4));

}
#endif
#if 0

WritePCX(char *name,char far *map,int hs,int vs )
{
	FILE	*Handle;
	int	i,j;


	char	S[80];
	char Block[3*256];


	unsigned int last,cnt;

	unsigned int k,m;
	unsigned int d;

	if( (Handle=fopen(name,"wb"))!=NULL)
		{


	   ZHEAD.X2=hs-1;
	   ZHEAD.Xres= ZHEAD.Bpl= hs;
	   ZHEAD.Y2=vs-1;
	   ZHEAD.Yres= vs;


	   fwrite(&ZHEAD, sizeof(ZHEAD), 1, Handle );


	   last= *(map);


	   cnt=0xc1;
	   for (i=0;i<vs;i++)
		   {
		   for (k=0;k<hs;k++)
			   {
			   d= *(map++);
			   fwrite(&cnt, sizeof(char), 1, Handle );
			   fwrite(&d, sizeof(char), 1, Handle );
			   }
		   }


      GetPallet(Block);


	   for (i=0;i<256;i++)
		   {
		   Block[3*i] <<= 2;
		   Block[3*i+1] <<= 2;
		   Block[3*i+2] <<= 2;
		   }

	   d=12;
	   fwrite(&d, sizeof(char), 1, Handle );
	   fwrite(Block, sizeof(Block), 1, Handle );

	   for (i=0;i<256;i++)
		   {
		   Block[3*i] >>= 2;
		   Block[3*i+1] >>= 2;
		   Block[3*i+2] >>= 2;
		   }


	   fclose( Handle );
   }


}

#endif

GetPallet( Block )
	unsigned char	*Block;
{
	char	far *P;
	union	REGS	R;
	struct SREGS SR;

	P= Block;
	SR.es= FP_SEG(P);
	R.x.ax= 0x1017;
	R.x.bx=0;
	R.x.cx=256;
	R.x.dx=FP_OFF(P);
	int86x(0x10, &R, &R, &SR );
}
SetPallet( Block )
	unsigned char	*Block;
{
	char	far *P;
	union	REGS	R;
	struct SREGS SR;

	P= Block;
	SR.es= FP_SEG(P);
	R.x.ax= 0x1012;
	R.x.bx=0;
	R.x.cx=256;
	R.x.dx=FP_OFF(P);
	int86x(0x10, &R, &R, &SR );
}

