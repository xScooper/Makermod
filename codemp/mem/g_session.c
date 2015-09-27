// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( gclient_t *client ) {
	const char	*s;
	const char	*var;
	int			i = 0;
	char		siegeClass[64];
	char		saberType[64];
	char		saber2Type[64];

	strcpy(siegeClass, client->sess.siegeClass);

	while (siegeClass[i])
	{ //sort of a hack.. we don't want spaces by siege class names have spaces so convert them all to unused chars
		if (siegeClass[i] == ' ')
		{
			siegeClass[i] = 1;
		}

		i++;
	}

	if (!siegeClass[0])
	{ //make sure there's at least something
		strcpy(siegeClass, "none");
	}

	//Do the same for the saber
	strcpy(saberType, client->sess.saberType);

	i = 0;
	while (saberType[i])
	{
		if (saberType[i] == ' ')
		{
			saberType[i] = 1;
		}

		i++;
	}

	strcpy(saber2Type, client->sess.saber2Type);

	i = 0;
	while (saber2Type[i])
	{
		if (saber2Type[i] == ' ')
		{
			saber2Type[i] = 1;
		}

		i++;
	}

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %s %s %s %i %i %i %i %i %i %i %s %s %s", 
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.teamLeader,
		client->sess.setForce,
		client->sess.saberLevel,
		client->sess.selectedFP,
		client->sess.duelTeam,
		client->sess.siegeDesiredTeam,
		siegeClass,
		saberType,
		saber2Type,
		client->sess.permissions,
		client->sess.arm,
		client->sess.totalTime,
		client->sess.makerModFlags,
		(int)client->sess.mark[0],
		(int)client->sess.mark[1],
		(int)client->sess.mark[2],
		client->sess.ip,
		#ifdef RANKINGMOD
		client->sess.playerUsername,
		#endif
		client->sess.username, // can be blank - keep at end
		client->sess.password  // can be blank - keep at end
		);

	var = va( "session%i", client - level.clients );

	trap_Cvar_Set( var, s );
}


/*
================
G_WriteNPCSessionData

For NPC anti-crash system.
================
*/
void G_WriteNPCSessionData()
{
	const char	*s;
	const char	*var;

	trap_Cvar_Set( "sessionNPC", va("%i", g_gametype.integer) );

	s = va("%i %i %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", 
		level.npcTypesFull,
		level.npcTypes,
		npcTypesArray[0].NPCTypes,
		npcTypesArray[1].NPCTypes,
		npcTypesArray[2].NPCTypes,
		npcTypesArray[3].NPCTypes,
		npcTypesArray[4].NPCTypes,
		npcTypesArray[5].NPCTypes,
		npcTypesArray[6].NPCTypes,
		npcTypesArray[7].NPCTypes,
		npcTypesArray[8].NPCTypes,
		npcTypesArray[9].NPCTypes,
		npcTypesArray[10].NPCTypes,
		npcTypesArray[11].NPCTypes,
		npcTypesArray[12].NPCTypes,
		npcTypesArray[13].NPCTypes,
		npcTypesArray[14].NPCTypes,
		npcTypesArray[15].NPCTypes
		);

	var = "sessionNPC";

	trap_Cvar_Set( var, s );
}
/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData( gclient_t *client ) {
	char	s[MAX_STRING_CHARS];
	const char	*var;
	int			i = 0;

	// bk001205 - format
	int teamLeader;
	int spectatorState;
	int sessionTeam;

	var = va( "session%i", client - level.clients );
	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i %i %i %i %i %i %s %s %s %i %i %i %i %f %f %f %s %s %s",
		&sessionTeam,                 // bk010221 - format
		&client->sess.spectatorTime,
		&spectatorState,              // bk010221 - format
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&teamLeader,                   // bk010221 - format
		&client->sess.setForce,
		&client->sess.saberLevel,
		&client->sess.selectedFP,
		&client->sess.duelTeam,
		&client->sess.siegeDesiredTeam,
		&client->sess.siegeClass,
		&client->sess.saberType,
		&client->sess.saber2Type,
		&client->sess.permissions,
		&client->sess.arm,
		&client->sess.totalTime,
		&client->sess.makerModFlags,
		&(client->sess.mark[0]),
		&(client->sess.mark[1]),
		&(client->sess.mark[2]),
		&client->sess.ip,
		#ifdef RANKINGMOD
		&client->sess.playerUsername,
		#endif
		&client->sess.username,  // can be blank keep at end
		&client->sess.password  // can be blank keep at end
		);

	while (client->sess.siegeClass[i])
	{ //convert back to spaces from unused chars, as session data is written that way.
		if (client->sess.siegeClass[i] == 1)
		{
			client->sess.siegeClass[i] = ' ';
		}

		i++;
	}

	i = 0;
	//And do the same for the saber type
	while (client->sess.saberType[i])
	{
		if (client->sess.saberType[i] == 1)
		{
			client->sess.saberType[i] = ' ';
		}

		i++;
	}

	i = 0;
	while (client->sess.saber2Type[i])
	{
		if (client->sess.saber2Type[i] == 1)
		{
			client->sess.saber2Type[i] = ' ';
		}

		i++;
	}

	// bk001205 - format issues
	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;

	client->ps.fd.saberAnimLevel = client->sess.saberLevel;
	client->ps.fd.saberDrawAnimLevel = client->sess.saberLevel;
	client->ps.fd.forcePowerSelected = client->sess.selectedFP;
}
/*
================
G_ReadNPCSessionData
================
*/
void G_ReadNPCSessionData()
{
	char		s[MAX_STRING_CHARS];
	const char	*var;

	var = "sessionNPC";

	trap_Cvar_VariableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		&level.npcTypesFull,
		&level.npcTypes,
		&npcTypesArray[0].NPCTypes,
		&npcTypesArray[1].NPCTypes,
		&npcTypesArray[2].NPCTypes,
		&npcTypesArray[3].NPCTypes,
		&npcTypesArray[4].NPCTypes,
		&npcTypesArray[5].NPCTypes,
		&npcTypesArray[6].NPCTypes,
		&npcTypesArray[7].NPCTypes,
		&npcTypesArray[8].NPCTypes,
		&npcTypesArray[9].NPCTypes,
		&npcTypesArray[10].NPCTypes,
		&npcTypesArray[11].NPCTypes,
		&npcTypesArray[12].NPCTypes,
		&npcTypesArray[13].NPCTypes,
		&npcTypesArray[14].NPCTypes,
		&npcTypesArray[15].NPCTypes
		);
}

