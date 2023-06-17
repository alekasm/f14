PAGE 79,124
;/************************************************/
;/*       VERTICAL RETRACE VECTOR SUPPORT        */
;/*                                              */
;/*       NAME:    CTRLVECT.ASM                  */
;/*       VERSION: 0001                          */
;/*       AUTHOR:  JOHN REEGO                    */
;/*       DATE:    8-15-87  (edit: AWH 11/01/92) */
;/*                                              */
;/*       MACHINE: IBM PC (8088)                 */
;/*                                              */
;/*       REMARKS: Contains Routines to Set and  */
;/*                Handle Control Break          */
;/*                                              */
;/*       PROCEDURES: _set_ctrl_b    (C Callable)*/
;/*                   _reset_ctrl_b  (C Callable)*/
;/*                    handle_ctrl_b             */
;/*                                              */
;/************************************************/

        TITLE   CTRLVECT.asm

        .MODEL MEDIUM

Public  _set_ctrl_b
Public  _reset_ctrl_b
Public  _ctrl_b_flag
Public  handle_ctrl_b


        .DATA

_ctrl_b_flag db 0           ; tells whether or not control break is set
CTRL_B_SEG      dw ?
CTRL_B_DISP     dw ?
CTRL_C_SEG      dw ?
CTRL_C_DISP     dw ?

        .CODE

;/******************************************************************/
;/*                                                                */
;/*       PROC-NAME   : _set_ctrl_b                                */
;/*       AUTHOR      : JOHN REEGO                                 */
;/*       PURPOSE     : sets the BIOS Control Break Interrupt      */
;/*                                                                */
;/*       INPUT       : ds = data segemnt                          */
;/*                                                                */
;/*       OUTPUT      : none                                       */
;/*                                                                */
;/*       USED REGS   : ax,bx                                      */
;/*                                                                */
;/*       REMARKS     :                                            */
;/*                                                                */
;/*       PROCEDURES  : none                                       */
;/*                                                                */
;/******************************************************************/

_set_ctrl_b PROC FAR
        push  si                          ; Save Regs
        push  di                          ;
        push  dx                          ;
        push  ds                          ;

        push  ds

        mov   si,1Bh * 4                  ; save old vector address
        call  FAR PTR get_the_vector      ;
        mov   ds:CTRL_B_DISP,bx           ;
        mov   ds:CTRL_B_SEG,ax            ;

        mov   ax, cs                      ; Set Interrupt Vector
        mov   dx, OFFSET handle_ctrl_b    ;
        mov   ds, ax                      ;
        mov   ax, 251bh                   ;
        int   21h                         ;

        pop   ds

        mov   si,23h * 4                  ; save old vector address
        call  FAR PTR get_the_vector      ;
        mov   ds:CTRL_C_DISP,bx           ;
        mov   ds:CTRL_C_SEG,ax            ;

        mov   ax, cs                      ; Set Interrupt Vector
        mov   dx, OFFSET handle_ctrl_b    ;
        mov   ds, ax                      ;
        mov   ax, 2523h                   ;
        int   21h                         ;

        pop   ds                          ; Restore Regs
        pop   dx                          ;
        pop   di                          ;
        pop   si                          ;
        ret                               ;
_set_ctrl_b ENDP

;/******************************************************************/
;/*                                                                */
;/*       PROC-NAME   : _reset_ctrl_b                              */
;/*       AUTHOR      : JOHN REEGO                                 */
;/*       PURPOSE     : reset the ctrl_b interrupt vector          */
;/*                                                                */
;/*       INPUT       : ds = data segment                          */
;/*                                                                */
;/*       OUTPUT      : none                                       */
;/*                                                                */
;/*       USED REGS   : ax,bx,cx,dx                                */
;/*                                                                */
;/*       REMARKS     :                                            */
;/*                                                                */
;/*       PROCEDURES  : None                                       */
;/*                                                                */
;/******************************************************************/

_reset_ctrl_b PROC FAR

        push  ds                           ;
        push  ds
        mov   ax, ds:CTRL_B_SEG            ; get address values for next handler
        mov   dx, ds:CTRL_B_DISP           ;
        mov   ds, ax                       ;
        mov   ax, 251bh                    ;
        int   21h                          ; Use DOS to Set the Vector

        pop   ds
        mov   ax, ds:CTRL_C_SEG            ; get address values for next handler
        mov   dx, ds:CTRL_C_DISP           ;
        mov   ds, ax                       ;
        mov   ax, 2523h                    ;
        int   21h                          ; Use DOS to Set the Vector

        pop   ds                           ;
        ret                                ;
_reset_ctrl_b ENDP

;/******************************************************************/
;/*                                                                */
;/*       PROC-NAME   : get_the_vector                             */
;/*       AUTHOR      : JOHN REEGO                                 */
;/*       PURPOSE     : get an interrupt vector                    */
;/*                                                                */
;/*       INPUT       : si = vector address                        */
;/*                                                                */
;/*       OUTPUT      : ax = vector segment                        */
;/*                     bx = vector displacement                   */
;/*                                                                */
;/*       USED REGS   : none                                       */
;/*                                                                */
;/*       REMARKS     :                                            */
;/*                                                                */
;/*       PROCEDURES  : none                                       */
;/*                                                                */
;/******************************************************************/

get_the_vector   PROC FAR
        push  ds
        xor   ax,ax            ; zero segment register
        mov   ds,ax            ;
        mov   bx,ds:[si]       ; fetch the vectors offset  part
        mov   ax,ds:[si]+2     ; fetch the vectors segment part
        pop   ds
        ret
get_the_vector   ENDP

                                      ;
handle_ctrl_b Proc FAR                ;
        push  ds                      ;
        push  ax                      ;
        mov   ax, DGROUP              ;
        mov   ds, ax                  ;
        mov   ds:_ctrl_b_flag, 1      ; set the flag to TRUE
        pop   ax                      ;
        pop   ds                      ;
        iret                          ;
handle_ctrl_b ENDP

        END

