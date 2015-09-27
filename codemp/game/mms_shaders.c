#include "g_local.h"
#include "mms_shader.h"

MD3Info_Shader	md3infoShaders[MM_MAX_SHADERS];
MD3Info			md3info[MM_MAX_MD3INFO];

int mmNumMD3InfoShaders = 0;
int mmNumMD3Info = 0;

PlayerShaderRemaps mmPlayerShaderRemaps[MAX_CLIENTS];
mm_clientVersion playerClients[MAX_CLIENTS];

int MM_AddShaderInfo(md3Shader_t *shader)
{
	int i;

	for(i = 0; i < mmNumMD3InfoShaders;i++)
	{
		//if(md3infoShaders[i].shaderId == shader->shaderIndex)		// Silly me, can't use this serverside.
		//	return i;
		if(!Q_stricmp(md3infoShaders[i].name, shader->name))
			return i;
	}

	// We didn't find it, so let's add it.
	md3infoShaders[mmNumMD3InfoShaders].name = malloc(strlen(shader->name)+1);
	memcpy(md3infoShaders[mmNumMD3InfoShaders].name, shader->name, strlen(shader->name)+1);
	//md3infoShaders[mmNumMD3InfoShaders].shaderId = shader->shaderIndex;

	return mmNumMD3InfoShaders++;
}

gentity_t* SelectedEnt(gentity_t *ent);
void Cmd_mShaderInfo_f( gentity_t *ent )
{
	int numArgs = trap_Argc();
	gentity_t *target;
	MD3Info *info = NULL;
	int i;

	//if(numArgs == 2)
	{
		target = SelectedEnt(ent);
		if(!target)
		{
			MM_SendMessage( ent-g_entities, va("print \"No selected entity.\n\""));
			return;
		}
	}

	info = FindMD3Info(target);

	if(!info)
	{
		MM_SendMessage( ent-g_entities, va("print \"Selected entity is not a model.\n\""));
		return;
	}

	MM_SendMessage( ent-g_entities, va("print \"Shader info for model: ^3%s^7, it has ^3%i ^7shaders.\n\"", info->name, info->numShaders));
	//MM_SendMessage( ent-g_entities, va("print \"It has ^3%i ^7shaders.\n\"", info->numShaders));

	for(i = 0; i < info->numShaders;i++)
		MM_SendMessage( ent-g_entities, va("print \"#%i: ^3%s\n\"",i+1, md3infoShaders[info->shaders[i]].name));
}

void Cmd_mShaderGroup_f( gentity_t *ent )
{
	int numArgs = trap_Argc();
	char arg[1024] = { 0 };
	gentity_t *target;
	RemapSettings *group;
	int num, groupCounter = 0;

	if(numArgs == 2)
	{
		target = SelectedEnt(ent);
		if(!target)
			return;
	}
	else
	{
		MM_SendMessage( ent-g_entities, va("print \"Command usage: mShaderGroup <group name/0>, if you set it to 0 it will choose the default group.\n\""));
		return;
	}

	trap_Argv( 1, arg, sizeof(arg) );
	if(isNumber(arg,&num))
		group = FindGroup(ent-g_entities, target, NULL, &groupCounter, NULL);
	else group = FindGroup(ent-g_entities, target, arg, &groupCounter, NULL);

	if(!group)
	{
		MM_SendMessage( ent-g_entities, va("print \"This group hasn't been created yet.\n\""));
		return; 
	}

	target->s.trickedentindex = 1;
	target->s.trickedentindex2 = ent - g_entities;
	target->s.trickedentindex3 = groupCounter;


}

