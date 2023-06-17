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

include	clip.equ
include	graph.ext



;***********************************************************
;	c type calls
;
;	GDrawLine(x1,y1,x2,y2,COLOR); PolyLine(x1,y1,x2,y2);
;	FillPoly(color);
;
;***********************************************************
;	assembly type calls
;
;	Clip32:si= clip record Clip16:si=clip record
;	DLine:si= record-draws line PLine:si= record-draws poly line
;
;************************************************************



	extrn	linebuf:word



	PUBLIC	GDrawLine, PolyLine, FillPoly

	PUBLIC	Clip32, Clip16, DLine, PLine

.CODE
;
PolyLine	PROC USES si, XL:WORD,YL:WORD,XH:WORD,YH:WORD
	lea	si,linebuf
	mov	ax,XL
	cwd
	mov	[si].CX1,ax
	mov	[si].CX1HI,dx
	mov	ax,YL
	cwd
	mov	[si].CY1,ax
	mov	[si].CY1HI,dx
	mov	ax,XH
	cwd
	mov	[si].CX2,ax
	mov	[si].CX2HI,dx
	mov	ax,YH
	cwd
	mov	[si].CY2,ax
	mov	[si].CY2HI,dx
	call	FAR PTR Clip32
	call	FAR PTR Pline
	ret
PolyLine	ENDP

PLine	PROC
;[si]= clip record
	mov	ax,[si].CFLAG		;get flags

	or	pflag,ax		;save the info for the fill

;check for any z clipped and add to zclip line
	mov	ax,[si].CFLAG		;get flags
	test	al,CLIPPEDZ		;any z clips ?
	je	@F			;no
;
	call	addzpolyline
@@:

	mov	ax,[si].CFLAG		;get flags
	test	al,BADLINE		;main bad?
	jne	drcp_5			;yes main is bad
	or	word ptr goodlftrt,4	;for fill sides later

	call	FLine			;fill from record

	mov	ax,[si].CFLAG		;get flags
	test	al,(XTRA1RT OR XTRA1LFT)	;pt1 got any xtra's?
	je	drcp_2				;no
	mov	bx,[si].CY1		;y of pt1
	mov	cx,[si].C1XTRA		;y xtra of pt 1
	test	al,XTRA1LFT		;is it off left?
	jne	drcp_1			;yes
	call	rightpolyline
	jmp	drcp_2
drcp_1:
	call	leftpolyline
drcp_2:
	test	al,(XTRA2RT OR XTRA2LFT)	;pt1 got any xtra's?
	je	drcp_4			;no
	mov	bx,[si].CY2		;y of pt2
	mov	cx,[si].C2XTRA		;y xtra of pt 2
	test	al,XTRA2LFT
	jne	drcp_3			;off left
	call	rightpolyline
	jmp	drcp_4
drcp_3:
	call	leftpolyline
drcp_4:
;
drcp_10:
	ret
;

drcp_5:
	test	al,(XTRA1RT OR XTRA1LFT)	;off on an x side?
	je	drcp_4			;nope leave
	mov	bx,[si].C1XTRA
	mov	cx,[si].C2XTRA
	test	al,XTRA1LFT
	jne	drcp_6		; off left
	call	rightpolyline
	jmp	drcp_4
drcp_6:
	call	leftpolyline
	jmp	drcp_4
PLine	ENDP
;
;
leftpolyline:
;bx=y1 cx=y2 of left edge to fill on poly
	cmp	cx,bx
	jge	flp_1
	xchg	cx,bx
flp_1:
	cmp	cx,lefthi
	jl	flp_2
	mov	lefthi,cx
flp_2:
	cmp	bx,leftlo
	jge	flp_3
	mov	leftlo,bx
flp_3:
	ret
;
rightpolyline:
;bx=y1 cx=y2 of left edge to fill on poly
	cmp	cx,bx
	jge	frp_1
	xchg	cx,bx
