	DOSSEG
	.MODEL MEDIUM,C
        page  60, 132


;/***************************************/
;
;	clip.asm
;	Scott Spanburg
;	01\02\90
;
;
;/***************************************/

FALSE     equ   0
TRUE      equ   not FALSE

TOOL3D    equ	FALSE
GAME      equ	TRUE

include global.inc
include	colors.equ
include	clip.equ
include	graph.ext
include clip.ext

	COMM	_XEB:WORD:1024*3

rotatebuf	EQU	_XEB
perspbuf	EQU	_XEB+4096


STANDARD	EQU	1



SMUL	MACRO	val
	imul	WORD PTR val
	shl	ax,1
	rcl	dx,1
	ENDM
SMULREG	MACRO	reg
	imul	reg
	shl	ax,1
	rcl	dx,1
	ENDM

ABSAX	MACRO
	cwd
	xor	ax,dx
	sub	ax,dx
	ENDM

ABSAXDX	MACRO
	LOCAL	abs_ok
	or	dx,dx
	jns	abs_ok
	not	ax
	not	dx
	add	ax,1
	adc	dx,0
abs_ok:
	ENDM
SAR4	MACRO	reg
	sar	reg,1
	sar	reg,1
	sar	reg,1
	sar	reg,1
	ENDM
SHL4	MACRO	reg
	shl	reg,1
	shl	reg,1
	shl	reg,1
	shl	reg,1
	ENDM
WALKDWN	MACRO			;ax has hi! word, dx has lo word
	mov	dl,dh
	mov	dh,al
	mov	al,ah
	cbw
	xchg	ax,dx
	ENDM
WALKUP	MACRO			;ax has hi! word, dx has garbage
	cwd
	mov	dl,ah
	mov	ah,al
	sub	al,al
	ENDM
