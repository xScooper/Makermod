///////////////////////////////////////////////////////////////////
//
//  Engine alteration
//
//  All engine modifications that are to be present at run-time
//  go in here
//
//  By BobaFett
//
///////////////////////////////////////////////////////////////////
//  Windows and linux compatible

#include "../disasm/disasm.h"
#include "g_engine.h"
#undef INFINITE

#pragma warning(disable : 4211)

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
	#include <unistd.h>
	#include <string.h>
    #include <stdlib.h>
    typedef unsigned char byte;
	#define _stricmp strcasecmp
#endif

#ifdef _WIN32
// Windows defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x41A230;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FBE0;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x40FDB0;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x40F490;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x4435D0;
#else
// Linux defs here
static void (*NET_OutOfBandPrint)( int sock, netadr_t adr, const char *format, ... ) = (void (*)(int, netadr_t, const char*,...))0x807B744;
static void (*Com_Printf2)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072CA4;
static void (*Com_DPrintf)( const char *fmt, ... ) = (void (*)(const char *, ...))0x8072ED4;
static const char *(*Cmd_Argv)( int arg ) = (const char *(*)(int))0x812C264;
static void (*SV_SendServerCommand)(client_t *cl, const char *fmt, ...) = (void(*)(client_t *cl, const char *fmt, ...))0x8056214;

#endif

void Com_sprintf (char *dest, int size, const char *fmt, ...);

typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;
/*
static short   BigShort (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}*/

static const char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	if (a.type == NA_LOOPBACK) {
		Com_sprintf (s, sizeof(s), "loopback");
	} else if (a.type == NA_BOT) {
		Com_sprintf (s, sizeof(s), "bot");
	} else if (a.type == NA_IP) {
		Com_sprintf (s, sizeof(s), "%i.%i.%i.%i:%hu",
			a.ip[0], a.ip[1], a.ip[2], a.ip[3], BigShort(a.port));
	} else {
		Com_sprintf (s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%hu",
		a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], 
		BigShort(a.port));
	}

	return s;
}

typedef struct {
	int addr;
	int size;
	char origbytes[24];
} PatchData_t;

typedef enum {
	PATCH_JUMP,
	PATCH_CALL,
} PatchType_e;

extern vmCvar_t		jkg_antifakeplayer;
extern vmCvar_t		jkg_allowbaseclients;

// ==================================================
// UnlockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address writable for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int UnlockMemory(int address, int size) {
	int ret;
	int dummy;
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dummy);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
		return (ret == 0);
	}
#endif
}

// ==================================================
// LockMemory (WIN32 & Linux compatible)
// --------------------------------------------------
// Makes the memory at address read-only for at least
// size bytes.
// Returns 1 if successful, returns 0 on failure.
// ==================================================
static int LockMemory(int address, int size) {
	int ret;
#ifdef _WIN32
	ret = VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READ, NULL);
	return (ret != 0);
#else
	// Linux is a bit more tricky
	int page1, page2;
	page1 = address & ~( getpagesize() - 1);
	page2 = (address+size) & ~( getpagesize() - 1);
	if( page1 == page2 ) {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
	} else {
		ret = mprotect((char *)page1, getpagesize(), PROT_READ | PROT_EXEC);
		if (ret) return 0;
		ret = mprotect((char *)page2, getpagesize(), PROT_READ | PROT_EXEC);
		return (ret == 0);
	}
#endif
}

// ==================================================
// JKG_PlacePatch (WIN32 & Linux compatible)
// --------------------------------------------------
// Patches the code at address to make it go towards
// destination.
// The instruction used is either JMP or CALL, 
// depending on the type specified.
//
// Before the code is modified, the code page is
// unlocked. If you wish to stop it from being locked
// again, specify 1 for nolock
//
// This function returns a malloced PatchData_t.
// To remove the patch, call JKG_RemovePatch. This
// will also free the PatchData_t.
// ==================================================

static PatchData_t *JKG_PlacePatch( int type, unsigned int address, unsigned int destination ) {
	PatchData_t *patch = malloc(sizeof(PatchData_t));
	t_disasm dasm;
	int addr = address;
	int sz = 0;
	int opsz;
	// Disassemble the code and determine the size of the code we have to replace
	while (sz < 5) {
		opsz = Disasm((char*)addr, 16, addr, &dasm, DISASM_CODE);
		if (opsz == 0) {
			return NULL;	// Should never happen
		}
		sz += opsz;
		addr += opsz;
	}
	if (sz == 0 || sz > 24) {
		// This really shouldnt ever happen, in the worst case scenario,
		// the block is 20 bytes (4 + 16), so if we hit 24, something went wrong
		return NULL;
	}
	patch->addr = address;
	patch->size = sz;
	memcpy(patch->origbytes, (const void *)address, sz);
	UnlockMemory(address, sz); // Make the memory writable
	*(unsigned char *)address = type == PATCH_JUMP ? 0xE9 : 0xE8;
	*(unsigned int *)(address+1) = destination - (address + 5);
	memset((void *)(address+5),0x90,sz-5);	// Nop the rest
	LockMemory(address, sz);
	return patch;
}