frp_1:
	cmp	cx,righthi
	jl	frp_2
	mov	righthi,cx
frp_2:
	cmp	bx,rightlo
	jge	frp_3
	mov	rightlo,bx
frp_3:
	ret
;
addzpolyline:
	lea	bx,zlinebuf
	cmp	zhit,0
	je	@F
	add	bx,CX2			;pt 2-dest in zclip rec
@@:
	inc	zhit
;move [si].z stuff to bx
	mov	ax,[si].CZX1
	mov	[bx].CX1,ax
	mov	ax,[si].CZX1HI
	mov	[bx].CX1HI,ax
	mov	ax,[si].CZY1
	mov	[bx].CY1,ax
	mov	ax,[si].CZY1HI
	mov	[bx].CY1HI,ax
	ret
;
;

FillPoly	PROC	COLOR:WORD
;if any good add left and right parts
; else left and right only if both are non virgin
;add the sides if applicable

	mov	ax,COLOR
	push	ax
	call	SetColor
	add	sp,2

	cmp	zhit,2		;2 zclip pts= a line to draw
	jne	@F		;nope
;draw the line at zlinebuf
	push	si
	lea	si,zlinebuf
	call	FAR PTR Clip32
	call	FAR PTR PLine
	pop	si
@@:

	mov	dx,goodlftrt	;get any good lines flag

	or	dx,dx		;any good lines drawn?
	je	fp_1		;no only add sides if left and right
	mov	ax,pflag	;get clip info
	test	al,(XTRA1RT OR XTRA2RT)		;any right parts?
	je	fp_2		;nope try left
;add right side
	call	fp_addrt
fp_2:
	mov	ax,pflag
	test	al,(XTRA1LFT OR XTRA2LFT)	;left side?
	je	fp_3			;nope thats it
;add left side
	call	fp_addlft
fp_3:
	mov	ax,COLOR
;	cmp	al,-2
;	jne	fp_10_1
;	call	PLight
;	jmp	fp_6
;fp_10_1:
	call	PFill			;fill from table

fp_6:
	mov	ax,7fffh		;max int
	mov	rightlo,ax
	mov	leftlo,ax
	not	ax
	mov	righthi,ax
	mov	lefthi,ax
	sub	ax,ax
	mov	goodlftrt,ax
	mov	pflag,ax
	mov	zhit,ax
	ret
FillPoly	ENDP
;
fp_1:
;no goods-only fill if both left and right
	mov	ax,pflag
	test	al,(XTRA1RT OR XTRA2RT)		;right part?
	je	fp_6				;nope leave
	test	al,(XTRA1LFT OR XTRA2LFT)	;left also?
	je	fp_6				;nope leave
	call	fp_addrt
	call	fp_addlft
	jmp	fp_3




fp_addrt:
	mov	ax,GXHI
	inc	ax
	push	ax
	push	rightlo
	push	righthi

	call	VertFline		;(ylo,yhi,xval)
	add	sp,6			;clean up
	ret
fp_addlft:
	mov	ax,GXLO
	dec	ax
	push	ax
	push	leftlo
	push	lefthi

	call	VertFline		;(ylo,yhi,xval)
	add	sp,6			;clean up
	ret
;
;
GDrawLine	PROC USES si, XL:WORD,YL:WORD,XH:WORD,YH:WORD, COLOR:WORD
	mov	ax,COLOR
	push	ax
	call	SetColor
	add	sp,2

	lea	si,linebuf
	mov	ax,XL
	cwd
	mov	[si].CX1,ax
	mov	[si].CX1HI,dx
	mov	ax,YL
	cwd
	mov	[si].CY1,ax
	mov	[si].CY1HI,dx
	mov	ax,XH
	cwd
	mov	[si].CX2,ax
	mov	[si].CX2HI,dx
	mov	ax,YH
	cwd
	mov	[si].CY2,ax
	mov	[si].CY2HI,dx
	call	FAR PTR Clip32
	call	FAR PTR Dline
	ret
