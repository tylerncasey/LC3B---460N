.ORIG x1200

STW R0, R6, #-1     ; save R0 onto stack
STW R1, R6, #-2     ; save R1 onto stack
STW R2, R6, #-3     ; save R2 onto stack
STW R3, R6, #-4     ; save R3 onto stack

LEA R0, ptbr
LDW R0, R0, #0      ; R0 = x1000 (page table base)
LEA R2, mask
LDW R2, R2, #0       ; R2 = xFFFE (mask to clear bit 0)

LEA R1, nentries
LDW R1, R1, #0       ; R1 = 128 (number of entries)

loop LDW R3, R0, #0      ; R3 = PTE value
AND R3, R3, R2       ; clear reference bit (bit 0)
STW R3, R0, #0       ; store back
ADD R0, R0, #2       ; next PTE entry (word-aligned)
ADD R1, R1, #-1      ; decrement counter
BRp loop

LDW R3, R6, #-4     ; restore R3
LDW R2, R6, #-3     ; restore R2
LDW R1, R6, #-2     ; restore R1
LDW R0, R6, #-1     ; restore R0

RTI

ptbr .FILL x1000
nentries .FILL #128
mask .FILL xFFFE
.END