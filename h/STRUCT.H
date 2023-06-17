PAGE 79,124
;************************************************************************
;                                                                       *
;       Project: Stealth Fighter(TM)                                    *
;                                                                       *
;<t>    Structure Definations                                           *
;                                                                       *
;       Author: Jim Synoski                                             *
;       Written: Feb 1988                                               *
;       Last Editted: Feb 12,1988                                       *
;                                                                       *
;       Copyright (C) 1988 by MicroProse Software, All Rights Reserved. *
;                                                                       *
;***********************************************************************/


;
;-----------------

;  RasPort structure for text printing

RasPort STRUC
PageNr  dw      0
XBgn    dw      0
YBgn    dw      0
Width1  dw      0
Length1 dw      0
DrawMode dw     0
APen    dw      0
BPen    dw      0
FontNr  dw      0
RasPort ENDS

JAM1    equ     0
JAM3    equ     2

; Sprite structure

SpriteDef   STRUC 
SrcScrnBase dw 0      ; Pointer to sprite graphics data
SrcX     dw    0
SrcY     dw    0
DestScrnBase dw 0
DestX    dw    0
DestY    dw    0
PixWide  dw    0      ; How many pixels wide in X dirn
PixHgt   dw    0      ; How many pixels wide in Y dirn
ClipSTop  dw    0  
ClipSBot  dw    0  
ClipSLft  dw    0   
ClipSRgt  dw    0  
NoOfColors db  0  
Color1   db    0      ; Color used if only a one color sprite
Xscale   dw    0
Yscale   dw    0
SpriteDef ENDS


BLACK      equ   0
BLUE       equ   1
GREEN      equ   2
CYAN       equ   3
RED        equ   4
MAGENTA    equ   5
BROWN      equ   6
WHITE      equ   7
DARK_GRAY  equ   8
LT_BLUE    equ   9
LT_GREEN   equ  0ah
LT_CYAN    equ  0bh
LT_RED     equ  0ch
LT_MAGENTA equ  0dh
YELLOW     equ  0eh
LT_WHITE   equ  0fh