GDrawLine	ENDP

Dline	PROC
;[si]= line record
	mov	ax,[si].CFLAG		;get draw flags
	test	al,BADLINE		;line flagged as bad?
	jne	drwm_ss1

	push	es
	call	Line
	pop	es

drwm_ss1:
	ret
Dline	ENDP
;
;
;

Clip16	PROC
	jmp	sclip_16		;try this
Clip16	ENDP	



pinpts:
;pin pts to di+8,di+10 flag at di+12
;al,ah=flags
	push	bx
	mov	bx,cx		;y pt 1
	test	al,UPMSK
	je	lclip25
	mov	bx,GYLO
lclip25:
	test	al,DWNMSK
	je	lclip26
	mov	bx,GYHI
lclip26:
	mov	[si].C1XTRA,bx
	mov	bx,bp		;y pt 2
	test	ah,UPMSK
	je	lclip27
	mov	bx,GYLO
lclip27:
	test	ah,DWNMSK
	je	lclip28
	mov	bx,GYHI
lclip28:
	mov	[si].C2XTRA,bx
	sub	bl,bl
	test	al,RTMSK
	je	lclip20
	or	bl,XTRA1RT
lclip20:
	test	al,LFTMSK
	je	lclip21
	or	bl,XTRA1LFT
lclip21:
	test	ah,RTMSK
	je	lclip22
	or	bl,XTRA2RT
lclip22:
	test	ah,LFTMSK
	je	lclip23
	or	bl,XTRA2LFT
lclip23:
	mov	bh,bl
	mov	[si].CFLAG,bx
	pop	bx
	ret

lclipptok:
	mov	bp,bx
	mov	dx,cx
	clc
	ret
;
lclipclipit:
;bx=x1,cx=y1,al=flag of clip
	or	al,al
	je	lclipptok
;clip x first
	test	al,(LFTMSK OR RTMSK)
	je	lclip12				;do y
	mov	bp,GXHI
	test 	al,LFTMSK	;clip to GXLO?
	je	lclip11		;no
	mov	bp,GXLO
lclip11:
	mov	ax,bp
	sub	ax,bx
	imul	tdely
	idiv	tdelx
	add	ax,cx		;ax=new y bp=edge x
	mov	dx,ax		;dx=new y bp=edge x
	mov	al,UPMSK	;fake off top
	cmp	dx,GYLO
	jl	lclip12		;clip to ylo
	cmp	dx,GYHI
	jg	lclip14		;clip to y hi
	clc
	ret			;new val good
lclip12:
;clip to y
	mov	bp,GYLO
	test	al,UPMSK
	jne	lclip13
lclip14:
	mov	bp,GYHI
lclip13:
	mov	ax,bp
	sub	ax,cx		;delta y
	imul	tdelx
	idiv	tdely
	add	ax,bx		;dx=new x bp=edge y
	mov	dx,ax
	xchg	dx,bp		;dx=new y bp=new x
;test x if bad,then line wierd
	cmp	bp,GXLO
	jl	lclipbadlft
	cmp	bp,GXHI
	jg	lclipbadrt
	clc
	ret
lclipbadrt:
	mov	al,(XTRA1RT OR XTRA2RT)
	mov	ah,al
	stc
	ret
lclipbadlft:
	mov	al,(XTRA1LFT OR XTRA2LFT)
	mov	ah,al
	stc
	ret

;
flagpt:
;bx=x cx=y al=return bp= pin val  
	sub	al,al
	cmp	bx,GXLO
	jo	lclip33			;can only underflow
lclip34:
	jl	lclip1
lclip2:
	cmp	bx,GXHI
	jg	lclip3
lclip4:
	cmp	cx,GYLO
	jo	lclip35			;can only underflow
lclip36:
	jl	lclip5
lclip6:
	cmp	cx,GYHI
	jg	lclip7
	or	al,al
	ret
