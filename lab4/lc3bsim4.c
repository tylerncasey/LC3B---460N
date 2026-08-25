/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
  IRD,
  COND2,
  COND1,
  COND0,
  J5,
  J4,
  J3,
  J2,
  J1,
  J0,
  LD_MAR,
  LD_MDR,
  LD_IR,
  LD_BEN,
  LD_REG,
  LD_CC,
  LD_PC,
  LD_PRIV,
  LD_VECT,
  LD_EXC,
  LD_SSP,
  LD_USP,
  GATE_PC1,
  GATE_PC0,
  GATE_MDR,
  GATE_ALU,
  GATE_MARMUX,
  GATE_SHF,
  GATE_PSR,
  GATE_SP,
  GATE_VECT,
  PCMUX1,
  PCMUX0,
  DRMUX1,
  DRMUX0,
  SR1MUX1,
  SR1MUX0,
  ADDR1MUX,
  ADDR2MUX1,
  ADDR2MUX0,
  MARMUX,
  SPMUX1,
  SPMUX0,
  PRIVMUX,
  PSRMUX,
  VECTMUX1,
  VECTMUX0,
  ALUK1,
  ALUK0,
  MIO_EN,
  R_W,
  DATA_SIZE,
  LSHF1,
  /* MODIFY: you have to add all your new control signals */
  CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x) { return (x[IRD]); }
