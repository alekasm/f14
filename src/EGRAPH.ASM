	DOSSEG
	.MODEL MEDIUM,C
        page  60, 132


;/***************************************/
;
;	Graph.asm
;	Scott Spanburg
;	12/18/89
;
;
;/***************************************/

FALSE     equ   0
TRUE      equ   not FALSE

include	clip.equ

SCREEN_WIDTH	equ	320/8

.CODE

;***********************************************************
;	c type calls
;
;	InitGraph(mode); SetWorkPage(page); SetDisplayPage(page)
;	GetMem(amount); GDrawDot(X,Y,COLOR);
;	VertFLine(yl,yh,xval);
;
;
;***********************************************************
;	assembly type calls
;
;	SetColor:ax=color Line:si= clip record FLine:si= clip record
;	PFill: fills from min/max table
;
;************************************************************


	PUBLIC	InitGraph ,SetWorkPage, GDrawDot, modeblock

	PUBLIC	SetColor, Line, FLine, PFill

	PUBLIC	GMODE, GWORKPAGE, GDISPLAYPAGE, GXLO
	PUBLIC	GYLO, GXHI, GYHI, GXMID, GYMID, GCOLOR
	PUBLIC	GROW, GCOL, GFONT, GFCOLOR, GBCOLOR
	PUBLIC	GRPAGE0, GRPAGE1, GRPAGE2, GRPAGE3, GRPAGE

	PUBLIC	LINEST,FillTransTbl



	PUBLIC	FLine


;	PUBLIC	TRANSGREYTBL,TRANSGREYPTRS


InitGraph	PROC	MODE:WORD


	call	VGAInitGraph
	ret

InitGraph	ENDP


GDrawDot:

	jmp	FAR PTR VGADrawDot

SetColor	PROC

	call	VGASetColor
	ret
SetColor	ENDP
Line	PROC

	call	VGALine
	ret
Line	ENDP
PFill	PROC

	call	VGAPFill
	ret
PFill	ENDP
;
;
VGAInitGraph:
;sets linest tables, get memory buffers

	mov	cx,200
	mov	ax,ds
	mov	es,ax
	lea	di,LINEST
	sub	ax,ax
@@:
	stosw
	add	ax,320
	loop	@B
;	lea	bx,cs:VGAjmptbl
;	mov	GraphBase,bx
	ret
;
;
SetWorkPage	PROC	GPAGE:WORD
	mov	ax,GPAGE
	mov	GWORKPAGE,ax
@@:
	mov	bx,ax
	shl	bx,1
	mov	ax,GRPAGE0[bx]
	mov	GRPAGE,ax	;easy seg loading
	ret
SetWorkPage	ENDP
;
;
;
VGASetColor:
 	mov	GCOLOR,ax
	mov	ax,GRPAGE
	mov	es,ax	 		;for ease of setup
	ret
;

VGADrawDot	PROC	X:WORD,Y:WORD,COLOR:WORD
	mov	ax,X
	cmp	ax,GXLO
	jl	sd_out
	cmp	ax,GXHI
	jg	sd_out
	mov	ax,Y
	cmp	ax,GYLO
	jl	sd_out
	cmp	ax,GYHI
	jg	sd_out
	shl	ax,1
	mov	bx,ax
	mov	ax,GRPAGE
	mov	es,ax
	mov	di,LINEST[bx]
	add	di,X
	mov	ax,COLOR
;	mov	GCOLOR,ax

	cmp	ah,251
	je	sd_out

	STOSB
sd_out:
	ret
VGADrawDot	ENDP
;


VGALine:
;si points at clip record !!!!!



        push    si
        push    di

        mov     ax,GRPAGE
        mov     es,ax


	mov	cx,[si].CY2
	mov	di,[si].CY1
	mov	bp,[si].CX2
	mov	bx,[si].CX1

	mov	ax,cx
	sub	ax,di

	cwd
	xor	ax,dx
	sub	ax,dx		;ABSAX

	mov	si,ax		;si= absdy
	mov	ax,bp
	sub	ax,bx

	cwd
	xor	ax,dx
	sub	ax,dx		;ABSAX

	mov	dx,ax		;dx= absdx
	cmp	si,dx
	ja	vyline		;dy>dx
;dx>dy
	cmp	bp,bx		;x2>x1 ?
	jge	@F		;yes
;swap pts
	xchg	bp,bx
	xchg	cx,di