void Cmd_mSetShader_f( gentity_t *ent )
{
	int numArgs = trap_Argc();
	char args[3][1024] = { 0 };
	int number;
	gentity_t *target;
	MD3Info *info = NULL;
	int i,j;
	qboolean validFrom = qfalse;

	if(numArgs == 3 || numArgs == 4)
	{
		target = SelectedEnt(ent);
		if(!target)
			return;
	}
	else
	{
		MM_SendMessage( ent-g_entities, va("print \"Command usage: mShader <shader name/id> <shader name/id> <group>(optional), where id can be found with mShaderInfo.\nFirst argument has to be a valid name or id from the model. Group is a name identifier for different remap groups.\n\""));
		return;
	}

	// Find the MD3Info
	info = FindMD3Info(target);
	if(!info)
	{
		MM_SendMessage( ent-g_entities, va("print \"Selected entity is not a model.\n\""));
		return;
	}

	// Fetch arguments
	trap_Argv( 1, args[0], sizeof(args[0]) );
	trap_Argv( 2, args[1], sizeof(args[1]) );

	// Check if any of the arguments are numbers
	for(i = 0; i < 2; i++)
	{
		if(isNumber(args[i], &number))
		{
			number--;
			if(number >= info->numShaders || number < 0)
			{
				MM_SendMessage( ent-g_entities, va("print \"Argument %i, is not a valid ID.\n\"", i+1));
				return;
			}
			// Let's work with the names from here on
			Q_strncpyz(args[i], md3infoShaders[info->shaders[number]].name, sizeof(args[0]));
			if(i==0)
				validFrom = qtrue;
		}
		else if(i == 0) // Only check the FROM texture/shader
		{
			// If it's not a number let's match the texture/shader name to the one on the model.
			for(j = 0; j < info->numShaders; j++)
			{
				if(!Q_stricmp(md3infoShaders[info->shaders[j]].name,args[0]))
				{
					// Found it, so it's valid.
					validFrom = qtrue;
					break;
				}
			}
		}
	}

	if(!validFrom)
	{
		MM_SendMessage( ent-g_entities, va("print \"The chosen shader/texture: %s, is not a valid selection for this model.\n\"", args[0]));
		return;
	}

	if(numArgs == 4)
		trap_Argv( 3, args[2], sizeof(args[2]) );

	// Check if they're identical, then we're just gonna remove the remap.
	if(!Q_stricmp(args[0],args[1]))
	{
		// Remove it.
		MMS_RemoveShaderRemap(ent, target, args);
		return;
	}

	// Update the remap records for the player.
	MM_AddShaderRemap(ent, target, args);

	//target->s.trickedentindex = ent - g_entities;
	//target->s.trickedentindex2 = (target->s.trickedentindex2) ? 0 : 1;
}

MD3Info* FindMD3Info(gentity_t *target)
{
	// md3 info
	md3Header_t* header;
	md3Surface_t* surface;
	md3Shader_t* shader;
	char *data, *dataPtr;
	int i,j,k;
	fileHandle_t	f;
	qboolean found = qfalse;
	MD3Info *info = NULL;

	if(target->s.eType != ET_MOVER)
	{
		return NULL;
	}

	// See if it has already been cached
	for(i = 0; i < mmNumMD3Info; i++)
	{
		if(!Q_stricmp(md3info[i].name, target->model2))
		{
			found = qtrue;
			info = &md3info[i];
			break;
		}
	}

	if(!found)
	{
		int totalNumShaders = 0;
		j = trap_FS_FOpenFile( va( "%s", target->model2), &f, FS_READ);

		data = malloc(128000);//BG_Alloc( 128000 );

		dataPtr = data;

		trap_FS_Read( data, min(j, 127449), f ); 

		data[min(j, 127500)] = '\0';

		if ( j < sizeof( md3Header_t ) )
		{
			trap_FS_FCloseFile(f);
			free(data);
			return NULL;
		}

		Q_strncpyz(md3info[mmNumMD3Info].name, target->model2, sizeof(md3info[mmNumMD3Info].name));
		md3info[i].numShaders = 0;

		header = (md3Header_t*) data;
		surface = (md3Surface_t*) (data+header->ofsSurfaces);
		dataPtr = (char*)surface;

		
		for(k = 0; k < header->numSurfaces;k++)
		{
			int l;
			md3info[mmNumMD3Info].numShaders += surface->numShaders;
			shader = (md3Shader_t *) ( (byte *)surface + surface->ofsShaders );

			for(l = 0; l < surface->numShaders; l++, shader++)
				md3info[mmNumMD3Info].shaders[totalNumShaders++] = MM_AddShaderInfo(shader);

			// Move to next surface
			surface = (md3Surface_t*)(dataPtr+surface->ofsEnd);
			dataPtr = (char*)surface;
		}

		// Set the pointer, and increase the counter
		info = &md3info[mmNumMD3Info++];

		// Free data and file
		free(data);
		trap_FS_FCloseFile(f);
	}

	return info;
}

