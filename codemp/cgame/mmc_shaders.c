#include "cg_local.h"
#include "mmc_shaders.h"

MD3Info_Shader	md3infoShaders[MM_MAX_SHADERS];
MD3Info			md3info[MM_MAX_MD3INFO];

int mmNumMD3InfoShaders = 0;
int mmNumMD3Info = 0;



PlayerShaderRemaps mmPlayerShaderRemaps[MAX_CLIENTS];
qboolean mm_EnableRemaps = qfalse;

void ParseUpdateRemap()
{
	char b_type[MAX_STRING_CHARS];
	char b_clientNum[MAX_STRING_CHARS];
	char b_group[MAX_STRING_CHARS];		// Optional
	char b_objectName[MAX_STRING_CHARS];
	char b_from[MAX_STRING_CHARS];
	char b_to[MAX_STRING_CHARS];
	int numArgs = trap_Argc();
	int type, clientNum;
	int from, to;
	qboolean hasGroup;

	// Check if we have a group name
	hasGroup = (numArgs == 7) ? qtrue : qfalse;

	// Read the arguments
	trap_Argv(1, b_type, sizeof(b_type));
	trap_Argv(2, b_clientNum, sizeof(b_clientNum));

	if(hasGroup)
	{
		trap_Argv(3, b_group, sizeof(b_group));
		trap_Argv(4, b_objectName, sizeof(b_objectName));
		trap_Argv(5, b_from, sizeof(b_from));
		trap_Argv(6, b_to, sizeof(b_to));
	}
	else
	{
		trap_Argv(3, b_objectName, sizeof(b_objectName));
		trap_Argv(4, b_from, sizeof(b_from));
		trap_Argv(5, b_to, sizeof(b_to));
	}

	// Convert numbers
	type = atoi(b_type);
	clientNum = atoi(b_clientNum);

	// Clientside is going to be working with the shader ID's
	from = R_FindShader(b_from);
	to = R_FindShader(b_to);


	if(type == 3)	// Add remap
	{
		if(hasGroup)
			MMC_AddShaderRemap(clientNum, b_objectName, from, to, b_group);
		else MMC_AddShaderRemap(clientNum, b_objectName, from, to, NULL);
	}
	else if(type == 4) // Remove remap
	{
		if(hasGroup)
			MMC_RemoveShaderRemap(clientNum, b_objectName, from, b_group);
		else MMC_RemoveShaderRemap(clientNum, b_objectName, from, NULL);
	}
	else if(type == 1) // Change remap
	{
		if(hasGroup)
			MMC_ChangeShaderRemap(clientNum, b_objectName, from, to, b_group);
		else MMC_ChangeShaderRemap(clientNum, b_objectName, from, to, NULL);
	}
	else
	{
		if(hasGroup)
			CG_Printf("Unknown remap update: ( %i %s %s %s %s )", type, b_group, b_objectName, b_from, b_to);
		else CG_Printf("Unknown remap update: ( %i %s %s %s )", type, b_objectName, b_from, b_to);
	}
}

void MMC_AddShaderRemap(int clientNum, char *b_objectName, int from, int to, char *groupName)
{
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = playerRemaps->remapSettings;
	RemapSettings *prev = NULL;

	while(group)
	{
		qboolean skip = qfalse;
		if(groupName)
		{
			if(Q_stricmp(group->name, groupName))
				skip = qtrue;
		}
		else
		{
			if(group->name[0] != 0)
				skip = qtrue;
		}


		if(!skip && !Q_stricmp(group->objectName, b_objectName))
			break;

		prev = group;
		group = group->next;
	}

	if(!group)
	{
		// Group doesn't exist.
		if(prev)
		{
			prev->next = (RemapSettings*)malloc(sizeof(RemapSettings));
			group = prev->next;
		}
		else 
		{
			playerRemaps->remapSettings = (RemapSettings*)malloc(sizeof(RemapSettings));
			group = playerRemaps->remapSettings;
		}

		// Initialize the new group
		if(groupName)
			Q_strncpyz(group->name, groupName, sizeof(group->name));
		else memset(group->name, 0, sizeof(group->name));
		Q_strncpyz(group->objectName, b_objectName, sizeof(group->objectName));
		group->next = group->prev = NULL;
		group->remap = NULL;
		group->numRemaps = 0;
		if(prev)
			group->prev = prev;

		// Increase count
		playerRemaps->numRemapSettings++;
	}

	// Now we have the correct group
	if(group)
	{
		ShaderRemaps *remap = group->remap;
		ShaderRemaps *prev = NULL;
		int done = 0;

		// Check if it has already been remapped. We'll have to change it.
		while(remap)
		{
			if(remap->from == from)
			{
				remap->to = to;
				done = 1;
				break;
			}
			prev = remap;
			remap = remap->next;
		}

		if(!done)
		{
			// It didn't already exist, so it's a new remap. Time to add it.
			if(prev)
			{
				prev->next = (ShaderRemaps*)malloc(sizeof(ShaderRemaps));
				remap = prev->next;
			}
			else 
			{
				group->remap = (ShaderRemaps*)malloc(sizeof(ShaderRemaps));
				remap = group->remap;
			}

			// Initialize the new group
			remap->from = from;
			remap->to = to;
			remap->next = remap->prev = NULL;
			if(prev)
				remap->prev = prev;

			// Increase count
			group->numRemaps++;
			done = 3;
		}
	}
}

