/*
ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
º********************   MPS Labs Graphic Library   *********************º
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
º  File:  Library.h														º
º																		º
º  Auth:  David McKibbin												º
º																		º
º  Edit:  dtm   July 20, 1992         1:18 pm							º
º																		º
º  Note:  HEADER definitions for MPSLIB?.LIB							º
º																		º
ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
º   Copyright (c) 1991 by MicroProse Software, All Rights Reserved.		º
ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/

#ifndef EXEC_TYPES_H
#include "types.h"
#endif


#define  KBD_STAT1	(*(unsigned char far *)0x00000417)
#define  KBD_STAT2	(*(unsigned char far *)0x00000418)
#define  BIOS_TICK	(*(unsigned long far *)0x0000046C)


/***************		CONSTANT defines		***************/

#define	SET		(0 << 8)	/* Line drawing modes, | with color	*/
#define	AND		(1 << 8)
#define	OR		(2 << 8)
#define	XOR		(3 << 8)

#define BLACK	0
#define BLUE	1
#define GREEN	2
#define CYAN	3
#define RED	 	4
#define PURPLE	5
#define BROWN	6
#define LGRAY	7
#define DGRAY	8
#define LBLUE	9
#define LGREEN	10
#define LCYAN	11
#define LRED	12
#define LPURPLE	13
#define YELLOW	14
#define WHITE	15

#define MGRAY	DGRAY		/* aliases */
#define LGREY	LGRAY
#define DGREY	DGRAY
#define MGREY	MGRAY

#define	BITHERCMODE			0x01
#define	BIT85COLOR			0x02
#define	BITNOERASE			0x80

#define MultiplePages		0	/* Multiple graphics pages are supported */
#define AnimationSupport	2	/* Yes animation is supported			 */
#define Dithered85Color		4	/* 320x200 85 color support through		 */
								/*	dithered 640x200 mode.				 */
#define TextIs16Bit			6	/* this is a variable.					 */
#define In85ColorMode		8	/* this is a variable.					 */
#define GraphicPage		   10	/* segment of GraphicPage 0				 */

#define JAM1	   0	/* print foreground only */
#define JAM2	   1	/* print foreground and background */
#define JAM3	   2	/* Special CGA mode for the game */
#define JAM4	   3	/* Special CGA mode for the TrackCam */
#define COMPLEMENT 4	/* XOR the bits */


/***************		pseudo FUNCtion defines		***************/

#define SetDrMd(a,b) a->DrawMode=(b)
#define SetAPen(a,b) a->APen=(b)
#define SetBPen(a,b) a->BPen=(b)
#define GetFont(a,b) a->FontNr
#define SetFont(a,b) a->FontNr=(b)


/***************		STRUCTure defines		***************/

struct RastPort {
	int		PageNr;		/* page number		  */
	int		XBgn;		/* window begin X pos */
	int		YBgn;		/* window begin Y pos */
	int		Width1;		/* window width-1	  */
	int		Length1;	/* window length-1	  */

	int		DrawMode;	/* Jam1, Jam2, ...	  */
	int		APen;		/* foreground color   */
	int		BPen;		/* background color   */

	int		FontNr;		/* Font designator    */
};

typedef struct RastPort RPS;

struct	ClipPort {
	int		x, y;
	int		w, h;
};

struct	STX {				/*								   T   C   B   */
	signed char  x, y;		/* values are signed chars, ie. [-127..0..127] */
	signed char  w, z;		/*								   L   C   R   */
};

struct  Catdir {
	char		filename[12];
	unsigned	time;
	unsigned	date;
	long		size;
	long		offset;
};


/***************		USER/RESIDENT "C" prototypes		***************/