lclip1:
 	or	al,LFTMSK
	jmp	lclip2
lclip3:
 	or	al,RTMSK
	jmp	lclip4
lclip5:
 	or	al,UPMSK
	jmp	lclip6
lclip7:
 	or	al,DWNMSK
	ret
lclip33:
	mov	byte ptr overflow,1
	jmp	lclip34
lclip35:
	mov	byte ptr overflow,1
	jmp	lclip36
;
;**********************************************
; 32 bit clip routines
;
;
clp32_ok:
	sub	ax,ax
	mov	[si].CFLAG,ax	;line is good -no xtras
	ret
;
reclip32:
;extra entry point for 16 bit pseudo failures
;sign extends and clips 32 bit
	mov	ax,[si].CX1
	cwd
	mov	[si].CX1+2,dx
	mov	ax,[si].CY1
	cwd
	mov	[si].CY1+2,dx
	mov	ax,[si].CX2
	cwd
	mov	[si].CX2+2,dx
	mov	ax,[si].CY2
	cwd
	mov	[si].CY2+2,dx
	jmp	clp32_50		;forced entry

Clip32	PROC
;[si]= x1,y1,x2,y2 of 32 bit vals
;
;figure if 16 bit here
	mov	ax,[si].CX1	;get lo of x1
	cwd
	cmp	dx,[si].CX1+2	;hi bit match hi word?
	jne	clp32_50	;no mot 16 bit for sure
	mov	ax,[si].CX2	;get lo of x2
	cwd
	cmp	dx,[si].CX2+2	;hi bit match hi word?
	jne	clp32_50	;no mot 16 bit for sure
	mov	ax,[si].CY1	;get lo of y1
	cwd
	cmp	dx,[si].CY1+2	;hi bit match hi word?
	jne	clp32_50	;no mot 16 bit for sure
	mov	ax,[si].CY2	;get lo of y2
	cwd
	cmp	dx,[si].CY2+2	;hi bit match hi word?
	jne	clp32_50	;no mot 16 bit for sure
	jmp	SClip16		;try 16 bit clip
clp32_50:
	mov	ax,[si].CX2  	;lo x2
	mov	dx,[si].CX2+2	;hi x2
	mov	bx,[si].CY2	;lo y2
	mov	cx,[si].CY2+2	;hi y2
	call	flag32
	mov	di,bp	;save flag of pt 2
	mov	ax,[si].CX1  	;lo x1
	mov	dx,[si].CX1+2	;hi x1
	mov	bx,[si].CY1	;lo y1
	mov	cx,[si].CY1+2	;hi y1
	call	flag32
	mov	ax,bp		;save flag1
	mov	bx,di		;get flag 2
	or	ax,bx		;all good?
	je	clp32_ok	;yes
	mov	ax,bp
	test	ax,bx		;any shared?
	je	clp32noshare	;nope-needs clipping
	and	ax,bx		;get common outs
	test	ax,(UPMSK OR DWNMSK)	;off in y only?
	jne	clp32noparts	;yup dont draw line
;off to one x or other
	mov	di,bx		;bp=flag1 di=flag2
	call	pin32
	or	al,BADLINE	;no main line + xtra parts
;	mov	al,BADLINE
	mov	ah,al
	mov	[si].CFLAG,ax	;ax = flag of xtras
	ret
clp32noparts:
	mov	al,BADLINE	;no line -no parts
	mov	ah,al
	mov	[si].CFLAG,ax
	ret
;
clp32noshare:
;di=pt 2 outs bp= pt1 outs
;copy pts to scratch area
	push	bp
	push	di
	push	si
	mov	di,OFFSET clipscratch
	push	es
	mov	ax,ds
	mov	es,ax
	mov	cx,8
	rep	movsw		;copy to clipscratch
	sub	si,16		;point back at originals
	mov	cx,8
	rep	movsw		;copy to clip scratch +16
	pop	es
	pop	si
	pop	di
	pop	bp
	call	pin32		;stuff pin vals and get xtra parts flag
	mov	ah,al
	mov	[si].CFLAG,ax
	or	di,di			;pt 2 out?
	je	clp32_10		;no clip one
	or	bp,bp
	jne	clp32both
