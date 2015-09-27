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

#pragma warning( disable : 4996 )  // Deprecated warning
#include "../disasm/disasm.h"
#include "ASMCompiler.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// ==================================================
// L I N U X
// --------------------------------------------------
// This compiler is completely compatible with any
// Linux operating machine, after changing a few API
// names.
// ==================================================

#ifdef __linux__

	#define stricmp strcasecmp
	#define strnicmp strncasecmp
	#define _stricmp strcasecmp
	#define sprintf_s snprintf

#endif

// ==================================================
// S T R U C T
// --------------------------------------------------
// Internal structs which are used to compile everything
// of the passed source code. These contain symbols,
// opcodes and the objects.
// ==================================================

typedef struct asmc_symbol_s {

	const char				*name;		// Name of the symbol.
	const char				*value;		// Value of symbol.
	int						 isLabel;	// 1 if this symbol is a label.
	struct asmc_symbol_s	*next;		// Linked list.

} asmc_symbol_t;

typedef struct asmc_opcode_s {

	int			 type;					// ASMC_TYPE_xxx - 0 = Code, 1 = Label, 2 = Data (db).
	const char	*line;					// Pointer to line in question.
	char		 assembly[256];			// Opcode in ASM.
	char		 code[16];				// Opcode in machine code.
	int			 address;				// Address of this opcode.
	int			 size;					// Size of this opcode (in bytes).
	int			 linenr;				// Line number this opcode corresponds to.
	char		*data;					// Used for opcodes of type 2.
	int			 hasSymbols;			// 1 if the opcode has a symbol/label reference.
	int			 isJump;				// 1 if the opcode is a Jcc or a Call.
} asmc_opcode_t;

typedef struct asmc_object_s {

	asmc_symbol_t	 *symbols;			// Linked list of symbols.
	const char		 *code;				// Pointer to source code.
	int				  codeallocd;		// 1 if we have a copy of the source instead of a pointer.
	char			 *codebuff;			// Pointer to code buffer.
	const char		**lines;			// Pointer to lines inside codebuff.
	int				  linecount;
	asmc_opcode_t	**opcodes;			// Opcodes.
	int				  opcodecount;
	int				  size;				// Total size of the compiled code (set after calling ASMC_Compile).
	char			  errormsg[256];
	char			  errorline[256];

} asmc_object_t;

// ==================================================
// ASMC_New
// --------------------------------------------------
// Creates a new ASMC(ompiler) struct and returns the 
// handle (asmc_t *).
// ==================================================

asmc_t *ASMC_New() {

	asmc_object_t *asmc = malloc( sizeof( asmc_object_t ));
	memset( asmc, 0, sizeof( asmc_object_t ));
	return ( asmc_t * ) asmc;
}

// ==================================================
// ASMC_Free
// --------------------------------------------------
// Frees the ASMC struct previously created by ASMC_New
// and will also set the handle to NULL (hence the **)
// ==================================================

void ASMC_Free( asmc_t **asmc )
{
	asmc_object_t *asmcobj;
	asmc_symbol_t *sym;

	if ( !asmc || !*asmc )
	{
		return;
	}

	asmcobj = *( asmc_object_t ** ) asmc;

	for (sym = asmcobj->symbols; sym; )
	{
		void *nextptr;
		nextptr = sym->next;
		free(( void * ) sym->name );
		free(( void * ) sym->value );
		free(( void * ) sym);
		sym = ( asmc_symbol_t * ) nextptr;
	}

	if ( asmcobj->codeallocd )
	{
		free(( void * ) asmcobj->code);
	}

	if ( asmcobj->codebuff )
	{
		free(( void * ) asmcobj->codebuff);
	}

	if ( asmcobj->lines )
	{
		free(( void * ) asmcobj->lines);
	}

	free( asmcobj );
	*asmc = 0;
}

// ==================================================
// ASMC_SetSource
// --------------------------------------------------
// Sets the asm source code to use, if copysource is 1, 
// the code will be copied internally, otherwise
// the code will be stores as a pointer to your code.
// This means you cannot free the source buffer until 
// you call ASMC_Free ( or use ASMC_SetSource again ).
// ==================================================

