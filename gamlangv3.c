/*
    gamlang compiler v3 by falkman
    Last revision: October 26, 2009
    
    Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/us/
    
    Todo:
        Add return values to function prototypes.
        
    Changes in this version:
        Integer parser improved.
        Added ability to set whether to optimize for size or speed.
        Added size optimized version for puts().
        Improved puts to correctly get the string to output.
*/

#define SPEED 0
#define SIZE  1

#define OPTIMIZE_FOR SIZE

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
    // Implemented is number for specifying if the function was
    // placed into the code as a function. If this is not null,
    // it will be the offset in the code of where it is implemented.
    int    implemented;
    
    // Called is a number of how many times the function is called
    // Stored for optimization purposes,
    int    called;
    
    // Identifier contains the function name to
    // be searched for (ex. "puts")
    // *** This must be null terminated! ***
    char   identifier[50];
    
    // params contains the formatting of the parameters
    // Each parameter is a 1 character symbol for the param.
    // Param list:
    // s = string
    // c = char
    // i = int
    // *** This list must be null terminated! ***
    char   params[10];
    
    int    (*function)(char*, struct functions*);
    struct functions *next;
};

struct program{
    char func[512];
    int  funclen;
    char *funcptr;
    
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
int addfunc(char identifier[50], char params[10], int (*function)(char*, struct functions*), func_t *to)
{
    to->next = malloc(sizeof(func_t));
    if(!to->next){
        perror("malloc() error ");
        return -1;
    }
    to = to->next;
    strcpy(to->params, params);
    strcpy(to->identifier, identifier);
    to->function = function;
    to->next = 0;
    to->implemented = 0;
    to->called = 0;
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

int add_initscr(char *code, func_t *func)
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

int add_puts(char *code, func_t *func)
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
    char puts_func_data[] = {
        0x50,             // push ax
        0x53,             // push bx
        0x51,             // push cx
        0xB4, 0x0F,       // mov ah, 0x0F
                          // lewp:
        0x8A, 0x07,       // mov al, byte [bx]
        0xAB,             // stosw
        0x43,             // inc bx
        0x49,             // dec cx
        0x75, 0xF9,       // jmp short lewp
        0x59,             // pop cx
        0x5B,             // pop bx
        0x58,             // pop ax
        0xC3
    };
    char puts_func_call[] = {
        0xBB, 0x00, 0x00, // mov bx, <dataddr>
        0xB9, 0x00, 0x00, // mov cx, <strlen>
        0xBA, 0x00, 0x00, // mov dx, <puts>
        0xFF, 0xD2        // call dx
    };
    char *ptr;
    
    puts("Adding puts to the binary...");
    
    // This next if handles optimizations, the amount of code necessary for
    // hardcoding puts is shown as:
    // s = 15c
    // s meaning the number of bytes required
    // c meaning the number of calls
    // By having puts as a function, and calling it, it initially takes 16 bytes
    // for the function, then another 11 bytes per call, yielding:
    // s = 11c + 15
    
    // If there are more than 3 calls to puts, it is more efficient (size wise)
    // to set puts up as a function, and call it.
    // After 50 calls, using it as a function would take 185 less bytes.
    code++;
    if(func->called > 3 && OPTIMIZE_FOR == SIZE){
        // Place the code into the code
        memcpy(prog.codeptr, puts_func_call, sizeof(puts_func_call));
        prog.codelen += sizeof(puts_func_call);
        prog.codeptr += sizeof(puts_func_call);
        
        // If the function is not implemented, place the function code in the data
        if(!func->implemented){
            func->implemented = prog.datalen + 0x7C00;
            memcpy(prog.dataptr, puts_func_data, sizeof(puts_func_data));
            prog.dataptr += sizeof(puts_func_data);
            prog.datalen += sizeof(puts_func_data);
        }
        // Set up the call
        addreloc(prog.codelen - 4, func->implemented);
        
        // Add the string to the data, and set up its pointer
        ptr = strchr(code, ')') - 1;
        stringlen = ptr - code;
        *ptr = 0;
        *(uint16_t*)(prog.codeptr - 7) = stringlen;
        strcpy(prog.dataptr, code);
        addreloc(prog.codelen - 10, prog.datalen + 0x7C00);
        prog.dataptr += stringlen;
        prog.datalen += stringlen;
    } else {
        // Copy the code into the code
        memcpy(prog.codeptr, puts_data, sizeof(puts_data));
        prog.codelen += sizeof(puts_data);
        prog.codeptr += sizeof(puts_data);
        
        // Add the string to the data, and set up its pointer
        ptr = strchr(code, ')') - 1;
        stringlen = ptr - code;
        *ptr = 0;
        *(uint16_t*)(prog.codeptr - 9) = stringlen;
        strcpy(prog.dataptr, code);
        addreloc(prog.codelen - 12, prog.datalen + 0x7C00);
        prog.dataptr += stringlen;
        prog.datalen += stringlen;
    }
    
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
// and checks the function for validity.
int parsefuncs(char *data, func_t *funclist)
{
    int    linenum = 1, inquote = 0, ret = 0;
    char   *ptr = data, *ptr2, *ptr3, *fparamptr, *paramptr, nulldata = '\0';
    func_t *funcptr;

    do {
        ptr2 = ptr;
        
        while(*ptr2){                                     // Hardcoded strchr() due to line counting and quote handling
            if(*ptr2 == '\"' || *ptr2 == '\''){           // inquote handler
                if(ptr2 != data){                         // Make sure we can safely check ptr-1
                    if(*(ptr2 - 1) != '\\')               // Check if it's not a control character
                        inquote = inquote ? 0 : linenum;
                }
                else inquote = inquote ? 0 : linenum;     // If we cannot check ptr-1, it can't be a control character
            }
            else if(*ptr2 == ';' && !inquote)
                break;
            else if(*ptr2 == '\n')
                linenum++;
            ptr2++;
        }
        
        if(*ptr2){
            *ptr2 = 0;
            
            while(!isalpha(*ptr)) ptr++;
            
            paramptr = strchr(ptr, '(');
            if(!paramptr){
                printf("Line %d: Expected '('\n", linenum);
                ret = -1;
            }
            funcptr = funclist;
            while(funcptr){
                if(!strncmp(funcptr->identifier, ptr, paramptr-ptr)){
                    fparamptr = funcptr->params;
                    paramptr++;
                    while(*fparamptr){
                        switch (*fparamptr) {
                            case 's':
                                if(*paramptr != '\"'){
                                    printf("Line %d: Param %d: Invalid parameter, expected string\n", linenum, (int)(fparamptr - funcptr->params + 1));
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                ptr3 = strpbrk(paramptr + 1, ",)");
                                if(!ptr3){
                                    printf("Line %d: Expected ')'\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                else if(*ptr3 == ',' && !*(fparamptr + 1)){
                                    printf("Line %d: Too many parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                else if(*ptr3 == ')' && *(fparamptr + 1)){
                                    printf("Line %d: Too few parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                paramptr = ptr3 + 1;
                                break;
                            case 'c':
                                if(*paramptr != '\'' || *(paramptr + 2) != '\''){
                                    printf("Line %d: Param %d: Invalid parameter, expected character\n", linenum, (int)(fparamptr - funcptr->params + 1));
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                else if(*(paramptr + 3) == ',' && !*(fparamptr + 1)){
                                    printf("Line %d: Too many parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                else if(*(paramptr + 3) == ')' && *(fparamptr + 1)){
                                    printf("Line %d: Too few parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                paramptr += 4;
                                break;
                            case 'i':
                                do {
                                    if(*paramptr < '0' || *paramptr > '9'){
                                        printf("Line %d: Param %d: Invalid parameter, expected integer\n", linenum, (int)(fparamptr - funcptr->params + 1));
                                        fparamptr = &nulldata;
                                        ret = -1;
                                        break;
                                    }
                                    paramptr++;
                                } while(*paramptr != ',' && *paramptr != ')' && *paramptr);
                                if(fparamptr == &nulldata)
                                    break;
                                if(*ptr3 == ',' && !*(fparamptr + 1)){
                                    printf("Line %d: Too many parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                else if(*ptr3 == ')' && *(fparamptr + 1)){
                                    printf("Line %d: Too few parameters\n", linenum);
                                    fparamptr = &nulldata;
                                    ret = -1;
                                    break;
                                }
                                paramptr = ptr3 + 1;
                        }
                        fparamptr++;
                    }
                    if(!ret) funcptr->called++;
                }
                funcptr = funcptr->next;
            }
            
            *ptr2 = ';';
            ptr = ptr2 + 1;
        }
    } while(*ptr);
    
    return ret;
}

// execfuncs by falkman
// Takes in data and the funclist, and goes through
// and executes the correct function
int execfuncs(char *data, func_t *funclist)
{
    int    linenum = 1, inquote = 0, ret = 0;
    char   *ptr = data, *ptr2, *ptr3, *paramptr;
    func_t *funcptr;

    do {
        ptr2 = ptr;
        
        while(*ptr2){                                     // Hardcoded strchr() due to line counting and quote handling
            if(*ptr2 == '\"' || *ptr2 == '\''){           // inquote handler
                if(ptr2 != data){                         // Make sure we can safely check ptr-1
                    if(*(ptr2 - 1) != '\\')               // Check if it's not a control character
                        inquote = inquote ? 0 : linenum;
                }
                else inquote = inquote ? 0 : linenum;     // If we cannot check ptr-1, it can't be a control character
            }
            else if(*ptr2 == ';' && !inquote)
                break;
            else if(*ptr2 == '\n')
                linenum++;
            ptr2++;
        }
        
        if(*ptr2){
            *ptr2 = 0;
            
            while(!isalpha(*ptr)) ptr++;
            
            paramptr = strchr(ptr, '(');
            funcptr = funclist;
            while(funcptr){
                if(!strncmp(funcptr->identifier, ptr, paramptr-ptr)){
                    paramptr++;
                    
                    ptr3 = strchr(ptr, ')') + 1;
                    *ptr3 = 0;
                    funcptr->function(paramptr, funcptr);
                    *ptr3 = ')';
                }
                funcptr = funcptr->next;
            }
            
            *ptr2 = ';';
            ptr = ptr2 + 1;
        }
    } while(*ptr);
    
    return ret;
}

int main()
{
    char   *buf = loadfile("test.gamlang");
    func_t stdfunc, *funcptr = &stdfunc;
    
    // Initilize the program
    createprog();
    
    // Check for basic syntax errors
    if(syntaxcheck(buf) < 0){
        free(buf);
        return -1;
    }
    
    // Set up functions
    strcpy(funcptr->identifier, "initscr");
    *funcptr->params = 0;
    funcptr->function = add_initscr;
    funcptr->next = 0;
    funcptr->implemented = 0;
    funcptr->called = 0;
    addfunc("puts", "s", add_puts, funcptr);
    if(parsefuncs(buf, &stdfunc) < 0){
        freefunc(stdfunc.next);
        free(buf);
        return -1;
    }
    execfuncs(buf, &stdfunc);
    
    writeprog("kernel.bin");
    freefunc(stdfunc.next);
    freereloc();
    free(buf);
    
    return 0;
}