;clip pt 2 only
	push	si
	mov	si,OFFSET clipscratch
	mov	di,si
	add	di,8
	call	clip32onegood		;clip pt 2
	pop	si
	mov	[si].CX2,ax
	mov	[si].CY2,bx
	ret
clp32_10:
;clip pt 1 -pt 2 already good
	push	si
	mov	si,OFFSET clipscratch
	mov	di,si
	add	si,8
	call	clip32onegood		;clip pt 1
	pop	si
	mov	[si].CX1,ax
	mov	[si].CY1,bx
	ret
clp32both:
;find a common midpoint and then clip pt 1 and pt 2 to edges
;di=flag2 bp=flag 1
;clipscratch has pt1 and pt 2
	mov	wflag1,bp
	mov	wflag2,di	;save outs
;clip scratch has copy of pts and clipscratch +16 has copy
	push	si		;save si
	mov	si,OFFSET clipscratch
	mov	di,si
	add	di,8
	call	getonein		;try to find a point on screen
	pop	si
	jc	clp32_25		;cant find one use bp for side flags
	sub	di,8			;point back at clipscratch
	mov	[di],ax
	mov	[di+2],dx
	mov	[di+4],bx
	mov	[di+6],cx		;result at clipscratch
	push	si
	mov	si,di
	add	di,16			;point 1 bad
	call	clip32onegood
	mov	di,si			;point at clipscratch
	pop	si
	mov	[si].CX1,ax		;clipped x
	mov	[di],ax			;new good x at clipscratch
	mov	[di+2],dx
	mov	[si].CY1,bx
	mov	[di+4],bx
	mov	[di+6],cx
	push	si
	mov	si,di
	add	di,16+8			;point at bad 2
	call	clip32onegood
	pop	si
	mov	[si].CX2,ax
	mov	[si].CY2,bx
	ret
clp32_25:					;line is bad use bp for side
	mov	al,(BADLINE OR XTRA1LFT OR XTRA2LFT)
	test	bp,LFTMSK
	jne	clp32_26	;good guess
	xor	al,(XTRA1LFT OR XTRA1RT OR XTRA2LFT OR XTRA2RT)
clp32_26:
	mov	ah,al
	mov	[si].CFLAG,ax
	ret


lclipallok:
	sub	ax,ax
	mov	[si].CFLAG,ax	;line is good -no xtras
	ret
lclipnoparts:
	mov	al,BADLINE	;no line -no parts
	mov	ah,al
	mov	[si].CFLAG,ax
	ret
sclip_16::
SClip16:
	mov	overflow,0
	mov	bx,[si].CX2
	mov	cx,[si].CY2
	call	flagpt
	mov	ah,al
	mov	dx,bx
	mov	bp,cx
	mov	bx,[si].CX1
	mov	cx,[si].CY1
	call	flagpt
	or	ax,ax
	je	lclipallok		;there great
	test	ah,al			;same outs?
	je	lclipnoshared		;no off to one quad or other
	push	ax
	and	ah,al			;only common bits
	test	ah,(UPMSK OR DWNMSK)	;was it top or bottom?
	pop	ax
	jne	lclipnoparts		;yes no parts of line at all
;off to one x or other
	call	pinpts			;set up the extra parts
	mov	bl,(BADLINE OR XTRA1RT OR XTRA2RT)
	test	al,RTMSK		;off to right?
	jne	lclip10	  		;yes
	mov	bl,(BADLINE OR XTRA1LFT OR XTRA2LFT)
lclip10:
	mov	bh,bl
	mov	[si].CFLAG,bx
	ret
