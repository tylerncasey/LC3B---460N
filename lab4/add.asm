    .ORIG x3000

;initialize x4000 to 0
    LEA R0, interrupt    ; R0 = address of interrupt 
    LDW R0, R0, #0      ; R0 = value at interrupt
    ADD R3, R3, #3      ; ******Added Line for Unaligned checking*********   
;    AND R1, R1, #0      ; R1 = 0
;    ADD R1, R1, #1      ; R1 = 1
;    STW R1, R0, #0      ; interruptVal = 1

;calculate sum of first 20 signed bits from xC000. Store sum at xC014
;    AND R3, R3, #0      ; initialize sum to 0
;    LEA R2, count
;    LDW R2, R2, #0      ; R2 = 20
;    LEA R0, data        ; R0 = address of data label   PC = x3010
;    LDW R0, R0, #0      ; R0 = xC000
;loop LDB R1, R0, #0      ; R1 = value at data
;    ADD R3, R3, R1      ; R3 = R3 + val from data array
;    ADD R0, R0, #1
;    ADD R2, R2, #-1
;    BRp loop            ; loop while count (R2) > 0
;    STW R3, R0, #0      ; store sum at xC014    PC = x301E, ends at cycle 1304

;Protection Exception
;    AND R4, R4, #0
;    STW R3, R4, #0      ; store sum at x0000 

;Unaligned Exception
    ADD R0, R0, #3      
    STW R3, R0, #0      ; store sum at xC017 cycle 57 to check

;Invalid Opcode Exception
;    .FILL xA251         ; invalid opcode 1010, (first four bits are opcode)
;    .FILL xB000         ; invalid opcode 1011, (first four bits are opcode)

    ADD R0, R0, #0  ; test line
    HALT

interrupt .FILL x4000
count .FILL #20
data .FILL xC000
.END