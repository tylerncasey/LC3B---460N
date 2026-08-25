.ORIG x3000

LEA R0, BYTE    ;load addr of label
LDW R0, R0, #0  ;load addr

LDB R1, R0, #1
LDB R0, R0, #0  ;load data into R1 and R0
LEA R3, LMASK   
LDW R3, R3, #0  ;load a lowerMask for sign extension issues

AND R1, R1, R3
AND R0, R0, R3  ;mask to make sure data is a byte


AND R4, R4, #0
ADD R4, R4, #-1 ;r4 holds -1

AND R3, R3, #0
ADD R1, R1, #0
BRz SKIP    ;check if one of the numbers is 0

MULT ADD R3, R3, R0
ADD R1, R1, R4
BRp MULT    ;multiply the numbers together through an addition loop

SKIP LEA R2, BYTE
LDW R2, R2, #0  ;load addr x3100
STB R3, R2, #2  ;store product into x3102

LEA R0, MASK    
LDW R0, R0, #0  ;load upper mask to check for overflow

AND R1, R3, R0  ;compare product and mask, if not 0 then there is overflow
BRnp OVERFLOW

AND R0, R0, #0
STB R0, R2, #3  ;store 0 at x3103 if no overflow
BR DONE

OVERFLOW AND R0, R0, #0
ADD R0, R0, #1
STB R0, R2, #3  ;store 1 at x3103 if overflow

DONE HALT

BYTE .FILL x3100
MASK .FILL xFF00
LMASK .FILL x00FF
.END
