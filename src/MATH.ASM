	DOSSEG
	.MODEL MEDIUM,C
        page  60, 132


;/***************************************/
;	MATH.ASM
;	Scott Spanburg (Update of J.Reego earlier code-made c compatible)
;	10/24/89
;	PROCS
;		SinB(B)		j.r.	  !!!! funny return val (7fffh = +1)
;		CosB(B)		j.r.	  !!!! funny return val
;		ARCTAN(DELY,DELX)	j.r.
;		ASinB(A,B)	j.r.
;		ACosB(A,B)	j.r.
;		ARCSIN(B)	s.s
;		Rotate(V,M1)	s.s 	;quick point rotate-word truncated
;		Mult3X3(M1,M2)	s.s	;assumes trid matrices(mults by 2)
;		Mult1X3(V,M1,VL)s.s	;precise returns long (not *2  !!!!)
;		Trans3X3(M1,M2)	s.s	;makes M2= M1 transposed
;		Copy3X3(M1,M2)	s.s	;copy a 3x3 from M1 to M2
;		Normalize1X3(V1)s.s	;make 3x3 a unit mat(length=7fff)
;		Normalize3X3(M1)s.s	;make 3x3 a unit mat(length=7fff)
;		MakeMatHPR(H,P,R,M1)	s.s ;makes a trig matrix from H,P,R
;		MakeMatRPH(R,P,H,M1)	s.s ;makes a trig matrix from H,P,R
;		GetHPR(M1,H,P,R)	ss opposite of make hpr
;		Dist2D		s.s
;		Dist3D		s.s
;		GetAndyAngles	s.s	;return f 19 3d angles
;		TMul(val1,val2)	s.s	;mult 2 vals trig style
;		MulDiv(val1*val2/val3)	s.s
;
;		
;
;3d points are row matrices 1X3
;rotations are [x' y' z'] = [x y z] [A B C]
;				    [D E F]	
;				    [G H I]
;where [A B C] is the new i basis vector
;   [D E F] is new j basis
;   [G H I] is new k basis
;/***************************************/


FALSE     equ   0
TRUE      equ   not FALSE

DEG	equ	16384/90


IV	equ	0
C1	equ	IV
JV	equ	2
C2	equ	JV
KV	equ	4
C3	equ	KV

R1	equ	0
R2	equ	6
R3	equ	12

C1LO	equ	0
C1HI	equ	2
C2LO	equ	4
C2HI	equ	6
C3LO	equ	8
C3HI	equ	10

SMUL	MACRO	val
	imul	WORD PTR val
	shl	ax,1
	rcl	dx,1
	ENDM
ABSAX	MACRO
	cwd
	xor	ax,dx
	sub	ax,dx
	ENDM

MATHSEG		MACRO
	mov	bx,SEG _MATHTBLS
	mov	es,bx
	ENDM

_MATHTBLS	SEGMENT	PARA PUBLIC 'DAT'
.XLIST
SIN_TABLE label word
INCLUDE 	sin.tbl
ATAN_TABLE      label word
INCLUDE    	atn.tbl
INCLUDE	   	tantorad.tbl
ARCSINTABLE	label	word
INCLUDE	  	arcsin.tbl
.LIST
_MATHTBLS	ENDS

.CODE










;/**********************************************************************/
;SinB(B)	B=angle 0-ffffh (hi 12 bits precision)
; returns	ax= sin of B (7fffh=1 !!! 8000h=-1 !!!)
;return val is 1/2 as big as should be -no absolute val of +-1
SinB	PROC	USES es,B:WORD
	MATHSEG			;set the math segment
 	mov	ax,B		;get angle
	mov	bx,ax		;use as index
	shr	bx,1			;[2] shift bx down by 4
	shr	bx,1			;[2] then mult by 2 for word lookup
	test	ah,040h			;[4] sin values moving away
	jz	@F			;quadrant 1 or 3
	not	bx
@@:
	and	bx,0ffeh		;[3] use appropriate bits
        mov   bx, es:SIN_TABLE[bx]         ;  [13] read the sin
        or    ah, ah                    ;   [3] negtive sign value to be returned ?
        jns   @F	                 ;   (6) positive quadrant
        neg   bx                        ;   [3]
@@:
	mov	ax,bx			;return in ax
        ret
SinB	ENDP

;/**********************************************************************/
;CosB(B)	B=angle 0-ffffh (12 hi bits)
;return same as sin function above !!!!!!!
CosB	PROC	uses es, B:WORD
	MATHSEG				;address tables
	mov	ax,B			;get angle
 	mov	bx,ax			;[2] use bx as index
	shr	bx,1			;[2] shift bx down by 4
	shr	bx,1			;[2] then mult by 2 for word lookup
	test	ah,040h			;[4] sin values moving away
	jnz	@F
	not	bx
@@:
	and	bx,0ffeh		;[3] use appropriate bits
        mov   bx, es:SIN_TABLE[bx]         ;  [13] read the sin
        add   ah, 01000000b             ;   [4] check for quadrants 2 , 3
        or    ah, ah                    ;   [3] negtive sign value to be returned ?
        jns   @F	                 ;   (6) positive quadrant
        neg   bx                        ;   [3]
@@:
  	mov	ax,bx			;return in ax
	ret
CosB	ENDP

;/**********************************************************************/
;ARCTAN(DELY,DELX)	Dy= delta y(rise) 0-ffffh, Dx= deltax(run) 0-ffffh
ARCTAN	PROC	uses es,DELY:WORD,DELX:WORD
	MATHSEG
        sub   ax, ax            ; [2]
        sub   bx, bx            ; [2]
	mov	cx,DELX		;get delta x
	mov	dx,DELY		;get delta y
        or    cx, cx            ; [3]        is x positive ?
        jns   @F
        add   bl, 8             ; (2)
        neg   cx                ; (1.5)      make x positive
@@:
        or    dx, dx            ; [3]        is y positive ?
        jns   @F
        add   bl, 4             ; (2)
        neg   dx                ; (1.5)      make y positive
@@:
        cmp   cx, dx            ; [3]        is Dx > Dy ?
        jae   @F
        add   bl, 2             ; (2)
        xchg  cx, dx            ; (1.5)      switch Dx and Dy
@@:
	cmp	dx,cx
	jae	zero_out1	;overflow
	or    cx, cx            ; [3]        are we dividing by 0 ?
        jz    zero_out1         ; [4]        don't divide by 0
        DIV   cx                ; [155]
	cwd
	or	ax,ax
        mov   dx, OCTANT_ADJUST[bx]     ; [13]  fetch the base angle value
        mov   bl, ah                    ;  [2]  set up to read ATAN from 256 entry table
	shl	al,1		;move in 3 more bits of precision
	rcl	bx,1
	shl	al,1
	rcl	bx,1
	shl	al,1
	rcl	bx,1
	shl	al,1
	rcl	bx,1
        shl   bx, 1                     ;  [2]-word entries
zero_in1:
        or    dl, dl                    ;  [3]  dl contains even/odd information
        js    subtract                  ;  (6)
        sub   dl, dl                    ;  [2]  zero the add/sub indicator
        add   dx,es:ATAN_TABLE[bx]        ; [13]  add 0-45 deg angle to the OCTANT ADJUST
	mov	ax,dx			;return in ax
        ret                             ;  [8]
zero_out1:
        mov   dx, OCTANT_ADJUST[bx]     ; [13] fetch the base angle value
        mov   bx, 01ffeh                  ;  [8] point to the last entry (max out)
        jmp   zero_in1                  ;  [1] get back in their and fight!
subtract:
        sub   dl, dl                    ;  [2] zero add/sub indicator
        sub   dx, es:ATAN_TABLE[bx]        ; [13] sub 0-45 deg angle from the OCTANT_ADJUST
	mov	ax,dx			;return in ax
        ret                             ;  [8]
ARCTAN	ENDP

;/**********************************************************************/
;ASinB(A,B)	A= 0-ffffh multiplier B=angle (12 hi bits)
ASinB	PROC	USES es,A:WORD,B:WORD

	MATHSEG				;set up math segment
	mov	ax,B			;get angle
	mov	dx,A			;get multiplier
 	mov	bx,ax			;[2] use bx as index
	shr	bx,1			;[2] shift bx down by 4
	shr	bx,1			;[2] then mult by 2 for word lookup
	test	ah,040h			;[4] sin values moving away
	jz	@F
	not	bx
@@:
	and	bx,0ffeh		;[3] use appropriate bits
        mov   bx, es:SIN_TABLE[bx]         ;  [13] read the sin
        or    ah, ah                    ;   [3] negtive sign value to be returned ?
        jns   @F
        neg   bx                        ;   [3]
@@:
        mov   ax, bx                    ;   [2]

	sar	ax,1
	add	ax,1

        IMUL  dx                        ; [144]  A x SIN(B)

	shl	ax,1
	rcl	dx,1
	shl	ax,1
	rcl	dx,1

;        mov   bh, ah                    ;   [2]
;        shl   ax, 1                     ;   [2]  needs to be shifted left once
;	rcl	dx,1
;        or    bh, bh                    ;   [3]
;        jns   @F
;        or    dl, 1                     ; (1.5)
;@@:
	mov	ax,dx			;return val in ax
        ret
ASinB	ENDP

;/**********************************************************************/
;ACosB()	A=signed int multiplier B= angle(12 hi bits)
ACosB	PROC	USES es,A:WORD,B:WORD
 	MATHSEG
	mov	ax,B			;get angle
	mov	dx,A			;get multiplier
	mov	bx,ax			;[2] use bx as index
	shr	bx,1			;[2] shift bx down by 4
	shr	bx,1			;[2] then mult by 2 for word lookup
	test	ah,040h			;[4] sin values moving away
	jnz	@F
	not	bx
@@:
	and	bx,0ffeh		;[3] use appropriate bits
        mov   bx, es:SIN_TABLE[bx]         ;  [13] read the sin
        add   ah, 01000000b             ;   [4] check for quadrants 2 , 3
        or    ah, ah                    ;   [3] negtive sign value to be returned ?
        jns   @F
        neg   bx                        ;   [3]
@@:
        mov   ax, bx                    ;   [2]

	sar	ax,1
	add	ax,1

        IMUL  dx                        ; [144]


	shl	ax,1
	rcl	dx,1
	shl	ax,1
	rcl	dx,1

;
;        mov   bh, ah                    ;   [2]
;        shl   ax, 1                     ;   [2]
;	rcl	dx,1
;        or    bh, bh                    ;   [3]
;        jns   @F
;        or    dl, 1                     ; (1.5)
;@@:
	mov	ax,dx
        ret
ACosB	ENDP

;/**********************************************************************/
;ARCSIN(B)	B= sin of angle as above ! 7fffh = +1
ARCSIN	PROC	USES es,B:WORD
	MATHSEG
	mov	ax,B			;get sin of angle
	mov	bx,ax
	or	ax,ax
	jns	@F			;already pos dont worry
	neg	bx
@@:
	shr	bx,1			;[2] shift bx down by 5
	shr	bx,1			;[2] then mult by 2 for word lookup
	shr	bx,1			;[2]
	shr	bx,1			;[2] = 4 total shift rights
	and	bx,7feh			;1024 entry word table
	mov	bx,es:ARCSINTABLE[bx]	;get value
	or	ax,ax
	jns	@F
	neg	bx			;negate angle
@@:
	mov	ax,bx
	ret
ARCSIN	ENDP

;/**********************************************************************/
;Dist2D(A,B)	A= signed int delta B= signed int delta-returns integer
Dist2D	PROC	USES es,A:WORD,B:WORD
	MATHSEG
	mov	ax,A
	mov	bx,B
	cwd			;getabs of del1
	xor	ax,dx
	sub	ax,dx
	xchg	ax,bx		;save and get abs of del2
	cwd
	xor	ax,dx
	sub	ax,dx
	cmp	ax,bx		;make angle <45 divisop must be greater
	je	d2d_1		;equal use 1.414 times ax
	jb	d2d_2		;ax is smaller use them as is
	xchg	bx,ax		;ax must have smaller val
d2d_2:
	mov	cx,bx		;save largest
	mov	dx,ax		;pre mult by 16 bits
	sub	ax,ax
	div	bx		;get frac valu
	cwd
	or	ax,ax
	mov	dx,bx
	mov	bl,ah		;256 table index
	sub	bh,bh
	shl	bx,1		;word table
	mov	ax,es:tnrdtbl[bx]
	mov	bx,dx
d2d_3:
	mul	bx
	mov	ax,dx
	add	ax,bx
	cmp	ax,cx		;is dist > largest val ?
	jae	d2d_4		;yes
	mov	ax,cx		;fix embarrasing mistake
d2d_4:
	ret
d2d_1:
	mov	cx,bx
	mov	ax,es:tnrdtbl+2*256
	jmp	d2d_3
Dist2D	ENDP

;/**********************************************************************/
;Dist3D(A,B,C)	A=signed int delta, B= signed int delta, C=signed int delta
Dist3D	PROC	USES es,A:WORD,B:WORD,C:WORD
	mov	ax,A
	mov	bx,B
	push	bx
	push	ax
	call	Dist2D
	add	sp,4		;clean up stack
;check for overflow here
	or	ax,ax
	js	d3d_1		;too big fo 2d routine
	mov	bx,C
	push	bx
	push	ax
	call	Dist2D
	add	sp,4		;clean up stack
	ret			;ax=dist unsigned
d3d_1:
	mov	bx,ax 		;val too big use biggest +smaller/2
	mov	ax,C
	ABSAX
	shr	ax,1
	add	ax,bx
	ret
Dist3D	ENDP

;/**********************************************************************/

;Rotate(V,M1)
;rotates a point in 3d -MAY OVERFLOW- truncates to word
;assumes matrix is a trig matrix(7fffh= +1 )
Rotate	PROC	USES si di,V:PTR,M1:PTR

	mov	si,V		;x,y,z pts
	mov	di,M1		;3X3 mat add
	mov	bx,[si].C1	;x
	mov	cx,[si].C2	;y
	mov	bp,[si].C3	;z
;do i contributions first
	mov	ax,bx		;x*r1c1
	imul	WORD PTR [di].R1.C1
	mov	[si].C1,dx
	mov	ax,bx		;x*r1c2
	imul	WORD PTR [di].R1.C2
	mov	[si].C2,dx
	mov	ax,bx		;xr1c3
	imul	WORD PTR [di].R1.C3
	mov	[si].C3,dx
;that frees up bx
;now figure result i comp
	mov	ax,cx	  	;y*r2c1
	imul	WORD PTR [di].R2.C1
	mov	bx,dx
	mov	ax,bp		;+z*r3c1
	imul	WORD PTR [di].R3.C1
	add	bx,dx
	add	bx,[si].C1	;+x*r1c1-computed above
	shl	bx,1		;*2 for trig mult
	mov	[si].C1,bx	;new x
;now figure result j comp
	mov	ax,cx	  	;y*r2c2
	imul	WORD PTR [di].R2.C2
	mov	bx,dx
	mov	ax,bp		;+z*r3c2
	imul	WORD PTR [di].R3.C2
	add	bx,dx
	add	bx,[si].C2	;+x*r1c2-computed above
	shl	bx,1		;*2 for trig mult
	mov	[si].C2,bx	;new y
;now figure result k comp
	mov	ax,cx	  	;y*r2c3
	imul	WORD PTR [di].R2.C3
	mov	bx,dx
	mov	ax,bp		;+z*r3c3
	imul	WORD PTR [di].R3.C3
	add	bx,dx
	add	bx,[si].C3	;+x*r3c1-computed above
	shl	bx,1		;*2 for trig mult
	mov	[si].C3,bx	;new z
	ret
Rotate	ENDP

;/**********************************************************************/
;Mult3X3(M1,M2,M3) M1=near address of 3X3 matrix1 M2= near address of mat 2
; M3= near address of destination matrix
;[M3]=[M1]X[M2] in this order
;assumes both matrices are of 7fff=+1 type (trig type)
Mult3X3	PROC	USES si di ,M1:PTR,M2:PTR,M3:PTR
	mov	si,M1		;get mat 1
	mov	bx,M2		;get mat 2
	mov	di,M3		;get dest mat
	call	rowcol		;first row
	add	si,R2
	call	rowcol		;2nd row
	add	si,R2
	call	rowcol		;3rd row
	add	si,R2
	ret
Mult3X3	ENDP
rowcol:
	mov	cx,3
@@:
	mov	ax,[si].R1.C1
	imul	WORD PTR [bx].R1.C1
	mov	bp,dx
	mov	RMDR,ax				;extra precision
	mov	ax,[si].R1.C2
	imul	WORD PTR [bx].R2.C1
	add	RMDR,ax
	adc	bp,dx
	mov	ax,[si].R1.C3
	imul	WORD PTR [bx].R3.C1
	add	RMDR,ax
	adc	dx,bp
	shl	RMDR,1
	rcl	dx,1			;make up for trig muls
	mov	[di],dx			;result
	add	di,2			;next dest column
	add	bx,C2			;next column of 2nd matrix
	loop	@B			;3 columns
	sub	bx,C3+C2		;point back at first column of mat 2
	ret

;/**********************************************************************/
;Trans3X3(M1,M2)	M1= source M2= dest of transpose of M1
Trans3X3	PROC	USES si di,M1:PTR,M2:PTR
	mov	si,M1
	mov	di,M2
	lodsw			;get R1C1
	mov	[di].R1.C1,ax	;put at R1C1
	lodsw			;get R1C2
	mov	[di].R2.C1,ax	;put at R2C1
	lodsw			;get R1C3
	mov	[di].R3.C1,ax	;put at R3C1
	lodsw
	mov	[di].R1.C2,ax
	lodsw
	mov	[di].R2.C2,ax
	lodsw
	mov	[di].R3.C2,ax
	lodsw
	mov	[di].R1.C3,ax
	lodsw
	mov	[di].R2.C3,ax
	lodsw
	mov	[di].R3.C3,ax
	ret
Trans3X3	ENDP

;/**********************************************************************/
;Copy3X3(M1,M2)	M1= source M2= dest 
Copy3X3	PROC	USES es si di,M1:PTR,M2:PTR
	mov	ax,ds
	mov	es,ax
	mov	si,M1
	mov	di,M2
	mov	cx,9
	rep	movsw
	ret
Copy3X3	ENDP

;/**********************************************************************/
;Mult1X3(V,M1,VL) V= WORD row vector(point), M1= 3X3 mat (not assumed trig!!)
;dest VL = LONG row vector (not multiplied by 2 for trig functs)
.386
Mult1X3	PROC	USES esi edi,V:PTR,M1:PTR,VL:PTR

	mov	si,V		;x,y,z pts
	mov	di,M1		;3X3 mat add
	mov	bx,VL		;get dest

        push    ebp

        movsx   ecx,WORD PTR [si].C1         ;cx= xmul
        movsx   edx,WORD PTR [si].C2       ;dx= ymul
        movsx   esi,WORD PTR [si].C3       ;si= zmul

        movsx   ebp,WORD PTR [di].R1.C1
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C1
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C1
        imul    eax,esi
        add     ebp,eax
        mov     [bx],ebp

        movsx   ebp,WORD PTR [di].R1.C2
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C2
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C2
        imul    eax,esi
        add     ebp,eax
        mov     [bx+4],ebp

        movsx   ebp,WORD PTR [di].R1.C3
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C3
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C3
        imul    eax,esi
        add     ebp,eax
        mov     [bx+8],ebp

        pop     ebp

	ret
Mult1X3	ENDP
.8086
;/**********************************************************************/
;MakeMatHPR(H,P,R,M1) H=head,P=pitch,R=roll,M1= dest matrix (trig style)
;M1=	[CSH*CR-SH*SNP*SR	CSH*SR+SH*SNP*CR	-SH*CP	]
;	[-CP*SR			CP*CR			SNP	]
;	[SH*CR+CSH*SNP*SR	SH*SR-CSH*SNP*CR	CSH*CP	]
;
MakeMatHPR	PROC	USES di,H:WORD,P:WORD,R:WORD,M1:PTR
	mov	di,M1

	push	H
	call	SinB
	mov	SH,ax
	call	CosB		;H is still on stack for this call!!!!!
	mov	CSH,ax
	add	sp,2		;clean up

	push	R
	call	SinB
	mov	SR,ax
	call	CosB		;R is still on stack !!!!!
	mov	CR,ax
	add	sp,2		;clean up

	push	P
	call	SinB
	mov	SNP,ax
	call	CosB		;P still on stack !!!!
	mov	CP,ax
	add	sp,2

;M1=	[CSH*CR-SH*SNP*SR	CSH*SR+SH*SNP*CR	-SH*CP	]
	mov	ax,SR
	SMUL	SNP
	mov	ax,dx
	SMUL	SH
	neg	dx
	mov	cx,dx
	mov	ax,CR
	SMUL	CSH
	add	dx,cx
	mov	[di].R1.C1,dx

	mov	ax,CR
	SMUL	SNP
	mov	ax,dx
	SMUL	SH
	mov	cx,dx
	mov	ax,SR
	SMUL	CSH
	add	dx,cx
	mov	[di].R1.C2,dx

	mov	ax,CP
	SMUL	SH
	neg	dx
	mov	[di].R1.C3,dx

;	[-CP*SR			CP*CR			SNP	]
	mov	ax,SR
	SMUL	CP
	neg	dx
	mov	[di].R2.C1,dx

	mov	ax,CR
	SMUL	CP
	mov	[di].R2.C2,dx

	mov	dx,SNP
	mov	[di].R2.C3,dx

;	[SH*CR+CSH*SNP*SR	SH*SR-CSH*SNP*CR	CSH*CP	]
	mov	ax,SR
	SMUL	SNP
	mov	ax,dx
	SMUL	CSH
	mov	cx,dx
	mov	ax,CR
	SMUL	SH
	add	dx,cx
	mov	[di].R3.C1,dx

	mov	ax,CR
	SMUL	SNP
	mov	ax,dx
	SMUL	CSH
	neg	dx
	mov	cx,dx
	mov	ax,SR
	SMUL	SH
	add	dx,cx
	mov	[di].R3.C2,dx

	mov	ax,CP
	SMUL	CSH
	mov	[di].R3.C3,dx
	ret
MakeMatHPR	ENDP

;/**********************************************************************/
;GetHPR(M1, H,P,R) M1= source matrix (trig style), H=head,P=pitch,R=roll,
;M1=	[CSH*CR-SH*SNP*SR	CSH*SR+SH*SNP*CR	-SH*CP	]
;	[-CP*SR			CP*CR			SNP	]
;	[SH*CR+CSH*SNP*SR	SH*SR-CSH*SNP*CR	CSH*CP	]
;
GetHPR	PROC	USES di,M1:PTR,H:PTR,P:PTR,R:PTR

	mov	di,M1		;get mat address

	mov	ax,[di].R2.C3	;get sin of pitch
	push	ax		;send sin to arcsin routine
	call	ARCSIN
	add	sp,2		;clean up
;ax= pitch -90 - 90 *DEG	;cospitch is always positive !!!
	push	ax
	ABSAX			;too close to 90*DEG ?
	cmp	ax,16300
	ja	fixhpr		;yes-punt

	mov	bx,[di].R3.C3	;get ch*cp
	mov	cx,[di].R1.C3	;get -sh*cp
	neg	cx
	push	bx    		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
;ax= head
	push	ax

	mov	bx,[di].R2.C2	;cr*cp
	mov	cx,[di].R2.C1	;-sr*cp
	neg	cx
	push	bx		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
nhpr:
;ax= roll
	mov	di,R
	mov	[di],ax	
	pop	ax
	mov	di,H
	mov	[di],ax
	pop	ax	
	mov	di,P
	mov	[di],ax	

	ABSAX
	cmp	ax,16000
	jb	@F
	mov	di,M1		;get mat address
	mov	ax,[di].R2.C2	;cos pitch
	push	ax
	mov	ax,[di].R2.C3
	push	ax
	call	ARCTAN
	add	sp,4
	mov	di,P
	mov	[di],ax	
@@:

	ret
fixhpr:
;pitch on stack
;make roll=0 sr=0,cr=1
;then SH*CR+CH*SP*SR=R3.C1 = SH
; CH*CR-SH*SP*SR=R1.C1 = CH

;fix pitch

	mov	bx,[di].R1.C1	;CH
	mov	cx,[di].R3.C1	;SH
	push	bx    		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
;ax= head
	push	ax
	sub	ax,ax
;ax=roll
	jmp	nhpr		;set the vars-leave
GetHPR	ENDP




	if FALSE

;/**********************************************************************/
;GetAndyANgles(M1, H,P,R) M1= source matrix (trig style),A.Hollis style
;M1=	[CSH*CR-SH*SNP*SR	-CP*SR		SH*CR+CSH*SNP*SR]
;	[CSH*SR+SH*SNP*CR	CP*CR		SH*SR-CSH*SNP*CR]
;	[-SH*CP			SNP		CSH*CP		]

;
GetAndyAngles	PROC	USES di,M1:PTR,H:PTR,P:PTR,R:PTR

	mov	di,M1		;get mat address

	mov	ax,[di].R3.C2	;get sin of pitch
	push	ax		;send sin to arcsin routine
	call	ARCSIN
	add	sp,2		;clean up
;ax= pitch -90 - 90 *DEG	;cospitch is always positive !!!
	push	ax
	ABSAX			;too close to 90*DEG ?
	cmp	ax,16000
	ja	afixhpr		;yes-punt

	mov	bx,[di].R3.C3	;get ch*cp
	mov	cx,[di].R3.C1	;get -sh*cp
	neg	cx
	push	bx    		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
;ax= head
	push	ax

	mov	bx,[di].R2.C2	;cr*cp
	mov	cx,[di].R1.C2	;-sr*cp
	neg	cx
	push	bx		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
anhpr:
;ax= roll
	mov	di,R
	mov	[di],ax	
	pop	ax
	mov	di,H
	mov	[di],ax
	pop	ax	
	mov	di,P
	mov	[di],ax	
	ret
afixhpr:
;pitch on stack
;make roll=0 sr=0,cr=1
;then SH*CR+CH*SP*SR=R3.C1 = SH
; CH*CR-SH*SP*SR=R1.C1 = CH

;fix pitch
;	pop	ax		;get pitch
;	cwd			;make +- 90*DEG
;	mov	ax,16384
;	xor	ax,dx
;	sub	ax,dx

	mov	bx,[di].R1.C1	;CH
	mov	cx,[di].R1.C3	;SH
	push	bx    		;delx
	push	cx		;dely
	call	ARCTAN
	add	sp,4		;clean up
;ax= head
	push	ax
	sub	ax,ax		;roll=0
	jmp	anhpr		;set the vars-leave
GetAndyAngles	ENDP


;/**********************************************************************/
;Normalize1X3(V1) -normalizes a vector to length 7fffh
Normalize1X3	PROC	USES si,V1:PTR

	mov	si,V1
	push	[si].C1
	push	[si].C2
	push	[si].C3
	call	Dist3D			;get row vector length
	add	sp,6			;clean up
	mov	cx,ax
	shr	cx,1
	shr	cx,1
	jcxz	nleave			;dont div by zero-leave
	mov	bx,7fffh/8

	mov	ax,[si].C1
	imul	bx
	idiv	cx
	cwd
	or	ax,ax
	shl	ax,1
	mov	[si].C1,ax
	mov	ax,[si].C2
	imul	bx
	idiv	cx
	cwd
	or	ax,ax
	shl	ax,1
	mov	[si].C2,ax
	mov	ax,[si].C3
	imul	bx
	idiv	cx
	cwd
	or	ax,ax
	shl	ax,1
	mov	[si].C3,ax
nleave:
	ret
Normalize1X3	ENDP

;/**********************************************************************/
;Normalize3X3(M1) -normalizes a matrix by row to length 7fffh
Normalize3X3	PROC	USES si,M1:PTR
	mov	ax,M1
	push	ax
	call	Normalize1X3
	add	sp,2
	mov	ax,M1
	add	ax,R2
	push	ax
	call	Normalize1X3
	add	sp,2
	mov	ax,M1
	add	ax,R3
	push	ax
	call	Normalize1X3
	add	sp,2
	ret
Normalize3X3	ENDP



	endif


;/**********************************************************************/
;TMul(val1,val2)
TMul	PROC	val1:WORD,val2:WORD
	mov	ax,val1
	imul	val2
	shl	ax,1
	rcl	dx,1
	mov	ax,dx
	cwd
	ret
TMul	ENDP

;/**********************************************************************/




;MulDiv(val1*val2/val3)
MulDiv	PROC	val1:WORD,val2:WORD,val3:WORD
	mov	ax,val3
	ABSAX
	jne	@F		;dont div by zero
	inc	val3
@@:
	mov	cx,ax
	mov	ax,val2
	ABSAX
	cmp	ax,cx
	jae	@F
	mov	ax,val1
	imul	val2
	idiv	val3
	cwd
	or	ax,ax
	ret
@@:
	mov	ax,val2
	cwd
	idiv	val3
	cwd
	or	ax,ax

	imul	val1
	ret

MulDiv	ENDP


;MulDiv(val1*val2/val3)
MDiv	PROC	val1:WORD,val2:WORD,val3:WORD
	mov	ax,val1
	imul	val2
	idiv	val3
	ret

MDiv	ENDP



;/**********************************************************************/
;LDiv(val1/val2)
LDiv	PROC	val1lo:WORD,val1hi:WORD,val2:WORD
	mov	ax,val2
	ABSAX
	jne	@F		;dont div by zero
	inc	val2
@@:
	mov	cx,ax
	mov	ax,val1hi
	ABSAX
	inc	ax
	shl	ax,1
	cmp	ax,cx
	ja	@F
	mov	ax,val1lo
	mov	dx,val1hi
	idiv	cx
	cwd
	or	ax,ax
	ret
@@:
;overflow problem- make answer maximum
; dx =0 or -1 = sign
	mov	ax,7fffh
	xor	ax,dx
	sub	ax,dx
	ret
LDiv	ENDP

;/**********************************************************************/
;LMul(val1*val2)
LMul	PROC	val1:WORD,val2:WORD
	mov	ax,val1
	imul	val2
	ret
LMul	ENDP


;/**********************************************************************/
;LDivI(val1/val2)
LDivI   PROC	val1lo:WORD,val1hi:WORD,val2:WORD
	mov	ax,val1lo
        mov     dx,val1hi
	idiv	val2
	ret
LDivI	ENDP



;/**********************************************************************/
;MakeMatRPH(R,P,H,M1) R=roll,P=pitch,H=head,M1= dest matrix (trig style)
;M1=	[CR*CH+SR*SP*SH		SR*CP		-CR*SH+SR*SP*CH	]
;	[-SR*CH+CR*SP*SH	CR*CP		SR*SH+CR*SP*CH	]
;	[CP*SH			-SP		CP*CH		]
;
MakeMatRPH	PROC	USES di,R:WORD,P:WORD,H:WORD,M1:PTR
	mov	di,M1

	push	H
	call	SinB
	mov	SH,ax
	call	CosB		;H is still on stack for this call!!!!!
	mov	CSH,ax
	add	sp,2		;clean up

	push	R
	call	SinB
	mov	SR,ax
	call	CosB		;R is still on stack !!!!!
	mov	CR,ax
	add	sp,2		;clean up

	push	P
	call	SinB
	mov	SNP,ax
	call	CosB		;P still on stack !!!!
	mov	CP,ax
	add	sp,2


;M1=	[CR*CH+SR*SP*SH		SR*CP		-CR*SH+SR*SP*CH	]
	mov	ax,SH
	SMUL	SNP
	mov	ax,dx
	SMUL	SR
	mov	cx,dx
	mov	ax,CSH
	SMUL	CR
	add	dx,cx
	mov	[di].R1.C1,dx

	mov	ax,CP
	SMUL	SR
	mov	[di].R1.C2,dx

	mov	ax,CSH
	SMUL	SNP
	mov	ax,dx
	SMUL	SR
	mov	cx,dx
	mov	ax,SH
	SMUL	CR
	neg	dx
	add	dx,cx
	mov	[di].R1.C3,dx


;	[-SR*CH+CR*SP*SH	CR*CP		SR*SH+CR*SP*CH	]
	mov	ax,SH
	SMUL	SNP
	mov	ax,dx
	SMUL	CR
	mov	cx,dx
	mov	ax,CSH
	SMUL	SR
	neg	dx
	add	dx,cx
	mov	[di].R2.C1,dx

	mov	ax,CP
	SMUL	CR
	mov	[di].R2.C2,dx

	mov	ax,CSH
	SMUL	SNP
	mov	ax,dx
	SMUL	CR
	mov	cx,dx
	mov	ax,SH
	SMUL	SR
	add	dx,cx
	mov	[di].R2.C3,dx


;	[CP*SH			-SP		CP*CH		]
	mov	ax,SH
	SMUL	CP
	mov	[di].R3.C1,dx

	mov	dx,SNP
	neg	dx
	mov	[di].R3.C2,dx

	mov	ax,CSH
	SMUL	CP
	mov	[di].R3.C3,dx
	ret
MakeMatRPH	ENDP
;/**********************************************************************/
SumOfSquares	PROC	USES bx cx, A:WORD,B:WORD,C:WORD
	mov	ax,A
	imul	ax
	mov	bx,ax
	mov	cx,dx
	mov	ax,B
	imul	ax
	add	bx,ax
	adc	cx,dx
	mov	ax,C
	imul	ax
	add	ax,bx
	adc	dx,cx
	ret
SumOfSquares	ENDP



;/**********************************************************************/
.DATA



RMDR	dw	0

SH	DW	0
CSH	DW	0
SNP	DW	0
CP	DW	0
SR	DW	0
CR	DW	0

OCTANT_ADJUST   label word
        dw  0000h               ;   0   1       2nd byte = Even/Odd
        dw  4080h               ;  90   2
        dw  0080h               ;   0   3
        dw 0C000h               ; 270   4
        dw  8080h               ; 180   5
        dw  4000h               ;  90   6
        dw  8000h               ; 180   7
        dw 0C080h               ; 270   8

	END



