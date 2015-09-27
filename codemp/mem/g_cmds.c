// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

#ifdef RANKINGMOD
#include "..\ToastPlayerInfo\ToastPlayerInfo.h"
#endif

#include "bg_saga.h"
#include "be_aas.h"
#include "../qcommon/qfiles.h" 
  
#include "../../ui/menudef.h"			// for the voice chats

extern stringID_table_t animTable[MAX_ANIMATIONS+1];
//#include "../cgame/animtable.h"

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

#include "../namespace_begin.h"
void WP_SetSaber( int entNum, saberInfo_t *sabers, int saberNum, const char *saberName );
#include "../namespace_end.h"

void Cmd_NPC_f( gentity_t *ent );
void SetTeamQuick(gentity_t *ent, int team, qboolean doBegin);
void DismembermentTest(gentity_t *self);
void G_TestLine(vec3_t start, vec3_t end, int color, int time);

#define SERVER_CREATOR -5

#define MAX_SPAWN_CHARS 4096

#define EMPTY_MODEL "*1"   // the model we use to make sure nothing is displayed. null -> RGB axes

#define CHARACTER_END {-1,-1,-1,-1}

#define MAX_ARM_LENGTH 500
#define MIN_ARM_LENGTH -500

// Only capitals at the moment
mmodcharacter_t mmodFont[] = { 
	{ {0,0,0,4}, {0,4,4,4}, {4,4,4,0}, {0,2,4,2}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //A
	{ {0,0,0,4}, {0,4,4,3}, {4,3,0,2}, {0,2,4,1}, {4,1,0,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //B
	{ {0,0,0,4}, {0,4,4,4}, {0,0,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //C
	{ {0,0,0,4}, {0,4,4,2}, {4,2,0,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //D
	{ {0,0,0,4}, {0,4,4,4}, {0,2,3,2}, {0,0,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //E
	{ {0,0,0,4}, {0,4,4,4}, {0,2,3,2}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //F
	{ {0,0,0,4}, {0,4,4,4}, {0,0,4,0}, {4,0,4,2}, {4,2,3,2}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //G	
	{ {0,0,0,4}, {0,2,4,2}, {4,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //H
	{ {0,0,4,0}, {2,0,2,4}, {0,4,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //I
	{ {0,0,2,0}, {2,0,2,4}, {0,4,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //J
	{ {0,0,0,4}, {4,4,0,2}, {0,2,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //K
	{ {0,0,0,4}, {0,0,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //L
	{ {0,0,0,4}, {0,4,2,2}, {2,2,4,4}, {4,4,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //M
	{ {0,0,0,4}, {0,4,4,0}, {4,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //N
	{ {0,0,0,4}, {0,4,4,4}, {4,4,4,0}, {4,0,0,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //O
	{ {0,0,0,4}, {0,4,4,4}, {4,4,4,2}, {4,2,0,2}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //P
	{ {0,0,0,4}, {0,4,4,4}, {4,4,4,0}, {4,0,0,0}, {4,0,3,1}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //Q
	{ {0,0,0,4}, {0,4,4,4}, {4,4,4,2}, {4,2,0,2}, {0,2,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //R
	{ {0,4,4,4}, {0,4,0,2}, {0,2,4,2}, {4,2,4,0}, {4,0,0,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //S
	{ {2,0,2,4}, {0,4,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //T
	{ {0,4,0,0}, {0,0,4,0}, {4,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //U
	{ {0,4,2,0}, {2,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //V
	{ {0,4,1,0}, {1,0,2,2}, {2,2,3,0}, {3,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //W
	{ {0,4,4,0}, {0,0,4,4}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //X
	{ {0,4,2,2}, {2,2,4,4}, {2,2,2,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END }, //Y
	{ {0,4,4,4}, {4,4,0,0}, {0,0,4,0}, CHARACTER_END, CHARACTER_END, CHARACTER_END, CHARACTER_END } //Z
};


char *permissionCode[32] =
{
	"EMPOWER",
	"TELE",
	"SCALE",
	"ANIM",
	"PLACE_OBS",
	"PLACE_FX",
	"LOAD_SAVE",
	"",
	"",
	"",
	"",
	"ADMIN_SAY",
	"ADMIN_TELE",
	"",
	"NPC_SPAWN",
	"NOCLIP",
	"GOD",
	"SLAP",
	"SLAY",
	"ADMIN_OBS",
	"ADMIN_NPCS",
	"NEW_USER",
	"ADMIN_USER",
	"ADMIN_PERMISSION",
	"ADMIN_STATUS",
	"ADMIN_KICK",
	"ADMIN_BAN",
	"SAVE_MAPOBS",
	"MARK_EDGES",
	"",
	"DEVELOPER_TESTING",
	""
};

void Mplace_Think( gentity_t *ent );

gentity_t* SelectedEnt(gentity_t *ent)
{
	gentity_t* target;
	
	if ( ent->client->manipulating == 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You have not selected an object,\nplease mplace one or mselect one.\n\""));
		return 0;
	}

	target = &g_entities[ent->client->manipulating];

	if ( !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) && target->creator != ent - g_entities + 1)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You're not the owner of this object,\nplease mplace or mselect another one.\n\""));
		return 0;
	}
	
	if (target->client && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This is a player entity.\n\""));
		return 0;
	}

	if ( !target->inuse )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Object has been deleted/replaced since you selected it.\n\""));
		return 0;
	}

	return target;
}

qboolean HasPermission( gentity_t* ent, int permissionClass )
{
	if ( !(ent->client->sess.permissions & permissionClass) ) //!g_cheats.integer &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return qfalse;
	}
	else
		return qtrue;
}

// Get an empty slot from our array or spawnVar strings
char* GetSpawnVarsSlot()
{
	int i;

	for( i = firstEmptySpawnVarsSlot ; i < MAX_SPAWN_VARS_SLOTS ; i++ )
	{
		if ( spawnVars[i][0] == '\0' )
			break;
	}
	
	if ( i >= MAX_SPAWN_VARS_SLOTS )
	{
		return 0;
	}
	else
	{
		firstEmptySpawnVarsSlot = i + 1;
		return &(spawnVars[i][0]);
	}
}

void FreeSpawnVarsSlot(char* slot)
{
	firstEmptySpawnVarsSlot = 0;
	*slot = '\0';
}

// Spawn types

qboolean CanExistHere( vec3_t pos, int entityType )
{
	vec3_t delta;
	gentity_t* ent;
	int allowableDistanceFromSpawn;

	allowableDistanceFromSpawn = g_spawnProtectDistance.value;

	if ( allowableDistanceFromSpawn == 0 )
		return qtrue;

	ent = g_entities;

	// Nothing's allowed near to a spawn point
	while (( ent = G_Find (ent, FOFS(classname), "info_player_deathmatch")) != NULL ) 
	{
		VectorSubtract( ent->s.origin, pos, delta );
		if ( (VectorLength(delta)) < allowableDistanceFromSpawn )
			return qfalse;
	}

	while (( ent = G_Find (ent, FOFS(classname), "info_player_start")) != NULL ) 
	{
		VectorSubtract( ent->s.origin, pos, delta );
		if ( (VectorLength(delta)) < allowableDistanceFromSpawn )
			return qfalse;
	}
	
	return qtrue;
}

#define SPACE_LENGTH 1

void GrabObject( gentity_t* grabee, gentity_t* grabber );
// Add spawn variables to an ent and run a spawn function for the appropriate class.
void AddSpawnVars( gentity_t* target, char* vars )
{
	int		type, i, numChars, fieldLength, valueLength;
	char	*ptr;
	gentity_t *grabber;

	if ( vars == 0 )
		return;

	VectorCopy( target->r.currentOrigin, target->s.origin );


	type = target->s.eType;

	//if ( target->creator > 0 &&  g_entities[target->creator - 1].client->sess.permissions & PERMISSION_TESTING )
	//{
		//trap_UnlinkEntity( target );
	//}
	
	G_ApplySpawnVars( target, target->spawnFields, vars );

	if ( target->s.eType != ET_PORTAL && stricmp(target->classname, "misc_turretG2") )
		target->s.eType = type;
	//target->classname = "mplaced";
	
	if ( target->spawnFields )
		FreeSpawnVarsSlot( target->spawnFields );

	target->spawnFields = GetSpawnVarsSlot();


	// Keep a record of our spawn vars for use when saving and possibly after commands like mmove.
	if ( target->spawnFields == 0 )
	{
		G_Printf( "ERROR: No more spawnVars slots %i.\n", target-g_entities );
	}
	else
	{	
		if ( strlen( vars ) > MAX_SPAWN_VARS_LENGTH )
		{
			G_Printf( "ERROR: Entity spawn field string longer than expected.\n" );
		}
		else
		{
			// We'll use the spawn var array in level, because this has already had duplicates replaced
			ptr = target->spawnFields;
			numChars = 0;

			for( i = 0 ; i < level.numSpawnVars ; i++ )
			{
				fieldLength = strlen( level.spawnVars[i][0] );
				valueLength = strlen( level.spawnVars[i][1] );

				numChars +=  fieldLength + SPACE_LENGTH + 1 + valueLength +1  + SPACE_LENGTH;

				if ( numChars > MAX_SPAWN_VARS_LENGTH )
					break;

				strcpy( ptr, level.spawnVars[i][0] );
				ptr += strlen( ptr );
				
				*ptr = ' ';
				ptr++;

				*ptr = '\"';
				ptr++;

				strcpy( ptr, level.spawnVars[i][1] );
				ptr += strlen( ptr );

				*ptr = '\"';
				ptr++;

				*ptr = ' ';
				ptr++;
			}

			if ( numChars > MAX_SPAWN_VARS_LENGTH )
				G_Printf( "ERROR: Couldn't fit everything into spawnVar string.\n" ); 
		}
	}	

	if ( target->s.eType == ET_FX ) // need our custom moving code to rotate fx's
	{
		target->think = Mplace_Think;
		target->nextthink = level.time;
	}
	if ( target->grabbing)
	{
		grabber = &g_entities[target->creator-1];
		GrabObject(target, grabber);
	}
}

void Cmd_mSet_f( gentity_t *ent ) 
{
	gentity_t	*target;
	char*		args;
	int i, nArgs;
	char buffer[MAX_STRING_CHARS];

//	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
//	{
//		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
//		return;
//	}

	if ( !HasPermission( ent, PERMISSION_TESTING ) )
		return;

	nArgs = trap_Argc();

	if ( nArgs < 2  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mset <fieldname> <value> <fieldname> <value>.......\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	args = buffer;

    // put all of the arguments in inverted comma's
	for( i = 1 ; i <= nArgs && args < (buffer + sizeof(buffer)) ; i++ )
	{
		*args = '\"';
		args++;
		
		trap_Argv( i, args, sizeof( buffer ) - (args-buffer) );
		args+= strlen(args);

		if (args < (&buffer[sizeof(buffer)]))
		{
			*args = '\"';
			args++;
		}
	}

	if (args < (&buffer[sizeof(buffer)]))
		*args = '\0';
	else
		buffer[sizeof(buffer)-1] = 0;

	AddSpawnVars( target, buffer );
}

// accepts a space-delimitted string of permissioning symbols
int StringToPermissionsNum( char* permissionString )
{
	int j, result;
	char *spacePos, *dataPos;

	result = 0;

	if ( permissionString == 0 )
		return 0;

	// I didn't use str str because we can't stop someone adding a permission symbol that's a 
	// part of another larger symbol.

	for( dataPos = permissionString ; *dataPos != 0 ; dataPos = spacePos + 1 )  
	{
		spacePos = strchr(dataPos, ' ');

		if ( spacePos == 0 )
			spacePos = permissionString + strlen( permissionString );

		for( j = 0 ; j < MAX_USER_PERMISSION_CODE ; j++ )
		{
			if ( Q_stricmpn(permissionCode[j], dataPos, spacePos - dataPos) == 0 )
			{
				result |= (1 << j);
				break;
			}
		}
	}

	return result;
}

// Log the fact that a screen print was sent to all clients.
// Stop our automated screenprints obscuring it for each client.
void G_LogScreenPrintAll()
{
	int i;
	gclient_t *cl;

	for ( i=0 ; i< g_maxclients.integer ; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		cl->pers.lastMessageTime = level.time;
	}
}

// this function has been modified to handle partial names and client numbers
static int G_ClientNumFromNetname(int forClient, char *name)
{
	int i = 0;
	int length;

	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	gclient_t*	cl;

	// convert number
	SanitizeString2( (char*)name, s2 );
	length = strlen(s2);
	if ( length <= 2 && isdigit(s2[0]) && (length == 1 || isdigit(s2[1])) )
	{
		int num = atoi(s2);
		
		if ( num < 0 || num >= level.maxclients ) {
			trap_SendServerCommand( forClient, va("print \"Bad client slot: %i\n\"", num));
			return -1;
		}

		cl = &level.clients[num];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( forClient, va("print \"Client %i is not active\n\"", num));
			return -1;
		} 

		return num;
	}

	// check for a name match
	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if ( cl->pers.connected != CON_CONNECTED ) 
			continue;

		SanitizeString2( cl->pers.netname, n2 );
		if ( !Q_stricmpn(n2, s2, length) ) 
		{
			return i;
		}
	}

	trap_SendServerCommand( forClient, va("print \"User %s is not on the server\n\"", s2));
	return -1;
}

void mplace_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	vec3_t		org, dir, up;
	float		scale;
	int			i, numChunks, size = 0;
	material_t	chunkType = self->material;
	
	// if a missile is stuck to us, blow it up so we don't look dumb
	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		if ( g_entities[i].s.groundEntityNum == self->s.number && ( g_entities[i].s.eFlags & EF_MISSILE_STICK ))
		{
			G_Damage( &g_entities[i], self, self, NULL, NULL, 99999, 0, MOD_CRUSH ); //?? MOD?
		}
	}

	//So chunks don't get stuck inside me
	self->s.solid = 0;
	self->r.contents = 0;
	self->clipmask = 0;
	trap_LinkEntity(self); 

	VectorSet(up, 0, 0, 1);

	if ( self->target && attacker != NULL )
	{
		G_UseTargets(self, attacker);
	}

	VectorSubtract( self->r.absmax, self->r.absmin, org );// size

	numChunks = random() * 6 + 18;

	// This formula really has no logical basis other than the fact that it seemed to be the closest to yielding the results that I wanted.
	// Volume is length * width * height...then break that volume down based on how many chunks we have
	scale = sqrt( sqrt( org[0] * org[1] * org[2] )) * 1.75f;

	if ( scale > 48 )
	{
		size = 2;
	}
	else if ( scale > 24 )
	{
		size = 1;
	}

	scale = scale / numChunks;

	if ( self->radius > 0.0f )
	{
		// designer wants to scale number of chunks, helpful because the above scale code is far from perfect
		//	I do this after the scale calculation because it seems that the chunk size generally seems to be very close, it's just the number of chunks is a bit weak
		numChunks *= self->radius;
	}

	VectorMA( self->r.absmin, 0.5, org, org );
	VectorAdd( self->r.absmin,self->r.absmax, org );
	VectorScale( org, 0.5f, org );

	if ( attacker != NULL && attacker->client )
	{
		VectorSubtract( org, attacker->r.currentOrigin, dir );
		VectorNormalize( dir );
	}
	else
	{
		VectorCopy(up, dir);
	} 

	if ( !(self->spawnflags & 2048) ) // NO_EXPLOSION
	{
		// we are allowed to explode
		G_MiscModelExplosion( self->r.absmin, self->r.absmax, size, chunkType );
	}

	if (self->genericValue15)
	{ //a custom effect to play
		vec3_t ang;
		VectorSet(ang, 0.0f, 1.0f, 0.0f);
		G_PlayEffectID(self->genericValue15, org, ang);
	}

	//Don't do for makermod as it seems to cause server crashes
	if ( self->creator == 0 && self->splashDamage > 0 && self->splashRadius > 0 )  
	{
		gentity_t *te;
		//explode
		G_RadiusDamage( org, self, self->splashDamage, self->splashRadius, self, NULL, MOD_UNKNOWN );

		te = G_TempEntity( org, EV_GENERAL_SOUND );
		te->s.eventParm = G_SoundIndex("sound/weapons/explosions/cargoexplode.wav");
	}

	//FIXME: base numChunks off size?
	G_Chunks( self->s.number, org, dir, self->r.absmin, self->r.absmax, 300, numChunks, chunkType, 0, (scale*self->mass) );

	trap_AdjustAreaPortalState( self, qtrue );

	//G_Printf("PRE- mplace_die %s", AllocPoints[self-g_entities].AllocPointer);
	//G_Free(self-g_entities);
	//Btrap_TrueFree((void **)&AllocPoints[self-g_entities].AllocPointer);
	//G_Printf("POST- mplace_die %s", AllocPoints[self-g_entities].AllocPointer);

	self->think = G_FreeEntity;
	self->nextthink = level.time + 50;
	//G_FreeEntity( self );
}


#define NUM_CONFLICTING_CLASSES 13
#define BOBBING_INDEX 0
#define ROTATING_INDEX 1
#define PLATFORM_INDEX 2
#define PENDULUM_INDEX 3
#define DOOR_INDEX 4
#define BUTTON_INDEX 5
const char* g_conflictingClasses[NUM_CONFLICTING_CLASSES] = { "func_bobbing", "func_rotating", "func_plat", "func_pendulum", "func_door", "func_button", "misc_turret", "misc_turretG2", "trigger_push", "target_speaker", "emplaced_gun", "misc_ammo_floor_unit", "misc_shield_floor_unit" };


// newClassname is the new class we're applying to this ent. If it's 0, we assume the user is applying pain or breakability.
qboolean IsConflictingClass( gentity_t* ent, char* newClassname )
{
	int i, j, length;
	char* ptr, *end;
	int newClassIndex;
	char spawnFields[MAX_SPAWN_VARS_LENGTH];

	//G_Printf( "TEST45" );

	if ( ent->spawnFields == 0 )
		return qfalse;


	//G_Printf( "TEST46" );

	length = strlen( ent->spawnFields );

	newClassIndex = -1;

	if ( newClassname && strlen(newClassname) > 0 )
	{
		for( j = 0 ; j < NUM_CONFLICTING_CLASSES ; j++ )
		{
			if ( strcmp(newClassname, g_conflictingClasses[j]) == 0 )
			{
				newClassIndex = j;
				break;
			}
		}

		//G_Printf( "TEST47" );

		if ( newClassIndex == -1 )
			return qfalse; // the new class we're applying isn't one that causes conflicts.
	}

	//G_Printf( "TEST51 %i ", newClassIndex );


	for ( i = 0 ; i < length ; i++ )
	{
		ptr = strstr( &(ent->spawnFields[i]), "classname" );

		if ( ptr == 0 )
			return qfalse;

		ptr += 11; // skip to the actual classname

		//G_Printf( "TEST52 %s", ptr );

		for( end = ptr ; *end != '\"' && *end != '\0' ; end++ )
			;

		for( j = 0 ; j < NUM_CONFLICTING_CLASSES ; j++ )
		{
			//G_Printf( "TEST53 %i", end - ptr );
			if ( j == newClassIndex )
				continue; // re-applying a class doesn't cause conflicts.

			if ( Q_strncmp(ptr, g_conflictingClasses[j], end - ptr) == 0 )
			{
				// rotating obs are compatible with quite a few other obs
				if ( j == ROTATING_INDEX )
				{
					if ( newClassIndex == BOBBING_INDEX ||
						 newClassIndex == PLATFORM_INDEX ||
						 newClassIndex == PENDULUM_INDEX ||
						 newClassIndex == DOOR_INDEX ||
						 newClassIndex == BUTTON_INDEX )
						 continue;
				}

				if (newClassIndex == ROTATING_INDEX ) 
				{
						if ( j == BOBBING_INDEX ||
							j	 == PLATFORM_INDEX || 
							j == PENDULUM_INDEX ||
							j == DOOR_INDEX ||
							j == BUTTON_INDEX)
						 continue;
				}

				// Platform and pendulum are also vaguely compatible
				if ( (newClassIndex == PLATFORM_INDEX && j == PENDULUM_INDEX) ||
					 (newClassIndex == PENDULUM_INDEX && j == PLATFORM_INDEX) )
					 continue;

				//G_Printf( "TEST1: %s", g_conflictingClasses[j] );
				return qtrue;
			}
		}
	}

	//G_Printf( "TEST54" );

	return qfalse;
}

qboolean IsPainOrBreakable( gentity_t* ent, int num )
{

	if (num == 2 || !num)
	{
		if ( ent->health != 0 && ent->die == mplace_die )
			return qtrue; // breakable
	}

	if (num == 1 || !num)
	{
		if ( ent->splashDamage > 0 && ent->spawnflags & 4 )
			return qtrue; // pain inflicting ob
	}

	return qfalse;
}

/*
================
RemoveConfig
By Scooper
================
*/
int RemoveConfig(const char *buffer, int start, int max, int id)
{
	gentity_t *config;
	int i = 0;

	if(!id)
	{
		for(i; i < ENTITYNUM_MAX_NORMAL;i++)
		{
			config = &g_entities[i];

		/*	if (!config->target6 || !config->target6[0] || !buffer || !buffer[0])
				return;*/

			if( !Q_stricmp(config->target6, buffer) )
			{
				//G_Printf("test3 %s %s", buffer, config->target6);
				return 0;
			}
		}
	}
	//G_Printf("test4 %s %s", buffer, config->target6);
	if (!id)
	return G_RemoveConfigstringIndex(buffer, start, max, 0);
	else return G_RemoveConfigstringIndex(buffer, start, max, id);
}

void DoAnim( gentity_t* ent, int anim, int length, qboolean saberAllowed, qboolean allowWalk ) // length = -1 for anim-determined length
{
	if ( !saberAllowed )
	{
		if ( ent->client->ps.saberHolstered == 1 
			&& ent->client->saber[1].model 
			&& ent->client->saber[1].model[0] )
		{
			//turn off second saber
			G_Sound( ent, CHAN_WEAPON, ent->client->saber[1].soundOff );
		}
		else if ( ent->client->ps.saberHolstered == 0 )
		{
			//turn off first
			G_Sound( ent, CHAN_WEAPON, ent->client->saber[0].soundOff );
		}

		ent->client->ps.saberHolstered = 2;
	}

//	if ( length > 0 )
//		G_SetAnim(ent, NULL, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 5000);
//	else
//	{
		if ( ent->client->ps.groundEntityNum != ENTITYNUM_NONE ) 
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
			ent->client->ps.forceDodgeAnim = anim; //BOTH_SIT3;

			if ( length > 0 )
				ent->client->ps.forceHandExtendTime = level.time + length;
			else
				ent->client->ps.forceHandExtendTime = level.time + BG_AnimLength(ent->localAnimIndex, (animNumber_t)anim);

			ent->client->ps.pm_type = PM_FREEZE;
		}
//	}

}




qboolean LoadSizes( gentity_t *obj )
{
	qboolean found;
	int i, fileStatus;
	char fileData[MAX_OBINFO_FILE_LENGTH];
	fileHandle_t	f;

	// Use our cached model sizes if possible, otherwise get the info from disk.

	found = qfalse;

	for( i = 0 ; i < MAX_MODEL_SIZES && modelSize[i].model[0] != 0 ; i++ )
	{
		if ( Q_stricmp(obj->model2, modelSize[i].model) == 0 )
		{
			found = qtrue;
			break;
		}
	}

	if ( !found && i < MAX_MODEL_SIZES )
	{
		fileStatus = trap_FS_FOpenFile( va( "ObInfo/%s", obj->model2 ), &f, FS_READ);

		if (f)
		{
			

			trap_FS_Read( fileData, min(fileStatus, MAX_OBINFO_FILE_LENGTH), f );

			Q_strncpyz( modelSize[i].model, obj->model2, sizeof(modelSize[i].model) );

			// The first number is the file version
			// The last number, is for some yet-to-be-discovered need.
			if ( sscanf(fileData, "1 %f %f %f %f %f %f 0", &(modelSize[i].up), &(modelSize[i].down), &(modelSize[i].xleft), &(modelSize[i].xright), &(modelSize[i].yforward), &(modelSize[i].yback) ) == 6 )
				found = qtrue;

			trap_FS_FCloseFile(f);
		}
	}

	if ( !found )
	{
		// See if we can get some bounds from the MD3 file. 
		int j;
		char* data, *dataPtr; //, *nextSpace;

		md3Header_t* header;
		md3Frame_t* frame;


		j = trap_FS_FOpenFile( va( "%s", obj->model2), &f, FS_READ);

		// if the file already exists, check we've got the right password.
		if ( !f )
		{
			G_Printf( "LOADSIZES() FAILED TO LOAD MD3: File not found\n" );
			return qfalse;
		}

		//G_Printf( "2\n" );

/*		if ( j > 127500 )
		{
			G_Printf( "LOADSIZES() FAILED TO LOAD MD3: File too large\n" );

			trap_FS_FCloseFile( f );
			return qfalse;
		} */

		data = malloc(128000);//BG_Alloc( 128000 );

		dataPtr = data;

		trap_FS_Read( data, min(j, 127449), f ); 

		data[min(j, 127500)] = '\0';

		if ( j < sizeof( md3Header_t ) )
		{
			trap_FS_FCloseFile(f);
			free(data);
			return qfalse;
		}

		header = (md3Header_t*) data;

		if ( header->numFrames <= 0 || header->ofsFrames <= 0 )
		{
			trap_FS_FCloseFile( f );
			free(data);
			G_Printf( "LOADSIZES() FAILED TO LOAD MD3: File too short\n" );
			return qfalse;
		}

		frame = (md3Frame_t*) (data + header->ofsFrames);
		
		Q_strncpyz( modelSize[i].model, obj->model2, sizeof(modelSize[i].model) );

		modelSize[i].up       = frame->bounds[1][2];
		modelSize[i].down     = frame->bounds[0][2];
		modelSize[i].xleft    = frame->bounds[0][0];
		modelSize[i].xright   = frame->bounds[1][0];
		modelSize[i].yforward = frame->bounds[1][1];
		modelSize[i].yback    = frame->bounds[0][1];
		
		found = qtrue;

		free(data);
		trap_FS_FCloseFile(f);

	}

	if ( found ) 
	{
		float scaleFactor = min( obj->s.iModelScale, 300 ); // maximum bbox size is 3 times the normal to make sure we don't have too much empty space

		scaleFactor /= 100.;

		if ( scaleFactor == 0 )
			scaleFactor = 1;

		VectorSet( obj->r.maxs, modelSize[i].xright * scaleFactor, modelSize[i].yforward * scaleFactor, modelSize[i].up * scaleFactor );
		VectorSet( obj->r.mins, (modelSize[i].xleft * scaleFactor), (modelSize[i].yback * scaleFactor), modelSize[i].down * scaleFactor );

				// Calculate a grabRadius
		if ( obj->s.iModelScale > 300 )
		{
			float length = (obj->s.iModelScale * max( VectorLength(obj->r.maxs), VectorLength(obj->r.mins) )) / 300;
			obj->grabRadius = length;
		}
		else
			obj->grabRadius = max( VectorLength(obj->r.maxs), VectorLength(obj->r.mins) );

		//VectorSet( obj->r.absmax, modelSize[i].horizontal, modelSize[i].horizontal, modelSize[i].up );
		//VectorSet( obj->r.absmin, ((-1)*modelSize[i].horizontal), ((-1)*modelSize[i].horizontal), modelSize[i].down );
		
		// keep a copy of our x-values for use in a glitchy-movement-bug workaround during clientthink()
		obj->ymaxs = obj->r.maxs[0];
		obj->ymins = obj->r.mins[0];

		// prevent glitchy-movement bug
		obj->r.maxs[0] = min( obj->r.maxs[0], obj->r.maxs[1] );
		obj->r.mins[0] = max( obj->r.mins[0], obj->r.mins[1] );
		return qtrue;
	}

	return qfalse;
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;
	
	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}
		
		if( cl->accuracy_shots ) {
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
			cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy, 
			cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
			cl->ps.persistant[PERS_EXCELLENT_COUNT],
			cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], 
			cl->ps.persistant[PERS_DEFEND_COUNT], 
			cl->ps.persistant[PERS_ASSIST_COUNT], 
			perfect,
			cl->ps.persistant[PERS_CAPTURES]);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return qfalse;
	}

	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		return qfalse;
	}

	if (  ent->client && ent->client->ps.duelInProgress )
		return qfalse;

	return qtrue;
}

#define BOX_NUM_CORNERS 8
#define THINPLANE_SCALEUP 100 // because our rotation function only works in whole numbers

void SetupClippingBox( gentity_t *ent )
{
	int i, rotAxis, j;
	vec3_t edge1, edge2, direction, temp;
	//float usefulAngle;
	//int sideCount;
	float length;
	//float sidesRatio[3];
	float side[2];
	vec3_t thinPlane[3];
	vec3_t corner[8];

	if ( ent->s.eType != ET_MOVER ) 
		return;

	// Always rotate from angles (0,0,0)
	LoadSizes( ent );

	// Swap to our actual clipping box, instead of the bug work-around.
	ent->r.maxs[0] = ent->ymaxs;
	ent->r.mins[0] = ent->ymins;


/*		// find the ratio between sides of the clipping box affected by each axis of rotation
	for( rotAxis = 0 ; rotAxis < 3 ; rotAxis++ )  
	{
		sideCount = 0;

		for( i = 0 ; i < 3 ; i++ )
		{
			if ( (rotAxis == 0 && i != 1) || (rotAxis == 1 && i != 2) || (rotAxis == 2 && i != 0) )
			{
				side[sideCount] = ent->r.maxs[i] - ent->r.mins[i];
				sideCount++;
			}
		}

		if ( g_entities[ent->creator - 1].client && (g_entities[ent->creator - 1].client->sess.permissions & PERMISSION_TESTING) )
		trap_SendServerCommand( ent->creator - 1, va("print \"s%f %f\"", side[0], side[1] ));


		if ( side[1] > side[0] && side[1] != 0 )
			sidesRatio[rotAxis] = side[0]/side[1];
		else if ( side[0] != 0 )
			sidesRatio[rotAxis] = side[1]/side[0];
		else
			sidesRatio[rotAxis] = 1;
	} 
*/

	// Track the corners of the clipping box as it's rotated
	VectorCopy( ent->r.maxs, corner[0] );
	VectorCopy( ent->r.mins, corner[1] );
	VectorSet( corner[2], ent->r.maxs[0], ent->r.maxs[1], ent->r.mins[2] );
	VectorSet( corner[3], ent->r.maxs[0], ent->r.mins[1], ent->r.mins[2] );
	VectorSet( corner[4], ent->r.mins[0], ent->r.maxs[1], ent->r.maxs[2] );
	VectorSet( corner[5], ent->r.mins[0], ent->r.mins[1], ent->r.maxs[2] );
	VectorSet( corner[6], ent->r.maxs[0], ent->r.mins[1], ent->r.maxs[2] );
	VectorSet( corner[7], ent->r.mins[0], ent->r.maxs[1], ent->r.mins[2] );

	for( i = 0 ; i < 3 ; i++ )
		VectorSet( thinPlane[i], 0, 0, 0 );

	for( rotAxis = 0 ; rotAxis < 3 ; rotAxis++ )  
	{
		for( i = 0 ; i < 3 ; i++ )
		{
			if ( i != rotAxis )
			{
				side[0] = ent->r.maxs[i] - ent->r.mins[i];
				side[1] = ent->r.maxs[rotAxis] - ent->r.mins[rotAxis];

				if ( side[0]/side[1] > 1.5 && side[0]/side[1] > thinPlane[rotAxis][rotAxis] )
					thinPlane[rotAxis][rotAxis] = side[0]/side[1] * THINPLANE_SCALEUP;
			}
		}
	} 


//	if ( g_entities[ent->creator - 1].client && (g_entities[ent->creator - 1].client->sess.permissions & PERMISSION_TESTING) )
//		trap_SendServerCommand( ent->creator - 1, va("print \"%s %s %s\n\"", vtos(thinPlane[0]), vtos(thinPlane[1]), vtos(thinPlane[2]) ));

	// Rotate maxs,mins and the vectors pointing out from the thinnest planes in our 

	// Second axis
	VectorSet( direction, 1, 0, 0 );
	RotatePointAroundVector( edge1, direction, ent->r.maxs, ent->r.currentAngles[2] );
	RotatePointAroundVector( edge2, direction,  ent->r.mins, ent->r.currentAngles[2] );


	for( i = 0 ; i < 3 ; i++ )
	{
		if ( VectorLengthSquared(thinPlane[i]) != 0 )
		{
			VectorCopy( thinPlane[i], temp );
			RotatePointAroundVector( thinPlane[i], direction, temp, ent->r.currentAngles[2] );
		}
	}

	for( i = 0 ; i < BOX_NUM_CORNERS ; i++ )
	{
		VectorCopy( corner[i], temp );
		RotatePointAroundVector( corner[i], direction, temp, ent->r.currentAngles[2] );
	}


	// Axis for first angle.
	VectorSet( direction, 0, 1, 0 );

	VectorCopy( edge1, temp );
	RotatePointAroundVector( edge1, direction, temp, ent->r.currentAngles[0] );

	VectorCopy( edge2, temp );
	RotatePointAroundVector( edge2, direction, temp, ent->r.currentAngles[0] );

	for( i = 0 ; i < 3 ; i++ )
	{
		if ( VectorLengthSquared(thinPlane[i]) != 0 )
		{
			VectorCopy( thinPlane[i], temp );
			RotatePointAroundVector( thinPlane[i], direction, temp, ent->r.currentAngles[0] );
		}
	}

	for( i = 0 ; i < BOX_NUM_CORNERS ; i++ )
	{
		VectorCopy( corner[i], temp );
		RotatePointAroundVector( corner[i], direction, temp, ent->r.currentAngles[0] );
	}

	//if ( g_entities[ent->creator - 1].client && (g_entities[ent->creator - 1].client->sess.permissions & PERMISSION_TESTING) )
	//	trap_SendServerCommand( ent->creator - 1, va("print \"%s\n\"", vtos(edge1) ));

	// Third axis
	VectorSet( direction, 0, 0, 1 );
	VectorCopy( edge1, temp );
	RotatePointAroundVector( edge1, direction, temp, ent->r.currentAngles[1] );

	VectorCopy( edge2, temp );
	RotatePointAroundVector( edge2, direction, temp, ent->r.currentAngles[1] );

	for( i = 0 ; i < 3 ; i++ )
	{
		if ( VectorLengthSquared(thinPlane[i]) != 0 )
		{
			VectorCopy( thinPlane[i], temp );
			RotatePointAroundVector( thinPlane[i], direction, temp, ent->r.currentAngles[1] );
		}
	}

	for( i = 0 ; i < BOX_NUM_CORNERS ; i++ )
	{
		VectorCopy( corner[i], temp );
		RotatePointAroundVector( corner[i], direction, temp, ent->r.currentAngles[1] );
	}

	VectorClear( ent->r.maxs );
	VectorClear( ent->r.mins );


	for( j = 0 ; j < BOX_NUM_CORNERS ; j++ )
	{ 
		// Make sure they're working
		for( i = 0 ; i < 3 ; i++ )
		{
			ent->r.maxs[i] = max( corner[j][i], ent->r.maxs[i] );
			ent->r.mins[i] = min( corner[j][i], ent->r.mins[i] );

	//		if ( ent->r.maxs[i] < 0 ) 
	//			ent->r.maxs[i] = 0;

	//		if ( ent->r.mins[i] > 0 ) 
	//			ent->r.mins[i] = 0;
		}
	}

	// scale down flat objects if they aren't parallel with one of the axes.
	// make them smallest when we're at 45 degrees to an axis

/*	if ( g_entities[ent->creator - 1].client && (g_entities[ent->creator - 1].client->sess.permissions & PERMISSION_TESTING) )
		trap_SendServerCommand( ent->creator - 1, va("print \"sR%f %f %f\n\"", sidesRatio[0], sidesRatio[1], sidesRatio[2] ));

	for( rotAxis = 0 ; rotAxis < 3 ; rotAxis++ )
	{
		//if ( sidesRatio[rotAxis] > 0.7 )
		//	continue; // don't bother scaling as the object isn't very flat about this axis of rotation.

		usefulAngle = (abs(ent->r.currentAngles[rotAxis]) % 90) * 2;

		if ( usefulAngle < 10 || usefulAngle > 170 )
			continue;

		for( i = 0 ; i < 3 ; i++ )
		{
			if ( (rotAxis == 0 && i != 1) || (rotAxis == 1 && i != 2) || (rotAxis == 2 && i != 0) )
			{ 
				ent->r.maxs[i] -= abs( sin( usefulAngle/360*3.142*2 )*(1 - sidesRatio[rotAxis])*ent->r.maxs[i] );
				ent->r.mins[i] += abs( sin( usefulAngle/360*3.142*2 )*(1 - sidesRatio[rotAxis])*ent->r.mins[i] );
			}
		}
	}  */


	if ( g_entities[ent->creator - 1].client && (g_entities[ent->creator - 1].client->sess.permissions & PERMISSION_TESTING) )
		trap_SendServerCommand( ent->creator - 1, va("print \"%s %s %s\n\"", vtos(thinPlane[0]), vtos(thinPlane[1]), vtos(thinPlane[2]) ));


	for( rotAxis = 0 ; rotAxis < 3 ; rotAxis++ )
	{
		if ( VectorLengthSquared(thinPlane[rotAxis]) == 0 )
			continue;

		length = VectorLength( thinPlane[rotAxis] );

		for( i = 0 ; i < 3 ; i++ )
		{
			if ( abs(thinPlane[rotAxis][i]) > 0.01 && abs(thinPlane[rotAxis][i]) < (length - 1) )  // the -1 ensures we don't bother for small ammounts off-axis
			{
				//trap_SendServerCommand( ent->creator - 1, va("print \"%f %f \n\"", length, thinPlane[rotAxis][i] ));

				//trap_SendServerCommand( ent->creator - 1, va("print \"---%i %f %f\n\"", i,(1.0 - abs(thinPlane[rotAxis][i]) /length), ent->r.maxs[i]  ));

				ent->r.maxs[i] -= ent->r.maxs[i] * (abs(thinPlane[rotAxis][i]) /length) * length/(THINPLANE_SCALEUP*10) * ( 1 + sin( 3.142* (thinPlane[rotAxis][i]* thinPlane[rotAxis][i]/(length*length)) ));
				ent->r.mins[i] += abs(ent->r.mins[i]) * (abs(thinPlane[rotAxis][i]) /length) * length/(THINPLANE_SCALEUP*10)  * (1+sin( 3.142* (thinPlane[rotAxis][i]* thinPlane[rotAxis][i]/(length*length)) ));
//				trap_SendServerCommand( -1, va("print \"%s %i\n", vtos(ent->r.mins), i));
			}
		}
	} 

	// Keep a copy of our x-values for use in a glitchy-movement-bug workaround during clientthink()
	ent->ymaxs = ent->r.maxs[0];
	ent->ymins = ent->r.mins[0];

	// Prevent JKA glitchy-movement bug
	ent->r.maxs[0] = min( ent->r.maxs[0], ent->r.maxs[1] );
	ent->r.mins[0] = max( ent->r.mins[0], ent->r.mins[1] );
}

void Mplace_Think( gentity_t *ent )
{
	vec3_t origin, angles;
	qboolean stillMoving;
//	char test[MAX_STRING_CHARS];

	//trap_SendServerCommand( ent->creator - 1, va("print \"t \"" ));

//	if ( ent->s.pos.trType == TR_STATIONARY && ent->s.apos.trType == TR_STATIONARY )
//	{
//		trap_SetBrushModel( ent, ent->model2 );
//		return;
//	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	BG_EvaluateTrajectory( &ent->s.apos, level.time, angles );

	VectorCopy( origin, ent->r.currentOrigin );
	VectorCopy( angles, ent->r.currentAngles );
	//VectorCopy( angles, ent->s.angles );
	//VectorCopy( origin, ent->s.origin );

	stillMoving = qfalse;

	if ( ent->s.pos.trType == TR_INTERPOLATE  )
	{
		if ( ent->enemy == 0 || ent->enemy->client == 0 || ent->enemy->inuse == qfalse )
		{
			ent->enemy = 0;
			ent->s.pos.trType = TR_STATIONARY;
		}
		else
		{
			vec3_t vfwd, armOffset;
			AngleVectors( ent->enemy->client->ps.viewangles, vfwd, NULL, NULL );
			
			if ( ent->s.eType == ET_FX )
				VectorScale( vfwd, ent->enemy->client->sess.arm, armOffset );
			else 
				VectorScale( vfwd, ent->enemy->client->sess.arm + (ent->enemy->client->sess.arm == 0 ? 0 : ent->grabRadius), armOffset );


			VectorAdd( ent->enemy->r.currentOrigin, armOffset, ent->s.pos.trBase );
			SnapVector( ent->s.pos.trBase );

			// Move the effect straight away
			// We don't do this with solid objects because people riding on the object would fall off.
			if ( ent->s.eType == ET_FX )
				VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin ); 

			stillMoving = qtrue;
		}
	}
	else if ( ent->s.pos.trType != TR_SINE && ent->s.pos.trType != TR_LINEAR && level.time  > ent->s.pos.trDuration + ent->s.pos.trTime )
	{
		if ( ent->s.pos.trType != TR_STATIONARY )
		{
			vec3_t mins, maxs;

			VectorCopy( ent->r.mins, mins );
			VectorCopy( ent->r.maxs, maxs );

			//VectorClear( ent->r.mins );
			//VectorClear( ent->r.maxs );
			//trap_LinkEntity( ent );

			ent->s.pos.trType = TR_STATIONARY;
			VectorCopy( origin, ent->s.pos.trBase );

			if ( ent->enemy != 0 && ent->enemy->client != 0 && ent->enemy->inuse == qtrue )
			{
				// Object is being grabbed.
				ent->s.pos.trType = TR_INTERPOLATE;
				stillMoving = qtrue;
			}
		}
	}
	else
	{
		stillMoving = qtrue;
	}

	if ( ent->s.apos.trType == TR_INTERPOLATE )
	{
		if ( ent->enemy == 0 || ent->enemy->client == 0 || ent->enemy->inuse == qfalse )
		{
			ent->s.apos.trType = TR_STATIONARY;
			ent->enemy = 0;
		}
		else
		{
			VectorSet( angles, ent->r.currentAngles[0], ent->enemy->client->ps.viewangles[1], ent->r.currentAngles[2] );
			SnapVector( angles );
			VectorCopy( angles, ent->s.apos.trBase );
			//VectorCopy( angles, ent->r.currentAngles );
			stillMoving = qtrue;
			//SetupClippingBox(ent);
			//trap_LinkEntity( ent );
			//trap_LinkEntity( ent ); 
		}
	}
	else if ( ent->s.apos.trType != TR_SINE && ent->s.apos.trType != TR_LINEAR && level.time  > ent->s.apos.trDuration + ent->s.apos.trTime )
	{
		if ( ent->s.apos.trType != TR_STATIONARY )
		{
			ent->s.apos.trType = TR_STATIONARY;
			VectorCopy( angles, ent->s.apos.trBase );

			SetupClippingBox( ent );
			trap_LinkEntity( ent );


			if ( ent->enemy != 0 && ent->enemy->client != 0 && ent->enemy->inuse == qtrue )
			{
				// The object is being grabbed
				ent->s.apos.trType = TR_INTERPOLATE;
				//VectorSet( angles, ent->r.currentAngles[0], ent->enemy->client->ps.viewangles[1], ent->r.currentAngles[2] );
				//VectorCopy( angles, ent->s.apos.trBase );
				stillMoving = qtrue;
			}
		}
	}
	else
	{
		stillMoving = qtrue;
		//SnapVector( ent->r.currentAngles );
	}

	// make sure fx's still do damage after they've moved.
	// but don't do damage while it's grabbed.
	if ( ent->s.eType == ET_FX )
		fx_runner_think( ent );
//	else if ( ent->oldThink )
//		ent->oldThink( ent );



	if ( stillMoving == qtrue )
	{
		ent->think = Mplace_Think;
		ent->nextthink = level.time + FRAMETIME;
	}
	else
	{
		//if ( ent->oldThink )
		//{
		//	ent->think = ent->oldThink;
		//	ent->nextthink = level.time + FRAMETIME;
		//}

		if ( ent->s.eType != ET_FX && ent->s.eType != ET_MOVER )
		{
			AddSpawnVars( ent, va("origin \"%i %i %i\" angles \"%i %i %i\"", (int)ent->r.currentOrigin[0], (int)ent->r.currentOrigin[1], (int)ent->r.currentOrigin[2], (int)ent->r.currentAngles[0], (int)ent->r.currentAngles[1], (int)ent->r.currentAngles[2]) );
		}
	}
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( (unsigned char) *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if (s[0] >= '0' && s[0] <= '9') {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
	return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *cmdent, int baseArg)
{
	char		name[MAX_TOKEN_CHARS];
	gentity_t	*ent;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( cmdent ) ) {
		return;
	}

	if (baseArg)
	{

		char otherindex[MAX_TOKEN_CHARS];

		trap_Argv( 1, otherindex, sizeof( otherindex ) );

		if (!otherindex[0])
		{
			Com_Printf("giveother requires that the second argument be a client index number.\n");
			return;
		}

		i = atoi(otherindex);

		if (i < 0 || i >= MAX_CLIENTS)
		{
			Com_Printf("%i is not a client index\n", i);
			return;
		}

		ent = &g_entities[i];

		if (!ent->inuse || !ent->client)
		{
			Com_Printf("%i is not an active client\n", i);
			return;
		}

		if ( !(ent->client->sess.makerModFlags & GMOD_REFUSING) )
		{
			trap_SendServerCommand( cmdent-g_entities, va("print \"%s^7 is refusing giveother attempts.\n\"", ent->client->pers.netname));
			return;
		}
		trap_SendServerCommand( -1, va("print \"%s^7 used giveother on %s\n\"", cmdent->client->pers.netname, ent->client->pers.netname));

	}
	else
	{
		ent = cmdent;
	}

	trap_Argv( 1+baseArg, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}
	
	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap_Argv( 2+baseArg, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
			ent->client->ps.ammo[i] = num;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		//if ( !(ent->flags & FL_GODMODE) && g_godAllowed.value == 0  && !(ent->client->sess.permissions & PERMISSION_GOD) )
		//{
		//	trap_SendServerCommand( ent-g_entities, va("print \"This command is not allowed.\n\""));
		//	return;
		//}

		if (trap_Argc() == 3+baseArg) {
			trap_Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
		}

		if (!give_all)
			return;
	}

	if (Q_stricmp(name, "excellent") == 0) {
		ent->client->ps.persistant[PERS_EXCELLENT_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "impressive") == 0) {
		ent->client->ps.persistant[PERS_IMPRESSIVE_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "gauntletaward") == 0) {
		ent->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "defend") == 0) {
		ent->client->ps.persistant[PERS_DEFEND_COUNT]++;
		return;
	}
	if (Q_stricmp(name, "assist") == 0) {
		ent->client->ps.persistant[PERS_ASSIST_COUNT]++;
		return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
	char	*msg;

	if ( !(ent->client->sess.permissions & PERMISSION_GOD)  && !CheatsOk(ent) ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"God mode is not allowed.\n\""));
		return;
	}

	if ( !(ent->flags & FL_GODMODE) && g_godAllowed.value == 0  && !(ent->client->sess.permissions & PERMISSION_GOD) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"God mode is not allowed.\n\""));
		return;
	}

	ent->flags ^= FL_GODMODE;

	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char	*msg;

	if (  !(ent->client->sess.permissions & PERMISSION_NPC_SPAWN) && !(ent->client->sess.permissions & PERMISSION_NPC_ADMIN) && !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char	*msg;

	if ( !(ent->client->sess.permissions & PERMISSION_NOCLIP) && !CheatsOk( ent ) ) {
		return;
	}

	if ( ent->client->noclip ) {
		msg = "noclip OFF\n";
	} else {
		msg = "noclip ON\n";
	}
	ent->client->noclip = !ent->client->noclip;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
	
		trap_SendServerCommand( ent-g_entities, 
			"print \"levelshot is not allowed.\n\"" );

		return;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	// doesn't work in single player
	if ( g_gametype.integer != 0 ) {
		trap_SendServerCommand( ent-g_entities, 
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}
void GrabObject( gentity_t* grabee, gentity_t* grabber )
{
	vec3_t vfwd, armOffset;

	// There's a bug somewhere that makes new player's armlengths crazy 
	// Until we find it use this cludge
	if ( grabber->client->sess.arm > MAX_ARM_LENGTH || grabber->client->sess.arm < MIN_ARM_LENGTH )
	{
		grabber->client->sess.arm = DEFAULT_ARM_LENGTH;
	}

	grabee->enemy = grabber;
	grabee->s.apos.trType = TR_INTERPOLATE;
	grabee->s.pos.trType = TR_INTERPOLATE;
	grabee->grabbing = qtrue;

	if ( grabee->think != Mplace_Think )
		grabee->oldThink = grabee->think;

	grabee->think = Mplace_Think;
	grabee->nextthink = level.time + FRAMETIME;

	//trap_SendServerCommand( grabber-g_entities, va("print \"ra%i\n\"", grabee->grabRadius ));

	// hold it out in front of us
	AngleVectors( grabber->client->ps.viewangles, vfwd, NULL, NULL );
	//VectorScale( vfwd, grabber->client->sess.arm + max(VectorLength(grabee->r.maxs), VectorLength(grabee->r.mins))/*max( VectorLength(grabee->r.absmax), VectorLength(grabee->r.absmin) ) /*max(DotProduct(vfwd, ent->r.maxs), DotProduct(vfwd, ent->r.mins))*/, armOffset );

	if ( grabee->s.eType == ET_FX )
		VectorScale( vfwd, grabee->enemy->client->sess.arm, armOffset );
	else 
	{
		VectorScale( vfwd, grabee->enemy->client->sess.arm + ((grabee->enemy->client->sess.arm == 0) ? 0 : grabee->grabRadius), armOffset );
		//trap_SendServerCommand( grabber-g_entities, va("print \"tot%i\n\"", (int)(grabee->enemy->client->sess.arm + ((grabee->enemy->client->sess.arm == 0) ? 0 : grabee->grabRadius) )));
	}

	VectorAdd( grabber->r.currentOrigin, armOffset, grabee->s.pos.trBase );
	VectorCopy( grabee->s.pos.trBase, grabee->s.origin );
	VectorCopy( grabee->s.pos.trBase, grabee->r.currentOrigin );

	grabee->s.apos.trBase[0] = grabee->r.currentAngles[0];
	grabee->s.apos.trBase[1] = grabber->client->ps.viewangles[1]; 
	grabee->s.apos.trBase[2] = grabee->r.currentAngles[2];

	SnapVector( grabee->s.apos.trBase );
	SnapVector( grabee->s.pos.trBase );

	trap_LinkEntity( grabee );
}

/*
==================
Cmd_TeamTask_f

From TA.
==================
*/
void Cmd_TeamTask_f( gentity_t *ent ) {
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->client - level.clients;

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	trap_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	trap_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if (ent->health <= 0) {
		return;
	}

	if ((g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL) &&
		level.numPlayingClients > 1 && !level.warmupTime)
	{
		if (!g_allowDuelSuicide.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "ATTEMPTDUELKILL")) );
			return;
		}
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}

gentity_t *G_GetDuelWinner(gclient_t *client)
{
	gclient_t *wCl;
	int i;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		wCl = &level.clients[i];
		
		if (wCl && wCl != client && /*wCl->ps.clientNum != client->ps.clientNum &&*/
			wCl->pers.connected == CON_CONNECTED && wCl->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return &g_entities[wCl->ps.clientNum];
		}
	}

	return NULL;
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if (g_gametype.integer == GT_SIEGE)
	{ //don't announce these things in siege
		return;
	}

	if ( client->sess.sessionTeam == TEAM_RED ) 
	{
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
		G_LogScreenPrintAll();
	}
	else if ( client->sess.sessionTeam == TEAM_BLUE ) 
	{
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
		G_LogScreenPrintAll();
	}
	else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) 
	{
		trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));
		G_LogScreenPrintAll();
	} 
	else if ( client->sess.sessionTeam == TEAM_FREE ) 
	{
		if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		{
			/*
			gentity_t *currentWinner = G_GetDuelWinner(client);

			if (currentWinner && currentWinner->client)
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s %s\n\"",
				currentWinner->client->pers.netname, G_GetStringEdString("MP_SVGAME", "VERSUS"), client->pers.netname));
			}
			else
			{
				trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
				client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			}
			*/
			//NOTE: Just doing a vs. once it counts two players up
		}
		else
		{
			trap_SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
			G_LogScreenPrintAll();
		}
	}

	G_LogPrintf ( "setteam:  %i %s %s\n",
				  client - &level.clients[0],
				  TeamName ( oldTeam ),
				  TeamName ( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
	int			loners = 0;
	int			doubles = 0;

	if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
	{
		return qtrue;
	}

	G_PowerDuelCount(&loners, &doubles, qfalse);

	if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
	{
		return qtrue;
	}

	if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
	{
		return qtrue;
	}

	return qfalse;
}

/*
=================
SetTeam
=================
*/
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_SCOREBOARD;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( g_gametype.integer >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

		if ( g_teamForceBalance.integer && !g_trueJedi.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					trap_SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	if (g_gametype.integer == GT_SIEGE)
	{
		if (client->tempSpectate >= level.time &&
			team == TEAM_SPECTATOR)
		{ //sorry, can't do that.
			return;
		}

		client->sess.siegeDesiredTeam = team;
		//oh well, just let them go.
		/*
		if (team != TEAM_SPECTATOR)
		{ //can't switch to anything in siege unless you want to switch to being a fulltime spectator
			//fill them in on their objectives for this team now
			trap_SendServerCommand(ent-g_entities, va("sb %i", client->sess.siegeDesiredTeam));

			trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time the round begins.\n\"") );
			return;
		}
		*/
		if (client->sess.sessionTeam != TEAM_SPECTATOR &&
			team != TEAM_SPECTATOR)
		{ //not a spectator now, and not switching to spec, so you have to wait til you die.
			//trap_SendServerCommand( ent-g_entities, va("print \"You will be on the selected team the next time you respawn.\n\"") );
			qboolean doBegin;
			if (ent->client->tempSpectate >= level.time)
			{
				doBegin = qfalse;
			}
			else
			{
				doBegin = qtrue;
			}

			if (doBegin)
			{
				// Kill them so they automatically respawn in the team they wanted.
				if (ent->health > 0)
				{
					ent->flags &= ~FL_GODMODE;
					ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
					player_die( ent, ent, ent, 100000, MOD_TEAM_CHANGE ); 
				}
			}

			if (ent->client->sess.sessionTeam != ent->client->sess.siegeDesiredTeam)
			{
				SetTeamQuick(ent, ent->client->sess.siegeDesiredTeam, qfalse);
			}

			return;
		}
	}

	// override decision if limiting the players
	if ( (g_gametype.integer == GT_DUEL)
		&& level.numNonSpectatorClients >= 2 )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( (g_gametype.integer == GT_POWERDUEL)
		&& (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer )
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	//If it's siege then show the mission briefing for the team you just joined.
//	if (g_gametype.integer == GT_SIEGE && team != TEAM_SPECTATOR)
//	{
//		trap_SendServerCommand(clientNum, va("sb %i", team));
//	}

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		MaintainBodyQueue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		g_dontPenalizeTeam = qfalse;

	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		if ( (g_gametype.integer != GT_DUEL) || (oldTeam != TEAM_SPECTATOR) )	{//so you don't get dropped to the bottom of the queue for changing skins, etc.
			client->sess.spectatorTime = level.time;
		}
	}

	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			//SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	//make a disappearing effect where they were before teleporting them to the appropriate spawn point,
	//if we were not on the spec team
	if (oldTeam != TEAM_SPECTATOR)
	{
		gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = clientNum;
	}

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	if (!g_preventTeamBegin)
	{
		ClientBegin( clientNum, qfalse );
	}
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	ent->client->ps.m_iVehicleNum = 0;
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = 0;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		oldTeam = ent->client->sess.sessionTeam;
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTBLUETEAM")) );
			break;
		case TEAM_RED:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTREDTEAM")) );
			break;
		case TEAM_FREE:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
			break;
		case TEAM_SPECTATOR:
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if (gEscaping)
	{
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( g_gametype.integer == GT_DUEL
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	if (g_gametype.integer == GT_POWERDUEL)
	{ //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
		trap_SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	ent->client->switchTeamTime = level.time + 5000;
}

/*
=================
Cmd_DuelTeam_f
=================
*/
void Cmd_DuelTeam_f(gentity_t *ent)
{
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (g_gametype.integer != GT_POWERDUEL)
	{ //don't bother doing anything if this is not power duel
		return;
	}

	/*
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You cannot change your duel team unless you are a spectator.\n\""));
		return;
	}
	*/

	if ( trap_Argc() != 2 )
	{ //No arg so tell what team we're currently on.
		oldTeam = ent->client->sess.duelTeam;
		switch ( oldTeam )
		{
		case DUELTEAM_FREE:
			trap_SendServerCommand( ent-g_entities, va("print \"None\n\"") );
			break;
		case DUELTEAM_LONE:
			trap_SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
			break;
		case DUELTEAM_DOUBLE:
			trap_SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->client->switchDuelTeamTime > level.time )
	{ //debounce for changing
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	trap_Argv( 1, s, sizeof( s ) );

	oldTeam = ent->client->sess.duelTeam;

	if (!Q_stricmp(s, "free"))
	{
		ent->client->sess.duelTeam = DUELTEAM_FREE;
	}
	else if (!Q_stricmp(s, "single"))
	{
		ent->client->sess.duelTeam = DUELTEAM_LONE;
	}
	else if (!Q_stricmp(s, "double"))
	{
		ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
	}

	if (oldTeam == ent->client->sess.duelTeam)
	{ //didn't actually change, so don't care.
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //ok..die
		int curTeam = ent->client->sess.duelTeam;
		ent->client->sess.duelTeam = oldTeam;
		G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
		ent->client->sess.duelTeam = curTeam;
	}
	//reset wins and losses
	ent->client->sess.wins = 0;
	ent->client->sess.losses = 0;

	//get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	ent->client->switchDuelTeamTime = level.time + 5000;
}

int G_TeamForSiegeClass(const char *clName)
{
	int i = 0;
	int team = SIEGETEAM_TEAM1;
	siegeTeam_t *stm = BG_SiegeFindThemeForTeam(team);
	siegeClass_t *scl;

	if (!stm)
	{
		return 0;
	}

	while (team <= SIEGETEAM_TEAM2)
	{
		scl = stm->classes[i];

		if (scl && scl->name[0])
		{
			if (!Q_stricmp(clName, scl->name))
			{
				return team;
			}
		}

		i++;
		if (i >= MAX_SIEGE_CLASSES || i >= stm->numClasses)
		{
			if (team == SIEGETEAM_TEAM2)
			{
				break;
			}
			team = SIEGETEAM_TEAM2;
			stm = BG_SiegeFindThemeForTeam(team);
			i = 0;
		}
	}

	return 0;
}

gentity_t* AtCrossHairs( gentity_t* ent, int mask, vec3_t endpoint )
{
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;
	gentity_t*	target;

	if (!ent || !ent->client || ent->health < 1 ||
		(ent->client->ps.pm_flags & PMF_FOLLOW) || ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(ent->client->ps.forceHandExtend != HANDEXTEND_NONE && ent->client->ps.forceHandExtend != HANDEXTEND_DRAGGING))
	{
		return 0;
	}

	if (ent->client->ps.emplacedIndex)
	{ //on an emplaced gun or using a vehicle, don't do anything when hitting use key
		return 0;
	}

	if (ent->s.number < MAX_CLIENTS && ent->client && ent->client->ps.m_iVehicleNum)
	{
		gentity_t *currentVeh = &g_entities[ent->client->ps.m_iVehicleNum];
		if (currentVeh->inuse && currentVeh->m_pVehicle)
		{
			return 0;
		}
	}

	SwapClippingBounds( ent->r.currentOrigin );

	VectorCopy(ent->client->ps.origin, viewspot);
	viewspot[2] += ent->client->ps.viewheight;

	VectorCopy( viewspot, src );
	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );

	VectorMA( src, 8192, vf, dest );  // 64 is the normal use distance

	// Trace ahead to find a valid target
	trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, mask );

	
	
	if ( trace.fraction == 1.0f || trace.entityNum < 1 )
	{
		if ( endpoint )
			VectorCopy( dest, endpoint );

		SwapClippingBounds( ent->r.currentOrigin );
		return 0;
	}

	target = &g_entities[trace.entityNum];

	// Ignore triggers for plats and doors as we don't care about these for the moment.
	if ( Q_stricmp( g_entities[trace.entityNum].classname, "noclass" ) == 0 ||
						Q_stricmp( g_entities[trace.entityNum].classname, "trigger_door" ) == 0 )
	{
		mask &= ~CONTENTS_TRIGGER;
		target = AtCrossHairs( ent, mask, endpoint );
	}

	SwapClippingBounds( ent->r.currentOrigin );

	if ( endpoint )
		VectorCopy( trace.endpos, endpoint );

	return target;
}

/*
=================
Cmd_SiegeClass_f
=================
*/
void Cmd_SiegeClass_f( gentity_t *ent )
{
	char className[64];
	int team = 0;
	int preScore;
	qboolean startedAsSpec = qfalse;

	if (g_gametype.integer != GT_SIEGE)
	{ //classes are only valid for this gametype
		return;
	}

	if (!ent->client)
	{
		return;
	}

	if (trap_Argc() < 1)
	{
		return;
	}

	if ( ent->client->switchClassTime > level.time )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSSWITCH")) );
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		startedAsSpec = qtrue;
	}

	trap_Argv( 1, className, sizeof( className ) );

	team = G_TeamForSiegeClass(className);

	if (!team)
	{ //not a valid class name
		return;
	}

	if (ent->client->sess.sessionTeam != team)
	{ //try changing it then
		g_preventTeamBegin = qtrue;
		if (team == TEAM_RED)
		{
			SetTeam(ent, "red");
		}
		else if (team == TEAM_BLUE)
		{
			SetTeam(ent, "blue");
		}
		g_preventTeamBegin = qfalse;

		if (ent->client->sess.sessionTeam != team)
		{ //failed, oh well
			if (ent->client->sess.sessionTeam != TEAM_SPECTATOR ||
				ent->client->sess.siegeDesiredTeam != team)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCLASSTEAM")) );
				return;
			}
		}
	}

	//preserve 'is score
	preScore = ent->client->ps.persistant[PERS_SCORE];

	//Make sure the class is valid for the team
	BG_SiegeCheckClassLegality(team, className);

	//Set the session data
	strcpy(ent->client->sess.siegeClass, className);

	// get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	if (ent->client->tempSpectate < level.time)
	{
		// Kill him (makes sure he loses flags, etc)
		if (ent->health > 0 && !startedAsSpec)
		{
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		}

		if (ent->client->sess.sessionTeam == TEAM_SPECTATOR || startedAsSpec)
		{ //respawn them instantly.
			ClientBegin( ent->s.number, qfalse );
		}
	}
	//set it back after we do all the stuff
	ent->client->ps.persistant[PERS_SCORE] = preScore;

	ent->client->switchClassTime = level.time + 5000;
}

/*
=================
Cmd_ForceChanged_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap_SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap_Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap_Argv( 1, arg, sizeof( arg ) );

		if (arg && arg[0])
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
	char truncSaberName[64];
	int i = 0;

	if (!siegeOverride &&
		g_gametype.integer == GT_SIEGE &&
		ent->client->siegeClass != -1 &&
		(
		 bgSiegeClasses[ent->client->siegeClass].saberStance ||
		 bgSiegeClasses[ent->client->siegeClass].saber1[0] ||
		 bgSiegeClasses[ent->client->siegeClass].saber2[0]
		))
	{ //don't let it be changed if the siege class has forced any saber-related things
        return qfalse;
	}

	while (saberName[i] && i < 64-1)
	{
        truncSaberName[i] = saberName[i];
		i++;
	}
	truncSaberName[i] = 0;

	if ( saberNum == 0 && (Q_stricmp( "none", truncSaberName ) == 0 || Q_stricmp( "remove", truncSaberName ) == 0) )
	{ //can't remove saber 0 like this
        strcpy(truncSaberName, "Kyle");
	}

	//Set the saber with the arg given. If the arg is
	//not a valid sabername defaults will be used.
	WP_SetSaber(ent->s.number, ent->client->saber, saberNum, truncSaberName);

	if (!ent->client->saber[0].model[0])
	{
		assert(0); //should never happen!
		strcpy(ent->client->sess.saberType, "none");
	}
	else
	{
		strcpy(ent->client->sess.saberType, ent->client->saber[0].name);
	}

	if (!ent->client->saber[1].model[0])
	{
		strcpy(ent->client->sess.saber2Type, "none");
	}
	else
	{
		strcpy(ent->client->sess.saber2Type, ent->client->saber[1].name);
	}

	if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
	{
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
	}

	return qtrue;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {\
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg )
{
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	/*
	// no chatting to players in tournements
	if ( (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	*/
	//They've requested I take this out.

	if (g_gametype.integer == GT_SIEGE &&
		ent->client && (ent->client->tempSpectate >= level.time || ent->client->sess.sessionTeam == TEAM_SPECTATOR) &&
		other->client->sess.sessionTeam != TEAM_SPECTATOR &&
		other->client->tempSpectate < level.time)
	{ //siege temp spectators should not communicate to ingame players
		return;
	}

	if (locMsg)
	{
		trap_SendServerCommand( other-g_entities, va("%s \"%s\" \"%s\" \"%c\" \"%s\"", 
			mode == SAY_TEAM ? "ltchat" : "lchat",
			name, locMsg, color, message));
	}
	else
	{
		trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"", 
			mode == SAY_TEAM ? "tchat" : "chat",
			name, Q_COLOR_ESCAPE, color, message));
	}
}

#define EC		"\x19"

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*locMsg = NULL;

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_ALL;
	}

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && g_gametype.integer >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, locMsg );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, name, text, locMsg );
	}
}

void ServerMessage( char *message, char *name )
{
	char		text[MAX_SAY_TEXT];
	gentity_t	*other, *ent = NULL;
	int			color, mode, j;
	char		name2[64];

	if (!g_serverMessages.integer)
		return;

	Q_strncpyz( text, message, sizeof(text) );

	if (name)
		Com_sprintf (name2, sizeof(name2), "%s%c%c"EC" ", name , Q_COLOR_ESCAPE, COLOR_WHITE );
	else 
		Com_sprintf (name2, sizeof(name2), "%s%c%c"EC" ", "^7Automatic Server Message -", Q_COLOR_ESCAPE, COLOR_WHITE );
	mode = SAY_ALL;
	color = COLOR_WHITE;

	for (j = 0; j < level.maxclients; j++) {
		other = &g_entities[j];
		G_SayTo( ent, other, mode, color, name2, text, NULL );
	}
}

/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap_Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap_Argc () < 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	targetNum = atoi( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
	gentity_t *te;
	char arg[MAX_TOKEN_CHARS];
	char *s;
	int i = 0;

	if (g_gametype.integer < GT_TEAM)
	{
		return;
	}

	if (trap_Argc() < 2)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->tempSpectate >= level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
		return;
	}

	trap_Argv(1, arg, sizeof(arg));

	if (arg[0] == '*')
	{ //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
		return;
	}

	s = va("*%s", arg);

	//now, make sure it's a valid sound to be playing like this.. so people can't go around
	//screaming out death sounds or whatever.
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (!bg_customSiegeSoundNames[i])
		{
			break;
		}
		if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //it matches this one, so it's ok
			break;
		}
		i++;
	}

	if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
	{ //didn't find it in the list
		return;
	}

	te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
	te->s.groundEntityNum = ent->s.number;
	te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
	te->r.svFlags |= SVF_BROADCAST;
}


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( gentity_t *ent ) {
	int		player;
	int		order;
	char	str[MAX_TOKEN_CHARS];

	trap_Argv( 1, str, sizeof( str ) );
	player = atoi( str );
	trap_Argv( 2, str, sizeof( str ) );
	order = atoi( str );

	if ( player < 0 || player >= MAX_CLIENTS ) {
		return;
	}
	if ( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) {
		return;
	}
	G_Say( ent, &g_entities[player], SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

void Cmd_mOrigin_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
		trap_SendServerCommand( ent-g_entities, va("print \"Origin: %s\n\"", vtos( ent->r.currentOrigin ) ) );
	else
		trap_SendServerCommand( ent-g_entities, "print \"This command isn't allowed in spectator mode.\n\"");
}

void Cmd_mMark_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	int  nArgs, i;

	nArgs = trap_Argc();


	if ( ent->client->sess.mark[0] == 0 && ent->client->sess.mark[1] == 0 && ent->client->sess.mark[2] == 0 )
	{
		ent->client->sess.makerModFlags &= ~GMOD_GRABBING;
		trap_SendServerCommand( ent-g_entities, va("print \"Automatic Grabbing OFF. Use /mgrabbing to turn it back on.\n\"") );
	}

	if ( nArgs < 2  )
	{
		VectorCopy( ent->r.currentOrigin, ent->client->sess.mark ); 
	}
	else if ( nArgs != 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmark OR mmark x y z\n\"" ) );
	}else 
	{
		for (i = 0; i < 3 ; i++ )
		{
			trap_Argv( i+1, buffer, sizeof( buffer ) );
			ent->client->sess.mark[i] = atof( buffer );
		}
	}

	trap_SendServerCommand( ent-g_entities, va("print \"Marked: %s\n\"", vtos( ent->client->sess.mark ) ) );//vtos( ent->r.currentOrigin ) ) );
}

void Cmd_mUnmark_f( gentity_t *ent ) 
{
	trap_SendServerCommand( ent-g_entities, va("print \"This command has been replaced.\nTo use auto-grabbing type /mgrabbing\n\"") );
}

void Cmd_mMarkfoot_f( gentity_t *ent ) 
{
	if ( ent->client->sess.mark[0] == 0 && ent->client->sess.mark[1] == 0 && ent->client->sess.mark[2] == 0 )
	{
		ent->client->sess.makerModFlags &= ~GMOD_GRABBING;
		trap_SendServerCommand( ent-g_entities, va("print \"Automatic Grabbing OFF. Use /mgrabbing to turn it back on.\n\"") );
	}

	VectorCopy( ent->r.currentOrigin, ent->client->sess.mark ); 

	ent->client->sess.mark[2] -= OFFSET_TO_FEET;

	trap_SendServerCommand( ent-g_entities, va("print \"Marked: %s\n\"", vtos( ent->r.currentOrigin ) ) );
}

void Cmd_mGrabbing_f( gentity_t *ent ) 
{
	if ( ent->client->sess.makerModFlags & GMOD_GRABBING )
	{
		ent->client->sess.makerModFlags &= ~GMOD_GRABBING;
		trap_SendServerCommand( ent-g_entities, va("print \"Automatic Grabbing OFF.\n\"") );
	}
	else
	{
		ent->client->sess.makerModFlags |= GMOD_GRABBING;
		trap_SendServerCommand( ent-g_entities, va("print \"Automatic Grabbing ON.\n\"") );
	}
}

void Cmd_mAllowGive_f( gentity_t *ent ) 
{

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mallowgive\n\""));
		return;
	}

	if ( ent->client->sess.makerModFlags & GMOD_REFUSING )
	{
		ent->client->sess.makerModFlags &= ~GMOD_REFUSING;
		trap_SendServerCommand( ent-g_entities, va("print \"Refusing giveother.\n\"") );
	}
	else
	{
		ent->client->sess.makerModFlags |= GMOD_REFUSING;
		trap_SendServerCommand( ent-g_entities, va("print \"Accepting giveother.\n\"") );
	}
}


void Cmd_mPassword_f( gentity_t *ent ) 
{
	char password[MAX_STRING_CHARS];

	if ( trap_Argc() != 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mpassword <password>\n\""));
		if (ent->client->sess.objectPassword && trap_Argc() == 1)
			trap_SendServerCommand( ent-g_entities, va("print \"Current password: %s\n\"", ent->client->sess.objectPassword));
		return;
	}

	trap_Argv( 1, password, sizeof(password) );

	if ( strlen(password) > MAX_PASSWORD_LENGTH )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Password too long. Password must be %i characters or less.\n\"", (int) MAX_PASSWORD_LENGTH));
		return;
	}

	Q_strncpyz( ent->client->sess.objectPassword, password, sizeof(ent->client->sess.objectPassword) );

	trap_SendServerCommand( ent-g_entities, va("print \"Your password: %s\n\"", ent->client->sess.objectPassword ));

}

void Cmd_mSetPassword_f( gentity_t *ent ) 
{
	gentity_t   *target;
	char password[MAX_STRING_CHARS];
	char buffer[MAX_TOKEN_CHARS];
	int entNum;

	if ( trap_Argc() > 3 || trap_Argc() == 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mSetPassword <password>, or mSetPassword <password> <ID>.\n\t\tmSetPassword <none> to turn off password.\n\""));
		return;
	}

	if ( trap_Argc() == 3)
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		entNum = atoi(buffer);

		if ( entNum < 0 || entNum >= MAX_GENTITIES )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: %i isn't a valid entity/object number.\n\"", entNum));
			return;
		}
		target = &g_entities[entNum];
	}else
	target = SelectedEnt( ent );

	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You have no selected object.\n\""));
		return;
	}

	if ( ((target->creator - 1 != ent-g_entities) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You don't have permission to put password on this.\n\""));
		return;
	}

	trap_Argv( 1, password, sizeof(password) );

	if ( strlen(password) > MAX_PASSWORD_LENGTH )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Password too long. Password must be %i characters or less.\n\"", (int) MAX_PASSWORD_LENGTH));
		return;
	}

	if (!(Q_stricmp (password, "none")))
	{
		target->passworded = 0;
		target->objectPassword[0] = 0;
		return;
	}

	Q_strncpyz( target->objectPassword, password, sizeof(target->objectPassword) );
	target->passworded = 1;

	trap_SendServerCommand( ent-g_entities, va("print \"Password set to: %s\n\"", target->objectPassword ));
}

void Cmd_mSetPasswordT_f( gentity_t *ent ) 
{
	gentity_t   *target;
	vec3_t dummy;
	char password[MAX_STRING_CHARS];

	if ( trap_Argc() > 2 || trap_Argc() == 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mSetPasswordT <password> (Aim at your target).\n\t\tmSetPassword <none> to turn off password.\n\""));
		return;
	}

	target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy );
//|CONTENTS_BODY
	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything to put password on.\n\""));
		return;
	}

	if ( ((target->creator - 1 != ent-g_entities) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything you're allowed to set password on.\n\""));
		return;
	}

	trap_Argv( 1, password, sizeof(password) );

	if ( strlen(password) > MAX_PASSWORD_LENGTH )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Password too long. Password must be %i characters or less.\n\"", (int) MAX_PASSWORD_LENGTH));
		return;
	}

	if (!(Q_stricmp (password, "none")))
	{
		target->passworded = 0;
		target->objectPassword[0] = 0;
		return;
	}

	Q_strncpyz( target->objectPassword, password, sizeof(target->objectPassword) );
	target->passworded = 1;

	trap_SendServerCommand( ent-g_entities, va("print \"Password set to: %s\n\"", target->objectPassword ));
}

void Cmd_mClip_f( gentity_t *ent ) 
{
	gentity_t   *target;
	char buffer[MAX_TOKEN_CHARS];
	int entNum;

	if ( trap_Argc() > 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mClip, or mClip <ID>.\n\""));
		return;
	}

	if ( trap_Argc() == 2)
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		entNum = atoi(buffer);

		if ( entNum < 0 || entNum >= MAX_GENTITIES )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: %i isn't a valid entity/object number.\n\"", entNum));
			return;
		}
		target = &g_entities[entNum];
	}else
	target = SelectedEnt( ent );

	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You have no selected object.\n\""));
		return;
	}

	if ( ((target->creator - 1 != ent-g_entities) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You don't have permission to use mclip on this.\n\""));
		return;
	}

	ent->r.contents = 0;

	trap_AdjustAreaPortalState(ent,qtrue);
}


void Cmd_mdraw_f( gentity_t *ent ) 
{

	if ( !HasPermission(ent,PERMISSION_TESTING) )
		return;

	if ( ent->client->godModFlags & GMOD_DRAWING )
	{
		ent->client->godModFlags &= ~GMOD_DRAWING;
		trap_SendServerCommand( ent-g_entities, va("print \"Drawing Stopped.\n\"") );
	}
	else
	{
		ent->client->nextDrawTime = level.time;
		ent->client->godModFlags |= GMOD_START_DRAWING;
		ent->client->godModFlags |= GMOD_DRAWING;
		trap_SendServerCommand( ent-g_entities, va("print \"Drawing Started.\n\"") );
	}
}

void Cmd_mRotate_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];

	gentity_t	*target;
	vec3_t move;

	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (angles in degrees): mrotate <angle1> <angle2> <angle3>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	//G_FreeEntity( target );

	//trap_UnlinkEntity(target);

	//target->s.pos.trType = TR_INTERPOLATE;


	if ( ent->client->sess.permissions & PERMISSION_TESTING )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%f %f %f\n\"", ent->r.currentAngles[0], ent->r.currentAngles[1], ent->r.currentAngles[2] ));
	}

	//SnapVector( ent->r.currentAngles );

	if ( ent->client->sess.permissions & PERMISSION_TESTING )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%f %f %f\n\"", target->s.apos.trBase[0], target->s.apos.trBase[1], target->s.apos.trBase[2] ));
	}

	//VectorCopy( ent->r.currentAngles, target->s.apos.trBase );

	if ( ent->client->sess.permissions & PERMISSION_TESTING )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%f %f %f\n\"", target->s.apos.trBase[0], target->s.apos.trBase[1], target->s.apos.trBase[2] ));
	}
	

	//VectorCopy( angles, ent->s.apos.trBase );
	target->s.apos.trType = TR_LINEAR_STOP;
	target->s.apos.trDuration = 3 * 1000;  // move for 3secs unless stopped
	target->s.apos.trTime = level.time;

	for (i = 0; i < 3 ; i++ )
	{
		trap_Argv( i+1, buffer, sizeof( buffer ) );
		target->s.apos.trDelta[i] = atof( buffer )/3; // r.currentOrigin [i] += atof( buffer ); //
		move[i] = atof( buffer );
		//VectorCopy( target->r.currentOrigin, target->s.origin );
	}

	//VectorAdd( ent->r.currentAngles, move , target->s.apos.trBase );


	target->nextthink = level.time;
	if ( target->think != Mplace_Think )
		target->oldThink = target->think;
	target->think = Mplace_Think;
	
	if ( target->s.eType != ET_FX && target->s.eType != ET_MOVER )
		target->enemy = 0;
}

void Cmd_mMarkSides_f( gentity_t *ent ) 
{
	//char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	vec3_t position;

	int i;

	if ( !(ent->client->sess.permissions & PERMISSION_MARK_EDGES) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmarksides\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( ent->r.currentAngles[0] != 0 || ent->r.currentAngles[1] != 0 || ent->r.currentAngles[2] != 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot mark edges if an object has been rotated.\n\""));
		return;
	}
	
	// Swap to our actual clipping box, instead of the bug work-around.
	ent->r.maxs[0] = ent->ymaxs;
	ent->r.mins[0] = ent->ymins;

	VectorCopy( target->r.currentOrigin, position );
	
	// Process the components of our position along each axis
	for( i = 0 ; i < 2 ; i++ )
	{
		int component =  ent->r.currentOrigin[i] - position[i];
		if ( component > 0 )
		{
			if ( component > target->r.maxs[i] )
			{
//				for( j = 0 ; j < 3 ; j++ )
//				{
//					target->r.maxs[j] = component;
//					target->r.mins[j] = component * -1;
//				}

				target->r.maxs[i] = component;
				//target->r.mins[i] = component * -1;
				//target->r.mins[i == 0 ? 1 : 0] = component * -1;
				//target->r.maxs[i == 0 ? 1 : 0] = component;
			}
		}
		else
		{
			if ( component < target->r.mins[i] )
			{
				//for( j = 0 ; j < 3 ; j++ )
				//{
				//	target->r.maxs[i] = component * -1;
				//	target->r.mins[i] = component;
				//}
				//target->r.maxs[i] = component * -1;
				target->r.mins[i] = component;
				//target->r.mins[i == 0 ? 1 : 0] = component;
				//target->r.maxs[i == 0 ? 1 : 0] = component * -1;
			}
		}
	}

	ent->ymaxs = ent->r.maxs[0];
	ent->ymins = ent->r.mins[0];

	// prevent glitchy-movement bug
	ent->r.maxs[0] = min( ent->r.maxs[0], ent->r.maxs[1] );
	ent->r.mins[0] = max( ent->r.mins[0], ent->r.mins[1] );

	trap_LinkEntity( target ); 
}

void Cmd_mMarkAllSides_f( gentity_t *ent ) 
{
	//char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	vec3_t position;

	int i, largestSide;

	if ( !(ent->client->sess.permissions & PERMISSION_MARK_EDGES) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmarkallsides\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( ent->r.currentAngles[0] != 0 || ent->r.currentAngles[1] != 0 || ent->r.currentAngles[2] != 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot mark edges if an object has been rotated.\n\""));
		return;
	}
	
	largestSide = 0;

	// Swap to our actual clipping box, instead of the bug work-around.
	ent->r.maxs[0] = ent->ymaxs;
	ent->r.mins[0] = ent->ymins;

	VectorCopy( target->r.currentOrigin, position );

	// Process the components of our position along each axis
	for( i = 0 ; i < 2 ; i++ )
	{
		int component =  ent->r.currentOrigin[i] - position[i];
		if ( component > 0 )
		{
			//if ( component > target->r.maxs[i] )
			//{
//				for( j = 0 ; j < 3 ; j++ )
//				{
//					target->r.maxs[j] = component;
//					target->r.mins[j] = component * -1;
//				}

				target->r.maxs[i] = component;

				if ( largestSide < abs(component) )
					largestSide = abs( component );
				//target->r.mins[i] = component * -1;
				//target->r.mins[i == 0 ? 1 : 0] = component * -1;
				//target->r.maxs[i == 0 ? 1 : 0] = component;
			//}
		}
		else
		{
			//if ( component < target->r.mins[i] )
			//{
				//for( j = 0 ; j < 3 ; j++ )
				//{
				//	target->r.maxs[i] = component * -1;
				//	target->r.mins[i] = component;
				//}
				//target->r.maxs[i] = component * -1;
				target->r.mins[i] = component;

				if ( largestSide < abs(component) )
					largestSide = abs( component );
				//target->r.mins[i == 0 ? 1 : 0] = component;
				//target->r.maxs[i == 0 ? 1 : 0] = component * -1;
			//}
		}
	}

	for ( i = 0 ; i < 2 ; i++)
	{
		target->r.mins[i] = largestSide * -1;
		target->r.maxs[i] = largestSide;
	}

	ent->ymaxs = ent->r.maxs[0];
	ent->ymins = ent->r.mins[0];

	// prevent glitchy-movement bug
	ent->r.maxs[0] = min( ent->r.maxs[0], ent->r.maxs[1] );
	ent->r.mins[0] = max( ent->r.mins[0], ent->r.mins[1] );

	trap_LinkEntity( target ); 
}

void Cmd_mClearEdges_f( gentity_t *ent ) 
{
	//char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;

	if ( !(ent->client->sess.permissions & PERMISSION_MARK_EDGES) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mclearedges\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	VectorClear( target->r.mins );
	VectorClear( target->r.maxs );

	ent->ymaxs = ent->r.maxs[0];
	ent->ymins = ent->r.mins[0];

	trap_LinkEntity( target );
}

void Cmd_mMarktopbottom_f( gentity_t *ent ) 
{
	//char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	vec3_t position;

	int i, component;

	if ( !(ent->client->sess.permissions & PERMISSION_MARK_EDGES) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmarktopbottom\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( target->r.currentAngles[0] != 0 || target->r.currentAngles[1] != 0 || target->r.currentAngles[2] != 0 )
	{
		trap_SendServerCommand( target-g_entities, va("print \"ERROR: Cannot mark edges if an object has been rotated.\n\""));
		return;
	}
	
	// Swap to our actual clipping box, instead of the bug work-around.
	ent->r.maxs[0] = ent->ymaxs;
	ent->r.mins[0] = ent->ymins;

	VectorCopy( target->r.currentOrigin, position );

	i = 2; // vertical component

	component = ent->r.currentOrigin[i] -OFFSET_TO_FEET - position[i];
	if ( component > 0 )
	{
		if ( component > target->r.maxs[i] )
			target->r.maxs[i] = component;
	}
	else
	{
		if ( component < target->r.mins[i] )
			target->r.mins[i] = component;
	}

	ent->ymaxs = ent->r.maxs[0];
	ent->ymins = ent->r.mins[0];

	trap_LinkEntity( target );
}

void Cmd_mSaveEdges_f( gentity_t *ent ) 
{
	gentity_t* target;
	int i, fileStatus;
	char			fileData[MAX_OBINFO_FILE_LENGTH];
	fileHandle_t	f;

	if ( !(ent->client->sess.permissions & PERMISSION_MARK_EDGES) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: msaveedges\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( target->r.currentAngles[0] != 0 || target->r.currentAngles[1] != 0 || target->r.currentAngles[2] != 0 )
	{
		trap_SendServerCommand( target-g_entities, va("print \"ERROR: Cannot save edges if an object has been rotated.\n\""));
		return;
	}

	// Swap to our actual clipping box, instead of the bug work-around.
	ent->r.maxs[0] = ent->ymaxs;
	ent->r.mins[0] = ent->ymins;

	for( i = 0 ; i < MAX_MODEL_SIZES && modelSize[i].model[0] != 0 ; i++ )
	{
		if ( Q_stricmp(target->model2, modelSize[i].model) == 0 )
			break;
	}

	if ( i < MAX_MODEL_SIZES )
	{
		qboolean atEnd;
		atEnd = qfalse;

		if ( *(target->model2) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"UNEXPECTED ERROR: Selected object doesn't have a model name\n\""));
			return;
		}
		
		if ( modelSize[i].model[0] == 0 )
			atEnd = qtrue;

		Q_strncpyz( modelSize[i].model, target->model2, MAX_MODEL_NAME_LENGTH );

		modelSize[i].up       = target->r.maxs[2];
		modelSize[i].down     = target->r.mins[2];
		modelSize[i].xleft    = target->r.mins[0];
		modelSize[i].xright   = target->r.maxs[0];
		modelSize[i].yforward = target->r.maxs[1];
		modelSize[i].yback    = target->r.mins[1];

		i++;
		if ( atEnd && i < MAX_MODEL_SIZES )
			modelSize[i].model[0] = 0; //mark this as the end

		i--;
	}

	fileStatus = trap_FS_FOpenFile( va( "ObInfo/%s", modelSize[i].model ), &f, FS_WRITE);

	if (!f)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Failed to write to file\n\""));
		return;
	}
	
	// The first number is the file version
	// The last number, is for some yet-to-be-discovered need.
	Com_sprintf( fileData, MAX_OBINFO_FILE_LENGTH, "1 %f %f %f %f %f %f 0", modelSize[i].up, modelSize[i].down, modelSize[i].xleft, modelSize[i].xright, modelSize[i].yforward, modelSize[i].yback );

	trap_FS_Write( fileData, min(MAX_OBINFO_FILE_LENGTH, strlen(fileData)), f );

	trap_FS_FCloseFile(f);

	trap_SendServerCommand( ent-g_entities, va("print \"Object edges saved\n\""));

	//trap_UnlinkEntity( target ); // try unlinking first because without it the BBOX on clients wasn't updated properly


	// Update the object so that the changes we've made take effect.
	SetupClippingBox( target );

	trap_UnlinkEntity( target ); // try unlinking first because without it the BBOX on clients wasn't updated properly

	trap_LinkEntity( target );
}

void Cmd_mSaberLength_f( gentity_t *ent ) 
{
	char buffer[MAX_TOKEN_CHARS];

	if ( !(ent->client->sess.permissions & PERMISSION_TESTING) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: msaberlen <length>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof(buffer) );

	G_SetSaber( ent, 0, buffer, qtrue );

	//length = atoi( buffer );

	//for ( n = 0; n < MAX_BLADES; n++ )
	//{
	//	ent->client->saber[0].blade[n].lengthMax = length;
	//}

	//for ( n = 0; n < MAX_BLADES; n++ )
	//{
	//	ent->client->saber[1].blade[n].lengthMax = length;
	//}

	//trap_LinkEntity( ent );
}



void Cmd_mpspecial_f( gentity_t *ent ) 
{
	gentity_t	*target;
	int  nArgs;
	char buffer[MAX_STRING_CHARS];
	char spawnString[MAX_STRING_CHARS];

//	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
//	{
//		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
//		return;
//	}

	if ( !HasPermission( ent, PERMISSION_TESTING ) )
		return;

	nArgs = trap_Argc();

	if ( nArgs < 2  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mset <fieldname> <value> <fieldname> <value>.......\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( level.entCount > (MAX_GENTITIES - g_objectMargin.integer))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of entities\n\""));
		return;
	}

	target = G_Spawn();

	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of entities\n\""));
		return;
	}

	target->creator = ent - g_entities + 1;

	ent->client->manipulating = target-g_entities;

	sprintf( spawnString, "classname %s origin \"%i %i %i\"", buffer, (int)ent->client->sess.mark[0], (int)ent->client->sess.mark[1], (int)ent->client->sess.mark[2] );

	AddSpawnVars( target, spawnString );

	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Object grabbed. Use /mgrabbing to turn off auto-grabbing.\n\"", target-g_entities,vtos(target->s.origin)));
		GrabObject( target, ent );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Entity placed:%i  Origin: %s\n\"", target-g_entities,vtos(target->s.origin)));
		target->grabbing = qfalse;
	}
	//G_TestLine( ent->r.currentOrigin, target->s.origin, 0, 5000 );
}


void Cmd_mPlatform_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	int height, lip, damage, speed;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 ||  trap_Argc() > 5 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mplatform <height> <optional-speed> <optional-lip-height> <optional-damage-when-blocked>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_plat") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This kind of object can't be a platform.\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	height = atoi( buffer );
	
	speed = 200;
	lip = 8;
	damage = 0;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		speed = atoi( buffer );
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof( buffer ) );
		lip = atoi( buffer );
	}

	if ( trap_Argc() > 4 )
	{
		trap_Argv( 4, buffer, sizeof( buffer ) );
		damage = atof( buffer );
	}

	target->s.origin[2] += height;
	VectorCopy( target->s.origin, target->s.pos.trBase );
	VectorCopy( target->s.origin, target->r.currentOrigin );


	trap_LinkEntity( target );

	sprintf( buffer, "classname func_plat height %i speed %i lip %i dmg %i origin \"%i %i %i\"", height,
		speed, lip, damage, (int)target->s.origin[0], (int)target->s.origin[1], (int)target->s.origin[2]);

	AddSpawnVars( target, buffer );
}


void Cmd_mButton_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	int  lip, speed;
	float wait;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 ||  trap_Argc() > 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mbutton <speed> <optional-wait-before-reset> <optional-lip-height>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_button") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Can't make this kind of object a button.\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	speed = atoi( buffer );
	
	speed = 200;
	lip = 4;
	wait = 1;


	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		lip = atoi( buffer );
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof( buffer ) );
		wait = atof( buffer );
	}

	//target->s.origin[2] += height;
	//VectorCopy( target->s.origin, target->s.pos.trBase );
	//VectorCopy( target->s.origin, target->r.currentOrigin );


	trap_LinkEntity( target );

	sprintf( buffer, "classname func_button  speed %i wait %f lip %i", speed, wait, lip );

	AddSpawnVars( target, buffer );
}

void Cmd_mDoor_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	int amplitude, wait, damage, speed;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 ||  trap_Argc() > 6 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mdoor <movement-distance> <optional-speed> <optional-wait> <optional-damage>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_door") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Can't make this kind of object a door.\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	amplitude = atoi( buffer );
	
	wait = 3;
	speed = 400;
	damage = 0;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		speed = atoi( buffer );
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		wait = atoi( buffer );
	}

	if ( trap_Argc() > 5 )
	{
		trap_Argv( 5, buffer, sizeof( buffer ) );
		damage = atoi( buffer );
	}

	sprintf( buffer, "classname func_door height %i speed %i wait %i dmg %i", amplitude, speed, wait, damage );

	AddSpawnVars( target, buffer );
}


void Cmd_mBobbing_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	int amplitude, damage, axis, time;
	float phase;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 ||  trap_Argc() > 6 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mbobbing <amplitude> <optional-bob-time-secs> <optional-axis> <optional-offset-in-cycle> <optional-damage-when-blocked>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_bobbing") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Can't make this kind of object bob.\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	amplitude = atoi( buffer );
	
	time = 4;
	phase = 0;
	damage = 0;
	axis = 0;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		time = atof( buffer );
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof( buffer ) );
		axis = atoi( buffer );
	}

	if ( trap_Argc() > 4 )
	{
		trap_Argv( 4, buffer, sizeof( buffer ) );
		phase = atof( buffer );
	}

	if ( trap_Argc() > 5 )
	{
		trap_Argv( 5, buffer, sizeof( buffer ) );
		damage = atoi( buffer );
	}

	sprintf( buffer, "classname func_bobbing height %i spawnflags %i speed %i phase %f dmg %i", amplitude, axis, time, phase, damage );

	AddSpawnVars( target, buffer );
}

void Cmd_mPendulum_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	int speed, damage, phase;
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mpendulum <swing-angle> <optional-offset-in-cycle> <optional-damage-if-blocked>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_pendulum") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Can't make this kind of object a pendulum.\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	speed = atoi( buffer );
	
	phase = 0;
	damage = 0;


	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		phase = atoi( buffer );
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof( buffer ) );
		damage = atoi( buffer );
	}

	sprintf( buffer, "classname func_pendulum speed %i damage %i phase %i", speed, damage, phase );

	AddSpawnVars( target, buffer );
}

void Cmd_mJumpp_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	int x, y, z;
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 && trap_Argc() !=  4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mjumpp <optional-x-push> <optional-y-push> <optional-z-push>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "trigger_push") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Can't make this kind of object a jump pad.\n\""));
		return;
	}

	if ( trap_Argc() == 4 )
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		x = atoi( buffer );

		trap_Argv( 2, buffer, sizeof( buffer ) );
		y = atoi( buffer );

		trap_Argv( 3, buffer, sizeof( buffer ) );
		z = atoi( buffer );

		sprintf( buffer, "classname trigger_push spawnflags 16 pos3 \"%i %i %i\"", x, y, z );

		AddSpawnVars( target, buffer );
		trap_SendServerCommand( ent-g_entities, va("print \"Your object is now a jump pad.\n\""));

	}
	else
	{
		sprintf( buffer, "classname trigger_push spawnflags 4 pos3 \"%i %i %i\"", target->r.currentOrigin[0], target->r.currentOrigin[1], target->r.currentOrigin[2]+200 );

		AddSpawnVars( target, buffer );

		trap_SendServerCommand( ent-g_entities, va("print \"Your object is now a jump pad.\nSet the direction with /mdest.\nAlternatively, use /mjumpp <x-push> <y-push> <z-push>\""));
	}
}

void Cmd_mLight_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	int brightness, red, green, blue;
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 5 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mlight <brightness> <optional-red> <optional-green> <optional-blue>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	trap_Argv( 1, buffer, sizeof( buffer ) );
	brightness = atoi( buffer );

	red = 1;
	green = 1;
	blue = 1;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		red = atoi( buffer );
		green = 0;
		blue = 0;
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof( buffer ) );
		green = atoi( buffer );
	}

	if ( trap_Argc() > 4 )
	{
		trap_Argv( 4, buffer, sizeof( buffer ) );
		blue = atoi( buffer );
	}

	if ( red < 0 )
		red = 0;

	if ( green < 0 )
		green = 0;

	if ( blue < 0 )
		blue = 0;

	sprintf( buffer, "classname func_static light %i color \"%i %i %i\"", brightness, red, green, blue );

	AddSpawnVars( target, buffer );
}

void Cmd_mkillsw_f( gentity_t *ent ) 
{
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mkillsw\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	AddSpawnVars( target, "classname target_kill" );
}

void Cmd_mTelesw_f( gentity_t *ent ) 
{
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtelesw\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	AddSpawnVars( target, "classname target_teleporter" );

	trap_SendServerCommand( ent-g_entities, "print \"Mark the teleporter destination with /mdest.\n\"" );

}

//switchable printer
void Cmd_mprintsw_f( gentity_t *ent ) 
{
	char		*message;
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mprintsw <message>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	message = ConcatArgs( 1 );

	AddSpawnVars( target, va("classname target_print message \"%s\"", message) );
}

//switchable sound player
void Cmd_msoundsw_f( gentity_t *ent ) 
{
	char		*message;
	gentity_t	*target;
	fileHandle_t f;
	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: msoundsw <sound-name>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "target_speaker") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This kind of object can't be made a speaker.\n\""));
		return;
	}

	message = ConcatArgs( 1 );

	i = trap_FS_FOpenFile( va( "sound/%s.mp3", message), &f, FS_READ);

	if (!f || i <= 0)
		i = trap_FS_FOpenFile( va( "sound/%s.wav", message), &f, FS_READ);

	if (!f || i <= 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Sound unavailable on server. Did you get the name right?\n\""));
		return;
	}

	trap_FS_FCloseFile( f );

	AddSpawnVars( target, va("classname target_speaker noise \"sound/%s\"", message) );
}

//switchable printer
void Cmd_mjumpsw_f( gentity_t *ent ) 
{
	gentity_t	*target;
	vec3_t direction, angles;
	int speed;
	char		buffer[MAX_TOKEN_CHARS];

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 4 && trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mjumpsw <x> <y> <z>\nmjumpsw\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( trap_Argc() == 4 )
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		direction[0] = atof( buffer );

		trap_Argv( 2, buffer, sizeof( buffer ) );
		direction[1] = atof( buffer );

		trap_Argv( 3, buffer, sizeof( buffer ) );
		direction[2] = atof( buffer );

		vectoangles( direction, angles );

		speed = VectorLength( direction );

		sprintf( buffer, "classname target_push angles \"%f %f %f\" speed %i", angles[0], angles[1], angles[2], speed );

		AddSpawnVars( target, buffer );
	}
	else
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		speed = atoi( buffer );

		sprintf( buffer, "classname target_push speed %i", speed );

		AddSpawnVars( target, buffer );

		trap_SendServerCommand( ent-g_entities, va("print \"Set the jump direction with /mdest.\n\""));
	}
}

void Cmd_musable_f( gentity_t *ent ) 
{
	gentity_t	*target;
	qboolean    retainClass;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: musable\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	retainClass = qfalse;

	if ( strcmp(target->classname, "func_static") == 0 || 
			strcmp(target->classname, "func_bobbing") == 0 ||
			strcmp(target->classname, "func_plat") == 0 ||
			strcmp(target->classname, "func_door") == 0 || 
			strcmp(target->classname, "func_button") == 0 ||
			strcmp(target->classname, "func_pendulum") == 0 ||
			strcmp(target->classname, "func_static") == 0 ||
			strcmp(target->classname, "func_breakable") == 0 || 
			strcmp(target->classname, "func_wall") == 0 ||
			strcmp(target->classname, "func_glass") == 0 ||
			strcmp(target->classname, "func_usable") == 0 )
	{
		retainClass = qtrue;
	}

	// didn't use func_usable because the ob disappears after the first use unless you set it to do
	// shader animation.
	AddSpawnVars( target, va("classname %s spawnflags 64", retainClass ? target->classname : "func_static")  );
}

void Cmd_mtouchable_f( gentity_t *ent ) 
{
	gentity_t	*target;
	qboolean    retainClass;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtouchable\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;


	// didn't use func_usable because the ob disappears after the first use unless you set it to do
	// shader animation.
	AddSpawnVars( target, va("classname trigger_multiple wait 3")  );
}

#define MAX_UID_LENGTH 40
char* UID(int seed)
{
	clock_t now; 
	static char uid[MAX_UID_LENGTH+1];
	
	now = clock();

	Rand_Init( level.time );

	// if you change the four letters below, you'll need to change other parts of this
	// file.
	Q_strncpyz( uid, va("%i%iqxhy",now, seed,Q_irand(0, INT_MAX)), MAX_UID_LENGTH+1 ); 

	return uid;
}

void Cmd_mconnectto_f( gentity_t *ent ) 
{
	gentity_t	*from, *obj;
	gentity_t	*to;
	vec3_t		dummy;
	char		buffer[MAX_TOKEN_CHARS];
	int n;

	//if ( !HasPermission(ent, PERMISSION_TESTING) )	
//		return;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2 || trap_Argc() < 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mconnectto, or mconnectto <id>\n\""));
		return;
	}

	from = SelectedEnt( ent );

	if ( from == 0 )
		return;

	if ( trap_Argc() == 2 )
	{
		int entNum;

		trap_Argv( 1, buffer, sizeof( buffer ) );
		entNum = atoi(buffer);
		
		if ( entNum < 0 || entNum >= MAX_GENTITIES )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: %i isn't a valid entity/object number.\n\"", entNum));
			return;
		}

		to = &g_entities[entNum];
	}
	else
	{
		to = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG, dummy );
	}

	if ( !to )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything you're allowed to connect to.\n\""));
		return;
	}
	
	if ( ((to->creator - 1 != ent-g_entities) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)) || to->s.eType == ET_NPC )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything you're allowed to connect to.\n\""));
		return;
	}

	if (to->targetname && from->target && stricmp(to->targetname, from->target) == 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Already connected to this object.\n\""));
		return;
	}

	if (to->target && from->targetname && stricmp(to->target, from->targetname) == 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This object is already connected to your selected one. Loop not allowed.\n\""));
		return;
	}

	if (from == to)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: An object can't connect to itself.\n\""));
		return;
	}


	// Stop any spawner we're connecting to
	if ( to->classname && (stricmp(to->classname, "NPC_spawner") == 0 ||
					stricmp(to->classname, "NPC_Vehicle") == 0) )
	{
		AddSpawnVars( to, "closetarget \"yyryeusgnggewrwrssagfv\" targetname \" \" delay 0 wait 2" );
		to->closetarget = "";
		to->targetname = "";
	}

	//trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( !to->targetname || strlen(to->targetname) == 0 )
	{
		if ( !from->target || strlen(from->target) == 0 )
		{
			AddSpawnVars( from, va("target %s", UID(from - g_entities)) );
		}

		AddSpawnVars( to, va("targetname %s", from->target) );
	}
	else 
	{
		// The thing we're connecting to already has a name
		if( !from->target || strlen(from->target) == 0 )
		{
			// The source of our connection doesn't have a name
			AddSpawnVars( from, va("target %s", to->targetname) );
		}
		else
		{
			// The "source" of our connection also has a name
			// Is the thing we're connecting from still connected to anything else?
			qboolean connected;
			connected = qfalse;

			for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
			{
				obj = &g_entities[n];

				if (!obj->inuse )
						continue;

				if ( obj->creator == ent-g_entities + 1 && (obj->s.eType != ET_NPC) )
				{
					if ( Q_stricmp(from->target, obj->targetname) == 0 )
					{
						connected = qtrue;
						break;
					}
				}	
			} 
			
			if ( !connected )
			{
				AddSpawnVars( from, va("target %s", to->targetname) );
			}
			else
			{
				// The source end of our connection is already connected to something else
				// Is our conectee still connected-to by anything else?
				if ( strstr(to->targetname, "qxhy") != NULL )
				{
					qboolean connectedto;
					connectedto = qfalse;

					// It's one of our automaically generated UID's
					// Check whether anything still refers to it.
					for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
					{
						obj = &g_entities[n];

						if (!obj->inuse )
								continue;

						if ( obj->creator == ent-g_entities + 1 && (obj->s.eType != ET_NPC) )
						{
							if ( Q_stricmp(to->targetname, obj->target) == 0 )
							{
								connectedto = qtrue;
								break;
							}

							if ( Q_stricmp(to->targetname, obj->closetarget) == 0 )
							{
								connectedto = qtrue;
								break;
							}

							if ( Q_stricmp(to->targetname, obj->opentarget) == 0 )
							{
								connectedto = qtrue;
								break;
							}
						}	
					} 

					if ( connectedto )
					{
						trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Both the object you're connecting to and the object\nyou're connecting from already have connections.\nCan't make another connection without affecting some of these.\n\""));
						return;
					}
					else
					{
						// give the thing we're comnnecting to a new name
						AddSpawnVars( to, va("targetname %s", UID(from - g_entities)) );
						AddSpawnVars( from, va("target %s", to->targetname) );
					}
				}
				else
				{
					trap_SendServerCommand( ent-g_entities, va("print \"ERROR: The object you're trying to connect to has been given a name.\nCan't connect to it without losing other connections.\n If you want to connect to this object as well, you must clear its old name.\n\""));
					return;
				}
			}	
		}	
	}

	trap_SendServerCommand( ent-g_entities, "print \"Connection successful.\n\"");
}

void Cmd_mspawner_f( gentity_t *ent ) 
{
	char		name[10];
	char		buffer[MAX_TOKEN_CHARS];
	char		spawnString[MAX_STRING_CHARS];
	char*		classname;
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: ^5mspawner <npc/vehicle> <name>^7\nE.g. ^5/mspawner npc jawa^7\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( stricmp(buffer, "npc") == 0 )
	{
		classname = "NPC_spawner";
		
	}
	else if ( stricmp(buffer, "vehicle") == 0 )
	{
		classname = "NPC_Vehicle";
	}

	trap_Argv( 2, buffer, sizeof( buffer ) );
	
	Q_strncpyz( name, va("zqyq%i", (int)(target-g_entities)) , 10 );

	sprintf( spawnString, "classname %s NPC_type %s targetname %s closetarget %s count -1 delay 10", classname, buffer, name, name );

	G_Printf("sp: %s", spawnString );

	AddSpawnVars( target, spawnString );

	trap_SendServerCommand( ent-g_entities, va("print \"Your object is now a spawner.\nUse /mdest to choose a different spawn position.\n\""));
}

void Cmd_mWrite_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	char* message;
	int red, green, blue;
	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot, cursor, start, end, offset1, offset2;
	vec3_t      up, right, normal;
	int colour, time;

	if ( !HasPermission(ent, PERMISSION_TESTING) )
		return;

	if ( trap_Argc() < 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mwrite <colour> <time-in-secs> <message>\n\""));
		return;
	}

	VectorCopy(ent->client->ps.origin, viewspot);
	viewspot[2] += ent->client->ps.viewheight;

	VectorCopy( viewspot, src );
	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );

	VectorMA( src, 8192, vf, dest );  // 64 is the normal use distance

	// Trace ahead to find a valid target
	trap_Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_TRIGGER);
	
	if ( trace.fraction == 1.0f || trace.entityNum < 1 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"ERROR: Can't see anything to write on.n\"");
		return;
	}
	else
	{
		VectorCopy( trace.endpos, cursor );
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	colour = atoi( buffer );

	trap_Argv( 2, buffer, sizeof( buffer ) );
	time = atoi( buffer );
	time *= 1000; // convert to milliseconds

	//vectoangles( trace.plane.normal, normal );
	 
	 
	//if ( up[2] < 0 )
	//	VectorScale(up, -1, up); // don't allow upside-down  or angled text on walls

	

	if ( trace.plane.normal[2] < VectorLength(trace.plane.normal)*0.9 )
	{
		// we're writing on something wall-like
		// make sure we write on horizontal lines
		VectorSet(up, 0, 0, 1);
		
		CrossProduct( trace.plane.normal, up, right );
	}
	else
	{
		CrossProduct( trace.plane.normal, vf, right );
	}

	CrossProduct( right, trace.plane.normal, up );
	//normal[PITCH] -= 90;
	//normal[YAW] -= 180;
	//AngleVectors( normal, up, right, NULL );

	VectorNormalize( up );
	VectorNormalize( right );

	VectorScale( up, 4, up );
	VectorScale( right, -3, right );

	for( message = ConcatArgs( 3 ) ; message && *message != '\0' ; message++ )
	{
		// Display the lines that make up this character
		if ( *message >= 'a' && *message <= 'z' )
		{
			int i;
			mmodcharacter_t* line;
			line = &(mmodFont[*message - 'a']);

			for( i = 0 ; i < MAX_LINES_PER_CHAR && (*line)[i].x1 > -1 ; i++ )
			{
				VectorScale( right, ((*line)[i].x1), offset1 );
				VectorScale( up, ((*line)[i].y1), offset2 );
				VectorAdd( cursor, offset1, start );
				VectorAdd( start, offset2, start );

				VectorScale( right, ((*line)[i].x2), offset1 );
				VectorScale( up, ((*line)[i].y2), offset2 );
				VectorAdd( cursor, offset1, end );
				VectorAdd( end, offset2, end );
				
				G_TestLine( start, end, colour, time );
			}
		}

		// move cursor to next character
		VectorScale( right, MAX_CHAR_WIDTH + CHAR_SPACING, offset1 );
		VectorAdd( cursor, offset1, cursor ); 
	}	
}


void Cmd_mtelep_f( gentity_t *ent ) 
{
	gentity_t	*target;
	qboolean isfx;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtelep -  turns the currently selected object into a teleporter\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( target->s.eType == ET_FX )
		isfx = qtrue;
	else
		isfx = qfalse;


	AddSpawnVars( target, "classname trigger_teleport pos3 \"0 0 0\"" );

	if (isfx && 0)
	{
		target->s.eType = ET_FX;
		target->think = Mplace_Think;
		target->nextthink = level.time;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"Your object is now a teleporter.\nSet the destination with /mdest.\n\""));

}

void Cmd_mDest_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mdest -  marks the destination for your teleporter\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	sprintf( buffer, "pos3 \"%i %i %i\"", (int) ent->r.currentOrigin[0], (int) ent->r.currentOrigin[1], (int) ent->r.currentOrigin[2]);

	AddSpawnVars( target, buffer );

	trap_SendServerCommand( ent-g_entities, va("print \"Destination Marked: %s\n\"", vtos( ent->r.currentOrigin ) ) );
}

void Cmd_mRotating_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;
	int spinangles[3];
	int damage, i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 4 ||  trap_Argc() > 5 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mrotating <speed1> <speed2> <speed3> <optional-dammage-if-blocked>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	if ( IsConflictingClass(target, "func_rotating") || IsPainOrBreakable(target,0) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This kind of object can't rotate.\n\""));
		return;
	}
	
	damage = 0;

	for( i = 0 ; i < 3 ; i++ )
	{
		trap_Argv( i+1, buffer, sizeof( buffer ) );
		spinangles[i] = atoi( buffer );
	}

	if ( trap_Argc() > 4 )
	{
		trap_Argv( 4, buffer, sizeof( buffer ) );
		damage = atoi( buffer );
	}

	sprintf( buffer, "classname func_rotating spinangles \"%i %i %i\" dmg %i", spinangles[0], spinangles[1], spinangles[2], damage );

	AddSpawnVars( target, buffer );
}

void Cmd_mClasst_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	gentity_t	*target;

	if ( !HasPermission(ent,PERMISSION_TESTING) )
		return;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mclass <class-number>\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;
	
	if ( !target->client )
	{
		trap_SendServerCommand( ent-g_entities, "print \"Not an NPC you can change\n\"");
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	
	target->client->NPC_class = atoi( buffer );
}


void Cmd_mMove_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];

	gentity_t	*target;
	vec3_t		dest;

	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmove x y z\n\""));
		return;
	}

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;

	//G_FreeEntity( target );

	//trap_UnlinkEntity(target);

	//target->s.pos.trType = TR_INTERPOLATE;
	SnapVector( target->r.currentOrigin );

	if (!CanExistHere(target->r.currentOrigin, ENT_FX))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You can't stop movement at this point.\n\""));
		return;
	}


	VectorCopy( target->r.currentOrigin, target->s.pos.trBase );

	target->s.pos.trType = TR_LINEAR_STOP;
	target->s.pos.trDuration = 10*1000;  // move for 10secs unless stopped
	target->s.pos.trTime = level.time;

	for (i = 0; i < 3 ; i++ )
	{
		trap_Argv( i+1, buffer, sizeof( buffer ) );
		target->s.pos.trDelta[i] = atof( buffer ); // r.currentOrigin [i] += atof( buffer ); //
		//VectorCopy( target->r.currentOrigin, target->s.origin );
	}

	VectorScale(target->s.pos.trDelta,10,target->s.pos.trDelta);
	VectorAdd(target->s.origin,target->s.pos.trDelta,dest);
	VectorScale(target->s.pos.trDelta,0.1,target->s.pos.trDelta);

//	trap_SendServerCommand( ent-g_entities, va("print \"Origin: %s \n trDelta: %s \n dest: %s \n\"", vtos(target->s.origin),
//		vtos(target->s.pos.trDelta),vtos(dest)));


	if ( !CanExistHere(dest, ENT_FX) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: The ending point of this mmove is illegal.\n\""));
		VectorClear(target->s.pos.trDelta);
	}

	if ( target->s.pos.trDelta[0] == 0 && target->s.pos.trDelta[1] == 0 && target->s.pos.trDelta[2] == 0 )
		trap_LinkEntity( target ); // (0 0 0) stops all movement. Make sure our clipping area is in the right place.

//	if ( target->s.eType != ET_FX ) //TODO Is this really correct?
//	{
		target->nextthink = level.time;
		if ( target->think != Mplace_Think )
			target->oldThink = target->think;
		target->think = Mplace_Think;
//		target->s.apos.trType = TR_STATIONARY;
		target->enemy = 0;
//	}

	//trap_LinkEntity (target);	 
}

void Cmd_mMoveAll_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];

	gentity_t	*target, *obj;
	vec3_t forward;
	vec3_t		dest;

	int i,n;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 4 && trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmove x y z\n               mmove <speed-in-direction-you're-looking>\n\""));
		return;
	}

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
				continue;

		if ( obj->creator == ent-g_entities + 1 && (obj->s.eType != ET_NPC)  )
		{
	
			target = obj;

			//G_FreeEntity( target );

			//trap_UnlinkEntity(target);

			//target->s.pos.trType = TR_INTERPOLATE;
			SnapVector( target->r.currentOrigin );

			if (!CanExistHere(target->r.currentOrigin, ENT_FX))
				{
					trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You can't stop movement at this point.\n\""));
					return;
				}

			VectorCopy( target->r.currentOrigin, target->s.pos.trBase );

			target->s.pos.trType = TR_LINEAR_STOP;
			target->s.pos.trDuration = 10*1000;  // move for 10secs unless stopped
			target->s.pos.trTime = level.time;

			if ( trap_Argc() == 4 ) 
			{
				for (i = 0; i < 3 ; i++ )
				{
					trap_Argv( i+1, buffer, sizeof( buffer ) );
					target->s.pos.trDelta[i] = atof( buffer ); // r.currentOrigin [i] += atof( buffer ); //
					//VectorCopy( target->r.currentOrigin, target->s.origin );
				}
			}
			else
			{
				float speed;

				AngleVectors(ent->client->ps.viewangles, forward, 0, 0); 

				trap_Argv( 1, buffer, sizeof( buffer ) );
				speed = atof( buffer);

				for (i = 0; i < 3 ; i++ )
				{	
					target->s.pos.trDelta[i] = speed*forward[i]; // r.currentOrigin [i] += atof( buffer ); //
					//VectorCopy( target->r.currentOrigin, target->s.origin );
				}
			}

			VectorScale(target->s.pos.trDelta,10,target->s.pos.trDelta);
			VectorAdd(target->s.origin,target->s.pos.trDelta,dest);
			VectorScale(target->s.pos.trDelta,0.1,target->s.pos.trDelta);

		//	trap_SendServerCommand( ent-g_entities, va("print \"Origin: %s \n trDelta: %s \n dest: %s \n\"", vtos(target->s.origin),
		//		vtos(target->s.pos.trDelta),vtos(dest)));


			if ( !CanExistHere(dest, ENT_FX) )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"ERROR: The ending point of this mmove is illegal.\n\""));
				VectorClear(target->s.pos.trDelta);
			}

			if ( target->s.pos.trDelta[0] == 0 && target->s.pos.trDelta[1] == 0 && target->s.pos.trDelta[2] == 0 )
				trap_LinkEntity( target ); // (0 0 0) stops all movement. Make sure our clipping area is in the right place.

//			if ( target->s.eType != ET_FX )
//			{
				target->nextthink = level.time;
				if ( target->think != Mplace_Think)
					target->oldThink = target->think;
				target->think = Mplace_Think;
				target->enemy = 0;
//			}

		}	
	} 
}

void Cmd_mslay_f( gentity_t *ent )
{
	gentity_t* target;
	qboolean announceSlaying;

	announceSlaying = qfalse;

	if ( trap_Argc() > 1 && !(ent->client->sess.permissions & PERMISSION_SLAY) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mslay\n\""));
		return;
	}
	else if ( trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mslay <optional-client-name-or-number>\n\""));
		return;
	}

	if ( trap_Argc() == 1 )
	{
		target = ent;
	}
	else
	{
		char buffer[MAX_TOKEN_CHARS];
		int targetNum;

		trap_Argv( 1, buffer, sizeof( buffer ) );
		targetNum = G_ClientNumFromNetname(ent - g_entities, buffer);

		if ( targetNum < 0 )
			return;

		target = &g_entities[targetNum];

		announceSlaying = qtrue;
	}

	if (target->inuse && target->client)
	{
		target->flags &= ~FL_GODMODE;
		target->client->ps.stats[STAT_HEALTH] = target->health = -999;
		player_die (target, ent, ent, 100000, MOD_UNKNOWN);
	}


	if ( target->health < 1 )
	{
		DismembermentTest( target );
	}

	if ( announceSlaying )
	{
		trap_SendServerCommand( -1, va("cp \"%s^7 has been slain\n\"", target->client->pers.netname ) );
		G_LogScreenPrintAll();
	}
}

void Cmd_mAttack_f( gentity_t *ent )
{
	gentity_t	*target, *ment;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;

	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_NPC_SPAWN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (while looking at a player/npc) : mattack\n\""));
		return;
	}

	target = AtCrossHairs( ent, MASK_PLAYERSOLID|MASK_NPCSOLID, 0 );
	
	if ( !target )
		return;

//	target = &g_entities[trace.entityNum];

//	if ( (ent->client->sess.permissions < ADMIN_SCALEALL_LEVEL) && ( target->creator != ent - g_entities + 1) )
//		return;

	if ( target->client )
	{
		for ( i = 0; i < ENTITYNUM_MAX_NORMAL; i++ )
		{
			ment = &g_entities[i];
			
			if ( !ment )
				continue;

			if ( (ment->creator == ent-g_entities + 1) && ment->client && ment->NPC && ment != target )
			{
				if ( ment->NPC )
					ment->NPC->goalEntity = target;

				ment->enemy = target;
			}
		}
	}
}

void Cmd_mDefend_f( gentity_t *ent )
{
	gentity_t	*ment, *target;
	vec3_t		dummy;

	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_NPC_SPAWN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (while looking at a player/npc) : mdefend\n\""));
		return;
	}

	if (!ent || !ent->client || ent->health < 1 ||
		(ent->client->ps.pm_flags & PMF_FOLLOW) || ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(ent->client->ps.forceHandExtend != HANDEXTEND_NONE && ent->client->ps.forceHandExtend != HANDEXTEND_DRAGGING))
	{
		return;
	}

	if (ent->client->ps.emplacedIndex)
	{ //on an emplaced gun or using a vehicle, don't do anything when hitting use key
		return;
	}

	if (ent->s.number < MAX_CLIENTS && ent->client && ent->client->ps.m_iVehicleNum)
	{
		gentity_t *currentVeh = &g_entities[ent->client->ps.m_iVehicleNum];
		if (currentVeh->inuse && currentVeh->m_pVehicle)
		{
			return;
		}
	}

	target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy );

//	if ( (ent->client->sess.permissions < ADMIN_SCALEALL_LEVEL) && ( target->creator != ent - g_entities + 1) )
//		return;

	if ( target->client )
	{
		for ( i = 0; i < ENTITYNUM_MAX_NORMAL; i++ )
		{
			ment = &g_entities[i];
			
			if ( !ment )
				continue;

			if ( (ment->creator == ent-g_entities + 1) && ment->client && ment != target )
			{
				ment->NPC->defendEnt = target;
			}
		}
	}
}

void Cmd_mFollow_f( gentity_t *ent )
{
	gentity_t	*ment, *target;
	vec3_t		dummy;

	int i;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_NPC_SPAWN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (while looking at a player/npc) : mdefend\n\""));
		return;
	}

	if (!ent || !ent->client || ent->health < 1 ||
		(ent->client->ps.pm_flags & PMF_FOLLOW) || ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(ent->client->ps.forceHandExtend != HANDEXTEND_NONE && ent->client->ps.forceHandExtend != HANDEXTEND_DRAGGING))
	{
		return;
	}

	if (ent->client->ps.emplacedIndex)
	{ //on an emplaced gun or using a vehicle, don't do anything when hitting use key
		return;
	}

	if (ent->s.number < MAX_CLIENTS && ent->client && ent->client->ps.m_iVehicleNum)
	{
		gentity_t *currentVeh = &g_entities[ent->client->ps.m_iVehicleNum];
		if (currentVeh->inuse && currentVeh->m_pVehicle)
		{
			return;
		}
	}

	target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy );

//	if ( (ent->client->sess.permissions < ADMIN_SCALEALL_LEVEL) && ( target->creator != ent - g_entities + 1) )
//		return;

	if (!target)
		return;

	if ( target->client )
	{
		for ( i = 0; i < ENTITYNUM_MAX_NORMAL; i++ )
		{
			ment = &g_entities[i];
			
			if ( !ment )
				continue;

			if ( (ment->creator == ent-g_entities + 1) && ment->client && ment != target )
			{
				ment->NPC->goalEntity = target;
			}
		}
	}
}


void Cmd_mstatus_f( gentity_t *ent )
{
	gclient_t* cl;
	int i;
	char buffer[MAX_TOKEN_CHARS];

	if ( !HasPermission(ent, PERMISSION_STATUS) )
		return;

	if ( trap_Argc() > 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mstatus\n\"") );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"  Name                 UserName        IP                       NPCs\n\"") );

	for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
	{
		if ( cl->pers.connected != CON_CONNECTED ) 
			continue;

		SanitizeString2( cl->pers.netname, buffer );

	//	if (cl->npcCount == NULL)
	//		cl->npcCount = 0;

		trap_SendServerCommand( ent-g_entities, va("print \"%i %-20.20s ^7%-15.15s %-26.26s %i \n\"", i, buffer, cl->sess.username, cl->sess.ip, cl->npcCount) );
	}

	trap_SendServerCommand( ent-g_entities, va("print \"\nFree Object Slots: %i  Free NPC Slots: %i  NPCs: %i NPC models used: %i\n\"", MAX_GENTITIES - (level.entCount + g_objectMargin.integer),(g_npcLimit.integer-level.npcCount), level.npcCount, level.npcTypes) );
}

void Cmd_mkick_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	int clientNum;

	if ( !HasPermission(ent, PERMISSION_KICK) )
		return;

	if ( trap_Argc() < 2 || trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mkick <client-name-or-number>\n\"") );// <optional-reason>\n\"") );
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	clientNum = G_ClientNumFromNetname( ent-g_entities, buffer );

	if ( clientNum == -1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Could not identify player %s\n\"", buffer) );
		return;	
	}

	G_Printf( "ADMIN KICK: Admin: %s  Kicked Client: %i    %s\n",ent->client->sess.username, clientNum, level.clients[clientNum].sess.ip); 

	//strcpy(netname, cl->pers.netname);
	//Q_CleanStr(netname);
	trap_SendConsoleCommand( EXEC_INSERT, va("clientkick \"%i\"\n", clientNum) );
}

void Cmd_mlistbans_f( gentity_t *ent )
{
	if ( !HasPermission(ent, PERMISSION_BAN) )
		return;

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mlistbans\n\"") );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"Banned IP's: %s\n\"", g_banIPs.string) );
}

//Ban IP address but don't kick
void Cmd_mban_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];

	if ( !HasPermission(ent, PERMISSION_BAN) )
		return;

	if ( trap_Argc() < 2 || trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mban <ip-address-or-entire-subnet>\n\"") );// <optional-reason>\n\"") );
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	G_Printf( "ADMIN BAN: Admin User: %s  Banned IP: %s\n",ent->client->sess.username, buffer); 

	trap_SendConsoleCommand( EXEC_INSERT, va("addip \"%s\"\n", buffer) );

	trap_SendServerCommand( ent-g_entities, va("print \"Banned.\n\"") );
}

void Cmd_munban_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];

	if ( !HasPermission(ent, PERMISSION_BAN) )
		return;

	if ( trap_Argc() < 2 || trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: munban <ip-address-or-entire-subnet>\n\"") );
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );


	G_Printf( "ADMIN UNBAN: Admin User: %s  Unbanned IP: %s\n",ent->client->sess.username, buffer); 

	trap_SendConsoleCommand( EXEC_INSERT, va("removeip \"%s\"\n", buffer) );

	trap_SendServerCommand( ent-g_entities, va("print \"Unbanned.\n\"") );

}

void Cmd_mslap_f( gentity_t *ent )
{
	gentity_t* target;
	qboolean announceSlap;

	announceSlap = qfalse;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_SLAP) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 1 && !(ent->client->sess.permissions & PERMISSION_SLAP) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mslap\n\""));
		return;
	}
	else if ( trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mslap <optional-client-name-or-number>\n\""));
		return;
	}

	if ( trap_Argc() == 1 )
	{
		target = ent;
	}
	else
	{
		char buffer[MAX_TOKEN_CHARS];
		int targetNum;

		trap_Argv( 1, buffer, sizeof( buffer ) );
		targetNum = G_ClientNumFromNetname(ent - g_entities, buffer);

		if ( targetNum < 0 )
			return;

		target = &g_entities[targetNum];

		announceSlap = qtrue;
	}

	if (BG_KnockDownable(&target->client->ps))
	{
		target->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
		target->client->ps.forceDodgeAnim = 0;
		if (trap_Argc() > 1)
		{
			target->client->ps.forceHandExtendTime = level.time + 1100;
			target->client->ps.quickerGetup = qfalse;
		}
		else
		{
			target->client->ps.forceHandExtendTime = level.time + 700;
			target->client->ps.quickerGetup = qtrue;
		}
	}

	G_Damage( target, ent, ent, NULL, NULL, max(ent->client->ps.stats[STAT_HEALTH], 20), DAMAGE_NO_PROTECTION|DAMAGE_NO_KNOCKBACK, MOD_UNKNOWN ); 

	if ( announceSlap )
	{
		trap_SendServerCommand( -1, va("cp \"%s ^7has been slapped\n\"", target->client->pers.netname ) );
		G_LogScreenPrintAll();
	}
}

#define MAX_OBKEY_START_POS 25
#define MAX_OBKEY_POS 30

void SaveObs( gentity_t *ent, char* directory, char* file, char* enteredPass  )
{
	char		filename[MAX_TOKEN_CHARS];
	char		password[MAX_TOKEN_CHARS];
	char*		encodingKey = "Object Data file %s saved successfully.\n";
	fileHandle_t f;
	gentity_t	*ment; 
	int i, j, savedCount, encodingKeyPos, length;
	char *data, *dataPtr, *spawnFields;

	if ( !ent )
		return;

	savedCount = 0;

	Com_sprintf( filename, MAX_TOKEN_CHARS, "%s\\%s", directory, file );

	data = malloc(128000);

	dataPtr = data;

	if ( enteredPass )
	{
		length = trap_FS_FOpenFile(filename, &f, FS_READ);

		// if the file already exists, check we've got the right password.
		if ( f != 0 )
		{		
			trap_FS_Read( data, min(length, 32), f ); 

			data[min(length, 32)] = '\0';

			dataPtr = data;

			if ( *dataPtr == '5' )
			{
				dataPtr++;
				
				encodingKeyPos = *dataPtr - 'b';

				dataPtr++;

				// Versions 5 and above are encrypted to protect people's passwords and buildings
				for ( i = 2 ; i < min(length, 33) ; i++, encodingKeyPos++ )
				{
					if ( encodingKeyPos > MAX_OBKEY_POS ) 
						encodingKeyPos = 0;

					// very simple and crackable encoding - just EOR our file with the key.
					data[i] ^= encodingKey[encodingKeyPos];
				}
			}
			else
			{
				// Skip the space before the password in file versions before 5
				dataPtr++;
				dataPtr++;

			}

			if ( (sscanf(dataPtr, "%s ", password)== 1) && (strcmp(password, enteredPass) != 0) )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"Failed to write file.\nPassword doesn't match the file that already exists with this name.\nIf this is the first time you've saved to this file,\nsomeone has already taken the name. Try another name.\n\""));
				//BG_TempFree( 128000 );
				free( data );
				return;
			}

			trap_FS_FCloseFile( f );
		}
	}

	trap_FS_FOpenFile(filename, &f, FS_WRITE);

	if (!f)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Unable to open file for writing\n\""));
		free( data );
		//BG_TempFree( 128000 );
		return;
	}

	// Version 1 save code
	//if ( ent->client->sess.permissions < ADMIN_SAVEOB_LEVEL )
	//{
	//	Com_sprintf( data, 128000, "1 %s ", buffer );

	//	for ( i = 0; (dataPtr - data < 127000) && (i < ENTITYNUM_MAX_NORMAL) ; i++ )	
	//	{	
	//		ment = &g_entities[i];
	//		if ( (ment->inuse) && (ment->creator == ent-g_entities +1) && (strcmp( ment->classname, "mplaced") == 0) )
	//		{
	//			dataPtr += strlen( dataPtr );
	//			
	//			if ( (ment->s.eType == ET_MOVER) )
	//			{
	//				Com_sprintf( dataPtr, 127800 - (dataPtr - data), "%s %f %f %f %f %f %f ", ment->model, ment->r.currentOrigin[0],ment->r.currentOrigin[1],ment->r.currentOrigin[2], ment->r.currentAngles[0],ment->r.currentAngles[1],ment->r.currentAngles[2] );
	//			}
	//			else if (ment->s.eType == ET_FX)
	//			{
	//				Com_sprintf( dataPtr, 127800 - (dataPtr - data), "%s %f %f %f %f %f %f %i %f %i %i ", ment->target6, ment->r.currentOrigin[0],ment->r.currentOrigin[1],ment->r.currentOrigin[2], 
	//																							ment->r.currentAngles[0],ment->r.currentAngles[1],ment->r.currentAngles[2], ment->delay, ment->random, ment->splashDamage, ment->splashRadius );
	//			}

	//			savedCount++;
	//		}
	//	}
	//}
	//else
	//{

		encodingKeyPos = rand() % (MAX_OBKEY_START_POS + 1);

		Com_sprintf( data, 128000, "5%c%s ",'b' + encodingKeyPos, enteredPass );

		for ( i = 0; (dataPtr - data < 127000) && (i < ENTITYNUM_MAX_NORMAL) ; i++ )	
		{	
			ment = &g_entities[i];
			if ( (ment->inuse) && (ment->creator == ent-g_entities +1) && ment->s.eType != ET_NPC  )
			{
				dataPtr += strlen( dataPtr );
	
				if ( ment->spawnFields == 0 )
					spawnFields = " ";
				else
					spawnFields = ment->spawnFields;

				if ( (ment->s.eType == ET_MOVER) && ment->takedamage == qfalse )
				{
					Com_sprintf( dataPtr, 127800 - (dataPtr - data), "%s %.0f %.0f %.0f %.0f %.0f %.0f %i %i %s}", ment->model2, ment->s.pos.trBase[0],ment->s.pos.trBase[1],ment->s.pos.trBase[2], ment->s.apos.trBase[0],ment->s.apos.trBase[1],ment->s.apos.trBase[2],
						ment->s.iModelScale, ment->takedamage, spawnFields );
				}
				else if ( (ment->s.eType == ET_MOVER) && ment->takedamage == qtrue )
				{
					Com_sprintf( dataPtr, 127800 - (dataPtr - data), "%s %.0f %.0f %.0f %.0f %.0f %.0f %i %i %i %i %.0f %s}", ment->model2, ment->s.pos.trBase[0],ment->s.pos.trBase[1],ment->s.pos.trBase[2], ment->s.apos.trBase[0],ment->s.apos.trBase[1],ment->s.apos.trBase[2],
						ment->s.iModelScale, ment->takedamage, ment->health, ment->material, ment->radius, spawnFields );
				}
				else if (ment->s.eType == ET_FX)
				{
					Com_sprintf( dataPtr, 127800 - (dataPtr - data), "%s %.0f %.0f %.0f %.0f %.0f %.0f %i %.0f %i %i %s}", ment->target6, ment->s.pos.trBase[0],ment->s.pos.trBase[1],ment->s.pos.trBase[2], ment->s.apos.trBase[0],ment->s.apos.trBase[1],ment->s.apos.trBase[2],
						ment->delay, ment->random, ment->splashDamage, ment->splashRadius, spawnFields );
				}
				else
				{
					if ( spawnFields[1] == '\0' )
						continue;

					Com_sprintf( dataPtr, 127800 - (dataPtr - data), "{%s}", spawnFields );
				}

				savedCount++;
			}
		}
	//}
	//}
	
	length = strlen(data);
	
	// Versions 5 and above are encrypted to protect people's passwords and work
	for ( j = 2 ; j < length ; j++, encodingKeyPos++ )
	{
		if ( encodingKeyPos > MAX_OBKEY_POS ) 
			encodingKeyPos = 0;

		// very simple and crackable encoding - just EOR our file with the key.
		data[j] ^= encodingKey[encodingKeyPos];
	}

	data[length] = 0;

	trap_SendServerCommand( ent-g_entities, va("print \"%i objects saved\n\"", savedCount));

	trap_FS_Write(data, length, f);
	 
	trap_FS_FCloseFile( f );

	free( data );
	//BG_TempFree( 128000 );
}

void Cmd_mSaveobs_f( gentity_t *ent )
{
	char	buffer[MAX_TOKEN_CHARS];
	char	password[MAX_TOKEN_CHARS];
	gentity_t	*obj;
	int		n, obCount;

	//if ( ent->client->sess.permissions < ADMIN_SAVEOB_LEVEL )
	//{
	//	trap_SendServerCommand( ent-g_entities, va("print \"This command is disabled for testing. It should be back soon.\n\""));
	//	return;
	//}

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_LOAD_SAVE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc()!= 3 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: msaveobs <filename> <password>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( strchr(buffer, '/') || strchr(buffer, '.') || strchr(buffer, '\\'))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO SAVE: Illegal punctuation characters in filename.\n Try removing the punctuation.\n\""));
		return;
	}

	if ( strlen(buffer) > 25 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO SAVE: Filename too long.\n\""));
		return;
	}


	// Don't save if we don't have any obs to save (prevents accidental overwrites)
	obCount = 0;

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
				continue;

		if ( obj->creator == ent-g_entities + 1 && (obj->s.eType != ET_NPC) )
			obCount++;
	} 

	if ( obCount < 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO SAVE: No objects to save.\n\""));
		return;
	}

	//get password
	trap_Argv( 2, password, sizeof( password ) );

	SaveObs( ent, "SavedObjects", buffer, password );
}


void Cmd_mSaveMapobs_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	vmCvar_t	mapname;

	//if ( ent->client->sess.permissions < ADMIN_SAVEOB_LEVEL )
	//{
	//	trap_SendServerCommand( ent-g_entities, va("print \"This command is disabled for testing. It should be back soon.\n\""));
	//	return;
	//}

	if ( !(ent->client->sess.permissions & PERMISSION_SAVE_MAPOBS) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc()!= 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: msavemapobs\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( strchr(buffer, '.') )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO SAVE: Illegal punctuation characters in filename.\n\""));
		return;
	}

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	SaveObs( ent, "StartupObjects", mapname.string, 0 );
}

// enteredPass == 0  => no password checking
void LoadObs( gentity_t *ent, char* directory, char* file, char* enteredPass )
{
	char		buffer[MAX_TOKEN_CHARS];
	char		password[MAX_TOKEN_CHARS];
	char		filename[MAX_TOKEN_CHARS];
	char		spawnFields[MAX_SPAWN_VARS_LENGTH];
	char*		encodingKey = "Object Data file %s saved successfully.\n";
	fileHandle_t f;
	gentity_t	*ment; 
	int i, numArguments, encodingKeyPos, length, temp = 0;
	char* data, *dataPtr; 

	Com_sprintf( filename, MAX_TOKEN_CHARS, "%s\\%s",directory, file );

	length = trap_FS_FOpenFile(filename, &f, FS_READ);

	// if the file already exists, check we've got the right password.
	if ( !f )
	{
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: File not found\n\""));
		else
			G_Printf( "FAILED TO LOAD: File not found\n" );

		return;
	}


	if ( length > 127500 )
	{
		// The file's too large for our buffer.

		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: File too large\n\""));
		else
			G_Printf( "FAILED TO LOAD: File too large\n" );

		trap_FS_FCloseFile( f );
		return;
	}

	data = malloc(128000);

	dataPtr = data;

	trap_FS_Read( data, min(length, 127500), f ); 

	if ( *dataPtr == '5' )
	{
		dataPtr++;
		
		encodingKeyPos = *dataPtr - 'b';

		dataPtr++;

		// Versions 5 and above are encrypted to protect people's passwords and work
		for ( i = 2 ; i < length + 1 ; i++, encodingKeyPos++ )
		{
			if ( encodingKeyPos > MAX_OBKEY_POS ) 
				encodingKeyPos = 0;

			// very simple and crackable encoding - just EOR our file with the key.
			data[i] ^= encodingKey[encodingKeyPos];
		}
	}
	else
	{
		// Skip the space before the password in file versions before 5
		dataPtr++;
		dataPtr++;
	}

	data[min(length, 127500)] = '\0';

	if ( (sscanf(dataPtr, "%s ", password)== 1) && (enteredPass != 0) && (strcmp(password, enteredPass) != 0) )
	{
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"Failed to load file - incorrect password.\n\""));
		else
			G_Printf( "Failed to load file. Password does not match.\n" );

		trap_FS_FCloseFile( f );
		free(data);
		return;
	}

	dataPtr = strchr( dataPtr, ' ' );
	dataPtr++;

	// file version 1
	if ( *data == '1' )
	{
		for ( i = 0; (dataPtr - data < 127000) && (i < ENTITYNUM_MAX_NORMAL) ; i++ )	
		{
			int j, size;
			size = 32;

			if ( level.entCount > MAX_GENTITIES - g_objectMargin.integer )
			{
				if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : There are already too many objects.\n\""));
				else
					G_Printf( "LOADING INCOMPLETE : There are already too many objects.\n" );

				return;
			}
			
			if ( level.entCount > (MAX_GENTITIES - g_objectMargin.integer))
			{
				if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : The game has run out of entities.\n\""));
				else
					G_Printf( "LOADING INCOMPLETE : The game has run out of entities.\n" );

				free( data );
				trap_FS_FCloseFile( f );
				return;
			}

			ment = G_Spawn();

			if ( ment == 0 )
			{
				if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : The game has run out of entities.\n\""));
				else
					G_Printf( "LOADING INCOMPLETE : The game has run out of entities.\n" );

				free( data );
				G_FreeEntity( ment );
				trap_FS_FCloseFile( f );
				return;
			}



			numArguments = sscanf( dataPtr, "%s %f %f %f %f %f %f ", buffer, &(ment->r.currentOrigin[0]),&(ment->r.currentOrigin[1]),&(ment->r.currentOrigin[2]), &(ment->r.currentAngles[0]),&(ment->r.currentAngles[1]),&(ment->r.currentAngles[2]));

			if ( numArguments != 7 )
			{
				if ( (numArguments != 0) && (numArguments != EOF) )
				{
					if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid entity description.\n\""));
					else
						G_Printf( "LOADING INCOMPLETE : Stopped at an invalid entity description.\n" );
				}
				else if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
				else
					G_Printf( "Loaded %i objects.\n", i );

				G_FreeEntity( ment );
				trap_FS_FCloseFile( f );
				free( data );
				return;
			}

			for(j = 0 ; j < 7 ; j++ )
			{
				dataPtr = strchr( dataPtr, ' ' );
				if (dataPtr == 0)
				{
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
					else
						G_Printf( "Loaded %i objects.\n", j );

					free( data );
					return;
				}

				dataPtr++;
			}

			if ( buffer[strlen(buffer) - 4] == '.' ) // the dot in .md3
			{
				ment->model = EMPTY_MODEL;
				//ment->spawnflags |= 4; //crusher
				//ment->model2 = G_NewString( buffer );
				//ment->model2 = G_NewString2( buffer );//, ment-g_entities
				ment->model2 = G_NewStringForEnt (buffer, ment->s.number);
				ment->s.eType = ET_MOVER;
			}
			else
			{
				//ment->target6 = G_NewString2( buffer );//, ment-g_entities
				ment->target6 = G_NewStringForEnt (buffer, ment->s.number);
				ment->s.eType = ET_FX;

				if ( sscanf(dataPtr, "%i %f %i %i ", &ment->delay, &ment->random, &ment->splashDamage, &ment->splashRadius) != 4 )
				{
					if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid effect.\n\""));
					else
					{
						G_Printf( "LOADING INCOMPLETE : Stopped at an invalid effect.\n" );
						//trap_TrueFree((void **)&AllocPointerEfx);
					}
					//G_Printf("PRE- LoadObs1 %s", AllocPoints[ment-g_entities].AllocPointer);
					//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
					//G_Printf("POST- LoadObs1 %s", AllocPoints[ment-g_entities].AllocPointer);
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					free( data );
					return;
				}

				for(j = 0 ; j < 4 ; j++ )
				{
					dataPtr = strchr( dataPtr, ' ' );
					if (dataPtr == 0)
					{
						//G_Printf("PRE- LoadObs2 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs2 %s", AllocPoints[ment-g_entities].AllocPointer);
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
						else
							G_Printf( "Loaded %i objects.\n", i );
						free( data );
						return;
					}

					dataPtr++;
				}
			}
/*
			dataPtr += nextSpace + 1;

			ment->model2 = G_NewString( dataPtr );

			// Get our origin.
			for( j = 0 ; j < 3 ; j++ )
			{
				dataPtr = nextSpace + 1;
				nextSpace = strchr( dataPtr );
				if (nextSpace == 0)
				{
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					return;
				}
				*nextSpace = '\0';

				ment->r.currentOrigin[j] = atof( dataPtr );
			}
			
			// Get our angles
			for( j = 0 ; j < 3 ; j++ )
			{
				dataPtr = nextSpace + 1;
				nextSpace = strchr( dataPtr );
				if (nextSpace == 0)
				{
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					return;
				}
				*nextSpace = '\0';

				ment->r.currentAngles[j] = atof( dataPtr );
			} */

			ment->classname = "mplaced";
			ment->creator = ent ? ent - g_entities + 1 : SERVER_CREATOR;
			VectorCopy( ment->r.currentOrigin, ment->s.origin );
			VectorCopy( ment->s.origin, ment->s.pos.trBase );

			if ( ment->s.eType == ET_MOVER )
			{
				ment->s.modelindex = G_ModelIndex( ment->model );
				ment->s.modelindex2 = G_ModelIndex( ment->model2 );
				if ( !ent )
				{
					ment->s.eFlags = EF_PERMANENT;
					trap_SetBrushModel( ment, ment->model2);
					G_BSPIndex(ment->model2);
					level.mNumBSPInstances++;
				}
				else
				{
					// These two lines used to be applied to effects too.
					VectorSet (ment->r.mins, -1*size, -1*size, -1*size);
					VectorSet (ment->r.maxs, size, size, size);
					
					ment->ymins = ment->r.mins[0];
					ment->ymaxs = ment->r.maxs[0];

					ment->r.contents = CONTENTS_SOLID;
					ment->s.solid = SOLID_BBOX;
				}
				
				SetupClippingBox(ment);
			}
			else if ( ment->s.eType == ET_FX )
			{
				ment->s.modelindex = G_EffectIndex( ment->target6 );
				if (ment->s.modelindex > (MAX_FX-10))
				{
					if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"Max different effects hit.\n\""));
						else
							G_Printf( "Max different effects hit.\n", i );
					free( data );
					//G_Printf("PRE- LoadObs3 %s", AllocPoints[ment-g_entities].AllocPointer);
					//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
					//G_Printf("POST- LoadObs3 %s", AllocPoints[ment-g_entities].AllocPointer);
					RemoveConfig(ment->target6, CS_EFFECTS, MAX_FX, 0);
					G_FreeEntity( ment );
					return;
				}
				ment->r.contents = CONTENTS_FOG;
				ment->s.speed = ment->delay;
				ment->s.time = ment->random;
				ment->s.modelindex2 = FX_STATE_OFF;
				ment->think = fx_runner_think; 
				ment->nextthink = level.time + 200;

				if ( ment->splashDamage > 0 )
					ment->spawnflags |= 4;

				size = 32;
				VectorSet (ment->r.mins, -1*size, -1*size, -1*size);
				VectorSet (ment->r.maxs, size, size, size);
					
				ment->ymins = ment->r.mins[0];
				ment->ymaxs = ment->r.maxs[0];

				ment->r.contents = CONTENTS_FOG;
				AddUse( ment,  fx_runner_use );
				//trap_TrueFree((void **)&AllocPointerEfx);
			}
			

			ment->clipmask = 0;
			ment->s.pos.trType = TR_STATIONARY;
			ment->s.pos.trTime = 0;
			ment->s.pos.trDuration = 0;
			
			VectorClear( ment->s.pos.trDelta );
			VectorCopy( ment->r.currentAngles, ment->s.angles );
			VectorCopy( ment->s.angles, ment->s.apos.trBase );
			VectorSet(ment->modelScale, 1, 1, 1);
			
			if ( ment->s.eType == ET_FX )
			{
				G_SetOrigin( ment, ment->s.origin );
				G_SetAngles( ment, ment->s.angles );
			}

			trap_LinkEntity (ment);
		}
	}
	else if ( *data == '2' || *data == '3' || *data == '4' || *data == '5' ) // fileversions 2, 3 4 and 5
	{
		int fileVer;
		
		fileVer = *data - '0';

		for ( i = 0; (dataPtr - data < 127000) && (i < ENTITYNUM_MAX_NORMAL) ; i++ )	
		{	
			int j, size;
			size = 32;

			if ( level.entCount > MAX_GENTITIES - g_objectMargin.integer )
			{
				if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : There are already too many objects.\n\""));
				else
					G_Printf( "LOADING INCOMPLETE : There are already too many objects.\n" );

				return;
			}

			if ( level.entCount > (MAX_GENTITIES - g_objectMargin.integer))
			{
				if ( ent )
					trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : The game has run out of entities.\n\""));
				else
					G_Printf( "LOADING INCOMPLETE : The game has run out of entities.\n" );

				free( data );
				trap_FS_FCloseFile( f );
				return;
			}

			ment = G_Spawn();
			if ( *dataPtr != '{' )
			{
				if ( ment == 0 )
				{
					if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : The game has run out of entities.\n\""));
					else
						G_Printf( "LOADING INCOMPLETE : The game has run out of entities.\n" );

					free( data );
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					return;
				}

				numArguments = sscanf( dataPtr, "%s %f %f %f %f %f %f ", buffer, &(ment->r.currentOrigin[0]),&(ment->r.currentOrigin[1]),&(ment->r.currentOrigin[2]), &(ment->r.currentAngles[0]),&(ment->r.currentAngles[1]),&(ment->r.currentAngles[2]) );

				if ( numArguments != 7 )
				{
					if ( (numArguments != 0) && (numArguments != EOF) )
					{
						if ( ent != 0 )
							trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid entity description.\n\""));
						else
							G_Printf( "LOADING INCOMPLETE : Stopped at an invalid entity description.\n" );
					}
					else if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
					else
						G_Printf( "Loaded %i objects.\n", i );

					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					free( data );
					return;
				}

				for(j = 0 ; j < 7 ; j++ )
				{
					dataPtr = strchr( dataPtr, ' ' );
					if (dataPtr == 0)
					{
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
						else
							G_Printf( "Loaded %i objects.\n", j );

						free( data );
						return;
					}

					dataPtr++;
				}

				if ( buffer[strlen(buffer) - 4] == '.' ) // the dot in .md3
				{
					ment->model = EMPTY_MODEL;
					//ment->spawnflags |= 4;
					//ment->model2 = G_NewString2( buffer );//, ment-g_entities
					ment->model2 = G_NewStringForEnt (buffer, ment->s.number);
					ment->s.eType = ET_MOVER;
					
					if ( fileVer >= 3 )
					{
						// Version 3 includes model scaling
						ment->s.iModelScale = 100;

						if ( sscanf(dataPtr, "%i ", &ment->s.iModelScale) != 1 )
						{
							//G_Printf("PRE- LoadObs4 %s", AllocPoints[ment-g_entities].AllocPointer);
							//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
							//G_Printf("POST- LoadObs4 %s", AllocPoints[ment-g_entities].AllocPointer);
							G_FreeEntity( ment );
							trap_FS_FCloseFile( f );
							if ( ent )
								trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid object.\n\""));
							else
							{
								G_Printf( "LOADING INCOMPLETE : Stopped at an invalid object.\n" );
								//trap_TrueFree((void **)&AllocPointerObj);
							}
							free( data );
							return;
						}

						dataPtr = strchr( dataPtr, ' ' );
						if (dataPtr == 0)
						{
							//G_Printf("PRE- LoadObs5 %s", AllocPoints[ment-g_entities].AllocPointer);
							//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
							//G_Printf("POST- LoadObs5 %s", AllocPoints[ment-g_entities].AllocPointer);
							G_FreeEntity( ment );
							trap_FS_FCloseFile( f );
							if ( ent )
								trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
							else
								G_Printf( "Loaded %i objects.\n", i );

							free( data );
							return;
						}
						dataPtr++;
					}



					ment->takedamage = 0;

					if ( sscanf(dataPtr, "%i ", &ment->takedamage) != 1 )
					{
						//G_Printf("PRE- LoadObs6 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs6 %s", AllocPoints[ment-g_entities].AllocPointer);
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid object.\n\""));
						else
						{
							G_Printf( "LOADING INCOMPLETE : Stopped at an invalid object.\n" );
						}
						free( data );
						return;
					}

					dataPtr = strchr( dataPtr, ' ' );
					if (dataPtr == 0)
					{
						//G_Printf("PRE- LoadObs7 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs7 %s", AllocPoints[ment-g_entities].AllocPointer);
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
						else
							G_Printf( "Loaded %i objects.\n", i );

						free( data );
						return;
					}
					dataPtr++;

					if ( ment->takedamage )
					{
						if ( sscanf(dataPtr, "%i %i %f ", &ment->health, &ment->material, &ment->radius) != 3 )
						{
							//G_Printf("PRE- LoadObs8 %s", AllocPoints[ment-g_entities].AllocPointer);
							//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
							//G_Printf("POST- LoadObs8 %s", AllocPoints[ment-g_entities].AllocPointer);
							G_FreeEntity( ment );
							trap_FS_FCloseFile( f );
							if ( ent )
								trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid set of damage information.\n\""));
							else
							{
								G_Printf( "LOADING INCOMPLETE : Stopped at an invalid set of damage information.\n" );
								//trap_TrueFree((void **)&AllocPointerObj);
							}
							free( data );
							return;
						}
					
						for(j = 0 ; j < 3 ; j++ )
						{
							dataPtr = strchr( dataPtr, ' ' );
							if (dataPtr == 0)
							{
								//G_Printf("PRE- LoadObs9 %s", AllocPoints[ment-g_entities].AllocPointer);
								//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
								//G_Printf("POST- LoadObs9 %s", AllocPoints[ment-g_entities].AllocPointer);
								G_FreeEntity( ment );
								trap_FS_FCloseFile( f );
								if ( ent )
									trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
								else
									G_Printf( "Loaded %i objects.\n", i );

								free( data );
								return;
							}

							dataPtr++;
						}
						
						if ( ment->s.eType == ET_MOVER )
							ment->genericValue4 = 1; // mark as breakable

						ment->die = mplace_die;
					}
				}
				else
				{
					//ment->target6 = G_NewString2( buffer );//, ment-g_entities
					ment->target6 = G_NewStringForEnt (buffer, ment->s.number);
					ment->s.eType = ET_FX;

					if ( sscanf(dataPtr, "%i %f %i %i ", &ment->delay, &ment->random, &ment->splashDamage, &ment->splashRadius) != 4 )
					{
						//G_Printf("PRE- LoadObs10 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs10 %s", AllocPoints[ment-g_entities].AllocPointer);
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at an invalid effect.\n\""));
						else
						{
							G_Printf( "LOADING INCOMPLETE : Stopped at an invalid effect.\n" );
							//trap_TrueFree((void **)&AllocPointerEfx);				
						}
						free( data );
						return;
					}

					for(j = 0 ; j < 4 ; j++ )
					{
						dataPtr = strchr( dataPtr, ' ' );
						if (dataPtr == 0)
						{
							//G_Printf("PRE- LoadObs11 %s", AllocPoints[ment-g_entities].AllocPointer);
							//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
							//G_Printf("POST- LoadObs11 %s", AllocPoints[ment-g_entities].AllocPointer);
							G_FreeEntity( ment );
							trap_FS_FCloseFile( f );
							if ( ent )
								trap_SendServerCommand( ent-g_entities, va("print \"Loaded %i objects.\n\"", i));
							else
								G_Printf( "Loaded %i objects.\n", i );
							free( data );
							return;
						}

						dataPtr++;
					}
				}

				if ( fileVer >= 4 )
				{
					char* end;

					end = strchr( dataPtr, '}' ); 

					if ( end == 0 )
					{
						//G_Printf("PRE- LoadObs12 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs12 %s", AllocPoints[ment-g_entities].AllocPointer);
						G_FreeEntity( ment );
						trap_FS_FCloseFile( f );
						if ( ent )
							trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at invalid spawn fields.\n\""));
						else
						{
							G_Printf( "LOADING INCOMPLETE : Stopped at invalid spawn fields.\n" );

						}
						free( data );
						return;
					}
					if(!temp)
					{
						G_Printf("LoadObs - %s: %s %s %i",ent->client->pers.netname, file, enteredPass, (end - dataPtr) );
						temp++;
					}
					Q_strncpyz( spawnFields, dataPtr, end - dataPtr );

					dataPtr = end;

					dataPtr++;
				}

				/* ment->classname = "mplaced";
				ment->creator = ent ? ent - g_entities + 1 : SERVER_CREATOR;
				VectorCopy( ment->r.currentOrigin, ment->s.origin );
				VectorCopy( ment->s.origin, ment->s.pos.trBase );

				if ( ment->s.eType == ET_MOVER )
				{
					ment->s.modelindex = G_ModelIndex( ment->model );
					ment->r.contents = CONTENTS_SOLID;
					ment->s.solid = SOLID_BBOX;
				}
				else if ( ment->s.eType == ET_FX )
				{
					ment->s.modelindex = G_EffectIndex( ment->target6 );
					ment->r.contents = CONTENTS_FOG;
					ment->s.speed = ment->delay;
					ment->s.time = ment->random;
					ment->s.modelindex2 = FX_STATE_OFF;
					ment->think = fx_runner_think; 
					ment->nextthink = level.time + 200;

					if ( ment->splashDamage > 0 )
						ment->spawnflags |= 4;
				}

				VectorSet (ment->r.mins, -1*size, -1*size, -1*size);
				VectorSet (ment->r.maxs, size, size, size);
				obj->ymins = ment->r.mins[0];
				obj->ymaxs = ment->r.maxs[0];
				ment->clipmask = 0; //MASK_SOLID;
				ment->s.pos.trType = TR_STATIONARY;
				ment->s.pos.trTime = 0;
				ment->s.pos.trDuration = 0;
				
				VectorClear( ment->s.pos.trDelta );
				VectorCopy( ment->r.currentAngles, ment->s.angles );
				VectorCopy( ment->s.angles, ment->s.apos.trBase );
				VectorSet(ment->modelScale, 1, 1, 1);
				
				if ( ment->s.eType == ET_FX )
				{
					G_SetOrigin( ment, ment->s.origin );
					G_SetAngles( ment, ment->s.angles );
				}

				trap_LinkEntity (ment);
				//G_Printf( "12\n" );*/


				ment->classname = "mplaced";
				ment->creator = ent ? ent - g_entities + 1 : SERVER_CREATOR;
				VectorCopy( ment->r.currentOrigin, ment->s.origin );
				VectorCopy( ment->s.origin, ment->s.pos.trBase );

				if ( ment->s.eType == ET_MOVER )
				{
					ment->s.modelindex = G_ModelIndex( ment->model );
					ment->s.modelindex2 = G_ModelIndex( ment->model2 );
					if ( qfalse && !ent )
					{
						G_Printf( "setting model" );

						ment->model = ment->model2;
						ment->s.modelindex = G_ModelIndex( ment->model );
						ment->s.eFlags = EF_PERMANENT;
						trap_SetBrushModel( ment, ment->model2);
						G_BSPIndex(ment->model2);
						level.mNumBSPInstances++;
					}
					else
					{
						// These two lines used to be applied to effects too.
						VectorSet (ment->r.mins, -1*size, -1*size, -1*size);
						VectorSet (ment->r.maxs, size, size, size);

						
						ment->ymins = ment->r.mins[0];
						ment->ymaxs = ment->r.maxs[0];

						ment->r.contents = CONTENTS_SOLID;
						ment->s.solid = SOLID_BBOX;
					}

					SetupClippingBox( ment );
				}
				else if ( ment->s.eType == ET_FX )
				{
					ment->s.modelindex = G_EffectIndex( ment->target6 );
					if (ment->s.modelindex > (MAX_FX-10))
					{
						if ( ent )
								trap_SendServerCommand( ent-g_entities, va("print \"Max different effects hit.\n\""));
							else
								G_Printf( "Max different effects hit.\n", i );
						free( data );
						//G_Printf("PRE- LoadObs13 %s", AllocPoints[ment-g_entities].AllocPointer);
						//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
						//G_Printf("POST- LoadObs13 %s", AllocPoints[ment-g_entities].AllocPointer);
						RemoveConfig(ment->target6, CS_EFFECTS, MAX_FX, 0);
						G_FreeEntity( ment );
						return;
					}
					ment->r.contents = CONTENTS_FOG;
					ment->s.speed = ment->delay;
					ment->s.time = ment->random;
					ment->s.modelindex2 = FX_STATE_OFF;
					ment->think = fx_runner_think; 
					ment->nextthink = level.time + 200;

					if ( ment->splashDamage > 0 )
						ment->spawnflags |= 4;

					size = 32;
					VectorSet (ment->r.mins, -1*size, -1*size, -1*size);
					VectorSet (ment->r.maxs, size, size, size);
						
					ment->ymins = ment->r.mins[0];
					ment->ymaxs = ment->r.maxs[0];
					AddUse( ment, fx_runner_use );
					//trap_TrueFree((void **)&AllocPointerEfx);
				}

				ment->clipmask = 0; //MASK_SOLID;
				ment->s.pos.trType = TR_STATIONARY;
				ment->s.pos.trTime = 0;
				ment->s.pos.trDuration = 0;
				
				VectorClear( ment->s.pos.trDelta );
				VectorCopy( ment->r.currentAngles, ment->s.angles );
				VectorCopy( ment->s.angles, ment->s.apos.trBase );
				VectorSet(ment->modelScale, 1, 1, 1);
				
				if ( ment->s.eType == ET_FX )
				{
					G_SetOrigin( ment, ment->s.origin );
					G_SetAngles( ment, ment->s.angles );
				}
			}
			else // new {spawnfields only object} savefile entry
			{
				char* end;

				dataPtr++;

				ment->creator = ent ? ent - g_entities + 1 : SERVER_CREATOR;

				end = strchr( dataPtr, '}' ); 

				if ( end == 0 )
				{
					//G_Printf("PRE- LoadObs14 %s", AllocPoints[ment-g_entities].AllocPointer);
					//Btrap_TrueFree((void **)&AllocPoints[ment-g_entities].AllocPointer);
					//G_Printf("POST- LoadObs14 %s", AllocPoints[ment-g_entities].AllocPointer);
					G_FreeEntity( ment );
					trap_FS_FCloseFile( f );
					if ( ent )
						trap_SendServerCommand( ent-g_entities, va("print \"LOADING INCOMPLETE : Stopped at invalid spawn fields.\n\""));
					else
						G_Printf( "LOADING INCOMPLETE : Stopped at invalid spawn fields.\n" );
					free( data );
					return;
				}
				if(!temp)
				{
					G_Printf("LoadObs - %s: %s %s %i",ent->client->pers.netname, file, enteredPass, (end - dataPtr) );
					temp++;
				}
				Q_strncpyz( spawnFields, dataPtr, end - dataPtr );

				dataPtr = end;

				dataPtr++;
				
			}

			trap_LinkEntity (ment);
		
			if ( strlen( spawnFields ) > 3 ) // ignore a few spaces
			{
				//G_Printf("TRACE - SpawnFields: %s", spawnFields);
				AddSpawnVars( ment, spawnFields );
			}

		}
	}
	
	trap_FS_FCloseFile( f );

	//trap_TrueFree((void **)&AllocPointerEfx);
	//trap_TrueFree((void **)&AllocPointerObj);
	free( data );


}

void Cmd_mLoadObs_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	char		password[MAX_TOKEN_CHARS];

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_LOAD_SAVE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc()!= 3 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mloadobs <filename> <password>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( strchr(buffer, '/') || strchr(buffer, '.') || strchr(buffer, '\\') )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: Illegal punctuation characters in filename.\n\""));
		return;
	}

	if ( strlen(buffer) > 25 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: Filename too long.\n\""));
		return;
	}

	//get password
	trap_Argv( 2, password, sizeof( password ) );

	LoadObs( ent, "SavedObjects", buffer, password );
}
void Cmd_mMD3Info_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	fileHandle_t f; 
	int i;
	char* data, *dataPtr; 

	md3Header_t* header;
	md3Frame_t* frame;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_TESTING) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc()!= 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmd3info <filename>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( strchr(buffer, '.') )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD MD3: Illegal punctuation characters in filename.\n\""));
		return;
	}

	if ( strlen(buffer) > 25 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD MD3: Filename too long.\n\""));
		return;
	}

	i = trap_FS_FOpenFile( va( "models/map_objects/%s.md3", buffer), &f, FS_READ);

	// if the file already exists, check we've got the right password.
	if ( !f )
	{
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD MD3: File not found\n\""));
		else
			G_Printf( "FAILED TO LOAD: File not found\n" );

		trap_FS_FCloseFile(f);
		return;
	}

//	if ( i > 127500 )
//	{
//		if ( ent )
//			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD MD3: File too large\n\""));
//		else
//			G_Printf( "FAILED TO LOAD MD3: File too large\n" );
//
//		trap_FS_FCloseFile( f );
//		return;
//	}


	data = malloc(128000);//BG_Alloc( 128000 );

	dataPtr = data;

	trap_FS_Read( data, min(i, 127500), f ); 

	data[min(i, 127500)] = '\0';

	if ( i > sizeof( md3Header_t ) )
	{
		header = (md3Header_t*) data;
		trap_SendServerCommand( ent-g_entities, va("print \"Name: %s  NumFrames: %i  NumSkins: %i  NumSurfaces: %i  NumTags: %i  OFS_Frames: %i Flags: %i  ID: %i  Ver:  \n\"",header->name, header->numFrames, header->numSkins, header->numSurfaces, header->numTags, header->ofsFrames, header->flags, header->ident, header->version ));

		if ( header->numFrames <= 0 || header->ofsFrames <= 0 ) {
			free(data);
			trap_FS_FCloseFile(f);
			return;
		}

		frame = (md3Frame_t*) (data + header->ofsFrames);

		//trap_SendServerCommand( ent-g_entities, va("print \"Bounds: %s %s   Orig: %s \nName: %s   Radius: %f   \n\"", vtos(frame->bounds[0]), vtos(frame->bounds[1]), vtos(frame->localOrigin), frame->name, frame->radius ));

		trap_SendServerCommand( ent-g_entities, va("print \"Bounds: %s  \n\"", vtos(frame->bounds[0]) ));
		trap_SendServerCommand( ent-g_entities, va("print \"Bounds: %s  \n\"", vtos(frame->bounds[1]) ));
		trap_SendServerCommand( ent-g_entities, va("print \"Origin: %s  \n\"", vtos(frame->localOrigin) ));
		trap_SendServerCommand( ent-g_entities, va("print \"Radius: %f  \n\"", frame->radius ));
	
		trap_SendServerCommand( ent-g_entities, va("print \"Name: %s  \n\"", frame->name ));
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"MD3 file too short\n\""));
	}
	free(data);
	trap_FS_FCloseFile(f);
}

#define MODEL_DIR_NAME_LENGTH 19  // "models/map_objects/"
#define MODEL_FILE_EXTENSION_LENGTH 4// ".md3"
#define FX_DIR_NAME_LENGTH 19  // "effects/"
#define FX_FILE_EXTENSION_LENGTH 4// ".efx"
#define ENDING_ZERO 1


// Provides a meaningful string to describe an entity, beyond its classname
// requires a
char* EntityType( gentity_t *ent )
{
	static char name[MAX_TOKEN_CHARS];
	int length;

	if ( ent->s.eType == ET_MOVER )
	{
		if ( ent->model == 0 || strcmp(ent->model, "*1") == 0 || strlen(ent->model) == 0 )
		{
			// looks like the model name is in model2
			if ( ent->model2 == 0 )
				return "Unknown";

			length = strlen(ent->model2);

			if ( length == 0 )
				return "Unknown";
			
			if ( Q_stricmpn("models/map_objects/", ent->model2, MODEL_DIR_NAME_LENGTH) != 0 ||
					Q_stricmpn(".md3", ent->model2+length-MODEL_FILE_EXTENSION_LENGTH, MODEL_FILE_EXTENSION_LENGTH) != 0 )
				return ent->model2;

			//remove the irrelevant bits of the model name
			Q_strncpyz( name, ent->model2 + MODEL_DIR_NAME_LENGTH, length - MODEL_DIR_NAME_LENGTH - MODEL_FILE_EXTENSION_LENGTH + ENDING_ZERO );		 
			return name;
		}
		else
		{
			length = strlen(ent->model);

			// looks as though the model name is in model 1
			if ( (Q_stricmpn("models/map_objects/", ent->model, MODEL_DIR_NAME_LENGTH) != 0) || 
					(Q_stricmpn(".md3", ent->model+length-MODEL_FILE_EXTENSION_LENGTH, MODEL_FILE_EXTENSION_LENGTH) != 0) )
				return ent->model; // not the kind of model name we're expecting

			
			//remove the irrelevant bits of the model name
			Q_strncpyz( name, ent->model + MODEL_DIR_NAME_LENGTH, strlen(ent->model) - MODEL_DIR_NAME_LENGTH - MODEL_FILE_EXTENSION_LENGTH + ENDING_ZERO );		 
			return name;
		}
	}
	else if ( ent->s.eType == ET_FX )
	{	
		if ( ent->target6 )
		{
			length = strlen(ent->target6);

			if ( !length )
				return "Unknown fx";

			if ( (Q_stricmpn("effects/", ent->target6, FX_DIR_NAME_LENGTH) != 0) ||
					(Q_stricmpn(".efx", ent->target6+length-FX_FILE_EXTENSION_LENGTH, FX_FILE_EXTENSION_LENGTH) != 0) )
				return ent->target6; // not the kind of fx name we're expecting

			Q_strncpyz( name, ent->target + FX_DIR_NAME_LENGTH, strlen(ent->target6) - FX_DIR_NAME_LENGTH - FX_FILE_EXTENSION_LENGTH + ENDING_ZERO );		 
			return name;
		}
	}
	else
	{
		return "Unknown";
	}

	return "Unknown";
}

#define OBS_PER_PAGE 16

void Cmd_mListobs_f( gentity_t *ent ) 
{
	gentity_t *obj;
	int n, clientNum, obCount, pageNum, maxCount;

	char buffer[MAX_TOKEN_CHARS];

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 3 || trap_Argc() < 2 || (trap_Argc() > 2 && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: ^5mlistobs <page-number>^7 - lists your objects. \n\""));

		if ( (ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) )
			trap_SendServerCommand( ent-g_entities, va("print \"^5mlistobs <page-number> <player/player num>^7 - lists the objects belonging to another player\n\""));

		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	for ( n = 0 ; buffer[n] != 0 ; n++ )
	{
		if ( !isdigit(buffer[n]) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"ERROR: Expecting page number to be numeric.\n\"");
			return;
		}	
	}

	pageNum = atoi(buffer);

	if ( pageNum == 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"ERROR: The first page is page 1\n\"");
		return;
	}

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		clientNum = G_ClientNumFromNetname( ent-g_entities, buffer );

		if ( clientNum < 0 )
			return;
	}
	else
	{
		clientNum = ent-g_entities;
	}

	trap_SendServerCommand( ent-g_entities, "print \"ObNum Model/Effect Name                   Origin                   Pain\n\"" );

	maxCount = (pageNum - 1 + 1) * (OBS_PER_PAGE) + 1;
	for ( obCount = 0, n = 0 ; n < ENTITYNUM_MAX_NORMAL ; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
		continue;


		if ( obj->creator == clientNum + 1 &&  (obj->s.eType != ET_NPC )  )
		{
			obCount++;

			if ( obCount >= (pageNum - 1)* OBS_PER_PAGE && obCount < maxCount )
				trap_SendServerCommand( ent-g_entities, va("print \"%4i  %-35.35s %-24.24s %i\n\"", n, EntityType(obj), vtos(obj->r.currentOrigin), obj->damage) );
		}
	}

	trap_SendServerCommand( ent-g_entities, va("print \"[Page %i of %i]\n\"", pageNum, obCount/OBS_PER_PAGE + 1));
}

void Cmd_mTrace_f( gentity_t *ent )
{	
	char		buffer[MAX_TOKEN_CHARS*2];
	vec3_t		dummy;
	gentity_t	*target; 
	char*		ptr, *dest;
	int			entNum;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: ^5mtrace^7 - tells you about the thing you're looking at\n mtrace <object-number>\n\""));
		return;
	}

	if ( trap_Argc() == 2 )
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		entNum = atoi(buffer);

		if ( entNum < 0 || entNum >= MAX_GENTITIES )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Not a valid object number.\n\""));
			return;
		}	

		target = &(g_entities[ entNum ]);
	}
	else
	{
		target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy );
	}

	if ( !target || !target->classname || strcmp(target->classname,"worldspawn") == 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"I don't see an entity.\n\"" ));
		return;
	}
	else if ( strcmp(target->classname,"player") == 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This is a player entity.\n\"" ));
		return;
	}

	// The thing we're interested in belongs to some other user
	if ( (target->creator - 1) < 0 || (target->creator - 1) > level.maxclients - 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This object doesn't belong to anyone.\n\"" ));
		
		if ( !ent->client->sess.permissions & PERMISSION_TESTING ) 
			return;
	}
	else
	{
		SanitizeString2( level.clients[target->creator - 1].pers.netname, buffer );
		trap_SendServerCommand( ent-g_entities, va("print \"Owner: %s\n\"", buffer ));
	}

	if ( !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) && ( target->creator != ent - g_entities + 1) )
		return;

	if ( ent->client->sess.permissions & PERMISSION_TESTING )
	{
		// if there's a spawn field, replace the inverted commas with \"
		dest = buffer;

		*buffer = 0;

		if ( target->spawnFields )
		{	
			for( ptr = target->spawnFields ; *ptr != '\0' ; ptr++, dest++ )
			{
				if ( *ptr == '"' )
				{
					*dest = '\'';
				}
				else
				{
					*dest = *ptr;
				}
			}
		}

		*dest = '\0';

		trap_SendServerCommand( ent-g_entities, va("print \"Owner No: %i  Object No: %i  Class: %s  Origin: %s  Angle: %s  \nModel/Type: %s  BBoxMin: %s  BBoxMax: %s \nHealth: %i Pain: %i \nSpawnFields: %s...\n\"", target->creator - 1, target - g_entities, target->classname, vtos(target->r.currentOrigin), vtos(target->r.currentAngles), EntityType(target), vtos(target->r.mins), vtos(target->r.maxs), target->health, target->splashDamage, buffer  ));
	}
	else
		trap_SendServerCommand( ent-g_entities, va("print \"Owner No: %i  Object No: %i  Origin: %s  \nAngle: %s  Model/Type: %s  \nHealth: %i  Pain: %i  Password: %s \n\"", target->creator - 1, target - g_entities, vtos(target->r.currentOrigin), vtos(target->r.currentAngles), /*target->model*/EntityType(target), target->health, target->splashDamage, target->objectPassword ));

}



void Cmd_mselect_f( gentity_t *ent )
{	
	char		buffer[MAX_TOKEN_CHARS];

	gentity_t	*target; 
	trace_t		trace;
	vec3_t		src, dest, vf, dummy;
	vec3_t		viewspot;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mselect - select the obj you're looking at\n mselect <entity-number>\n\""));
		return;
	}

	if ( trap_Argc() == 2 )
	{
		int entNum;

		trap_Argv( 1, buffer, sizeof( buffer ) );
		entNum = atoi(buffer);
		
		if ( entNum < 0 || entNum >= MAX_GENTITIES )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: %i isn't a valid entity/object number.\n\"", entNum));
			return;
		}

		target = &g_entities[entNum];
	}
	else
	{
		if (!ent || !ent->client || ent->health < 1 ||
		(ent->client->ps.pm_flags & PMF_FOLLOW) || ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(ent->client->ps.forceHandExtend != HANDEXTEND_NONE && ent->client->ps.forceHandExtend != HANDEXTEND_DRAGGING))
		{
			return;
		}

		if (ent->client->ps.emplacedIndex)
		{ //on an emplaced gun or using a vehicle, don't do anything when hitting use key
			return;
		}

		if (ent->s.number < MAX_CLIENTS && ent->client && ent->client->ps.m_iVehicleNum)
		{
			gentity_t *currentVeh = &g_entities[ent->client->ps.m_iVehicleNum];
			if (currentVeh->inuse && currentVeh->m_pVehicle)
			{
				return;
			}
		}
		
		// use the actual object clipping boxes, not the cut-down ones, that are a workaround for
		// a clipping bug in the code we can't change for makermod

		target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy ); 

		if ( !target )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Cannot see an object/entity.\n\""));
			return;
		}
	}

	if ( (!(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) && ( target->creator != ent - g_entities + 1)) ||
		  (!target->inuse))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Not an object/entity you're allowed to select.\n\""));
		return;
	}
	

	if ( target->s.eType == ET_NPC && !(ent->client->sess.permissions & PERMISSION_TESTING)  )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Cannot select this entity. It hasn't been mplaced.\n\""));
		return;
	}

	ent->client->manipulating = target - g_entities; 

	if ( ent->client->sess.permissions & PERMISSION_TESTING )
		trap_SendServerCommand( ent-g_entities, va("print \"Selected object: %i  class: %s  angle: %s  model: %s model2: %s \nBBoxMin: %s  BBoxMax: %s \n Creator Client: %i   Health: %i \n\"", target - g_entities, target->classname, vtos(target->r.currentAngles), target->model, target->model2, vtos(target->r.mins), vtos(target->r.maxs), target->creator - 1, target->health ));
	else
		trap_SendServerCommand( ent-g_entities, va("print \"Selected object: %i\n\"", target - g_entities));

}

void Cmd_mMapShader_f( gentity_t *ent )
{
	char		buffer1[MAX_TOKEN_CHARS];
	char		buffer2[MAX_TOKEN_CHARS];
 
	float f;

	if ( !(ent->client->sess.permissions & PERMISSION_TESTING) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 3 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mmapshader - <Old-Shader> <New-Shader>\n\""));
		return;
	}

	trap_Argv( 1, buffer1, sizeof( buffer1 ) );
	trap_Argv( 2, buffer2, sizeof( buffer2 ) );

	 f = level.time * 0.001;

	AddRemap(buffer1, buffer2, f);
	trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
}

void Cmd_mKillAll_f( gentity_t *ent )
{
	int n;
	gentity_t *obj;
	qboolean removeConfig = qfalse;
	char buffer[MAX_TOKEN_CHARS];

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mkillall\n\""));
		return;
	}

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
				continue;

		if ( obj->creator == ent-g_entities + 1 && (obj->s.eType != ET_NPC) )
		{
			if(obj->s.eType == ET_FX && obj->target6 && obj->target6[0])
			{
				removeConfig = qtrue;
				Q_strncpyz(buffer, obj->target6,sizeof(buffer));
			}

			//G_Printf("PRE- mkillall_f %s", AllocPoints[obj-g_entities].AllocPointer);
			//Btrap_TrueFree((void **)&AllocPoints[obj-g_entities].AllocPointer);
			//G_Printf("POST- mkillall_f %s", AllocPoints[obj-g_entities].AllocPointer);

			trap_UnlinkEntity(obj);
			G_FreeEntity(obj);

			if(removeConfig)
				RemoveConfig(buffer, CS_EFFECTS, MAX_FX, 0);
		}
	} 
}

void Cmd_mKill_f( gentity_t *ent )
{
	gentity_t	*target;
	qboolean removeConfig = qfalse;
	char buffer[MAX_TOKEN_CHARS];

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mkill - kills the currently selected object (mremove also does this)\n\""));
		return;
	}	

	target = SelectedEnt( ent );

	if ( target == 0 )
		return;
	if ( target->client )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This is a player entity!\n\""));
		return;
	}

	if(target->s.eType == ET_FX && target->target6 && target->target6[0])
	{
		removeConfig = qtrue;
		Q_strncpyz(buffer, target->target6,sizeof(buffer));
	}


	if ( (strcmp(target->classname,"misc_turret") == 0) && (target->target_ent != 0) )
	{
		//G_Printf("PRE- mkill2_f %s", AllocPoints[target->target_ent-g_entities].AllocPointer);
		//Btrap_TrueFree((void **)&AllocPoints[target->target_ent-g_entities].AllocPointer);
		//G_Printf("POST- mkill2_f %s", AllocPoints[target->target_ent-g_entities].AllocPointer);
		trap_UnlinkEntity( target->target_ent );
		G_FreeEntity( target->target_ent );
	}

	
	//Q_strncpyz(buffer, target->model2,sizeof(buffer));

	//G_Printf("%s",buffer);

	//G_Printf("PRE- mkill_f %s", AllocPoints[target-g_entities].AllocPointer);
	//Btrap_TrueFree((void **)&AllocPoints[target-g_entities].AllocPointer);
	//G_Printf("POST- mkill_f %s", AllocPoints[target-g_entities].AllocPointer);

	trap_UnlinkEntity(target);

	G_FreeEntity( target );


	//trap_TrueFree((void **)&Teest);

	if(removeConfig)
	{
		RemoveConfig(buffer, CS_EFFECTS, MAX_FX, 0);
	}


}

void Cmd_mEffect_f( gentity_t *ent )
{
	//char buffer[MAX_TOKEN_CHARS];
	char* effect;

	vec3_t defaultDir;
	int index;

	if ( !(ent->client->sess.permissions & PERMISSION_PLACE_FX) ) //(!g_cheats.integer) &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: meffect <effect-name>\n\""));
		return;
	}

	effect = ConcatArgs( 1 );

	if ( *effect == '*' )
	{
		G_EffectIndex( effect );
	}
	else
	{
		VectorSet(defaultDir, 0, 0, 1);

		index = G_EffectIndex( effect );

		if ( index == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Effect not found.\n\""));
			return;	
		}

		G_PlayEffectID( index, ent->client->sess.mark, defaultDir );
	}
}

void Cmd_mKillt_f( gentity_t *ent )
{
	gentity_t	*target;
	qboolean removeConfig = qfalse;
	char buffer[MAX_TOKEN_CHARS];
	
	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) && !(ent->client->sess.permissions & PERMISSION_PLACE_FX))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: SelectedEnt - kills a created object\n\""));
		return;
	}

	if (!ent || !ent->client || ent->health < 1 ||
		(ent->client->ps.pm_flags & PMF_FOLLOW) || ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(ent->client->ps.forceHandExtend != HANDEXTEND_NONE && ent->client->ps.forceHandExtend != HANDEXTEND_DRAGGING))
	{
		return;
	}

	if (ent->client->ps.emplacedIndex)
	{ //on an emplaced gun or using a vehicle, don't do anything when hitting use key
		return;
	}

	if (ent->s.number < MAX_CLIENTS && ent->client && ent->client->ps.m_iVehicleNum)
	{
		gentity_t *currentVeh = &g_entities[ent->client->ps.m_iVehicleNum];
		if (currentVeh->inuse && currentVeh->m_pVehicle)
		{
			return;
		}
	}

	// Use the real clipping boxes, not the workaround ones

	target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE, 0 );

	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Where's the object?\n\""));
		return;
	}

	if ( !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) && ( target->creator != ent - g_entities + 1) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Cannot see an object you're allowed to move.\n\""));
		return;
	}

	if ( target->s.eType == ET_NPC )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Cannot kill this entity. It hasn't been mplaced.\n\""));
		return;
	}
	
	if ( (strcmp(target->classname,"misc_turret") == 0) && (target->target_ent != 0) )
	{
		//G_Printf("PRE- mkillT2_f %s", AllocPoints[target->target_ent-g_entities].AllocPointer);
		//Btrap_TrueFree((void **)&AllocPoints[target->target_ent-g_entities].AllocPointer);
		//G_Printf("POST- mkillT2_f %s", AllocPoints[target->target_ent-g_entities].AllocPointer);
		trap_UnlinkEntity( target->target_ent );
		G_FreeEntity( target->target_ent );
	}

	if(target->s.eType == ET_FX && target->target6 && target->target6[0])
	{
		removeConfig = qtrue;
		Q_strncpyz(buffer, target->target6,sizeof(buffer));
	}

	//G_Printf("PRE- mkillT_f %s", AllocPoints[target-g_entities].AllocPointer);
	//Btrap_TrueFree((void **)&AllocPoints[target-g_entities].AllocPointer);
	//G_Printf("POST- mkillT_f %s", AllocPoints[target-g_entities].AllocPointer);
	trap_UnlinkEntity(target);
	G_FreeEntity( target );

	//G_Printf("PRE- mkillT3_f %s", AllocPoints[target-g_entities].AllocPointer);
	//trap_TrueFree((void **)&AllocPoints[target-g_entities].AllocPointer);
	//G_Printf("POST- mkillT3_f %s", AllocPoints[target-g_entities].AllocPointer);

	if(removeConfig)
		RemoveConfig(buffer, CS_EFFECTS, MAX_FX, 0);
}

void Cmd_mSayOrigin_f( gentity_t *ent )
{
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{
		G_Say( ent, NULL, SAY_ALL, va("My origin is: %s - You can teleport here with /mtelelast\n", vtos(ent->r.currentOrigin)) );
		VectorCopy( ent->r.currentOrigin, level.lastSaidOrigin );
	}
	else
		trap_SendServerCommand( ent-g_entities, "print \"This command isn't allowed in spectator mode.\n\"");
}

static const char *gameNames[] = {
	"Free For All",
	"Holocron FFA",
	"Jedi Master",
	"Duel",
	"Power Duel",
	"Single Player",
	"Team FFA",
	"Siege",
	"Capture the Flag",
	"Capture the Ysalamiri"
};

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

/*
==================
SanitizeString2

Rich's revised version of SanitizeString
==================
*/
void SanitizeString2( char *in, char *out )
{
	int i = 0;
	int r = 0;

	while (in[i])
	{
		if (i >= MAX_NAME_LENGTH-1)
		{ //the ui truncates the name here..
			break;
		}

		if (in[i] == '^')
		{
			if (in[i+1] >= 48 && //'0'
				in[i+1] <= 57) //'9'
			{ //only skip it if there's a number after it for the color
				i += 2;
				continue;
			}
			else
			{ //just skip the ^
				i++;
				continue;
			}
		}

		if (in[i] < 32)
		{
			i++;
			continue;
		}

		out[r] = in[i];
		r++;
		i++;
	}
	out[r] = 0;
}

/*
==================
G_ClientNumberFromStrippedName

Same as above, but strips special characters out of the names before comparing.
==================
*/
int G_ClientNumberFromStrippedName ( const char* name )
{
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];
	int			i;
	gclient_t*	cl;

	// check for a name match
	SanitizeString2( (char*)name, s2 );
	for ( i=0, cl=level.clients ; i < level.numConnectedClients ; i++, cl++ ) 
	{
		SanitizeString2( cl->pers.netname, n2 );
		if ( !strcmp( n2, s2 ) ) 
		{
			return i;
		}
	}

	return -1;
}

/*
==================
Cmd_CallVote_f
==================
*/
extern void SiegeClearSwitchData(void); //g_saga.c
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
//	int		n = 0;
//	char*	type = NULL;
	char*		mapName = 0;
	const char*	arenaInfo;

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	if ( !( (g_allowMapVote.integer != 0) && (!Q_stricmp( arg1, "map" )) ) ) // override the general voting-allowed variable
	{
		if ( !g_allowVote.integer ) {
			
			if (g_allowMapVote.integer != 0)
				trap_SendServerCommand( ent-g_entities, va("print \"Only map voting is allowed.\n\"" ) );
			else
				trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
			return;
		}
	}

	if ( level.voteTime || level.voteExecuteTime >= level.time ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEINPROGRESS")) );
		return;
	}
	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXVOTES")) );
		return;
	}

	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
			return;
		}
	}


	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) || 
		strchr( arg1, '\r' ) || strchr( arg2, '\r') ||  //Added protection against q3cbufexec
		strchr( arg1, '\n' ) || strchr( arg2, '\n')) { //-Scooper
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "map_restart" ) ) {
	} else if ( !Q_stricmp( arg1, "nextmap" ) ) {
	} else if ( !Q_stricmp( arg1, "map" ) ) {
	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
	} else if ( !Q_stricmp( arg1, "kick" ) ) {
	} else if ( !Q_stricmp( arg1, "clientkick" ) ) {
	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
	} else if ( !Q_stricmp( arg1, "timelimit" ) ) {
	} else if ( !Q_stricmp( arg1, "fraglimit" ) ) {
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>.\n\"" );
		return;
	}

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) )
	{
		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i < GT_FFA || i >= GT_MAX_GAME_TYPE) {
			trap_SendServerCommand( ent-g_entities, "print \"Invalid gametype.\n\"" );
			return;
		}

		level.votingGametype = qtrue;
		level.votingGametypeTo = i;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, gameNames[i] );
	}
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		if (g_allowMapVote.integer == 0 && (!G_DoesMapSupportGametype(arg2, trap_Cvar_VariableIntegerValue("g_gametype"))))
		{
			//trap_SendServerCommand( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
			return;
		}

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if ( g_cheats.integer != 0 )
		{	
			Com_sprintf( level.voteString, sizeof( level.voteString ), "devmap %s", arg2 );
		}
		else if (*s) 
		{
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} 
		else 
		{
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		
		arenaInfo	= G_GetArenaInfoByMap(arg2);
		if (arenaInfo)
		{
			mapName = Info_ValueForKey(arenaInfo, "longname");
		}

		if (!mapName || !mapName[0])
		{
			mapName = arg2;
		}

 
		if ( g_cheats.integer != 0 )
		{
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", arg2);
		}
		else
		{
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", mapName);
		}
	}
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected == CON_DISCONNECTED )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}
			
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			clientid = G_ClientNumberFromStrippedName(arg2);

			if (clientid == -1)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
				return;
			}
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) ) 
	{
		char	s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		SiegeClearSwitchData();
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	} 
	else
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	trap_SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
	}
	ent->client->mGameFlags |= PSG_VOTED;

	trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, va("%s", level.voteDisplayString) );	
	trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_VOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
		return;
	}
	if (g_gametype.integer != GT_DUEL &&
		g_gametype.integer != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
			return;
		}
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

	ent->client->mGameFlags |= PSG_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
		return;
	}
	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MAXTEAMVOTES")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap_Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team)
			level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
	}
	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	trap_SendServerCommand( ent-g_entities, va("print \"Please use mtele instead\n\""));
	return;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

void Cmd_minfo_f( gentity_t *ent ) 
{
	char buffer[MAX_TOKEN_CHARS];	

	if ( trap_Argc() == 2 ) 
	{
		trap_Argv( 1, buffer, sizeof(buffer) );

		if ( Q_stricmp("emotes", buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You can use the the following emotes:\n\n^5msleep, msit, msit2, msit3,msit4,\n^5mhips, matease, msurrender, mkneel, msplits, mplane,\n^5mthrow1, mthrow2, mthrow3, mflip, mshowoff, mcheer, mcower,\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5mnoisy, mspin, mnod, mshake, msignal1, msignal2, msignal3, msignal4\n\""));
			if ( ent->client->sess.permissions & PERMISSION_ANIM )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"\n^5/manim^7 - enact an animation - bind this command for custom emotes\n\""));
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mlistanims^7 - list the available animations\n\""));
			}
		}
#ifdef RANKINGMOD
		if (Q_stricmp("score", buffer) == 0)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"This server is running a work-in-progress mod in honour of trimbo's byjka mod for duel-scoring. If you think of a good name for this mod or have any suggestions for features, please let Toast know. His e-mail address is toast1@blueyonder.co.uk\nCommands available so far are;\n\n/newuser - create a username for yourself\n/login - login with your username\n/logout\n/newpass - change your password\n/stats - see your rank and score etc.\n/ranks - see a list of all ranks\n/top10 - see the top10 players on this server\n/help - see this information again\n\nFor more info on how to use a command (what little info there is at the moment) type the command on it's own, without any arguments.\n\"" ) );
			return;
		}
#endif

#ifdef MM_WIP // Work in progress commands in the build?
		else if ( Q_stricmp("wip", buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"The following work-in-progress commands are incomplete. \nThere may be bugs and future behaviour is likely to change. \nYou are welcome to try them out at your own risk.\n\n\"");
			
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mrotating^7 - makes the selected ob rotate\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mpendulum^7 - makes the selected ob rotate like a pendulum\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mbobbing^7 - makes the selected ob bob\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mplatform^7 - makes the selected ob act like an elevator\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mtelep^7 - makes an ob/fx into a teleporter\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mdest^7 - marks the destination for the selected ob (E.g. teleporter/jumppad)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlight^7 - makes an ob/fx into a light source\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mjumpp^7 - makes an object into a jump/push pad\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlistso^7 - list special objects you can place\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mmoveall^7 - with one number, now moves in direction you're looking\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mspawner^7 - turns your object into an npc spawner\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/musable^7 - makes your object usable\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mconnectto^7 - connects your ob up to activate the ob you're looking at\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mkillsw^7 - make selected ob a switchable killer (can be used with musable or connected to)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mtelesw^7 - make seleceted ob a switchable teleporter (usable or connected to)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mprintsw^7 - make selected ob a switchable printer (usable or connected to)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mjumpsw^7 - make selected ob a switchable jump/pusher(usable or connected to)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/msoundsw^7 - make selected ob a switchable sound player(usable or connected to)\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlistsnd^7 - list the sounds you can use\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mtouchable^7 - make the selected ob activate itself or its connected obs when touched\n\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mallowgive^7 - toggles if you accept or refuse giveother attempts. Default: Refuse\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/msetpassword^7 - sets password needed for usage. <none> turns off password.\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/msetpasswordt^7 - sets password on targeted object. <none> turns off password.\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mpassword^7 - enter a password to use passworded objects.\n\""));



		

			//trap_SendServerCommand( ent-g_entities, va("print \"^5/mdoor^7 - makes the manipulated ob act like a\n\""));
		}
#endif
		else if ( Q_stricmp("properties", buffer) == 0 )
		{			
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mpain^7 - sets the pain caused by an fx\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mbreakable^7 - makes an object breakable\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mbreakableall^7 - makes all your objects breakable\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mscale^7 - scales the currently selected object\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mscalet^7 - scales the object or npc you're looking at\n\""));

		}
		else if ( Q_stricmp("testing", buffer) == 0 && (ent->client->sess.permissions & PERMISSION_TESTING) )
		{			
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mmd3info^7 - displays header-data from an md3 file\n\""));
		}
		else if ( Q_stricmp("admin", buffer) == 0 )
		{			
			trap_SendServerCommand( ent-g_entities, va("print \"\nUser Management\n\n\""));

			if ( ent->client->sess.permissions & PERMISSION_ADMIN_SAY )
			{
				trap_SendServerCommand( ent-g_entities, "print \"^5/mpsay^7 - sends a screen print message to a user or all users\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"^5/mannounce^7 - displays a screen print message to all users for 20secs\n\"");
			}

			if ( ent->client->sess.permissions & PERMISSION_STATUS )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mstatus^7 - lists of players, their ip addresses and usernames \n\""));

			trap_SendServerCommand( ent-g_entities, va("print \"^5/mslap^7 - slaps yourself or a given player\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mslay^7 - slays yourself or a given player\n\""));
			
			if ( ent->client->sess.permissions & PERMISSION_KICK )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mkick^7 - kicks a given player\n\""));
			
			if ( ent->client->sess.permissions & PERMISSION_BAN )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mban^7 - bans a given player\n\""));
				trap_SendServerCommand( ent-g_entities, va("print \"^5/munban^7 - unbans a given player\n\n\""));
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mlistbans^7 - lists the currently banned ip's\n\n\""));
		
			}

			if ( ent->client->sess.permissions & PERMISSION_NPC_ADMIN )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^5/npc kill everything^7 - kills the npc's belonging to all players\n\""));
			}

			trap_SendServerCommand( ent-g_entities, va("print \"\nUser Permissioning\n\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlogin^7 - login\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlogout^7 - logout\n\""));
			if ( ent->client->sess.permissions & PERMISSION_NEW_USER )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mnewuser^7 - creates a new username with matching password\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mchangepass^7 - changes your password\n\""));
			if ( ent->client->sess.permissions & PERMISSION_USER_ADMIN )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mresetpass^7 - resets a user's password to his/her username\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mpermissions^7 - see your permissions or those of another user\n\""));
			
			if ( ent->client->sess.permissions & PERMISSION_USER_ADMIN )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mpermission^7 - give a user one or more permisions\n\""));
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mforbid^7 - take away one or more permissions\n\""));
			}

			if ( ent->client->sess.permissions & PERMISSION_MARK_EDGES )
			{
				trap_SendServerCommand( ent-g_entities, "print \"\nSolid Sizing\n\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"You can set the size of the solid box for each model.\nIf the size isn't set, the game uses a box containing the whole object.\nThis command affects all future instances of a model and is intended for\nadvanced users only.\n You may need to kill the ob and place it again for\nthe size changes to take effect.\n The behaviour of these commands is likely to change in future.\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"^5/mmarksides^7 - mark the sides of the bounding box\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"^5/mmarkallsides^7 - mark all the sides to be your distance from model's origin\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"^5/mmarktopbottom^7 - mark the top or bottom of the model with your feet\n\"");
				trap_SendServerCommand( ent-g_entities, "print \"^5/msaveedges^7 - save the edges you've marked so that they affect all users\n\"");			
			}


		}
		else if ( g_cheats.integer && Q_stricmp("cheats", buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"STANDARD CHEAT COMMANDS (on all servers)\n\n^5/god^7 - makes your character invulnerable^5\n/give all^7 - gives you all weapons and inventory items (including jetpack)\n^5/noclip^7   - lets you float through walls and makes you invulnerable\n^5/notarget^7 - stops computer-controlled characters attacking you\n^5/npc spawn^7 - spawn an NPC (non-player character). (E.g. /npc spawn jawa jim)\n^5/npc kill^7 - kill a named NPC (E.g. /npc kill jim)\n^5/npc spawn vehicle^7 - spawn a vehicle (E.g. /npc spawn vehicle rancor_vehicle mike)\n^5/npc kill all^7 - kill all npc's that don't belong to someone else. Maker Mod has\nstopped this killing absolutely everything\n") );
		}
		else if ( Q_stricmp("placing", buffer) == 0 && ((ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) || (ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS)) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^5/mmark^7 - mark a position for use with other commands\n^5/mmarkfoot^7 - marks the position of your feet (useful for placing objects on the map)\n^5/mlist^7 - without a folder-name lists the folders that contain map objects.\n^5/mlist^7 - With a folder name lists the models in a given folder.\n^5/mplace^7 - lets you put an inanimate object on the map (either at an mmark'ed position\n or at typed coordinates). Object names take the form 'vjun/globe', where the part before\n the '/' is the object's folder name.\n\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mlistfx^7 - lists the effects you can put on the map (works like mlist)\n^5/mplacefx^7 - places an effect on the map.\n\"");
		}
		else if ( Q_stricmp("manip", buffer) == 0 && ((ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) || (ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS)) )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mlistobs^7 - lists the objects belonging to a player\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mtrace^7 - gives you info on a game entity you're looking at or by number\n\n\""));

			trap_SendServerCommand( ent-g_entities, va("print \"^5/mselect^7 - changes the selected object/effect to the one you specify or have targetted \n\n\""));
			// mgrab stuff
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mgrab^7 - lets you grab the selected object/fx\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mgrabt^7 - lets you grab the object/fx in your targets\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mdrop^7 - lets you drop anything you're grabbing\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/marm^7 - sets the length of your grabbing arm\n\""));
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mgrabbing^7 - turns auto-grabbing on and off.\n\n\""));	

			trap_SendServerCommand( ent-g_entities, "print \"^5/mmove^7 - start the currently selected object moving in a given direction. Its speed\n is determined by the size of the numbers used for movement. (E.g. /mmove 0 20 0)\n (E.g.the opposite direction /mmove 0 -20 0) (E.g. to stop an object moving /mmove 0 0 0)\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"I recommend you bind the 6 directions and stop to keys to make it easier to move things)\n^5/mkill^7 - kill the currently selected object\n/^5mkillt^7 - kills the ob/fx you're targeting\n^5/mkillall^7 - kill all of the objects you've created\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mrotate^7 - rotates the object you've selected.\n\"");
			trap_SendServerCommand( ent-g_entities, va("print \"^5/mmoveall^7 - move all of your objects and effects in one go\n\""));
		}
		else if ( Q_stricmp("saving", buffer) == 0 && ((ent->client->sess.permissions & PERMISSION_LOAD_SAVE) || (ent->client->sess.permissions & PERMISSION_SAVE_MAPOBS)) )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^5/msaveobs^7 - save a password protected file on the server containing the objects you've created\n^5/mloadobs^7 - load a previously saved set of objects\n\n^5/mlistfx^7 - lists the effects you can put on the map (works like mlist)\n^5/mplacefx^7 - places an effect on the map.\n\"");
			
			if ( ent->client->sess.permissions & PERMISSION_SAVE_MAPOBS )
				trap_SendServerCommand( ent-g_entities, "print \"^5/msavemapobs^7 - saves objects to be automatically loaded with the current map\n\"");

		}
		else if ( Q_stricmp("player", buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^5/mempower^7 - gives you all force powers\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mscaleme^7 - changes your size (less than one is small E.g. 0.5, larger than one is big)\n^5/mscalet^7 - changes the size of an NPC you're close to and looking at (if it's your npc)\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mattack^7 - tells your npc's to attack the npc/player in your crosshairs. Npc response varies.\n\"");
		}
		else if ( Q_stricmp("tele", buffer) == 0 && ((ent->client->sess.permissions & PERMISSION_TELE) || (ent->client->sess.permissions & PERMISSION_TELE_ADMIN))  )
		{
			trap_SendServerCommand( ent-g_entities, "print \"^5/mmark^7 - mark a position for use with other commands\n^5/mtele^7 - teleport to a marked position or typed position\n^5/morigin^7 - gives your current position on the map\n^5/msayorigin^7 - says your position so that other people can teleport to it\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mtelelast^7 - teleports to the last position announced with msayorigin\n\"");
			trap_SendServerCommand( ent-g_entities, "print \"^5/mtelesp^7 - teleports to one of the map's starting points\n\"");
			if (ent->client->sess.permissions & PERMISSION_TELE_ADMIN)
			{
				trap_SendServerCommand( ent-g_entities, "print \"^5/mtelet^7 - teleports you to the place targetted by your cross-hairs\n\"");
			}

		}
		else
		{
			trap_SendServerCommand( ent-g_entities, "print \"Unrecognised minfo keyword\n\"");
		}
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, "print \"MAKERMOD v1.0b\nCelebrating our power to learn to make amazing things.\nIf you have a bug to report, a feature to suggest, or just want to chat\nvisit the makermod forum at ^5www.makermod.com^7\n\nType the following commands into the console for help on a\nparticular subject.\n\n\"" );

#ifdef RANKINGMOD
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo score^7 - Find out how to login and record your duelling stats\n\""));
#endif
		if ( g_cheats.integer ) //|| ent->client->sess.permissions & PERMISSION_CHEATS) <----- not implemented yet
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo cheats^7 - tells you about some of the cheat commands you can use\n\""));
		
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo emotes^7 - Maker Mod's emotes\n\""));
		
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo player^7 - things you can do to yourself and/or other npc's/players\n\""));
		
		if ( (ent->client->sess.permissions & PERMISSION_TELE) || (ent->client->sess.permissions & PERMISSION_TELE_ADMIN) )
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo tele^7 - teleporting with Maker Mod\n\""));
		if ( (ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) || (ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) )
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo placing^7 - placing objects and effects\n\""));
		if ( (ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) || (ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) )
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo manip^7 - manipulating placed objects and effects\n\""));		
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo properties^7 - setting object properties\n\""));		
		if ( (ent->client->sess.permissions & PERMISSION_LOAD_SAVE) || (ent->client->sess.permissions & PERMISSION_SAVE_MAPOBS) )
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo saving^7 - saving/loading placed objects and effects\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo admin^7 - admin commands\n\""));
#ifdef MM_WIP
		trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo wip^7 - work-in-progress commands\n\""));
#endif
		if ( ent->client->sess.permissions & PERMISSION_TESTING )
			trap_SendServerCommand( ent-g_entities, va("print \"^5/minfo testing^7 - commands for debugging and testing\n\""));

	}
}

#define MAX_KEY_START_POS 25
#define MAX_KEY_POS 30

qboolean WriteUserFile( gentity_t *ent, qboolean overwrite, char* username, char* password, int permissions, int credits, int otherValue )
{
	int i, encodingKeyPos, length;
	char		fileData[MAX_USER_FILE_LENGTH];
	char* encodingKey = "Data Successfully written for user %s";
	fileHandle_t	f;

	if ( overwrite == qfalse )
	{
		// Check we're not overwriting anything
		i = trap_FS_FOpenFile( va( "Users/%s", username), &f, FS_READ);

		if ( f && i > 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED: Username already exists. Please try another username.\n\""));
			trap_FS_FCloseFile(f);
			return qfalse;
		}
	}

	i = trap_FS_FOpenFile( va( "Users/%s", username), &f, FS_WRITE);

	if (!f)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"WARNING : Failed to save user data\n\""));
		return qfalse;
	}
	
	srand( (unsigned)time( NULL ) );

	encodingKeyPos = rand() % (MAX_KEY_START_POS + 1);

	// The first number is the file version
	// The last number, is for some yet-to-be-discovered need.
	Com_sprintf( fileData, MAX_USER_FILE_LENGTH, "2%c%s %i %i %i", encodingKeyPos + 'c', password, permissions, credits, otherValue );

	length = strlen(fileData);

	for ( i = 2 ; i < length ; i++, encodingKeyPos++ )
	{
		if ( encodingKeyPos > MAX_KEY_POS ) 
			encodingKeyPos = 0;

		// very simple and crackable encoding - just EOR our file with the key.
		fileData[i] ^= encodingKey[encodingKeyPos];
	}

	trap_FS_Write( fileData, min(MAX_USER_FILE_LENGTH, length), f );

	trap_FS_FCloseFile(f);

	return qtrue;
}

// Read a user file into a client entity
// if password == 0, don't check the password
qboolean ReadUserFile( gentity_t *ent, char* username, char* password )
{
	int permissions, credits, otherValue;
	int i, encodingKeyPos, length;
	qboolean succeeded, doDecryption;
	char* encodingKey = "Data Successfully written for user %s";
	char		fileData[MAX_USER_FILE_LENGTH];
	char* ptr;
	fileHandle_t	f;

	succeeded = qtrue;
	doDecryption = qtrue;

	length = trap_FS_FOpenFile( va( "Users/%s", username), &f, FS_READ);

	if (!f || length <= 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Username not found\n\""));
		return qfalse;
	}
	
	trap_FS_Read( fileData, MAX_USER_FILE_LENGTH, f );

	Q_strncpyz( ent->client->sess.username, username, sizeof(ent->client->sess.username) );

	// The first number is the file version
	ptr = fileData;
	if ( *ptr == '1' )
	{
		// File Version 1
		doDecryption = qfalse; //no encryption in v1

		ptr++;
		if ( *ptr == ' ' )
			ptr++;
		else 
			succeeded = qfalse;

		// skip the superfluous username in version 1 files.
		while ( *ptr != ' ' && *ptr != '\0' )
			ptr++;
	

		// and space after username
		if ( *ptr == ' ' )
			ptr++;
	}
	else if ( *ptr == '2' )
	{
		ptr++;
		encodingKeyPos = *ptr - 'c';
		ptr++;

		if ( encodingKeyPos > MAX_KEY_START_POS )
			succeeded = qfalse;
	}
	else
	{
		succeeded = qfalse;
	}
	
	if ( succeeded && doDecryption )
	{
		for ( i = 2 ; i < length ; i++, encodingKeyPos++ )
		{
			if ( encodingKeyPos > MAX_KEY_POS ) 
				encodingKeyPos = 0;

			// very simple and crackable encoding - just EOR our file with the key.
			fileData[i] ^= encodingKey[encodingKeyPos];
		}
	}

	if ( succeeded && sscanf(ptr, "%s %i %i %i" , ent->client->sess.password, &permissions, &credits, &otherValue) == 4 )
	{
		if ( password && strcmp(ent->client->sess.password, password) != 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Wrong password. (Reminder : passwords are case-sensitive)\n\""));	
			ent->client->sess.username[0] = '\0';
			ent->client->sess.password[0] = '\0';
			trap_FS_FCloseFile(f);
			return qfalse;
		}
		
		ent->client->sess.permissions = permissions;	
	}
	else
	{
		succeeded = qfalse;
	}
	
	if ( !succeeded )
	{
		ent->client->sess.username[0] = '\0';
		ent->client->sess.password[0] = '\0';
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Corrupt/Unrecognised user-file format. Please tell admin.\n\""));
	}

	trap_FS_FCloseFile(f);

	return succeeded;
}


void DecryptTest( gentity_t *ent, char* username )
{
	int permissions, credits, otherValue;
	int i, encodingKeyPos, length;
	qboolean succeeded, doDecryption;
	char* encodingKey = "Data Successfully written for user %s";
	char		fileData[MAX_USER_FILE_LENGTH];
	char* ptr;
	fileHandle_t	f;

	succeeded = qtrue;
	doDecryption = qtrue;

	length = trap_FS_FOpenFile( va( "Users/%s", username), &f, FS_READ);

	if (!f || length <= 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Username not found\n\""));
		return;
	}
	
	trap_FS_Read( fileData, MAX_USER_FILE_LENGTH, f );

	Q_strncpyz( ent->client->sess.username, username, sizeof(ent->client->sess.username) );

	// The first number is the file version
	ptr = fileData;
	if ( *ptr == '1' )
	{
		// File Version 1
		doDecryption = qfalse; //no encryption in v1

		ptr++;
		if ( *ptr == ' ' )
			ptr++;
		else 
			succeeded = qfalse;

		// skip the superfluous username in version 1 files.
		while ( *ptr != ' ' && *ptr != '\0' )
			ptr++;
	

		// and space after username
		if ( *ptr == ' ' )
			ptr++;
	}
	else if ( *ptr == '2' )
	{
		ptr++;
		encodingKeyPos = *ptr - 'c';
		ptr++;

		if ( encodingKeyPos > MAX_KEY_START_POS )
			succeeded = qfalse;
	}
	else
	{
		succeeded = qfalse;
	}
	
	if ( succeeded && doDecryption )
	{
		for ( i = 2 ; i < length ; i++, encodingKeyPos++ )
		{
			if ( encodingKeyPos > MAX_KEY_POS ) 
				encodingKeyPos = 0;

			// very simple and crackable encoding - just EOR our file with the key.
			fileData[i] ^= encodingKey[encodingKeyPos];
		}
	}

	if ( succeeded && sscanf(ptr, "%s %i %i %i" , ent->client->sess.password, &permissions, &credits, &otherValue) == 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Decrypted:\nUsername: %s\nPassword: %s\nPermissions: %i\nCredits: %i\nOtherValue: %i\n\"",username, ent->client->sess.password, permissions, credits, otherValue));	
	}
	else
	{
		succeeded = qfalse;
	}
	
	if ( !succeeded )
	{
		ent->client->sess.username[0] = '\0';
		ent->client->sess.password[0] = '\0';
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Corrupt/Unrecognised user-file format. Please tell admin.\n\""));
	}

	trap_FS_FCloseFile(f);
}
void Cmd_mNewUser_f( gentity_t *ent ) 
{
	char		username[MAX_TOKEN_CHARS];
	char		buffer[MAX_TOKEN_CHARS];
	int			permissions;

	if ( !(ent->client->sess.permissions & PERMISSION_NEW_USER) ) //!g_cheats.integer &&
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 3 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mnewuser <username> <optional-permissions>\n\""));
		return;
	}	

	trap_Argv( 1, username, sizeof(username) );

	if ( strlen(username) > MAX_USERNAME_LENGTH )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Username too long. It must be %i character or less\n\"", (int)MAX_USERNAME_LENGTH));
		return;
	}

	if ( strchr(username, '/') || strchr(username, '.') || strchr(username, '\\') )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Punctuation characters are forbidden\n\""));
		return;
	}

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof(buffer) );
		permissions = atoi( buffer );
	}
	else
	{
		permissions = StringToPermissionsNum( g_defaultNewUserPermissions.string );
	}

	WriteUserFile( ent, qfalse, username, username, permissions, 0, 0 );
}

void Cmd_mChangePass_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	char		password[MAX_TOKEN_CHARS];

//	if ( !(ent->client->sess.permissions & PERMISSION_NEW_USER) ) //!g_cheats.integer &&
//	{
//		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
//		return;
//	}

	if ( ent->client->sess.username[0] == '\0' )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Not logged in\n\""));
		return;
	}

	if ( trap_Argc() != 4 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mchangepass <old-password> <new-password> <new-password>\n\""));
		return;
	}	

	trap_Argv( 1, buffer, sizeof(buffer) );

	// Make we don't save a temporary permission setting.
	if ( !ReadUserFile( ent, ent->client->sess.username, 0 ) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Failed to change password\n\""));
		return;
	}

	if ( strcmp(ent->client->sess.password, buffer) != 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : Old password incorrect\n\""));
		return;
	}

	trap_Argv( 2, password, sizeof(password) );
	trap_Argv( 3, buffer, sizeof(buffer) );

	if ( strcmp(buffer, password) != 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : New passwords don't match\n\""));
		return;
	}

	if ( strlen(buffer) > MAX_PASSWORD_LENGTH )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR : New password too long. Password must be %i characters or less.\n\"", (int) MAX_PASSWORD_LENGTH));
		return;
	}

	strcpy( ent->client->sess.password, buffer );

	WriteUserFile( ent, qtrue, ent->client->sess.username, ent->client->sess.password, ent->client->sess.permissions, 0, 0 );

	trap_SendServerCommand( ent-g_entities, va("print \"Password changed\n\""));
}


void Cmd_mResetPass_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	char		username[MAX_TOKEN_CHARS];
	int			permissions;

	if ( !HasPermission(ent, PERMISSION_USER_ADMIN) ) //!g_cheats.integer &&
		return;

	if ( trap_Argc() != 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mresetpass <username>\n\""));
		return;
	}	

	trap_Argv( 1, buffer, sizeof(buffer) );
	
	// store our own info while we load the other user's file
	permissions = ent->client->sess.permissions;
	Q_strncpyz( username, ent->client->sess.username, sizeof( ent->client->sess.username ) );

	ReadUserFile( ent, buffer, 0 );

	if ( WriteUserFile(ent, qtrue, ent->client->sess.username, ent->client->sess.username, ent->client->sess.permissions, 0, 0) )
		trap_SendServerCommand( ent-g_entities, va("print \"Password reset\n\""));

	// copy our user file back into our game entitiy
	ent->client->sess.permissions = permissions;
	Q_strncpyz( ent->client->sess.username, username, sizeof( ent->client->sess.username ) );
}

void Cmd_mtele_f( gentity_t *ent ) 
{
	gentity_t* target;
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i, j;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_TELE_ADMIN) && !(ent->client->sess.permissions & PERMISSION_TELE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() == 5 || trap_Argc() == 4  ) 
	{
		VectorClear( angles );
		for ( i = 0 ; i < 3 ; i++ ) {
			trap_Argv( i + 1, buffer, sizeof( buffer ) );
			origin[i] = atof( buffer );
		}

		if ( trap_Argc() == 5 )
		{
			trap_Argv( 4, buffer, sizeof( buffer ) );
			angles[YAW] = atof( buffer );
		}

		TeleportPlayer( ent, origin, angles );
	}	
	else if ( trap_Argc() == 1 )
	{
		VectorClear( angles );
		//angles[YAW] = 0;
		VectorCopy( ent->client->ps.viewangles, angles );
		TeleportPlayer( ent, ent->client->sess.mark, angles );
	}
	else if ( (ent->client->sess.permissions & PERMISSION_TELE_ADMIN) && trap_Argc() == 2 )
	{
		// teleport the caller to a given player.

		VectorClear( angles );
		trap_Argv( 1, buffer, sizeof( buffer ) );

		i = G_ClientNumFromNetname(ent-g_entities, buffer);


		if ( i == -1 )
			return;

		memcpy( origin, g_entities[i].r.currentOrigin, sizeof(origin) );

		origin[1] -= 40;

		TeleportPlayer( ent, origin, angles );
	}
	else if ( (ent->client->sess.permissions & PERMISSION_TELE_ADMIN) && trap_Argc() == 3 )
	{
		// Teleport a person to another person.

		VectorClear( angles );
		trap_Argv( 1, buffer, sizeof( buffer ) );

		i = G_ClientNumFromNetname(ent-g_entities, buffer);

		trap_Argv( 2, buffer, sizeof( buffer ) );

		j = G_ClientNumFromNetname(ent-g_entities, buffer);
		target = &g_entities[j];

		if ( i == -1 || j == -1 )
			return;

		memcpy( origin, g_entities[i].r.currentOrigin, sizeof(origin) );

		origin[1] -= 40;

		TeleportPlayer( target, origin, angles );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtele <x> <y> <z> <optional-yaw>\nor to teleport to your mmark'ed position - /mtele\n\""));
		if (ent->client->sess.permissions & PERMISSION_TELE_ADMIN)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"ADMIN usage: mtele <id>, teleports you to the player.\nmtele <id1> <id2>, teleports id2 to id1.\n\""));
		}
		return;
	}
}




void Cmd_mgrab_f( gentity_t *ent ) 
{
	gentity_t   *target;

	if ( !g_cheats.integer && !HasPermission(ent, PERMISSION_PLACE_OBJECTS) )
		return;

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mgrab\n\""));
		return;
	}	

	target = SelectedEnt( ent );

	if ( target )
		GrabObject( target, ent );
}

void Cmd_marm_f( gentity_t *ent ) 
{
	char		buffer[MAX_TOKEN_CHARS];
	int armLength;

	if ( !g_cheats.integer && !HasPermission(ent, PERMISSION_PLACE_OBJECTS) )
		return;

	if ( trap_Argc() > 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: marm <optional-arm-length>\n\""));
		return;
	}	

	if ( trap_Argc() == 2 )
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );
		armLength = atoi( buffer );

		if ( armLength > 500 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"WARNING: Maximum arm length is 500.\n\""));
			armLength = 500;
		}
		else if ( armLength < -500 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"WARNING: Minimum arm length is -500.\n\""));
			armLength = -500;
		}

		ent->client->sess.arm = armLength;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"Your grabbing arm length is %i. The default is %i.\n\"", ent->client->sess.arm, DEFAULT_ARM_LENGTH ));
}



void Cmd_mgrabt_f( gentity_t *ent ) 
{
	gentity_t   *target;
	vec3_t dummy;

	if ( !g_cheats.integer && !HasPermission(ent, PERMISSION_PLACE_OBJECTS) )
		return;

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mgrabt\n\""));
		return;
	}

	target = AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_FOG|CONTENTS_TRIGGER, dummy );
	
	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything to grab.\n\""));
		return;
	}
	
	if ( ((target->creator - 1 != ent-g_entities) && !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)) || target->s.eType == ET_NPC )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Cannot see anything you're allowed to grab.\n\""));
		return;
	}

	GrabObject( target, ent );
}

void Cmd_mDrop_f( gentity_t *ent ) 
{
	int n;
	gentity_t* obj;
	qboolean wasGrabbed;

	if ( !g_cheats.integer && !HasPermission(ent, PERMISSION_PLACE_OBJECTS) )
		return;

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mdrop\n\""));
		return;
	}

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
				continue;

		if ( ((obj->creator == ent-g_entities + 1) || (ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)) 
				&& (obj->s.eType != ET_NPC) )
		{
			if ( obj->enemy != ent )
				continue;

			if ( (obj->s.eType == ET_FX && !CanExistHere(obj->r.currentOrigin,ENT_FX)) ||
					(obj->s.eType != ET_FX && !CanExistHere(obj->r.currentOrigin,ENT_OBJECT)) )
			{
				obj->s.pos.trType = TR_INTERPOLATE;
				obj->s.pos.trType = TR_INTERPOLATE;
				trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Objects can't be dropped here.\nPlease try another part of the map.\n\""));
				continue;
			}

			wasGrabbed = qfalse;

			obj->enemy = 0;
			obj->grabbing = qfalse;

			if ( obj->s.apos.trType == TR_INTERPOLATE )
			{
				obj->s.pos.trType = TR_STATIONARY;
				wasGrabbed = qtrue;
			}
				
			if ( obj->s.pos.trType == TR_INTERPOLATE )
			{
				obj->s.pos.trType = TR_STATIONARY;
				wasGrabbed = qtrue;
			}

			if ( wasGrabbed )
			{
				//if ( obj->oldThink != 0 )
				//	obj->think = obj->oldThink;


				SnapVector( obj->r.currentOrigin );
				SnapVector( obj->r.currentAngles );
				VectorCopy( obj->r.currentOrigin, obj->s.pos.trBase );
				VectorCopy( obj->r.currentAngles, obj->s.apos.trBase );
				VectorCopy( obj->r.currentOrigin, obj->s.origin );
				VectorCopy( obj->r.currentAngles, obj->s.angles );
				SetupClippingBox( obj );

				//if ( ent->s.eType != ET_FX && ent->s.eType != ET_MOVER )
				//{
				//	AddSpawnVars( ent, va("origin \"%i %i %i\" angles \"%i %i %i\"", (int)ent->r.currentOrigin[0], (int)ent->r.currentOrigin[1], (int)ent->r.currentOrigin[2], (int)ent->r.currentAngles[0], (int)ent->r.currentAngles[1], (int)ent->r.currentAngles[2]) );
				//}
			}
		}	
	} 

	trap_SendServerCommand( ent-g_entities, va("print \"Dropped.\n\""));
}

void Cmd_mtelet_f( gentity_t *ent ) {
	vec3_t		origin, angles;

	if ( !(ent->client->sess.permissions & PERMISSION_TESTING) && !(ent->client->sess.permissions & PERMISSION_TELE_ADMIN) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtelet\n\""));
		return;
	}

	VectorCopy( ent->client->ps.viewangles, angles );

	AtCrossHairs( ent, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE|CONTENTS_TERRAIN, origin );

	origin[2] += 48;

	TeleportPlayer( ent, origin, angles );
}


void Cmd_mtelelast_f( gentity_t *ent ) {
	vec3_t		origin, angles;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_TELE_ADMIN) && !(ent->client->sess.permissions & PERMISSION_TELE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtelelast\n\""));
		return;
	}

	if ( level.lastSaidOrigin[0] == 0 && level.lastSaidOrigin[1] == 0 && level.lastSaidOrigin[2] == 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Error: Noone has yet said an origin this game\n\""));
		return;
	}

	VectorCopy( ent->client->ps.viewangles, angles );

	VectorCopy( level.lastSaidOrigin, origin );

	TeleportPlayer( ent, origin, angles );
}

void Cmd_manim_f( gentity_t *ent ) 
{
	int anim;
	int length;
	char		buffer[MAX_TOKEN_CHARS];
		
	length = -1;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_ANIM) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 3 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : manim <animimation-ID> <optional-animation-length>\n\""));
		return;
	}	

	trap_Argv( 1, buffer, sizeof( buffer ) );
	anim = GetIDForString(animTable, buffer);

	if ( anim < 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Invalid animation ID. Did you type it properly?\n\""));
		return;
	}

	if ( trap_Argc() > 2 )  
	{
		trap_Argv( 2, buffer, sizeof( buffer ) );
		length = atoi( buffer );
	}

	DoAnim( ent, anim, length, qtrue, qtrue );
}


void Cmd_mlistanims_f( gentity_t *ent ) 
{
	int page, i , end;
	char		buffer[MAX_TOKEN_CHARS];

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_ANIM) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mlistanims <page-number>\n\""));
		return;
	}	

	trap_Argv( 1, buffer, sizeof( buffer ) );

	page = atoi( buffer );

	i = max(0, (page-1) * 20 - 1); 
	end = i + 20;

	for( ; i < MAX_ANIMATIONS && i <= end ; i++ )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", animTable[i].name ));
	}

}

void Cmd_mpermissions_f( gentity_t *ent ) 
{
	int		i;
	char	buffer[MAX_TOKEN_CHARS];
	char	username[MAX_TOKEN_CHARS];
	char    indicator[2];



	indicator[ sizeof(indicator) -1 ] = 0;

	if ( trap_Argc() > 2 || trap_Argc() == 1 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mpermissions <optional-username>\nWithout a username, all permission symbols are listed (#'s indicate your permissions).\n\""));
		
		if ( trap_Argc() > 2 )
			return;
	}	

	if ( trap_Argc() == 1 ) 
	{
		for( i = 0 ; i < MAX_USER_PERMISSION_CODE ; i++ )
		{
			if ( *(permissionCode[i]) != 0 )
			{
				if ( ( ((1 << i) & (ent->client->sess.permissions)) != 0 ) )
					indicator[0] = '#';
				else
					indicator[0] = ' ';

				trap_SendServerCommand( ent-g_entities, va("print \"%s ^5%s\n\"", indicator, permissionCode[i] ));
			}
		}
	}

	if ( trap_Argc() == 2 )
	{
		int permissions;
		qboolean isThisUser;

		if ( !(ent->client->sess.permissions & PERMISSION_PERMISSION) )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to look at other people's permissions.\n\""));
			return;
		}

		trap_Argv( 1, buffer, sizeof( buffer ) );

		if ( Q_stricmp( buffer, ent->client->sess.username ) == 0 )
		{
			isThisUser = qtrue;
		}
		else
		{
			permissions = ent->client->sess.permissions;
			Q_strncpyz( username, ent->client->sess.username, sizeof( ent->client->sess.username ) );
			isThisUser = qfalse;
		}

		if ( isThisUser || ReadUserFile(ent, buffer, 0) )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s has the follwoing permissions...\n\"", buffer) );
	
			for( i = 0 ; i < MAX_USER_PERMISSION_CODE ; i++ )
			{
				if ( *(permissionCode[i])!= 0 && ( ((1 << i) & (ent->client->sess.permissions)) != 0  ) )
					trap_SendServerCommand( ent-g_entities, va("print \"^5%s\n\"", permissionCode[i] ));
			}
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Failed to get user permissions\n\"") );
		}
		
		// Recover our own user data
		if ( !isThisUser )
		{
			ent->client->sess.permissions = permissions;
			Q_strncpyz( ent->client->sess.username, username, sizeof( ent->client->sess.username ) );
		}
	}
}

void Cmd_mPermission_f( gentity_t *ent ) 
{
	int		i, j;
	char	buffer[MAX_TOKEN_CHARS];
	char	username[MAX_TOKEN_CHARS];
	char	realUsername[MAX_TOKEN_CHARS];
	char	realPassword[MAX_TOKEN_CHARS];
	qboolean foundPermissionCode;
	int realPermissions;
	qboolean isThisUser;

	if ( !HasPermission(ent, PERMISSION_PERMISSION) ) //
		return;

	if ( trap_Argc() < 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mpermission <username> <permission-symbol> <permission-symbol-2>...<permission-symbol-n> \n\""));
		return;
	}	

	trap_Argv( 1, username, sizeof( username ) );

	if ( Q_stricmp( username, ent->client->sess.username ) == 0 )
	{
		isThisUser = qtrue;
	}
	else
	{
		realPermissions = ent->client->sess.permissions;
		Q_strncpyz( realUsername, ent->client->sess.username, sizeof( ent->client->sess.username ) );
		Q_strncpyz( realPassword, ent->client->sess.password, sizeof( ent->client->sess.password ) );
		isThisUser = qfalse;
	}

	if ( (!isThisUser) && (!ReadUserFile(ent, username, 0)) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Failed change permissions for user %s\n\"", username) );
	}
	else
	{
		for( j = 2 ; j < trap_Argc() ; j++ )
		{
			trap_Argv( j, buffer, sizeof( buffer ) );
			
			foundPermissionCode = qfalse;	

			for( i = 0 ; i < MAX_USER_PERMISSION_CODE ; i++ )
			{
				if ( Q_stricmp(permissionCode[i], buffer) == 0 )
				{
					foundPermissionCode = qtrue;

					if  ( ((1 << i) & (ent->client->sess.permissions)) == 0 && ((1 << i) & (realPermissions)) != 0 ) 
					{
						ent->client->sess.permissions |= (1 << i);
						trap_SendServerCommand( ent-g_entities, va("print \"^6%s^7 now has ^5%s\n\"", username, permissionCode[i] ));
	
					}
					else if ( ((1 << i) & (realPermissions)) == 0 )
					{
						trap_SendServerCommand( ent-g_entities, va("print \"You don't have ^5%s\n\"", permissionCode[i] ));
					}
					else
					{
						trap_SendServerCommand( ent-g_entities, va("print \"^6%s^7 already has ^5%s\n\"", username, permissionCode[i] ));
					}

					break;
				}
			}
			
			if ( !foundPermissionCode )
				trap_SendServerCommand( ent-g_entities, va("print \"^7No such permission code ^5%s\n\"", buffer ));

		}
		
		WriteUserFile( ent, qtrue, ent->client->sess.username, ent->client->sess.password, ent->client->sess.permissions, 0, 0 );
	}

	// Recover our own user data
	if ( !isThisUser )
	{
		ent->client->sess.permissions = realPermissions;
		Q_strncpyz( ent->client->sess.username, realUsername, sizeof(ent->client->sess.username) );
		Q_strncpyz( ent->client->sess.password, realPassword, sizeof(ent->client->sess.password) );
	}
}

void Cmd_mForbid_f( gentity_t *ent ) 
{
	int		i, j;
	char	buffer[MAX_TOKEN_CHARS];
	char	username[MAX_TOKEN_CHARS];
	char	realUsername[MAX_TOKEN_CHARS];
	char	realPassword[MAX_TOKEN_CHARS];
	qboolean foundPermissionCode;
	int realPermissions;

	qboolean isThisUser;

	if ( !HasPermission(ent, PERMISSION_PERMISSION) ) //
		return;

	if ( trap_Argc() < 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mforbid <username> <permission-symbol> <permission-symbol-2>...<permission-symbol-n>\n\""));
		return;
	}	

	trap_Argv( 1, username, sizeof( username ) );

	if ( Q_stricmp( username, ent->client->sess.username ) == 0 )
	{
		isThisUser = qtrue;
	}
	else
	{
		realPermissions = ent->client->sess.permissions;
		Q_strncpyz( realUsername, ent->client->sess.username, sizeof( ent->client->sess.username ) );
		Q_strncpyz( realPassword, ent->client->sess.password, sizeof( ent->client->sess.password ) );
		isThisUser = qfalse;
	}

	if ( (!isThisUser) && (!ReadUserFile(ent, username, 0)) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Failed change permissions for user %s\n\"", username) );
	}
	else
	{
		for( j = 2 ; j < trap_Argc() ; j++ )
		{
			trap_Argv( j, buffer, sizeof( buffer ) );
	
			foundPermissionCode = qfalse;

			for( i = 0 ; i < MAX_USER_PERMISSION_CODE ; i++ )
			{
				if ( Q_stricmp(permissionCode[i], buffer) == 0 )
				{
					foundPermissionCode = qtrue;

					if  ( ((1 << i) & (ent->client->sess.permissions)) != 0 && ((1 << i) & (realPermissions)) != 0 ) 
					{
						ent->client->sess.permissions &= ~(1 << i);
						trap_SendServerCommand( ent-g_entities, va("print \"^5%s^7 removed from user ^6%s\n\"", permissionCode[i], username ));
	
					}
					else if ( ((1 << i) & (realPermissions)) == 0 )
					{
						trap_SendServerCommand( ent-g_entities, va("print \"You don't have ^5%s\n\"", permissionCode[i] ));
					}
					else
					{
						trap_SendServerCommand( ent-g_entities, va("print \"^6%s^7 didn't have ^5%s\n\"", username, permissionCode[i] ));
					}

					break;
				}
			}

			if ( !foundPermissionCode )
				trap_SendServerCommand( ent-g_entities, va("print \"^7No such permission code ^5%s\n\"", buffer ));
		}
		
		WriteUserFile( ent, qtrue, ent->client->sess.username, ent->client->sess.password, ent->client->sess.permissions, 0, 0 );
	}

	// Recover our own user data
	if ( !isThisUser )
	{
		ent->client->sess.permissions = realPermissions;
		Q_strncpyz( ent->client->sess.username, realUsername, sizeof(ent->client->sess.username) );
		Q_strncpyz( ent->client->sess.password, realPassword, sizeof(ent->client->sess.password) );
	}
}

void Cmd_mLicence_f( gentity_t *ent )
{
	int		i, j;
	char	buffer[MAX_TOKEN_CHARS];
	char	username[MAX_TOKEN_CHARS];
	char	realUsername[MAX_TOKEN_CHARS];
	char	realPassword[MAX_TOKEN_CHARS];
	int realPermissions;
	qboolean isThisUser;

	if ( !HasPermission(ent, PERMISSION_PERMISSION) ) //
		return;

	if ( trap_Argc() < 2 ) 
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mforbid <username> <permission-symbol> <permission-symbol-2>...<permission-symbol-n>\n\""));
		return;
	}	

	trap_Argv( 1, username, sizeof( username ) );

	if ( Q_stricmp( username, ent->client->sess.username ) == 0 )
	{
		isThisUser = qtrue;
	}
	else
	{
		realPermissions = ent->client->sess.permissions;
		Q_strncpyz( realUsername, ent->client->sess.username, sizeof( ent->client->sess.username ) );
		Q_strncpyz( realPassword, ent->client->sess.password, sizeof( ent->client->sess.password ) );
		isThisUser = qfalse;
	}

	if ( (!isThisUser) && (!ReadUserFile(ent, username, 0)) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Failed change permissions for user %s\n\"", username) );
	}
	else
	{
		for( j = 2 ; j < trap_Argc() ; j++ )
		{
			trap_Argv( j, buffer, sizeof( buffer ) );
	
			for( i = 0 ; i < MAX_USER_PERMISSION_CODE ; i++ )
			{
				if ( Q_stricmp(permissionCode[i], buffer) == 0 )
				{
					if  ( ((1 << i) & (ent->client->sess.permissions)) != 0 && ((1 << i) & (realPermissions)) != 0 ) 
					{
						ent->client->sess.permissions &= ~(1 << i);
						trap_SendServerCommand( ent-g_entities, va("print \"%s removed from user %s\n\"", permissionCode[i], username ));
	
					}
					else
					{
						trap_SendServerCommand( ent-g_entities, va("print \"%s didn't have %s\n\"", username, permissionCode[i] ));
					}

					break;
				}
			}
		}
		
		WriteUserFile( ent, qtrue, ent->client->sess.username, ent->client->sess.password, ent->client->sess.permissions, 0, 0 );
	}

	// Recover our own user data
	if ( !isThisUser )
	{
		ent->client->sess.permissions = realPermissions;
		Q_strncpyz( ent->client->sess.username, realUsername, sizeof(ent->client->sess.username) );
		Q_strncpyz( ent->client->sess.password, realPassword, sizeof(ent->client->sess.password) );
	}


}

void G_Unempower( gentity_t * ent )
{
		int i;

		//unempower
		for ( i = 0 ; i < NUM_FORCE_POWERS ; i++ )
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;

		ent->client->godModFlags &= ~GMOD_EMPOWERED; 
		ent->client->ps.eFlags &= ~EF_BODYPUSH;

		WP_InitForcePowers( ent );
}


void Cmd_mempower_f( gentity_t *ent )
{
	int i;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_EMPOWER) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( !(ent->client->godModFlags & GMOD_EMPOWERED) )
	{

		for ( i = 0 ; i < NUM_FORCE_POWERS ; i++ )
		{
			ent->client->ps.fd.forcePowersKnown |= (1 << i);
			ent->client->ps.fd.forcePowerLevel[i] = FORCE_LEVEL_3;
		}

		ent->client->godModFlags |= GMOD_EMPOWERED; // forever

		trap_SendServerCommand( ent-g_entities, va("print \"You are empowered\n\""));
	}
	else
	{
		G_Unempower( ent );
		trap_SendServerCommand( ent-g_entities, va("print \"You have relinquished your powers\n\""));
	}
}

void Cmd_mlightme_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	int brightness;

	if ( !HasPermission(ent, PERMISSION_TESTING) )
		return;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_SCALE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage : mlightme <brightness>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );
	
	brightness = atoi(buffer);

	ent->s.constantLight = brightness;

	trap_LinkEntity( ent );

}

void Cmd_mscaleme_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	vec_t factor, oldFactor;
	vec3_t tempVect;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_SCALE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (1 = normal scale factor): mscaleme <scale-factor>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	factor = atof(buffer);

	if ( factor < 0.05 )
		factor = 0.05; //smaller than this is close to invisible

	oldFactor = ent->modelScale[0];

	VectorSet(ent->modelScale, factor, factor, factor);

	//ent->client->ps.viewheight = -16;

	//VectorScale( ent->r.maxs, factor/oldFactor, tempVect);
	//VectorCopy( tempVect, ent->r.maxs );

	//VectorScale( ent->r.mins, factor/oldFactor, tempVect);
	//VectorCopy( tempVect, ent->r.mins );

	//VectorScale( ent->r.maxs, factor/oldFactor, tempVect);
	//VectorCopy( tempVect, ent->r.absmax );

	//VectorScale( ent->r.mins, factor/oldFactor, tempVect);
	//VectorCopy( tempVect, ent->r.absmin );
 
	ent->client->ps.crouchheight = CROUCH_VIEWHEIGHT*factor;
	ent->client->ps.standheight = DEFAULT_VIEWHEIGHT*factor;

	factor = factor * 100;

	ent->client->ps.iModelScale = (int) factor;
	//trap_LinkEntity(ent);
}

void Cmd_mScalet_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	vec_t factor;

	gentity_t	*target;
	trace_t		trace;
	vec3_t		src, dest, vf;
	vec3_t		viewspot;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_SCALE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (1 = normal scale factor): mscalet <scale-factor>\n\""));
		return;
	}

	target = AtCrossHairs( ent, MASK_NPCSOLID, 0 );

	if ( !target )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Where's the thing to scale?\n\""));
		return;
	}

	if ( (!(target->s.eType == ET_NPC && (ent->client->sess.permissions & PERMISSION_NPC_ADMIN)) 
		|| ( target->s.eType != ET_NPC &&  (ent->client->sess.permissions & PERMISSION_PLACE_ADMIN)))
		&& ( target->creator != ent - g_entities + 1) )
		return;

	trap_Argv( 1, buffer, sizeof( buffer ) );

	factor = atof(buffer);

	if ( factor < 0.05 )
		factor = 0.05; //smaller than this is close to invisible

	VectorSet(target->modelScale, factor, factor, factor);

	if ( target->client &&  target->s.NPC_class != CLASS_VEHICLE )
	{
		ent->client->ps.crouchheight = CROUCH_VIEWHEIGHT*factor;
		ent->client->ps.standheight = DEFAULT_VIEWHEIGHT*factor;
	}

	factor = factor * 100;
	if ( target->client )
	{
		
		target->client->ps.iModelScale = (int) factor;
	}
	else
	{
		target->s.iModelScale = factor;
	}

	if ( target->s.eType == ET_MOVER )
	{
		LoadSizes( target );
	}

	AddSpawnVars( target, va("model2scale %i",target->s.iModelScale) );
}

void Cmd_mscale_f( gentity_t *ent )
{
	char		buffer[MAX_TOKEN_CHARS];
	vec_t factor;

	gentity_t	*target;

	if ( (!g_cheats.integer) && !(ent->client->sess.permissions & PERMISSION_SCALE) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage (1 = normal scale factor): mscalet <scale-factor>\n\""));
		return;
	}

	trap_SendServerCommand( ent-g_entities, va("print \"This command scales objects. To scale yourself, use /mscaleme.\n\""));

	target = SelectedEnt( ent );

	if ( !target )
		return;

	if ( !(ent->client->sess.permissions & PERMISSION_PLACE_ADMIN) && ( target->creator != ent - g_entities + 1) )
		return;

	trap_Argv( 1, buffer, sizeof( buffer ) );

	factor = atof(buffer);

	VectorSet(target->modelScale, factor, factor, factor);

	factor = factor * 100;
	if ( target->client )
	{
		
		target->client->ps.iModelScale = (int) factor;
	}
	else
	{
		target->s.iModelScale = factor;
	}

	if ( target->s.eType == ET_MOVER )
	{
		LoadSizes( target );
	}

	AddSpawnVars( target, va("model2scale %i",target->s.iModelScale) );
}

void Cmd_mList_f( gentity_t *ent )
{
	int numEntries;
	char buffer[MAX_TOKEN_CHARS];
	char list[2048];
	char* listptr;
	int i, length;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  mlist <optional-folder-name>\n\""));
		return;
	}

	if ( trap_Argc() == 1 )
	{
		numEntries = trap_FS_GetFileList("models/map_objects", "/", list, 2048 );

		listptr = list;

		trap_SendServerCommand( ent-g_entities, va("print \"Use /mlist <folder-name> to see the contents of one of these folders.\n\""));

		for( i = 0 ; i < numEntries ; i++ )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
			listptr += (strlen( listptr ) + 1);
		}

		return;
	}
	else
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );

		if ( strstr(buffer, "models") == buffer )  
		{
			// scan through full model path
			numEntries = trap_FS_GetFileList(buffer, ".md3", list, 2048 );

			listptr = list;

			for( i = 0 ; i < numEntries ; i++ )
			{
				length = strlen( listptr );

				if ( strchr(listptr,'/') || strchr(listptr,'\\') )
				{
					// this list contains folders, display only the folders
					trap_SendServerCommand( ent-g_entities, va("print \"Use /mlist <folder-name> to see the contents of one of these folders.\n\""));

					numEntries = trap_FS_GetFileList(buffer, "/", list, 2048 );
					listptr = list;

					for( i = 0 ; i < min(numEntries,100) ; i++ )
					{
						trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
						listptr += (strlen( listptr ) + 1);
					}

					return;
				}
				listptr += (length + 1);
			}
		}
		else
			numEntries = trap_FS_GetFileList(va("models/map_objects/%s", buffer), ".md3", list, 2048 );

		listptr = list;

		if ( numEntries == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"No models found. Did you enter the right folder name?\n\""));
		}

		for( i = 0 ; i < numEntries ; i++ )
		{
			length = strlen( listptr );
			*(listptr + length - 4) = 0;
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
			listptr += (length + 1);
		}
	}
}

void Cmd_mListsnd_f( gentity_t *ent )
{
	int numEntries;
	char buffer[MAX_TOKEN_CHARS];
	char list[2048];
	char* listptr;
	int i, length, lastnum;
	qboolean scanDirs, listedSomething;

	listedSomething = qfalse;
	scanDirs = qfalse;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  mlistsnd <optional-folder-name>\n\""));
		return;
	}

	if ( trap_Argc() == 1 )
	{
		numEntries = trap_FS_GetFileList("sound", "/", list, 2048 );

		G_Printf( "sfx: %i", numEntries );

		listptr = list;

		trap_SendServerCommand( ent-g_entities, va("print \"Use /mlistsnd <folder-name> to see the contents of one of these folders.\n\""));

		for( i = 0 ; i < numEntries ; i++ )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
			listptr += (strlen( listptr ) + 1);
		}

		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );


	// this list contains folders, display only the folders
	trap_SendServerCommand( ent-g_entities, va("print \"Use /mlistsnd <folder-name> to see the contents of one of these folders.\n\""));

	numEntries = trap_FS_GetFileList(va("sound/%s", buffer), "/", list, 2048 );

	if ( numEntries > 0 )
		listedSomething = qtrue;

	listptr = list;

	for( i = 0 ; i < min(numEntries,100) ; i++ )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
		listptr += (strlen( listptr ) + 1);
	}
	lastnum = numEntries;

	// scan through full path
	numEntries = trap_FS_GetFileList(va("sound/%s", buffer), ".mp3", list, 2048 );


	if ( numEntries > 0 )
		listedSomething = qtrue;

	listptr = list;

	if ( numEntries == 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"No sounds found. Did you enter the right folder name?\n\""));
	}

	for( i = 0 ; i < min(numEntries, 100-lastnum) ; i++ )
	{
		length = strlen( listptr );
			
		if ( !strchr(listptr,'/') && !strchr(listptr,'\\') )
		{
			*(listptr + length - 4) = 0;
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
		}

		listptr += (length + 1);
	}

	numEntries = trap_FS_GetFileList(va("sound/%s", buffer), ".wav", list, 2048 );
	
	if ( numEntries > 0 )
		listedSomething = qtrue;

	listptr = list;

	for( i = 0 ; i < min(numEntries, 128) ; i++ )
	{
		length = strlen( listptr );
			
		if ( !strchr(listptr,'/') && !strchr(listptr,'\\') )
		{
			*(listptr + length - 4) = 0;
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
		}

		listptr += (length + 1);
	}

	if ( !listedSomething )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"No sounds found. Did you enter the right folder name?\n\""));
	}
}

void Cmd_mlistfx_f( gentity_t *ent )
{
	int numEntries;
	char buffer[MAX_TOKEN_CHARS];
	char list[2048];
	char* listptr;
	int i, length;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  mlistfx <optional-folder-name>\n\""));
		return;
	}

	if ( trap_Argc() == 1 )
	{
		numEntries = trap_FS_GetFileList("effects/", "/", list, 2048 );

		listptr = list;

		trap_SendServerCommand( ent-g_entities, va("print \"Use /mlistfx <folder-name> to see the contents of one of these folders.\n\""));

		for( i = 0 ; i < numEntries ; i++ )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
			listptr += (strlen( listptr ) + 1);
		}
	}
	else
	{
		trap_Argv( 1, buffer, sizeof( buffer ) );

		numEntries = trap_FS_GetFileList(va("effects/%s", buffer), ".efx", list, 2048 );

		listptr = list;

		if ( numEntries == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"No models found. Did you enter the right folder name?\n\""));
		}

		for( i = 0 ; i < numEntries ; i++ )
		{
			length = strlen( listptr );
			*(listptr + length - 4) = 0;
			trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", listptr) );
			listptr += (length + 1);
		}
	}
}

void Cmd_mlistso_f( gentity_t *ent )
{
	int numEntries;
	gitem_t	*it;
	char buffer[MAX_TOKEN_CHARS];
	int i, length;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS ) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 2)
	{
		trap_SendServerCommand( ent-g_entities, "print \"Command usage:  mlistso <optional-object-type>\n\"");
		return;
	}

    if ( trap_Argc() == 1 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"You can find out how to place the following kinds of special object;\n\"");
		trap_SendServerCommand( ent-g_entities, "print \"^5/mlistso weapons^7 \n\"");
		trap_SendServerCommand( ent-g_entities, "print \"^5/mlistso items^7 \n\"");
		trap_SendServerCommand( ent-g_entities, "print \"^5/mlistso machines^7 - machines players can interact with\n\"");

		return;
	}
	
	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( stricmp(buffer, "machines")== 0 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace gun^7\n\"") );
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace ammounit^7\n\"") );
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace shieldunit^7\n\"") );
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace turret^7\n\"") );
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace miniturret^7\n\"") );
		trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace death	^7\n\"") );

	}
	else if ( stricmp(buffer, "weapons")== 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"The following /mplace commands place weapons;\n\"");
	
		for ( it = bg_itemlist + 1 ; it->classname ; it++) 
		{
			if ( it->makermodName && *(it->makermodName)!= 0 && it->giType == IT_WEAPON )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace %s^7\n\"", it->makermodName) );
		}
	}
	else if ( stricmp(buffer, "items")== 0 )
	{
		trap_SendServerCommand( ent-g_entities, "print \"The following /mplace commands place items;\n\"");
	
		for ( it = bg_itemlist + 1 ; it->classname ; it++) 
		{
			if ( it->makermodName && *(it->makermodName)!= 0 && it->giType != IT_WEAPON )
				trap_SendServerCommand( ent-g_entities, va("print \"^5/mplace %s^7\n\"", it->makermodName) );
		}

	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Unrecognised mlistso keyword, %s.\n\"", buffer));
		return;
	}
	

}


void Cmd_mPClass_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	char value[MAX_TOKEN_CHARS];
	char* args, *dataPtr, *commaPtr;
	char spawnString[MAX_SPAWN_CHARS];
	int j;

	args = 0;

	if ( !HasPermission(ent, PERMISSION_TESTING) ) //!g_cheats.integer && ) ) //
		return;

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  mpclass <classname> <optional spawning arguments>\n\""));
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	if ( strchr(buffer, ',') )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO SPAWN: Class name must not include commas\n\""));
		return;
	}

	if ( trap_Argc() > 2 )
		args = ConcatArgs( 2 );

	//level.spawning = qtrue;
	level.numSpawnVars = 0;

	Com_sprintf( spawnString, sizeof(spawnString), "classname,%s,origin,%f %f %f%s%s,", buffer,
		            ent->client->sess.mark[0],  ent->client->sess.mark[1], ent->client->sess.mark[2],
				    args == 0 ? "" : ",",  args == 0 ? "" : args );

	// ------ another function

	dataPtr = spawnString;

	trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", dataPtr ));

	j= 0;

	while( qtrue ) //(test = sscanf(dataPtr, "32%s,32%s,", buffer, value))  == 2 )
	{
		commaPtr = strchr( dataPtr, ',' );
		
		if (!commaPtr)
			break;

		Q_strncpyz( buffer, dataPtr, commaPtr - dataPtr + 1 );

		dataPtr = commaPtr;

		dataPtr++;


		commaPtr = strchr( dataPtr, ',' );
		
		if (!commaPtr)
			break;

		Q_strncpyz( value, dataPtr, commaPtr - dataPtr + 1);

		dataPtr = commaPtr;

		dataPtr++;


		AddSpawnField( buffer, value );

		trap_SendServerCommand( ent-g_entities, va("print \"%s, %s\n\"", buffer, value ));
	}

	trap_SendServerCommand( ent-g_entities, va("print \"number of recs:%i\n\"", j ));

	G_SpawnGEntityFromSpawnVars(qfalse);


	/*obj = G_Spawn();

	if ( !obj )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of\n\""));
		return;
	}

	if ( trap_Argc() == 5 )
	{
		// Set position of our entity
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			trap_Argv( i + 1, buffer, sizeof( buffer ) );
			obj->s.origin[i] = atof( buffer );
		}



		// Set the model name.
		trap_Argv( 4, buffer, sizeof( buffer ) );
	}*/
}


void Cmd_mplace_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
//	char test[MAX_TOKEN_CHARS];
	gentity_t *obj;
	int i;
	int size;
	vec3_t pos;
	fileHandle_t	f;
	qboolean fullPath;
	//alloc_t *alloc;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  ^5/mplace <foldername/modelname>^7\n\""));
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage:  ^5/mplace <special-ob-name> <optional-special-ob-parameters>^7 \n\""));
		return;
	}

	if ( level.entCount > MAX_GENTITIES - g_objectMargin.integer )
	{
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Unable to place. There are already too many objects.\n\""));

		return;
	}

	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
		VectorCopy( ent->r.currentOrigin, pos );
	else
		VectorCopy( ent->client->sess.mark, pos );

	if ( !CanExistHere(pos, ENT_FX) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You aren't allowed to place obs here. Please try /mmark somewhere else.\n\""));
		return;
	}

	if ( level.entCount > (MAX_GENTITIES - g_objectMargin.integer))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of entities\n\""));
		return;
	}
	obj = G_Spawn();

	if ( !obj )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of entities\n\""));
		return;
	}

	VectorCopy( ent->client->sess.mark, obj->s.origin );

	// Set the model name.
	trap_Argv( 1, buffer, sizeof( buffer ) );

//	if ( FS_FileExists( buffer ) != qtrue )
//	{
//		trap_SendServerCommand( ent-g_entities, va("print \"No such map on the server.\n\""));
//		return;
//	}

	if ( !strchr( buffer, '/' ) && !strchr( buffer, '\\' ) )
	{
		//Spawn our special object

		gitem_t	*it;
		char* classname;
		char spawnString[MAX_STRING_CHARS];
		char append[MAX_STRING_CHARS];

		append[0] = '\0';
	
		if ( stricmp(buffer, "ammounit") == 0 )
			classname = "misc_ammo_floor_unit";
		else if ( stricmp(buffer, "shieldunit") == 0 )
			classname = "misc_shield_floor_unit";
		else if ( stricmp(buffer, "miniturret") == 0 )
			classname = "misc_turretG2";
		else if ( stricmp(buffer, "turret") == 0 )
			classname = "misc_turret";
		else if ( stricmp(buffer, "gun") == 0 )
			classname = "emplaced_gun";
		else if ( stricmp(buffer, "deathturret") == 0 )
		{
			classname = "misc_turretG2";
			sprintf( append, "spawnflags 8" );
		}
		else
		{
			for ( it = bg_itemlist + 1 ; it->classname ; it++) {
				if ( it->makermodName && (stricmp(buffer, it->makermodName) == 0) )
					break;
			}

			if ( !it->classname )
			{
				trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Unrecognised special object name - %s.\n\"", buffer));
				return;
			}

			classname = it->classname;
		}

		obj->creator = ent - g_entities + 1;


		ent->client->manipulating = obj-g_entities;

		sprintf( spawnString, "classname %s origin \"%i %i %i\" %s", classname, (int)ent->client->sess.mark[0], (int)ent->client->sess.mark[1], (int)ent->client->sess.mark[2], append );

		if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
		{
			// if things like the the turret start at 0 0 0 some of their models don't display or grab properly
			VectorCopy( ent->r.currentOrigin, obj->s.origin );
		}

		AddSpawnVars( obj, spawnString );

		if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Object grabbed. Use /mgrabbing to turn off auto-grabbing.\n\"", obj-g_entities,vtos(obj->s.origin)));
			GrabObject( obj, ent );
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Entity placed:%i  Origin: %s\n\"", obj-g_entities,vtos(obj->s.origin)));
			obj->grabbing = qfalse;
		}
		
		return;
	}


	if ( strstr(buffer, "models") == buffer )
	{
		i = trap_FS_FOpenFile( va( "%s.md3", buffer), &f, FS_READ);
		fullPath = qtrue;
	}
	else
	{
		i = trap_FS_FOpenFile( va( "models/map_objects/%s.md3", buffer), &f, FS_READ);
		fullPath = qfalse;
	}

	if (!f || i <= 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Model unavailable on server. Did you get the name right?\n\""));
		G_FreeEntity( obj );
		return;
	}

	trap_FS_FCloseFile(f);

	obj->model = EMPTY_MODEL;
	//obj->spawnflags |= 4;
	if ( fullPath )
		//obj->model2 = G_NewString2( va( "%s.md3", buffer) );//, obj-g_entities
		obj->model2 = G_NewStringForEnt (va ("%s.md3", buffer), obj->s.number);
	else{
		//obj->model2 = G_NewString2( va( "models/map_objects/%s.md3", buffer));//,obj-g_entities );
		obj->model2 = G_NewStringForEnt (va ("models/map_objects/%s.md3", buffer), obj->s.number);
	}




//	if ( trap_Argc() == 6 )
//	{
//		trap_Argv( 5, buffer, sizeof( buffer ) );
//	}
//	else
//	{
//		trap_Argv( 2, buffer, sizeof( buffer ) );
//	}

//	size = atoi(buffer);

	size = 32;

//	if ( trap_Argc() > 5 )
//	{
//		trap_Argv( 6, buffer, sizeof( buffer ) );
//		obj->classname = G_NewString (buffer);
//	}
//	else
	obj->classname = "mplaced";
	
	// which client created it?
	obj->creator = ent - g_entities + 1;


	// move editor origin to pos
	VectorCopy( obj->s.origin, obj->s.pos.trBase );
	VectorCopy( obj->s.origin, obj->r.currentOrigin );

	//Com_sprintf(temp, MAX_TOKEN_CHARS, "#%s", obj->model);
	//trap_SetBrushModel( obj, temp ); 
	
	//  Q_strncpyz(test,va("*%s",obj->model2),sizeof(obj->model2));
	//Q_strncpyz(test,va("*79"),sizeof(test));//,obj->model2),sizeof(obj->model2));
	//trap_SetBrushModel(obj, test);
	obj->s.modelindex = G_ModelIndex( obj->model );
	obj->s.modelindex2 = G_ModelIndex( obj->model2 );
	//Q_strncpyz(obj->model2,va("*79"),sizeof(obj->model2));
	//  obj->model = obj->model2;
	//obj->s.modelindex = G_ModelIndex( obj->model );
	//obj->s.modelindex2 = G_ModelIndex( obj->model2 );
	//obj->s.eFlags = EF_PERMANENT;
	//  trap_SetBrushModel( obj, test);
	//  G_Printf("%i %i %i  %i",CS_MODELS,obj->s.modelindex,obj->s.modelindex2, atoi( test + 1 ));
	//obj->s.modelindex2 = obj->s.modelindex;
	//obj->s.modelindex2 = G_ModelIndex( obj->model2 );
	//  obj->r.bmodel = qfalse;

	if ( !LoadSizes(obj) )
	{
		VectorSet (obj->r.mins, -1*size, -1*size, -1*size);
		VectorSet (obj->r.maxs, size, size, size);
		obj->ymins = obj->r.mins[0];
		obj->ymaxs = obj->r.maxs[0];
	}

	obj->s.eType = ET_MOVER;
	//laserTrap->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	//laserTrap->s.pos.trType = TR_GRAVITY;
	obj->r.contents = CONTENTS_SOLID;//|CONTENTS_NOCLIP//Test-Scooper

	//VectorSet( laserTrap->r.mins, -LT_SIZE, -LT_SIZE, -LT_SIZE );
	//VectorSet( laserTrap->r.maxs, LT_SIZE, LT_SIZE, LT_SIZE );
	obj->clipmask = 0; //MASK_SOLID;
	//SnapVector( laserTrap->s.pos.trBase );			// save net bandwidth
	
	//SnapVector( laserTrap->s.pos.trDelta );			// save net bandwidth

	//obj->s.apos.trType = TR_GRAVITY;
	//obj->s.apos.trTime = level.time;
	//obj->s.apos.trBase[YAW] = rand()%360;
	//obj->s.apos.trBase[PITCH] = rand()%360;
	//obj->s.apos.trBase[ROLL] = rand()%360;

	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	//ent->s.solid = SOLID_BBOX;
	VectorClear( ent->s.pos.trDelta );
	VectorClear( obj->s.angles );
	VectorClear( obj->r.currentAngles );
	VectorCopy( obj->s.angles, obj->s.apos.trBase );
	VectorSet(obj->modelScale, 1, 1, 1);

	obj->s.iModelScale = 100;

	//ent->s.eFlags = EF_PERMANENT;


	trap_LinkEntity (obj);
	

	ent->client->manipulating = obj-g_entities;

	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Object grabbed. Use /mgrabbing to turn off auto-grabbing.\n\"", obj-g_entities,vtos(obj->s.origin)));
		GrabObject( obj, ent );
	}
	else
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Entity placed:%i  Origin: %s\n\"", obj-g_entities,vtos(obj->s.origin)));
		obj->grabbing = qfalse;
	}
	//trap_TrueFree((void **)&AllocPointerObj);
}

void Cmd_mplacefx_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	char forbidden[MAX_TOKEN_CHARS];
	gentity_t *obj;
	int i, delay;
	int size;
	char* ptr, *token;
	vec3_t pos;
	fileHandle_t	f;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 3 && trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mplacefx <effectname> <delay-between-firings-in-milliseconds> <optional-random-delay-component-in-ms>\n1 second is 1000 milliseconds.\n\""));
		return;
	}

	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
		VectorCopy( ent->r.currentOrigin, pos );
	else
		VectorCopy( ent->client->sess.mark, pos );

	if ( !CanExistHere(pos, ENT_OBJECT) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: You aren't allowed to place effects here. Please try somewhere else.\n\""));
		return;
	}

	trap_Argv( 2, buffer, sizeof(buffer) );

	delay =  atoi(buffer);


//	if ( delay < 200 && !(ent->client->sess.permissions & PERMISSION_TESTING) )
//	{
//		trap_SendServerCommand( ent-g_entities, "print \"The delay must be 200 milliseconds or more.\n\"");
//		return;
//	}

	if ( level.entCount > MAX_GENTITIES - g_objectMargin.integer )
	{
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Unable to place. There are already too many objects.\n\""));
		return;
	}

	obj = G_Spawn();

	if ( !obj )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Run out of objects.\n\""));
		return;
	}

	//if ( trap_Argc() == 5 )
	//{
	//	// Set position of our entity
	//	for ( i = 0 ; i < 3 ; i++ ) 
	//	{
	//		trap_Argv( i + 1, buffer, sizeof( buffer ) );
	//		obj->s.origin[i] = atof( buffer );
	//	}

	//	// Set the model name.
	//	trap_Argv( 4, buffer, sizeof( buffer ) );


	//}
	//else
	//{
	VectorCopy( ent->client->sess.mark, obj->s.origin );

	// Get the effect name.
	trap_Argv( 1, buffer, sizeof( buffer ) );


	i = trap_FS_FOpenFile( va( "effects/%s.efx", buffer), &f, FS_READ);

	if (!f || i <= 0)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Effect unavailable on server. Did you get the name right?\n\""));
		G_FreeEntity( obj );
		return;
	}

	trap_FS_FCloseFile(f);

	// Check this fx hasn't been forbidden
	ptr = forbidden;

	Q_strncpyz( forbidden,  g_forbiddenFXs.string, sizeof(buffer) );
	token = COM_ParseExt(&ptr, qfalse);

	// check the npc isn't forbidden
	while ( strlen(token) > 0 )
	{
		if ( stricmp(token, buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Effect %s is not allowed\n\"", buffer));
			G_FreeEntity( obj );
			return;
		}
		token = COM_ParseExt(&ptr, qfalse);
	}

	// switch the direction of slashes and check again
	Q_strncpyz( forbidden,  g_forbiddenFXs.string, sizeof(buffer) );

	for( ptr = forbidden ; *ptr != 0 ; ptr++ )
	{
		if ( *ptr == '/' )
			*ptr = '\\';
		else if ( *ptr == '\\' )
			*ptr = '/';
	}

	ptr = forbidden;

	token = COM_ParseExt(&ptr, qfalse);

	// check the npc isn't forbidden
	while ( strlen(token) > 0 )
	{
		if ( stricmp(token, buffer) == 0 )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Effect %s is not allowed\n\"", buffer));
			G_FreeEntity( obj );
			return;
		}
		token = COM_ParseExt(&ptr, qfalse);
	}


	obj->s.modelindex = G_EffectIndex( buffer );
	if (obj->s.modelindex > (MAX_FX-10))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Max different effects hit.\n\"", buffer));
		RemoveConfig(buffer, CS_EFFECTS, MAX_FX, 0);
		G_FreeEntity( obj );
		return;
	}
	//G_Printf("%i - %i",obj->s.modelindex, MAX_FX-10);


	// keep a record of the effect name for msaveobs
	//obj->target6 = G_NewString2( buffer );//, obj-g_entities
	obj->target6 = G_NewStringForEnt (buffer, obj->s.number);

	size = 32;

	obj->classname = "mplaced";
	
	// which client created it?
	obj->creator = ent - g_entities + 1;


	// move editor origin to pos
	VectorCopy( obj->s.origin, obj->s.pos.trBase );
	VectorCopy( obj->s.origin, obj->r.currentOrigin );


	//trap_SetBrushModel( obj, obj->model );
	//obj->s.modelindex = G_ModelIndex( obj->model );


	VectorSet (obj->r.mins, -1*size, -1*size, -1*size);
	VectorSet (obj->r.maxs, size, size, size);

	obj->ymins = obj->r.mins[0];
	obj->ymaxs = obj->r.maxs[0];

	//obj->s.eType = ET_MOVER;
	//laserTrap->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	//laserTrap->s.pos.trType = TR_GRAVITY;
	obj->r.contents = CONTENTS_FOG;

	//VectorSet( laserTrap->r.mins, -LT_SIZE, -LT_SIZE, -LT_SIZE );
	//VectorSet( laserTrap->r.maxs, LT_SIZE, LT_SIZE, LT_SIZE );
	obj->clipmask = 0;//MASK_SOLID;
	//SnapVector( laserTrap->s.pos.trBase );			// save net bandwidth
	
	//SnapVector( laserTrap->s.pos.trDelta );			// save net bandwidth

	//obj->s.apos.trType = TR_GRAVITY;
	//obj->s.apos.trTime = level.time;
	//obj->s.apos.trBase[YAW] = rand()%360;
	//obj->s.apos.trBase[PITCH] = rand()%360;
	//obj->s.apos.trBase[ROLL] = rand()%360;

	obj->s.pos.trType = TR_STATIONARY;
	obj->s.pos.trTime = 0;
	obj->s.pos.trDuration = 0;
	//ent->s.solid = SOLID_BBOX;
	VectorClear( ent->s.pos.trDelta );


	// so give us the default of up
	VectorSet( obj->s.angles, -90, 0, 0 );


	//VectorCopy( ent->s.angles, obj->s.angles );
	VectorCopy( obj->s.angles, obj->s.apos.trBase );
	VectorCopy( obj->s.angles, obj->r.currentAngles );
	VectorSet(obj->modelScale, 1, 1, 1);

	//trap_LinkEntity (obj);

	ent->client->manipulating = obj-g_entities;

	trap_SendServerCommand( ent-g_entities, va("print \"entity placed:%i: %s\n\"", obj-g_entities,vtos(obj->s.origin)));

	//G_SpawnString( "fxFile", "", &fxFile );
	//// Get our defaults
	//G_SpawnInt( "delay", "200", &ent->delay );
	//G_SpawnFloat( "random", "0", &ent->random );
	//G_SpawnInt( "splashRadius", "16", &ent->splashRadius );
	//G_SpawnInt( "splashDamage", "5", &ent->splashDamage );

	// important info transmitted
	obj->s.eType = ET_FX;

	// Setup the delay
	//trap_Argv( 2, buffer, sizeof(buffer) );
	obj->delay = delay; //atoi(buffer);
	obj->s.speed = obj->delay;

	// Randomness
	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof(buffer) );
		obj->random = atoi(buffer);
	}
	else
	{
		obj->random = 0;
	}
	obj->s.time = obj->random;

	obj->s.modelindex2 = FX_STATE_OFF;

	obj->splashRadius = 16;
	obj->splashDamage = 0;

	// Give us a bit of time to spawn in the other entities, since we may have to target one of 'em
	obj->think = fx_runner_think; 
	obj->nextthink = level.time + 200;

	// Save our position and link us up!
	G_SetOrigin( obj, obj->s.origin );
	G_SetAngles( obj, obj->s.angles );
	AddUse( obj, fx_runner_use );

	trap_LinkEntity( obj );
	 
	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
	{
		fx_runner_think( obj );
		trap_SendServerCommand( ent-g_entities, va("print \"Effect grabbed. Use /mgrabbing to turn auto-grabbing on and off.\n\"", obj-g_entities,vtos(obj->s.origin)));
		GrabObject( obj, ent );
	}
	else
		obj->grabbing = qfalse;
	//trap_TrueFree((void **)&AllocPointerEfx);
}

//SnowTest
void Cmd_mweather_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	gentity_t *obj;
	int i;
	int size;
	vec3_t pos;

	if (!(ent->client->sess.permissions & PERMISSION_TESTING) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

		VectorCopy( ent->r.currentOrigin, pos );


	if ( level.entCount > (MAX_GENTITIES - g_objectMargin.integer))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR :Run out of entities\n\""));
		return;
	}

	obj = G_Spawn();

	if ( !obj )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: Run out of objects.\n\""));
		return;
	}

	VectorCopy( ent->client->sess.mark, obj->s.origin );


//	obj->s.modelindex = G_EffectIndex( buffer );

	// keep a record of the effect name for msaveobs
//	obj->target6 = G_NewString( buffer );

	size = 32;

	obj->classname = "mplaced";
	
	// which client created it?
	obj->creator = ent - g_entities + 1;


	// move editor origin to pos
	VectorCopy( obj->s.origin, obj->s.pos.trBase );
	VectorCopy( obj->s.origin, obj->r.currentOrigin );


	//trap_SetBrushModel( obj, obj->model );
	//obj->s.modelindex = G_ModelIndex( obj->model );


	VectorSet (obj->r.mins, -1*size, -1*size, -1*size);
	VectorSet (obj->r.maxs, size, size, size);

	obj->ymins = obj->r.mins[0];
	obj->ymaxs = obj->r.maxs[0];

	//obj->s.eType = ET_MOVER;
	//laserTrap->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	//laserTrap->s.pos.trType = TR_GRAVITY;
	obj->r.contents = CONTENTS_FOG;

	//VectorSet( laserTrap->r.mins, -LT_SIZE, -LT_SIZE, -LT_SIZE );
	//VectorSet( laserTrap->r.maxs, LT_SIZE, LT_SIZE, LT_SIZE );
	obj->clipmask = 0;//MASK_SOLID;
	//SnapVector( laserTrap->s.pos.trBase );			// save net bandwidth
	
	//SnapVector( laserTrap->s.pos.trDelta );			// save net bandwidth

	//obj->s.apos.trType = TR_GRAVITY;
	//obj->s.apos.trTime = level.time;
	//obj->s.apos.trBase[YAW] = rand()%360;
	//obj->s.apos.trBase[PITCH] = rand()%360;
	//obj->s.apos.trBase[ROLL] = rand()%360;

	obj->s.pos.trType = TR_STATIONARY;
	obj->s.pos.trTime = 0;
	obj->s.pos.trDuration = 0;
	//ent->s.solid = SOLID_BBOX;
	VectorClear( ent->s.pos.trDelta );


	// so give us the default of up
	VectorSet( obj->s.angles, -90, 0, 0 );


	//VectorCopy( ent->s.angles, obj->s.angles );
	VectorCopy( obj->s.angles, obj->s.apos.trBase );
	VectorCopy( obj->s.angles, obj->r.currentAngles );
	VectorSet(obj->modelScale, 1, 1, 1);

	//trap_LinkEntity (obj);

	ent->client->manipulating = obj-g_entities;

	trap_SendServerCommand( ent-g_entities, va("print \"entity placed:%i: %s\n\"", obj-g_entities,vtos(obj->s.origin)));

	//G_SpawnString( "fxFile", "", &fxFile );
	//// Get our defaults
	//G_SpawnInt( "delay", "200", &ent->delay );
	//G_SpawnFloat( "random", "0", &ent->random );
	//G_SpawnInt( "splashRadius", "16", &ent->splashRadius );
	//G_SpawnInt( "splashDamage", "5", &ent->splashDamage );

	// important info transmitted
	obj->s.eType = ET_FX;

	// Setup the delay
	//trap_Argv( 2, buffer, sizeof(buffer) );
//	obj->delay = delay; //atoi(buffer);
//	obj->s.speed = obj->delay;

/*	// Randomness
	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof(buffer) );
		obj->random = atoi(buffer);
	}
	else
	{
		obj->random = 0;
	}
	obj->s.time = obj->random;*/

	obj->s.modelindex2 = FX_STATE_OFF;

	obj->splashRadius = 16;
	obj->splashDamage = 0;

	// Give us a bit of time to spawn in the other entities, since we may have to target one of 'em
	obj->think = fx_runner_think; 
	obj->nextthink = level.time + 200;

	// Save our position and link us up!
	G_SetOrigin( obj, obj->s.origin );
	G_SetAngles( obj, obj->s.angles );
	AddUse( obj, fx_runner_use );

	trap_LinkEntity( obj );
	 
	if  ( ent->client->sess.makerModFlags & GMOD_GRABBING )
	{
		fx_runner_think( obj );
		trap_SendServerCommand( ent-g_entities, va("print \"Effect grabbed. Use /mgrabbing to turn auto-grabbing on and off.\n\"", obj-g_entities,vtos(obj->s.origin)));
		GrabObject( obj, ent );
	}
	else
		obj->grabbing = qfalse;

	sprintf( buffer, "classname fx_snow");

	AddSpawnVars( obj, buffer );
}


void Cmd_mPain_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	gentity_t *obj;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_FX) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 3 || trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mpain <damage> <optional-damage-radius>\n\""));
		return;
	}

	obj = SelectedEnt( ent );

	if ( obj == 0 )
		return;

	if ( IsConflictingClass(obj, NULL) || IsPainOrBreakable(obj,2) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This kind of object can't inflict pain.\n\""));
		return;
	}

	if ( trap_Argc() == 3 )
	{
		int radius;

		trap_Argv( 2, buffer, sizeof(buffer) );
		radius = atoi(buffer);

		if ( radius < 200 )
		{
			obj->splashRadius = radius; 
		}
		else
		{
			trap_SendServerCommand( ent-g_entities, va("print \"Radius too large. Please try a smaller one.\n\""));
			return;
		}

	}

	trap_Argv( 1, buffer, sizeof(buffer) );
	obj->splashDamage =  atoi(buffer);

	if ( obj->splashDamage > 0 )
		obj->spawnflags |= 4; // Apply damage
	else
		obj->spawnflags &= ~4; // Don't apply dammage

	trap_SendServerCommand( ent-g_entities, va("print \"Pain set\n\""));
}

void Cmd_mtelesp_f( gentity_t *ent )
{
	gentity_t *spawnPoint;
	vec3_t origin, avoid, angles;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_TELE) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mtelesp\n\""));
		return;
	}

	VectorCopy( ent->r.currentOrigin, origin );
	VectorCopy( origin, avoid );
	
	VectorClear( angles );

	spawnPoint = SelectRandomFurthestSpawnPoint( avoid, origin, angles, g_gametype.integer );

	TeleportPlayer( ent, spawnPoint->s.origin, angles );
}

void Cmd_mPushable_f( gentity_t *ent )
{
	gentity_t *obj;

	if ( !(ent->client->sess.permissions & PERMISSION_TESTING) )// !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() != 1 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mpushable\n\""));
		return;
	}

	obj = SelectedEnt( ent );

	if ( obj == 0 )
		return;

	obj->s.eType = ET_MISSILE;

	trap_LinkEntity(obj);
}

void Cmd_mBreakable_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	gentity_t *obj;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 4 || trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mbreakable <object-health> <optional-material-number> <optional-chunk-factor>\n\""));
		return;
	}


//  scooper said breakable npc spawners would produce unkillable ghost-like npc's. 
//  Haven't been able to reproduce
//	if ( obj->NPC_type != 0 )
//	{
//		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mbreakable <object-health> <optional-material-number> <optional-chunk-factor>\n\""));
//		return;
//	}

	obj = SelectedEnt( ent );

	if ( obj == 0 )
		return;

	if ( IsConflictingClass(obj, 0) || IsPainOrBreakable(obj,1) )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"ERROR: This kind of object can't be made breakable.\n\""));
		return;
	}

	//obj->flags |= FL_SHIELDED;//only damagable by lightsabers
	obj->takedamage = qtrue;

	if ( trap_Argc() > 1 )
	{
		trap_Argv( 1, buffer, sizeof(buffer) );
		obj->health = atoi(buffer);
	}
	else
		obj->health = 10;

	obj->die = mplace_die;

	if ( obj->s.eType == ET_MOVER ) //&& obj->NPC_type == 0 ) // npc spawners use genericValue4
		obj->genericValue4 = 1;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof(buffer) );
		obj->material = atoi(buffer);
	}
	else
	{
		obj->material = MAT_LT_STONE;
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof(buffer) );
		obj->radius = atoi(buffer);
	}
	else
	{
		obj->radius = 0;
	}
}

void Cmd_mBreakableAll_f( gentity_t *ent )
{
	char buffer[MAX_TOKEN_CHARS];
	gentity_t *obj;
	int n, health, material, radius;
	qboolean complained;

	if ( !g_cheats.integer && !(ent->client->sess.permissions & PERMISSION_PLACE_OBJECTS) ) //
	{
		trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		return;
	}

	if ( trap_Argc() > 4 || trap_Argc() < 2 )
	{
		trap_SendServerCommand( ent-g_entities, va("print \"Command usage: mbreakableall <object-health> <optional-material-number> <optional-chunk-factor>\n\""));
		return;
	}


	if ( trap_Argc() > 1 )
	{
		trap_Argv( 1, buffer, sizeof(buffer) );
		health = atoi(buffer);
	}
	else
		health = 10;

	if ( trap_Argc() > 2 )
	{
		trap_Argv( 2, buffer, sizeof(buffer) );
		material = atoi(buffer);
	}
	else
	{
		material = MAT_LT_STONE;
	}

	if ( trap_Argc() > 3 )
	{
		trap_Argv( 3, buffer, sizeof(buffer) );
		radius = atoi(buffer);
	}
	else
	{
		radius = 0;
	}

	complained = qfalse;

	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
		continue;

		if ( obj->creator == ent-g_entities + 1 && (obj->s.eType == ET_MOVER || obj->s.eType == ET_FX ) )
		{
			
			if ( IsConflictingClass(obj, 0) || IsPainOrBreakable(obj,0) )
			{
				if ( complained )
					continue;

				
				//SendServerCommand( ent-g_entities, va("print \"WARNING: Not all of your objects can be made breakable.\n\""));
				complained = qtrue;
				continue;
			}

			//obj->flags |= FL_SHIELDED;//only damagable by lightsabers
			obj->takedamage = qtrue;
			obj->die = mplace_die;
			obj->health = health;
			obj->radius = radius;
			obj->material = material;

			if ( obj->s.eType == ET_MOVER )
				obj->genericValue4 = 1;
		}
	}
}

/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {

	gentity_t *obj;
	int n, creator;

	char buffer[MAX_TOKEN_CHARS];

	if ( trap_Argc() > 1 )
	{
		trap_Argv( 1, buffer, sizeof(buffer) );
		creator = atoi(buffer) + 1;
	}
	else
	{
		creator = SERVER_CREATOR;
	}


	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		obj = &g_entities[n];

		if (!obj->inuse )
		continue;

		if ( obj->creator == creator && (obj->s.eType != ET_NPC) )
		{
			trap_SendServerCommand( ent-g_entities, va("print \"%i %s %f %f %f, %f %f %f \n\"", n, obj->model2 == 0 ? "zero" : obj->model2, obj->r.currentOrigin[0], obj->r.currentOrigin[1], obj->r.currentOrigin[2],
				obj->r.currentAngles[0], obj->r.currentAngles[1], obj->r.currentAngles[2]));
		}
	}
/*
	int max, n, i;

	max = trap_AAS_PointReachabilityAreaIndex( NULL );

	n = 0;
	for ( i = 0; i < max; i++ ) {
		if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
			n++;
	}

	//trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
	trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
*/
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	if (ps->m_iVehicleNum)
	{
		return 0;
	}
	
	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	if (!BG_IsItemSelectable(ps, forcedUse))
	{
		return 0;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
	case HI_MEDPAC_BIG:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap_Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap_Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT );
		if (tr.fraction > 0.9 && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap_Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	case HI_JETPACK: //do something?
		return 1;
	case HI_HEALTHDISP:
		return 1;
	case HI_AMMODISP:
		return 1;
	case HI_EWEB:
		return 1;
	case HI_CLOAK:
		return 1;
	default:
		return 1;
	}
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		if (ent->client->ps.saberHolstered == 2)
		{
			ent->client->ps.saberHolstered = 0;

			if (ent->client->saber[0].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			}
			if (ent->client->saber[1].soundOn)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
			}
		}
		else
		{
			ent->client->ps.saberHolstered = 2;
			if (ent->client->saber[0].soundOff)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
			}
			if (ent->client->saber[1].soundOff &&
				ent->client->saber[1].model[0])
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
			}
			//prevent anything from being done for 400ms after holster
			ent->client->ps.weaponTime = 400;
		}
	}
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	qboolean usingSiegeStyle = qfalse;
	
	if ( !ent || !ent->client )
	{
		return;
	}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{ //no cycling for akimbo
		if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
		{//can turn second saber off 
			if ( ent->client->ps.saberHolstered == 1 )
			{//have one holstered
				//unholster it
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
				ent->client->ps.saberHolstered = 0;
				//g_active should take care of this, but...
				ent->client->ps.fd.saberAnimLevel = SS_DUAL;
			}
			else if ( ent->client->ps.saberHolstered == 0 )
			{//have none holstered
				if ( (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
				{//can't turn it off manually
				}
				else if ( ent->client->saber[1].bladeStyle2Start > 0
					&& (ent->client->saber[1].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
				{//can't turn it off manually
				}
				else
				{
					//turn it off
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
					ent->client->ps.saberHolstered = 1;
					//g_active should take care of this, but...
					ent->client->ps.fd.saberAnimLevel = SS_FAST;
				}
			}

			if (d_saberStanceDebug.integer)
			{
				trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle dual saber blade.\n\"") );
			}
			return;
		}
	}
	else if (ent->client->saber[0].numBlades > 1
		&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
	{ //use staff stance then.
		if ( ent->client->ps.saberHolstered == 1 )
		{//second blade off
			if ( ent->client->ps.saberInFlight )
			{//can't turn second blade back on if it's in the air, you naughty boy!
				if (d_saberStanceDebug.integer)
				{
					trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade in air.\n\"") );
				}
				return;
			}
			//turn it on
			G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
			ent->client->ps.saberHolstered = 0;
			//g_active should take care of this, but...
			if ( ent->client->saber[0].stylesForbidden )
			{//have a style we have to use
				WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
				if ( ent->client->ps.weaponTime <= 0 )
				{ //not busy, set it now
					ent->client->ps.fd.saberAnimLevel = selectLevel;
				}
				else
				{ //can't set it now or we might cause unexpected chaining, so queue it
					ent->client->saberCycleQueue = selectLevel;
				}
			}
		}
		else if ( ent->client->ps.saberHolstered == 0 )
		{//both blades on
			if ( (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE) )
			{//can't turn it off manually
			}
			else if ( ent->client->saber[0].bladeStyle2Start > 0
				&& (ent->client->saber[0].saberFlags2&SFL2_NO_MANUAL_DEACTIVATE2) )
			{//can't turn it off manually
			}
			else
			{
				//turn second one off
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				ent->client->ps.saberHolstered = 1;
				//g_active should take care of this, but...
				if ( ent->client->saber[0].singleBladeStyle != SS_NONE )
				{
					if ( ent->client->ps.weaponTime <= 0 )
					{ //not busy, set it now
						ent->client->ps.fd.saberAnimLevel = ent->client->saber[0].singleBladeStyle;
					}
					else
					{ //can't set it now or we might cause unexpected chaining, so queue it
						ent->client->saberCycleQueue = ent->client->saber[0].singleBladeStyle;
					}
				}
			}
		}
		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to toggle staff blade.\n\"") );
		}
		return;
	}

	if (ent->client->saberCycleQueue)
	{ //resume off of the queue if we haven't gotten a chance to update it yet
		selectLevel = ent->client->saberCycleQueue;
	}
	else
	{
		selectLevel = ent->client->ps.fd.saberAnimLevel;
	}

	if (g_gametype.integer == GT_SIEGE &&
		ent->client->siegeClass != -1 &&
		bgSiegeClasses[ent->client->siegeClass].saberStance)
	{ //we have a flag of useable stances so cycle through it instead
		int i = selectLevel+1;

		usingSiegeStyle = qtrue;

		while (i != selectLevel)
		{ //cycle around upward til we hit the next style or end up back on this one
			if (i >= SS_NUM_SABER_STYLES)
			{ //loop back around to the first valid
				i = SS_FAST;
			}

			if (bgSiegeClasses[ent->client->siegeClass].saberStance & (1 << i))
			{ //we can use this one, select it and break out.
				selectLevel = i;
				break;
			}
			i++;
		}

		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle given class stance.\n\"") );
		}
	}
	else
	{
		selectLevel++;
		if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
		{
			selectLevel = FORCE_LEVEL_1;
		}
		if (d_saberStanceDebug.integer)
		{
			trap_SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle stance normally.\n\"") );
		}
	}
/*
#ifndef FINAL_BUILD
	switch ( selectLevel )
	{
	case FORCE_LEVEL_1:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
		break;
	case FORCE_LEVEL_2:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
		break;
	case FORCE_LEVEL_3:
		trap_SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
		break;
	}
#endif
*/
	if ( !usingSiegeStyle )
	{
		//make sure it's valid, change it if not
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
	}

	if (ent->client->ps.weaponTime <= 0)
	{ //not busy, set it now
		ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
	}
	else
	{ //can't set it now or we might cause unexpected chaining, so queue it
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (g_gametype.integer == GT_DUEL || g_gametype.integer == GT_POWERDUEL)
	{ //rather pointless in this mode..
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	//if (g_gametype.integer >= GT_TEAM && g_gametype.integer != GT_SIEGE)
	if (g_gametype.integer >= GT_TEAM)
	{ //no private dueling in team modes
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: No longer doing this..

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

/*	if (ent->client->ps.duelInProgress)
	{
		return;
	}*/

	//New: Don't let a player duel if he just did and hasn't waited 10 seconds yet (note: If someone challenges him, his duel timer will reset so he can accept)
/*	if (ent->client->ps.fd.privateDuelTime > level.time)
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_JUSTDID")) );
		return;
	}*/

/*	if (G_OtherPlayersDueling())
	{
		trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "CANTDUEL_BUSY")) );
		return;
	}*/

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}

		if (g_gametype.integer >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap_SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname) );

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (ent->flags & FL_GODMODE)//Turn off god in duel.
			{
				ent->flags ^= FL_GODMODE;

				trap_SendServerCommand( ent-g_entities, va("print \"godmode OFF\n\""));
				trap_SendServerCommand( challenged-g_entities, va("print \"godmode OFF\n\""));
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap_SendServerCommand( challenged-g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")) );
			trap_SendServerCommand( ent-g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname) );
			challenged->client->pers.lastMessageTime = level.time;
			ent->client->pers.lastMessageTime = level.time;
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

#ifndef FINAL_BUILD

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[saberMoveData[self->client->ps.saberMove].animToUse].name);
}

void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
{
	int argNum = trap_Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;

	if (argNum < 2)
	{
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	G_SetAnim(self, NULL, SETANIM_BOTH, i, flags, 0);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags, 0);
}

void Bot_SetForcedMovement(int bot, int forward, int right, int up);

#ifndef FINAL_BUILD
extern void DismembermentByNum(gentity_t *self, int num);
extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

qboolean TryGrapple(gentity_t *ent)
{
	if (ent->client->ps.weaponTime > 0)
	{ //weapon busy
		return qfalse;
	}
	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //force power or knockdown or something
		return qfalse;
	}
	if (ent->client->grappleState)
	{ //already grappling? but weapontime should be > 0 then..
		return qfalse;
	}

	if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
	{
		return qfalse;
	}

	if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
	{
		Cmd_ToggleSaber_f(ent);
		if (!ent->client->ps.saberHolstered)
		{ //must have saber holstered
			return qfalse;
		}
	}

	//G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB )
	{ //providing the anim set succeeded..
		ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
		if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
		{
			ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
		}
		ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
	return qtrue;
	}

	//return qfalse;

	//if ( ent->client->ps.groundEntityNum != ENTITYNUM_NONE ) 
	//{
	//	ent->client->ps.forceHandExtend = HANDEXTEND_TAUNT;
	//	ent->client->ps.forceDodgeAnim = BOTH_MEDITATE;//BOTH_SIT3;
	//	ent->client->ps.forceHandExtendTime = level.time + BG_AnimLength(ent->localAnimIndex, (animNumber_t)BOTH_MEDITATE);
	//}

	///*G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, /*BOTH_KYLE_GRAB*/BOTH_SLEEP6START, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD, 0);
	//if (ent->client->ps.torsoAnim == BOTH_SLEEP6START /*BOTH_KYLE_GRAB*/)
	//{ //providing the anim set succeeded..
	//	ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
	//	if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
	//	{
	//		ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
	//	}
	//	ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
	//	return qtrue;
	//}

	//return qfalse; 
	return qfalse;
//}
}

#ifndef FINAL_BUILD
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
#endif

#ifdef RANKINGMOD
/*
=================
PlayerPerformResponses

Send responses for the players from the player-scoring DLL
=================
*/
void PerformPlayerResponses()
{
	char message[MAX_STRING_CHARS];
	enum RESPONSE_TYPE type;
	int client;

	while( PlayerGetNextResponse( &type, &client, message, MAX_STRING_CHARS ) )
	{
		if ( type == CLIENT_PRINT )
		{
			trap_SendServerCommand( client, va("print \"%s\n\"", message ) );
		}
		else if ( type == PRINT_ALL_CLIENTS )
		{
			trap_SendServerCommand( -1, va("print \"%s\n\"", message ) );
		}
		else if ( type == BIG_PRINT )
		{
			trap_SendServerCommand( client, va("cp \"%s\n\"", message ) );
		}
		else if ( type == BIG_PRINT_ALL )
		{
			trap_SendServerCommand( -1, va("cp \"%s\n\"", message ) );
		}
	}
}
#endif




/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap_Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0) {
		if (g_gametype.integer < GT_TEAM)
		{ //not a team game, just refer to regular say.
			Cmd_Say_f (ent, SAY_ALL, qfalse);
		}
		else
		{
			Cmd_Say_f (ent, SAY_TEAM, qfalse);
		}
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}

	if (Q_stricmp(cmd, "voice_cmd") == 0)
	{
		Cmd_VoiceCommand_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

#ifdef RANKINGMOD
	// Toast's Commands
	if (Q_stricmp(cmd, "login") == 0) 
	{
		if (trap_Argc() == 3)
		{
			char userName[MAX_STRING_CHARS];
			char password[MAX_STRING_CHARS];

			trap_Argv( 1, userName, sizeof(userName) );
			trap_Argv( 2, password, sizeof(password) );

			if ( PlayerLogin( userName, password, clientNum ) )
			{
				strcpy( level.clients[clientNum].sess.playerUsername, userName );
			}
			PerformPlayerResponses();
			return;
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: login <username> <password>\n\"" ) );
			return;
		}
	}

	if (Q_stricmp(cmd, "logout") == 0)
	{
		PlayerLogout( clientNum );
		PerformPlayerResponses();
		return;
	}

	if (Q_stricmp(cmd, "newuser") == 0)
	{
		if (trap_Argc() == 4)
		{
			char userName[MAX_STRING_CHARS];
			char password1[MAX_STRING_CHARS];
			char password2[MAX_STRING_CHARS];
			int len;

			trap_Argv( 1, userName, sizeof(userName) );
			trap_Argv( 2, password1, sizeof(password1) );
			trap_Argv( 3, password2, sizeof(password2) );
			
			for( len = 0 ; len < MAX_STRING_CHARS && userName[len] != 0 ; len++ )
			{
			}

			len--;

			if ( len > MAX_USERNAME_LENGTH )
			{
				trap_SendServerCommand( clientNum, va("print \"Usernames must be 15 characters or less in length.\n\"" ) );
				return;
			}
		
			PlayerAddUser(userName, password1, password2, clientNum);

			PerformPlayerResponses();

			return;
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: newuser <username> <password> <password-again-for-verification>\nPasswords are case sensitive.\n\"" ) );
			return;
		}
	}

	if (Q_stricmp(cmd, "newpass") == 0)
	{
		if (trap_Argc() == 4)
		{
			char oldPass[MAX_STRING_CHARS];
			char newPass1[MAX_STRING_CHARS];
			char newPass2[MAX_STRING_CHARS];

			trap_Argv( 1, oldPass, sizeof(oldPass) );
			trap_Argv( 2, newPass1, sizeof(newPass1) );
			trap_Argv( 3, newPass2, sizeof(newPass2) );

			PlayerNewPassword( oldPass, newPass1, newPass2, clientNum ); 
			PerformPlayerResponses();

			return;
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: newpass <old-password> <new-password> <new-password>\nPasswords are case sensitive.\n\"" ) );
			return;
		}
	}

	if (Q_stricmp(cmd, "stats") == 0)
	{
		PlayerStats( clientNum, 0 ); // 0 here means don't say the stats
		PerformPlayerResponses();
		return;
	}

	if (Q_stricmp(cmd, "ranks") == 0)
	{
		PlayerRanks( clientNum );
		PerformPlayerResponses();
		return;
	}

	if (Q_stricmp(cmd, "top10") == 0)
	{
		PlayerTop10( clientNum );
		PerformPlayerResponses();
		return;
	}

	if (Q_stricmp(cmd, "resetpass") == 0)
	{
		if (trap_Argc() == 3)
		{
			char username1[MAX_STRING_CHARS];
			char username2[MAX_STRING_CHARS];

			trap_Argv( 1, username1, sizeof(username1) );
			trap_Argv( 2, username2, sizeof(username2) );

			PlayerResetPassword( username1, username2, clientNum ); 
			PerformPlayerResponses();

			return;
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: resetpass <username> <username-again-for-verification>\n\"" ) );
			return;
		}
	}

#endif

	if (Q_stricmp(cmd, "mpsay") == 0)
	{
		char* message;
		char client[MAX_STRING_CHARS];
		int target;

		if ( !(ent->client->sess.permissions & PERMISSION_ADMIN_SAY) )
		{
			trap_SendServerCommand( clientNum, va("print \"You don't have permission to use this command. \n\"" ) );
			return;
		}


		if (trap_Argc() < 3 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mpsay <client> <message>\n\"" ) );
			return;
		}

		trap_Argv( 1, client, sizeof(client) );
		
		if ( Q_stricmp(client, "all") == 0 )
		{
			target = -1;
		}
		else
		{
			target = G_ClientNumFromNetname(clientNum, client);
		
			if ( target == -1 )
				return;
		}

		message = ConcatArgs( 2 );
		
		trap_SendServerCommand( target, va("cp \"%s\n\"", message ) );

		if ( target == -1 )
			G_LogScreenPrintAll();
		else
			level.clients[target].pers.lastMessageTime = level.time;

		if ( target >= 0 )
			trap_SendServerCommand( clientNum, va("print \"%s -> %s\n\"", message, g_entities[target].client->pers.netname) );
		else
			trap_SendServerCommand( clientNum, va("print \"%s -> all\n\"", message) );

		return;
	}

	if (Q_stricmp(cmd, "mannounce") == 0)
	{
		char* message;

		if ( !(ent->client->sess.permissions & PERMISSION_ADMIN_SAY) )
		{
			trap_SendServerCommand( clientNum, va("print \"You don't have permission to use this command. \n\"" ) );
			return;
		}

		if (trap_Argc() < 2 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mannounce <message>\n\"" ) );
			return;
		}

		message = ConcatArgs( 1 );
		Q_strncpyz( level.announcement, message, sizeof(level.announcement) );
	
		level.announceStop = level.time + 20000; //Show message for 20 secs


		trap_SendServerCommand( clientNum, "print \"Announcing...\n\"" );
		return;
	}

	if (Q_stricmp(cmd, "mtestdecrypt") == 0)
	{
		char username[MAX_STRING_CHARS];

		if ( !(ent->client->sess.permissions & PERMISSION_TESTING ) )
		{
			trap_SendServerCommand( clientNum, va("print \"You don't have permission to use this command. \n\"" ) );
			return;
		}

		if (trap_Argc() < 1 || trap_Argc() > 2 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mtestdecrypt <username>\n\"" ) );
			return;
		}

		trap_Argv( 1, username, sizeof(username) );

		DecryptTest(ent,username);
		return;
	}
	if (Q_stricmp(cmd, "mtest") == 0)
	{
		int num, i;
//		gentity_t *NPC;
		char buffer[MAX_STRING_CHARS];
		//char buffer2[MAX_STRING_CHARS];

		if (trap_Argc() < 1 || trap_Argc() > 2 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mtest Number\n\"" ) );
			return;
		}

		trap_Argv( 1, buffer, sizeof(buffer) );
		num = atoi(buffer);
/*
		if (trap_Argc() == 3)
		{
			trap_Argv( 2, buffer2, sizeof(buffer));
			//trap_SetConfigstring( num, buffer2);
			return;
		}*/
	
		//trap_SetConfigstring( num, "");
		//return;*/
		//trap_TrueFree((void **)&AllocPointerObj);
		//trap_SendServerCommand(-1, va("print \"%s\n\"", AllocPoints[num].AllocPointer));
//		trap_TrueFree((void **)&AllocPointer);
		//trap_GetConfigstring(num,buffer,sizeof(buffer));

		/*NPC = &g_entities[num];
		if (!NPC->NPC){
			trap_SendServerCommand(ent-g_entities, va("print \"%i\n\"",num ));
			return;
		}*/

	//	for (i = 0; i < 15; i++)
	//		trap_SendServerCommand( ent-g_entities, va("print \"Choice%i: %s \n\"",i+1, npcTypesArray[i].NPCTypes ));
		return;

		/*if (NPC->ghoul2 && trap_G2_HaveWeGhoul2Models(NPC->ghoul2))
		{
			trap_SendServerCommand(ent-g_entities, va("print \"Has ghoul2.\n\"" ));
			//trap_G2API_CleanGhoul2Models(&(NPC->ghoul2));
			trap_SendServerCommand(-1, va("kg2 %i", num));
		}

		trap_SendServerCommand(ent-g_entities, va("print \"%i\n\"",num ));
		return;*/
	}
	if (Q_stricmp(cmd, "mlogin") == 0)
	{
		char password[MAX_STRING_CHARS];
		char username[MAX_STRING_CHARS];

		if (trap_Argc() < 2 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mlogin <username> <password>\n\"" ) );
			return;
		}

		trap_Argv( 1, username, sizeof(username) );
		trap_Argv( 2, password, sizeof(password) );
	
		if ( ReadUserFile(ent, username, password) )
		{
			trap_SendServerCommand( clientNum, va("print \"Welcome %s\n\"", username ) );
		}
			
	
		/*char password[MAX_STRING_CHARS];

		if (trap_Argc() < 1 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mlogin <password>\n\"" ) );
			return;
		}

		trap_Argv( 1, password, sizeof(password) );
		
		if ( strcmp(password, "admiN132") == 0 )
		{
			trap_SendServerCommand( clientNum, va("print \"Logged in successfully\n\"" ) );
			ent->client->sess.permissions  = PERMISSION_TESTING * 2 - 1; // all below and including PERMISSION_TESTING
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Invalid Password\n\"" ) );
			G_Printf( "mlog: Failed login attempt from client %i, %s\n", clientNum, ent->client->pers.netname  );
		}*/

		return;
	}

	if (Q_stricmp(cmd, "mlogout") == 0)
	{
		char username[MAX_STRING_CHARS];

		if (trap_Argc() != 1 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mlogout\n\"" ) );
			return;
		}

		if ( ent->client->sess.username[0] == 0 )
		{
			trap_SendServerCommand( clientNum, va("print \"Not logged in\n\"" ) );
			return;
		}

		Q_strncpyz( username, ent->client->sess.username, sizeof(ent->client->sess.username) );
		
		ent->client->sess.username[0] = 0;
		ent->client->sess.password[0] = 0;
		ent->client->sess.permissions = StringToPermissionsNum( g_defaultNonUserPermissions.string );

		trap_SendServerCommand( clientNum, va("print \"%s logged out\n\"", username ) );
			
	
		/*char password[MAX_STRING_CHARS];

		if (trap_Argc() < 1 )
		{
			trap_SendServerCommand( clientNum, va("print \"Command usage: mlogin <password>\n\"" ) );
			return;
		}

		trap_Argv( 1, password, sizeof(password) );
		
		if ( strcmp(password, "admiN132") == 0 )
		{
			trap_SendServerCommand( clientNum, va("print \"Logged in successfully\n\"" ) );
			ent->client->sess.permissions  = PERMISSION_TESTING * 2 - 1; // all below and including PERMISSION_TESTING
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"Invalid Password\n\"" ) );
			G_Printf( "mlog: Failed login attempt from client %i, %s\n", clientNum, ent->client->pers.netname  );
		}*/

		return;
	}



	// ignore all other commands when at intermission
	if (level.intermissiontime)
	{
		qboolean giveError = qfalse;
		//rwwFIXMEFIXME: This is terrible, write it differently

		if (!Q_stricmp(cmd, "give"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "giveother"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "god"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "notarget"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "noclip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "kill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamtask"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "levelshot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follownext"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "followprev"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "team"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "duelteam"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "siegeclass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "forcechanged"))
		{ //special case: still update force change
			Cmd_ForceChanged_f (ent);
			return;
		}
		else if (!Q_stricmp(cmd, "where"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "vote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callteamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "gc"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "setviewpos"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "stats"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtele"))
		{
			giveError = qtrue;
		}
		else if ( !Q_stricmp(cmd, "mtelet") )
		{
			giveError = qtrue;
		}
		else if ( !Q_stricmp(cmd, "mtelelast") )
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "morigin"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msayorigin"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mempower"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mscaleme"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mscale"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mscalet"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mplace"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpclass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mplacefx"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmark"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "munmark"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mgrabbing"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mdraw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmarkfoot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmove"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mbobbing"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mset"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mbutton"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mplatform"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mrotating"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mclasst"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpendulum"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtelep"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpspecial"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlight"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkillsw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msoundsw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mjumpsw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtelesw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mprintsw"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "musable"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtouchable"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mspawner"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "connectto"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mwrite"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mjumpp"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mdest"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmoveall"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkillt"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mremove"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkillall"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmanipulate"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mselect"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmapshader"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mgrabt"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mgrab"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "marm"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mdrop"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlist"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlistsnd"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "minfo"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "help"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mrotate"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "msaveobs"))
		{
			giveError = qtrue;
		}		
		else if (!Q_stricmp(cmd, "msavemapobs"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mloadobs"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "mmd3info"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlistobs"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtrace"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "meffect"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "mlistfx"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "mlistother"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "mlistso"))
		{
			giveError = qtrue;
		}	
		else if (!Q_stricmp(cmd, "mpain"))
		{
			giveError = qtrue;
		}
		else if (Q_stricmp (cmd, "mbreakableall") == 0)
		{
			giveError = qtrue;
		}
		else if (Q_stricmp (cmd, "mbreakable") == 0)
		{
			giveError = qtrue;
		}
		else if (Q_stricmp (cmd, "mpushable") == 0)
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mslay"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mstatus"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mslap"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkick"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mban"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "munban"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlistbans"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mannounce"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mattack"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mdefend"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mfollow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mtelesp"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mgrapple"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "manim"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mlistanims"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpermissions"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpermission"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mforbid"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mnewuser"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mchangepass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mresetpass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmarksides"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmarkallsides"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmarktopbottom"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mclearedges"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msaveedges"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mnod"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mshake"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mcomeon"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mthrow3"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mthrow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mthrow2"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mspin"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msit"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msit2"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msit3"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msit4"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msleep"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mkneel"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mplane"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mflip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mshowoff"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mnoisy"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msplits"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mhips"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msurrender"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "matease"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mcower"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mcheer"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msignal1"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msignal2"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msignal3"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msignal4"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msaberlen"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mmarksize"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mallowgive"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mpassword"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msetpassword"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "msetpasswordt"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "mclip"))
		{
			giveError = qtrue;
		}
		
		if (giveError)
		{
			trap_SendServerCommand( clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
		}
		else
		{
			Cmd_Say_f (ent, qfalse, qtrue);
		}
		return;
	}

	if ( (Q_stricmp(cmd, "minfo") == 0) || (Q_stricmp(cmd, "help") == 0) )
	{
		Cmd_minfo_f( ent );
	}
	else if (Q_stricmp (cmd, "mattack") == 0)
	{
		Cmd_mAttack_f( ent );
	}
	else if (Q_stricmp (cmd, "mdefend") == 0)
	{
		Cmd_mDefend_f( ent );
	}
	else if (Q_stricmp (cmd, "mfollow") == 0)
	{
		Cmd_mFollow_f( ent );
	}
	else if (Q_stricmp (cmd, "mtele") == 0)
	{
		Cmd_mtele_f( ent );
	}
	else if (Q_stricmp (cmd, "mtelet") == 0)
	{
		Cmd_mtelet_f( ent );
	}
	else if (Q_stricmp (cmd, "mtelelast") == 0)
	{
		Cmd_mtelelast_f( ent );
	}
	else if (Q_stricmp (cmd, "mchangepass") == 0)
	{
		Cmd_mChangePass_f( ent );
	}
	else if (Q_stricmp (cmd, "mresetpass") == 0)
	{
		Cmd_mResetPass_f( ent );
	}
	else if (Q_stricmp (cmd, "mnewuser") == 0)
	{
		Cmd_mNewUser_f( ent );
	}
	else if (Q_stricmp (cmd, "manim") == 0)
	{
		Cmd_manim_f( ent );
	}
	else if (Q_stricmp (cmd, "mlistanims") == 0)
	{
		Cmd_mlistanims_f( ent );
	}
	else if (Q_stricmp (cmd, "mpermissions") == 0)
	{
		Cmd_mpermissions_f( ent );
	}
	else if (Q_stricmp (cmd, "mpermission") == 0)
	{
		Cmd_mPermission_f( ent );
	}
	else if (Q_stricmp (cmd, "mforbid") == 0)
	{
		Cmd_mForbid_f( ent );
	}
	else if (Q_stricmp (cmd, "mtelesp") == 0)
	{
		Cmd_mtelesp_f( ent );
	}
	else if (Q_stricmp (cmd, "meffect") == 0)
	{
		Cmd_mEffect_f( ent );
	}
	else if (Q_stricmp (cmd, "mslay") == 0)
	{
		Cmd_mslay_f( ent );
	}
	else if (Q_stricmp (cmd, "mstatus") == 0)
	{
		Cmd_mstatus_f( ent );
	}
	else if (Q_stricmp (cmd, "mslap") == 0)
	{
		Cmd_mslap_f( ent );
	}
	else if (Q_stricmp (cmd, "mkick") == 0)
	{
		Cmd_mkick_f( ent );
	}
	else if (Q_stricmp (cmd, "mban") == 0)
	{
		Cmd_mban_f( ent );
	}
	else if (Q_stricmp (cmd, "munban") == 0)
	{
		Cmd_munban_f( ent );
	}
	else if (Q_stricmp (cmd, "mlistbans") == 0)
	{
		Cmd_mlistbans_f( ent );
	}
	else if (Q_stricmp (cmd, "mmarktopbottom") == 0)
	{
		Cmd_mMarktopbottom_f( ent );
	}
	else if (Q_stricmp (cmd, "mmarksides") == 0)
	{
		Cmd_mMarkSides_f( ent );
	}
	else if (Q_stricmp (cmd, "mmarkallSides") == 0)
	{
		Cmd_mMarkAllSides_f( ent );
	}
	else if (Q_stricmp (cmd, "msaberlen") == 0)
	{
		Cmd_mSaberLength_f( ent );
	}
	else if (Q_stricmp (cmd, "mclearedges") == 0)
	{
		Cmd_mClearEdges_f( ent );
	}
	else if (Q_stricmp (cmd, "msaveobs") == 0)
	{
		Cmd_mSaveobs_f( ent );
	}
	else if (Q_stricmp (cmd, "msavemapobs") == 0)
	{
		Cmd_mSaveMapobs_f( ent );
	}
	else if (Q_stricmp (cmd, "mloadobs") == 0)
	{
		Cmd_mLoadObs_f( ent );
	}
	else if (Q_stricmp (cmd, "mmd3info") == 0)
	{
		Cmd_mMD3Info_f( ent );
	}
	else if (Q_stricmp (cmd, "mlistobs") == 0)
	{
		Cmd_mListobs_f( ent );
	}
	else if (Q_stricmp (cmd, "mtrace") == 0)
	{
		Cmd_mTrace_f( ent );
	}
	else if (Q_stricmp (cmd, "morigin") == 0)
	{
		Cmd_mOrigin_f( ent );
	}
	else if (Q_stricmp (cmd, "msayorigin") == 0)
	{
		Cmd_mSayOrigin_f( ent );
	}
	else if (Q_stricmp (cmd, "mempower") == 0)
	{
		Cmd_mempower_f( ent );
	}
// This command doesn't work, sadly
//	else if (Q_stricmp (cmd, "mlightme") == 0)
//	{
//		Cmd_mlightme_f( ent );
//	}
	else if (Q_stricmp (cmd, "mscaleme") == 0)
	{
		Cmd_mscaleme_f( ent );
	}
	else if (Q_stricmp (cmd, "mscale") == 0)
	{
		Cmd_mscale_f( ent );
	}
	else if (Q_stricmp (cmd, "mscalet") == 0)
	{
		Cmd_mScalet_f( ent );
	}
	else if (Q_stricmp (cmd, "mplace") == 0)
	{
		Cmd_mplace_f( ent );
	}
	else if (Q_stricmp (cmd, "mweather") == 0)
	{
		Cmd_mweather_f( ent );
	}
	else if (Q_stricmp (cmd, "mpclass") == 0)
	{
		Cmd_mPClass_f( ent );
	}
	else if (Q_stricmp (cmd, "mplacefx") == 0)
	{
		Cmd_mplacefx_f( ent );
	}
	else if (Q_stricmp (cmd, "msetpain") == 0)
	{
		trap_SendServerCommand( clientNum, va("print \"ERROR: This command has changed names to mpain\n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
	}
	else if (Q_stricmp (cmd, "mpain") == 0)
	{
		Cmd_mPain_f( ent );
	}
	else if (Q_stricmp (cmd, "mbreakable") == 0)
	{
		Cmd_mBreakable_f( ent );
	}
	else if (Q_stricmp (cmd, "mbreakableall") == 0)
	{
		Cmd_mBreakableAll_f( ent );
	}
	else if (Q_stricmp (cmd, "mpushable") == 0)
	{
		Cmd_mPushable_f( ent );
	}
	else if (Q_stricmp (cmd, "mmark") == 0)
	{
		Cmd_mMark_f( ent );
	}
	else if (Q_stricmp (cmd, "munmark") == 0)
	{
		Cmd_mUnmark_f( ent );
	}
	else if (Q_stricmp (cmd, "mgrabbing") == 0)
	{
		Cmd_mGrabbing_f( ent );
	}
	else if (Q_stricmp (cmd, "mdraw") == 0)
	{
		Cmd_mdraw_f( ent );
	}
	else if (Q_stricmp (cmd, "mmarkfoot") == 0)
	{
		Cmd_mMarkfoot_f( ent );
	}
	else if (!Q_stricmp(cmd, "mmove"))
	{
		Cmd_mMove_f( ent );
	}
	else if (!Q_stricmp(cmd, "mset"))
	{
		Cmd_mSet_f( ent );
	}
#ifdef MM_WIP
	else if (!Q_stricmp(cmd, "mbobbing"))
	{
		Cmd_mBobbing_f( ent );
	}
	else if (!Q_stricmp(cmd, "mdoor"))
	{
		Cmd_mDoor_f( ent );
	}
	else if (!Q_stricmp(cmd, "mbutton"))
	{
		Cmd_mButton_f( ent );
	}
	else if (!Q_stricmp(cmd, "mplatform"))
	{
		Cmd_mPlatform_f( ent );
	}
	else if (!Q_stricmp(cmd, "mpendulum"))
	{
		Cmd_mPendulum_f( ent );
	}
	else if (!Q_stricmp(cmd, "mtelep"))
	{
		Cmd_mtelep_f( ent );
	}
	else if (!Q_stricmp(cmd, "mpspecial"))
	{
		Cmd_mpspecial_f( ent );
	}
	else if (!Q_stricmp(cmd, "mlight"))
	{
		Cmd_mLight_f( ent );
	}
	else if (!Q_stricmp(cmd, "mkillsw"))
	{
		Cmd_mkillsw_f( ent );
	}
	else if (!Q_stricmp(cmd, "msoundsw"))
	{
		Cmd_msoundsw_f( ent );
	}
	else if (!Q_stricmp(cmd, "mjumpsw"))
	{
		Cmd_mjumpsw_f( ent );
	}
	else if (!Q_stricmp(cmd, "mtelesw"))
	{
		Cmd_mTelesw_f( ent );
	}
	else if (!Q_stricmp(cmd, "mprintsw"))
	{
		Cmd_mprintsw_f( ent );
	}
	else if (!Q_stricmp(cmd, "musable"))
	{
		Cmd_musable_f( ent );
	}
	else if (!Q_stricmp(cmd, "mtouchable"))
	{
		Cmd_mtouchable_f( ent );
	}
	else if (!Q_stricmp(cmd, "mspawner"))
	{
		Cmd_mspawner_f( ent );
	}
	else if (!Q_stricmp(cmd, "mconnectto"))
	{
		Cmd_mconnectto_f( ent );
	}
	else if (!Q_stricmp(cmd, "mwrite"))
	{
		Cmd_mWrite_f( ent );
	}
	else if (!Q_stricmp(cmd, "mjumpp"))
	{
		Cmd_mJumpp_f( ent );
	}
	else if (!Q_stricmp(cmd, "mdest"))
	{
		Cmd_mDest_f( ent );
	}
	else if (!Q_stricmp(cmd, "mrotating"))
	{
		Cmd_mRotating_f( ent );
	}
	else if (!Q_stricmp(cmd, "mclasst"))
	{
		Cmd_mClasst_f( ent );
	}
#endif
	else if (!Q_stricmp(cmd, "mmoveall"))
	{
		Cmd_mMoveAll_f( ent );
	}
	else if (!Q_stricmp(cmd, "msaveedges"))
	{
		Cmd_mSaveEdges_f( ent );
	}
	else if (!Q_stricmp(cmd, "mkillt"))
	{
		Cmd_mKillt_f( ent );
	}
	else if (!Q_stricmp(cmd, "mkillall"))
	{
		Cmd_mKillAll_f( ent );
	}
	else if (!Q_stricmp(cmd, "mkill"))
	{
		Cmd_mKill_f( ent );
	}
	else if (!Q_stricmp(cmd, "mremove"))
	{
		Cmd_mKill_f( ent );
	}
	else if (!Q_stricmp(cmd, "mmanipulate"))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This command has changed to /mselect\n\""));
	}
	else if (!Q_stricmp(cmd, "mselect"))
	{
		Cmd_mselect_f( ent );
	}
	else if (!Q_stricmp(cmd, "mmapshader"))
	{
		Cmd_mMapShader_f( ent );
	}
	else if (!Q_stricmp(cmd, "mgrabt"))
	{
		Cmd_mgrabt_f( ent );
	}
	else if (!Q_stricmp(cmd, "mgrab"))
	{
		Cmd_mgrab_f( ent );
	}
	else if (!Q_stricmp(cmd, "marm"))
	{
		Cmd_marm_f( ent );
	}
	else if (!Q_stricmp(cmd, "mdrop"))
	{
		Cmd_mDrop_f( ent );
	}
	else if (!Q_stricmp(cmd, "mrotate"))
	{
		Cmd_mRotate_f( ent );
	}
	else if (!Q_stricmp(cmd, "mlist"))
	{
		Cmd_mList_f( ent );
	}
	else if (!Q_stricmp(cmd, "mlistsnd"))
	{
		Cmd_mListsnd_f( ent );
	}
	else if (!Q_stricmp(cmd, "mlistfx"))
	{
		Cmd_mlistfx_f( ent );
	}
	else if (!Q_stricmp(cmd, "mlistother"))
	{
		trap_SendServerCommand( ent-g_entities, va("print \"This command has changed to /mlistso\n\""));
	}
	else if (!Q_stricmp(cmd, "mlistso"))
	{
		Cmd_mlistso_f( ent );
	}
	else if (Q_stricmp (cmd, "give") == 0)
	{
		Cmd_Give_f (ent, 0);
	}
	else if (Q_stricmp (cmd, "giveother") == 0)
	{ //for debugging pretty much
		Cmd_Give_f (ent, 1);
	}
	else if (Q_stricmp (cmd, "mnod") == 0)
	{
		DoAnim( ent, BOTH_HEADNOD, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mshake") == 0)
	{
		DoAnim( ent, BOTH_HEADSHAKE, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mcomeon") == 0)
	{
		DoAnim( ent, BOTH_COME_ON1, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mthrow3") == 0)
	{
		DoAnim( ent, BOTH_ALORA_SPIN_THROW, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mthrow") == 0)
	{
		DoAnim( ent, BOTH_SABERTHROW1START, 2300, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mthrow2") == 0)
	{
		DoAnim( ent, BOTH_SABERTHROW2START, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mspin") == 0)
	{
		DoAnim( ent, BOTH_ALORA_SPIN, -1, qtrue, qtrue );
	}
//	else if (Q_stricmp (cmd, "mscouser") == 0)
//	{
//		DoAnim( ent, BOTH_STEADYSELF1, -1, qtrue, qtrue );
//	}
	else if (Q_stricmp (cmd, "msit") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_SIT1 && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_MEDITATE_END, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_SIT1, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "msit2") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_SIT2 && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_SIT2TOSTAND5, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_SIT2, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "msit3") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_SIT3 && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_SIT3TOSTAND5, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_SIT3, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "msit4") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_SIT4 && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_SIT3TOSTAND5, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_SIT4, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "msleep") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_SLEEP1 && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_SLEEP1GETUP2, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_SLEEP1, 90000000, qfalse, qfalse );
	}
	else if (Q_stricmp (cmd, "mkneel") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_STAND_TO_KNEEL && ent->client->ps.forceHandExtend )
			DoAnim( ent, BOTH_MEDITATE_END, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_STAND_TO_KNEEL, 90000000, qfalse, qfalse );
	}
	else if (Q_stricmp (cmd, "mplane") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_CEILING_CLING )
			DoAnim( ent, BOTH_ARIAL_F1, -1, qtrue, qfalse );
		else	
			DoAnim( ent, BOTH_CEILING_CLING, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "mflip") == 0)
	{
		int num;
		num = rand() % 5;

		if ( num == 0 )
			DoAnim( ent, BOTH_ARIAL_F1, -1, qtrue, qtrue );
		else if ( num == 1 )
			DoAnim( ent, BOTH_ALORA_FLIP_1, -1, qtrue, qtrue );
		else if ( num == 2 )
			DoAnim( ent, BOTH_ALORA_FLIP_2, -1, qtrue, qtrue );
		else if ( num == 3 )
			DoAnim( ent, BOTH_ALORA_FLIP_3, -1, qtrue, qtrue );
		else if ( num == 4 )
			DoAnim( ent, BOTH_ALORA_FLIP_B, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mshowoff") == 0)
	{
		int num;
		num = rand() % 3;

		if ( num == 0 )
			DoAnim( ent, BOTH_ROSH_HEAL, -1, qtrue, qtrue );
		else if ( num == 1 )
			DoAnim( ent, BOTH_TAVION_SCEPTERGROUND, -1, qtrue, qtrue );
		else if ( num == 2 )
			DoAnim( ent, BOTH_TAVION_SWORDPOWER, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mnoisy") == 0)
	{
		DoAnim( ent, BOTH_SONICPAIN_HOLD, -1, qtrue, qfalse );
	}
//	else if (Q_stricmp (cmd, "mturn") == 0)
//	{
//		DoAnim( ent, BOTH_SPIN1, -1, qtrue, qtrue );
//	}
//	else if (Q_stricmp (cmd, "mlunge") == 0)
//	{
//		DoAnim( ent, BOTH_TUSKENLUNGE1, -1, qtrue, qtrue );
//	}
	else if (Q_stricmp (cmd, "msplits") == 0)
	{
		if ( ent->client->ps.forceDodgeAnim == BOTH_DISMEMBER_LLEG )
			DoAnim( ent, BOTH_MEDITATE_END, -1, qtrue, qfalse );
		else	
			DoAnim( ent, BOTH_DISMEMBER_LLEG, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "mhips") == 0)
	{
		if ( ent->client->ps.torsoAnim == BOTH_STAND5TOSTAND8 )
			DoAnim( ent, BOTH_STAND8TOSTAND5, -1, qtrue, qfalse );
		else
			DoAnim( ent, BOTH_STAND5TOSTAND8, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "msurrender") == 0)
	{
		if ( ent->client->ps.torsoAnim == TORSO_SURRENDER_START )
			DoAnim( ent, TORSO_SURRENDER_STOP, -1, qtrue, qfalse );
		else
			DoAnim( ent, TORSO_SURRENDER_START, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "matease") == 0)
	{
		DoAnim( ent, BOTH_STAND4, 90000000, qtrue, qfalse );
	}
	else if (Q_stricmp (cmd, "mcower") == 0)
	{
		DoAnim( ent, BOTH_COWER1, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "mcheer") == 0)
	{
		DoAnim( ent, BOTH_TUSKENTAUNT1, -1, qfalse, qtrue );
	}
	else if (Q_stricmp (cmd, "msignal1") == 0)
	{
		DoAnim( ent, TORSO_HANDSIGNAL1, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "msignal2") == 0)
	{
		DoAnim( ent, TORSO_HANDSIGNAL2, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "msignal3") == 0)
	{
		DoAnim( ent, TORSO_HANDSIGNAL3, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "msignal4") == 0)
	{
		DoAnim( ent, TORSO_HANDSIGNAL4, -1, qtrue, qtrue );
	}
	else if (Q_stricmp (cmd, "t_use") == 0 && CheatsOk(ent))
	{ //debug use map object
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			gentity_t *targ;

			trap_Argv( 1, sArg, sizeof( sArg ) );
			targ = G_Find( NULL, FOFS(targetname), sArg );

			while (targ)
			{
				if (targ->use)
				{
					targ->use(targ, ent, ent);
				}
				targ = G_Find( targ, FOFS(targetname), sArg );
			}
		}
	}

	else if (Q_stricmp (cmd, "mallowgive") == 0)
	{
		Cmd_mAllowGive_f( ent );
	}
	else if (!(Q_stricmp (cmd, "mpassword")))
	{
		Cmd_mPassword_f( ent );
	}
	else if (!(Q_stricmp (cmd, "msetpassword")))
	{
		Cmd_mSetPassword_f( ent );
	}
	else if (!(Q_stricmp (cmd, "msetpasswordt")))
	{
		Cmd_mSetPasswordT_f( ent );
	}
	else if (!(Q_stricmp (cmd, "mclip")))
	{
		Cmd_mClip_f( ent );
	}

	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if ( Q_stricmp( cmd, "NPC" ) == 0 &&( ent->client->sess.permissions & PERMISSION_NPC_SPAWN || ent->client->sess.permissions & PERMISSION_NPC_ADMIN || CheatsOk(ent)) )
	{
		Cmd_NPC_f( ent );
	}
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "teamtask") == 0)
		Cmd_TeamTask_f (ent);
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "duelteam") == 0)
		Cmd_DuelTeam_f (ent);
	else if (Q_stricmp (cmd, "siegeclass") == 0)
		Cmd_SiegeClass_f (ent);
	else if (Q_stricmp (cmd, "forcechanged") == 0)
		Cmd_ForceChanged_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
//	else if (Q_stricmp (cmd, "stats") == 0)
//		Cmd_Stats_f( ent );
	else if (Q_stricmp (cmd, "mgrapple") == 0)
	{

		if ( !(ent->client->sess.permissions & PERMISSION_TESTING) ) //(!g_cheats.integer) &&
		{
			trap_SendServerCommand( ent-g_entities, va("print \"You do not have permission to use this command\n\""));
		}
		else
		{
			TryGrapple(ent);
		}
	}

	//for convenient powerduel testing in release
	else if (Q_stricmp(cmd, "killother") == 0 && CheatsOk( ent ) && ( ent->client->sess.permissions & PERMISSION_SLAY ) )
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = G_ClientNumFromNetname(clientNum, sArg);

			if (entNum >= 0 && entNum < MAX_GENTITIES)
			{
				gentity_t *kEnt = &g_entities[entNum];

				if (kEnt->inuse && kEnt->client)
				{
					kEnt->flags &= ~FL_GODMODE;
					kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
					player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
				}
			}
		}
	}
#ifdef _DEBUG
	else if (Q_stricmp(cmd, "relax") == 0 && CheatsOk( ent ))
	{
		if (ent->client->ps.eFlags & EF_RAG)
		{
			ent->client->ps.eFlags &= ~EF_RAG;
		}
		else
		{
			ent->client->ps.eFlags |= EF_RAG;
		}
	}
	else if (Q_stricmp(cmd, "holdme") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap_Argv( 1, sArg, sizeof( sArg ) );

			entNum = atoi(sArg);

			if (entNum >= 0 &&
				entNum < MAX_GENTITIES)
			{
				gentity_t *grabber = &g_entities[entNum];

				if (grabber->inuse && grabber->client && grabber->ghoul2)
				{
					if (!grabber->s.number)
					{ //switch cl 0 and entitynum_none, so we can operate on the "if non-0" concept
						ent->client->ps.ragAttach = ENTITYNUM_NONE;
					}
					else
					{
						ent->client->ps.ragAttach = grabber->s.number;
					}
				}
			}
		}
		else
		{
			ent->client->ps.ragAttach = 0;
		}
	}
	else if (Q_stricmp(cmd, "limb_break") == 0 && CheatsOk( ent ))
	{
		if (trap_Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int breakLimb = 0;
	
			trap_Argv( 1, sArg, sizeof( sArg ) );
			if (!Q_stricmp(sArg, "right"))
			{
				breakLimb = BROKENLIMB_RARM;
			}
			else if (!Q_stricmp(sArg, "left"))
			{
				breakLimb = BROKENLIMB_LARM;
			}

			G_BreakArm(ent, breakLimb);
		}
	}
	else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			DismembermentTest(ent);
		}
	}
	else if (Q_stricmp(cmd, "debugstupidthing") == 0 && CheatsOk( ent ))
	{
		int i = 0;
		gentity_t *blah;
		while (i < MAX_GENTITIES)
		{
			blah = &g_entities[i];
			if (blah->inuse && blah->classname && blah->classname[0] && !Q_stricmp(blah->classname, "NPC_Vehicle"))
			{
				Com_Printf("Found it.\n");
			}
			i++;
		}
	}
	else if (Q_stricmp(cmd, "arbitraryprint") == 0 && CheatsOk( ent ))
	{
		trap_SendServerCommand( -1, va("cp \"Blah blah blah\n\""));
		G_LogScreenPrintAll();
	}
	else if (Q_stricmp(cmd, "handcut") == 0 && CheatsOk( ent ))
	{
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		if (trap_Argc() > 1)
		{
			trap_Argv( 1, sarg, sizeof( sarg ) );

			if (sarg[0])
			{
				bCl = atoi(sarg);

				if (bCl >= 0 && bCl < MAX_GENTITIES)
				{
					gentity_t *hEnt = &g_entities[bCl];

					if (hEnt->client)
					{
						if (hEnt->health > 0)
						{
							gGAvoidDismember = 1;
							hEnt->flags &= ~FL_GODMODE;
							hEnt->client->ps.stats[STAT_HEALTH] = hEnt->health = -999;
							player_die (hEnt, hEnt, hEnt, 100000, MOD_SUICIDE);
						}
						gGAvoidDismember = 2;
						G_CheckForDismemberment(hEnt, ent, hEnt->client->ps.origin, 999, hEnt->client->ps.legsAnim, qfalse);
						gGAvoidDismember = 0;
					}
				}
			}
		}
	}
	else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk( ent ))
	{
		trace_t tr;
		vec3_t fPos;

		AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

		fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
		fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
		fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask);

		if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other && other->inuse && other->client)
			{
				vec3_t entDir;
				vec3_t otherDir;
				vec3_t entAngles;
				vec3_t otherAngles;

				if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(ent);
				}

				if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
					(other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
				{
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );

					StandardSetBodyAnim(ent, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					ent->client->ps.saberMove = LS_NONE;
					ent->client->ps.saberBlocked = 0;
					ent->client->ps.saberBlocking = 0;

					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					StandardSetBodyAnim(other, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;
				}
			}
		}
	}
#endif
	else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
		}
	}
	//begin bot debug cmds
	else if (Q_stricmp(cmd, "debugBMove_Forward") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Back") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Right") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Left") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Up") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap_Argc() > 1);
		trap_Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, -1, arg);
	}
	//end bot debug cmds
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugSetSaberMove") == 0)
	{
		Cmd_DebugSetSaberMove_f(ent);
	}
	else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0)
	{
		Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	else if (Q_stricmp(cmd, "debugDismemberment") == 0)
	{
		/*Cmd_Kill_f (ent);
		if (ent->health < 1)
		{*/
			char	arg[MAX_STRING_CHARS];
			int		iArg = 0;

			if (trap_Argc() > 1)
			{
				trap_Argv( 1, arg, sizeof( arg ) );

				if (arg[0])
				{
					iArg = atoi(arg);
				}
			}

			DismembermentByNum(ent, iArg);
	/*	}*/
	}
	else if (Q_stricmp(cmd, "debugDropSaber") == 0)
	{
		if (ent->client->ps.weapon == WP_SABER &&
			ent->client->ps.saberEntityNum &&
			!ent->client->ps.saberInFlight)
		{
			saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
		}
	}
	else if (Q_stricmp(cmd, "debugKnockMeDown") == 0)
	{
		if (BG_KnockDownable(&ent->client->ps))
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
			ent->client->ps.forceDodgeAnim = 0;
			if (trap_Argc() > 1)
			{
				ent->client->ps.forceHandExtendTime = level.time + 1100;
				ent->client->ps.quickerGetup = qfalse;
			}
			else
			{
				ent->client->ps.forceHandExtendTime = level.time + 700;
				ent->client->ps.quickerGetup = qtrue;
			}
		}
	}
	else if (Q_stricmp(cmd, "debugSaberSwitch") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			Cmd_ToggleSaber_f(targ);
		}
	}
	else if (Q_stricmp(cmd, "debugIKGrab") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			targ->client->ps.heldByClient = ent->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKBeGrabbedBy") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			ent->client->ps.heldByClient = targ->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKRelease") == 0)
	{
		gentity_t *targ = NULL;

		if (trap_Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap_Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			targ->client->ps.heldByClient = 0;
		}
	}
	else if (Q_stricmp(cmd, "debugThrow") == 0)
	{
		trace_t tr;
		vec3_t tTo, fwd;

		if (ent->client->ps.weaponTime > 0 || ent->client->ps.forceHandExtend != HANDEXTEND_NONE ||
			ent->client->ps.groundEntityNum == ENTITYNUM_NONE || ent->health < 1)
		{
			return;
		}

		AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);
		tTo[0] = ent->client->ps.origin[0] + fwd[0]*32;
		tTo[1] = ent->client->ps.origin[1] + fwd[1]*32;
		tTo[2] = ent->client->ps.origin[2] + fwd[2]*32;

		trap_Trace(&tr, ent->client->ps.origin, 0, 0, tTo, ent->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other->inuse && other->client && other->client->ps.forceHandExtend == HANDEXTEND_NONE &&
				other->client->ps.groundEntityNum != ENTITYNUM_NONE && other->health > 0 &&
				(int)ent->client->ps.origin[2] == (int)other->client->ps.origin[2])
			{
				float pDif = 40.0f;
				vec3_t entAngles, entDir;
				vec3_t otherAngles, otherDir;
				vec3_t intendedOrigin;
				vec3_t boltOrg, pBoltOrg;
				vec3_t tAngles, vDif;
				vec3_t fwd, right;
				trace_t tr;
				trace_t tr2;

				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );

				ent->client->ps.forceHandExtend = HANDEXTEND_PRETHROW;
				ent->client->ps.forceHandExtendTime = level.time + 5000;

				ent->client->throwingIndex = other->s.number;
				ent->client->doingThrow = level.time + 5000;
				ent->client->beingThrown = 0;

				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );

				other->client->ps.forceHandExtend = HANDEXTEND_PRETHROWN;
				other->client->ps.forceHandExtendTime = level.time + 5000;

				other->client->throwingIndex = ent->s.number;
				other->client->beingThrown = level.time + 5000;
				other->client->doingThrow = 0;

				//Doing this now at a stage in the throw, isntead of initially.
				//other->client->ps.heldByClient = ent->s.number+1;

				G_EntitySound( other, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
				G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*jump1.wav") );
				G_Sound(other, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/objectHit.wav" ));

				//see if we can move to be next to the hand.. if it's not clear, break the throw.
				VectorClear(tAngles);
				tAngles[YAW] = ent->client->ps.viewangles[YAW];
				VectorCopy(ent->client->ps.origin, pBoltOrg);
				AngleVectors(tAngles, fwd, right, 0);
				boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
				boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
				boltOrg[2] = pBoltOrg[2];

				VectorSubtract(boltOrg, pBoltOrg, vDif);
				VectorNormalize(vDif);

				VectorClear(other->client->ps.velocity);
				intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
				intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
				intendedOrigin[2] = other->client->ps.origin[2];

				trap_Trace(&tr, intendedOrigin, other->r.mins, other->r.maxs, intendedOrigin, other->s.number, other->clipmask);
				trap_Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID);

				if (tr.fraction == 1.0 && !tr.startsolid && tr2.fraction == 1.0 && !tr2.startsolid)
				{
					VectorCopy(intendedOrigin, other->client->ps.origin);
				}
				else
				{ //if the guy can't be put here then it's time to break the throw off.
					vec3_t oppDir;
					int strength = 4;

					other->client->ps.heldByClient = 0;
					other->client->beingThrown = 0;
					ent->client->doingThrow = 0;

					ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
					G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

					other->client->ps.forceHandExtend = HANDEXTEND_NONE;
					VectorSubtract(other->client->ps.origin, ent->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					other->client->ps.velocity[0] = oppDir[0]*(strength*40);
					other->client->ps.velocity[1] = oppDir[1]*(strength*40);
					other->client->ps.velocity[2] = 150;

					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					ent->client->ps.velocity[0] = oppDir[0]*(strength*40);
					ent->client->ps.velocity[1] = oppDir[1]*(strength*40);
					ent->client->ps.velocity[2] = 150;
				}
			}
		}
	}
#endif
#ifdef VM_MEMALLOC_DEBUG
	else if (Q_stricmp(cmd, "debugTestAlloc") == 0)
	{ //rww - small routine to stress the malloc trap stuff and make sure nothing bad is happening.
		char *blah;
		int i = 1;
		int x;

		//stress it. Yes, this will take a while. If it doesn't explode miserably in the process.
		while (i < 32768)
		{
			x = 0;

			trap_TrueMalloc((void **)&blah, i);
			if (!blah)
			{ //pointer is returned null if allocation failed
				trap_SendServerCommand( -1, va("print \"Failed to alloc at %i!\n\"", i));
				break;
			}
			while (x < i)
			{ //fill the allocated memory up to the edge
				if (x+1 == i)
				{
					blah[x] = 0;
				}
				else
				{
					blah[x] = 'A';
				}
				x++;
			}
			trap_TrueFree((void **)&blah);
			if (blah)
			{ //should be nullified in the engine after being freed
				trap_SendServerCommand( -1, va("print \"Failed to free at %i!\n\"", i));
				break;
			}

			i++;
		}

		trap_SendServerCommand( -1, "print \"Finished allocation test\n\"");
	}
#endif
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugShipDamage") == 0)
	{
		char	arg[MAX_STRING_CHARS];
		char	arg2[MAX_STRING_CHARS];
		int		shipSurf, damageLevel;

		trap_Argv( 1, arg, sizeof( arg ) );
		trap_Argv( 2, arg2, sizeof( arg2 ) );
		shipSurf = SHIPSURF_FRONT+atoi(arg);
		damageLevel = atoi(arg2);

		G_SetVehDamageFlags( &g_entities[ent->s.m_iVehicleNum], shipSurf, damageLevel );
	}
#endif
	else
	{
		if (Q_stricmp(cmd, "addbot") == 0)
		{ //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
//			trap_SendServerCommand( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
			trap_SendServerCommand( clientNum, va("print \"%s.\n\"", G_GetStringEdString("MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER")));
		}
		else
		{
			trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
		}
	}
}
