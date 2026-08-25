.ORIG x1200
STW R0, R6, #-1 ;store values of used regs onto stack
STW R1, R6, #-2

LEA R0, loc 
LDW R0, R0, #0 ; R0 = x4000
LDW R1, R0, #0 ; R1 = value AT x4000
ADD R1, R1, #1 ; increment value
STW R1, R0, #0 ; store back to x4000
LDW R1, R6, #-2 ;restore values of used regs from stack
LDW R0, R6, #-1

RTI ;return from interrupt
loc .FILL x4000
.END