lclipnoshared:
;bx=x1,cx=y1  dx=x2,bp=y2
;al=pt1 flag  ah=pt2 flag
	call	pinpts

	sub	dx,bx		;delx
	jo	lclip_30	;too big
	mov	tdelx,dx
	sub	bp,cx		;dely
	jo	lclip_30	;too big
	mov	tdely,bp
	mov	flag2,ah	;save info about pt 2
	test	byte ptr overflow,1	;did an overflow occur
	je	lclip_31
lclip_30:
	jmp	handleoverflow
lclip_31:
	call	lclipclipit		;ax=new y bp=new x
	jc	lclipwierdline	;line doesnt intersect -just add side parts
;dx=new y bp=new x
	mov	[si].CX1,bp
	mov	[si].CY1,dx
	mov	bx,[si].CX2
	mov	cx,[si].CY2
	mov	al,flag2
	call	lclipclipit
	jc	lclipwierdline	;line doesnt intersect -just add side parts
;cant be wierd -theoretically
	mov	[si].CX2,bp
	mov	[si].CY2,dx
	ret
lclipwierdline:
	or	al,BADLINE	;no main line use corect side from clip result
	mov	ah,al
	mov	[si].CFLAG,ax
	ret
;
handleoverflow:
	jmp	reclip32	;re do using 32 bit
;

Clip32	ENDP	
	
	
;
getonein:
;si,di=x1,y1,x2,y2 find a point in 
;
	mov	cnt,32
	mov	ax,[di]
	mov	dx,[di+2]
	mov	bx,[di+4]
	mov	cx,[di+6]
gt1in_1:
	dec	cnt
	js	gt1in_4		;cant converge -bad
	add	ax,[si]
	adc	dx,[si+2]
	add	bx,[si+4]
	adc	cx,[si+6]
	sar	dx,1
	rcr	ax,1
	sar	cx,1
	rcr	bx,1
	call	flag32		;get outs
	je	gt1in_3		;its in -leave a winner
	test	bp,wflag2	;out on pt 2 side?
	jne	gt1in_2		;yes	
	mov	[si],ax
	mov	[si+2],dx
	mov	[si+4],bx
	mov	[si+6],cx
	mov	ax,[di]
	mov	dx,[di+2]
	mov	bx,[di+4]
	mov	cx,[di+6]
	jmp	gt1in_1
gt1in_2:
	test	bp,wflag1	;also out on pt 1 side?
	jne	gt1in_4
	mov	[di],ax
	mov	[di+2],dx
	mov	[di+4],bx
	mov	[di+6],cx
	jmp	gt1in_1
gt1in_3:
	clc
	ret
gt1in_4:
	stc
	ret
;

;
onein_2:
	cmp	ax,-1			;= -1?
	jne	onein_3			;no dont fix
	sub	ax,ax
	sub	dx,dx
	jmp	onein_3
onein_4:
	cmp	bx,-1
	jne	onein_10
	sub	bx,bx
	sub	cx,cx
	jmp	onein_10
clip32onegood:
;di= bad x,y,flag si=good x,y,flag
;
	mov	cnt,32			;max tries
	mov	ax,[di]
	mov	dx,[di+2]
	mov	bx,[di+4]
	mov	cx,[di+6]
onein_1:
	dec	cnt
	js	onein_7			;fall out
	add	ax,[si]
	adc	dx,[si+2]
	add	bx,[si+4]
	adc	cx,[si+6]
;check for -1 fiasco	then average
	cmp	ax,dx			;both = -1?
	je	onein_2			;yes fix
	sar	dx,1
	rcr	ax,1
onein_3:
	cmp	bx,cx			;both = -1
	je	onein_4
	sar	cx,1
	rcr	bx,1
onein_10:
;ax:dx=avg x bx:cx= avg y
	call	flag32			;get flag
	jne	onein_5			;still out
	call	edge32			;on edge = done?
	jne	onein_6			;yes leave and stuff
	mov	[si],ax
	mov	[si+2],dx
	mov	[si+4],bx
	mov	[si+6],cx
	mov	ax,[di]
	mov	dx,[di+2]
	mov	bx,[di+4]
	mov	cx,[di+6]
	jmp	onein_1
