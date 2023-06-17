PAGE 79,124
;************************************************************************
;									*
;  File:  MCGA.asm							*
;									*
;  Auth:  David McKibbin						*
;									*
;  Edit:  dtm   May 6, 1991          12:11 pm				*
;									*
;  This file contains MCGA initialization routines			*
;									*
;	Routines in this file:						*
;									*
;		MCGAcolors (1st_color, num_colors, RGB_array)		*
;									*
;									*
;    Copyright (C) 1990 by MicroProse Software, All Rights Reserved.	*
;									*
;************************************************************************

	TITLE MCGA - MCGA specific routines

	.MODEL	MEDIUM, C

	.CODE	;MPS_CODE


;MCGAcolors	PROC	uses si di ds, color1:WORD, colors:WORD, RGBs:FAR PTR CHAR
MCGAcolors	PROC	uses si di,    color1:WORD, colors:WORD, RGBs:PTR CHAR

	mov	bx,color1	; BL = current color register#
	mov	cx,colors	; CX = color count
;	lds	si,RGBs
	mov	si,RGBs

	xor	ax,ax
	mov	es,ax
	test	byte ptr es:[0489h],02
	jnz	GrayS

W8VB:	mov	dx,03DAh
@@:	in	al,dx		; wait for Vertical Retrace +
	test	al,08
	jz	@B

	mov	bp,0		; zero RGB/VB counter
	jmp	RGB

Next1:	inc	bp
	js	RGB		; if BP<0, then we're past VB & 13xing
	jz	W8VB		; if BP=0, then we've spent our 13x

	mov	dl,0DAh		; if BP>0, then we're in VB & counting
	in	al,dx
	test	al,08
	jz	Do13x		; VB- so stop counting & setup our 13x

RGB:	cli
	mov	dl,0C8h
	mov	al,bl
	inc	bx
	out	dx,al		; select color register
	jmp	$+2
	jmp	$+2

	inc	dx
	lodsb
	out	dx,al
	jmp	$+2
	lodsb
	out	dx,al
	jmp	$+2
	lodsb
	out	dx,al
	sti

	loop	Next1
	ret


Do13x:	mov	ax,bp
	mov	ah,13		; determined by experiment
	mul	ah
	neg	ax
	mov	bp,ax		; BP = -(BP*13)
	jmp	RGB



GrayS:	mov	dx,4CCDh	; 19661. * RED (30%)
	lodsb
	xor	ah,ah
	mul	dx
	mov	di,ax
	mov	bp,dx

	mov	dx,970Ah	; 38666. * GREEN (59%)
	lodsb
	xor	ah,ah
	mul	dx
	add	di,ax
	adc	bp,dx

	mov	dx,1C29h	; 7209. * BLUE (11%)
	lodsb
	xor	ah,ah
	mul	dx
	add	ax,di
	adc	dx,bp
	shl	ax,1
	adc	dx,+00
	mov	bh,dl		; BH = summed mono value
	jmp	Grey1

@@:	in	al,dx		; wait for Vertical Retrace +
	test	al,08
	jz	@B

Grey1:	mov	dx,03DAh
	in	al,dx		; check for tail of VB interval
	test	al,01
	jz	@B

	cli
	mov	dl,0C8h
	mov	al,bl
	inc	bx
	out	dx,al		; select color register
	jmp	$+2
	jmp	$+2

	inc	dx
	mov	al,bh
	out	dx,al
	jmp	$+2
	out	dx,al
	jmp	$+2
	out	dx,al
	sti

	loop	GrayS
	ret

MCGAcolors	ENDP


IFDEF	ON/OFF

CRToff:	push	ax		; code from COMPAQ VGA BIOS
	push	dx
	mov	dx,03C4h
	mov	al,01
	out	dx,al
	inc	dx
	in	al,dx
	or	al,20h
	mov	ah,al
	mov	al,01
	dec	dx
	out	dx,ax
	pop	dx
	pop	ax
	ret
	nop


CRTon:	push	ax
	push	dx
	mov	dx,03C4h
	mov	al,01
	out	dx,al
	inc	dx
	in	al,dx
	and	al,0DFh
	mov	ah,al
	mov	al,01
	dec	dx
	out	dx,ax
	pop	dx
	pop	ax
	ret

ENDIF


	END

