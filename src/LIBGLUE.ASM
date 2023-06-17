        .MODEL MEDIUM

	Include ..\h\RpAsm.H

        public _SetOvrlyRp
        public _FixEGA

        extrn  SetOvrlyRp:far
        extrn  FixEGA:far

	extrn	ClipWd:word, ClipLn:word, XBgn:word, YBgn:word

        .CODE

_SetOvrlyRp      PROC
        mov     bx,sp
        mov     bx,[bx+4]

	mov	ax,[bx+RpXBgn]
	mov	XBgn,ax
	mov	ax,[bx+RpYBgn]
	mov	YBgn,ax
	mov	ax,[bx+RpWidth1]
	mov	ClipWd,ax
	mov	ax,[bx+RpLength1]
	mov	ClipLn,ax

        call    SetOvrlyRp
        ret
_SetOvrlyRp      ENDP

_FixEGA      PROC
        call    FixEGA
        ret
_FixEGA      ENDP
        END