@@:
;x2>x1 xinc=+1
;di= ptaddress, ax= color,cx= cnt, dx=absdx, si=absdy, bx=error, bp=xory add
	mov	ax,320
	cmp	cx,di			;y2>y1 ?
	jge	@F
	neg	ax
@@:
	shl	di,1
	mov	di,linest[di]
	add	di,bx			;di = screen add of pt1
	mov	cx,bp
	sub	cx,bx		;cx= delx= pix cnt
	inc	cx		;for loop
   	mov	bp,ax		;bp= yinc
;fix error,color
        mov     bx,cx           ; init error delta
;        inc     bx		;already inc'd cx
        shr     bx,1
        neg     bx
	mov	ax,GCOLOR
;
vplt_1:
	STOSB
        add     bx,si           ; add dy to error delta
        jns     @F		;if pos-update ypos
	loop	vplt_1		;keep going
	jmp	vpl_out
@@:
;redo y
	add	di,bp		;next line
	sub	bx,dx	        ; if went positive, restart delta
	loop	vplt_1
vpl_out:
        pop     di
        pop     si
        ret
vyline:
;dy>dx

	xchg	dx,si		;swap the deltas

	cmp	cx,di		;y2>y1 ?
	jge	@F		;yes
;swap pts
	xchg	bp,bx
	xchg	cx,di
@@:
;y2>y1 yinc=320
;di= ptaddress, ax= color,cx= cnt, dx=absdx, si=absdy, bx=error, bp=xory add
	mov	ax,1
	cmp	bp,bx			;x2>x1 ?
	jge	@F
	neg	ax
@@:
   	mov	bp,ax		;bp= xinc
	sub	cx,di		;cx= dely= pix cnt
	inc	cx		;for loop
	shl	di,1
	mov	di,linest[di]
	add	di,bx			;di = screen add of pt1
;fix error,color
        mov     bx,cx           ; init error delta
;        inc     bx		;already inc'd cx
        shr     bx,1
        neg     bx
	mov	ax,GCOLOR
;
vplt_3:
	mov	ES:[di],al	;plot pixel
	add	di,320		;next y line
        add     bx,si           ; add dx to error delta
        jns     @F		;if pos-update xpos
	loop	vplt_3		;keep going
	jmp	vpl_out
@@:
;redo x
	add	di,bp		;next x pos
	sub	bx,dx	        ; if went positive, restart delta
	loop	vplt_3
	jmp	vpl_out
;
;
;
FLine	PROC
;si points at clip record !!!!!
	push	si
	push	di

	mov	ax,[si].CX1
	mov	dx,[si].CX2
	mov	bx,[si].CY1
	mov	si,[si].CY2
;make sure x1<=x2
	cmp	ax,dx
	jle	@F
	xchg	ax,dx		;swap pt1 and pt2
	xchg	bx,si
@@:
	mov	di,bx		;y1
	shl	di,1		;*2 for word table
	sub	dx,ax		;get delx
	mov	bp,2		;assume pos dely, 2 for word tables
	cmp	si,bx		;y2 >y1?
	jge	@F
	neg	bp		;no neg dely dir
	xchg	si,bx		;si= maxy bx=miny
@@:
;error trap the line
;bx= lo y si= hi y
	cmp	bx,GYLO
	jl	flin_2
	cmp	bx,GYHI
	jg	flin_2
	cmp	si,GYLO
	jl	flin_2
	cmp	si,GYHI
	jg	flin_2
	cmp	bx,MINFILLY	;lowest minmax table entry ?
	jge	@F		;no
	mov	MINFILLY,bx
@@:
	sub	si,bx		;get abs dely
	cmp	si,dx		;dely >delx ?
	ja	flinedy		;yes-go do it
	mov	cx,dx		;pix= delx+1, for loop
	inc	cx		;# pix +1

	mov	bx,cx		;get abs delx
	shr	bx,1		;rmdr=-delx/2
	neg	bx		;all set for draw
	jmp	flin_1
flinfixy:
	cmp	ax,MAXTBL[di]	;greater than current max ?
	jle	@F
	mov	MAXTBL[di],ax
@@:
	sub	bx,dx		;reset remainder, rmdr=rmdr-delx
	dec	cx		;that it ?
	je	flin_2		;yes
	inc	ax		;next x
	add	di,bp		;next table entry
flin_1:
	cmp	ax,MINTBL[di]	;less than current min ?
	jge	@F
	mov	MINTBL[di],ax
@@:
	add	bx,si		;rmdr=rmdr+dely
	jns	flinfixy	;next y line
	inc	ax		;next x pixel
	loop	@B		;keep going
