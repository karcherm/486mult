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
; This needs to be a multiple of the cache way size of the L1 cache.
; If two cache lines are this far apart, they compete for the same set
; of cache lines in L1.

; Cyrix 486SLC / 486SLC2 processors   have 1 way  of 1K.
; Cyrix 80486SX/DX processors         have 4 ways of 2K each => total  8K.
; Classic Intel 80486 processors      have 4 ways of 2K each => total  8K.
; Late Intel and AMD 80486 processors have 4 ways of 4K each => total 16K.
; So 4K is a multiple of the cache way size for any kind of 486 processor.
cache_way_size = 4096

_DATA   SEGMENT WORD USE16
_iterations dw 1000h
PUBLIC _iterations
_DATA   ENDS

DGROUP  GROUP _DATA

ALIGNEDCODE SEGMENT PARA USE16
    ASSUME CS:ALIGNEDCODE, DS:DGROUP
; all data reads alias to the cache line "dummy" is stored in
; don't put measurement code here to create a virtual split between
; "data L1" and "code L1".
dummy      db 10h

defx MACRO N
_x&N     PROC
    ; disable speaker driver and timer gate
    in      al, 61h
    and     al, 0FCh
    out     61h, al
    mov     ah, al

    ; initialize timer to measure gate pulse width
    mov     al, 0b4h
    out     43h, al
    xor     al, al
    out     42h, al
    out     42h, al

    ; restore port 61h value to AL
    mov     al, ah

    push    si
    push    di
    push    ds
    mov     cx, [_iterations]
    push    cs
    pop     ds
    ; Load addresses into registers to get more compact instructions in
    ; the core loop, to reduce instruction fetching pressure on L1.
    mov     bx, cache_way_size
    mov     si, 2*cache_way_size
    mov     di, 4*cache_way_size
    inc     ax          ; set LSB of AL (open timer gate)
    jmp     SHORT a&N
    ALIGN 16
a&N:
    out     61h, al
l&N:
REPT N
    nop
ENDM
    ; start fetching a cache line
    mov     ah, [ds:bx]
    ; this instructions is stalled till the byte of the cache line
    ; is fetched
    mov     ah, [ds:bx+15]

    ; repeat the process with 4 other cache lines aliasing to the same
    ; set of L1 cache lines (5 cache lines in total), exceeding L1
    ; cache associativity.
    mov     ah, [ds:si]
    mov     ah, [ds:si+15]
    ; Some NOPs to relieve the core/L1 interface to allow undisturbed
    ; fetching of the next cache line into the prefetch queue
    nop
    nop
    nop
    nop
    mov     ah, [ds:bx+si]
    mov     ah, [ds:bx+si+15]
    mov     ah, [ds:di]
    mov     ah, [ds:di+15]
    mov     ah, [ds:bx+di]
    mov     ah, [ds:bx+di+15]
    loop    l&N
    dec     ax
    out     61h, al       ; close timer gate
    pop     ds
    pop     di
    pop     si

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
    defx    8
ENDS

END