/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData( gclient_t *client, char *userinfo, qboolean isBot ) {
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	client->sess.siegeDesiredTeam = TEAM_FREE;

	sess->makerModFlags = GMOD_GRABBING;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) {
		if ( g_teamAutoJoin.integer ) {
			sess->sessionTeam = PickTeam( -1 );
			BroadcastTeamChange( client, -1 );
		} else {
			// always spawn as spectator in team games
			if (!isBot)
			{
				sess->sessionTeam = TEAM_SPECTATOR;	
			}
			else
			{ //Bots choose their team on creation
				value = Info_ValueForKey( userinfo, "team" );
				if (value[0] == 'r' || value[0] == 'R')
				{
					sess->sessionTeam = TEAM_RED;
				}
				else if (value[0] == 'b' || value[0] == 'B')
				{
					sess->sessionTeam = TEAM_BLUE;
				}
				else
				{
					sess->sessionTeam = PickTeam( -1 );
				}
				BroadcastTeamChange( client, -1 );
			}
		}
	} else {
		value = Info_ValueForKey( userinfo, "team" );
		if ( value[0] == 's' ) {
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		} else {
			switch ( g_gametype.integer ) {
			default:
			case GT_FFA:
			case GT_HOLOCRON:
			case GT_JEDIMASTER:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					level.numNonSpectatorClients >= g_maxGameClients.integer ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_DUEL:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) {
					sess->sessionTeam = TEAM_SPECTATOR;
				} else {
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_POWERDUEL:
				//sess->duelTeam = DUELTEAM_LONE; //default
				{
					int loners = 0;
					int doubles = 0;

					G_PowerDuelCount(&loners, &doubles, qtrue);

					if (!doubles || loners > (doubles/2))
					{
						sess->duelTeam = DUELTEAM_DOUBLE;
					}
					else
					{
						sess->duelTeam = DUELTEAM_LONE;
					}
				}
				sess->sessionTeam = TEAM_SPECTATOR;
				break;
			}
		}
	}

	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;

	sess->siegeClass[0] = 0;
	sess->saberType[0] = 0;
	sess->saber2Type[0] = 0;
	sess->arm = DEFAULT_ARM_LENGTH;
	sess->username[0] = 0 ;
	sess->permissions = StringToPermissionsNum( g_defaultNonUserPermissions.string );
	VectorSet( sess->mark, 0, 0, 0 );
	client->sess.makerModFlags = GMOD_GRABBING;
	client->sess.totalTime = 0;
#ifdef RANKINGMOD
	sess->playerUsername[0] = ".";
#endif

	G_WriteClientSessionData( client );
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession( void ) {
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt ) {
		level.newSession = qtrue;
		G_Printf( "Gametype changed, clearing session data.\n" );
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData( void ) {
	int		i;

	trap_Cvar_Set( "session", va("%i", g_gametype.integer) );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_CONNECTED ) {
			G_WriteClientSessionData( &level.clients[i] );
		}
	}
	//trap_Cvar_Set( "sessionNPC", va("%i", g_gametype.integer) );
//	for ( i = 0 ; i < 15; i++ ) {
//		if ( npcTypesArray[i].NPCTypes[0] )
	//		G_WriteNPCSessionData();
//	}
}

/*void G_FreeMemorySession( void )
{
	int i;
	//G_Printf("%s",level.maxclients);
	for (i = level.maxclients; i < MAX_GENTITIES; i++) {
	//	G_LogPrintf("Point %i",i);
		//if (&AllocPoints[i].AllocPointer)
		//{
		//	G_Printf("Test %i",i);
		G_Free(i);
			//Btrap_TrueFree((void **)&AllocPoints[i].AllocPointer);
		//}
	}
}*/