;fell out from done in x-check for last max
	dec	ax		;went one too far
	cmp	ax,MAXTBL[di]	;greater than current max ?
	jle	@F
	mov	MAXTBL[di],ax
@@:
flin_2:
	pop	di
	pop	si
	ret
;
flinedy:
;ax= xlo
;dx= abs delx
;si= abs dely
;bx= ylo
;di = y1 min/max
	mov	cx,si
	inc	cx		;# lines+1
	mov	bx,cx		;rmdr= -dely/2
	shr	bx,1
	neg	bx
	jmp	flin_3
flin_4:
	dec	cx		;that it ?
	je	flin_5		;yes
	sub	bx,si		;rmdr=rmdr-dely
	inc	ax		;next x
	add	di,bp		;next table entry
flin_3:
	cmp	ax,MINTBL[di]	;less than min ?
	jge	@F
	mov	MINTBL[di],ax
@@:
	cmp	ax,MAXTBL[di]	;greater than cur max ?
	jle	@F
	mov	MAXTBL[di],ax
@@:
	add	bx,dx		;rmdr=rmdr+delx
	jns	flin_4
	add	di,bp
	loop	flin_3
flin_5:
	pop	di
	pop	si
	ret
FLine	ENDP
;
VertFline	PROC	YL:word,YH:WORD,XV:WORD
	mov	ax,XV
	mov	bx,YL
	mov	cx,YH
	cmp	cx,bx
	jge	@F
	xchg	bx,cx
@@:
;clip to y bounds
;bx= lo,cx=hi
	cmp	cx,GYLO		;hi too low ?
	jle	vl_out		;yes(= because of single line)
	cmp	bx,GYHI		;lo too hi ?
	jge	vl_out
	cmp	bx,GYLO
	jge	@F
	mov	bx,GYLO
@@:
	cmp	cx,GYHI
	jle	@F
	mov	cx,GYHI
@@:
 	sub	cx,bx	;# lines
	jcxz	vl_out	;dont put a single side-messes up top or bottom

	cmp	bx,MINFILLY	;lowest minmax table entry ?
	jge	@F		;no
	mov	MINFILLY,bx
@@:
	inc	cx	;+ 1 for loop
	shl	bx,1	;word tables
vfl_1:
	cmp	ax,MINTBL[bx]	;less than min ?
	jge	@F
	mov	MINTBL[bx],ax
@@:
	cmp	ax,MAXTBL[bx]	;greater than cur max ?
	jle	@F
	mov	MAXTBL[bx],ax
@@:
	add	bx,2		;nect entry
	loop	vfl_1
vl_out:
	ret
VertFline	ENDP
;



VGAPFL_INVISO:
	mov	bx,MINFILLY
	cmp	bx,7fffh
	jne	@F
	ret
@@:
	mov	MINFILLY,7fffh
	shl	bx,1
@@:
	cmp	MINTBL[bx],7fffh
	je	@F
	mov	word ptr MINTBL[bx],07fffh
	mov	word ptr MAXTBL[bx],08001h
	add	bx,2
	jmp	@B
@@:
	ret
;


vgapfl_t1:
	mov	word ptr MINFILLY,07fffh
	ret
VGAPFL_TRANSFILL:
	mov	bx,MINFILLY
	cmp	bx,7fffh
	jae	vgapfl_t1
	shl	bx,1

vgapfl_t2:
	mov	di,LINEST[bx]
	mov	si,MINTBL[bx]
	mov	bp,MAXTBL[bx]
	cmp	si,7fffh
	je	vgapfl_t1		;thats it
;now do hline
	cmp	si,GXHI
	jg	vgapfl_t3
	cmp	bp,GXLO
	jl	vgapfl_t3
	cmp	si,GXLO
	jge	@F
	mov	si,GXLO
@@:
	cmp	bp,GXHI
	jle	@F
	mov	bp,GXHI
@@:					;bx= xlo,bp=xhi, di= lineaddress
	add	di,si

	mov	cx,bp
	sub	cx,si			;cx= len in bytes

	shl	si,1
	xor	si,bx
	test	si,2
	jne	@F
	inc	di
	dec	cx
	js	vgapfl_t3
@@:
	shr	cx,1
	inc	cx
@@:
	stosb
	inc	di
	loop	@B
vgapfl_t3:
	mov	word ptr MINTBL[bx],07fffh
	mov	word ptr MAXTBL[bx],08001h
	add	bx,2		;next line
	jmp	vgapfl_t2
;

