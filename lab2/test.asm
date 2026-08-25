.ORIG x3000
    ; --- Group 1: Arithmetic & Logic ---
    ADD R1, R1, #15     ; Max positive 5-bit immediate (#15)
    ADD R2, R1, #-16    ; Max negative 5-bit immediate (-16)
    AND R3, R1, x-1     ; Hex negative constant (x-1 is all 1s)
    XOR R4, R2, R3      ; Register mode XOR
    NOT R5, R4          ; NOT instruction (Check for x3F suffix)

    ; --- Group 2: Shifts ---
    LSHF R1, R1, #4     ; Left shift 4
    RSHFL R2, R2, #3    ; Logical right shift 3
    RSHFA R3, R3, #2    ; Arithmetic right shift 2

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