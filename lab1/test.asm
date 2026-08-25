.ORIG x3000
        ; --- Group 1: Arithmetic & Logic ---
    ADD R1, R1, #15     ; Max positive 5-bit immediate (#15), r1 holds x000F
    ADD R2, R2, #-16    ; Max negative 5-bit immediate (-16), r2 holds xFFF0
    ADD R7, R7, #-1     ; checking for overflow into 32 bits, r7 holds xFFFF
    ADD R6, R7, #1      ; would fault if not low16bits(), r6 holds x0000
    AND R3, R1, x-1     ; Hex negative constant (x-1 is all 1s), r3 holds x000F
    XOR R4, R2, R3      ; Register mode XOR, r4 holds xFFFF
    NOT R5, R4          ; NOT instruction (Check for x3F suffix), r5 holds x0000

    ; --- Group 2: Shifts ---
    LSHF R1, R1, #4     ; Left shift 4, r1 holds x00F0
    RSHFL R2, R2, #3    ; Logical right shift 3, r2 holds x1FFE
    RSHFA R3, R3, #2    ; Arithmetic right shift 2, r3 holds x0003

    ; --- Group 3: Memory & Byte Operations ---
    LEA R0, REMOTE      ; Large forward offset
    STW R1, R0, #2      ; Store Word (Word offset #2 = 4 bytes)
    LDB R6, R0, #-1     ; Load Byte (Byte offset #-1)
    STB R6, R0, #5      ; Store Byte (Byte offset #5)

    ; --- Group 4: Control Flow ---
    BRnzp SKIP          ; Unconditional branch
BACK JMP R1              ; Jump to register address
SKIP JSR SUB             ; Jump to Subroutine (11-bit offset)
    BRn BACK            ; Conditional branch (9-bit offset)
    HALT                ; Should be TRAP x25 

    .FILL x7FFF         ; Max positive 16-bit signed [cite: 52, 216]
REMOTE .FILL xFFFF      ; All ones 
SUB RET                 ; Should be 0xC1C0
.END

;answers
;0x3000
;0x126F
;0x1470
;0x567F
;0x9883
;0x9B3F
;0xD244
;0xD493
;0xD6F2
;0xE009
;0x7202
;0x2C3F
;0x3C05
;0x0E01
;0xC040
;0x4804
;0x09FD
;0xF025
;0x7FFF
;0xFFFF
;0xC1C0