extern qboolean renderRemapped;
extern RemapClientData remapClientData[1024];

void MM_ClearRemapFrame()
{
	renderRemapped = qfalse;
	if(mm_EnableRemaps)
		memset(remapClientData, 0, sizeof(remapClientData));
}

void MMC_RemoveShaderRemap(int clientNum, char *b_objectName, int from, char *groupName)
{
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = NULL, *prevGroup = NULL;
	int groupCounter = 0, remapCounter = 0;
	qboolean foundRemove = qfalse;

	group = C_FindGroup(clientNum, b_objectName, groupName, &groupCounter, &prevGroup);

	if(group)
	{
		ShaderRemaps *prev = NULL;
		ShaderRemaps *remap = C_FindRemap(group, from, &remapCounter, &prev);
		if(remap)
		{
			//Found it, remove it.
			if(prev)
			{
				prev->next = remap->next;
				if(remap->next)
					remap->next->prev = prev;
			}
			else
			{
				// First element
				if(remap->next)
					remap->next->prev = NULL;
				group->remap = remap->next;
			}

			group->numRemaps--;
			free(remap);

			// Group is empty too, remove that as well.
			if(!group->numRemaps)
			{
				int i;
				if(prevGroup)
				{
					prevGroup->next = group->next;
					if(group->next)
						group->next->prev = prevGroup;
				}
				else
				{
					// First element
					if(group->next)
						group->next->prev = NULL;
					playerRemaps->remapSettings = group->next;
				}

				//// Update the 
				//for(i = 0; i < 1024; i++)
				//{
				//	if(remapClientData[i].active && remapClientData[i].clientNum == clientNum)
				//		if(remapClientData[i].group == groupCounter)
				//			remapClientData[i].active = qfalse;
				//}

				playerRemaps->numRemapSettings--;
				free(group);
			}
		}
	}
}


RemapSettings *C_FindGroup(int clientNum, char *objectName, char *groupName, int *groupCounter, RemapSettings **prev)
{
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = playerRemaps->remapSettings;
	while(group)
	{
		qboolean skip = qfalse;
		if(groupName && groupName[0])
		{
			if(Q_stricmp(group->name, groupName))	
				skip = qtrue;
		}
		else
		{
			if(group->name[0] != 0)
				skip = qtrue;
		}

		if(!skip && !Q_stricmp(group->objectName, objectName))// If we found it, break!
			break;

		if(prev)
			*prev = group;
		group = group->next;
		if(groupCounter)
			(*groupCounter)++;
	}

	return group;
}

ShaderRemaps *C_FindRemap(RemapSettings *group, int remapTo, int *remapCounter, ShaderRemaps **prev)
{
	ShaderRemaps *remap = group->remap;
	while(remap)
	{
		if(remap->from == remapTo)// If we found it, break!
			break;

		if(prev)
			*prev = remap;
		remap = remap->next;
		if(remapCounter)
			(*remapCounter)++;
	}

	return remap;
}