onein_5:
	mov	[di],ax
	mov	[di+2],dx
	mov	[di+4],bx
	mov	[di+6],cx
	jmp	onein_1
onein_6:
	ret
onein_7:
	mov	ax,[si]
	mov	dx,[si+2]
	mov	bx,[si+4]
	mov	cx,[si+6]
	ret
;

;



;
pin32:
;stuff pins of pt1 and 2 bp= pt1 flag, di=pt2 flag
;ax returns with xtra parts flag
;
	test	bp,UPMSK
	jne	pin32_1
	test	bp,DWNMSK
	jne	pin32_2
	mov	bx,[si].CY1
	jmp	pin32_4
pin32_1:
	mov	bx,GYLO
	jmp	pin32_4
pin32_2:
	mov	bx,GYHI
pin32_4:
	mov	[si].C1XTRA,bx

	test	di,UPMSK
	jne	pin32_5
	test	di,DWNMSK
	jne	pin32_7
	mov	bx,[si].CY2
	jmp	pin32_6
pin32_5:
	mov	bx,GYLO
	jmp	pin32_6
pin32_7:
	mov	bx,GYHI
pin32_6:
	mov	[si].C2XTRA,bx
;now set xtra clip flag
	sub	al,al
	test	bp,RTMSK
	je	pin32_10
	or	al,XTRA1RT
pin32_10:
	test	bp,LFTMSK
	je	pin32_11
	or	al,XTRA1LFT
pin32_11:
	test	di,RTMSK
	je	pin32_12
	or	al,XTRA2RT
pin32_12:
	test	di,LFTMSK
	je	pin32_13
	or	al,XTRA2LFT
pin32_13:
	mov	ah,al
	ret
;
;
edge32:
;ax:dx=xval  bx:cx=yval
;bp returned 0 for non edge
	sub	bp,bp
	or	dx,dx		;hi word = 0
	jne	edg_1		;no cant be edge
	or	cx,cx
	jne	edg_1		;same for y
	cmp	ax,GXLO		;is x on low edge?
	je	edg_2		;yes -leave good
	cmp	ax,GXHI
	je	edg_2
	cmp	bx,GYLO
	je	edg_2
	cmp	bx,GYHI
	je	edg_2
edg_1:
	or	bp,bp
	ret
edg_2:
	inc	bp
	ret
;
flag32:
;ax:dx, bx:cx =x,y
;return bp with flag bits of outs-same as flags 16
;
	sub	bp,bp
	or	dx,dx
	js	fl32_1
	jne	fl32_2
	cmp	ax,GXLO
	jb	fl32_1			;note the unsigned jmp
	cmp	ax,GXHI
	ja	fl32_2
fl32_3:
	or	cx,cx
	js	fl32_4
	jne	fl32_5
	cmp	bx,GYLO
	jb	fl32_4
	cmp	bx,GYHI
	ja	fl32_5
	or	bp,bp
	ret
fl32_1:
	or	bp,LFTMSK
	jmp	fl32_3
fl32_2:
	or	bp,RTMSK
	jmp	fl32_3
fl32_4:
	or	bp,UPMSK
	ret
fl32_5:
	or	bp,DWNMSK
	ret
;
;************************************************
.DATA

clipscratch	dw	16 dup(0)
zlinebuf	dw	16 dup(0)

wflag1	dw	0
wflag2	dw	0
tdelx	dw	0
tdely	dw	0
cnt	dw	0
pflag	dw	0
leftlo	dw	7fffh
lefthi	dw	8000h
rightlo	dw	7fffh
righthi	dw	8000h
goodlftrt	dw	0
zhit		dw	0
overflow	db	0
flag2		db	0


	END