extern	int		     OpenFile (char *file, int attrib);		/* fileio.c */
extern	void	     CloseFile (int fh);
extern	int		     CreateFile (char *file);
extern	int			 GetRpPixel (RPS *Rp, int x, int y);
extern	int		     LoadNearData (char *file, char *buffer);
extern	int		     LoadFarData (char *file, char far *buffer);
extern	int		     StoreFarData (char *file, char far *buffer, int count);
extern	int		     LoadData (char far *buff, char *file);
extern	void	     LoadBrush (int page, int x, int y, char *file, char *palette);
extern	void	     LoadPicture  (int page, char *file);
extern	void	     LoadPicNoPal (int page, char *file);
extern	void	     LoadPicToPal (int page, char *file, char *palette);
extern	void	     LoadPalNoPic (char *file);
extern	void	     LoadPalToPal (char *file, char *palette);
extern	UWORD	     LoadSprite (char *file);
extern	void	     LoadPicGmem (char *file, int page);
extern	void	     LoadPicFmem (char *file, int OutSeg);
extern	void	     DecodePic (int InSeg, int OutSeg);
extern	int		     DecodeData (char far *in, char far *out);
extern	void	     SetFileio (int fh);
extern	int 	     GetASector (int fh);
extern	int		     OpenCat (char *file);
extern	void	     CloseCat (void);
extern	int		     SavePicture (int page, char *file);
extern	int		     SaveSprite  (int page, int x, int y, int w, int h, char *file);
extern	int		     SearchCat (char *file, struct Catdir *dir);

extern	int		     LoadIFFPicture (int page, char *file);	/* iffio.c */
extern	int		     LoadIFFBrush (int page, int x, int y, char *file);
extern	UWORD 	     LoadIFFSprite (char *file);

extern	int		     LoadFont (int font, int index, char *file);	/* fileio.c */
extern	int		     LoadIFFFont (int font, char *file);	/* loadfont.c */


/***************		USER/RESIDENT "ASM" prototypes		***************/

extern	void	far  InitSGF (char *palette);				/* lzwio.asm */
extern	void	far  InitSDF (void);
extern	void	far  ReadSDF (char far *buffer, int count);
extern	void	far  ReadLine  (char *buffer);
extern	void	far  WriteLine (char *buffer);

extern	void	far  Install_VB(void);
extern	void	far  Remove_VB (void);

extern	int		far  MouseON(void);								/* mouse.asm */
extern	int		far  MouseOFF(void);
extern	void	far  MouseTYPE(int HotX, int HotY, int sprite);
extern	void	far  MouseSPOT (int x, int y);
extern	void	far  MouseSHOW(void);
extern	void	far  MouseHIDE(void);
extern	void	far  MouseLOCK(int page);
extern	void	far  MouseFREE(void);
extern	int 	far  MouseHITS(void);
extern	void 	far  MouseUSER(void (far *)());
extern	int	*	far  Mouse(int ax, int bx, int cx, int dx);
extern	volatile int MouseX;
extern	volatile int MouseY;
extern	volatile int Button;

extern	void	far    set_ctrl_b (void);					/* ctrlvect.asm */
extern	void	far  reset_ctrl_b (void);

extern	UWORD	far  LoadOverlay (char *filename, char *dataname);	/* overload.asm */
extern	void	far  InitOverlay (UWORD seg);
extern	void	far  RemoveOverlay (UWORD seg);

extern	void	far  Draw (void);
extern	void	far  FDraw (void);
extern	void	far  FilPgn (void);
extern	void	far  FilBgn (void);
extern	void	far  DrawLine(RPS *Rp, int x1, int y1, int x2, int y2, int color);
extern	void	far  DrawDot(RPS *Rp, int x, int y, int color);
extern	void	far  DrawPoly(RPS *Rp, int pts, int *List, int color);
extern	void	far  RectFill(RPS *Rp, int x, int y, int w, int h, int color);
extern	void	far  Clip();

extern	void	far  BGInit (void);							/* sprites.asm */
extern	void	far  BGRestore (int spage, int dpage);
extern	void	far  ReplaceSprite (RPS *Rp, int x, int y, UWORD SpriteHandle);
extern	void	far  StampSprite (RPS *Rp, int x, int y, UWORD SpriteHandle);