int GetCOND(int *x) {
  return ((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]);
} // changed
int GetJ(int *x) {
  return ((x[J5] << 5) + (x[J4] << 4) + (x[J3] << 3) + (x[J2] << 2) +
          (x[J1] << 1) + x[J0]);
}
int GetLD_MAR(int *x) { return (x[LD_MAR]); }
int GetLD_MDR(int *x) { return (x[LD_MDR]); }
int GetLD_IR(int *x) { return (x[LD_IR]); }
int GetLD_BEN(int *x) { return (x[LD_BEN]); }
int GetLD_REG(int *x) { return (x[LD_REG]); }
int GetLD_CC(int *x) { return (x[LD_CC]); }
int GetLD_PC(int *x) { return (x[LD_PC]); }
int GetGATE_PC(int *x) { return ((x[GATE_PC1] << 1) + x[GATE_PC0]); }//changed
int GetGATE_MDR(int *x) { return (x[GATE_MDR]); }
int GetGATE_ALU(int *x) { return (x[GATE_ALU]); }
int GetGATE_MARMUX(int *x) { return (x[GATE_MARMUX]); }
int GetGATE_SHF(int *x) { return (x[GATE_SHF]); }
int GetPCMUX(int *x) { return ((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x) { return ((x[DRMUX1] << 1) + x[DRMUX0]); }    // changed
int GetSR1MUX(int *x) { return ((x[SR1MUX1] << 1) + x[SR1MUX0]); } // changed
int GetADDR1MUX(int *x) { return (x[ADDR1MUX]); }
int GetADDR2MUX(int *x) { return ((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x) { return (x[MARMUX]); }
int GetALUK(int *x) { return ((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x) { return (x[MIO_EN]); }
int GetR_W(int *x) { return (x[R_W]); }
int GetDATA_SIZE(int *x) { return (x[DATA_SIZE]); }
int GetLSHF1(int *x) { return (x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
int GetLD_PRIV(int *x) { return (x[LD_PRIV]); }
int GetLD_VECT(int *x) { return (x[LD_VECT]); }
int GetLD_EXC(int *x) { return (x[LD_EXC]); }
int GetLD_SSP(int *x) { return (x[LD_SSP]); }
int GetLD_USP(int *x) { return (x[LD_USP]); }
int GetGATE_PSR(int *x) { return (x[GATE_PSR]); }
int GetGATE_SP(int *x) { return (x[GATE_SP]); }
int GetGATE_VECT(int *x) { return (x[GATE_VECT]); }
int GetSPMUX(int *x) { return ((x[SPMUX1] << 1) + x[SPMUX0]); }
int GetPRIVMUX(int *x) { return (x[PRIVMUX]); }
int GetPSRMUX(int *x) { return (x[PSRMUX]); }
int GetVECTMUX(int *x) { return ((x[VECTMUX1] << 1) + x[VECTMUX0]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct {

  int PC,  /* program counter */
      MDR, /* memory data register */
      MAR, /* memory address register */
      IR,  /* instruction register */
      N,   /* n condition bit */
      Z,   /* z condition bit */
      P,   /* p condition bit */
      BEN; /* ben register */

  int READY; /* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert
     it at a bad point in the cycle*/

  int REGS[LC_3b_REGS]; /* register file. */

  int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

  int STATE_NUMBER; /* Current State Number - Provided for debugging */

  /* For lab 4 */
  int INTV;                                 /* Interrupt vector register */
  int EXCV; /* Exception vector register */ // Don't use
  int SSP; /* Initial value of system stack pointer */
  /* MODIFY: You may add system latches that are required by your implementation
   */
  int PE;   // protection exception
  int UE;   // unaligned exception
  int PRIV; // privilege bit from PSR
  int VECT; // Vector register holding val of the exception or interrupt vector
  int USP;  // User Stack Pointer
  int INT;  // Interrupt flag

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) {
  int i;
  init_control_store(argv[1]);

  init_memory();
  for (i = 0; i < num_prog_files; i++) {
    load_program(argv[i + 2]);
  }
  CURRENT_LATCHES.Z = 1;
  CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
  memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER],
         sizeof(int) * CONTROL_STORE_BITS);
  CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
  CURRENT_LATCHES.INT = 0;  // interrupt flag is initialized to 0
  CURRENT_LATCHES.PRIV = 1; // initialize to user mode
  CURRENT_LATCHES.INTV = 0x01; // interrupt vector is initialized to 0x01
  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

#define imm5 signExtension(CURRENT_LATCHES.IR & 0x1F, 5)
#define offset6 signExtension(CURRENT_LATCHES.IR & 0x3F, 6)
#define PCoffset9 signExtension(CURRENT_LATCHES.IR & 0x1FF, 9)
#define PCoffset11 signExtension(CURRENT_LATCHES.IR & 0x7FF, 11)
#define steering ((CURRENT_LATCHES.IR >> 5) & 1)
#define sr2 (CURRENT_LATCHES.IR & 0x7)
#define sr1 (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2 ? 6 : (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1 ? ((CURRENT_LATCHES.IR >> 6) & 0x7) : ((CURRENT_LATCHES.IR >> 9) & 0x7)))
#define dr (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2 ? 6 : (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1 ? 7 : ((CURRENT_LATCHES.IR >> 9) & 0x7)))

void eval_micro_sequencer() {

  /*
   * Evaluate the address of the next state according to the
   * micro sequencer logic. Latch the next microinstruction.
   */
  // micro sequencer is responsible for the next state logic
  // microsequencer takes J, IRD, BEN, COND, R, IR[15:12] as inputs (R, BEN, IR
  // are in the latch alr), J, Cond, and IRD come from microinstruction
  int condCode = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
  int Jcode = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
  int IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);

  //Add interrupt setting logic
  if(CYCLE_COUNT == 299){
    NEXT_LATCHES.INT = 1;
  }
  if(CURRENT_LATCHES.STATE_NUMBER == 52){
    NEXT_LATCHES.INT = 0;
  }

  // Compute combinatorial exceptions right now so they are instantly accurate for this cycle
  int PE = (CURRENT_LATCHES.MAR <= 0x2FFF) && (CURRENT_LATCHES.PRIV);
  int UE = (CURRENT_LATCHES.MAR & 1) && GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);

  int EXC = PE || UE;

  if (IRD == 1) { // check IRD for Jcode or opcode
    NEXT_LATCHES.STATE_NUMBER = (CURRENT_LATCHES.IR >> 12) & 0x3F;
  } else {
    NEXT_LATCHES.STATE_NUMBER = Jcode;
    if (condCode == 1 && CURRENT_LATCHES.READY) {
      NEXT_LATCHES.STATE_NUMBER |= 0x02;
    } else if (condCode == 2 && CURRENT_LATCHES.BEN) {
      NEXT_LATCHES.STATE_NUMBER |= 0x04;
    } else if (condCode == 3 && ((CURRENT_LATCHES.IR >> 11) & 1)) {
      NEXT_LATCHES.STATE_NUMBER |= 0x01;
    } else if (condCode == 4 && CURRENT_LATCHES.PRIV) {
      NEXT_LATCHES.STATE_NUMBER |= 0x08;
    } else if (condCode == 5 && CURRENT_LATCHES.INT) {
      NEXT_LATCHES.STATE_NUMBER |= 0x10;
      NEXT_LATCHES.INT = 0;
    } else if (condCode == 6 && EXC) {
      NEXT_LATCHES.STATE_NUMBER = 63;
      NEXT_LATCHES.PE = 0;
      NEXT_LATCHES.UE = 0;
    }
  }
  memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int) * CONTROL_STORE_BITS);
}
int memReady = 0;
int memCycle = 1;
void cycle_memory() {

  /*
   * This function emulates memory and the WE logic.
   * Keep track of which cycle of MEMEN we are dealing with.
   * If fourth, we need to latch Ready bit at the end of
   * cycle to prepare microsequencer for the fifth cycle.
   */
  if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (memCycle % 4 == 0) { // check if mem is at 4th cycle
      NEXT_LATCHES.READY = 1;
      memCycle++;
    } else if (memCycle % 5 == 0) {
      memReady = 1;
      memCycle = 1;
      NEXT_LATCHES.READY = 0;
    } else {
      memCycle++;
      NEXT_LATCHES.READY = 0;
    }
  } else {
    NEXT_LATCHES.READY = 0;
    memCycle = 1;
    memReady = 0;
  }
}

int signExtension(int x, int n) {
  int shfAmount = 32 - n;
  return (x << shfAmount) >> shfAmount;
  /*
  if((x>>(n-1)) & 1){ //if most sig bit is 1 num is neg
    return (x | (0xFFFFFFFF << n)); //sign extend for 32 bits bc of C int
  }
  else
    return x;
  */
}

int MARMUX_BUS, PC_BUS, ALU_BUS, SHF_BUS, MDR_BUS;
int PSR_BUS, VECT_BUS, SP_BUS;
void eval_bus_drivers() {

  /*
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */

//MARMUX BUS 
    if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION)){
        //if MARMUX is 1 then get result from adder
        MARMUX_BUS = Low16bits(evalAddr1()+evalAddr2());
    }
    else{
        //LSHF1(ZEXT IR[7:0])
        MARMUX_BUS = Low16bits((0xFF & CURRENT_LATCHES.IR)<<1);
    }

  // PC BUS: Can send PC or PC-2 based on GATE_PC control signal
  if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
    PC_BUS = CURRENT_LATCHES.PC;
  } else if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
    PC_BUS = Low16bits(CURRENT_LATCHES.PC - 2);
  } else {
    PC_BUS = 0;
  }

  // ALU BUS
  // requires info on steering bit for either imm5 or sr2

  if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION) == 0) { // add
    if (steering) {
      ALU_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] + imm5);
    } else {
      ALU_BUS =
          Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]);
    }
  } else if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION) == 1) { // and
    if (steering) {
      ALU_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] & imm5);
    } else {
      ALU_BUS =
          Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]);
    }
  } else if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION) == 2) { // xor
    if (steering) {
      ALU_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ imm5);
    } else {
      ALU_BUS =
          Low16bits(CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2]);
    }
  } else if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION) == 3) { // passA
    ALU_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1]);
  }

  // SHF BUS
  int nLS = (CURRENT_LATCHES.IR >> 4) & 1;
  int amount4 = (CURRENT_LATCHES.IR & 0xF);
  int MSB = (CURRENT_LATCHES.REGS[sr1] >> 15);

  if (steering) { // RSHFA
    if (MSB)
      SHF_BUS = Low16bits((CURRENT_LATCHES.REGS[sr1] >> amount4) | (0xFFFF << (16 - amount4)));
    else
      SHF_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] >> amount4);
  } else if (nLS) { // RSHFL
    SHF_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] >> amount4);
  } else { // LSHF
    SHF_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] << amount4);
  }

  // MDR BUS
  int temp;
  if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) { // load word onto bus
    MDR_BUS = CURRENT_LATCHES.MDR;
  } else if (CURRENT_LATCHES.MAR & 1) { // if loading byte need to check if addr is even or odd (check LSB)
    temp = signExtension(((CURRENT_LATCHES.MDR >> 8) & 0xFF), 8);
    MDR_BUS = Low16bits(temp);
  } else {
    temp = signExtension((CURRENT_LATCHES.MDR & 0xFF), 8);
    MDR_BUS = Low16bits(temp);
  }

  // PSR BUS
  PSR_BUS = Low16bits((CURRENT_LATCHES.PRIV << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P);

  // VECT BUS
  VECT_BUS = Low16bits(0x0200 | ((CURRENT_LATCHES.VECT << 1) & 0xFF));

  // SP BUS
  if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0) {
    SP_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] + 2);
  } else if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
    SP_BUS = Low16bits(CURRENT_LATCHES.REGS[sr1] - 2);
  } else if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
    SP_BUS = Low16bits(CURRENT_LATCHES.USP);
  } else if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3) {
    SP_BUS = Low16bits(CURRENT_LATCHES.SSP);
  }
}