void ASMC_SetSource( asmc_t *asmc, int copysource, const char *source )
{
	asmc_object_t *asmcobj;

	if ( !asmc )
	{
		return;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	if ( asmcobj->codeallocd )
	{
		free(( void * ) asmcobj->code );
	}

	if ( copysource )
	{
		asmcobj->codeallocd = 1;
		asmcobj->code = malloc( strlen( source ) + 1 );
		strcpy(( char * ) asmcobj->code, source );
	} 
	else 
	{
		asmcobj->codeallocd = 0;
		asmcobj->code = source;
	}
}

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

const char *ASMC_ToHex( int value )
{
	static char Buff[2][10];
	static int BuffIdx;
	const char *ret;

	sprintf_s( Buff[BuffIdx], 10, "%08X", value );

	// Check if the first 'digit' is numeric, if not, add a 0 to it
	// This is because Olly's assembler doesn't like constants starting 
	// with a non-numeric value.

	if ( Buff[BuffIdx][0] >= 'A' )
	{
		sprintf_s(Buff[BuffIdx],10,"0%08X",value);
	}

	ret = Buff[BuffIdx];
	BuffIdx = ( BuffIdx + 1 ) & 1;
	return ret;
}

// ==================================================
// ASMC_FromHex
// --------------------------------------------------
// Converts a Hex value to 32-bit integer counterpart.
// This function supports both notations with or without 
// an 0x prefix. Due to the nature of the compiler, 9 
// digit hex codes are permitted as long as the first 
// digit is 0
// ==================================================

int ASMC_FromHex( const char* hex )
{
	int len = strlen(hex);
	int pos;
	int val;
	char ch;
	int result = 0;
	int hb; // Halfbyte (see below) to start with
	int fmt;

	// ==================================================
	//      7      6      5      4      3      2      1      0    <-- Halfbytes
	//    <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <xxxx> <-- Bits ( x = bit)
	// --------------------------------------------------
	// Each hex digit represents 4 bits
	// So 8 digits and 32 bits
	// ==================================================

	if ( hex[0] == '0' && hex[1] == 'x' )
	{
		fmt = 2;
	}
	else
	{
		fmt = 0;
	}

	// Invalid length
	if ( len < ( 1 + fmt ) || len > ( 8 + fmt ))
	{
		if (len == (9 + fmt) && hex[fmt] == '0')
		{
			fmt += 1; // Allow (0x)0FFFFFFFF for example
		}
		else 
		{
			return 0;
		}
	}

	// Determine starting halfbyte:
	hb = ( len - ( 1 + fmt ));

	// Alright parse it
	for ( pos = fmt; pos < len; hb--, pos++ )
	{
		ch = hex[pos];

		if (ch >= '0' && ch <= '9')
		{
			val = ch - '0';
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			val = ch - 'A' + 10;
		}
		else if (ch >= 'a' && ch <= 'f')
		{
			val = ch - 'a' + 10;
		}
		else
		{
			return 0; // Bad digit
		}

		// Ok the char is valid, parsy time
		result |= (( val & 15 ) << ( hb * 4 ));
	}

	return result;
}

// ==================================================
// ASMC_CopyString
// --------------------------------------------------
// This is an internal function which copies the string
// using a malloc.
// ==================================================

static char *ASMC_CopyString( const char *source )
{
	int len;
	char *newstr;

	if ( !source )
	{
		return 0;
	}

	len = strlen( source );
	newstr = malloc( len + 1 );
	strncpy( newstr, source, len );
	newstr[ len ] = 0;
	return newstr;
}

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

const char *ASMC_GetSymbol( asmc_t *asmc, const char *symbol )
{
	asmc_symbol_t *sym;
	asmc_object_t *asmcobj;

	if ( !symbol || !asmc )
	{
		return 0;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	for (sym = asmcobj->symbols; sym; sym = sym->next )
	{
		if ( !_stricmp(sym->name, symbol ))
		{
			return sym->value;
		}
	}

	return 0;
}

// ==================================================
// ASMC_DefineSymbol
// --------------------------------------------------
// Defines a symbol defined in the Asm Compiler object.
// Use $<symbol name>$ in your code to reference symbols!
// ==================================================

void ASMC_DefineSymbol( asmc_t *asmc, const char *symbol, const char *value )
{
	asmc_symbol_t *sym;
	asmc_object_t *asmcobj;

	if ( !symbol || !value || !asmc )
	{
		return;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	// First check if the symbol is already defined
	for (sym = asmcobj->symbols; sym; sym = sym->next )
	{
		if ( !_stricmp(sym->name, symbol ))
		{
			if ( sym->isLabel )
			{
				return;
			}

			free(( void * ) sym->value );
			sym->value = ASMC_CopyString( value );
			return;
		}
	}

	// Symbol wasn't defined, so create a new one
	sym = malloc( sizeof( asmc_symbol_t ));
	sym->name = ASMC_CopyString( symbol );
	sym->value = ASMC_CopyString( value );
	sym->isLabel = 0;
	sym->next = asmcobj->symbols;
	asmcobj->symbols = sym;
}

// ==================================================
// ASMC_UndefineSymbol
// --------------------------------------------------
// Undefines a symbol which was previously defined 
// with ASMC_DefineSymbol.
// ==================================================

void ASMC_UndefineSymbol( asmc_t *asmc, const char *symbol )
{
	asmc_symbol_t *sym, *prev = 0;
	asmc_object_t *asmcobj;

	if ( !symbol || !asmc )
	{
		return;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	for ( sym = asmcobj->symbols; sym; prev = sym, sym = sym->next )
	{
		if ( !_stricmp(sym->name, symbol ))
		{
			free(( void * ) sym->name );
			free(( void * ) sym->value );

			if ( prev )
			{
				prev->next = sym->next;
			}
			else
			{
				asmcobj->symbols = sym->next;
			}

			free(( void * ) sym );
			break;
		}
	}
	return;
}

// ==================================================
// ASMC_DefineSymbol
// --------------------------------------------------
// Internal function which defines a label.
// ==================================================

static void ASMC_DefineLabel( asmc_t *asmc, const char *label, const char *value )
{
	asmc_symbol_t *sym;
	asmc_object_t *asmcobj;
	
	if ( !label || !value || !asmc )
	{
		return;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	// First check if the symbol is already defined
	for ( sym = asmcobj->symbols; sym; sym = sym->next )
	{
		if ( !_stricmp(sym->name, label ))
		{
			free(( void * ) sym->value );
			sym->value = ASMC_CopyString( value );
			return;
		}
	}

	// Symbol wasn't defined, so create a new one
	sym = malloc( sizeof( asmc_symbol_t ));
	sym->name = ASMC_CopyString( label );
	sym->value = ASMC_CopyString( value );
	sym->isLabel = 1;
	sym->next = asmcobj->symbols;
	asmcobj->symbols = sym;
}

// ==================================================
// ASMC_Trim
// --------------------------------------------------
// This is an internal function which removes spaces,
// collapses embedded spaces and removes trailing
// spaces.
// ==================================================

static char *ASMC_Trim( char *str )
{
	char *ibuf = str, *obuf = str;
	int i = 0, cnt = 0;

	if ( str )
	{
		//  Remove leading spaces
		for ( ibuf = str; *ibuf && isspace(*ibuf); ++ibuf )
		{
			if (str != ibuf)
			{
				memmove(str, ibuf, ibuf - str);
			}
		}

		//  Collapse embedded spaces
		while ( *ibuf )
		{
			if ( isspace( *ibuf ) && cnt )
			{
				ibuf++;
			}
			else
			{
				if ( !isspace( *ibuf ))
				{
					  cnt = 0;
				}
				else
				{
					  *ibuf = ' ';
					  cnt = 1;
				}

				obuf[i++] = *ibuf++;
			}
		}

		obuf[i] = 0;

		//  Remove trailing spaces
		while (--i >= 0)
		{
			if (!isspace(obuf[i]))
			{
				break;
			}
		}

		obuf[++i] = 0;
	}

	return str;
}

// ==================================================
// ASMC_Cleanup
// --------------------------------------------------
// Cleans up the ASMC object when either the object
// has been used before or in case the compile has
// been aborted or finished.
// ==================================================

static void ASMC_Cleanup( asmc_object_t *asmcobj, int cleansymbols )
{
	int i;
	asmc_opcode_t *opcode;
	asmc_symbol_t *sym, *next = 0, *prev = 0;

	if ( !asmcobj )
	{
		return;
	}

	if ( asmcobj->codebuff )
	{
		free(( void * ) asmcobj->codebuff );
		asmcobj->codebuff = 0;
	}

	if ( asmcobj->lines )
	{
		free(( void * ) asmcobj->lines );
		asmcobj->lines = 0;
	}

	if ( asmcobj->opcodes )
	{
		for( i = 0; i < asmcobj->opcodecount; i++ )
		{
			opcode = asmcobj->opcodes[i];

			if ( opcode->data )
			{
				free(( void * ) opcode->data );
			}

			free(( void * ) opcode );
		}

		free(( void * ) asmcobj->opcodes );
		asmcobj->opcodes = 0;
	}

	asmcobj->opcodecount = 0;

	if ( !cleansymbols )
	{
		return;
	}

	// Remove all label symbols
	for ( sym = asmcobj->symbols; sym; sym = next )
	{
		if ( sym->isLabel )
		{
			free(( void * ) sym->name );
			free(( void * ) sym->value );

			if ( prev )
			{
				prev->next = sym->next;
			}
			else 
			{
				asmcobj->symbols = sym->next;
			}

			free(( void * ) sym );
		}
		else 
		{
			prev = sym;
			next = sym->next;
		}
	}	
}

// ==================================================
// ASMC_IsValidHex
// --------------------------------------------------
// This is an interal function which validates the hex
// contact which is passed.
// ==================================================

static int ASMC_IsValidHex( const char *hex )
{
	int p = 0, s =0;
	int len = 0;

	if ( hex[0] == '0' && hex[1] == 'x' )
	{
		p = s = 2;
	}

	while ( 1 )
	{
		if ( !hex[p] )
		{
			break;
		}

		if ( hex[p] >= '0' && hex[p] <= '9' )
		{
			len++;
		} 
		else if ( hex[p] >= 'a' && hex[p] <= 'f' ) 
		{
			len++;
		} 
		else if ( hex[p] >= 'a' && hex[p] <= 'f' ) 
		{
			len++;
		} 
		else 
		{
			return 0;
		}

		p++;
	}

	if ( len > 8 || len < 1 )
	{
		if ( len == 9 && hex[s] == '0' )
		{
			return 1;	// 0FFFFFFFF is valid
		}

		return 0;
	}

	return 1;
}

// ==================================================
// ASMC_IsValidHex
// --------------------------------------------------
// This is an interal function which parses a DB
// statement.
// ==================================================

static int ASMC_ParseDB( asmc_opcode_t *op, char* errbuff, int buffsize )
{
	char buff[20];
	char *data = malloc(16384);		// Temp buffer (if you manage to overflow this one.. you deserve the crash)
	char *t = &buff[0];
	char *d = data;
	char *s;
	unsigned int val;
	int dta = 0;					// Amount of data buffered (max 8 for hex, max 15 for dec)
	int inp = 0;					// 0 = awaiting data, 1 = string, 2 = dec, 3 = hex
	int nfi = 0;					// No Further Input (set to 1 when encountering whitespace after processed data)

	// Its a db!
	// For the sake of speed, lets process this one right off the bat
	op->type = 2;
	s = ( char * ) op->line + 3;

	while ( 1 )
	{
		if ( !*s )
		{
			if ( inp == 3 )
			{
				if ( dta == 0 )
				{
					// yeah right.. DB 0x,"lol"...error out
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Invalid Hex notation" );
					return 0;
				}

				*t = 0;

				if ( !ASMC_IsValidHex( buff ))
				{
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Invalid Hex notation: %s", &buff[0] );
					return 0;					
				}

				val = ASMC_FromHex(buff);

				if ( val < 255 )
				{
					*d++ = val;
				}
				else
				{
					*( int * ) d = val;
					d += 4;
				}
			}

			if ( inp == 2 )
			{
				*t = 0;
				val = atoi( buff );

				if ( val < 255 )
				{
					*d++ = val;
				}
				else
				{
					*( int * ) d = val;
					d += 4;
				}
			}

			break;
		}

		if ( isspace( *s ))
		{
			if ( inp != 0 )
			{
				nfi = 1;
			}

			s++;
			continue;
		}	

		if ( *s == ',' )
		{
			nfi = 0;

			if ( inp == 3 )
			{
				if ( dta == 0 )
				{
					// yeah right.. DB 0x,"lol"...error out
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Invalid Hex notation" );
					return 0;
				}

				*t = 0;

				if ( !ASMC_IsValidHex( buff ))
				{
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Invalid Hex notation: %s", &buff[0] );
					return 0;					
				}

				val = ASMC_FromHex( buff );

				if ( val < 255 )
				{
					*d++ = val;
				} 
				else 
				{
					*( int * ) d = val;
					d += 4;
				}
			}

			if ( inp == 2 )
			{
				*t = 0;
				val = atoi( buff );

				if ( val<255 )
				{
					*d++ = val;
				}
				else
				{
					*( int * ) d = val;
					d += 4;
				}
			}

			inp = 0;
			buff[0] = 0;
			t = &buff[0];
			s++;
			continue;
		}

		// Parse string
		if ( *s == '"' && inp == 0 && !nfi )
		{
			inp = 1;
			s++;

			while ( *s != '"' )
			{
				if ( !*s )
				{
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: String was not ended properly" );
					return 0;
				}

				*d++ = *s++;
			}

			s++;
			continue;
		}

		// Hex symbol - Parse up to 8 digits ( any more = error )
		if ( *s == '0' && *(s+1) == 'x' && inp == 0 )
		{
			inp = 3;
			dta = 0;
			s += 2;
			continue;
		}

		if ( *s >= '0' && *s <= '9' && !nfi )
		{
			// Decimal input
			if ( inp == 0 )
			{
				inp = 2;
				dta = 0;
				*t++ = *s++;
				continue;
			}
			// continue decimal input
			else if ( inp == 2 )
			{
				*t++ = *s++;
				dta++;

				if ( dta > 15 )
				{
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Oversize decimal value encountered" );
					return 0;
				}
			} 
			else if ( inp == 3 )
			{
				*t++ = *s++;
				dta++;

				if ( dta > 8 )
				{
					sprintf_s( errbuff, buffsize, "Error parsing db opcode: Oversize hex value encountered" );
					return 0;
				}
			}
			else
			{
				// This should never happen....
				sprintf_s( errbuff, buffsize, "Internal error (db parser: inp = %i)", inp );
				return 0;
			}

			continue;
		}

		if ( *s >= 'a' && *s <= 'f' && inp == 3 && !nfi )
		{
			*t++ = *s++;
			dta++;

			if ( dta > 8 )
			{
				sprintf_s( errbuff, buffsize, "Error parsing db opcode: Oversize hex value encountered" );
				return 0;
			}

			continue;
		}

		if ( *s >= 'A' && *s <= 'F' && inp == 3 && !nfi )
		{
			*t++ = *s++;
			dta++;

			if ( dta > 8 )
			{
				// Oversize
				sprintf_s( errbuff, buffsize, "Error parsing db opcode: Oversize hex value encountered" );
				return 0;
			}
			continue;
		}

		sprintf_s( errbuff, buffsize, "Error parsing db opcode: Invalid token encountered" );
		return 0;
	}

	// Determine the amount of data we got
	op->size = d - data;
	op->data = malloc( op->size );
	memcpy( op->data, data, op->size );
	free( data );
	return 1;
}

// ==================================================
// ASMC_GetSize
// --------------------------------------------------
// Gets the size of the compiled code, this should 
// ONLY be used after using ASMC_Compile ( with success )!
// ==================================================

int ASMC_GetSize( asmc_t *asmc )
{
	asmc_object_t *asmcobj;
	if ( !asmc ) return 0;
	asmcobj = ( asmc_object_t * ) asmc;
	return asmcobj->size;
}

// ==================================================
// ASMC_GetError
// --------------------------------------------------
// Shows the error that occoured last time ASMC_Compile 
// was used. This should only be used after using 
// ASMC_Compile ( without success ).
// ==================================================

const char *ASMC_GetError( asmc_t *asmc )
{
	asmc_object_t *asmcobj;
	if ( !asmc ) return 0;
	asmcobj = ( asmc_object_t * ) asmc;
	return asmcobj->errormsg;
}

// ==================================================
// ASMC_GetError
// --------------------------------------------------
// Shows the line which caused the error of ASMC_Compile.
// This should only be used after using ASMC_Compile 
// ( without success )!
// ==================================================

const char *ASMC_GetErrorLine( asmc_t *asmc )
{
	asmc_object_t *asmcobj;
	if ( !asmc ) return 0;
	asmcobj = ( asmc_object_t * )asmc;
	return asmcobj->errorline;
}

// ==================================================
// ASMC_ParseSymbols
// --------------------------------------------------
// This is an internal function which parses all the
// opcode's symbols and stores the parsed result in
// op->assembly.
// --------------------------------------------------
// Return codes:
//
// 0: OK
// 1: Parse error
// 2: Undefined symbol referenced
// ==================================================

static int ASMC_ParseSymbols( asmc_t *asmc, asmc_opcode_t *op, char* errbuff, int buffsize )
{
	char symb[256];
	char *sym = &symb[0];
	const char *symbl;
	const char *s = op->line;
	char *a = op->assembly;

	while( 1 )
	{
		if ( !*s )
		{
			*a = 0;
			break;
		}

		// We got a symbol, find the other $
		if ( *s == '$' )
		{
			sym = &symb[0];
			s++;
			while ( *s != '$' )
			{
				// Other $ not found, error
				if ( !*s )
				{
					sprintf_s( errbuff, buffsize, "Error parsing symbol: Could not find closing $ marker" );
					return 1;
				}

				*sym++ = *s++;
			}

			*s++;
			*sym = 0;
			symbl = ASMC_GetSymbol( asmc, symb );

			if ( !symbl )
			{
				sprintf_s( errbuff, buffsize, "Error parsing symbol: Undefined symbol referenced: %s", symb );
				return 2;
			}

			*a = 0;
			strcat( a,symbl );
			a += strlen( symbl );
			continue;
		}

		*a++ = *s++;
	}

	return 0;
}

// ==================================================
// ASMC_CompileOpcode
// --------------------------------------------------
// This is an internal function which compiles a single 
// opcode and take the code in op->assembly and
// compile that. If successful, it will return 0, 
// otherwise it'll return 1. Due to the different ways 
// an opcode can be compiled this function tries all 
// possibilities and searches for the shortest opcode.
// ==================================================

static int ASMC_CompileOpcode( asmc_opcode_t *op, char *errbuff, int buffsize )
{
	t_asmmodel asm;
	t_asmmodel final;
	int attempt;
	int constsize;
	char error[256];
	int oldlen = 20;
	int newlen;

	attempt = 0;

	while ( 1 )
	{
		for ( constsize = 0; constsize < 4; constsize++ )
		{
			newlen = Assemble( op->assembly, op->address, &asm, attempt, constsize, error );

			if ( newlen < 1 )
			{
				// First try to compile it failed, so the opcode is bad
				if ( !attempt && !constsize )
				{
					sprintf_s( errbuff,buffsize, "Error compiling opcode %s: %s", op->assembly, &error[0] );
					return 1;
				}

				// Out of combinations, get the best one we got so far
				if ( !constsize )
				{
					goto finished;
				}

			} 
			else if ( newlen < oldlen )
			{
				// Got a better result here!
				final = asm;
				oldlen = newlen;
			}
		}

		attempt++;
	}

finished:

	// Fill in the opcode with the best result we got so far
	strncpy( op->code, final.code, 16 );
	op->size = final.length;
	return 0;
}

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
// --------------------------------------------------
// Technical information:

// The compiler will do the following to compile the code:
//
// - Scan the source code and split it up in lines
// - Remove any bogus content (commends and blank lines)
// - Create opcodes
// - Add all labels to the symbols and mark all DB opcodes
// - Mark opcodes containing symbols and labels
// - First pass compiling (assembles each line, all labels defined as baseAddress)
// - Update label locations
// - Second pass compiling (assembles each line with labels)
// - Everytime an opcode changes size, update all following opcodes and restart the pass
// - Compute the total size of the code
// - Put the code in the buffer (assuming it fits!)
// - Clean up
//
// In total, there are 3 passes:
// 1. Analysis
// 2. Primary compiler run
// 3. Final compiler run ( gets repeated until everything is properly compiled ).
// ==================================================

int ASMC_Compile( asmc_t *asmc, int baseAddress, char* buffer, int buffersize )
{
	asmc_object_t *asmcobj;
	asmc_opcode_t *op;
	char *s;
	int i;
	int addr;
	int ret;
	int oldsz;
	int totalsz;
	char *ptr;

	if ( !asmc || !buffer )
	{
		return ASMC_ERROR_BADARGS;
	}

	asmcobj = ( asmc_object_t * ) asmc;

	if ( !asmcobj->code )
	{
		asmcobj->errorline[0] = 0;
		sprintf_s( asmcobj->errormsg, 256, "No source code defined" );
		return ASMC_ERROR_NOSOURCE;
	}

	ASMC_Cleanup( asmcobj, 1 );								// Get ready to compile
	asmcobj->codebuff = ASMC_CopyString( asmcobj->code );	// Copy code and begin parsing

	// Begin scanning the code
	{
		// First determine the amount of lines we have
		int lines = 1;
		s = asmcobj->codebuff;

		while ( s = strstr( s, "\n" ))
		{
			lines++;
			s++;
		}

		// Alright, time to alloc lines
		asmcobj->lines = malloc( 4 * lines );
		asmcobj->opcodes = malloc( 4 * lines );
		asmcobj->linecount = lines;
	}

	// Now we parse the lines
	{
		int line = 1;
		s = asmcobj->codebuff;
		asmcobj->lines[0] = asmcobj->codebuff;

		while ( s = strstr( s, "\n" ))
		{
			if ( *( s + 1 ) == '\r' )
			{
				asmcobj->lines[line++] = s+2;
			} 
			else 
			{
				asmcobj->lines[line++] = s+1;
			}

			*s++ = 0;
		}
	}

	// Create opcodes
	{
		asmcobj->opcodecount = 0;
		for ( i = 0; i<asmcobj->linecount; i++ )
		{
			s = strstr( asmcobj->lines[i], ";" );

			if ( s )
			{
				*s = 0; // Remove comment
			}

			s = ASMC_Trim(( char * )asmcobj->lines[i] );

			if ( *s == 0 )
			{
				continue;	// Blank line or only comments
			}

			op = malloc( sizeof( asmc_opcode_t ));
			memset( op, 0, sizeof( asmc_opcode_t ));

			op->line = s;
			op->linenr = i;

			// strncpy(op->assembly,s,256);
			asmcobj->opcodes[asmcobj->opcodecount++] = op;
		}
	}

	// ==================================================
	//
	//		 COMPILER STAGE 1 : Analysis
	//
	// ==================================================

	// ==================================================
	//
	//		COMPILER STAGE 1 - PHASE 1: Parse labels and DB's
	//
	// ==================================================

	// Opcode analysis
	for ( i = 0; i < asmcobj->opcodecount; i++ )
	{
		op = asmcobj->opcodes[i];

		// First, see if this is a label
		s = strstr(op->line, " ");

		if ( !s )
		{
			// Its a label
			if ( op->line[strlen(op->line)-1] == ':' )
			{
				op->type = ASMC_TYPE_LABEL;
				strncpy( op->assembly, op->line, 256 ); // Asm of a label = the label minus the colon (for easy symbol lookup)
				op->assembly[strlen( op->assembly ) - 1] = 0; // Remove the colon
				op->address = baseAddress;
				op->size = 0;

				if ( ASMC_GetSymbol(asmc, op->assembly ))
				{
					strncpy( asmcobj->errorline, op->line, 256 );
					sprintf_s( asmcobj->errormsg, 256, "Symbol clash: %s already defined", op->assembly );
					ASMC_Cleanup( asmcobj, 1 );
					return ASMC_ERROR_SYMBOLCLASH;
				}

				ASMC_DefineLabel( asmc, op->assembly, ASMC_ToHex( op->address ));
			}
		}

		if ( !strnicmp( op->line, "db ", 3 ))
		{
			if ( !ASMC_ParseDB(op, asmcobj->errormsg, 256 ))
			{
				strncpy( asmcobj->errorline, op->line, 256);
				ASMC_Cleanup(asmcobj, 1);
				return ASMC_ERROR_COMPILEERR;
			}
		}
	}
	
	// ==================================================
	//
	//		COMPILER STAGE 1 - PHASE 2: Mark jumps and symbol/label references
	//
	// ==================================================

	// Run through em again, this time however, only check the usage of symbols (since all labels are now processed)
	for ( i = 0; i < asmcobj->opcodecount; i++ )
	{
		op = asmcobj->opcodes[i];

		if ( !strnicmp( op->line,"call ", 5 ))
		{
			op->isJump = 1;
		}

		if ( op->line[0] == 'j' || op->line[0] == 'J' )
		{
			op->isJump = 1;
		}

		if ( strstr(op->line, "$" ))
		{
			op->hasSymbols = 1;
			ret = ASMC_ParseSymbols( asmc, op, asmcobj->errormsg, 256 );

			if ( ret )
			{
				strncpy( asmcobj->errorline, op->line, 256 );
				ASMC_Cleanup( asmcobj, 1 );
				return ret == 1 ? ASMC_ERROR_COMPILEERR : ASMC_ERROR_BADSYMBOLREF;
			}
		}
	}

	// ==================================================
	//
	//		COMPILER STAGE 2 - First compiler pass
	//
	// ==================================================

	addr = baseAddress;
	totalsz = 0;

	for ( i = 0; i < asmcobj->opcodecount; i++ )
	{
		op = asmcobj->opcodes[i];
		op->address = addr;

		// Label, redefine address
		if ( op->type == ASMC_TYPE_LABEL)
		{
			ASMC_DefineLabel(asmc, op->assembly, ASMC_ToHex( op->address ));
			continue;
		}
		// Data
		else if ( op->type == ASMC_TYPE_DATA )
		{
			addr += op->size;
			totalsz += op->size;
			continue;
		}
		// Code
		else 
		{
			if ( op->hasSymbols )
			{
				ret = ASMC_ParseSymbols( asmc, op, asmcobj->errormsg, 256 );
				if ( ret )
				{
					// Should never happen, but just in case
					strncpy( asmcobj->errorline, op->line, 256 );
					ASMC_Cleanup( asmcobj, 1 );
					return ret == 1 ? ASMC_ERROR_COMPILEERR : ASMC_ERROR_BADSYMBOLREF;
				}
			} 
			else
			{
				strncpy(op->assembly, op->line,255);
			}

			// Compile it
			if ( ASMC_CompileOpcode( op, asmcobj->errormsg, 256 ))
			{
				// Failed to compile
				strncpy( asmcobj->errorline, op->line, 256 );
				ASMC_Cleanup( asmcobj, 1 );
				return ASMC_ERROR_COMPILEERR;
			}

			addr += op->size;
			totalsz += op->size;
		}
	}


	// ==================================================
	//
	//		COMPILER STAGE 3 - Second compiler pass
	//
	// ==================================================

	// This time around, we got all labels 'defined'
	// So only recompile jumps/calls and symbol references
	// In case an opcode changes size, shift down all following instructions
	// and restart the pass

	for ( i = 0; i < asmcobj->opcodecount; i++ )
	{
		op = asmcobj->opcodes[i];

		// Recompile this one
		if ( op->type == ASMC_TYPE_CODE && ( op->isJump || op->hasSymbols ))
		{
			if ( op->hasSymbols )
			{
				ret = ASMC_ParseSymbols( asmc, op, asmcobj->errormsg, 256 );
				if ( ret )
				{
					// Should never happen, but just in case
					strncpy(asmcobj->errorline, op->line, 256);
					ASMC_Cleanup(asmcobj, 1);
					return ret == 1 ? ASMC_ERROR_COMPILEERR : ASMC_ERROR_BADSYMBOLREF;
				}
			}
			else
			{
				strncpy( op->assembly, op->line, 255 );
			}

			oldsz = op->size;

			// Compile it
			if ( ASMC_CompileOpcode(op, asmcobj->errormsg, 256 ))
			{
				// Failed to compile
				strncpy(asmcobj->errorline, op->line, 256);
				ASMC_Cleanup(asmcobj, 1);
				return ASMC_ERROR_COMPILEERR;
			}

			if ( oldsz != op->size )
			{
				totalsz -= oldsz;
				totalsz += op->size;

				// Alright, the opcode changed size
				// Time for a shift :P

				ret = op->size - oldsz; // Get offset

				for ( i = i + 1; i < asmcobj->opcodecount; i++ )
				{
					op = asmcobj->opcodes[i];
					op->address += ret;

					if ( op->type == ASMC_TYPE_LABEL )
					{
						ASMC_DefineLabel(asmc, op->assembly, ASMC_ToHex(op->address));
					}
				}

				i = -1; // Restart the pass
				continue;
			}
		}
	}

	// Last pass finished! we're done
	// Check the final size and copy the code into our buffer
	if ( buffersize < totalsz )
	{
		// >.<!
		asmcobj->errorline[0] = 0;
		sprintf_s( asmcobj->errormsg, 256, "Insufficient buffer space: %i bytes required, %i bytes available", totalsz, buffersize );
		ASMC_Cleanup( asmcobj, 1 );
		return ASMC_ERROR_NOSPACE;
	}

	asmcobj->size = totalsz;

	// Alright! Lets start copying the code into the buffer
	ptr = buffer;
	for ( i = 0; i < asmcobj->opcodecount; i++ )
	{
		op = asmcobj->opcodes[i];

		if ( op->size > 0 )
		{
			if ( op->type == ASMC_TYPE_DATA )
			{
				memcpy( ptr, op->data, op->size );
			}
			else 
			{
				memcpy( ptr, op->code, op->size );
			}

			ptr += op->size;
		}
	}

	// Successfully compiled ^_^
	ASMC_Cleanup( asmcobj, 0 );
	return ASMC_ERROR_OK;
}