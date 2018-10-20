/*
	gamlang compiler v1 by falkman
	Last revision: October 24, 2009
	
	Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/us/
	
	Todo:
		Add post-run function parsing to check for possible errors.
		Add param passing structure for handing to functions.
		Add return values to function prototypes.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

struct reloc {
	int          dataoff;
	int          codeoff;
	struct reloc *next;
} reloclist, *relocptr = &reloclist;

struct functions {
	char   identifier[50];
	int    (*function)(char*);
	struct functions *next;
};

struct program{
	char code[512];
	int  codelen;
	char *codeptr;
	
	char data[512];
	int  datalen;
	char *dataptr;
} prog;

typedef struct reloc     reloc_t;
typedef struct functions func_t;

// addfunc by falkman
// Takes in the indentifier, function, and allocates room and
// places them respectively in to->next.
// *** Be sure to free the linked list when finished ***
int addfunc(char identifier[50], int (*function)(char*), func_t *to)
{
	to->next = malloc(sizeof(func_t));
	if(!to->next){
		perror("malloc() error ");
		return -1;
	}
	to = to->next;
	strcpy(to->identifier, identifier);
	to->function = function;
	to->next = 0;
	return 0;
}

// freefunc by falkman
// Frees recursivly through funclist.
int freefunc(func_t *funclist)
{
	func_t *ptr;
	
	while(funclist){
		ptr = funclist->next;
		free(funclist);
		funclist = ptr;
	}
	
	return 0;
}

// addreloc by falkman
// Takes in the code offset, data offset, and allocates room and
// places them respectively in to->next.
// *** Be sure to free the linked list when finished ***
int addreloc(int codeoff, int dataoff)
{
	relocptr->next = malloc(sizeof(reloc_t));
	if(!relocptr->next){
		perror("malloc() error ");
		return -1;
	}
	relocptr = relocptr->next;
	relocptr->codeoff = codeoff;
	relocptr->dataoff = dataoff;
	relocptr->next    = 0;
	
	return 0;
}

// freereloc by falkman
// Frees recursivly through reloclist.
int freereloc()
{
	reloc_t *ptr2 = reloclist.next, *ptr;
	
	while(ptr2){
		ptr = ptr2->next;
		free(ptr2);
		ptr2 = ptr;
	}
	
	return 0;
}

// writeprog by falkman
// Links, writes, and reconfigures the pointers
int writeprog(char *filename)
{
	FILE *w = fopen(filename, "w");
	char halt_data[] = {
		0xF4,             // hlt
		0xEB, 0xFD        // jmp -3
	};
	
	if(!w){
		perror("fopen() error ");
		return -1;
	}
	memcpy(prog.codeptr, halt_data, sizeof(halt_data));
	prog.codeptr += sizeof(halt_data);
	prog.codelen += sizeof(halt_data);
	memcpy(prog.codeptr, prog.data, prog.datalen);
	relocptr = reloclist.next;
	while(relocptr){
		printf("Setting %x to %x\n", relocptr->codeoff, relocptr->dataoff + prog.codelen);
		*(uint16_t*)(prog.code + relocptr->codeoff) = relocptr->dataoff + prog.codelen;
		relocptr = relocptr->next;
	}
	fwrite(prog.code, 1, 512, w);
	fclose(w);
	
	return 0;
}

// createprog by falkman
// Properly initilizes the program structure for
// bootable usage.
void createprog()
{
	char init_data[] = {
		0xFA,             // cli
		0x31, 0xC0,       // xor ax, ax
		0x8E, 0xD0,       // mov ss, ax
		0xBC, 0x00, 0x7C, // mov sp, 0x7C00
		0xFB,             // sti
		0x8C, 0xC8,       // mov ax, cs
		0x8E, 0xD8        // mov ds, ax
	};
	
	// Set up the code
	memcpy(prog.code, init_data, sizeof(init_data));
	prog.codelen = sizeof(init_data);
	prog.codeptr = prog.code + prog.codelen;
	
	// Set up the data
	prog.datalen = 0;
	prog.dataptr = prog.data;
	
	// Add the boot sector magic
	prog.code[510] = 0x55;
	prog.code[511] = 0xAA;
}

int add_initscr(char *code)
{
	char initscr_data[] = {
		0xBF, 0x00, 0xB8, // mov di, 0xB800
		0x8E, 0xC7,       // mov es, di
		0x31, 0xFF,       // xor di, di
		0xB8, 0x03, 0x00, // mov ax, 0x0003
		0xCD, 0x10,       // int 0x10
	};
	
	puts("Adding initscreen to the binary...");
	memcpy(prog.codeptr, initscr_data, sizeof(initscr_data));
	prog.codelen += sizeof(initscr_data);
	prog.codeptr += sizeof(initscr_data);
	return 0;
}

int add_puts(char *code)
{
	int  stringlen;
	char puts_data[] = {
		0xB4, 0x0F,       // mov ah, 0x0F
		0xBB, 0x00, 0x00, // mov bx, <dataaddr>
		0xB9, 0x00, 0x00, // mov cx, <strlen>
		                  // lewp:
		0x8A, 0x07,       // mov al, byte [bx]
		0xAB,             // stosw
		0x43,             // inc bx
		0x49,             // dec cx
		0x75, 0xF9,       // jmp short lewp
	};
	char *ptr;
	
	puts("Adding puts to the binary...");
	memcpy(prog.codeptr, puts_data, sizeof(puts_data));
	prog.codelen += sizeof(puts_data);
	prog.codeptr += sizeof(puts_data);
	
	// Currently the code assumes you had quotes around the string.
	// At a later date this will be checked before even reaching this function.
	// You have been warned...
	code++;
	ptr = strchr(code, '\"');
	if(!ptr){
		puts("Can't find the closing quote");
		return -1;
	}
	stringlen = ptr - code;
	*ptr = 0;
	*(uint16_t*)(prog.codeptr - 9) = stringlen;
	strcpy(prog.dataptr, code);
	addreloc(prog.codelen - 12, prog.datalen + 0x7C00);
	prog.dataptr += stringlen;
	prog.datalen += stringlen;
	
	return 0;
}

// loadfile by falkman
// Takes the file specified by filename, and strips it of spaces
// and tabs, excluding ones in quotes. Returns a pointer to the
// data, or a null pointer on error.
// *** Be sure to free the result ***
char *loadfile(char *filename)
{
	char *data, inquote = 0, buf, *ptr;
	FILE *f = fopen(filename, "r");
	
	if(!f){
		perror("fopen() error ");
		return 0;
	}
	
	fseek(f, 0, SEEK_END);
	data = malloc(ftell(f) + 1);
	if(!data){
		perror("malloc() error ");
		fclose(f);
		return 0;
	}
	rewind(f);
	
	ptr = data;
	while((buf = fgetc(f)) != EOF){
		if(buf == '\"' || buf == '\''){
			if(ptr != data){            // Make sure we can safely check ptr-1
				if(*(ptr - 1) != '\\')  // Check if it's not a control character
					inquote = ~inquote;
			}
			else inquote = ~inquote;    // If we cannot check ptr-1, it can't be a control character
		}
		if((buf != ' ' || inquote) && buf != '\t')
			*ptr++ = buf;
	}
	*ptr = 0;
	puts(data);
	data = realloc(data, strlen(data) + 1);
	if(!data){
		puts("realloc() error ");
		fclose(f);
		return 0;
	}
	
	fclose(f);
	return data;
}

// syntaxcheck by falkman
// Takes a files data, and checks for syntax errors.
/* Current list of checks:
	Checks for invalid control characters.
	Checks for quotes needing to be closed.
*/
int syntaxcheck(char *data)
{
	int  inquote = 0, ret = 0, linenum = 1;
	char *ptr = data;
	
	while(*ptr++){
		if(*ptr == '\n')  // Newline counter
			linenum++;
		
		if(*ptr == '\\'){ // Check for invalid control characters
			if(*(ptr + 1) != '\"' && *(ptr + 1) != '\'' &&
			   *(ptr + 1) != 't'  && *(ptr + 1) != 'n'  &&
			   *(ptr + 1) != '\\'){
				   printf("Line %d: \"\\%c\" Invalid control character.\n", linenum, *(ptr + 1));
				   ret = -1;
			   }
		}
		
		if(*ptr == '\"' || *ptr == '\''){            // inquote handler
			if(ptr != data){                         // Make sure we can safely check ptr-1
				if(*(ptr - 1) != '\\')               // Check if it's not a control character
					inquote = inquote ? 0 : linenum;
			}
			else inquote = inquote ? 0 : linenum;    // If we cannot check ptr-1, it can't be a control character
		}
	}
	if(inquote)
		printf("Line %d: Expected '\"'\n", inquote);
	return ret;
}

