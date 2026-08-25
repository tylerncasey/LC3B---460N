/*
  Name 1: Tyler Casey
  UTEID 1: tnc824
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

//symbol table 
#define max_label_length 12
#define max_symbols 255
typedef struct{
    int address;
    char label[max_label_length + 1];
} TableEntry;

TableEntry symbolTable[max_symbols];
int symbolCount = 0;

FILE* infile = NULL;
FILE* outfile = NULL;

int main(int argc, char* argv[]) {
     /* open the source file */
     infile = fopen(argv[1], "r");
     outfile = fopen(argv[2], "w");
		 
     if (!infile) {
       printf("Error: Cannot open file %s\n", argv[1]);
       exit(4);
		 }
     if (!outfile) {
       printf("Error: Cannot open file %s\n", argv[2]);
       exit(4);
     }

     /* Do stuff with files */
    //first pass stores labels to symbol table
    firstPass();

    //print out symbol table
    printf("Symbol Table\n");
    for(int i = 0; i < symbolCount; i++) {
      printf("Index %d: Label = %s, Address = 0x%.4X\n", i, symbolTable[i].label, symbolTable[i].address);
    }
    //reset the file for second pass
    rewind(infile);
    //turn the asm file to machine code and produce an output file
    secondPass();

    fclose(infile);
    fclose(outfile);
}

