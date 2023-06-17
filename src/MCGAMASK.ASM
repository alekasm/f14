PAGE 79,124
;ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
;º********************   MPS Labs Graphic Overlay   ********************º
;º********************				    ********************º
;º********************        320 x 200 x 256	    ********************º
;ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
;º									º
;º  File:  MCGAmask.asm							º
;º									º
;º  Auth:  David McKibbin						º
;º									º
;º  Edit:  dtm   May 19, 1992          8:39 am				º
;º									º
;º  Call:    MaskCopy  (int far *mask)					º
;º	     BuildMask (int far *mask, int x, int y, int w, int h)	º
;º									º
;º	     mask: int x,y,w,h,SKIP/SIZE...				º
;º									º
;ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
;º   Copyright (c) 1991 by MicroProse Software, All Rights Reserved.	º
;ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

	TITLE	MCGAmask - MCGA masked rectangle copy routines

	.MODEL	MEDIUM, C

	Extrn	LibCanDo:far
        Extrn   FlirTbl:byte


ROWBYTES equ 320

	.CODE

Sseg	dw	0
Dseg	dw	0
Delta   dw      0

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   MaskCopy (int far *mask)						³
;³									³
;³	copy from page-1 to page-0 at&thru the mask			³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
MaskCopy	PROC FAR	USES SI DI DS, MaskSp:dword

        mov     Delta,0
        jmp     Entry2

MaskCopy        ENDP
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   MaskCopySkew (int far *mask, int word)				³
;³									³
;³	copy from page-1 to page-0 at&thru the mask, skewing x		³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
MaskCopySkew	PROC FAR	USES SI DI DS, MaskSp:dword, skew:word

        mov     ax,skew
        mov     Delta,ax

Entry2::mov	ax,10			; GraphicPage[0]
	push	ax
	call	LibCanDo
	add	sp,+2
	mov	Dseg,ax

	mov	ax,12			; GraphicPage[1]
	push	ax
	call	LibCanDo
	add	sp,+2
	mov	Sseg,ax

	lds	bx,MaskSp		; DS:BX -> Mask

	mov	di,[bx+0]		; X-pos
	mov	ax,[bx+2]		; Y-pos
	mov	dx,ROWBYTES
	mul	dx
	add	di,ax
	mov	es,Dseg			; ES:DI -> 1st pixel (dest)

	mov	ax,[bx+4]		; AX = width
	mov	dx,[bx+6]		; DX = height
	or	ax,ax
	jz	NoGo
	or	dx,dx
	jz	NoGo

	add	bx,8
	mov	bp,ds			; BP = FP_SEG(mask)

RowLp:	push	di
ColLp:	mov	ds,bp
	add	di,[bx+0]		; SKIP
	mov	cx,[bx+2]		; SIZE
	add	bx,4
	mov	ax,cx
	and	cx,7FFFh		; strip off EOL sentinel
	jz	new
	mov	ds,Sseg
	mov	si,di
        add     si,Delta                ; For x-skewed copies !!
	test	di,1
	jz	@F
	movsb				; word align destination
	dec	cx
@@:	shr	cx,1
	rep	movsw
	jnc	@F
	movsb				; move tail byte
@@:	or	ax,ax			; look for EOL sentinel
	jns	ColLp

new:	pop	di
	add	di,ROWBYTES
	dec	dx
	jnz	RowLp
NoGo:
	ret

MaskCopySkew	ENDP

PAGE
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³  BuildMask (int far *mask, int x, int y, int w, int h, int pagenr	³
;³									³
;³	return: mask size						³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
BuildMask  PROC FAR	Uses SI DI DS, SpMask:dword, SpX:word, SpY:word, \
				       SpWide:word,  SpHigh:word, PageNr:word

	mov	ax,PageNr
        shl     ax,1
        add     ax,10
	push	ax
	call	LibCanDo
	add	sp,+2
	mov	es,ax			; ES = GraphicPage[PageNr]

	lds	bx,SpMask		; DS:BX -> MASK
	push	bx

	mov	di,SpX			; DI = SpX
	mov	[bx+0],di
	mov	ax,SpY			; AX = SpY
	mov	[bx+2],ax
	mov	si,SpWide		; SI = SpWide
	mov	[bx+4],si
	mov	bp,SpHigh		; BP = SpHigh
	mov	[bx+6],bp
	add	bx,8

	mov	dx,ROWBYTES
	mul	dx
	add	di,ax			; ES:DI -> 1st pixel

	xor	ax,ax			; AX = 0 (mask color)
RowLp:	push	di
	mov	cx,si

ColLp:	mov	dx,cx
	repnz	scasb			; scan for 1st NULL
	jnz	@F			; if EOL, don't fixup
	inc	cx
	dec	di
@@:	sub	dx,cx
	mov	[bx+0],dx		; >> SKIP

	mov	dx,cx
	or	cx,cx			; set ZF for CX=0 case
	repz	scasb			; scan for 1st non-NULL
	jz	@F			; if EOL, don't fixup
	inc	cx
	dec	di
@@:	sub	dx,cx
	mov	[bx+2],dx		; >> SIZE

	add	bx,4
	or	cx,cx
	jnz	ColLp

	or	dx,dx			; Optimize: null SIZE only if only
	jnz	eol
	cmp	si,[bx-4]
	je	eol
	sub	bx,4			; SIZE==0 && SKIP!=SpWide

eol:	or	[bx-2],8000h		; set EOL sentinel

new:	pop	di
	add	di,ROWBYTES
	dec	bp
	jnz	RowLp

	or	dx,dx			; add in final optimize for AWH
	jnz	@F
	cmp	si,[bx]
	je	@F
	add	bx,4
@@:
	pop	ax
	sub	bx,ax			; return (size)
	mov	ax,bx
	inc	ax
	ret

BuildMask  ENDP


;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   FlirCopy (int far *mask)						³
;³									³
;³	copy from page-1 to page-0 at&thru the mask			³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
FlirCopy	PROC FAR	USES SI DI DS, MaskSp:dword

	mov	ax,10			; GraphicPage[0]
	push	ax
	call	LibCanDo
	add	sp,+2
	mov	Dseg,ax

	mov	ax,12			; GraphicPage[1]
	push	ax
	call	LibCanDo
	add	sp,+2
	mov	Sseg,ax

	lds	si,MaskSp		; DS:si -> Mask

	mov	di,[si+0]		; X-pos
	mov	ax,[si+2]		; Y-pos
	mov	dx,ROWBYTES
	mul	dx
	add	di,ax
	mov	es,Dseg			; ES:DI -> 1st pixel (dest)

	mov	ax,[si+4]		; AX = width
	mov	dx,[si+6]		; DX = height
	or	ax,ax
	jz	NoGo
	or	dx,dx
	jz	NoGo

	add	si,8
	mov	bp,ds			; BP = FP_SEG(mask)

        sub     bh,bh

RohLp:	push	di
Cl2Lp:	mov	ds,bp
	add	di,[si+0]		; SKIP
	mov	cx,[si+2]		; SIZE
	add	si,4
	and	cx,7FFFh		; strip off EOL sentinel
	jz	new
	mov	ds,Sseg
@@:
        mov     bl,[di]
        mov     al,ss:FlirTbl[bx]
        mov     [di],al
        inc     di

        dec     cx
        jnz     @B

new:	pop	di
	add	di,ROWBYTES
	dec	dx
	jnz	RohLp
NoGo:
	ret

FlirCopy	ENDP

	END