// parsefuncs by falkman
// Takes in data and the funclist, and goes through
// and executes the designated function.
int parsefuncs(char *data, func_t *funclist)
{
	int    linenum = 0;
	char   *ptr = data, scannext = 1, *ptr2, *ptr3, *params, buf;
	func_t *funcptr = funclist;
	
	do {
		if(*ptr == '\n')  // Newline counter
			linenum++;
		
		if(scannext){
			if(isalpha(*ptr)){
				scannext = 0;
				funcptr = funclist;
				while(funcptr){
					ptr2 = strchr(ptr, '(');
					if(!ptr2){
						puts("Something went very very wrong.");
						return -1;
					}
					if(!memcmp(ptr, funcptr->identifier, ptr2-ptr)){
						ptr3 = strchr(ptr, ')');
						if(!ptr3){
							puts("Something went very very wrong.");
							return -1;
						}
						buf = *ptr3;
						*ptr3 = 0;
						params = strdup(ptr2 + 1);
						if(!params){
							perror("strdup() error ");
							return -1;
						}
						*ptr3 = buf;
						if(funcptr->function(params) < 0){
							puts("The function failed.");
							free(params);
							return -1;
						}
						free(params);
						break;
					}
					funcptr = funcptr->next;
				}
			}
		}
		
		if(*ptr == ';')
			scannext = 1;
	} while(*ptr++);
	
	return 0;
}

int main()
{
	char   *buf = loadfile("test.gamlang");
	func_t stdfunc, *funcptr = &stdfunc;
	
	// Initilize the program
	createprog();
	
	// Check for basic syntax errors
	syntaxcheck(buf);
	
	// Set up functions
	strcpy(funcptr->identifier, "initscr(");
	funcptr->function = add_initscr;
	funcptr->next = 0;
	addfunc("puts(", add_puts, funcptr);
	parsefuncs(buf, &stdfunc);
	
	writeprog("kernel.bin");
	freefunc(stdfunc.next);
	freereloc();
	free(buf);
	
	return 0;
}


