/*****************************************************************************
TITLE: Assembler C Program																																
AUTHOR:   		ARKADEEP ACHARYA
ROLL_NUMBER:	2101AI41
DECLARATION OF AUTHORSHIP:
This c file, asm.c,is written by me as a part of the assignment of CS210 
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

//Defining Mnemonics
typedef struct Mnemonic {
	char* mnemonic;
	char* opcode;
	int no_of_operand_req;
} Mnemonic;

Mnemonic mnemonics[] = {
	{"ldc", "00000000", 1},
	{"adc", "00000001", 1},
	{"ldl", "00000010", 4},
	{"stl", "00000011", 4},
	{"ldnl", "00000100", 4},
	{"stnl", "00000101", 4},
	{"add", "00000110", 0},
	{"sub", "00000111", 0},
	{"shl", "00001000", 0},
	{"shr", "00001001", 0},
	{"adj", "00001010", 1},
	{"a2sp", "00001011", 0},
	{"sp2a", "00001100", 0},
	{"call", "00001101", 4},
	{"return", "00001110", 0},
	{"brz", "00001111", 4},
	{"brlz", "00010000", 4},
	{"br", "00010001", 4},
	{"HALT", "00010010", 0},
	{"data", NULL, 2},
	{"SET", NULL, 3}
};


//ERRORS
char* errors[] = {
	"No Error in first pass",
"duplicate label defined",
"No Such Label",
"not a number",
"Missing Operand",
"Unexpected Operand",
"Extra on End of Line",
"bogus memonic",
"Invalid Label",
"Undefined Error"
};



//SYMBOL TABLE
typedef struct SymbolTab_Node {
	char* symbol;
	int address;
	struct SymbolTab_Node* next;
} SymbolTab_Node;

// The Symbol Table
typedef struct SymbolTable {
	SymbolTab_Node* head;
	int (*insert)(struct SymbolTable*, char*);
	int (*assign)(struct SymbolTable*, char*, int);
	int (*getAddress)(const struct SymbolTable*, char*);
	bool (*hasUnassigned)(const struct SymbolTable*);
	bool (*update)(struct SymbolTable*, char*, int);
} SymbolTable;
SymbolTab_Node* createSymbolTab_Node(char* label, int address) {
	
	SymbolTab_Node* newNode = (SymbolTab_Node*) malloc(sizeof(SymbolTab_Node));
	
	newNode -> symbol = (char*) malloc(1024);
	newNode -> address = address;
	newNode -> next = NULL;
	strcpy(newNode -> symbol, label);
	
	return newNode;
}


int insertSymbolTable(SymbolTable* symbol_table, 
	char* label) {

	if (symbol_table -> head == NULL) {
		symbol_table -> head = createSymbolTab_Node(label, -1);;
		return 1;
	}

	SymbolTab_Node* head = symbol_table -> head;

	if (strcmp(label, head -> symbol) == 0) return 0;
	
	while (head -> next != NULL) {
		if (strcmp(label, (head -> next) -> symbol) == 0) return 0;
		head = head -> next;
	}

	
	head -> next = createSymbolTab_Node(label, -1);
	return 1;
}
/* Will assign the PC to a label in case it exists in the Symbol Table
 otherwise insert it and assign it the value. (RETURN 0)
 Will give error if address is already assigned (RETURN 1).*/
int assignSymbolTable(SymbolTable* symbol_table, 
	char* label, int PC) {

	(*symbol_table).insert(symbol_table, label);
	SymbolTab_Node* head = symbol_table -> head;

	while (strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}
	//check if address is already assigned
	if (head -> address != -1) return 1;

	head -> address = PC;
	return 0;
}
int getAddress(const SymbolTable* symbol_table, char* label) {
	
	SymbolTab_Node* head = symbol_table -> head;
	
	while (head != NULL && strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}

	if (head == NULL) return -1;
	return head -> address;
}

bool hasUnassigned(const SymbolTable* symbol_table) {

	SymbolTab_Node* head = symbol_table -> head;
	while (head != NULL) {
		if (head -> address == -1) return 1;
		head = head -> next;
	}
	return 0;
}

bool update(SymbolTable* symbol_table, char* label, int address) {

	SymbolTab_Node* head = symbol_table -> head;
	
	while (head != NULL && strcmp(head -> symbol, label) != 0) {
		head = head -> next;
	}

	if (head == NULL) return 0;
	head -> address = address;
	return 1;
}