extern	long	far  power (int numb, int exp_times_256);	/* power.asm */
extern	UWORD	far  XYangle (int side_A, int side_B);		/* xyangle.asm */
extern	UWORD	far  isqrt (long square);					/* isqrt.asm */
extern	UWORD	far  distance (int side_A, int side, int thresh);
extern	UWORD	far  hypot2 (int side_A, int side_B);
extern	UWORD	far  hypot3 (int side_A, int side_B, int side_C);

extern	SWORD	far  ClipWd;
extern	SWORD	far  ClipLn;

extern	ULONG		 SizeWrapFPrint();
extern	ULONG		 SizeWrapPrint();


/***************		Graphic Library prototypes		***************/

extern	void	far  AddLine (char *RowBuff, int page, int x, int y, int count);
extern	UWORD	far  AllocGraphicPage (int page);
extern	UWORD	far  AvailSysMem (void);
extern	UWORD	far  BuildSprite (int page, int x, int y, int wide, int high);
extern	void	far  ChangeColor (RPS *Rp, int x, int y, int wide, int high, int from_color, int to_color);
extern	int		far  CharWidth (int font, char ch);
extern	void	far  ClearPage (int page, int color);

extern	struct ClipPort far * far ClipSprite (RPS *Rp, int x, int y, UWORD SpriteHandle);

extern	int		far  CloseSprite (void);
extern	UWORD	far  DeAllocGraphicPage (int page);
extern	void	far  FadePage (int page, int tenths);
extern	char far *	far FontAddress (int font);
extern	int		far  FontHeight (int font);
extern	int		far  GetPixel (int page, int x, int y);
extern	void	far  GetLine (char *RowBuff, int page, int x, int y, int count);
extern	void	far  InstallFont (int font, void *font_data);
extern	int		far  LibCanDo (int what);
extern	int		far  OpenSprite (void);
extern	void	far  PutLine (char *RowBuff, int page, int x, int y, int count);
extern	void	far  RectCopy (RPS *SrcRp, int sx, int sy, int wide, int high, RPS *DstRp, int dx, int dy);
extern	int		far  RpPrint (RPS *Rp, int x, int y, char *String);
extern	struct ClipPort far * far ScaleSprite (RPS *Rp, int x, int y, int wide, int high, UWORD SpriteHandle);
extern	void	far  ScreenOff (void);
extern	void	far  ScreenOn (void);
extern	void	far  SetGraphicPage (int page, UWORD segment);
extern	void	far  SetPalette (int palette, int color);
extern	void	far  SetPalettes (char *color_array);
extern	void	far  SetColors (char *palette);
extern	void	far  Shake (int shakes);
extern	void	far  ShowPage (int page);
extern	void	far  ShowSprite (RPS *Rp, int x, int y, UWORD SpriteHandle);
extern	void	far  SprClose (void);
extern	int		far  SprLine (char *RowBuff);
extern	void	far  SprOpen (int SpWide, int SpHigh);
extern	void	far  SwapPage (int page);
extern	void	far  TurnOnGraphicsMode (int mode);


/***************		MISC Library prototypes		***************/

extern	int		far  IsKey (void);
extern	int		far  GetKey (void);
extern	int		far  EchoGetKey (void);
extern	void	far  FixKeyboard (void);

extern	int		far  Trigger2 (int button);
extern	void	far  InitJoy2 (UWORD mask);
extern	struct  STX  far ReadJoy2 (UWORD mask);


/***************		SOUND Library prototypes		***************/

extern	int		far  SndSysSetup();
extern	int		far  SndSounds();
extern	void	far  SndShutdown();
extern	int		far  GETTIME();
extern	void	far  SndDummy0();
extern	void	far  SndDummy1();
extern	void	far  SndDummy2();
extern	void	far  SndDummy3();
extern	void	far  SndDummy4();