VGAPFL_ANDFILL:
	mov	ah,al
	mov	si,MINFILLY
	cmp	si,7fffh
	jne	@F
vgapfl_a1:
	mov	word ptr MINFILLY,07fffh
	ret
@@:
	shl	si,1
vgapfl_a2:
	mov	bx,MINTBL[si]
	mov	cx,MAXTBL[si]
	cmp	bx,7fffh
	je	vgapfl_a1
;now do hline
	cmp	bx,GXHI
	jg	vgapfl_a3
	cmp	cx,GXLO
	jl	vgapfl_a3
	cmp	bx,GXLO
	jge	@F
	mov	bx,GXLO
@@:
	cmp	cx,GXHI
	jle	@F
	mov	cx,GXHI
@@:					;bx= xlo,bp=xhi, di= lineaddress
;
; ax = color
; bx = X1
; cx = X2
; si = Y1 * 2
;
	sub	cx,bx
	inc	cx			; make pixel count
	mov	di,LINEST[si]
	add	di,bx

	test	di,01
	jz	@F

	and	es:[di],al
	inc	di
	dec	cx
	jz	vgapfl_a3
@@:	mov	bx,cx
	shr	cx,1			; make CX = word count & save remainder in CARRY
	jz	last_a
@@:	and	es:[di],ax
	add	di,2
	loop	@B
last_a:	and	bx,1
	jz	vgapfl_a3
	and	es:[di],al
vgapfl_a3:
	mov	word ptr MINTBL[si],07fffh
	mov	word ptr MAXTBL[si],08001h
	add	si,2		;next line
	jmp	vgapfl_a2
;
VGAPFL_MAX:
	mov	ah,al
	mov	si,MINFILLY
	cmp	si,7fffh
	jne	@F
vgapfl_m1:
	mov	word ptr MINFILLY,07fffh
	ret
@@:
	shl	si,1
vgapfl_m2:
	mov	bx,MINTBL[si]
	mov	cx,MAXTBL[si]
	cmp	bx,7fffh
	je	vgapfl_m1
;now do hline
	cmp	bx,GXHI
	jg	vgapfl_m3
	cmp	cx,GXLO
	jl	vgapfl_m3
	cmp	bx,GXLO
	jge	@F
	mov	bx,GXLO
@@:
	cmp	cx,GXHI
	jle	@F
	mov	cx,GXHI
;
; ax = color
; bx = X1
; cx = X2
; si = Y1 * 2
;
@@:	sub	cx,bx
;	inc	cx			; make pixel count

	jcxz	vgapfl_m3

	mov	di,LINEST[si]
	add	di,bx

	mov	bl,ah
	and	bx,7
	shl	bx,1
; TRANSGREYPTRS was being filled with b/s anyways - so its gone!
;	mov	bx,TRANSGREYPTRS[bx]
	mov	bx,0

@@:
	mov	al,es:[di]
	xlat
	stosb
	loop	@B
vgapfl_m3:
	mov	word ptr MINTBL[si],07fffh
	mov	word ptr MAXTBL[si],08001h
	add	si,2		;next line
	jmp	vgapfl_m2





VGAPFL_ORFILL:
	mov	ah,al
	mov	si,MINFILLY
	cmp	si,7fffh
	jne	@F
vgapfl_o1:
	mov	word ptr MINFILLY,07fffh
	ret
@@:
	shl	si,1
vgapfl_o2:
	mov	bx,MINTBL[si]
	mov	cx,MAXTBL[si]
	cmp	bx,7fffh
	je	vgapfl_o1
;now do hline
	cmp	bx,GXHI
	jg	vgapfl_o3
	cmp	cx,GXLO
	jl	vgapfl_o3
	cmp	bx,GXLO
	jge	@F
	mov	bx,GXLO
@@:
	cmp	cx,GXHI
	jle	@F
	mov	cx,GXHI
;
; ax = color
; bx = X1
; cx = X2
; si = Y1 * 2
;
@@:	sub	cx,bx
	inc	cx			; make pixel count
	mov	di,LINEST[si]
	add	di,bx

	test	di,01
	jz	@F

	or	es:[di],al
	inc	di
	dec	cx
	jz	vgapfl_o3
@@:	mov	bx,cx
	shr	cx,1			; make CX = word count & save remainder in CARRY
	jz	last_o
@@:	or	es:[di],ax
	add	di,2
	loop	@B
last_o:	and	bx,1
	jz	vgapfl_o3
	or	es:[di],al
