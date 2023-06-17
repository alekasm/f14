	DOSSEG
	.MODEL MEDIUM,C

;	PUBLIC	viewmat

	PUBLIC	Seg3D

	PUBLIC	PalPtr3D

	PUBLIC	MAPBUF

	PUBLIC	FlirTbl

.CODE

TransScaleLine	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD,TRNSCLR:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W

	mov	dx,TRNSCLR
	cmp	dx,-1
	jne	L1
	mov	bp,ADDR
L4:
	mov	bx,[bp]
	mov	al,[bx+si]
	mov	es:[di],al
	add	bp,2
	add	di,1
	loop	L4
	ret
L1:
	mov	bp,ADDR
L5:
	mov	bx,[bp]
	mov	al,[bx+si]
	cmp	al,dl
	je	L3
	mov	es:[di],al
L3:
	add	bp,2
	add	di,1
	loop	L5

	ret
TransScaleLine	ENDP



TacTransScaleLine	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD,TRNSCLR:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W

	mov	dx,TRNSCLR
	mov	bp,ADDR
L5:
	mov	bx,[bp]
	mov	al,[bx+si]
	cmp	al,0
	je	L3
.386
        shr     al,5
.8086
        add     al,dl
	mov	es:[di],al
L3:
	add	bp,2
	add	di,1
	loop	L5

	ret
TacTransScaleLine	ENDP





.DATA

PalPtr3D	dw	defaultpal

defaultpal	dw	0FF00h,0FF01h,0FF02h,0FF03h,0FF04h,0FF05h,0FF06h,0FF07h
	dw	0FF08h,0FF09h,0FF0Ah,0FF0Bh,0FF0Ch,0FF0Dh,0FF0Eh,0FF0Fh	; 08
	dw	0FF10h,0FF11h,0FF12h,0FF13h,0FF14h,0FF15h,0FF16h,0FF17h	; 10
	dw	0FF18h,0FF19h,0FF1Ah,0FF1Bh,0FF1Ch,0FF1Dh,0FF1Eh,0FF1Fh	; 18
	dw	0FF20h,0FF21h,0FF02h,0FF23h,0FF24h,0FF25h,0FF06h,0FF27h	; 20
	dw	0FF28h,0FF29h,0FF2Ah,0FF2Bh,0FF2Ch,0FF2Dh,0FF2Eh,0FF2Fh	; 28
	dw	0FF30h,0FF02h,0FF04h,0FF33h,0FF0Ch,0FF35h,0FF06h,0FF37h	; 30
	dw	0FF38h,0FF01h,0FF3Ah,0FF3Bh,0FF3Ch,0FF3Dh,0FF3Eh,0FF3Fh	; 38

	dw	0FF40h,0FF41h,0FF42h,0FF43h,0FF44h,0FF45h,0FF46h,0FF47h
	dw	0FF48h,0FF49h,0FF4Ah,0FF4Bh,0FF4Ch,0FF4Dh,0FF4Eh,0FF4Fh
	dw	0FF50h,0FF51h,0FF52h,0FF53h,0FF54h,0FF55h,0FF56h,0FF57h
	dw	0FF58h,0FF59h,0FF5Ah,0FF5Bh,0FF5Ch,0FF5Dh,0FF5Eh,0FF5Fh
	dw	0FF60h,0FF61h,0FF62h,0FF63h,0FF64h,0FF65h,0FF66h,0FF67h
	dw	0FF68h,0FF69h,0FF6Ah,0FF6Bh,0FF6Ch,0FF6Dh,0FF6Eh,0FF6Fh
	dw	0FF70h,0FF71h,0FF72h,0FF73h,0FF74h,0FF75h,0FF76h,0FF77h
	dw	0FF78h,0FF79h,0FF7Ah,0FF7Bh,0FF7Ch,0FF7Dh,0FF7Eh,0FF7Fh
	dw	0FF80h,0FF81h,0FF82h,0FF83h,0FF84h,0FF85h,0FF86h,0FF87h
	dw	0FF88h,0FF89h,0FF8Ah,0FF8Bh,0FF8Ch,0FF8Dh,0FF8Eh,0FF8Fh
	dw	0FF90h,0FF91h,0FF92h,0FF93h,0FF94h,0FF95h,0FF96h,0FF97h
	dw	0FF98h,0FF99h,0FF9Ah,0FF9Bh,0FF9Ch,0FF9Dh,0FF9Eh,0FF9Fh
	dw	0FFA0h,0FFA1h,0FFA2h,0FFA3h,0FFA4h,0FFA5h,0FF07h,0FF07h	; A0
	dw	0FF0Eh,0FF0Eh,0FF08h,0FF08h,0FF08h,0FF00h,0FF00h,0FF00h	; A8
	dw	0FFB0h,0FFB1h,0FFB2h,0FFB3h,0FFB4h,0FFB5h,0FFB6h,0FFB7h	; B0
	dw	0FFB8h,0FFB9h,0FFBAh,0FFBBh,0FFBCh,0FFBDh,0FFBEh,0FFBFh	; B8
	dw	0FFC0h,0FFC1h,0FFC2h,0FFC3h,0FFC4h,0FFC5h,0FFC6h,0FFC7h	; C0
	dw	0FFC8h,0FFC9h,0FFCAh,0FFCBh,0FFCCh,0FFCDh,0FFCEh,0FFCFh	; C8
	dw	0FFD0h,0FFD1h,0FFD2h,0FFD3h,0FFD4h,0FFD5h,0FFD6h,0FFD7h	; D0
	dw	0FFD8h,0FFD9h,0FFDAh,0FFDBh,0FFDCh,0FFDDh,0FFDEh,0FFDFh	; D8
	dw	0FF07h,0FF07h,0FF0Eh,0FF08h,0FF00h,0FFE5h,0FF07h,0FF07h	; E0
	dw	0FF0Eh,0FF0Eh,0FF0Eh,0FF08h,0FF08h,0FF08h,0FF00h,0FF00h	; E8
	dw	0FFF0h,0FFF1h,0FFF2h,0FFF3h,0FFF4h,0FFF5h,0FFF6h,0FFF7h
	dw	0FFF8h,0FFF9h,0FFFAh,0FFFBh,0FFFCh,0FFFDh,0FFFEh,0FFFFh



;viewmat		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh ;view and inverse matrixes
;		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

Seg3D		dw	SEG bogusseg

	ALIGN 16				;put at paragraph
bogusseg 	dw	0

        ALIGN 256

FlirTbl	db	256 dup(0)


_MAPSG	SEGMENT	PARA PUBLIC 'MP'

MAPBUF	db	16384 dup(0)

_MAPSG	ENDS

	END