static void JKG_RemovePatch(PatchData_t **patch) {
	if (!*patch)
		return;
	UnlockMemory((*patch)->addr, (*patch)->size);
	memcpy((void *)(*patch)->addr, (*patch)->origbytes, (*patch)->size);
	LockMemory((*patch)->addr, (*patch)->size);
	*patch = 0;
}

// ==================================================
// Shell System for Hooks (By BobaFett & Deathspike)
// --------------------------------------------------
// This system, that I (BobaFett) call:
// The Shell System, allows for hooks to be created
// on both windows and linux with exactly the same code
// (OS specific asm changes not included)
//
// The system works as follows:
// Since compilers have the tendancy to add prologue
// and epilogue code to functions, we put our asm
// inside a safe 'shell'
// The shell is defined by:
//
// void *MyHook()
// {
//		__JKG_StartHook;			<-- Shell
//		{
//			// Hook code here		<-- Contents of shell
//		}
//		__JKG_EndHook;				<-- Shell
// }
//
// This code should be placed in a function returning
// a void *, as shown above.
// When called, it will return the pointer to the
// shell's contents, which can then be used to place
// hooks (ie. jumps).
//
// Note that the shell's contents (the hook in question)
// are not executed!
//
// For the actual asm, 3 defines are available:
// __asm1__ for zero/single operand opcodes	(push 10	)
// __asm2__ for dual operand opcodes		(mov eax, 1	)
// __asmL__ for labels						(mylabel:	)
//
// To compile this code on linux, you require the
// following in the gcc command line:
//  -masm=intel
// 
// NOTE: The hook's execution flow must NEVER get to
//       the shell layer! Always ensure the code is
//       ended with a jump or a return!
//
// ==================================================

#include "jkg_asmdefines.h"

// =================================================
// Hook 2:
// Custom anti-q3infoboom patch
// -------------------------------------------------
// Because Luigi Auriemma's 'fix' has side-effects
// such as userinfo being cut off from the connect
// packet. JKG implements a custom protection
// against this issue.
//
// This will also undo the fix from Luigi
//
// The hook is placed in SV_ConnectionlessPacket
// The patch is inside MSG_ReadStringLine
// =================================================
//
#ifdef __linux__
// Define linux symbols
#define _IBFIX_MSGPATCH 0x807803D
#define _IBFIX_PATCHPOS 0x8056E23
#define _IBFIX_QSTRICMP 0x807F434
#else
// Define windows symbols
#define _IBFIX_MSGPATCH 0x418B2C
#define _IBFIX_PATCHPOS 0x443F7F
#define _IBFIX_QSTRICMP 0x41B8A0
#endif

static PatchData_t *pIBFix;

void JKG_CheckConnectionlessPacket(const char *cmd);
static void *_Hook_InfoBoomFix()
{
	__JKG_StartHook;
	{
		__asm1__(	pushad									);	// Secure registers
		__asm1__(	push	ebx								);	// Secure registers
		__asm1__(	call	JKG_CheckConnectionlessPacket	);
		__asm2__(	add		esp, 4							);
		__asm1__(	popad									);
		__asm1__(	push	_IBFIX_QSTRICMP					);
		__asm1__(	ret										);
	}
	__JKG_EndHook;
}

static void JKG_CheckConnectionlessPacket(const char *cmd) {
	char *s;
	if (!_stricmp(cmd,"getstatus") || !_stricmp(cmd,"getinfo")) {
		// We got a risky function here, get arg 1 and do a cutoff if needed
		s = (char *)Cmd_Argv(1);
		if (strlen(s) > 32) {
			// POSSIBLE TODO: Add a check for malicious use and take action (ie. ban)
			s[32] = 0;	// 32 chars should be more than enough for the challenge number
		}
	} else if (!_stricmp(cmd,"connect")) {
		s = (char *)Cmd_Argv(1);
		if (strlen(s) > 980) {
			s[980] = 0;
		}

	}
}

// =================================================
// Hook 7:
// Download exploit fix
// -------------------------------------------------
// This hook here fixes the download exploit
// If a download is requested while ingame, it
// gets denied using a nice lil print, otherwise
// the download is redirected to a placeholder file
// that'll be sent instead.
//
// Hook located in SV_BeginDownload_f
// =================================================
//

#ifdef __linux__
// Define linux symbols
#define _DHFIX_PATCHPOS			0x804D723
#define _DHFIX_RETPOS1			0x804D544
#define _DHFIX_RETPOS2			0x804D751
#define _DHFIX_CLREG			ebx
#define _DHFIX_SVSCLIENTSPOS	0x83121EC


