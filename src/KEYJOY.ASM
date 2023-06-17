PAGE 79, 124
;************************************************************************
;									*
;	F19: Keyboard JOYSTICK						*
;									*
;<t>	Simulate a JOYSTICK using the keyboard cursor keys.		*
;									*
;  File:  KeyJoy.asm							*
;									*
;  Auth:  David McKibbin						*
;									*
;  Edit:  dtm   August 7, 1992       11:02 am							*
;  This file contains the lower level routines for JOYSTICK control	*
;									*
;	Routines in this file:						*
;									*
;		InstallKeyJoy()						*
;		RemoveKeyJoy()						*
;									*
;   Copyright (C) 1988 by MicroProse Software, All Rights Reserved.	*
;									*
;************************************************************************


	TITLE	KeyJoy.asm  --  Simulated JOYSTICK using KeyBoard


	.MODEL	MEDIUM, C

;
;----------	Parameters	----------
;

DOUBLE_CLICK	equ	5	; double-click threshold (ticks)
JOY_NULL	equ	80h	; X & Y values for CENTER / CLR
JOY_MID		equ	127; 90
JOY_MAX		equ	127	; X & Y values for Maximum DOWN / RIGHT
PAGE
;
;----------	BIOS interface	----------
;

BIOS_DATA	SEGMENT	at 0040h

		ORG	0017h
KbdStat1	db	?
KbdStat2	db	?

KbdAltX		db	?

KbdHead		dw	?
KbdTail		dw	?
KbdBuff		dw	16 DUP (?)

		ORG	006Ch
CLKtic		dw	?,?

		ORG	0080h
KbdStart	dw	?
KbdEnd		dw	?


BIOS_DATA	ENDS

INS_LOCK	equ	80h		; Bit definitions for KbdStat1
CAPS_LOCK	equ	40h
NUM_LOCK	equ	20h
SCRO_LOCK	equ	10h
ALT_KEY		equ	08h
CTRL_KEY	equ	04h
L_SHFT_KEY	equ	02h
R_SHFT_KEY	equ	01h

INS_KEY		equ	80h		; Bit definitions for KbdStat2
CAPS_KEY	equ	40h
NUM_KEY		equ	20h
SCRO_KEY	equ	10h
PAUS_LOCK	equ	08h
SYS_KEY		equ	04h
L_ALT_KEY	equ	02h
R_ALT_KEY	equ	01h

;
;----------	Hardware Equates	----------
;

PIC		equ	20h		; port address - 8259A
EOI		equ	20h		; End-of-Interrupt

KBD_DATA	equ	60h		; port address - KBD scan code data
KBD_STAT	equ	61h		; port address - KBD status/control

KEY_CLEAR	equ	80h		; resets current SCAN code @ KBD

KEY_INT		equ	09h		; Vector# for raw KEYBOARD interrupts
CLK_INT		equ	1Ch		; Vector# for BIOS User Clock interface

PAGE 
;
;----------	Bit defs for KeyBoard JoyStick	   ----------
;

UPP		equ	01h
DWN		equ	02h
LFT		equ	04h
RGT		equ	08h

;;U_L		equ	UPP or LFT	; NumPad "Home"
;;U_R		equ	UPP or RGT	; NumPad "PgUp"
;;D_L		equ	DWN or LFT	; NumPad "End"
;;D_R		equ	DWN or RGT	; NumPad "PgDn"

U_L		equ	0
U_R		equ	0
D_L		equ	0
D_R		equ	0

CLR		equ	80h		; reset the stick - NumPad "5"


;
;----------	Variables & Constant Data Tables	----------
;
	.DATA

	PUBLIC	KeyJoyX, KeyJoyY

KeyJoyX		db	?		; current KeyBoard Joystick "X" value
KeyJoyY		db	?		; current KeyBoard Joystick "Y" value

JoyKey		db	?		; cursor image: Right | Left | Down | Up
KeyTic		dw	?		; clock tic @ last key PRESS

LastKey		db	?		; Last Int_9 scan code
LastHit		db	?		; Last ACTIVE cursor key
KeySkip		db	?		; # of key scans to skip (E0 or E1 sequences)

ScanTable	db	UPP		; Scan Code 29 -- "Up arrow" (Tandy)
		db	  0		; Scan Code 2A
		db	LFT		; Scan Code 2B -- "Left arrow" (Tandy)

		db	47h-2Ch DUP (0)

		db	U_L		; Scan Code 47 -- "Home"
		db	UPP		; Scan Code 48 -- "Up arrow"
		db	U_R		; Scan Code 49 -- "PgUp"
		db	DWN		; Scan Code 4A -- "Down arrow" (Tandy)
		db	LFT		; Scan Code 4B -- "Left arrow"
		db	CLR		; Scan Code 4C -- "5"
		db	RGT		; Scan Code 4D -- "Right arrow"
		db	RGT		; Scan Code 4E -- "Right arrow" (Tandy)
		db	D_L		; Scan Code 4F -- "End"
		db	DWN		; Scan Code 50 -- "Down arrow"
		db	D_R		; Scan Code 51 -- "PgDn"
;
;----------
;
	.CODE
PAGE
;
;   InstallKeyJoy()  --  Install interrupt vectors for Keyboard Joystick
;
InstallKeyJoy	PROC	USES ds

	ASSUME	DS:BIOS_DATA
		mov	ax, BIOS_DATA
		mov	ds, ax