void MM_AddShaderRemap(gentity_t *owner, gentity_t *target, char args[3][1024])
{
	int clientNum = owner-g_entities;
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = NULL;
	int groupCounter = 0;
	int remapCounter = 0;

	// Check for group, find it or create it.
	if(args[2][0])
	{
		// There's a group, find out if it exists.
		RemapSettings *prev = NULL;

		//group = playerRemaps->remapSettings;
		group = FindGroup(owner-g_entities, target, args[2], &groupCounter, &prev);
		//while(group)
		//{
		//	if(!Q_stricmp(group->name, args[2]))	// If we found it, break!
		//		if(!Q_stricmp(group->objectName, target->model2))
		//			break;

		//	prev = group;
		//	group = group->next;
		//	groupCounter++;
		//}

		if(!group)
		{
			// We have a group name, but couldn't find the group. Time to create it.
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
			Q_strncpyz(group->name, args[2], sizeof(group->name));
			Q_strncpyz(group->objectName, target->model2, sizeof(group->objectName));
			group->next = group->prev = NULL;
			group->remap = NULL;
			group->numRemaps = 0;
			if(prev)
				group->prev = prev;

			// Increase count
			playerRemaps->numRemapSettings++;
		}
	}
	else
	{
		// Pick the default group then.
		RemapSettings *prev = NULL;

		group = playerRemaps->remapSettings;
		while(group)
		{
			if(group->name[0] == 0 && !Q_stricmp(group->objectName, target->model2))
				break;
			
			prev = group;
			group = group->next;
			groupCounter++;
		}

		if(!group)
		{
			// No default group, create one.
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
			//Q_strncpyz(group->name, args[2], sizeof(group->name));
			memset(group->name,0, sizeof(group->name));
			Q_strncpyz(group->objectName, target->model2, sizeof(group->objectName));
			group->next = group->prev = NULL;
			group->numRemaps = 0;
			group->remap = NULL;
			if(prev)
				group->prev = prev;

			// Increase count
			playerRemaps->numRemapSettings++;
		}
	}

	if(group)
	{
		//int i;
		ShaderRemaps *remap = group->remap;
		ShaderRemaps *prev = NULL;
		int done = 0;

		// Check if it has already been remapped. We'll have to change it.
		//for(i = 0; i < group->numRemaps; i++, remap = remap->next)
		while(remap)
		{
			if(!Q_stricmp(remap->from, args[0]))
			{
				if(Q_stricmp(remap->to, args[1]))
				{
					Q_strncpyz(remap->to, args[1], sizeof(remap->to));
					done = 1;
				}
				else done = 2;		// Remap already exists.
				break;
			}
			prev = remap;
			remap = remap->next;
			remapCounter++;
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
			Q_strncpyz(remap->from, args[0], sizeof(remap->from));
			Q_strncpyz(remap->to, args[1], sizeof(remap->to));
			remap->next = remap->prev = NULL;
			if(prev)
				remap->prev = prev;

			// Increase count
			group->numRemaps++;
			done = 3;
		}

		if(done != 0 && done != 2)
			IssuePlayerShaderUpdate(owner, -1, groupCounter, remapCounter, done, args[2]);
	}

	// Time to apply the actual settings to the entity itself.
	target->s.trickedentindex = 1;
	target->s.trickedentindex2 = owner - g_entities;
	target->s.trickedentindex3 = groupCounter;

	// trickedentindex, mark remapped
	// trickedentindex2, use for player ID
	// trickedentindex3, use for RemapSetting group
}

void InitPlayerRemaps(int clientNum)
{
	RemapSettings *settings;
	mmPlayerShaderRemaps[clientNum].numRemapSettings = 0;

	// Init a default group.
	settings = NULL;
	//settings = mmPlayerShaderRemaps[clientNum].remapSettings = (RemapSettings*)malloc(sizeof(RemapSettings));

	//memset(settings->name,0,sizeof(settings->name)); // the name for default group will just be completely blank
	//memset(settings->objectName,0,sizeof(settings->objectName));

	//settings->next = settings->prev = NULL;
	//settings->numRemaps = 0;
	//settings->remap = NULL;
	//mmPlayerShaderRemaps[clientNum].numRemapSettings = 1;
}

void InitAllPlayerRemaps()
{
	int i;
	for(i = 0; i < MAX_CLIENTS;i++)
		InitPlayerRemaps(i);
}

void ClearPlayerRemaps(int clientNum)
{
	RemapSettings *settings = mmPlayerShaderRemaps[clientNum].remapSettings;

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

		// Clear the client info. This should probably be moved later [FIXME]
		memset(&playerClients[clientNum], 0, sizeof(mm_clientVersion));

		// Issue a clear message to the other clients.
		MMS_ClearPlayerRemaps(clientNum);
	}
}

