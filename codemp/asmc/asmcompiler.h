// ==================================================
// ASMC(ompiler)
// --------------------------------------------------
// This utility program compiles assembly into machine 
// code at runtime with a simple yet effective API.
// --------------------------------------------------
// Written by BobaFett
// Assembler library from OllyDbg
// Copyright 2008 Lourens "BobaFett" Elzinga.
// --------------------------------------------------
// Conventions:
// 
// 1. Each opcode must be seperated by a newline
// 2. Labels must consist out of 1 word and end with a :
// 3. Symbols must be enclosed in $-signs ($symbol$)
// 4. Symbols may not contain spaces
// 5. Labels are treated as symbols, to jump to a label, use jmp $labelname$
// 6. Predefined symbols may not share names with labels
// 7. Symbols are NOT case sensitive (and therefore, neither are labels)
// 8. All symbols used must be defined, undefined symbols will abort the compilation
// 9. Comments must start with ;, block comments not supported
// 10. The code must be ANSI, unicode is not supported
// 11. All values are expected to be in hex format (use ASMC_ToHex)
// 12. Symbols are treated as text, if you with to pass an address, pass it in hex
// 13. DB may be used to specify arbitrary data
// 14. When using DB, you can either specify strings (enclosed in quotation marks) or binary values
// 15. Binary values specified in DB are treated as decimal unless they are prefixed with 0x
// 16. Use a comma (,) to seperate multiple values with DB (ie. DB "This is my strin",0x67,0 )
// 17. When using DB, hex entries may have up to 8 digits, no more.
// 18. If a value is below 255 (0xFF in hex), it will take 1 byte, otherwise it will take 4
// ==================================================

// ==================================================
// asmc_t
// --------------------------------------------------
// This is a handle to the ASMC(ompiler) struct, which
// is defined as void to prevent the user from messing up
// internal entries. The internal functions will cast
// this handle to the proper struct type.
// ==================================================

typedef void asmc_t;

// ==================================================
// T Y P E D E F   E N U M S
// --------------------------------------------------
// These enums are used as return values for 
// ASMC_Compile and opcode type tags in order to make
// our life a little easier.
// ==================================================

typedef enum
{

	ASMC_ERROR_OK,
	ASMC_ERROR_NOSOURCE,
	ASMC_ERROR_SYMBOLCLASH,
	ASMC_ERROR_BADSYMBOLREF,
	ASMC_ERROR_COMPILEERR,
	ASMC_ERROR_NOSPACE,
	ASMC_ERROR_BADARGS

} ASMC_Error_e;

typedef enum
{

	ASMC_TYPE_CODE,
	ASMC_TYPE_LABEL,
	ASMC_TYPE_DATA

} ASMC_Type_e;

// ==================================================
// ASMC_New
// --------------------------------------------------
// Creates a new ASMC(ompiler) struct and returns the 
// handle (asmc_t *).
// ==================================================

asmc_t *ASMC_New();

// ==================================================
// ASMC_Free
// --------------------------------------------------
// Frees the ASMC struct previously created by ASMC_New
// and will also set the handle to NULL (hence the **)
// ==================================================

void ASMC_Free( asmc_t **asmc );

// ==================================================
// ASMC_SetSource
// --------------------------------------------------
// Sets the asm source code to use, if copysource is 1, 
// the code will be copied internally, otherwise
// the code will be stores as a pointer to your code.
// This means you cannot free the source buffer until 
// you call ASMC_Free ( or use ASMC_SetSource again ).
// ==================================================

void ASMC_SetSource( asmc_t *asmc, int copysource, const char *source );

// ==================================================
// ASMC_ToHex
// --------------------------------------------------
// Converts a 32-bit value to its Hex counterpart.
// Because ASMC requires hex for everything ( including 
// symbols ), its highly recommended you use this function 
// to convert data and pointers.
//
// WARNING: Result is stored in a local buffer, DO NOT 
// store the pointers returned by this function. Make 
// a copy of it instead if you wish to store it
// ==================================================

const char *ASMC_ToHex( int value );

// ==================================================
// ASMC_FromHex
// --------------------------------------------------
// Converts a Hex value to 32-bit integer counterpart.
// This function supports both notations with or without 
// an 0x prefix. Due to the nature of the compiler, 9 
// digit hex codes are permitted as long as the first 
// digit is 0
// ==================================================

int ASMC_FromHex( const char* hex );

// ==================================================
// ASMC_GetSymbol
// --------------------------------------------------
// Obtains a symbol defined in the Asm Compiler object.
// This can be used to get addresses of labels ( as 
// they are converted to symbols ).
//
// NOTE: All symbols are text, so to convert symbols to 
// addresses, use ASMC_FromHex on it.
// ==================================================

const char *ASMC_GetSymbol( asmc_t *asmc, const char *symbol );

// ==================================================
// ASMC_DefineSymbol
// --------------------------------------------------
// Defines a symbol defined in the Asm Compiler object.
// Use $<symbol name>$ in your code to reference symbols!
// ==================================================

void ASMC_DefineSymbol( asmc_t *asmc, const char *symbol, const char *value );

// ==================================================
// ASMC_UndefineSymbol
// --------------------------------------------------
// Undefines a symbol which was previously defined 
// with ASMC_DefineSymbol.
// ==================================================

void ASMC_UndefineSymbol( asmc_t *asmc, const char *symbol );

// ==================================================
// ASMC_GetSize
// --------------------------------------------------
// Gets the size of the compiled code, this should 
// ONLY be used after using ASMC_Compile ( with success )!
// ==================================================

int ASMC_GetSize( asmc_t *asmc );

// ==================================================
// ASMC_GetError
// --------------------------------------------------
// Shows the error that occoured last time ASMC_Compile 
// was used. This should only be used after using 
// ASMC_Compile ( without success ).
// ==================================================

const char *ASMC_GetError( asmc_t *asmc );

// ==================================================
// ASMC_GetError
// --------------------------------------------------
// Shows the line which caused the error of ASMC_Compile.
// This should only be used after using ASMC_Compile 
// ( without success )!
// ==================================================

const char *ASMC_GetErrorLine( asmc_t *asmc );

// ==================================================
// ASMC_Compile
// --------------------------------------------------
// Attempts to compile the provided code after using
// ASMC_SetSource and stores the compiled code in
// the provided buffer.
// --------------------------------------------------
// Arguments:
//
// asmc	-			ASM Compiler object handle
// baseAddress -    Base address to use for compiling. 
//					If you intend to run the code, this should be the address
//					where the code starts.
// buffer -			Buffer where the compiled code is to be put in
// buffersize -		Size of buffer
// --------------------------------------------------
// Return values:
//
// ASMC_ERROR_OK			(0): Compiling successful
// ASMC_ERROR_NOSOURCE		(1): No source code provided (use ASMC_SetSource)
// ASMC_ERROR_SYMBOLCLASH	(2): Symbol clash (a label has the same name as a defined symbol)
// ASMC_ERROR_BADSYMBOLREF	(3): Undefined symbol referenced
// ASMC_ERROR_COMPILEERR	(4): Could not compile opcode
// ASMC_ERROR_NOSPACE		(5): Insufficient space in buffer
// ASMC_ERROR_BADARGS		(6): Arguments invalid
// ==================================================

int ASMC_Compile( asmc_t *asmc, int baseAddress, char *buffer, int buffersize );