;		and	KbdStat1, not NUM_LOCK

		xor	ax, ax
		mov	JoyKey, al
		mov	KeyTic, ax
		mov	LastKey, al
		mov	LastHit, al
		mov	KeySkip, al
		mov	KeyJoyX, JOY_NULL	; Reset JoyStick to CENTER
		mov	KeyJoyY, JOY_NULL

		xor	ax, ax			; DS = 0000 (Interrupt Vector Page)
		mov	ds, ax

		mov	bx, KEY_INT*4		; Save old Keyboard Vector
		mov	ax, [bx+0]
		mov	dx, [bx+2]
		mov	WORD PTR cs:oldint9+0, ax
		mov	WORD PTR cs:oldint9+2, dx

		mov	ax, OFFSET KBDint
		mov	dx, cs

		cli				; Install my Keyboard Vector
		mov	[bx+0], ax
		mov	[bx+2], dx
		sti

		ret

InstallKeyJoy	ENDP


;
;   RemoveKeyJoy  --  Remove interrupt vectors for Keyboard Joystick
;
RemoveKeyJoy	PROC	USES	ds

		xor	ax, ax			; DS = 0000 (Interrupt Vector Page)
		mov	ds, ax

		mov	bx, KEY_INT*4
		mov	ax, WORD PTR cs:oldint9+0
		mov	dx, WORD PTR cs:oldint9+2

		cli				; Restore Keyboard Vector
		mov	[bx+0], ax
		mov	[bx+2], dx
		sti

		ret

RemoveKeyJoy	ENDP

PAGE
;
;   KBDint  --  New KEYBOARD interrupt routine
;
KBDint:		sti
		pushf
		push	ds
		push	es
		push	ax
		push	bx  

	ASSUME	DS:DGROUP
		mov	ax, DGROUP
		mov	ds, ax

	ASSUME	ES:BIOS_DATA
		mov	ax, BIOS_DATA
		mov	es, ax

		mov	ah, KeySkip
		or	ah, ah
		jz	GetScan

SetSkips:	dec	ah
		mov	KeySkip, ah
		jmp	ByPass

GetScan:	in	al, KBD_DATA		; get the SCAN CODE

		cmp	LastKey, 0E0h		; Is this an escaped code ?
		mov	LastKey, al
		je	PassIt			; Yes, no NUM_LOCK checks

		cmp	al, 0E0h		; E0 sequences are 2 long
		mov	ah, 1
		je	SetSkips

		cmp	al, 0E1h		; E1 sequences are 3 long
		mov	ah, 3
		je	SetSkips

		xor	ah, ah			; AH = boolean accumulator
		test	KbdStat1, NUM_LOCK
		jz	@F
		xor	ah, 1
@@:		test	KbdStat1, L_SHFT_KEY or R_SHFT_KEY
		jz	@F
		xor	ah, 1
@@:		or	ah, ah
		jz	PassIt
		jmp	ByPass			; Skip if NUM_LOCK xor SHIFTs

PassIt:		mov	ah, al			; AH = SCAN CODE
		and	al, 7fh			; mask off pressed/released bit

		cmp	al, 51h
		ja	ByPass
		sub	al, 29h
		jb	ByPass

		mov	bx, OFFSET ScanTable
		xlatb				; AL = translated code
		or	al, al
		jz	ByPass
PAGE
		test	ah, 80h			; Upper bit of SCAN code is set
		jnz	KeyRelease		; 	for key released.

KeyPress:	cmp	JoyKey, 0
		jne	ByPass			; Only 1 key at a time !!!

		mov	JoyKey, al

		cmp	LastHit, al		; Same KEY as last time ?
		mov	LastHit, al
		jne	NewKey

		mov	bx, CLKtic		; 3600 secs in LSW is good enuf
		sub	bx, KeyTic
		cmp	bx, DOUBLE_CLICK

		mov	bh, JOY_MAX		; Got a Double Click !!!
		jb	@F

NewKey:		mov	bh, JOY_MID		; Got a Single Click !!!
		  	
@@:		mov	bl, JOY_NULL
		sub	bl, bh
		add	bh, JOY_NULL

		test	al, UPP
		jz	@F
		mov	KeyJoyY, bl

@@:		test	al, DWN
		jz	@F
		mov	KeyJoyY, bh

@@:		test	al, LFT
		jz	@F
		mov	KeyJoyX, bl

@@:		test	al, RGT
		jz	@F
		mov	KeyJoyX, bh

@@:		mov	bx, CLKtic
		mov	KeyTic, bx
		jmp	SHORT ByPass

KeyRelease:	cmp	JoyKey, al		; Only release if I'm active
		jne	ByPass

		mov	JoyKey, 0

		mov	KeyJoyX, JOY_NULL
		mov	KeyJoyY, JOY_NULL

ByPass:		mov	bx,KbdHead		; Strip out repeated characters
		cmp	bx,KbdTail		; from the KeyBoard buffer
		je	KBDexit

		mov	ax,es:[bx]		; AX = char @ HEAD

Strip:		add	bx,2
		cmp	bx,KbdEnd
		jb	@F
		mov	bx,KbdStart
@@:		cmp	bx,KbdTail
		je	KBDexit			; don't strip last char

		cmp	ax,es:[bx]
		jne	KBDexit

		mov	KbdHead,bx
		jmp	Strip

KBDexit:	pop	bx
		pop	ax
		pop	es
		pop	ds
		popf
		cli

		db	0EAh			; Jump Inter-Segment Direct Opcode
oldint9:	dd	0


	END