// Literal Table Node
typedef struct LiteralTableNode {
	int literal;
	int address;
	struct LiteralTableNode* next;
} LiteralTableNode;

// The Literal Table
typedef struct LiteralTable {
	LiteralTableNode* head;
	int (*insert)(struct LiteralTable*, int);
	int (*assign)(struct LiteralTable*, int);
} LiteralTable;

int address_ctr;

// Function to create and return a new Literal Table Node
LiteralTableNode* createLiteralTableNode(int literal, int address) {
	
	LiteralTableNode* newNode = (LiteralTableNode*) malloc(sizeof(LiteralTableNode));
	
	newNode -> literal = literal;
	newNode -> address = address;
	newNode -> next = NULL;
	
	return newNode;
}

// Inserts a literal in the Literal Table at the end.
int insertLiteralTable(LiteralTable* literal_table, 
	int literal) {

	if (literal_table -> head == NULL) {
		literal_table -> head = createLiteralTableNode(literal, -1);
		return 1;
	}

	LiteralTableNode* head = literal_table -> head;
	while (head -> next != NULL) {
		head = head -> next;
	}

	head -> next = createLiteralTableNode(literal, -1);
	return 1;
}

int assignLiteralTable(LiteralTable* literal_table, int address) {

	address_ctr = address - 1;
	LiteralTableNode* head = literal_table -> head;
	while (head != NULL) {
		head -> address = address++;
		head = head -> next;
	}
	return 1;
}

//Symbol table and literal table is defined
SymbolTable symbol_table = {NULL, insertSymbolTable, assignSymbolTable, getAddress, hasUnassigned, update};
LiteralTable literal_table = {NULL, insertLiteralTable, assignLiteralTable};

//utility Function
// Removes comment from a line
void remove_Comment(char* line) {
	char* comment = strchr(line, ';');
	if (comment != NULL) *comment = '\0';
	return;
}

// Removes ':'. RETURN 1 if ':' is present else 0
bool hasLabel(char* line) {
	char* colon = strchr(line, ':');
	if (colon != NULL) {
		*colon = ' ';
		return 1;
	}
	return 0;
}

// To check if label is valid
bool isValidLabel(char* label) {
	int l = strlen(label);
	if (l == 0 || (!isalpha(label[0]) && label[0] != '_')) return 0;
	for (int i = 0; i < l; i++) {
		if (!isalnum(label[i]) && label[i] != '_') return 0;
	}
	return 1;
}

// RETURN 1 if valid number else 0
bool isValidNumber(char* word) {
	char* endptr = NULL;
	errno = 0;
	int number = strtol(word, &endptr, 0);
	if (number == 0 && (word == endptr || errno != 0 
		|| (errno == 0 && word && *endptr != 0))) {
		return 0;
	}
	return 1;
}

// RETURN number conversion of given valid string
int to_Number(char* word) {
	char* endptr = NULL;
	return strtol(word, &endptr, 0);
}


int is_ValidMnemonic(char* mnemonic) {
	for (int i = 0; i < 21; i++) {
		if (strcmp(mnemonics[i].mnemonic, mnemonic) == 0) 
			return mnemonics[i].no_of_operand_req + 1;
	}
	return 0;
}

// RETURN 1 if mnemonic type requires operand
bool requiresOperand(int type) {
	return type > 1;
}

// Read a word from the instruction string.
int ReadWord(char** line, char* word) {
	int lineptr;
	int scan_error=-1;
	 scan_error = sscanf((*line), "%s%n", word, &lineptr);
	if (scan_error != -1) (*line) += lineptr;
	return scan_error;
}

int logError(int errcode, int line_num, FILE* log) {
	if (errcode && errcode < 8) fprintf(log, "ERROR: %s at line %d: \n",errors[errcode], line_num);
	else if (errcode) fprintf(log, "ERROR: %s\n", errors[errcode]);
	else fprintf(log, "%s\n", errors[errcode]);
	return errcode;
}

// FIRST PASS 

/*
1="No Error in first pass",
2="duplicate label defined",
3="No Such Label",
4="not a number",
5="Missing Operand",
6="Unexpected Operand",
7="Extra on End of Line",
8="bogus memonic",
9="Invalid Label",
10"Undefined Error"*/

