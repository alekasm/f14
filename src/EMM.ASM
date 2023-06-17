	.MODEL MEDIUM,C

	extrn	OldHandle:WORD

.CODE
EMMThere	PROC uses es si di


	mov	al,67h
	mov	ah,35h
	int	21h		;get vector 67h es:bx= handler address
	mov	di,10
	lea	si,emmname
	mov	cx,8
	repz	cmpsb
	jnz	@@bad

	mov	ah,40h
	int	67h		;get emm status
	or	ah,ah
	jne	@@bad

	mov	ax,-1
	mov	emminstalled,ax
	cwd
	ret
@@bad:
	sub	ax,ax
	mov	emminstalled,ax
	cwd
	ret
EMMThere	ENDP


EMSPageFrame	PROC
	cmp	emminstalled,0
	je	@@bad
	mov	ah,41h
	int	67h
	or	ah,ah
	jne	@@bad
	mov	ax,bx
	or	ax,ax
	ret
@@bad:
	sub	ax,ax
	ret
EMSPageFrame	ENDP


EMSPagesLeft	PROC
	cmp	emminstalled,0
	je	@@bad
	mov	ah,42h
	int	67h
	or	ah,ah
	jne	@@bad
	mov	ax,bx
	or	ax,ax
	ret
@@bad:
	sub	ax,ax
	ret
EMSPagesLeft	ENDP

EMSAllocatePages	PROC PAGES:WORD
	cmp	emminstalled,0
	je	@@bad
	mov	ah,43h
	mov	bx,PAGES
	or	bx,bx
	je	@@bad
	int	67h
	or	ah,ah
	jne	@@bad
	mov	ax,dx
	or	ax,ax
	ret
@@bad:
	sub	ax,ax
	ret

EMSAllocatePages	ENDP

EMSReleasePages	PROC HANDLE:WORD
	cmp	emminstalled,0
	je	@@bad
	mov	ah,45h
	mov	dx,HANDLE
	or	dx,dx
	je	@@bad
	int	67h
	or	ah,ah
	jne	@@bad
	mov	ax,-1
	or	ax,ax
	ret
@@bad:
	sub	ax,ax
	ret
EMSReleasePages	ENDP

EMMMapPage	PROC	HANDLE:WORD,LOGICAL:WORD, PHYSICAL:WORD
	cmp	emminstalled,0
	je	bad
	mov	bx,PHYSICAL
	cmp	bx,3
	ja	bad



	mov	dx,HANDLE
	cmp	dx,OldHandle
	je	@F
	mov	OldHandle,dx
	mov	ax,LOGICAL
	jmp	mapit
@@:
	mov	ax,LOGICAL
        cmp     emmpagemap[bx],ax
        je      OK
mapit:
        mov     emmpagemap[bx],ax

        xchg    ax,bx
        mov	ah,44h
	mov	dx,HANDLE
	int	67h
	or	ah,ah
	jne	bad
OK:
        mov	ax,-1
	or	ax,ax
	ret
bad:
	sub	ax,ax
	ret
EMMMapPage	ENDP

EMMMapMultiPage	PROC   USES si,HANDLE:WORD,STRT:WORD,LN:WORD,PG0:WORD
	cmp	emminstalled,0
	jne	@F
        sub     ax,ax
        ret
@@:
	mov	ax,HANDLE
	cmp	ax,OldHandle
        jmp     doit
@@:
        mov     bx,STRT
        add     bx,bx
        mov     cx,LN
        mov     ax,PG0
@@:
        cmp     emmpagemap[bx],ax
        jne     doit
        inc     ax
        add     bx,2
        dec     cx
        jne     @B
        mov     ax,-1
        or      ax,ax
        ret
doit:
        mov     dx,STRT
        mov     bx,dx
        add     bx,bx
        sub     si,si
        mov     cx,LN
        mov     ax,PG0
@@:
        mov     emmmpbuf[si],ax
        mov     emmmpbuf[si+2],dx
        mov     emmpagemap[bx],ax
        inc     ax
        inc     dx
        add     bx,2
        add     si,4
        dec     cx
        jne     @B

        mov     dx,HANDLE
	mov	OldHandle,dx
        lea     si,emmmpbuf
        mov     cx,LN
        mov     ax,05000h
        int     67h
@@:
        mov     ax,-1
        or      ax,ax
        ret
EMMMapMultiPage	ENDP

.DATA
emminstalled	dw	0
emmname	db	'EMMXXXX0'
emmpagemap      dw      -1,-1,-1,-1

emmmpbuf        dw      0,0, 1,1, 2,2, 3,3, 0,0, 0,0, 0,0, 0,0
	END


