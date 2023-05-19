;------------------------------------------------------------------------------;
;                                                                              ;
;  Go module int 24 ( critical error ) handler.                                ;
;                                                                              ;
;------------------------------------------------------------------------------;

        DOSSEG
        .MODEL SMALL

public  _goint24
public  _GoStealInt24
public  _GoRestoreInt24
public  _critical_error

.DATA
_critical_error DW      0
old24           DD      0

.DATA?
dos_version     LABEL   WORD
minorversion    DB      ?
majorversion    DB      ?

.CODE
;
; Set int 24 vector to point to my routine
;
_GoStealInt24   PROC
        push    ds
        push    es

        mov     ah , 30H        ; get dos version
        int     21H             ; call DOS
        xchg    ah , al         ; ah = major ver  al = minor ver.
        mov     word ptr dos_version , ax       ; save it

        mov     ax , 3524H      ; get vector
        int     21H

        mov     word ptr old24 , bx     ; save it
        mov     word ptr old24 + 2 , es

        push    cs
        pop     ds
        mov     dx , offset _TEXT:_goint24
        mov     ax , 2524H      ; change vector
        int     21H

        pop     es
        pop     ds
        ret
_GoStealInt24   ENDP
;
; Restore original int 24 vector
;
_GoRestoreInt24 PROC
        push    ds
        lds     dx , old24
        mov     ax , 2524H
        int     21H

        pop     ds
        ret
_GoRestoreInt24 ENDP

_goint24         PROC   FAR
        and     ah , 80H        ; disk error ?
        jne     short no_disk_err

        mov     ax , 3          ; fail !!!
        cmp     byte ptr majorversion , 3  ; fail on version 3.x
        je      short no_change
        xor     al , al         ; ignore if version 2.x
no_change:
        mov     word ptr _critical_error , 1    ; TRUE
no_disk_err:
        iret
_goint24        ENDP
                END
