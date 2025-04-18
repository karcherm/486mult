; Part of 486mult, trying to determine the 486 FSB multiplier using a microbenchmark
; Copyright (C) 2025  Michael Karcher

; This program is free software: you can redistribute it and/or modify
: it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.

; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>. 

.286
ITERATIONS = 1000h
base       = 16

ALIGNEDCODE SEGMENT PARA USE16
    ASSUME CS:ALIGNEDCODE
dummy      db 16h

defx MACRO N
    ALIGN 16
_x&N     PROC
    in      al, 61h
    and     al, 0FCh
    out     61h, al
    mov     ah, al
    mov     al, 0b4h
    out     43h, al
    xor     al, al
    out     42h, al
    out     42h, al
    mov     al, ah

    pushf
    push    si
    push    di
    push    ds
    push    cs
    pop     ds
    cli
    mov     bx, 1000h
    mov     si, 2000h
    mov     di, 4000h
    mov     cx, ITERATIONS
    inc     ax
    out     61h, al
l&N:
REPT base + N
    nop
ENDM
    mov     ah, [ds:bx]
    mov     ah, [ds:si]
    mov     ah, [ds:bx+si]
    mov     ah, [ds:di]
    mov     ah, [ds:bx+di]
    loop    l&N
    dec     ax
    out     61h, al
    pop     ds
    pop     di
    pop     si
    popf

    in      al, 42h
    xchg    ah, al
    in      al, 42h
    xchg    ah, al
    not     ax
    retf
_x&N     ENDP
PUBLIC _x&N
ENDM
    defx    0
    defx    1
    defx    2
    defx    3
    defx    4
    defx    5
    defx    6
    defx    7

END