void drive_bus() {

  /*
   * Datapath routine for driving the bus from one of the 5 possible
   * tristate drivers.
   */
  // when there is data needed to be put on the bus, use data from eval bus
  // driver and push data onto the bus depending on gates
  if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = MARMUX_BUS;
  } else if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = PC_BUS;
  } else if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = ALU_BUS;
  } else if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = SHF_BUS;
  } else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = MDR_BUS;
  } else if (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = PSR_BUS;
  } else if (GetGATE_VECT(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = VECT_BUS;
  } else if (GetGATE_SP(CURRENT_LATCHES.MICROINSTRUCTION)) {
    BUS = SP_BUS;
  } else {
    BUS = 0;
  }
}

void latch_datapath_values() {

  /*
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come
   * after drive_bus.
   */
  // depending on LD signals for each value certain values will be sent

  // MAR always gets data from BUS
  if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    NEXT_LATCHES.MAR = BUS;
  }
  // MDR can get data from mem or write to mem
  if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION) &&
      GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (memReady) { // need to check if mem is actually ready
      NEXT_LATCHES.MDR = Low16bits((MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8) + MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
    }
    memReady = 0;
  } else if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (CURRENT_LATCHES.MAR & 1)
      NEXT_LATCHES.MDR = Low16bits((BUS << 8) + (BUS & 0xFF));
    else
      NEXT_LATCHES.MDR = Low16bits(BUS);
  }
  if (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) &&
      GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (memReady) { // need to check if mem is actually ready (also check datasize)
      if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) { // if datasize is 1 store whole word
        MEMORY[CURRENT_LATCHES.MAR >> 1][0] = CURRENT_LATCHES.MDR & 0xFF;
        MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
      } else {
        if (CURRENT_LATCHES.MAR & 1) { // store upper byte if odd addr
          MEMORY[CURRENT_LATCHES.MAR >> 1][1] =
              (CURRENT_LATCHES.MDR >> 8) & 0xFF;
        } else { // store lower byte if even addr
          MEMORY[CURRENT_LATCHES.MAR >> 1][0] = CURRENT_LATCHES.MDR & 0xFF;
        }
      }
    }
    memReady = 0;
  }
  // IR also always gets its data from bus
  if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION)) {
    NEXT_LATCHES.IR = BUS;
  }
  // BEN based off cc logic and nzp
  int nzp = (CURRENT_LATCHES.IR >> 9) & 0x7;
  if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (nzp & (CURRENT_LATCHES.N << 2))
      NEXT_LATCHES.BEN = 1;
    else if (nzp & (CURRENT_LATCHES.Z << 1))
      NEXT_LATCHES.BEN = 1;
    else if (nzp & CURRENT_LATCHES.P)
      NEXT_LATCHES.BEN = 1;
    else
      NEXT_LATCHES.BEN = 0;
  }
  // REG
  // int dr = (CURRENT_LATCHES.IR >> 9) & 0x7;
  if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION)) {
    NEXT_LATCHES.REGS[dr] = BUS;
  }
  // CC
  if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0) {
      int tempBUS = signExtension(BUS, 16);
      if (tempBUS > 0) {
        NEXT_LATCHES.P = 1;
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
      } else if (tempBUS < 0) {
        NEXT_LATCHES.P = 0;
        NEXT_LATCHES.N = 1;
        NEXT_LATCHES.Z = 0;
      } else {
        NEXT_LATCHES.P = 0;
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 1;
      }
    } else {
      NEXT_LATCHES.P = BUS & 1;
      NEXT_LATCHES.Z = (BUS >> 1) & 1;
      NEXT_LATCHES.N = (BUS >> 2) & 1;
    }
  }
  // PC
  if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0) {
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
    } else if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
      NEXT_LATCHES.PC = BUS;
    } else if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
      NEXT_LATCHES.PC = Low16bits(evalAddr1() + evalAddr2());
    }
  }

  // PRIV
  if (GetLD_PRIV(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (GetPRIVMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
      NEXT_LATCHES.PRIV = 0;
    } else {
      NEXT_LATCHES.PRIV = (BUS >> 15) & 1;
    }
  }

  // EXC (PE and UE logic)
  if (GetLD_EXC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    int isVectorM = (CURRENT_LATCHES.MAR <= 0x2FFF);
    //int isVectorP = (BUS >= 0xFE00 && BUS <= 0xFFFF);
    //NEXT_LATCHES.PE = (isVectorM || isVectorP) && CURRENT_LATCHES.PRIV;
    NEXT_LATCHES.PE = isVectorM && CURRENT_LATCHES.PRIV;
    NEXT_LATCHES.UE = (CURRENT_LATCHES.MAR & 1) && GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
  }
  
  // VECT
  if (GetLD_VECT(CURRENT_LATCHES.MICROINSTRUCTION)) {
    if (GetVECTMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
      if (CURRENT_LATCHES.PE)
        NEXT_LATCHES.VECT = 0x02; // protection exception vector
      else if (CURRENT_LATCHES.UE)
        NEXT_LATCHES.VECT = 0x03; // unaligned exception vector
    } else if (GetVECTMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2) {
      NEXT_LATCHES.VECT = 0x04; // unknown opcode exception vector
    } else if (GetVECTMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3) {
      NEXT_LATCHES.VECT = CURRENT_LATCHES.INTV; // interrupt vector
    }
  }

  // SSP
  if (GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION)) {
    NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[sr1];
  }

  // USP
  if (GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION)) {
    NEXT_LATCHES.USP = CURRENT_LATCHES.REGS[sr1];
  }
}

int evalAddr1(){ //return value from addr1mux
    if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))
        return CURRENT_LATCHES.REGS[sr1];
    else
        return CURRENT_LATCHES.PC;
}
int evalAddr2(){ //return the val from addr2mux
    if(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 0){
        return 0;
    }
    else if(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1){
        return GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) ? offset6 << 1 : offset6;
    }
    else if(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2){
        return GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) ? PCoffset9 << 1 : PCoffset9;
    }
    else if(GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 3){
        return GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION) ? PCoffset11 << 1 : PCoffset11;
    }
}