void ClearAllPlayerRemaps()
{
	int i;
	for(i = 0; i < MAX_CLIENTS;i++)
		ClearPlayerRemaps(i);
}
/*
	Different types of updates.
	updateRemap <type> <info>
	Types:
		//0 - New group added
		//1 - Group removed
		3 - New remap added
		1 - Remap changed
		4 - Remap removed
*/
void IssuePlayerShaderUpdate(gentity_t *owner, int clientTarget, int groupCounter, int remapCounter, int type, char *groupName)
{
	// Find the correct group and remap.
	int clientNum = owner-g_entities;
	PlayerShaderRemaps* playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *settings = playerRemaps->remapSettings;
	ShaderRemaps *remap;
	int i;

	if(groupCounter > playerRemaps->numRemapSettings)
		return;

	for(i = 0; i < groupCounter; i++)
		settings = settings->next;

	if(remapCounter > settings->numRemaps)
		return;

	remap = settings->remap;
	for(i = 0; i < remapCounter; i++)
		remap = remap->next;


	// Send update
	if(groupName[0])
	{
		//trap_SendServerCommand( owner-g_entities, va("print \"updateRemap %i %i %s %s %s %s\n\"", type, clientNum, settings->name, settings->objectName, remap->from, remap->to ));
		if(clientTarget != -1)
			trap_SendServerCommand( clientTarget, va("updateRemap %i %i %s %s %s %s", type, clientNum, settings->name, settings->objectName, remap->from, remap->to ));
		else
		{
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(playerClients[i].active && playerClients[i].hasClient && playerClients[i].version == 1)
					trap_SendServerCommand( i, va("updateRemap %i %i %s %s %s %s", type, clientNum, settings->name, settings->objectName, remap->from, remap->to ));
			}
		}
	}
	else
	{
		//trap_SendServerCommand( owner-g_entities, va("print \"updateRemap %i %i %s %s %s\n\"", type, clientNum, settings->objectName, remap->from, remap->to ));
		if(clientTarget != -1)
			trap_SendServerCommand( clientTarget, va("updateRemap %i %i %s %s %s", type, clientNum, settings->objectName, remap->from, remap->to ));
		else
		{
			for(i = 0; i < MAX_CLIENTS; i++)
			{
				if(playerClients[i].active && playerClients[i].hasClient && playerClients[i].version == 1)
					trap_SendServerCommand( clientTarget, va("updateRemap %i %i %s %s %s", type, clientNum, settings->objectName, remap->from, remap->to ));
			}
		}
	}
}

RemapSettings *FindGroup(int clientNum, gentity_t *target, char *groupName, int *groupCounter, RemapSettings **prev)
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

		if(!skip && !Q_stricmp(group->objectName, target->model2))// If we found it, break!
			break;

		if(prev)
			*prev = group;
		group = group->next;
		if(groupCounter)
			(*groupCounter)++;
	}

	return group;
}

void MMS_RemoveShaderRemap(gentity_t *owner, gentity_t *target, char args[3][1024])
{
	int clientNum = owner-g_entities;
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[clientNum];
	RemapSettings *group = NULL, *prevGroup = NULL;
	int groupCounter = 0, remapCounter = 0;
	qboolean foundRemove = qfalse;

	group = FindGroup(clientNum, target, args[2], &groupCounter, &prevGroup);

	if(group)
	{
		ShaderRemaps *prev = NULL;
		ShaderRemaps *remap = FindRemap(group, args[0], &remapCounter, &prev);
		if(remap)
		{
			//Found it, remove it.
			IssuePlayerShaderUpdate(owner, -1, groupCounter, remapCounter, 4, args[2]);
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

				playerRemaps->numRemapSettings--;
				free(group);

				// If a group is removed, reset all the entities that are using that group.
				for(i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
				{
					gentity_t *ent = &g_entities[i];
					if(ent->s.trickedentindex && ent->s.trickedentindex2 == clientNum)
					{
						if(ent->s.trickedentindex3 == groupCounter)
						{
							ent->s.trickedentindex = 0;
							ent->s.trickedentindex2 = 0;
							ent->s.trickedentindex3 = 0;
						}
						// Also have to update the groupCounter for all groups after the current one
						else if( ent->s.trickedentindex3 > groupCounter)
							ent->s.trickedentindex3--;
					}
				}
			}
		}
	}
}

ShaderRemaps *FindRemap(RemapSettings *group, char *remapName, int *remapCounter, ShaderRemaps **prev)
{
	ShaderRemaps *remap = group->remap;
	while(remap)
	{
		if(!Q_stricmp(remap->from, remapName))// If we found it, break!
			break;

		if(prev)
			*prev = remap;
		remap = remap->next;
		if(remapCounter)
			(*remapCounter)++;
	}

	return remap;
}

// Update joining player.
void MMS_JoinUpdateAllRemaps(int clientNum)
{
	int i;

	for(i = 0; i < MAX_CLIENTS;i++)
	{
		if(mmPlayerShaderRemaps[i].numRemapSettings)
		{
			// We need to send updates for this player.
			MMS_JoinUpdateRemaps(clientNum, i);
		}
	}
}