;
;*****************************************************
;	3d object definition :
;
;	dw	dist,obj0,dist,obj1,..	;switch dist,object ptr
;
;obj0	dw	radius				;actual object-radius word
;	dw	matrices # 	;0= view mat, >0= articulate
;		db	var#,HPR		;H=0,P=1,R=0	;mat1
;		db	var#,HPR		;H=0,P=1,R=0	;mat2
;		db	var#,HPR		;H=0,P=1,R=0	;mat3
;		dw	x,y,zoff		;offset of mat1
;		dw	x,y,zoff		;offset of mat2
;		dw	x,y,zoff		;offset of mat3
;
;	dw	points #	;<0= starndard pt #, >0= # points to rotate
;
; 	dw	surface #	;<0= bsp'd 0= all show, >0 = frontface
; 		dw	{A,B,C,D}...	;surf coefs for each surf(if bsp or front)
; 		dw	{A,B,C,D}...	;surf coefs for each surf(if bsp or front)
;		dw	A HI BYTE=80h	new matrix LO BYTE = # of matrix
; 		dw	{A,B,C,D}...	;surf coefs for each surf(if bsp or front)
;		dw	A =8100h	surf animation(next 2 BYTES= var,val)
;			db	var#,value
; 		dw	{A,B,C,D}...	;surf coefs for each surf(if bsp or front)
;
;	db	#PTS,{pt#,pt#,..}	 ;lists of pts for each surf(bsp or f)
;	db	#PTS,{pt#,pt#,..}	 ;lists of pts for each surf(bsp or f)
;
; 	db	rootbsp		;optional bsp tree here
;	db	f,b.....
;
;points x,y,z
;	dw	{x,y,z}, {...}, ...
;	dw	x=8000h+# of matrix	;next matrix
;	dw	{x,y,z}, {...}, ...
;
;OR	db	{Pt#,pt#,pt#..}	;standard pt indices to persp
;
;	dw	lines #		;# of lines
;	db	{pt1,pt2},{..},....	;pt indicies of lines
;
;	dw	GOP0,GOP1,...		;gop list ptrs(if front or BSP)
;GOP0:
;	db	Gop{..},Gop{...},..,-1	;goplists for each surf
;GOP1:	db	Gop{..},Gop{...},..,-1	;.....
;
;
;GOP(BYTES)= -1= end of list, hi bit= anim, bit 6=dots, bit 5= lines
;lower 5 bits= cnt
;polys= line#,line#,line#,.., color fill
;dots= color, pt#,pt#,...
;lines= color, line#,line#,...
;anim lower 5 bits= var# in anim(WORD) list, next BYTE= anim value(ext to word)
;
;
;

	extrn	MULT1X3:PROC, MULT3X3:PROC, COPY3X3:PROC, TRANS3X3:PROC
	extrn	MakeMatHPR:PROC,Rotate:PROC
	extrn	ASinB:PROC,ACosB:PROC,ARCTAN:PROC,MulDiv:PROC
	extrn	Clip32:PROC

;	extrn	ShadeFunction:PROC

	PUBLIC	identitymat,linebuf, viewmat
	PUBLIC	zumefactor
	PUBLIC	Seg3D,LightX,LightY,LightZ
	PUBLIC	PalPtr3D, ClrSwapPtr3D, ClipPtr
	PUBLIC	OutLine,NOASPECT
	PUBLIC	defaultpal


	PUBLIC	parallel

;	PUBLIC	RejectObject, FigMatrices, FigFrontBack,
;	PUBLIC	SetDivByZero
;	PUBLIC	RotPoints, ClipLines, DrawSurfs, ResetDivByZero
;	PUBLIC	ClearPtsFB, LtIntense, QRot, Persp, DivOverflow, Clip3D
;	PUBLIC	Bspsort

	IF	TOOL3D

	EXTRN	SX:word, SY:word, GXMID:word, GYMID:word
	EXTRN	TileX:word, TileY:word

	ENDIF

	IF	GAME

	EXTRN	CRASHED:word
	EXTRN	Object:word
	ENDIF

.CODE

SetViewMat	PROC	MAT:WORD,ZOOM:WORD
;
;ZOOM	100h= lowest zoom
;
	mov	ax,ZOOM

	shl	ax,1

	ABSAX
	cmp	ax,100h
	jae	@F
   	mov	ax,100h
@@:
	mov	cx,ax
	mov	dx,07fh
	mov	ax,0ff00h
	div	cx
	mov	zumefactor,ax		;save it
;ax= zumefactor to "mult" z coords by
	lea	ax,viewmat
	push	ax
	mov	ax,MAT
	push	ax
	call	Copy3X3
	add	sp,4
	lea	ax,viewmat+18
	push	ax
	sub	ax,18
	push	ax
	call	Trans3X3


	lea	bx,viewmat
	mov	ax,zumefactor
	neg	ax
	mov	cx,ax
	SMUL	[bx].R1.C3
	mov	[bx].R1.C3,dx
	mov	ax,cx
	SMUL	[bx].R2.C3
	mov	[bx].R2.C3,dx
	mov	ax,cx
	SMUL	[bx].R3.C3
	mov	[bx].R3.C3,dx


; fisheye things here

	sar	WORD PTR [bx].R1.C1,1
	sar	WORD PTR [bx].R2.C1,1
	sar	WORD PTR [bx].R3.C1,1

	sar	WORD PTR [bx].R1.C2,1
	sar	WORD PTR [bx].R2.C2,1
	sar	WORD PTR [bx].R3.C2,1

;y screen fix

	cmp	NOASPECT,0
	je	@F
	neg	WORD PTR [bx].R1.C2
	neg	WORD PTR [bx].R2.C2
	neg	WORD PTR [bx].R3.C2
	jmp	svm_out
@@:
	mov	ax,[bx].R1.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R1.C2,ax

	mov	ax,[bx].R2.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R2.C2,ax

	mov	ax,[bx].R3.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R3.C2,ax

svm_out:
	add	sp,4
	ret
SetViewMat	ENDP

;
Draw3DObject PROC USES es si di, OBJ:WORD,XPOS:WORD,YPOS:WORD,ZPOS:WORD,MAT:PTR,ANIM:PTR
;x,y,z = 16 bit mat = 16 bit ptr to matrices if needed
;anim = 16 bit ptr to animation list-if needed
	mov	CRASH,0
	mov	ax,Seg3D	 ;get seg of 3d objects far ptrs
	mov	es,ax
	sub	si,si
	mov	ax,ES:[si]	;get # of objects 1 based
	cmp	ax,OBJ
	ja	@F
	ret
@@:
	mov	ax,XPOS
	mov	relobjpt.C1,ax
	mov	ax,YPOS
	mov	relobjpt.C2,ax
	mov	ax,ZPOS
	mov	relobjpt.C3,ax
	mov	ax,MAT	    		;set the object articulation matrix
	mov	objmatptr,ax
	mov	ax,ANIM
	mov	animptr,ax

	lea	ax,rotrelpt
	push	ax
	lea	ax,viewmat
	push	ax
	lea	ax,relobjpt
	push	ax
	call	Mult1X3			;rotate the point into view
	add	sp,6			;clean up


	mov	bx,OBJ			;get object #
	shl	bx,1
	mov	si,ES:[bx+2]			;get address of object
;ES:si points to dist,ptr,dist,ptr,etc
	mov	ax,WORD PTR rotrelpt.C3HI	;hi of rotated z val
	ABSAX
	mov	dx,ax
@@:
	mov	ax,ES:[si]		;get dist val
	add	si,2
	mov	bx,ES:[si]		;get ptr val
	add	si,2
	cmp	dx,ax			;farther away than switch val ?
	ja	@B			;yes-try next
	mov	si,bx
;ES[si]= object
	mov	ax,ES:[si]		;get radius
	add	si,2
	cmp	ax,dx			; compare radius of object to distance
	ja	@F			; no crash if far away
	mov	CRASH,80h
@@:	cmp	parallel,0
	jne	zapo
	call	RejectObject		;any showing ?
	je	zapo
	mov	CRASH,80h
	jmp	@F			;nope-return

zapo:	mov	cx,ES:[si]
	add	si,2			;get # matrices
	call	FigMatrices		;set up dump pts,matrix address
;ES:[si]= # points
	mov	ax,ES:[si]		;get # points
	add	si,2
	mov	numpoints,ax
;ES:[si]= # surfaces
	call	FigFrontBack		;set the surface nums and figure f/b
;ES:[si]= # points

	mov	ax,numsurfs
	ABSAX
	cmp	ax,4
	jae	NC
	mov	CRASH,80h

;setup div by zero vec
NC:
	call	SetDivByZero
	call	RotPoints		;rotate and perspective the points

;ES:[si]= # lines
	call	ClipLines		;zclip and screen clip lines
;ES:[si]=surface defs
	call	DrawSurfs

	IF	EGA_ONLY

;	call	EGAFix

	ENDIF

	call	ResetDivByZero

@@:
	mov	bx,WORD PTR CRASH
	and	bx,80h
	xor	bx,80h
	jz	@F
	nop		; trap here if collision occured (inside object)
@@:	or	CRASHED,bx
	ret
Draw3DObject	ENDP

Test3DObject PROC USES es si di,OBJ:WORD,XPOS:WORD,YPOS:WORD,ZPOS:WORD,MAT:PTR
;x,y,z = 16 bit mat = 16 bit ptr to matrices if needed
	mov	CRASH,0
	mov	ax,Seg3D	 ;get seg of 3d objects far ptrs
	mov	es,ax
	sub	si,si
	mov	ax,ES:[si]	;get # of objects 1 based
	cmp	ax,OBJ
	ja	@F
	ret
@@:
	mov	ax,XPOS
	mov	relobjpt.C1,ax
	mov	ax,YPOS
	mov	relobjpt.C2,ax
	mov	ax,ZPOS
	mov	relobjpt.C3,ax
	mov	ax,MAT	    		;set the object articulation matrix
	mov	objmatptr,ax

	lea	ax,rotrelpt
	push	ax
	lea	ax,viewmat
	push	ax
	lea	ax,relobjpt
	push	ax
	call	Mult1X3			;rotate the point into view
	add	sp,6			;clean up


	mov	bx,OBJ			;get object #
	shl	bx,1
	mov	si,ES:[bx+2]			;get address of object
;ES:si points to dist,ptr,dist,ptr,etc
	mov	ax,WORD PTR rotrelpt.C3HI	;hi of rotated z val
	ABSAX
	mov	dx,ax
@@:
	mov	ax,ES:[si]		;get dist val
	add	si,2
	mov	bx,ES:[si]		;get ptr val
	add	si,2
	cmp	dx,ax			;farther away than switch val ?
	ja	@B			;yes-try next
	mov	si,bx
;ES[si]= object
	mov	ax,ES:[si]		;get radius
	add	si,2
	cmp	ax,dx			; compare radius of object to distance
	ja	@F			; no crash if far away
	mov	CRASH,80h
@@:	call	RejectObject		;any showing ?
	je	zapo
	mov	CRASH,80h
	jmp	@F			;nope-return

zapo:	mov	cx,ES:[si]
	add	si,2			;get # matrices
	call	FigMatrices		;set up dump pts,matrix address
;ES:[si]= # points
	mov	ax,ES:[si]		;get # points
	add	si,2
	mov	numpoints,ax
;ES:[si]= # surfaces
	call	FigFrontBack		;set the surface nums and figure f/b
;ES:[si]= # points

	mov	ax,numsurfs
	ABSAX
	cmp	ax,4
	jae	@F
	mov	CRASH,80h

;setup div by zero vec
@@:
	mov	bx,WORD PTR CRASH
	and	bx,80h
	xor	bx,80h
	jz	@F
	nop		; trap here if collision occured (inside object)
@@:	or	CRASHED,bx
	ret
Test3DObject	ENDP


SetDivByZero:
	push	es
	sub	bx,bx
	mov	es,bx
	mov	ax,es:[bx]
	mov	WORD PTR divbyzerovec,ax
	mov	ax,es:[bx+2]
	mov	WORD PTR divbyzerovec+2,ax
	mov	ax,cs
	mov	es:[bx+2],ax
	lea	ax,cs:DivOverflow
	mov	es:[bx],ax
	pop	es
	ret
ResetDivByZero:
	push	es
	sub	bx,bx
	mov	es,bx
	mov	ax,WORD PTR divbyzerovec
	mov	es:[bx],ax
	mov	ax,WORD PTR divbyzerovec+2
	mov	es:[bx+2],ax
	pop	es
	ret
;
;
;
RejectObject:
;rotrelpt = rotated object center relative to eye
;ax= radius value
	mov	bx,ax

;	shr	ax,1
;	add	bx,ax

	mov	cx,zumefactor

	mov	ax,WORD PTR rotrelpt+10		;z hi
	ABSAX
	inc	ax
	shl	ax,1
	cmp	ax,cx
	jae	rj_bad				;overflow-leave


;	mov	ax,WORD PTR rotrelpt+8		;z lo
;	mov	dx,WORD PTR rotrelpt+10		;z hi
;	idiv	cx				;undo zoom



	mov	ax,WORD PTR rotrelpt+10

	add	ax,bx
	mov	dx,ax
	sub	bx,bx
	mov	bp,dx

	add	bx,WORD PTR rotrelpt+8	;lo of zval
	adc	bp,WORD PTR rotrelpt+10	;hi of zval
	js	rj_bad		;behind viewer

	mov	ax,WORD PTR rotrelpt		;lo of xval
	mov	dx,WORD PTR rotrelpt+2		;hi of x val
	ABSAXDX				;get abs val of long-macro
	cmp	dx,bp
	ja	rj_bad			;greater than 45 deg-leave

	mov	ax,WORD PTR rotrelpt+4		;lo of yval
	mov	dx,WORD PTR rotrelpt+6		;hi of y val
	ABSAXDX				;get abs val of long-macro
	cmp	dx,bp
	ja	rj_bad			;greater than 45 deg-leave
	sub	ax,ax			;its ok
	ret
rj_bad:
	mov	ax,-1
	or	ax,ax
	ret
;
;

FigMatrices:
;cx= # of matrices
;-1= standard, 0= view mat 1-4 ?= obj concat
;dump pt = rel obj pt
	mov	nummats,cx	;save for dumppts, standard pts
	mov	ax,relobjpt
	neg	ax
	mov	dumppts,ax
	mov	ax,relobjpt+2
	neg	ax
	mov	dumppts+2,ax
	mov	ax,relobjpt+4
	neg	ax
	mov	dumppts+4,ax

	cmp	cx,0
	jg	@F		;make some matrices
	lea	ax,viewmat
	mov	rotmatptr,ax
	ret
@@:
	mov	bp,cx		;save # of mats
	lea	bx,concatmats	;dest of 3X3
	mov	rotmatptr,bx	;save for points
	lea	di,viewmat	;transform of 3X3
	mov	bx,objmatptr	;object matrix
	jmp	fgm_2
fgm_1:
	mov	bl,ES:[si]	;get var #

	sub	bh,bh

;	and	bx,0fh
	shl	bx,1		;*2 for words
	add	bx,animptr	;plus base
	mov	bx,[bx]
	sub	ax,ax
	sub	dx,dx
	cmp	BYTE PTR ES:[si+1],1	;o=head,1=pisth,2=roll	;get HPR
	je	@F
	xchg	ax,bx
	jl	@F			;0=head=ax
	xchg	ax,dx
@@:
	add	si,2
;make hpr matrix at rotmat + inverse
	mov	di,rotmatptr
	add	di,18*2
	push	di		;dest
	push	dx
	push	bx
	push	ax
	call	MakeMatHPR	;(H,P,R,M1)
	add	sp,6
	pop	bx
	lea	di,concatmats	;obj*view transform
fgm_2:
	push	rotmatptr	;destination
	push	di		;transform
	push	bx		;artobj matrix
	call	Mult3X3		;(m1,m2,m3)
	pop	bx
	add	sp,2
	pop	di		;dest
	add	di,18		;inverse
	push	di		;inv dest
	push	bx		;artobj mat source
	call	Trans3X3
	add	sp,4
	add	rotmatptr,18*2	;next mat,inv pair
	dec	bp
	jne	fgm_1
	lea	bx,concatmats	;put the matrices ptr back
	mov	rotmatptr,bx	;save for points
;make dumppts
	mov	ax,WORD PTR rotrelpt.C1LO	;copy rotrel to ptsoff(long vector)
	mov	WORD PTR ptsoff.C1LO,ax
	mov	ax,WORD PTR rotrelpt.C1HI
	mov	WORD PTR ptsoff.C1HI,ax
	mov	ax,WORD PTR rotrelpt.C2LO
	mov	WORD PTR ptsoff.C2LO,ax
	mov	ax,WORD PTR rotrelpt.C2HI
	mov	WORD PTR ptsoff.C2HI,ax
	mov	ax,WORD PTR rotrelpt.C3LO
	mov	WORD PTR ptsoff.C3LO,ax
	mov	ax,WORD PTR rotrelpt.C3HI
	mov	WORD PTR ptsoff.C3HI,ax
	lea	ax,ptsoff
	mov	ptsoffptr,ax


	mov	bp,nummats	;get # of mats
	lea	bx,dumppts
	lea	di,concatmats+18	;inverse mat


	push	di		;rotate the relobjpt about obj inverse
	push	bx
	call	Rotate		;(V,M1)
	pop	bx
	pop	di

	call	LtDump0		;do the light point for object

	jmp	fgm_13		;do next pts-based on this pt
fgm_14:


	call	getptsoff

	mov	ax,dumppts.C1	;get dump1
	sub	ax,ES:[si].C1 	;minus offset of sub articulation
	mov	[bx].C1,ax
	mov	ax,dumppts.C2
	sub	ax,ES:[si].C2
	mov	[bx].C2,ax
	mov	ax,dumppts.C3
	sub	ax,ES:[si].C3
	mov	[bx].C3,ax
	add	si,6		;3 word offset
	push	di
	push	bx
	call	Rotate		;(V,M1)
	pop	bx
	pop	di

	call	LtDump1		;dump the sub light pts


fgm_13:
	add	ptsoffptr,12	;next ptsoff-3 long words
	add	bx,6		;next dump
	add	di,18*2		;next inv mat
	dec	bp
 	jne	fgm_14
fgm_3:
	ret
;
getptsoff:
;ES:[si]=xoff,yoff,zoff
;di-18= [OV], rotmatptr= [AOV]
;bx= dumppt
	mov	ax,ES:[si].C1	;get x off
	mov	[bx].C1,ax
	mov	ax,ES:[si].C2	;get y off
	mov	[bx].C2,ax
	mov	ax,ES:[si].C3	;get z off
	mov	[bx].C3,ax
	lea	ax,temprot
	push	ax
	push	rotmatptr
	push	bx
	call	Mult1X3
 	pop	bx
	add	sp,4

	neg	WORD PTR [bx].C1
	neg	WORD PTR [bx].C2
	neg	WORD PTR [bx].C3

	push	ptsoffptr	;destination
	mov	ax,di
	sub	ax,18
	push	ax
	push	bx
	call	Mult1X3		;(V,M1,VL)
;now add the relobjpt, temprotpt
	mov	bx,ptsoffptr
	mov	ax,WORD PTR rotrelpt.C1LO
	add	WORD PTR [bx].C1LO,ax
	mov	ax,WORD PTR rotrelpt.C1HI
	adc	WORD PTR [bx].C1HI,ax
	mov	ax,WORD PTR temprot.C1LO
	add	WORD PTR [bx].C1LO,ax
	mov	ax,WORD PTR temprot.C1HI
	adc	WORD PTR [bx].C1HI,ax

	mov	ax,WORD PTR rotrelpt.C2LO
	add	WORD PTR [bx].C2LO,ax
	mov	ax,WORD PTR rotrelpt.C2HI
	adc	WORD PTR [bx].C2HI,ax
	mov	ax,WORD PTR temprot.C2LO
	add	WORD PTR [bx].C2LO,ax
	mov	ax,WORD PTR temprot.C2HI
	adc	WORD PTR [bx].C2HI,ax

	mov	ax,WORD PTR rotrelpt.C3LO
	add	WORD PTR [bx].C3LO,ax
	mov	ax,WORD PTR rotrelpt.C3HI
	adc	WORD PTR [bx].C3HI,ax
	mov	ax,WORD PTR temprot.C3LO
	add	WORD PTR [bx].C3LO,ax
	mov	ax,WORD PTR temprot.C3HI
	adc	WORD PTR [bx].C3HI,ax

	pop	bx
	add	sp,4
	ret
;
LtDump0:
;di= matrix
	push	bx		;save original bx
	lea	bx,lightdumppts
	mov	templightptr,bx
	mov	ax,LightX
	mov	[bx],ax
	mov	ax,LightY
	mov	[bx].C2,ax
	mov	ax,LightZ
	mov	[bx].C3,ax
	push	di
	push	bx
	call	Rotate		;(V,M1)
	add	sp,2
	pop	di
	pop	bx
	add	templightptr,6
	ret
LtDump1:
;di= matrix
	push	bx		;save original bx
	mov	bx,templightptr
	mov	ax,lightdumppts.C1
	mov	[bx].C1,ax
	mov	ax,lightdumppts.C2
	mov	[bx].C2,ax
	mov	ax,lightdumppts.C3
	mov	[bx].C3,ax

	push	di
	push	bx
	call	Rotate		;(V,M1)
	add	sp,2
	pop	di
	pop	bx
	add	templightptr,6
	ret
;
;
fb_surfart:
;ES:[si] next bytes= var #, value
	push	bx
	mov	bl,ES:[si]
	and	bx,01fh
	shl	bx,1
	add	bx,animptr
	mov	al,ES:[si+1]
	add	si,2
	cbw
	cmp	ax,[bx]
	pop	bx
	je	fb_5		;do it
;skip it here
	mov	surfskip,07fh
	jmp	fb_5
fb_newmat:
;lo byte of ax= mat #
	and	ax,0ffh
	lea	bx,dumppts  	;make bx= dumppts+6*mat#
	shl	ax,1		;*2
	mov	templightptr,ax
	add	bx,ax
	shl	ax,1		;*4
	add	bx,ax		;*2+ *4= *6
	add	templightptr,ax
	lea	ax,lightdumppts
	add	templightptr,ax
	jmp	fb_5
FigFrontBack:
;es:si= front back info
	mov	ax,ES:[si]		;get # surfs
	add	si,2
	mov	numsurfs,ax	;0= none, neg=bspd(neg to get #), pos= # surfs
	or	ax,ax
	jne	fb_1
	ret
fb_1:
	ABSAX			;# of surfs
	mov	cx,ax
	sub	di,di		;	lea	di,frontback		;dest of front back info
	lea	bx,dumppts		;
	lea	ax,lightdumppts
	mov	templightptr,ax
fb_2:
	mov	surfskip,-1
fb_5:
	mov	ax,ES:[si]			;get a coef
	add	si,2
	cmp	ah,80h		;new matrix ?
	je	fb_newmat
	cmp	ax,8100h	;surface articulation ?
	je	fb_surfart
	push	cx
;ax= coef
	imul	WORD PTR [bx]		;A*x
	mov	cx,ax
	mov	bp,dx
	mov	ax,ES:[si]
	add	si,2
	imul	WORD PTR [bx+2]
	add	cx,ax
	adc	bp,dx
	mov	ax,ES:[si]
	add	si,2
	imul	WORD PTR [bx+4]
	add	cx,ax
	adc	bp,dx


	add	cx,ES:[si]		;lo of D
	adc	bp,ES:[si+2]
;sign of bp tells weather to draw or not
	add	si,4
	mov	ax,bp
	cwd
	not	dx		;dx=0 if no draw(bp=negative)
	cmp	parallel,0
	je	@F
	mov	dl,-1
@@:
	and	dl,surfskip
	mov	frontback[di],dl		;save f/b info
	jns	@F

;***********************************
; this code checks for floor surface
;***********************************
	mov	ax,ES:[si-10]		; [si]
	or	ax,ES:[si-6]		; [si+4]
	or	ax,ES:[si-4]		; [si+6]
	or	ax,ES:[si-2]		; [si+8]
	jne	fb_ss
	or	dl,80h
fb_ss:

;**********************************

	or	CRASH,dl

	call	LtIntense
@@:

	inc	di		;next byte in list
	pop	cx
	loop	fb_2		;next surface
fb_3:
;now set up the points flags
	call	ClearPtsFB	;clear flags out
;es:si= #PTS,pt#,pt#,.. next surf,etc
	sub	bx,bx
	mov	ax,numsurfs
	ABSAX
	mov	bp,ax
	mov	ax,1
	lea	di,frontback
fb_4:
	mov	cl,ES:[si]		;get # of pts
	inc	si
	and	cx,0ffh			;make a word
	je	fb_10			;no pts-was just a cutting surface
	cmp	BYTE PTR [di],07fh	;hi bit=0= not drawn
	ja	fb_9			;its ok
	add	si,cx			;skip the pts
	jmp	fb_10
fb_9:
	mov	bl,ES:[si]		;get the pt index
	inc	si
	mov	pointsfb[bx],al	;set point
	loop	fb_9
fb_10:
	inc	di
	dec	bp
	jne	fb_4
;set the bsp list here if applicable
	cmp	numsurfs,0
	js	@F		;make bsp list
	ret
@@:
	lea	di,polyorder  	;dest list -1 terminated
	lea	bx,frontback	;fornt back info 0= front -1=back
;ES:[si]= bsp tree   root,  frontptr,backptr,frntptr,backptr,...
	push	si
	call	Bspsort
	pop	si
	mov	ax,numsurfs
	neg	ax		;make it positive
	shl	ax,1		;2 bytes per surface
	inc	ax		;plus root !!!
	add	si,ax		;point past bsplist
	ret
;
ClearPtsFB:
;ABS(numpoints)= # of points
	mov	ax,numpoints
	ABSAX
	mov	cx,ax
	sub	ax,ax
	push	es
	mov	di,ds
	mov	es,di
	lea	di,pointsfb
	sub	ax,ax
	add	cx,2
	shr	cx,1
	rep	stosw
	pop	es
	ret
;
LtIntense:
;es:si = 10 past coefficients
;di= index into surface intensities
	push	bx
	mov	bx,templightptr
	sub	si,10

	mov	ax,ES:[si]
	add	si,2
	imul	WORD PTR [bx]		;A*x
	mov	cx,ax
	mov	bp,dx
	mov	ax,ES:[si]
	add	si,2
	imul	WORD PTR [bx+2]
	add	cx,ax
	adc	bp,dx
	mov	ax,ES:[si]
	add	si,2
	imul	WORD PTR [bx+4]
	add	cx,ax
	adc	bp,dx
;bp:cx= intensitie val
;	shl	cx,1
;	rcl	bp,1
;	shl	cx,1
;	rcl	bp,1
	shl	cx,1
	rcl	bp,1
	shl	cx,1
	rcl	bp,1
	mov	ax,bp
;ax= intensity
	add	si,4
	mov	lightshade[di],al		;save f/b info
	pop	bx
	ret
;
;
;
;
RotPoints:
;ES:si=add of, x,y,z, x,y,z, ...

	IF	TOOL3D

	mov	WORD PTR cnt,0

	ENDIF

	sub	di,di		;start at first point buf index
	mov	bx,rotmatptr	;get addres of rotation matrix
	mov	cx,numpoints	;get # of points
	cmp	cx,0		;positive = # of points, neg means standard
	je	rtp_out		;no points !!- wierd
	jns	@F
	jmp	PerspStandard	;do the persp on standard
@@:
	lea	di,rotatebuf	;get dest address for rotated pts
	lea	bp,DS:perspbuf	;get dest add of perspective buffer
	mov	rotpointptr,di	;set the rotate buffer address-for z clipping

	cmp	numsurfs,0	;all facing means no list
	jne	RotPtsFB	;go do front face points only
;now rotate all points
RotAllPts:
	call	QRot
	loop	RotAllPts	;do all pts
rtp_out:
	ret
rot_mat:
;lo 4 bits of ax= matrix # to use in rotations
	and	ax,0fh
;mult by 36
	push	di
	lea	di,ptsoff	;copy ptsoff to rotrelpt-len of 12

	mov	bx,rotmatptr
	shl	ax,1
	shl	ax,1
	add	bx,ax		;base + 4*
	add	di,ax
	shl	ax,1
	add	di,ax		;+4 +8 = 12
	shl	ax,1
	shl	ax,1
	add	bx,ax		;base + 4* + 32*
	add	si,2		;next x

;now copy ptsoff to relobjpt
	mov	ax,WORD PTR [di].C1LO
	mov	WORD PTR rotrelpt.C1LO,ax
	mov	ax,WORD PTR [di].C1HI
	mov	WORD PTR rotrelpt.C1HI,ax
	mov	ax,WORD PTR [di].C2LO
	mov	WORD PTR rotrelpt.C2LO,ax
	mov	ax,WORD PTR [di].C2HI
	mov	WORD PTR rotrelpt.C2HI,ax
	mov	ax,WORD PTR [di].C3LO
	mov	WORD PTR rotrelpt.C3LO,ax
	mov	ax,WORD PTR [di].C3HI
	mov	WORD PTR rotrelpt.C3HI,ax
	pop	di

	jmp	rtpfb_1		;continue
RotPtsFB:
;cx= points left es:[si]= x,y,z of points
;bx= mat add, di= offset in pointbuf, di/16 = offset in pointlist
;bp = persp buffer address
	sub	dx,dx		;first offset in buffer
rtpfb_1:
	mov	ax,ES:[si].C1	;x
;check for matrix change
	cmp	ax,8000h
	jb	@F
	cmp	ax,8010h
	jb	rot_mat		;8000h-800fh= matrix change LO 4 bits= mat #
@@:
	xchg	bx,dx		;point at f/b
	cmp	BYTE PTR pointsfb[bx],0
	xchg	dx,bx
	je	@F		;point doesnt show
	push	dx
	call	QRot		;go rot and persp point
	pop	dx
	inc	dx		;next f/b byte
	loop	rtpfb_1
	ret
@@:
	add	bp,8		;next persp buf
	add	di,16		;next rot buf
	add	si,6		;next points
	inc	dx		;next f/b byte
	loop	rtpfb_1
	ret
PerspStandard:
;cx= neg # of pts to do
;es:si= list of standard pts to perspect
	neg	cx			;make cnt positive

	lea	di,rotatebuf	;where to
	mov	rotpointptr,di	;set the rotate buffer address-for z clipping


@@:
	mov	al,ES:[si]	;get std pt #
	sub	ah,ah
	shl	ax,1
	shl	ax,1
	shl	ax,1		;*8= offset into persp buffer
	mov	bp,ax		;save *8
	shl	ax,1		;* 16= offset int rot points
	lea	bx,standardbuf	;where from
	lea	di,rotatebuf	;where to
	add	bx,ax		;actual address of standard
	add	di,ax		;actual address of rotated
	mov	ax,[bx].C1LO
	mov	dx,[bx].C1HI
	add	ax,WORD PTR rotrelpt.C1LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C1HI
	mov	[di].C1LO,ax
	mov	[di].C1HI,dx

	mov	ax,[bx].C2LO
	mov	dx,[bx].C2HI
	add	ax,WORD PTR rotrelpt.C2LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C2HI
	mov	[di].C2LO,ax
	mov	[di].C2HI,dx

	mov	ax,[bx].C3LO
	mov	dx,[bx].C3HI
	add	ax,WORD PTR rotrelpt.C3LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C3HI
	mov	[di].C3LO,ax
	mov	[di].C3HI,dx

	lea	bx,perspbuf		;base of persp dest
	add	bx,bp			;+ offset

	push	cx
;	push	si
	call	Persp
;	pop	si
	inc	si
	pop	cx
	loop	@B
	ret
;
qrot_chgmat:
;things on stack need to be updated
	pop	bp
	pop	bx
	pop	si
;al= # of mat
	sub	ah,ah
;add 18*2 *mat# to rotmatptr = 24h= ax<<2 + ax<<5 (ax*4+ax*32)
;copy ptsoff[ax] to relobjpt[] 4 long words len=12
	push	di
	lea	di,ptsoff
	mov	bx,rotmatptr		;get base
	shl	ax,1
	shl	ax,1
	add	di,ax			;ptsoff+*4
	add	bx,ax			;base + ax<<2
	shl	ax,1
	add	di,ax			;+4 +8 = *12
	shl	ax,1
	shl	ax,1
	add	bx,ax			;base+ ax<<2+ ax<<5
	add	si,2			;pt to x,y,z of next pt
;now copy ptsoff to relobjpt
	mov	ax,WORD PTR [di].C1LO
	mov	WORD PTR rotrelpt.C1LO,ax
	mov	ax,WORD PTR [di].C1HI
	mov	WORD PTR rotrelpt.C1HI,ax
	mov	ax,WORD PTR [di].C2LO
	mov	WORD PTR rotrelpt.C2LO,ax
	mov	ax,WORD PTR [di].C2HI
	mov	WORD PTR rotrelpt.C2HI,ax
	mov	ax,WORD PTR [di].C3LO
	mov	WORD PTR rotrelpt.C3LO,ax
	mov	ax,WORD PTR [di].C3HI
	mov	WORD PTR rotrelpt.C3HI,ax
	pop	di

	push	si
	push	bx
	push	bp
	jmp	qrot_1			;do next pt
QRot:
;ES:[si]= x,y,z, di= dest buf(long), bx= mat add, bp= persp buf add
	push	cx
	push	si
	push	bx
	push	bp
qrot_1:
	mov	ax,ES:[si].C1	;x
;check for matrix change
	cmp	ax,8000h
	jb	@F
	cmp	ax,8010h
	jb	qrot_chgmat	;8000h-800fh= matrix change LO 4 bits= mat #
@@:
	mov	cx,ES:[si].C2	;y
	mov	si,ES:[si].C3	;z
;do i contributions first
	mov	bp,ax		;save x
	imul	WORD PTR [bx].R1.C1		;x*r1c1
	mov	[di].C1LO,ax
	mov	[di].C1HI,dx
	mov	ax,bp		;x*r1c2
	imul	WORD PTR [bx].R1.C2
	mov	[di].C2LO,ax
	mov	[di].C2HI,dx
	mov	ax,bp		;x*r1c3
	imul	WORD PTR [bx].R1.C3
	mov	[di].C3LO,ax
	mov	[di].C3HI,dx
;now j
	mov	ax,cx		;y*r2c1
	imul	WORD PTR [bx].R2.C1
	add	[di].C1LO,ax
	adc	[di].C1HI,dx
	mov	ax,cx		;y*r2c2
	imul	WORD PTR [bx].R2.C2
	add	[di].C2LO,ax
	adc	[di].C2HI,dx
	mov	ax,cx		;y*r2c3
	imul	WORD PTR [bx].R2.C3
	add	[di].C3LO,ax
	adc	[di].C3HI,dx
;now k
	mov	ax,si		;z*r3c1
	imul	WORD PTR [bx].R3.C1
	add	ax,WORD PTR rotrelpt.C1LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C1HI
	add	[di].C1LO,ax
	adc	[di].C1HI,dx

	mov	ax,si		;z*r3c2
	imul	WORD PTR [bx].R3.C2
	add	ax,WORD PTR rotrelpt.C2LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C2HI
	add	[di].C2LO,ax
	adc	[di].C2HI,dx

	mov	ax,si		;z*r3c3
	imul	WORD PTR [bx].R3.C3
	add	ax,WORD PTR rotrelpt.C3LO 	;add the relative position-rotated
	adc	dx,WORD PTR rotrelpt.C3HI
	add	[di].C3LO,ax
	adc	[di].C3HI,dx
	pop	bx			;persp address-was in bp!!!
	push	bx			;or sub	sp,2
	call	Persp
	pop	bp
	pop	bx
	pop	si
	pop	cx
	add	bp,8		;next persp buf
	add	di,16		;next rot buf
	add	si,6		;next points
	ret
;
	IF	TOOL3D

parfix:	mov	ch,cl
	mov	cl,10h
	sub	cl,ch
	mov	ax,[di].C1HI
	shl	ax,cl
	add	ax,TileX
	cwd
	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	add	ax,GXMID
	adc	dx,0
	mov	SX[si],ax
	mov	SX[si+2],dx

	mov	ax,[di].C2HI
	shl	ax,cl
	add	ax,TileY
	cwd
	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx

	add	ax,GYMID
	adc	dx,0
	mov	SY[si],ax
	mov	SY[si+2],dx
	ret

;
ParPersp:
	mov	si,WORD PTR cnt
	inc	WORD PTR cnt
	shl	si,1
	shl	si,1

	mov	cl,parallel
	cmp	cl,10h
	jge	@F
	jmp	parfix
@@:	and	cl,0Fh
	mov	ax,[di].C1HI
	sar	ax,cl
	jnc	@F
	inc	ax
@@:	add	ax,TileX
	cwd
	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	add	ax,GXMID
	adc	dx,0
	mov	SX[si],ax
	mov	SX[si+2],dx

	mov	ax,[di].C2HI
	sar	ax,cl
	jnc	@F
	inc	ax
@@:	add	ax,TileY
	cwd
	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx

	add	ax,GYMID
	adc	dx,0
	mov	SY[si],ax
	mov	SY[si+2],dx
	ret

	ELSE

parfix:	mov	ch,cl
	mov	cl,10h
	sub	cl,ch
	mov	ax,[di].C1HI
	shl	ax,cl
	cwd
	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	mov	ax,[di].C2HI
	shl	ax,cl
	cwd
	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx
	ret


ParPersp:
	mov	cl,parallel
	cmp	cl,10h
	jge	@F
	jmp	parfix
@@:	and	cl,0Fh
	mov	ax,[di].C1HI
	sar	ax,cl
	jnc	@F
	inc	ax
@@:	cwd
	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	mov	ax,[di].C2HI
	sar	ax,cl
	jnc	@F
	inc	ax
@@:	cwd
	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx
	ret

	ENDIF
;
Persp:
;di= add of rotated pt, bx= dest persp address


	cmp	parallel,0
	jne	ParPersp



	mov	cx,[di].C3HI		;get hi word of z val

	cmp	cx,100h
	jl	prsp_zlo

;z is good- no underflow potential
	mov	dx,[di].C1LO+1
	mov	al,[di].C1HI+1
	cbw
	xchg	ax,dx
	idiv	cx		;dx= x result
	cwd

	or	ax,ax

	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	mov	dx,[di].C2LO+1
	mov	al,[di].C2HI+1
	cbw
	xchg	ax,dx
	idiv	cx		;dx= x result
	cwd

	or	ax,ax

	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx

	ret
prsp_zlo:
	cmp	cx,1
	jl	prsp_badz		;flag as bad z
;potentialy small z
;get some more z precision
	mov	cx,[di].C3LO+1

	shr	cx,1			;make sure sign bit is ok

;dont byte walk down x,y

	mov	dx,[di].C1HI
	mov	ax,[di].C1LO
	sar	dx,1
	rcr	ax,1

	idiv	cx
	cwd

	or	ax,ax

	mov	[bx].C1LO,ax
	mov	[bx].C1HI,dx

	mov	dx,[di].C2HI
	mov	ax,[di].C2LO
	sar	dx,1
	rcr	ax,1
	idiv	cx
	cwd

	or	ax,ax

	mov	[bx].C2LO,ax
	mov	[bx].C2HI,dx

	ret
prsp_badz:
	mov	[bx].C1HI,08000h	;mark as bad persp-needs z clip
	ret
;
DivOverflow:
; overflow routine
	push	bp
	mov	bp,sp
	add	WORD PTR [bp+2],3	;fix up return address

	xchg	ax,dx
	mov	dl,dh
	mov	dh,al
	mov	al,ah
	cbw
	xchg	ax,dx
        jcxz    @F      ; this really should never happen, but it does
	idiv	cx
	cwd
	mov	dl,ah
	mov	ah,al
	sub	al,al
	pop	bp
	iret

@@:     sub     ax,ax
        sub     dx,dx
        pop     bp
        iret
;
;

Clip3DNew	PROC X1,X1HI,Y1,Y1HI,Z1,Z1HI,X2,X2HI,Y2,Y2HI,Z2,Z2HI

clp_n1:
	mov	ax,X1
	mov	dx,X1HI
	add	ax,X2
	adc	dx,X2HI
	add	ax,1
	adc	dx,0
	sar	dx,1
	rcr	ax,1
	mov	WORD PTR XM,ax
	mov	WORD PTR XM+2,dx

	mov	ax,Y1
	mov	dx,Y1HI
	add	ax,Y2
	adc	dx,Y2HI
	add	ax,1
	adc	dx,0
	sar	dx,1
	rcr	ax,1
	mov	WORD PTR YM,ax
	mov	WORD PTR YM+2,dx

	mov	ax,Z1
	mov	dx,Z1HI
	add	ax,Z2
	adc	dx,Z2HI
	add	ax,1
	adc	dx,0
	sar	dx,1
	rcr	ax,1

	mov	cx,dx
	cmp	cx,1
	jbe	clp_n4

	dec	dx
	js	clp_n3

	inc	dx
	mov	Z1,ax
	mov	Z1HI,dx
	mov	ax,WORD PTR XM
	mov	dx,WORD PTR XM+2
	mov	X1,ax
	mov	X1HI,dx
	mov	ax,WORD PTR YM
	mov	dx,WORD PTR YM+2
	mov	Y1,ax
	mov	Y1HI,dx
	jmp	clp_n2
clp_n3:
	inc	dx
	mov	Z2,ax
	mov	Z2HI,dx
	mov	ax,WORD PTR XM
	mov	dx,WORD PTR XM+2
	mov	X2,ax
	mov	X2HI,dx
	mov	ax,WORD PTR YM
	mov	dx,WORD PTR YM+2
	mov	Y2,ax
	mov	Y2HI,dx
clp_n2:
	jmp	clp_n1
clp_n4:
	ret
Clip3DNew	ENDP



;
ClipLines:
;ES:[si]= # lines
	lea	bp,DS:linebuf		;get base of line clip records
	mov	cx,ES:[si]
	add	si,2
	jcxz	clpl_out		;thats it

clp_3:
;cx= num of lines
;ES:[si]= pt1,pt2 index of points built above
;bp = clip record destination
	mov	di,ES:[si]		;get pt1,pt2
	add	si,2
	mov	bx,di  			;pt1= di lo pt2 = bh
	mov	bl,bh
	and	di,0ffh			;di = pt1 index
	and	bx,0ffh			;bx = pt2 index
	cmp	numsurfs,0		;all front facing ?
	je	clp_1
	mov	al,pointsfb[di]		;get first pt fb
	and	al,pointsfb[bx]		;both pts fb == front ?
	je	clp_2			;no-do next line
clp_1:
	shl	di,1
	shl	bx,1
	shl	di,1
	shl	bx,1
	shl	di,1			;di = persp buf of pt1
	shl	bx,1			;bx = persp buf of pt 2
; now clip from bx, di to bp -z clip if necessary
	push	cx
	push	bp
	push	si
	call	Clip3D
	pop	si
	pop	bp
	pop	cx
clp_2:
	add	bp,CRECLEN		;22	;next clip rec
	loop	clp_3
clpl_out:
	ret
;
;
clpz_pt2:
;clip pt 2 z
;put pt 1 into si record pt1=di, pt2=bx
	mov	ax,perspbuf[di].C1LO		;get lo of x
	mov	dx,perspbuf[di].C1HI		;get hi
	add	ax,GXMID
	adc	dx,0
	mov	[si].CX1,ax		;put at pt 1 dest !!!
	mov	[si].CX1HI,dx
	mov	ax,perspbuf[di].C2LO		;get lo of y1
	mov	dx,perspbuf[di].C2HI		;get hi
	add	ax,GYMID
	adc	dx,0
	mov	[si].CY1,ax		;put at pt 1 dest !!!
	mov	[si].CY1HI,dx
;now z clip to z=1-no persp that way
;bx= pt 2-in persp buf-get in rotated buffer !!!!
	shl	di,1			;rot buf = 2 *persp buf
	shl	bx,1			;pt 2 index into rotate

; new way to try

	push	rotatebuf[bx].C3HI
	push	rotatebuf[bx].C3LO
	push	rotatebuf[bx].C2HI
	push	rotatebuf[bx].C2LO
	push	rotatebuf[bx].C1HI
	push	rotatebuf[bx].C1LO

	push	rotatebuf[di].C3HI
	push	rotatebuf[di].C3LO
	push	rotatebuf[di].C2HI
	push	rotatebuf[di].C2LO
	push	rotatebuf[di].C1HI
	push	rotatebuf[di].C1LO

	call	Clip3DNew
	add	sp,24

	mov	ax,WORD PTR XM+2
	mov	dx,WORD PTR XM
	WALKDWN

	shl	ax,1
	rcl	dx,1

	add	ax,GXMID
	adc	dx,0
	mov	[si].CX2,ax
	mov	[si].CX2HI,dx
	mov	[si].CZX1,ax  		;plus at the z clip info
	mov	[si].CZX1HI,dx

	mov	ax,WORD PTR YM+2
	mov	dx,WORD PTR YM
	WALKDWN

	shl	ax,1
	rcl	dx,1

	add	ax,GYMID
	adc	dx,0
	mov	[si].CY2,ax
	mov	[si].CY2HI,dx
	mov	[si].CZY1,ax  		;plus at the z clip info
	mov	[si].CZY1HI,dx

	call	Clip32
	mov	al,CLIPPEDZ
	mov	ah,al
	or	[si].CFLAG,ax		;mark as zclipped-pt2
	ret
;
Clip3D:
	mov	si,bp			;dest of clip record
	mov	ax,perspbuf[di].C1HI	;hi of x persp pt 1
	mov	dx,perspbuf[bx].C1HI	;hi of x persp pt 2
	cmp	ax,8000h		;bad z
	je	clp_4
	cmp	dx,8000h
	jne	@F
	jmp	clpz_pt2
@@:
;clip the line from [di], [bx], to [si]
	lea	ax,perspbuf
	add	bx,ax
	add	di,ax			;points at actual persp buf entry

	mov	ax,[di].C1LO
	mov	dx,[di].C1HI
	add	ax,GXMID
	adc	dx,0
	mov	[si].CX1,ax
	mov	[si].CX1HI,dx
	mov	ax,[di].C2LO
	mov	dx,[di].C2HI
	add	ax,GYMID
	adc	dx,0
	mov	[si].CY1,ax
	mov	[si].CY1HI,dx

	mov	ax,[bx].C1LO
	mov	dx,[bx].C1HI
	add	ax,GXMID
	adc	dx,0
	mov	[si].CX2,ax
	mov	[si].CX2HI,dx
	mov	ax,[bx].C2LO
	mov	dx,[bx].C2HI
	add	ax,GYMID
	adc	dx,0
	mov	[si].CY2,ax
	mov	[si].CY2HI,dx
	call	Clip32			;clip to screen
	ret
clp_badZ:
	mov	al,BADLINE
	mov	ah,al
	mov	[si].CFLAG,ax
	ret
clp_4:
	cmp	dx,8000h		;bad 2 also ?
	je	clp_badZ
clpz_pt1:
;clip pt 1 z
;put pt 2 into si record pt1=di, pt2=bx
	mov	ax,perspbuf[bx].C1LO		;get lo of x
	mov	dx,perspbuf[bx].C1HI		;get hi
	add	ax,GXMID
	adc	dx,0
	mov	[si].CX2,ax			;put at pt 2 dest !!!
	mov	[si].CX2HI,dx
	mov	ax,perspbuf[bx].C2LO		;get lo of y1
	mov	dx,perspbuf[bx].C2HI		;get hi
	add	ax,GYMID
	adc	dx,0
	mov	[si].CY2,ax		;put at pt 2 dest !!!
	mov	[si].CY2HI,dx
;now z clip to z=1-no persp that way
;di= pt 1-in persp buf-get in rotated buffer !!!!
	shl	di,1			;rot buf = 2 *persp buf
	shl	bx,1			;pt 2 index into rotate

; new way to try

	push	rotatebuf[di].C3HI
	push	rotatebuf[di].C3LO
	push	rotatebuf[di].C2HI
	push	rotatebuf[di].C2LO
	push	rotatebuf[di].C1HI
	push	rotatebuf[di].C1LO

	push	rotatebuf[bx].C3HI
	push	rotatebuf[bx].C3LO
	push	rotatebuf[bx].C2HI
	push	rotatebuf[bx].C2LO
	push	rotatebuf[bx].C1HI
	push	rotatebuf[bx].C1LO

	call	Clip3DNew
	add	sp,24

	mov	ax,WORD PTR XM+2
	mov	dx,WORD PTR XM
	WALKDWN

	shl	ax,1
	rcl	dx,1

	add	ax,GXMID
	adc	dx,0
	mov	[si].CX1,ax
	mov	[si].CX1HI,dx
	mov	[si].CZX1,ax  		;plus at the z clip info
	mov	[si].CZX1HI,dx

	mov	ax,WORD PTR YM+2
	mov	dx,WORD PTR YM
	WALKDWN

	shl	ax,1
	rcl	dx,1

	add	ax,GYMID
	adc	dx,0
	mov	[si].CY1,ax
	mov	[si].CY1HI,dx
	mov	[si].CZY1,ax  		;plus at the z clip info
	mov	[si].CZY1HI,dx

	call	Clip32
	mov	al,CLIPPEDZ
	mov	ah,al
	or	[si].CFLAG,ax		;mark as zclipped-pt1
	ret
;
;





;
DrawSurfs:
;ES:[si]= bsplist ptr or surf defs if no bsp
	mov	cx,numsurfs
	or	cx,cx			;0= no surfs,pos= no bsp
	js	drws_bsp		;bsp ordering
	jne	drws_all_front
	inc	cx			;1 for loop instruction
drws_all:
;ES:[si]= gop list
	push	cx
	call	DrawGops		;draw all gops for that surfaces
	pop	cx
	loop	drws_all
	ret
drws_all_front:
;ES:[si]= gop PTRS table
	sub	bx,bx
@@:
	push	cx
	call	drws_num
	pop	cx
	inc	bx
	loop	@B
	ret
drws_bsp:
;ES:[si] = gops ptrs
;polyorder= byte list -1 terminated
	lea	di,polyorder		;get list
@@:
	mov	bl,[di]
	inc	di
	cmp	bl,-1		;that it ?
	je	@F
	sub	bh,bh
	call	drws_num	;bl= surf num, ES:[si]= gop table
	jmp	@B		;next surf #
@@:
	ret
drws_num:
;ES:[si]= PTRS to GOPs, bh= # of surf to draw
	cmp	BYTE PTR frontback[bx],07fh	;hi bit=0= not drawn
	jbe	@F				;not drawn

	mov	al,BYTE PTR lightshade[bx]	;get intensity
	mov	LightValue,al

	push	di
	push	bx
	push	si
	shl	bx,1
	mov	si,ES:[si+bx]	;get gop ptr
	call	DrawGops		;draw all gops for that surfaces
	pop	si
	pop	bx
	pop	di
@@:
	ret
;
;
drwg_out:
	ret
drwg_anim:
;al AND 7fh= anim var #, BYTE PTR ES:[si]=value
	and	ax,07fh
	mov	bx,ax
	shl	bx,1		;word vars
	add	bx,animptr	;+ base = var address
	mov	al,ES:[si]
	inc	si
	cbw
	cmp	ax,[bx]		;value = var ?
	je	DrawGops	;yes-draw the next Gop
;skip past this Gop
	mov	al,ES:[si]
	inc	si
	cmp	al,-1
	je	drwg_out	;thats it
	and	al,01fh		;get # of pts,lines,polys
	sub	ah,ah
	inc	ax		;plus color byte
	add	si,ax		;
;fall	thru to next Gop
DrawGops:
;ES:[si]= gop
;gop bits set bit 7=line, bit 6 =dot, bits0-5  = # of things
	mov	al,ES:[si]
	inc	si
	cmp	al,-1		;that it ?
	je	drwg_out	;yes
	test	al,080h		;animation flag ?
	jne	drwg_anim	;yes-decide to do it or skip it
	test	al,20h		;dots ?
	je	@F
	jmp	drwg_dots	;yes
@@:
	test	al,40h		;lines ?
	jne	drwg_lines	;yes
;must be a poly
;al= #polylines, next bytes=line#,line#,line#..., color
	mov	cl,al
	sub	ch,ch

	mov	outlinenum,cx
	mov	savesi,si

@@:
;check for -1 case
	mov	bx,cx
	cmp	BYTE PTR ES:[si+bx],-1	;bogus fill color ?
	jne	@F
;skip past the lines,fill color
	add	si,bx
	inc	si
	jmp	DrawGops
@@:
	mov	bl,ES:[si]	;get line index
	inc	si
	sub	bh,bh
	shl	bx,1		;*24 for clip record
	shl	bx,1
	shl	bx,1
	mov	ax,bx
	shl	bx,1
	add	bx,ax		;+16+8=*24
	push	cx
	push	si
	lea	si,linebuf
	add	si,bx
	call	PLine
	pop	si
	pop	cx
	loop	@B
;now fill the poly
	mov	al,ES:[si]
	inc	si
	push	es
	push	si

;	IF STANDARD
;	mov	bx,ClrSwapPtr3D		;for indivdual color schemes
;	xlat
;	ENDIF

	mov	bx,PalPtr3D
	sub	ah,ah
	shl	ax,1
	add	bx,ax
	mov	ax,[bx]

;
;	cmp	OutLine,0
;	jne	dooutlines
;
;
;	cmp	GMODE,'M'
;	jne	@F
;
;	mov	ah,LightValue
;
;
;	push	ax		;send color parameter
;	call	ShadeFunction
;	add	sp,2
;@@:

	push	ax
	call	FillPoly
	add	sp,2		;clean up
	pop	si
	pop	es
	jmp	DrawGops
drwg_lines:
;ES:[si]= color,line#,line#,...
;al= # of lines
	and	al,01fh		;mask off any line,dot, etc bits
	mov	cl,al
	sub	ch,ch
	mov	al,ES:[si]	;get color
	inc	si

;	IF STANDARD
;	mov	bx,ClrSwapPtr3D		;for indivdual color schemes
;	xlat
;	ENDIF
;

	mov	bx,PalPtr3D
	sub	ah,ah
	shl	ax,1
	add	bx,ax
	mov	ax,[bx]

;	mov	bx,PalPtr3D
;	xlat


	push	es		;es gets messed up by setcolor-on purpose!
	call	SetColor	;set up the color
	pop	es
drwg_l1:
	mov	bl,ES:[si]	;get line index
	inc	si
	sub	bh,bh
	shl	bx,1		;*24 for clip record
	shl	bx,1
	shl	bx,1
	mov	ax,bx		;+8
	shl	bx,1
	add	bx,ax		;+16+8=24
	push	si
	lea	si,linebuf
	add	si,bx
	mov	ClipPtr,si
	mov	ax,[si].CFLAG		;get draw flags
	test	al,BADLINE		;line flagged as bad?
	jne	@F			;dont draw
	push	es
	push	cx
	call	Line
	pop	cx
	pop	es
@@:
	pop	si
	loop	drwg_l1
	jmp	DrawGops
;
dooutlines:
;es,si are on stack
;	push	ax		;send color parameter
;	call	ShadeFunction
;	add	sp,2

	push	ax
	call	FillPoly
	pop	ax		;ah = outline color
	pop	si
	pop	es

	push	si
	mov	si,savesi	;repoint at line indicies
	mov	cx,outlinenum

	mov	al,ah		;ah = color from shadefunction
	sub	ah,ah
	push	es		;es gets messed up by setcolor-on purpose!
	call	SetColor	;set up the color
	pop	es
outln_1:
	mov	bl,ES:[si]	;get line index
	inc	si
	sub	bh,bh
	shl	bx,1		;*24 for clip record
	shl	bx,1
	shl	bx,1
	mov	ax,bx		;+8
	shl	bx,1
	add	bx,ax		;+16+8=24
	push	si
	lea	si,linebuf
	add	si,bx
	mov	ax,[si].CFLAG		;get draw flags
	test	al,BADLINE		;line flagged as bad?
	jne	@F			;dont draw
	push	es
	push	cx
	call	Line
	pop	cx
	pop	es
@@:
	pop	si
	loop	outln_1
	pop	si
	jmp	DrawGops
;
;
drwg_dots:
;ES:[si]= color,dot#,dot#,dot#...
	and	al,01fh		;mask off any line,dot, etc bits
	mov	cl,al
	sub	ch,ch
	mov	al,ES:[si]
	inc	si

	mov	bx,PalPtr3D
	sub	ah,ah
	shl	ax,1
	add	bx,ax
	mov	ax,[bx]

	push	ax		;leave color on the stack !!!!!!
	mov	bp,cx		;cnt
drwg_dts1:
	mov	bl,ES:[si]	;get dot index
	inc	si
	sub	bh,bh
	shl	bx,1
	shl	bx,1
	shl	bx,1		;*8 for persp buffer
	lea	ax,perspbuf	;plus base
	add	bx,ax

	mov	dx,[bx].C1HI	;get hi
	cmp	dx,8000h	;bad Z ?
	je	@F		;yes-dont do
	mov	ax,[bx].C1LO	;get lo of x
	add	ax,GXMID
	adc	dx,0
	inc	dx
	cmp	dx,1		;was hi 0 or -1
	ja	@F		;no-do next
	mov	cx,ax		;save x val
	mov	ax,[bx].C2LO	;get lo of y
	mov	dx,[bx].C2HI	;get hi
	add	ax,GYMID
	adc	dx,0
	inc	dx
	cmp	dx,1		;was hi 0 or -1
	ja	@F		;no-do next
;cx= y ax= x
	pop	bx		;get color val

	cmp	bh,250
	jne	drwg_dts_ss
	xor	BYTE PTR dotflash,1
	xor	bl,BYTE PTR dotflash
drwg_dts_ss:

	push	bx		;save it back

	push	es		;save es
	push	si

	push	bx		;color
	push	ax		;y
	push	cx		;x
	call	GDrawDot		;(x,y,color)
	add	sp,6		;clean up

	pop	si
	pop	es		;get back pointer seg
@@:
	dec	bp
	jne	drwg_dts1
	add	sp,2		;clean off color
	jmp	DrawGops
;


;
Bspsort:
;di= dest of poly seq terminated by -1
;bx= list of front back byte info -1 =front, 0 = back
;ES:[si]= bsp tree   root,  frontptr,backptr,frntptr,backptr,...
;if ptr=-1 no children at that node in that direction
;
;follows the tree at ES:[si]
;structure is root ;     frontnode,backnode, frontnode,backnode ,..
;
; iterative -based on bsp tree display algorithm of
; FUCHS ET AL , COMPUTER GRAPHICS VOL 17, NUMBER 3  JULY 1983
;
; if eye infront of node poly then
;	follow tree back ptr
;	display poly (node)
;	follow tree front ptr
; else
;	follow tree front ptr
;	follow tree back ptr
; end
;
; iterative based on keeping "states" at each recursion
;

	MOV	tempbx,BX		;save fr/behind list address

	sub	bp,bp		;initialize recursion level
	MOV	AL,ES:[SI]	;	lodsb			;get root node-point at node ptrs
	ADD	SI,1
	sub	ah,ah
	mov	cx,ax
dostart:
	mov	bx,tempbx	;list add base
	add	bx,cx		;node #+ base = address of fr/behind byte
	mov	al,[bx]		;get front ,back info 0=front ,-1=back
	mov	bx,cx		;node #
	shl	bx,1		;bx= offset to node ptr
	sub	dx,dx		;assume state 0
	cmp	al,0		;foloow back-normal state 0?
	jne	st1		;yes
	mov	dx,2		;state 2 -folow front -dont display
st1:
;do states
dostate:
	cmp	dx,0
	je	state0
	cmp	dx,1
	je	state1
	cmp	dx,2
	je	state2
	cmp	dx,3
	je	state3
	jmp	state4
;
state0:
;follow tree -back ,display,follow tree front,previous node
	mov	dx,1		;state 1 next
	jmp	folback
;
state1:
;dsiplay,follow tree front, previous node

; display info here-cx= # of node = # of poly to put in list

	mov	[di],cl		;byte list
	inc	di		;next ptr

	mov	dx,4		;do prev node next
	jmp	folfront
;
state2:
;follow tree front, follow tree back, previous node
	mov	dx,3		;state 3 next
	jmp	folfront
;
state3:
;follow tree back, previous node
	mov	dx,4		;prev next
	jmp	folback
;
state4:
;previous node
;
	sub	bp,1		;recursion counter
	js	fin_all	;just finished root -leave
	pop	ax		;get node and state -ah=node al=state
	mov	dl,al 		;turn state to word
	sub	dh,dh
	mov	cl,ah		;turn node to word
	sub	ch,ch
	mov	bx,cx		;get node ptr offset
	shl	bx,1
	jmp	dostate	;go do the node at this state
fin_all:
	mov	byte ptr [di],-1	;end flag
	ret
;
folfront:
;if a node at front ptr then push state,node and start again
;else do "next" state -already set up and dostate
;
	mov	al,ES:[bx+si]	;front ptr word node val
	cmp	al,-1		;exist?
	je	flf1		;no
	jmp	nextstart	;positive=reg poly-push info and go one deeper node
;	or	al,al		;find out if special moveing object
;	jns	nextstart	;positive=reg poly-push info and go one deeper node
;	mov	[di],al		;byte list -neg = moving object
;	inc	di		;next ptr- act like didnt exist
flf1:
	jmp	dostate
folback:
;similar to folfront but foloow back ptr
	mov	al,ES:[bx+si+1]	;back ptr word node val
	cmp	al,-1		;exist?
	je	flf1		;no
nextstart:
;push state and nde info -start again
;ax= next node to be cx= this node dx= this state
	sub	ah,ah
	xchg	ax,cx		;cx= new node
	mov	ah,al		;save node in ah
	mov	al,dl		;state to al
	push	ax
	inc	bp		;recusion level
	jmp	dostart	;next deeper node
;
;
;
;
GetObjBox	PROC USES es si di, OBJ:WORD,DEST:PTR

	mov	di,DEST
	mov	WORD PTR [di],0
	mov	WORD PTR [di+2],0
	mov	WORD PTR [di+4],0

	mov	ax,Seg3D	 ;get seg of 3d objects far ptrs
	mov	es,ax
	sub	si,si
	mov	ax,ES:[si]	;get # of objects 1 based
	cmp	ax,OBJ
	ja	@F
	ret
@@:
	mov	bx,OBJ			;get object #
	shl	bx,1
	mov	si,ES:[bx+2]			;get address of object
;ES:si points to dist,ptr,dist,ptr,etc
	mov	ax,ES:[si]		;get dist val
	mov	si,ES:[si+2]		;get ptr val
;ES[si]= object
	mov	ax,ES:[si]		;get radius
	add	si,2
	mov	cx,ES:[si]
	add	si,2			;get # matrices
	call	FigMatrices		;set up dump pts,matrix address
;ES:[si]= # points
	mov	ax,ES:[si]		;get # points
	add	si,2
	mov	numpoints,ax
;ES:[si]= # surfaces
	push	di
	call	FigFrontBack		;set the surface nums and figure f/b

;ES:si=add of, x,y,z, x,y,z, ...
	pop	di
	mov	cx,numpoints	;get # of points
	cmp	cx,0		;positive = # of points, neg means standard
	jle	gtb_out		;standard or something

gtb_lp:
	mov	ax,ES:[si]	;x
	add	si,2
;check for matrix change
	cmp	ax,8000h
	jb	@F
	cmp	ax,8010h
	jb	gtb_lp
@@:
	ABSAX
	cmp	ax,[di]
	jbe	@F
	mov	[di],ax
@@:
	mov	ax,ES:[si]	;y
	add	si,2
	ABSAX
	cmp	ax,[di+2]
	jbe	@F
	mov	[di+2],ax
@@:
	mov	ax,ES:[si]	;z
	add	si,2
	ABSAX
	cmp	ax,[di+4]
	jbe	@F
	mov	[di+4],ax
@@:
	loop	gtb_lp
gtb_out:
	ret
GetObjBox	ENDP

;


;*******************************************************************

RotStandard	PROC	USES di si es bp
	mov	ax,Seg3D
	mov	es,ax
	sub	si,si
	mov	si,ES:[si]		;get # of objs
	shl	si,1
	add	si,2			;plus # obj word
	mov	si,ES:[si]		;get pointer to # of standard points
	mov	cx,ES:[si]		;get # of points
	add	si,2			;points follow
;ES:si=add of, x,y,z, x,y,z, ...
	sub	di,di		;start at first point buf index

	lea	bx,viewmat

	cmp	cx,0		;positive = # of points, neg means standard
	je	rts_out		;no points !!- wierd
	lea	di,standardbuf	;get dest address for rotated pts
;now rotate all points
@@:
	push	cx

	push	si

	mov	ax,ES:[si].C1	;x
	mov	cx,ES:[si].C2	;y
	mov	si,ES:[si].C3	;z
;do i contributions first
	mov	bp,ax		;save x
	imul	WORD PTR [bx].R1.C1		;x*r1c1
	mov	[di].C1LO,ax
	mov	[di].C1HI,dx
	mov	ax,bp		;x*r1c2
	imul	WORD PTR [bx].R1.C2
	mov	[di].C2LO,ax
	mov	[di].C2HI,dx
	mov	ax,bp		;x*r1c3
	imul	WORD PTR [bx].R1.C3
	mov	[di].C3LO,ax
	mov	[di].C3HI,dx
;now j
	mov	ax,cx		;y*r2c1
	imul	WORD PTR [bx].R2.C1
	add	[di].C1LO,ax
	adc	[di].C1HI,dx
	mov	ax,cx		;y*r2c2
	imul	WORD PTR [bx].R2.C2
	add	[di].C2LO,ax
	adc	[di].C2HI,dx
	mov	ax,cx		;y*r2c3
	imul	WORD PTR [bx].R2.C3
	add	[di].C3LO,ax
	adc	[di].C3HI,dx
;now k
	mov	ax,si		;z*r3c1
	imul	WORD PTR [bx].R3.C1
	add	[di].C1LO,ax
	adc	[di].C1HI,dx

	mov	ax,si		;z*r3c2
	imul	WORD PTR [bx].R3.C2
	add	[di].C2LO,ax
	adc	[di].C2HI,dx

	mov	ax,si		;z*r3c3
	imul	WORD PTR [bx].R3.C3
	add	[di].C3LO,ax
	adc	[di].C3HI,dx

	pop	si
	pop	cx

	add	di,16		;next rot buf
	add	si,6		;next points
	loop	@B
rts_out:
	ret
RotStandard	ENDP
;*******************************************************************






.DATA

	IF	STANDARD
standardbuf	dw	256*4*2 dup(0)		;256 32 bit x1,y1,z1,xtra

	ELSE
standardbuf	label	word
	ENDIF


divbyzerovec	dd	0			;divide by zero vector



;rotatebuf	dw	256*4*2 dup(0)		;256 32 bit x1,y1,z1,xtra
;perspbuf	dw	256*2*2 dup(0)		;256 32 bit xp,yp
linebuf		dw	256*12	dup(0)		;256 24 byte clip records
xtracliprec	dw	12 dup(0)		;for z clipping use
frontback	db	256	dup(0)		;front back info
lightshade	db	256	dup(0)		;front back info
polyorder	db	256	dup(-1)		;draw order
pointsfb	db	258	dup(0)		;front/back 258 for stosw!!
identitymat	dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

viewmat		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh ;view and inverse matrixes
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

concatmats	dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh	;area for concatinating mats
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh

dumppts		dw	8*3	dup(0)		;4 max dump points-for f/b

lightdumppts	dw	8*3	dup(0)		;4 max dump points-for f/b


relobjpt	dw	0,0,0			;x,y,z rel of object
rotrelpt	dd	0,0,0			;x,y,z "rotated" for reject-long
ptsoff		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
		dd	0,0,0
temprot		dd	0,0,0
rotpointptr	dw	standardbuf		;add of rotated pool
objmatptr	dw	identitymat		;add of object transform mat
animptr		dw	animlist		;address of animation list
rotmatptr	dw	viewmat			;address of concat matrix
pointlistptr	dw	0			;address of points/surf list
linelistptr	dw	0			;addres of lines/surf list
ptsoffptr	dw	0			;temp address holder
nummats		dw	0			;# of mats -1= standard 0= view
numsurfs	dw	0			;# of surfaces in object
numpoints	dw	0			;# of surfaces in object,<0=standard
zumefactor	dw	7fffh			;amount to scale z val by

animlist	dw	0,0,0,0			;animation values
surfskip	db	-1,-1			;for surf anim-padded to word

dotflash	db	0,0

NOASPECT	dw	0

OutLine		dw	0
outlinenum	dw	0
savesi		dw	0

XM		dd	0
YM		dd	0


tempbx		dw	0
templightptr	dw	0
;
;
ClipPtr		dw	0
PalPtr3D	dw	defaultpal
ClrSwapPtr3D	dw	defaultpal

LightX	dw	0
LightY	dw	64
LightZ	dw	0
LightValue	db	0,0
CRASH		db	0,0
parallel	db	0,0

	IF	TOOL3D

cnt		db	0,0

	ENDIF


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


Seg3D		dw	SEG bogusseg

ALIGN 16				;put at paragraph
bogusseg 	dw	0

	END


