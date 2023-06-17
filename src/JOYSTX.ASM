PAGE 79,124
;ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
;º********************				    ********************º
;º********************   MPS Labs Graphic Library   ********************º
;º********************				    ********************º
;ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
;º									º
;º  File:  JoyStx.asm							º
;º									º
;º  Auth:  David McKibbin						º
;º									º
;º  Edit:  dtm   September 4, 1992     5:12 pm				º
;º									º
;º  Call:    Trigger2(button#) - Query fire buttons			º
;º	     InitJoy2(mask)						º
;º	     ReadJoy2(mask) - returns a STRUCTure { signed char x, y	º
;º		      ^					       w, z }	º
;º		      ÀÄÄ bit-mask to select channels			º
;º									º
;ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¶
;º   Copyright (c) 1992 by MicroProse Software, All Rights Reserved.	º
;ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼

	TITLE JoyStx  -  JoyStick routines

	.MODEL	MEDIUM, C
	
	.DATA

        PUBLIC JoyVals
        PUBLIC MinStick
        PUBLIC MaxStick
        PUBLIC CenterPosition
        PUBLIC LowRange
        PUBLIC HighRange
        PUBLIC RawJoy
        PUBLIC DeadZone

JoyVals LABEL   BYTE

MinStick	dw	4 DUP (?)	; smallest value so far
MaxStick	dw	4 DUP (?)	; largest  value so far
CenterPosition	dw	4 DUP (?)	; raw values @ InitJoy2()

LowRange	dw	4 DUP (?)	; CenterPosition[i] - MinStick[i]
HighRange	dw	4 DUP (?)	; MaxStick[i] - CenterPosition[i]

RawJoy		dw	4 DUP (?)	; raw values (code loops)
JoyVal		db	4 DUP (?)	; scaled values [-128..0..127]

DeadZone        db      0

ColoSpecThere   dw      -1
ColoSpecPort    dw      0

	.CODE

ColoSpecSetup   PROC
        cmp     ColoSpecThere,-1
        jne     done
        mov     ColoSpecThere,0
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        mov     ax,08400h
        mov     dx,0f0h
        int     15h
        cmp     ax,'SG'
        jne     @F
        cmp     bx,0
        je      @F
        mov     ColoSpecPort,bx
        mov     ColoSpecThere,1
@@:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
done:
        ret
ColoSpecSetup   ENDP


PAGE
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   Trigger2 (button#)  --  query the JoyStick fire buttons		³
;³									³
;³	button# = [0..3]						³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
Trigger2	proc	FAR

        mov     bx,sp

        cmp     ColoSpecThere,-1
        jne     @F
        call    ColoSpecSetup
@@:
        cmp     ColoSpecThere,1
        jne     @F
        push    di
        push    si
        push    cx
        push    bx
        mov     ax,08400h
        mov     bx,ColoSpecPort
        mov     dx,0
        int     15h
        pop     bx
        pop     cx
        pop     si
        pop     di
        jmp     tr1
@@:
        mov     dx,0201h        ; Joystick port
        in      al,dx
tr1:
	not	al		; buttons are LOW true

        mov     cx,[bx+4]
        add     cx,4
        shr     al,cl
        and     ax,1

        ret

Trigger2	ENDP


;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   InitJoy2 (mask)  --  Initialize the JoySticks			³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
InitJoy2	proc	far		iMask:byte

        call    ColoSpecSetup


	push	ds
	mov	ax,DGROUP
	mov	ds,ax

	mov	ah,iMask
	call	JoyRead		; Get a Center Value

	mov	bx,0		; Init stick 1-X
iJoy:	shr	iMask,1
	jnc	@F
	call	InitS
@@:	inc	bx
	cmp	bx,4
	jb	iJoy

	pop	ds
	ret

InitJoy2	endp


;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   InitS  --  use the current STICK value for CENTER/MIN/MAX		³
;³									³
;³	entry:	BX = JoyStick index [0..3]; 0=X1, 1=Y1, 2=X2, 3=Y2	³
;³		DS = DGROUP						³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
InitS	proc	near

	shl	bx,1
	mov	ax,RawJoy[bx]
	mov	CenterPosition[bx],ax
	mov	MinStick[bx],ax
	mov	MaxStick[bx],ax
	shr	bx,1
	ret

InitS	ENDP

PAGE
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   ReadJoy2()  --  returns a STRUCTure (char x, y) of stick values	³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
ReadJoy2	proc	far		iMask:byte

	push	ds
	mov	ax,DGROUP
	mov	ds,ax

	mov	ah,iMask
	call	JoyRead

	mov	bx,0		; normalize the results
rJoy:	shr	iMask,1
	jnc	@F
	call	JoyConvert
@@:	inc	bx
	cmp	bx,4
	jb	rJoy

	mov	dx,WORD PTR JoyVal+2
	mov	ax,WORD PTR JoyVal

	pop	ds
	ret

ReadJoy2	endp

PAGE
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   JoyRead  --  DUAL STICK CODE					³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
JoyRead	proc	NEAR		; AH = bit-mask (input)

	push	bp
	push	si
	push	di

	xor	bx,bx		; BX = counter #1
	xor	bp,bp		; BP = counter #2
	xor	si,si		; SI = counter #3
	xor	di,di		; DI = counter #4

	mov	cx,-1		; CX = max of FFFF counts
	mov	dx,201h		; DX = JoyStick i/o address

        cmp     ColoSpecThere,1
        jne     @F
        mov     al,ah
        and     al,3
        add     al,2
        mov     al,1
        mov     ah,084h
        mov     bx,ColoSpecPort
        mov     dx,1
        int     15h
        mov     di,dx
        mov     si,cx
        mov     bp,bx
        mov     bx,ax
        jmp     st2
@@:

	cli
	out	dx,al
	jmp	$+2		; Let Joystick settle
	jmp	$+2

Timer:	in	al,dx
	and	al,AH		; AX = input bits of interest
	jz	Stop

	shr	al,1
	adc	bx,0
	shr	al,1
	adc	bp,0
	shr	al,1
	adc	si,0
	shr	al,1
	adc	di,0
	loop	Timer

Stop:	sti

st2:
	mov	RawJoy[0],bx
	mov	RawJoy[2],bp
	mov	RawJoy[4],si
	mov	RawJoy[6],di

	pop	di
	pop	si
	pop	bp
	ret
JoyRead	endp
PAGE
;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³									³
;³   JoyConvert  --  Normalize the current values to [-127..0..127]	³
;³									³
;³	entry:	BX = JoyStick index [0..3]; 0=X1, 1=Y1, 2=X2, 3=Y2	³
;³		DS = DGROUP						³
;³									³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
JoyConvert	proc	near

	shl	bx,1

@@:	mov	ax,RawJoy[bx]		; AX = JoyIn value
	mov	dx,ax
	sub	dx,CenterPosition[bx]	; DX = JoyIn-Center
	jb	Below
	ja	Above

;
;---- JoyIn == Center
;
Center:	mov	ah,0			; value = 0 for CENTER
	jmp	GotVal

;
;---- JoyIn < Center
;
Below:	neg	dx
	cmp	ax,MinStick[bx]
	ja	MinOK

NewMin:	;mov	MinStick[bx],ax		; new MinStick[]
;	mov	LowRange[bx],dx		; new LowRange[]
	mov	ah,-127			; value = -127 for MIN
	jmp	GotVal

MinOK:	xor	ax,ax
	div	LowRange[bx]
	shr	ax,1
        cmp     ah,DeadZone
        jae     NoDead
        sub     ax,ax
NoDead:	neg	ax			; value = [-127..0]
	jmp	GotVal

;
;---- JoyIn > Center
;
Above:	cmp	ax,MaxStick[bx]
	jb	MaxOK

NewMax:	;mov	MaxStick[bx],ax		; new MaxStick[]
;	mov	HighRange[bx],dx	; new HighRange[]
	mov	ah,127			; value = 127 for MAX
	jmp	GotVal

MaxOK:	xor	ax,ax
	div	HighRange[bx]
	shr	ax,1			; value = [0..127]

        cmp     ah,DeadZone
        jae     GotVal
        sub     ax,ax

GotVal:	shr	bx,1			; restore bx
	mov	JoyVal[bx],ah		; save value
	ret

JoyConvert	endp


	END