#else
// Define windows symbols
#define _DHFIX_PATCHPOS			0x43B3C7
#define _DHFIX_RETPOS1			0x43B2E0
#define _DHFIX_RETPOS2			0x43B3E8
#define _DHFIX_CLREG			esi
#define _DHFIX_SVSCLIENTSPOS	0x606224

#endif

static PatchData_t *pDHFIX;
int JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename);

static void *_Hook_DownloadHackFix()
{
	__JKG_StartHook;
	{
		__asm1__(	pushad									);	// Secure registers
		__asm2__(	mov		eax, _DHFIX_CLREG				);	// Get location of client_t
		__asm2__(	sub		eax, DS:[_DHFIX_SVSCLIENTSPOS]	);  // Work out the clientnum
		__asm2__(	xor		edx, edx						);  // by doing cl - svs.clients
		__asm2__(	mov		ecx, 0x51478					);  //
		__asm1__(	div		ecx								);  // Do division to get clientNum
		__asm1__(	push	0								);  // Push filename (placeholder)
		__asm1__(	push	_DHFIX_CLREG					);  // Push client_t *cl
		__asm1__(	push	eax								);  // Push clientNum
		__asm1__(	push	1								);  // Push 1 (for Cmd_Argv)
		__asm1__(	call	Cmd_Argv						);  // Call Cmd_Argv to get filename
		__asm2__(	add		esp, 4							);  // Clean up stack
		__asm2__(	mov		[esp+8], eax					);  // Replace the 0 we pushed
		__asm1__(	call	JKG_CheckDownloadRequest		);  // Call JKG_CheckDownloadRequest
		__asm2__(	add		esp, 0xC						);  // Clean up stack
		__asm2__(	test	eax, eax						);  // Check return value
		__asm1__(	popad									);  // Restore registers
		__asm1__(	je		bail							);  // If return value = 0, goto bail
		__asm1__(	push	_DHFIX_RETPOS1					);  // Push-ret call forward to
		__asm1__(	ret										);  // SV_CloseDownload
		__asmL__( bail:										);	// 
		__asm2__(	add		esp, 4							);  // Remove return address (we redirected a call)
		__asm1__(	push	_DHFIX_RETPOS2					);  // Push-ret jump to
		__asm1__(	ret										);  // the end of SV_BeginDownload_f
	}
	__JKG_EndHook;
}

static int JKG_CheckDownloadRequest(int clientNum, client_t *cl, const char *filename) {
	int illegal = 0;
	if (!filename) {
		illegal = 1;
	} else if (strlen(filename) < 4) {
		illegal = 1;
	} else if (Q_stricmpn(filename + strlen(filename) - 4, ".pk3", 4)) {
		illegal = 1;
	} else if (strstr(filename, "..")) {
		illegal = 1;
	}
	if (cl->state == CS_ACTIVE) {
		// These are 100% guaranteed to be fake
		if (illegal) {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Illegal download request detected\n\"", filename);
			return 0;
		} else {
			SV_SendServerCommand(cl, "print \"Download request for %s rejected: Download requested while in-game\n\"", filename);
			return 0;
		}
	} else {
		if (illegal) {
			// Get a substitute file and send that instead
			// TODO: use a cvar for this
			Q_strncpyz(cl->downloadName, "baddownload.txt", 64);
			return 0;
		}
		// Legal download (or substituted one ;P)
		return 1;
	}
}

void JKG_PatchEngine() {
	Com_Printf(" ------- Installing Engine Patches -------- \n");
	
	pIBFix = JKG_PlacePatch(PATCH_CALL, _IBFIX_PATCHPOS, (unsigned int)_Hook_InfoBoomFix()); // We'll be overwriting a call here
    if (!pIBFix) {
		Com_Printf("Warning: Failed to place hook 2: Q3infoboom fix\n");
    }
	///////////////////////////////
	// Patch 2: Revert the patch of Luigi (in case its patched)
	///////////////////////////////
	UnlockMemory(_IBFIX_MSGPATCH,1);
	*(unsigned int *)_IBFIX_MSGPATCH = (unsigned int)0x3FF;
	LockMemory(_IBFIX_MSGPATCH,1);

	///////////////////////////////
	// Hook 7: Download Hack Fix
	///////////////////////////////
	pDHFIX = JKG_PlacePatch(PATCH_CALL, _DHFIX_PATCHPOS, (unsigned int)_Hook_DownloadHackFix()); // We'll be overwriting a call here
    if (!pDHFIX) {
		Com_Printf("Warning: Failed to place hook 7: Download Hack Fix\n");
    }
	
	Com_Printf("Finished\n");
}

void JKG_UnpatchEngine() {
	JKG_RemovePatch(&pIBFix);
	JKG_RemovePatch(&pDHFIX);
}