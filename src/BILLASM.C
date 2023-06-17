//************************************************************************
//*
//*    BILLASM.C
//*
//*    Inline assembly routines since Andy refuses to switch to C6.0
//*
//*    Author: W. Becker
//*
//*    F-15 III
//*    Microprose Software, Inc.
//*    180 Lakefront Drive
//*    Hunt Valley, Maryland  21030
//*
//*************************************************************************

extern   char  MOUSE;
extern   int   MouseX, MouseY;

rotate_pt(int *s, int *d, int ang)
{
	int   SinB(),CosB();
	int   x, y;

	x = s[0];
	y = s[1];

	_asm {
      mov   ax,ang
      neg   ax
      push  ax
      call  CosB        ; CosB(OurRoll)
      mov   si,ax
      call  SinB
      add   sp,2        ; SinB(OurRoll)
      mov   di,ax

      mov   bx,s
      imul  y
      shl   ax,1
      rcl   dx,1
      mov   cx,dx       ; y*SinB(OurRoll)

      mov   ax,si
      imul  x
      shl   ax,1
      rcl   dx,1        ; x*CosB(ourRoll)
      sub   dx,cx

      mov   bx,d
      mov   [bx+0],dx   ; dx = x'

      mov   ax,si
      imul  y
      shl   ax,1
      rcl   dx,1
      mov   cx,dx       ; y*CosB(OurRoll)

      mov   ax,di
      imul  x
      shl   ax,1
      rcl   dx,1        ; x*SinB(OurRoll)
      add   dx,cx
      mov   [bx+2],dx   ; dx = y'

   }
}

//***************************************************************************
MousePOS(int x, int y)
{
   if (!MOUSE) return 0;

   _asm {
      mov   ax,4
      mov   cx,x
      shl   cx,1
      mov   MouseX,cx
      mov   dx,y
      mov   MouseY,dx
      int   33h         ; set mouse cursor position
   }
}