int  firstPass(FILE *assembly, FILE* log) {
	int PC = 0, err = 0, line_num = 0;
	char* line = (char*) malloc(1024);

	/* While lines are available, take entire line as input
	 in 'line' variable.*/
	while (fgets(line, 1024, assembly)) {
		line_num += 1;
		// Remove comment if any
		remove_Comment(line);

		char* word = (char*) malloc(1024);
		char* label = (char*) malloc(1024);

		/* Check if it is labelled. If label exists, 
		 scan it. If empty RETURN 2. If invalid RETURN 2.
		 If duplicate label detected, RETURN 1
		 Else insert it into the symbol table.*/
		bool labelled = hasLabel(line);
		if (labelled) {
			if (ReadWord(&line, label) == -1) {
				err = logError(8, line_num, log);
				continue;
			}
			if (isValidLabel(label)) {
				int is_duplicate=symbol_table.assign(&symbol_table, label, PC);
				if (is_duplicate==1) {
					err = logError(1, line_num, log);
					continue;
				}
			}
			else {
				err = logError(8, line_num, log);
				continue;
			}
		}

		/* Scan for next word. If nothing after label, continue loop.
		 Else, the next word should be mnemonic/'data'/'set'.*/
		if (ReadWord(&line, word) == -1) continue;
		int mnemonic_type = is_ValidMnemonic(word);
		if (!mnemonic_type) {
			err = logError(7, line_num, log);
			continue;
		}

		// Scan for the next word. 
		// If nothing after mnemonic and operand required, throw error
		// If something after mnemonic and operand not required, throw error
		if (requiresOperand(mnemonic_type)) {

			if (ReadWord(&line, word) == -1) {
				err = logError(4, line_num, log);
				continue;
			}
			if (!isValidLabel(word) && !isValidNumber(word)) {
				err = logError(3, line_num, log);
				continue;
			}

			if (isValidLabel(word)) 
				symbol_table.insert(&symbol_table, word);

			else if (isValidNumber(word)) {
				// If 'set' update value for the given label
				if (mnemonic_type == 4) {
					symbol_table.update(&symbol_table, label, to_Number(word));
					continue;
				}

				// // If not 'data' or 'set', then insert number in literal table
				if (mnemonic_type < 3) 
					literal_table.insert(&literal_table, to_Number(word));
			}
		} 

		else if (ReadWord(&line, word) != -1) {
			err = logError(5, line_num, log);
			continue;
		}
		
		// Find if extra words are there
		if (ReadWord(&line, word) != -1) {
			err = logError(6, line_num, log);
			continue;
		}
		PC += 1;
	}
	// Scan the Symbol Table for unassigned labels
	if(symbol_table.hasUnassigned(&symbol_table)) {
		err = logError(2, line_num, log);
	}

	// // Fill literal table with address
	 literal_table.assign(&literal_table, PC);

	if (err) return -1;
	err = logError(0, (line_num-1), log);
	return 0;
}

// UTILITY FUNCTIONS FOR SECOND PASS 

char* convertTo24bit(int x) {
	char c[] = "000000000000000000000000";
	for (int i = 23; i >= 0; i--) { 
		int k = x >> i; 
		if (k & 1) 
			c[23 - i] = '1'; 
		else
			c[23 - i] = '0';  
	} 
    char* ans = (char*) malloc(40);
    strcpy(ans, c);
    return ans;
}

char* convertTo32bit(int x) {
	char c[] = "00000000000000000000000000000000";
	for (int i = 31; i >= 0; i--) { 
		int k = x >> i; 
		if (k & 1) 
			c[31 - i] = '1'; 
		else
			c[31 - i] = '0';  
	}
    char* ans = (char*) malloc(40);
    strcpy(ans, c);
    return ans;
}

// RETURN 1 if Offset Required else 0
bool requiresOffset(char* mnemonic) {
	char* offsetMnemonics[] = {"ldl", "stl", "ldnl", "stnl", "call", "brz", "brlz", "br"};
	for (int i = 0; i < 8; i++) {
		if (strcmp(offsetMnemonics[i], mnemonic) == 0) return 1;
	}	
	return 0;
}

// Returns the 8-bit opcode of the given mnemonic
int getOpcode(char** opcode, char* mnemonic) {
	for (int i = 0; i < 20; i++) {
		if (strcmp(mnemonics[i].mnemonic, mnemonic) == 0) { 
			*opcode = mnemonics[i].opcode;
			return 1;
		}
	}
	return 0;
}

