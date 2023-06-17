        .MODEL  MEDIUM

;******************************
;
;       STUFF TO DO:
;
;               1) Rounding ???
;               2) Don't do shl, rcl until after add of terms !!
;               3) switch TrgMul params so ax gets loaded from mem.
;
;******************************

        EXTRN   _O:WORD, _SinHead:WORD, _CosHead:WORD, _SinPitch:WORD, 
        EXTRN   _CosPitch:WORD, _SinRoll:WORD, _CosRoll:WORD

        .CODE   
        EXTRN   F_SinDP:FAR, F_CosDP:FAR

        GAME    EQU     1

        .CODE MPS_CODE

        PUBLIC  _MakeRotationMatrix
_MakeRotationMatrix      PROC 
        push    bp
        mov     bp,sp
        push    si
        push    di
        mov     di,[bp+6]
        mov     cx,[bp+8]
        mov     si,[bp+10]
        mov     bp,[bp+12]

        call    FAR PTR MakeRotationMatrix

        pop     di
        pop     si
        pop     bp
        ret
_MakeRotationMatrix      ENDP


TrgMul  MACRO   arg1, arg2
        mov     ax,arg1
        imul    WORD PTR arg2
        shl     ax,1
        rcl     dx,1
ENDM

        PUBLIC  MakeRotationMatrix
MakeRotationMatrix      PROC 
        mov     bx,cx           ; Sin (Head)
        call    F_SinDP
        mov     _SinHead,bx

        mov     bx,cx           ; Cos (Head)
        call    F_CosDP
        mov     _CosHead,bx

        mov     bx,bp           ; Sin (Roll)
        call    F_SinDP
        mov     _SinRoll,bx

        mov     bx,bp           ; Cos (Roll)
        call    F_CosDP
        mov     _CosRoll,bx

        mov     bx,si           ; Sin (Pitch)
        call    F_SinDP
        mov     _SinPitch,bx

        mov     bx,si           ; Cos (Pitch)
        call    F_CosDP
        mov     _CosPitch,bx


        TrgMul  _SinPitch, _SinRoll
;        shl     ax,1
;        adc     dx,0
        mov     si,dx                   ; save this one
        TrgMul  dx,_SinHead
        mov     bx,ax
        mov     cx,dx
        TrgMul  _CosHead, _CosRoll
        add     ax,bx
        adc     dx,cx
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(0*6)+(0*2),dx

        TrgMul  _SinPitch, _CosRoll
;        shl     ax,1
;        adc     dx,0
        mov     bp,dx                   ; save this one, too.
        TrgMul  dx,_SinHead
        mov     bx,ax
        mov     cx,dx
        TrgMul  _CosHead, _SinRoll
        sub     bx,ax
        sbb     cx,dx
;        shl     bx,1
;        adc     cx,0
        mov     [di]+(0*6)+(1*2),cx
        
        TrgMul  _SinHead, _CosPitch
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(0*6)+(2*2),dx

        TrgMul  _SinRoll, _CosPitch
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(1*6)+(0*2),dx

        TrgMul  _CosRoll, _CosPitch
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(1*6)+(1*2),dx

        mov     ax,_SinPitch
        neg     ax
        mov     [di]+(1*6)+(2*2),ax

        TrgMul  si,_CosHead
        mov     bx,ax
        mov     cx,dx
        TrgMul  _SinHead, _CosRoll
        sub     bx,ax
        sbb     cx,dx
;        shl     bx,1
;        adc     cx,0
        mov     [di]+(2*6)+(0*2),cx

        TrgMul  bp,_CosHead
        mov     bx,ax
        mov     cx,dx
        TrgMul  _SinHead, _SinRoll
        add     ax,bx
        adc     dx,cx
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(2*6)+(1*2),dx

        TrgMul  _CosHead, _CosPitch
;        shl     ax,1
;        adc     dx,0
        mov     [di]+(2*6)+(2*2),dx

        ret
MakeRotationMatrix      ENDP


        PUBLIC _InvertMatrix
_InvertMatrix   PROC 
        mov     bx,OFFSET DGROUP:_O

 ; fall into asm routine !       
_InvertMatrix   ENDP

InvertMatrix    PROC 
        mov     ax,[bx+2]
        xchg    ax,[bx+6]
        mov     [bx+2],ax

        mov     ax,[bx+4]
        xchg    ax,[bx+12]
        mov     [bx+4],ax

        mov     ax,[bx+10]
        xchg    ax,[bx+14]
        mov     [bx+10],ax

        ret
InvertMatrix    ENDP
PAGE
        PUBLIC  _MatMul
        IF GAME
_MatMul         PROC 
        ENDIF
        push    bp
        mov     bp,sp
        push    si
        push    di
        mov     bx,[bp+6]
        mov     si,[bp+8]
        mov     di,[bp+10]
        call    FAR PTR MatMul
        pop     di
        pop     si
        pop     bp
        ret
_MatMul         ENDP
        
MulElem MACRO   Row, Col
        TrgMul  [bx]+(Row*6)+0, [si]+(0*6)+(Col*2)
        mov     bp,ax
        mov     cx,dx

        TrgMul  [bx]+(Row*6)+2, [si]+(1*6)+(Col*2)
        add     bp,ax
        adc     cx,dx

        TrgMul  [bx]+(Row*6)+4, [si]+(2*6)+(Col*2)
        add     bp,ax
        adc     cx,dx

;        shl     bp,1
;        adc     cx,0                                    ; round it.
        mov     WORD PTR [di]+(Row*6)+(2*Col),cx        ; store it.
ENDM

        PUBLIC  MatMul
MatMul          PROC 
        MulElem 0, 0
        MulElem 0, 1
        MulElem 0, 2
        MulElem 1, 0
        MulElem 1, 1
        MulElem 1, 2
        MulElem 2, 0
        MulElem 2, 1
        MulElem 2, 2

        ret
MatMul          ENDP


        END


