/*****************************************************************************
TITLE: Assembler C Program																																
AUTHOR:   		ARKADEEP ACHARYA
ROLL_NUMBER:	2101AI41
DECLARATION OF AUTHORSHIP:
This c file, emu.c, is written by me as a part of the assignment of CS210 
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


// The stack pointer, program counter and the registers
int SP, PC, A, B;

// Memory with 2^24 addresses
int mem[16777216]; 
char* Mnemonic[] = {"ldc", "adc", "ldl", "stl", "ldnl", "stnl", "add", "sub", "shl", "shr", "adj", "a2sp", "sp2a", "call", "return", "brz", "brlz", "br", "HALT"};

void ldc(int value) {
	B = A;
	A = value;
}

void adc(int value) {
	A += value;
}

void ldl(int offset) {
	B = A;
	A = mem[SP + offset];
}

void stl(int offset) {
	mem[SP + offset] = A;
	A = B;
}

void ldnl(int offset) {
	A = mem[A + offset];
}

void stnl(int offset) {
	mem[A + offset] = B;
}

void add() {
	A += B;
}

void sub() {
	A = B - A;
}

void shl() {
	A = B << A;
}

void shr() {
	A = B >> A;
}

void adj(int value) {
	SP += value;
}

void a2sp() {
	SP = A;
	A = B;
}

void sp2a() {
	B = A;
	A = SP;
}

void call(int offset) {
	B = A;
	A = PC;
	PC += offset;
}

void RETURN() {
	PC = A;
	A = B;
}

void brz(int offset) {
	if (A == 0) 
	PC += offset;
}

void brlz(int offset) {
	if (A < 0) 
	PC += offset;
}

void br(int offset) {
	PC += offset;
} 
//UTILITY FUNCTION

// Return instruction type. 1 = value, 2 = offset, 0 = not required
char* getType(int mnem) {
	if (mnem < 4 || mnem == 10) return "value";
	else if (mnem == 4 || mnem == 5) return "value";
	else if (mnem == 15 || mnem == 17) return "offset";
	else if (mnem == 13 || mnem == 16) return "offset";
	return "";
}

// Print ISA
void isa() {
	fprintf(stderr, "Opcode\tMnemonic\tOperand\n");
	for (int i = 0; i <= 18; i++) {
		fprintf(stderr, "%d\t%s\t%s\n", i, Mnemonic[i], getType(i));
	}
	fprintf(stderr, "\tSET\tvalue\n");
	return;
}

// Memory Dump until the number of instructions/data provided
void printMemory(int line_num, bool isBefore) {
	if (isBefore) 
	fprintf(stderr, "\nMemory dump before execution\n");
	else 
	fprintf(stderr, "\nMemory dump after execution\n");
	for (int i = 0; i < line_num; i++) {
		if (i % 4 == 0) fprintf(stderr, "\n%08x ", i);
		fprintf(stderr, "%08x ", mem[i]);
	}
	fprintf(stderr, "\n");
	return;
}

// Memory Dump until the number on instructions/data provided
void printMemoryToFile(int line_num, FILE* file) {
	for (int i = 0; i < line_num; i++) {
		if (i % 4 == 0) fprintf(file, "\n%08x ", i);
		fprintf(file, "%08x ", mem[i]);
	}
	fprintf(file, "\n");
	return;
}

// Convert 32 bit binary string to number
int binToNumber(char* bin) {
	int number = 0;
	for (int i = 0; i < 32; i++) {
		number *= 2;
		if (bin[i] == '1') number += 1; 
	}
	return number;
}

// Read machine code into memory
int readBinary(FILE* machine) {
	char* instruction = (char*) malloc(36);
	int line_num = 0;

	//READ LINES INTO INSTRUCTION STRING WHEN AVAILABLE
	while (fgets(instruction, 36, machine)) {
		// Store instruction in memory location
		mem[line_num] = binToNumber(instruction);

		line_num += 1;
	}
	return line_num;
}

// Extract mnemonic code from instruction
int getMnemonicCode(int instruction) {
	return ((1 << 8) - 1) & instruction;
}

// Extract mnemonic code from instruction
int getOperand(int instruction) {
	return (instruction >> 8);
}


void printTrace(int opcode, int operand) {
	fprintf(stderr, "PC=%08x, SP=%08x, A=%08x, B=%08x %s\t%08x\n", PC, SP, A, B, Mnemonic[opcode], operand);
}

void print_Options() {
	fprintf(stderr, "usage: emu [options] file.o\n");
	fprintf(stderr, "-trace\tshow instruction trace\n");
	fprintf(stderr, "-before\tshow memory dump before execution\n");
	fprintf(stderr, "-after\tshow memory dump after execution\n");
	fprintf(stderr, "-isa\tdisplay ISA\n");
}


int execute_Instruction(int opcode, int operand) {
	if (opcode == 0) ldc(operand);
	if (opcode == 1) adc(operand);
	if (opcode == 2) ldl(operand);
	if (opcode == 3) stl(operand);
	if (opcode == 4) ldnl(operand);
	if (opcode == 5) stnl(operand);
	if (opcode == 6) add();
	if (opcode == 7) sub();
	if (opcode == 8) shl();
	if (opcode == 9) shr();
	if (opcode == 10) adj(operand);
	if (opcode == 11) a2sp();
	if (opcode == 12) sp2a();
	if (opcode == 13) call(operand);
	if (opcode == 14) RETURN();
	if (opcode == 15) brz(operand);
	if (opcode == 16) brlz(operand);
	if (opcode == 17) br(operand);
	else return 0;
	return 1;
}

// Executes the machine code and prints out the trace if required
// RETURN 1: Invalid Opcode
int execute(bool trace) {
	SP = (1 << 23) -1;
	int operand = 0, opcode = 0;
	int instruction_num = 0;

	while ((opcode = getMnemonicCode(mem[PC])) <= 18) {
		
		operand = getOperand(mem[PC]);

		if (trace) printTrace(opcode, operand);
		if (opcode == 18) break;
		// Invalid Opcode
		if (opcode > 18) return 1;

		PC += 1;
		execute_Instruction(opcode, operand);
		instruction_num += 1;
	}
	fprintf(stderr, "\n%d instructions executed.\n", instruction_num);
	return 0;
}


int main(int argc, char* argv[]) {
	FILE* machine;
	FILE* output;

	if (argc < 3) {
		print_Options();
		return 1;
	}

	char* file_name = (char*) malloc(1024);
	strcpy(file_name, argv[2]);
	char* ptr = strchr(file_name, '.');
	if (ptr != NULL) *ptr = '\0';

	char* out_name = (char*) malloc(1024);
	out_name[0]='\0';
	strcat(out_name, file_name);
	strcat(out_name, "_memdump.txt");

	machine = fopen(argv[2], "rb");
	output = fopen(out_name, "w");

	bool trace = (strcmp(argv[1], "-trace") == 0)? 1: 0;
	bool before = (strcmp(argv[1], "-before") == 0)? 1: 0;
	bool after = (strcmp(argv[1], "-after") == 0)? 1: 0;
	bool ISA = (strcmp(argv[1], "-isa") == 0)? 1: 0;
	
	if (!(trace || before || after || ISA)) {
		print_Options();
		return 1;
	}

	int instructions_cnt = readBinary(machine);
	// If -before then print
	if (before) printMemory(instructions_cnt, 1);
	if (ISA) isa();

	execute(trace);

	if (after) printMemory(instructions_cnt, 0);
	printMemoryToFile(instructions_cnt, output);

	// Close the assembly file
	fclose(output);
	fclose(machine);
}