// Sets the 24-bit operand for the machine code. For data it sets 32-bits
int setOpCode(char** operand, char* word, char* mnemonic, 
	int mnemonic_type, int PC) {
	int num = 0;
	// For 'data'
	if (mnemonic_type == 3) {
		*operand = convertTo32bit(to_Number(word));
		return 1;
	}

	if (isValidNumber(word)) num = to_Number(word);
	else if (isValidLabel(word)) num = symbol_table.getAddress(&symbol_table, word);
	else return 0;

	if (requiresOffset(mnemonic) && isValidLabel(word)) num -= PC + 1;
	*operand = convertTo24bit(num);

	return 1;
}

// Convert Binary to Hexadecimal
char* BinaryToHexa(char* bin, int mode) {
	char* a = bin;
	int num = 0;
	do {
	    int b = *a == '1'? 1 : 0;
	    num = (num << 1) | b;
	    a++;
	} while (*a);
	char* hex = (char*) malloc(10);
	if (mode) sprintf(hex, "%08x", num);
	else sprintf(hex, "%x", num);
	return hex;
}



//SECOND PASS

int secondPass(FILE *assembly, FILE *object, FILE *listing, FILE* log) {
	int PC = 0, err = 0, line_num = 0;
	char* line = (char*) malloc(1024);
	fseek(assembly, 0, SEEK_SET);

	
	while (fgets(line, 1024, assembly)) {
				remove_Comment(line);
		
		char* label = (char*) malloc(1024);
		char* mnemonic = (char*) malloc(10);
		char* word = (char*) malloc(1024);
		char* opcode = (char*) malloc(10);
		char* operand = (char*) malloc(26);
		char* machine = (char*) malloc(36);
		machine[0] = '\0';

		bool labelled = hasLabel(line);
		if (labelled) {
			ReadWord(&line, label);
			fprintf(listing, "%08x\t\t\t\t%s:\n", PC, label);
		}

		if (ReadWord(&line, mnemonic) == -1) continue;
		int mnemonic_type = is_ValidMnemonic(mnemonic);
		getOpcode(&opcode, mnemonic);

		// For 'set' mnemonic type = no_of_operand-1
		if (mnemonic_type == 4) continue;

		if (requiresOperand(mnemonic_type)) {
			ReadWord(&line, word);			
			setOpCode(&operand, word, mnemonic, mnemonic_type, PC);
		}
		else setOpCode(&operand, "0", mnemonic, mnemonic_type, PC);

		strcat(machine, operand);
		if (mnemonic_type != 3) strcat(machine, opcode);

		fprintf(object, "%s\n", machine);

		fprintf(listing, "%08x\t%s\t%s\t0x%s\n", PC, BinaryToHexa(machine, 1), mnemonic, BinaryToHexa(operand, 0));
		
		PC += 1;
	}
	fprintf(log, "No Error in Pass Two\n");
	return 1;
}

int main(int argc, char* argv[]) {
	// Open the Assembly File
	FILE *assembly, *object, *listing, *log;
	
	char* file_name = (char*) malloc(1024);
	file_name[0]='\0';
	strcpy(file_name, argv[1]);
	char* ptr = strchr(file_name, '.');
	if (ptr != NULL) *ptr = '\0';

	char* object_name = (char*) malloc(1024);
	object_name[0]='\0';
	strcat(object_name, file_name);
	strcat(object_name, ".o");
	
	char* listing_name = (char*) malloc(1024);
	listing_name[0]='\0';
	strcat(listing_name, file_name);
	strcat(listing_name, ".lst");
	
	char* log_name = (char*) malloc(1024);
	log_name[0]='\0';
	strcat(log_name, file_name);
	strcat(log_name, ".log");
	
	assembly = fopen(argv[1], "r");
	object = fopen(object_name, "wb");
	listing = fopen(listing_name, "w");
	log = fopen(log_name, "w");

	// First Pass
	int first = firstPass(assembly, log);
	if (first) return 0;

	// Second Pass
	int second = secondPass(assembly, object, listing, log);

	
	fclose(log);
	fclose(listing);
	fclose(object);
	fclose(assembly);
}


/*; new_test.asm
; Test SET
val: SET 75
ldc     val
adc     val2
val2: SET 66
*/