#ifndef MMS_SHADERS_H
#define MMS_SHADERS_H
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

typedef struct ShaderRemaps{
	char	from[MAX_QPATH];
	char	to[MAX_QPATH];
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

typedef struct mm_clientVersion{
	qboolean active;
	qboolean hasClient;
	int version;
	int features;
	int auth;
	// Some message stuff, i'll store it here:
	int messageTime;
	int numMessages;
}mm_clientVersion;


#define MM_MAX_SHADERS 1024
#define MM_MAX_MD3INFO 512+256

MD3Info* FindMD3Info(gentity_t *target);
void MM_AddShaderRemap(gentity_t *owner, gentity_t *target, char args[3][1024]);
void MMS_JoinUpdateAllRemaps(int clientNum);
void MMS_ClearPlayerRemaps(int playerID);
void MMS_JoinUpdateRemaps(int clientNum, int playerID);

RemapSettings *FindGroup(int clientNum, gentity_t *target, char *groupName, int *groupCounter, RemapSettings **prev);
ShaderRemaps *FindRemap(RemapSettings *group, char *remapName, int *remapCounter, ShaderRemaps **prev);
void MMS_RemoveShaderRemap(gentity_t *owner, gentity_t *target, char args[3][1024]);
void IssuePlayerShaderUpdate(gentity_t *owner, int clientTarget, int groupCounter, int remapCounter, int type, char *groupName);

void Cmd_mShaderInfo_f( gentity_t *ent );
void Cmd_mSetShader_f( gentity_t *ent );
void Cmd_mShaderGroup_f( gentity_t *ent );
void InitAllPlayerRemaps();
void ClearAllPlayerRemaps();
void InitPlayerRemaps(int clientNum);
void ClearPlayerRemaps(int clientNum);

extern mm_clientVersion playerClients[MAX_CLIENTS];

#define MMS_VERSION 1
#define MMS_FEATURES 1

#endif