//read hex or decimal inputs from a string and return it
int toNum( char* pStr ){
   char* t_ptr;
   char* orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
            printf("Error: invalid decimal operand, %s\n",orig_pStr);
            exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);
     if (lNeg)
       lNum = -lNum;
 
     return lNum;
   }
   else if( *pStr == 'x' )	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++){
       if (!isxdigit(*t_ptr)){
	        printf("Error: invalid hex operand, %s\n",orig_pStr);
	        exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else{
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

//takes an instruction line and splits it into the corresponding fields of the LC3b
#define MAX_LINE_LENGTH 255
	enum
	{
	   DONE, OK, EMPTY_LINE
	};

int	readAndParse( FILE* pInfile, char* pLine, char** pLabel, char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4){
	char *lRet, *lPtr;
	int i;
	if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
		return( DONE );
	for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = tolower( pLine[i] );
	   
        /* convert entire line to lowercase */
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

	/* ignore the comments */
	lPtr = pLine;

	while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' ) 
	    lPtr++;

	   *lPtr = '\0';
	   if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
		return( EMPTY_LINE );

	   if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
	   {
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   }
	   
           *pOpcode = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
           *pArg1 = lPtr;

           if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg2 = lPtr;
	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg3 = lPtr;

	   if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	   *pArg4 = lPtr;

	   return( OK );
	}

//checks whether the opcode is valid, returns a 1 for valid, and a -1 for not an opcode
int isOpcode(char* pOpcode){
    char* opList[] = {"add", "and", "br", "brn", "brz", "brp", "brzp", "brnp", "brnz", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "rti", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor"};
    for(int i = 0; i<28; i++){
        if(strcmp(pOpcode, opList[i]) == 0)
            return 1;
    }
    return -1;
}

//bind all lables to a mem addr
void firstPass(){
    int PC; //set PC to the initial .orig line

    char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4; //instruction line components
    int lRet; //0 done, 1 ok, 2 emptyLine

    do{
		lRet = readAndParse( infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
      //set program counter to the starting point 
			if(strcmp(lOpcode, ".orig") == 0){
        PC = toNum(lArg1);
        continue; //will only happen once and does not take up any memory
      }
      //assign found labels to the symbol table and increment symbol count
      if(strlen(lLabel) > 0){
        symbolTable[symbolCount].address = PC;
        strcpy(symbolTable[symbolCount].label, lLabel);
        symbolCount++;
      }
      //increment PC as you continue through the program if there is an instruction
      if(isOpcode(lOpcode) == 1 || strcmp(lOpcode, ".fill") == 0){
        PC+=2;
      }
      //continue through the file until reaching .end or no more lines
      if(strcmp(lOpcode, ".end") == 0)
        lRet = DONE;
		}
	} while( lRet != DONE ); //continue until the line has been parsed 
    
}

//translates from asm to machine code
//output file is generated
void secondPass(){
    int PC; //set PC to the initial .orig line
    char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4; //instruction line components
    int lRet; //0 done, 1 ok, 2 emptyLine
    int lInstr; //machine code for the asm line

    do{
		lRet = readAndParse( infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
      lInstr = 0;
      //set program counter to the starting point 
			if(strcmp(lOpcode, ".orig") == 0){
        PC = toNum(lArg1);
        lInstr = PC;  //.orig sets the machine code to the starting location
        fprintf(outfile, "0x%.4X\n", lInstr); //add to the file
        continue;
      }
      //.fill only requires the machine code of the loaded number
      if(strcmp(lOpcode, ".fill") == 0){
        lInstr = (toNum(lArg1) & 0xFFFF);
        fprintf(outfile, "0x%.4X\n", lInstr); //add to the file
        PC+=2;  //increment PC
        continue;
      }
      //ADD, AND, XOR
      if((strcmp(lOpcode, "add") == 0) || (strcmp(lOpcode, "and") == 0) || (strcmp(lOpcode, "xor") == 0)){
        //set opcode to machine code
        if(strcmp(lOpcode, "add") == 0)
          lInstr |= (1<<12); //bits 15-12
        else if(strcmp(lOpcode, "and") == 0)
          lInstr |= (5<<12);
        else if(strcmp(lOpcode, "xor") == 0)
          lInstr |= (9<<12);
        
        //add dr and sr1
        int DR = convertReg(lArg1);
        int SR = convertReg(lArg2);
        lInstr |= (DR<<9); //bits 11-9
        lInstr |= (SR<<6); //bits 8-6

        //check if immediate or reg
        //no need to shift since they are last bits on the instr, and no change to 
        if(lArg3[0] == 'r'){
          //r means reg so convert
          lInstr |= convertReg(lArg3);
        }
        else{ //else it is an immediate value
          lInstr |= (1<<5); //set *steering bit*
          lInstr |= (toNum(lArg3)&0x1F);
        }
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file

        PC+=2;
        continue;
      }
      //LDW,LDB,STW,STB
      if((strcmp(lOpcode, "ldw") == 0) || (strcmp(lOpcode, "ldb") == 0) || (strcmp(lOpcode, "stw") == 0) || (strcmp(lOpcode, "stb") == 0)){
        //set opcodes
        if(strcmp(lOpcode, "ldw") == 0)
          lInstr |= (6<<12); //bits 15-12
        else if(strcmp(lOpcode, "ldb") == 0)
          lInstr |= (2<<12);
        else if(strcmp(lOpcode, "stw") == 0)
          lInstr |= (7<<12);
        else if(strcmp(lOpcode, "stb") == 0)
          lInstr |= (3<<12);

        //set directory and base reg
        lInstr |= ((convertReg(lArg1))<<9); //bits 11-9
        lInstr |= ((convertReg(lArg2))<<6); //bits 8-6
        //convert the offset
        lInstr |= (toNum(lArg3) & 0x3F);
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //BR (8 variations)
      if((strcmp(lOpcode, "br") == 0) || (strcmp(lOpcode, "brn") == 0) || (strcmp(lOpcode, "brz") == 0) || (strcmp(lOpcode, "brp") == 0)||(strcmp(lOpcode, "brzp") == 0) || (strcmp(lOpcode, "brnp") == 0) || (strcmp(lOpcode, "brnz") == 0) || (strcmp(lOpcode, "brnzp") == 0)){
        //opcode is already set since lIstr is initalized to zero
        //set nzp bits 
        if(strcmp(lOpcode, "brn") == 0)
          lInstr |= (4<<9);
        else if(strcmp(lOpcode, "brz") == 0)
          lInstr |= (2<<9);
        else if(strcmp(lOpcode, "brp") == 0)
          lInstr |= (1<<9);
        else if(strcmp(lOpcode, "brzp") == 0)
          lInstr |= (3<<9);
        else if(strcmp(lOpcode, "brnp") == 0)
          lInstr |= (5<<9);
        else if(strcmp(lOpcode, "brnz") == 0)
          lInstr |= (6<<9);
        else if(strcmp(lOpcode, "brnzp") == 0 || strcmp(lOpcode, "br") == 0)
          lInstr |= (7<<9);

        //convert label to machine code
        lInstr |= (calculateOffset(lArg1, PC) & 0x1FF);
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //halt
      if((strcmp(lOpcode, "halt") == 0)){
        lInstr |= 0xF025;
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //trap
      if((strcmp(lOpcode, "trap") == 0)){
        lInstr |= 0xF000;
        lInstr |= (toNum(lArg1)&0xFF);
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //nop (x0000) so no necessary adjustments
      if(strcmp(lOpcode, "nop") == 0){
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //ret has set machine code
      if(strcmp(lOpcode, "ret") == 0){
        lInstr |= 0xC1C0;
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //rti has set machine code
      if(strcmp(lOpcode, "rti") == 0){
        lInstr |= 8<<12;
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //jmp
      if(strcmp(lOpcode, "jmp") == 0){
        lInstr |= (0xC << 12);  //set opcode
        lInstr |= (convertReg(lArg1) << 6);  //set reg
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //jsr & jsrr
      if(strcmp(lOpcode, "jsr") == 0 || strcmp(lOpcode, "jsrr") == 0){
        lInstr |= (0x4 << 12);  //set opcode
        //jsr
        if(strcmp(lOpcode, "jsr") == 0){
          lInstr |= (1<<11);
          lInstr |= (calculateOffset(lArg1, PC)&0x7FF);
        }
        else if(strcmp(lOpcode, "jsrr") == 0){
          lInstr |= (convertReg(lArg1) << 6);  //set reg
        }
        
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //lea
      if(strcmp(lOpcode, "lea") == 0){
        lInstr |= (0xE << 12);  //set opcode
        lInstr |= (convertReg(lArg1) << 9); //set destination reg
        lInstr |= (calculateOffset(lArg2, PC) & 0x1FF); //convert offset, & for if negative
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //not
      if(strcmp(lOpcode, "not") == 0){
        lInstr |= (9 << 12);  //set opcode
        lInstr |= (convertReg(lArg1) << 9);  //set DR
        lInstr |= (convertReg(lArg2) << 6);  //set SR
        lInstr |= 0x3F; //set final bits to 1
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      //shifts
      if((strcmp(lOpcode, "lshf") == 0) || (strcmp(lOpcode, "rshfl") == 0) || (strcmp(lOpcode, "rshfa") == 0)){
        lInstr |= (0xD << 12);  //set opcode
        lInstr |= (convertReg(lArg1) << 9);  //set DR
        lInstr |= (convertReg(lArg2) << 6);  //set SR
        //set the control bits (can leave lshf since 00)
        if(strcmp(lOpcode, "rshfl") == 0)
          lInstr |= (1<<4);
        else if(strcmp(lOpcode, "rshfa") == 0)
          lInstr |= (3<<4);
        lInstr |= (toNum(lArg3)&0xF);
        
        fprintf(outfile, "0x%.4X\n", lInstr); //add to output file
        PC+=2; //increment PC
        continue;
      }
      
      //continue through the file until reaching .end or no more lines
      if(strcmp(lOpcode, ".end") == 0)
        lRet = DONE;
		}
	} while( lRet != DONE ); //continue until the line has been parsed 
    
}

//convert reg to an int value
int convertReg(char* regStr){
  if(regStr[0] == 'r'){ //assuming only valid registers input
    return regStr[1] - '0'; //return the integer value of the reg
  }
}

//convert label to an offset num
int calculateOffset(char* label, int currAddr){
  for(int i = 0; i< symbolCount; i++){
    if(strcmp(label, symbolTable[i].label) == 0)
      return (symbolTable[i].address - (currAddr+2))/2;//return offset of the addr
  }
  return -1;//if not found
}