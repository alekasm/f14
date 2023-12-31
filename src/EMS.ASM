PAGE 79,124
;浜様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様融
;�********************   MPS Labs Common Library    ********************�
;把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超
;�									�
;�  File:  EMS.asm							�
;�	   								�
;�  Auth:  David McKibbin						�
;�	   								�
;�  Edit:  dtm   February 18, 1993    12:16 pm				�
;�									�
;把陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳超
;�   Copyright (c) 1991 by MicroProse Software, All Rights Reserved.	�
;藩様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様夕

	TITLE	Expanded Memory System LIB

	.MODEL	MEDIUM, C

	Public	EMSseg
	Public	EMSerr


EMSraw	MACRO	func

	mov	ah,func+63
	int	67h
	mov	EMSerr,ah
	or	ah,ah
	ENDM


EMSint	MACRO	func
	LOCAL	Exit,Exec

	cmp	EMSseg,0
	jnz	Exec
	call	EMSinit
	jz	Exec
	jmp	Exit
Exec:
	EMSraw	func
Exit:
	ENDM

	.DATA

EMSname	db	'EMMXXXX0'
EMSseg	dw	0
EMSerr	db	0,0

	.CODE

;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSprsnt()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSprsnt  PROC uses SI DI ES

	xor	ax,ax
	mov	es,ax
	mov	es,es:[67h*4 + 2]
	mov	di,10
	lea	si,EMSname
	mov	cx,8
	repe	cmpsb
	jz	@F
	mov	ah,90h
@@:	mov	EMSerr,ah	; return AH=00 & ZR for A-OK
	ret			;	 AH=90 & NZ for error

EMSprsnt  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSinit()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSinit	PROC

	mov	EMSseg,0

	call	EMSprsnt
	.286
	shl	ax,8
	.8086
	jnz	@F

	EMSraw	1	; EMS: Get Manager Status
	jnz	@F

	EMSraw	2	; EMS: Get PageFrame Segment
	jnz	@F
	mov	EMSseg,bx
@@:
	ret

EMSinit	ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSstat()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSstat  PROC

	EMSint	1	; EMS: Get Manager Status
	mov	al,ah
	mov	ah,0
	ret

EMSstat  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSframe()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSframe  PROC	frame:ptr

	EMSint	2	; EMS: Get PageFrame Segment
	jnz	@F
	mov	cx,bx
	mov	bx,frame
	mov	[bx],cx
@@:
	ret

EMSframe  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSpages()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSpages  PROC	free:ptr, total:ptr

	EMSint	3	; EMS: Get number of pages
	jnz	@F
	mov	cx,bx
	mov	bx,free
	mov	[bx],cx
	mov	bx,total
	mov	[bx],dx
@@:
	ret

EMSpages  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSalloc()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSalloc  PROC	pages:word, hptr:ptr

	mov	bx,pages
	EMSint	4	; EMS: Get Handle and Allocate Memory
	jnz	@F
	mov	bx,hptr
	mov	[bx],dx
@@:
	ret

EMSalloc  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSmap()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSmap  PROC	ppage:byte, lpage:word, handle:word

	mov	al,ppage
	mov	bx,lpage
	mov	dx,handle
	EMSint	5	; EMS: Map Memory
	ret

EMSmap  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSfree()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSfree  PROC	handle:word

	mov	dx,handle
	EMSint	6	; EMS: Release Handle and Memory
	ret

EMSfree  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSver()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSver  PROC	ver:ptr

	EMSint	7	; EMS: Get EMS Version
	jnz	@F
	mov	bx,ver
	mov	[bx],ax
@@:
	ret

EMSver  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSsave()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSsave  PROC	handle:word

	mov	dx,handle
	EMSint	8	; EMS: Save Mapping Context
	ret

EMSsave  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSrest()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSrest  PROC	handle:word

	mov	dx,handle
	EMSint	9	; EMS: Restore Mapping Context
	ret

EMSrest  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMShands()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMShands  PROC	hands:ptr

	EMSint	12	; EMS: Get Number of EMS Handles
	jnz	@F
	mov	cx,bx
	mov	bx,hands
	mov	[bx],cx
@@:
	ret

EMShands  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMShsize()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMShsize  PROC	handle:word, pages:ptr

	mov	dx,handle
	EMSint	13	; EMS: Get Pages Owned by Handle
	jnz	@F
	mov	cx,bx
	mov	bx,pages
	mov	[bx],cx
@@:
	ret

EMShsize  ENDP


;敖陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳朕
;�									�
;�  EMSread()								�
;�									�
;青陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳陳潰
EMSread  PROC	handle:word

	ret

EMSread  ENDP

	END
