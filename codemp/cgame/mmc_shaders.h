#ifndef MMC_SHADERS_H
#define MMC_SHADERS_H
#include "../qcommon/qfiles.h"

typedef struct MD3Info_Shader{
	char *name;						// Name of the shader.
//	int shaderId;					// The shader index, real one used by engine. //Edit: Which obviously isn't used by server
}MD3Info_Shader;

typedef struct MD3Info {
	char	name[MAX_QPATH];			// Name of the MD3
	int		numShaders;					// How many shaders it has
	int		shaders[MD3_MAX_SHADERS];	// The ID to look the shader up in common shader pool.
	// MD3_MAX_SHADERS is per surface, --> MD3_MAX_SURFACES
}MD3Info;

#define MM_MAX_SHADERS 1024
#define MM_MAX_MD3INFO 512+256

typedef struct ShaderRemaps{
	//char	from[MAX_QPATH];
	//char	to[MAX_QPATH];
	int from;
	int to;
	struct ShaderRemaps *next, *prev;
}ShaderRemaps;

// A player can have multiple different remap settings. 
// Basically one for every different scenario.
typedef struct RemapSettings{
	char	name[64];				// Unique identifier for the setting.
	char	objectName[MAX_QPATH];	// Name of object that it remaps
	int		numRemaps;
	ShaderRemaps *remap;
	struct RemapSettings *next, *prev;
}RemapSettings;

// This structure keeps track of each players remaps
typedef struct PlayerShaderRemaps{
	int numRemapSettings;
	RemapSettings *remapSettings;
}PlayerShaderRemaps;

typedef struct ShaderRemap{
	int group;
	int objectID;
	int ownerNum;
}ShaderRemap;

typedef struct RemapClientData{
	qboolean active;
	int clientNum;
	int renderObjectID;
	int group;
}RemapClientData;

void ParseUpdateRemap();
void ParseClearRemap();
void MMC_ClientVersion();
void MMC_EnableFeature();

void MM_ClearRemapFrame();
int R_FindShader(char *name);
void MMC_RemoveShaderRemap(int clientNum, char *b_objectName, int from, char *groupName);
ShaderRemaps *C_FindRemap(RemapSettings *group, int remapTo, int *remapCounter, ShaderRemaps **prev);
RemapSettings *C_FindGroup(int clientNum, char *objectName, char *groupName, int *groupCounter, RemapSettings **prev);
void MM_RemovePatch_f();

void MMC_AddShaderRemap(int clientNum, char *b_objectName, int from, int to, char *groupName);
void MMC_ChangeShaderRemap(int clientNum, char *b_objectName, int from, int to, char *groupName);

extern qboolean mm_EnableRemaps;
#endif