vgapfl_o3:
	mov	word ptr MINTBL[si],07fffh
	mov	word ptr MAXTBL[si],08001h
	add	si,2		;next line
	jmp	vgapfl_o2

VGAPFill:
;fill horz lines from linest tbl
;MINFILLY= y start
	mov	ax,GCOLOR


	cmp	ah,250
	jb	VGAPFL_0		;dither fill

	cmp	ah,255
	jne	@F
	mov	ah,al			; solid fill-special for hi colors
	jmp	VGAPFL_0
@@:
	cmp	ah,254			;and fill
	jne	@F
	jmp	VGAPFL_ANDFILL
@@:
	cmp	ah,253			; or fill
	jne	@F
	jmp	VGAPFL_ORFILL
@@:
	cmp	ah,252
	jne	@F
	jmp	VGAPFL_TRANSFILL
@@:
	cmp	ah,251
	jne	@F
	jmp	VGAPFL_INVISO
@@:
	cmp	ah,250			;max trans fill
	jne	@F
	jmp	VGAPFL_MAX
@@:


VGAPFL_0:
	mov	si,ds
	lds	bx,DWORD PTR FillTransTbl
	or	bx,bx
	je	@F
	xlat
	xchg	ah,al
	xlat
	xchg	ah,al

@@:
	mov	ds,si
	mov	tempax,ax
	mov	si,MINFILLY
	cmp	si,7fffh
	jne	@F
VGAPFL_2:
	mov	word ptr MINFILLY,07fffh
	ret
@@:
	shl	si,1
VGAPFL_1:
	mov	di,LINEST[si]
	mov	bx,MINTBL[si]
	mov	bp,MAXTBL[si]
	cmp	bx,7fffh
	je	VGAPFL_2		;thats it
;now do hline
	cmp	bx,GXHI
	jg	VGAPFL_3
	cmp	bp,GXLO
	jl	VGAPFL_3
	cmp	bx,GXLO
	jge	@F
	mov	bx,GXLO
@@:
	cmp	bp,GXHI
	jle	@F
	mov	bp,GXHI
@@:					;bx= xlo,bp=xhi, di= lineaddress
	add	di,bx

	mov	ax,tempax
	test	si,2
	jne	@F
	xchg	al,ah
@@:
	test	bx,1
	jne	@F
	xchg	al,ah
@@:
	stosb				;write out at least one byte
	mov	cx,bp
	sub	cx,bx			;cx= len in bytes
	je	VGAPFL_3		;thats it
	test	di,1			;odd byte ?
	je	@F			;no-even -keep going
	xchg	al,ah
	stosb
	dec	cx
	je	VGAPFL_3
@@:
	xchg	al,ah
	shr	cx,1
	REP	STOSW
	jnc	VGAPFL_3
	STOSB
VGAPFL_3:
	mov	word ptr MINTBL[si],07fffh
	mov	word ptr MAXTBL[si],08001h
	add	si,2		;next line
	jmp	VGAPFL_1
;
;

.DATA
;GRAPH VARS

GraphBase	dw	0		;EGAjmpTbl

;
GMODE		DW	-1
GWORKPAGE	DW	0
GDISPLAYPAGE	DW	0
GXLO		DW	0
GYLO		DW	0
GXHI		DW	319
GYHI		DW	199
GXMID		DW	320/2
GYMID		DW	200/2
GCOLOR		DW	0
GROW		DW	0
GCOL		DW	0
GFONT		DW	0
GFCOLOR		DW	15
GBCOLOR		DW	0
GRPAGE0		DW	0
GRPAGE1		DW	0
GRPAGE2		DW	0
GRPAGE3		DW	0
GRPAGE		dw	0
;
modeblock	label	byte
LINEST		DW	200 dup(0a000h)
MINTBL		DW	201 dup(07fffh)		;201 is important !!!
MAXTBL		DW	201 dup(08001h)
MINFILLY	DW	7fffh
;
X1	DW	0
Y1	DW	0
X2	DW	0
Y2	DW	0
DDX	DW	0
ABSDY	DW	0
;



;TRANSGREYTBL	label	BYTE
;.XLIST
;i=0
;        REPT 256
;        db	i
;i=i+1
;        ENDM
;.LIST

FillTransTbl	dd	0

;TRANSGREYPTRS	dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL
;		dw	TRANSGREYTBL


tempax	dw	0


sdwnadd	dw	SCREEN_WIDTH-1
supadd	dw	-(SCREEN_WIDTH+1)

light	db	0,0
;
;
	END