void ParseClearRemap()
{
	char b_clientNum[MAX_STRING_CHARS];
	int clientNum;
	RemapSettings *settings;

	trap_Argv(1, b_clientNum, sizeof(b_clientNum));
	clientNum = atoi(b_clientNum);

	settings = mmPlayerShaderRemaps[clientNum].remapSettings;

	// Move to end
	if(settings)
	{
		while(settings->next)
			settings = settings->next;

		while(settings)
		{
			ShaderRemaps* remap = settings->remap;
			
			if(!remap)
			{
				// Move to end
				while(remap->next)
					remap = remap->next;
				
				while(remap)
				{
					remap = remap->prev;
					free(remap->next);
				}
			}
			settings = settings->prev;
			if(settings)
				free(settings->next);
		}

		mmPlayerShaderRemaps[clientNum].numRemapSettings = 0;
		mmPlayerShaderRemaps[clientNum].remapSettings = NULL;
	}
}

#define MMC_VERSION 1
#define MMC_FEATURES 1
void MMC_ClientVersion()
{
	//mmClientVersion
	int numArgs = trap_Argc();
	char b_auth[MAX_STRING_CHARS];
	int auth;

	if(numArgs != 2)
		return;

	trap_Argv(1, b_auth, sizeof(b_auth));
	auth = atoi(b_auth);
	trap_SendConsoleCommand(va("mmClientVersion %i %i %i", auth, MMC_VERSION, MMC_FEATURES));
}

void MMC_EnableFeature()
{
	//mmEnableFeature
	int numArgs = trap_Argc();
	char b_features[MAX_STRING_CHARS];
	int features;

	if(numArgs != 2)
		return;

	trap_Argv(1, b_features, sizeof(b_features));
	features = atoi(b_features);

	if(features & 1)
		mm_EnableRemaps = qtrue;
}

void MMC_ChangeShaderRemap(int clientNum, char *b_objectName, int from, int to, char *groupName)
{
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = NULL, *prevGroup = NULL;
	int groupCounter = 0, remapCounter = 0;
	qboolean foundRemove = qfalse;

	group = C_FindGroup(clientNum, b_objectName, groupName, &groupCounter, &prevGroup);

	if(group)
	{
		ShaderRemaps *prev = NULL;
		ShaderRemaps *remap = C_FindRemap(group, from, &remapCounter, &prev);
		remap->to = to;
	}
}



// #####################################################################
// #####################################################################
// #####################################################################
// #####################################################################


#include <windows.h>
#include "../disasm/disasm.h"
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

#include "jkg_asmdefines.h"
// 004A807B 028 8B 44 32 40          mov     eax, [edx+esi+40h]
#define SHADER_POS 0x4A807F
#define SHADER_RET 0x4A8086	
#define SHADER_ADDRESS_THING 0xFE742C

qboolean renderRemapped = qfalse;
int remappedObjectID = 0;
int remappedObjectID2 = 0;
int *currentRenderObject = (int*)0x00FE3794;


RemapClientData remapClientData[1024];
extern PlayerShaderRemaps mmPlayerShaderRemaps[MAX_CLIENTS];

static int MM_CheckShader(const int shaderIndex)
{
	if(!renderRemapped || !mm_EnableRemaps)
		return shaderIndex;

	//if(shaderIndex == remapShaderFrom)
	//{
	//	if(remappedObjectID == *currentRenderObject || remappedObjectID2 == *currentRenderObject)
	//	{
	//		if(remapShaderTo != 0)
	//			return remapShaderTo;

	//		return shaderIndex-1;
	//	}
	//}

	//remapClientNum = cent->currentState.trickedentindex2;
	//remapObjectID = *numRenderObjects;
	//remapGroup = cent->currentState.trickedentindex3;

	if(remapClientData[*currentRenderObject].active)// == remapObjectID)
	{
		int i;
		PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[remapClientData[*currentRenderObject].clientNum];//remapClientNum];
		RemapSettings *group = playerRemaps->remapSettings;
		ShaderRemaps *remap; 

		for(i = 0; i < remapClientData[*currentRenderObject].group; i++)//remapGroup;i++)
			group = group->next;

		remap = group->remap;
		for(i = 0; i < group->numRemaps; i++, remap = remap->next)
		{
			if(shaderIndex == remap->from)
				return remap->to;
		}
	}

	return shaderIndex;
}