void MMS_JoinUpdateRemaps(int clientNum, int playerID)
{
	PlayerShaderRemaps *playerRemaps = &mmPlayerShaderRemaps[playerID];
	int i,j;
	RemapSettings *group = playerRemaps->remapSettings;

	for(i = 0; i < playerRemaps->numRemapSettings; i++)
	{
		for(j = 0; j < group->numRemaps;j++)
				IssuePlayerShaderUpdate(&g_entities[playerID], clientNum, i, j, 3, group->name);

		group = group->next;
	}
}

void MMS_ClearPlayerRemaps(int playerID)
{
	int i;
	//trap_SendServerCommand( -1, va("clearRemap %i", playerID ));

	for(i = 0; i < MAX_CLIENTS; i++)
		if(playerClients[i].active && playerClients[i].hasClient && playerClients[i].version == 1)
			trap_SendServerCommand( i, va("clearRemap %i", playerID ));
	
}

void MMS_GetClientVersion(int clientNum)
{
	playerClients[clientNum].active = qtrue;
	playerClients[clientNum].hasClient = qfalse;
	playerClients[clientNum].version = 0;
	playerClients[clientNum].features = 0;
	playerClients[clientNum].numMessages = 0;
	playerClients[clientNum].auth = rand()%25000;
	trap_SendServerCommand( clientNum, va("mmClientVersion %i", playerClients[clientNum].auth));
}

void MMS_ConfirmClientVersion( gentity_t *ent)
{
	int clientNum = ent - g_entities;
	mm_clientVersion *client = &playerClients[clientNum];
	int numArgs = trap_Argc();
	char args[3][MAX_STRING_CHARS] = { 0 };
	int auth, version, features;

	if(numArgs != 4)
		return;

	trap_Argv( 1, args[0], sizeof(args[0]) );
	trap_Argv( 2, args[1], sizeof(args[1]) );
	trap_Argv( 3, args[2], sizeof(args[2]) );

	if(!isNumber(args[0], &auth) || !isNumber(args[1],&version) || !isNumber(args[2],&features))
		return;

	if(auth != client->auth)
		return;

	client->hasClient = qtrue;
	client->version = version;
	client->features = features;

	if(version == 1)
	{
		if(client->features & 1) // Supports and wants shader remaps
		{
			trap_SendServerCommand( clientNum, va("mmEnableFeature %i", 1));
			MMS_JoinUpdateAllRemaps(clientNum);
		}
	}
}


// Planned changes:
//Done// *Fix the messages to clients
// *Fix types
//Done// *Detect when a shader is changed to itself, thus remove it.
//Done-ish// *Make remove commands
//Done//	-Remove remap from a group
//Done//	-Remove a group
// *List remaps
//Done// *Make default groups with objectName. ####Urgent
// Update minfo
// Fix command names.
// Give a message to people who don't appear to have the client plugin



// Below is just a proof of rotatable bounding boxes. 
// Not complete, needs modified makermod scaling system and a clientside version.
//// 0B0 75 05                                   jnz     short loc_446399
//#define ADDR1 0x00446392
//// 010 75 05								   jnz     short loc_4460AE
//#define ADDR2 0x004460A7
//// 0040EF43  3D FF 01 00 00                    cmp     eax, 1FFh
//#define ADDR3 0x0040EF45
//byte *addr1 = (byte*)ADDR1;
//byte *addr2 = (byte*)ADDR2;
//byte *addr3 = (byte*)ADDR3;
//#include <windows.h>
//void TestRotateBoundingBoxPatch()
//{
//	int dummy;
//
//  // First two are just sabotaging a check to see if the object isn't a bmodel, which sets the angles to a default angle
//	VirtualProtect((LPVOID)addr1, 1, PAGE_EXECUTE_READWRITE, &dummy);
//	(*addr1) = 0xEB;
//	VirtualProtect((LPVOID)addr1, 1, PAGE_EXECUTE_READ, NULL);
//
//	VirtualProtect((LPVOID)addr2, 1, PAGE_EXECUTE_READWRITE, &dummy);
//	(*addr2) = 0xEB;
//	VirtualProtect((LPVOID)addr2, 1, PAGE_EXECUTE_READ, NULL);
//
//	// This one sabotages a check in CM_TransformedBoxTrace
//	// that checks if the model uses the bounding box index, (511) by increasing that number so the check never succeeds based on that criteria.
//	VirtualProtect((LPVOID)addr3, 1, PAGE_EXECUTE_READWRITE, &dummy);
//	(*addr3) = 0x02;
//	VirtualProtect((LPVOID)addr3, 1, PAGE_EXECUTE_READ, NULL);
//}