    .ORIG x3000

;calculate sum of first 20 signed bits from xC000. Store sum at xC014
    AND R3, R3, #0      ; initialize sum to 0
    LEA R2, count
    LDW R2, R2, #0      ; R2 = 20
    LEA R0, data        ; R0 = address of data label
    LDW R0, R0, #0      ; R0 = xC000
loop LDB R1, R0, #0      ; R1 = value at data
    ADD R3, R3, R1      ; R3 = R3 + val from data array
    ADD R0, R0, #1
    ADD R2, R2, #-1
    BRp loop            ; loop while count (R2) > 0
    STW R3, R0, #0      ; store sum at xC014
    JMP R3

;Unaligned Exception
;    ADD R0, R0, #3
;    STW R3, R0, #0      ; store sum at xC017

;Invalid Opcode Exception
;    .FILL xA251         ; invalid opcode 1010, (first four bits are opcode)
;    .FILL xB000         ; invalid opcode 1011, (first four bits are opcode)

    HALT

interrupt .FILL x4000
count .FILL #20
data .FILL xC000
.END