int temp = 0;
void __declspec(naked) _Hook_ShaderPatch(void)
{ 
	__asm 
	{ 
		add		esp, 4
		//mov		temp, eax
		pushad
		push	eax
		call	MM_CheckShader
		mov		temp, eax
		add		esp, 4
		popad
		mov		eax, temp
		mov		ebp, SHADER_ADDRESS_THING[eax*4]
		mov		edx, SHADER_RET
		jmp		edx	
	} 
}

// int __fastcall RE_RegisterShaderLightMap(int a1, const char *a2, int *a3, int a4)
// push    offset dword_547B94
// push    offset dword_547B54
int *RegShaderVar2 = (int*)0x547B94;
int *RegShaderVar1 = (int*)0x547B54;

int (__fastcall *RE_RegisterShaderLightMap)(const char *a2, int *a3) = (int(__fastcall*)(const char *a2, int *a3))0x004B6260;
//void (*derp)(char *herp) = (void(*)(char *herp))0xDEADBEEF;

// int __usercall R_FindShader<eax>(int a1<eax>, int *a2<ecx>, char *a3, int a4)
void (*_FindShader)() = (void(*)())0x004B5F00;
int DefaultShader = 0;
int ShaderReturn = 0;
int R_FindShader(char *name)
{
	__asm
	{

		//mov     eax, offset RegShaderVar2
		//mov     ecx, offset RegShaderVar1
		mov		eax,RegShaderVar2 
		mov		ecx,RegShaderVar1 
		push	1
		push	name
		call	_FindShader
		//mov ShaderReturn, eax
		mov		ecx, eax
		mov		edx, [ecx+0x68]
		mov     DefaultShader, edx
		mov		edx, [ecx+0x54]
		mov		ShaderReturn, edx
		add		esp, 8
	}

	if(DefaultShader != 1)
		return ShaderReturn;
	return 0;
}

//static void *_Hook_ShaderPatch()
//{
//	//int temp;
//	__JKG_StartHook;
//	{
//		__asm2__( mov   ebp, SHADER_ADDRESS_THING[eax*4]);
//
//		//__asm2__( mov	eax, [edx+esi+40h]	);
//		//__asm2__( mov	ebx, FE742Ch		);
//		
//		//__asm1__( pushad					);
//		//__asm1__( pushfd					);
//		//__asm1__( push	eax					);
//		//__asm1__( call	MM_CheckShader		);
//		//__asm2__( mov	temp, eax			);
//		//__asm2__( add	esp, 4				);
//		//__asm1__( popfd						);
//		//__asm1__( popad						);
//		//__asm2__( mov	eax, temp			);
//		__asm2__( mov	edx, SHADER_RET		);
//		__asm1__( jmp	edx					);
//		//return
//	}
//	__JKG_EndHook;
//}

static PatchData_t *shaderFix;
void CG_TestPatch_f(void) {
	//remapShaderTo = trap_R_RegisterShader("models/map_objects/factory/glass_b");
	//remapShaderFrom = trap_R_RegisterShader("textures/factory/cat_floor.tga");
	//remapShaderFrom = R_FindShader("textures/factory/cat_floor_b.tga");
	//R_FindShader( name, lightmapIndex, qtrue );
	shaderFix = JKG_PlacePatch(PATCH_CALL, SHADER_POS, (unsigned int)_Hook_ShaderPatch);
//	JKG_RemovePatch(&shaderFix);
}

void MM_RemovePatch_f(void) {
	JKG_RemovePatch(&shaderFix);
}

// Todo:
// better handling for default shader


// Testing UI to CG data transfer
// [UItoCG] - Scooper
//typedef struct vm_s {
//    int			programStack;
//    int			(*systemCall)( int *parms );
//    char		name[MAX_QPATH];
//	void		*dllHandle;
//}vm_t;
//
//vm_t **uivm = (vm_t**)0x00B28224;
//
//void (*UItoCG_Bridge)() = NULL;
//void Init_UItoCG_Bridge()
//{
//	UItoCG_Bridge = (void(*)())GetProcAddress((HMODULE)(*uivm)->dllHandle,"UItoCG_Bridge");
//
//	if(UItoCG_Bridge != NULL)
//		UItoCG_Bridge();
//}