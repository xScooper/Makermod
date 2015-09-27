// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "../qcommon/qfiles.h"
//#include "NPC_spawn.c" //Scooper
#include "mms_shader.h"


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

// Makermod new ban

typedef struct mmBan_s mmBan_t;
struct mmBan_s
{
	int			id;					// Id of the ban
	mmBan_t		*nextBan;			// Pointer to next ban
	int			type;				// type of ban. Normal / Hostname / Range
	char		*reason;			// Reason for ban
	int			expire;				// Time it expires.
	union {
		byte			IP[4];		// Holds the IP, Type 1
		byte			IP_r[2][4];	// Holds the IPs, Type 2
		struct {					// Holds the hostname, Type 3
			char		*host;		// Hostname
			qboolean	busy;		
		}Hostname; 
	};
};

// VVFIXME - We don't need this at all, but this is the quick way.
#ifdef _XBOX
#define	MAX_IPFILTERS	1
#else
#define	MAX_IPFILTERS	1024
#endif

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_CheckRange
=================
*/
qboolean G_CheckRange(byte m[])
{
	char		filename[MAX_TOKEN_CHARS];
	int 		length, ip[4], ip2[4];
	char		*data, *dataPtr;
	qboolean	inRange = qfalse;
	fileHandle_t 	f;


	Com_sprintf( filename, MAX_TOKEN_CHARS, "%s", g_banRanges.string );

	length = trap_FS_FOpenFile(filename, &f, FS_READ);


	if ( !f )
	{/*
		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: File not found\n\""));
		else*/
			G_Printf( "FAILED TO LOAD: File not found\n" );

		return qfalse;
	}

	if ( length > 127500 )
	{
		// The file's too large for our buffer.

/*		if ( ent )
			trap_SendServerCommand( ent-g_entities, va("print \"FAILED TO LOAD: File too large\n\""));
		else*/
			G_Printf( "FAILED TO LOAD: File too large\n" );

		trap_FS_FCloseFile( f );
		return qfalse;
	}

	data = malloc(128000);

	dataPtr = data;

	trap_FS_Read( data, min(length, 127500), f ); 

	data[min(length, 127500)] = '\0';
	while(*dataPtr != '\0')
	{
		if(*dataPtr == '\n')
			dataPtr++;
		if(*dataPtr == '\0')
			break;

		if ( (sscanf(dataPtr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) )
		{
			G_Printf( "Failed to load file.\n" );

			trap_FS_FCloseFile( f );
			free(data);
			return qfalse;
		}

		if(!strchr( dataPtr, '-' ))
		{
			G_Printf( va("Corrupt File: %s.\n", g_banRanges.string) );

			trap_FS_FCloseFile( f );
			free(data);
			return qfalse;
		}
		dataPtr = strchr( dataPtr, '-' );
		dataPtr++;
		dataPtr++;
		if ( (sscanf(dataPtr, "%d.%d.%d.%d", &ip2[0], &ip2[1], &ip2[2], &ip2[3]) != 4) )
		{
			G_Printf( "Failed to load file.\n" );

			trap_FS_FCloseFile( f );
			free(data);
			return qfalse;
		}

		if( (int)m[0] >= ip[0] )
		{
			inRange = qtrue;

			if( (int)m[0] == ip[0] )
			{
				if( (int)m[1] < ip[1] )
					inRange = qfalse;

				if( (int)m[1] == ip[1] )
				{
					if( (int)m[2] < ip[2] )
						inRange = qfalse;

					if( (int)m[2] == ip[2] )
						if ( (int)m[3] < ip[3] )
							inRange = qfalse;
				}
			}
		}

		if( inRange && (int)m[0] <= ip2[0] )
		{
			if( (int)m[0] == ip2[0] )
			{
				if( (int)m[1] > ip2[1] )
					inRange = qfalse;

				if( (int)m[1] == ip2[1] )
				{
					if( (int)m[2] > ip2[2] )
						inRange = qfalse;

					if( (int)m[2] == ip2[2] )
						if ( (int)m[3] > ip2[3] )
							inRange = qfalse;
				}
			}
		}else inRange = qfalse;

		if(inRange)
		{
			free(data);
			trap_FS_FCloseFile( f );
			return qtrue;
		}

		dataPtr = strchr( dataPtr, '\n' );
		if(dataPtr == NULL)
			break;
	}
	free(data);
	trap_FS_FCloseFile( f );
	return qfalse;
}

/*
=================
G_CheckIpFromFile
=================
*/
qboolean G_CheckIpFromFile(byte m[])
{
	char		filename[MAX_TOKEN_CHARS];
	int 		length, ip[4];
	char		*data, *dataPtr;
	fileHandle_t 	f;

	Com_sprintf( filename, MAX_TOKEN_CHARS, "%s", g_banList.string );

	length = trap_FS_FOpenFile(filename, &f, FS_READ);


	if ( !f )
	{
			G_Printf( "FAILED TO LOAD: File not found\n" );
		return qfalse;
	}

	if ( length > 127500 )
	{
			G_Printf( "FAILED TO LOAD: File too large\n" );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	data = malloc(128000);

	dataPtr = data;

	trap_FS_Read( data, min(length, 127500), f ); 

	data[min(length, 127500)] = '\0';
	while(*dataPtr != '\0')
	{
		if(*dataPtr == '\n')
			dataPtr++;
		if(*dataPtr == '\0')
			break;

		if ( (sscanf(dataPtr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4) )
		{
			G_Printf( "Failed to load file.\n" );

			trap_FS_FCloseFile( f );
			free(data);
			return qfalse;
		}
		if((int)m[0] == ip[0] &&
			(int)m[1] == ip[1] &&
			(int)m[2] == ip[2] &&
			(int)m[3] == ip[3])
		{
			free(data);
			trap_FS_FCloseFile( f );
			return qtrue;
		}

		dataPtr = strchr( dataPtr, '\n' );
		if(dataPtr == NULL)
			break;
	}

	free(data);
	trap_FS_FCloseFile( f );
	return qfalse;
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from, qboolean *isRange)
{
	byte			m[4];// = {'\0','\0','\0','\0'};
	int				i = 0;
	unsigned int	in;
	char			*p;

	while (i < 4)
	{
		m[i] = 0;
		i++;
	}

	i = 0;
	p = from;
	while (*p && i < 4) {
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned int *)m;

	if(G_CheckRange(m))
	{
		*isRange = qtrue;
		return g_filterBan.integer != 0;
	}
	if(G_CheckIpFromFile(m))
	{
		return g_filterBan.integer != 0;
	}

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str, qboolean Old )
{
	int		i, len;
	fileHandle_t f;

	if(Old)
	{
		for (i = 0 ; i < numIPFilters ; i++)
			if (ipFilters[i].compare == 0xffffffff)
				break;		// free spot
		if (i == numIPFilters)
		{
			if (numIPFilters == MAX_IPFILTERS)
			{
				G_Printf ("IP filter list is full\n");
				return;
			}
			numIPFilters++;
		}
		
		if (!StringToFilter (str, &ipFilters[i]))
			ipFilters[i].compare = 0xffffffffu;

		UpdateIPBans();
	}
	else
	{
		if(strchr(str, '-'))
			trap_FS_FOpenFile( g_banRanges.string, &f, FS_APPEND );
		else
			trap_FS_FOpenFile( g_banList.string, &f, FS_APPEND );

		len = strlen(str);
		str[len] = '\n';

		trap_FS_Write( str, len+1, f);
		trap_FS_FCloseFile(f);
	}
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t, qtrue );
		t = s;
	}
}

int ValidateIP( char *ip, int size )
{
	char	part[4][64];						// Set the size to 64 for now.
	int		i = 0, j, res;
	char	*p;

	p = ip;
	if(*p == ' ' || *p == '\r' || *p == '\n')	// Don't allow IP to start with a whitespace
		return 6;

	while(*p) {									// Replace . with ' ', so sscanf can do its thing.
		if( *p == '.') {
			*p = ' ';
			i++;
		}
		p++;
	}
												// I use this to make sure IPs are formatted in the right way.
	if(i != 3 || (sscanf(ip,"%s %s %s %s",part[0],part[1],part[2],part[3]) != 4))
		return 5;								// Format error. No need to put things back, won't be used anyways.

	p = ip;										// Put it all back.
	while(*p) {
		if(*p == ' ' && i) {					// Format should already be fine up until the final part of the IP.
			*p = '.';							// This won't put any whitespace that wasn't a . back.
			i--;								// <-- Because there's a counter.
		}
		p++;
	}
												// Now we need to make sure it's only numbers:
	for( i = 0; i < 4; i++) {
		p = part[i];
		while(*p) {
			if(*p < '0' || *p > '9')			
				return 1+i;						// Give error if there is something else in there.
			p++;
		}
		res = atoi(part[i]);					// If we've gotten this far, it's all numbers.
		if( res < 0 || res > 255)				// Check if it's within allowed range.
			return 1+i;
	}

	for(i = 0, res = 0, j = 0; i < 4; i++)
	{
		p = part[i];
		while(*p == '0') {
			p++;
			res++;
		}
		if(res > 1)
		{
			res = atoi(part[i]);
			Com_sprintf(part[i],sizeof(part[i]),"%i",res);
		}
		res = 0;
	}

	Com_sprintf(ip,size,"%s.%s.%s.%s",part[0],part[1],part[2],part[3]);
	return 0;
}
char	*ConcatArgs( int start );
/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];
	char		*p;
	int			error, i;
	

	if ( trap_Argc() < 2 || trap_Argc() == 3 || trap_Argc() > 4 ) {
		G_Printf("Usage: addip <ip-mask> OR <ip-mask - ip-mask>(for range ban)\n");
		return;
	}

	if(trap_Argc() == 2)
	{
		trap_Argv( 1, str, sizeof( str ) );

		error = ValidateIP(str, sizeof(str));				// Check if it's a valid IP

		if(!error) {							// No errors, go ahead.
			G_Printf( "IP: %s, added to banned IPs.",str );
			AddIP( str, qfalse );
			return;
		}
		else if(error == 6)
		{
			G_Printf("You can't start an IP with a whitespace character.");
			return;
		}
		else if(error == 5)
		{
			G_Printf("The IP you entered does not fit proper structure: X.X.X.X\n Where the X's are numbers between 0 and 255\n");
			return;
		}
		else
		{
			G_Printf("Illegal number detected in position %i\n Please use 0 - 255\n", error);
			return;
		}	
	}
	else 
	{
		char ip[2][20];
		char *p2;

		p = ConcatArgs(1);

		if(!strchr(p, '-') || (sscanf(p,"%s - %s", ip[0], ip[1]) != 2))
		{
			G_Printf("Usage: addip <ip-mask> OR <ip-mask - ip-mask>(for range ban)\n");
			return;
		}

		for(i = 0;i < 2;i++)
		{
			error = 0;
			error = ValidateIP(ip[i], sizeof(ip[i]));

			if(!error)
				continue;
			else if(error == 5)
			{
				G_Printf("One of the IPs you entered does not fit proper structure: X.X.X.X\n Where the X's are numbers between 0 and 255\n");
				return;
			}
			else
			{
				G_Printf("Illegal number detected in position %i of IP #%i\n Please use 0 - 255\n", error, i+1);
				return;
			}	
		}
		


		AddIP( va("%s - %s", ip[0], ip[1]), qfalse );
		G_Printf( "IP range: %s to %s, added to banned IP ranges.",ip[0], ip[1] );
	}

}

qboolean RemoveIP(char *str, char *file)
{
	char		filename[MAX_TOKEN_CHARS];
	int 		length;
	char		*data, *dataPtr, ipData[40], ipData2[17];
	char		list[1024][40];
	int			i = 0;
	fileHandle_t 	f;
	qboolean	found = qfalse;

	Com_sprintf( filename, MAX_TOKEN_CHARS, "%s", file );

	length = trap_FS_FOpenFile(filename, &f, FS_READ);

	if ( !f )
	{
			G_Printf( "FAILED TO LOAD: File not found\n" );
		return qfalse;
	}

	if ( length > 127500 )
	{
			G_Printf( "FAILED TO LOAD: File too large\n" );
		trap_FS_FCloseFile( f );
		return qfalse;
	}

	data = malloc(128000);

	dataPtr = data;

	trap_FS_Read( data, min(length, 127500), f ); 

	data[min(length, 127500)] = '\0';
	while(*dataPtr != '\0')
	{
		if(*dataPtr == '\n')
			dataPtr++;
		if(*dataPtr == '\0')
			break;

		if ( (sscanf(dataPtr, "%s", &ipData) != 1) )
		{
			G_Printf( "Failed to load file.\n" );

			trap_FS_FCloseFile( f );
			free(data);
			return qfalse;
		}
		if(strchr(dataPtr, '-'))
		{
			dataPtr = strchr(dataPtr, '-');
			dataPtr++;
			dataPtr++;
			if ( (sscanf(dataPtr, "%s", &ipData2) != 1) )
			{
				G_Printf( "Failed to load file.\n" );

				trap_FS_FCloseFile( f );
				free(data);
				return qfalse;
			}
			strcpy(ipData,va("%s - %s",ipData, ipData2));
		}
		if(i == 1024)
		{
			G_Printf( "Max number of IPs hit.\n" );

			trap_FS_FCloseFile( f );
			free(data);
			break;
		}

		if(strcmp(str, ipData))
		{
			length = strlen(ipData);
			strcpy(list[i], ipData);
			list[i][length+1] = '\0';
			list[i][length] = '\n';
			i++;
		}else found = qtrue;
		
		dataPtr = strchr( dataPtr, '\n' );
		if(dataPtr == NULL)
			break;
	}

	free(data);
	trap_FS_FCloseFile( f );

	trap_FS_FOpenFile(filename, &f, FS_WRITE);

	list[i][0] = '\0';

	for(i = 0; i < 1024; i++)
	{
		if(!list[i] || !list[i][0])
			break;
		trap_FS_Write( list[i], strlen(list[i]), f);
	}
	trap_FS_FCloseFile(f);

	return found;
}

/*
=================
Svcmd_ListBans_f
=================
*/
void Svcmd_ListBans_f(void)
{
	G_Printf("Banned IP's: %s\n\"", g_banIPs.string);
	G_Printf("%s: \n", g_banList.string);
	listbans_f(NULL, g_banList.string);
	G_Printf("%s: \n", g_banRanges.string);
	listbans_f(NULL, g_banRanges.string);
}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];
	char		*p;
	qboolean	found = qfalse;

	if ( trap_Argc() < 2 || trap_Argc() == 3 || trap_Argc() > 4 ) {
		G_Printf("Usage:  sv removeip <ip-mask> OR <ip-mask - ip-mask>(for range ban)\n");
		return;
	}

	if( trap_Argc() == 2)
	{

		trap_Argv( 1, str, sizeof( str ) );


		if(RemoveIP(str, g_banList.string))
			found = qtrue;

		if (!StringToFilter (str, &f))
			return;

		for (i=0 ; i<numIPFilters ; i++) {
			if (ipFilters[i].mask == f.mask	&&
				ipFilters[i].compare == f.compare) {
				ipFilters[i].compare = 0xffffffffu;
				G_Printf ("Removed.\n");

				UpdateIPBans();
				return;
			}
		}
	}
	else 
	{
		p = ConcatArgs(1);
		if(RemoveIP(p, g_banRanges.string))
			return;
	}

	if(!found)
		G_Printf ( "Didn't find %s.\n", str );
}

void TravMD3(const char *buffer, int level)
{
	int numEntries;
	char buffer2[MAX_TOKEN_CHARS];
	char *list;//[20048];
	char* listptr;
	int i, length;

	char fileData[MAX_OBINFO_FILE_LENGTH];
	fileHandle_t	f;
	int j,k;
	md3Header_t* header;
	md3Surface_t* surface;
	char* data, *dataPtr;
	int totalTriangles = 0;
	int maxTriangles = 0;
	int surfTriangles;

	char maxName[64];
	int maxSurfCount;

	list = malloc(20048);

	numEntries = trap_FS_GetFileList(buffer, ".md3", list, 20048 );

	listptr = list;
	

	for( i = 0 ; i < numEntries ; i++ )
	{
		length = strlen( listptr );

		/*if ( strchr(listptr,'/') || strchr(listptr,'\\') )
		{
			int numEntries2;
			char list2[2048];
			char* listptr2;

			// this list contains folders, display only the folders
//			MM_SendMessage( ent-g_entities, va("print \"Use /mlist <folder-name> to see the contents of one of these folders.\n\""));
			//G_Printf("bleh\n");
			numEntries2 = trap_FS_GetFileList(buffer, "/", list2, 2048 );
			listptr2 = list2;

			if(!(*listptr2))
				listptr2++;
			for( i = 0 ; i < min(numEntries2,100) ; i++ )
			{
				//MM_SendMessage( ent-g_entities, va("print \"%s\n\"", listptr) );
			//	for( j = 0; j < level;j++)
			//		G_Printf("\t");

		//		G_Printf("%s\n", listptr);
				Q_strncpyz(buffer2,va("%s%s",buffer,listptr2),MAX_TOKEN_CHARS);
				TravMD3(buffer2,level+1);
				listptr2 += (strlen( listptr2 ) + 1);
			}

		//	return;
		}
		else
		{*/
			length = strlen( listptr );
			*(listptr + length) = 0;
	//		for( j = 0; j < level;j++)
	//				G_Printf("\t");
			G_Printf("%s", listptr);
			//listptr += (length + 1);
	//	}

		j = trap_FS_FOpenFile( va( "models/%s", listptr), &f, FS_READ);

		data = malloc(j);

		dataPtr = data;

		trap_FS_Read( data, j/*min(j, 127449)*/, f ); 

		//data[min(j, 127500)] = '\0';

		if ( j < sizeof( md3Header_t ) )
		{
			trap_FS_FCloseFile(f);
			free(data);
			return;
		}

		header = (md3Header_t*) data;
		surface = (md3Surface_t*) (data+header->ofsSurfaces);
		dataPtr = (char*)surface;

		surfTriangles = 0;
		for(k = 0; k < header->numSurfaces;k++)
		{
			G_Printf(" %i",surface->numTriangles);
			totalTriangles += surface->numTriangles;
			surfTriangles += surface->numTriangles;

			surface = (md3Surface_t*)(dataPtr+surface->ofsEnd);
			dataPtr = (char*)surface;
		}
		if(surfTriangles > maxTriangles)
		{
			maxTriangles = surfTriangles;
			Q_strncpyz(maxName,header->name,64);
			maxSurfCount = header->numSurfaces;
		}
		G_Printf("\n");
		free(data);
		trap_FS_FCloseFile(f);

		listptr += (length + 1);
	}
	free(list);
	G_Printf("Total triangles: %i\n",totalTriangles);
	G_Printf("Average triangles: %i/%i = %f",totalTriangles,numEntries,(float)totalTriangles/(float)numEntries);
	G_Printf("Max triangles: %i\n",maxTriangles);
	G_Printf("Num surfaces: %i, Name: %s\n",maxSurfCount,maxName);
}

void Svcmd_FindMD3_f ( void ) {
	char buffer[MAX_TOKEN_CHARS];
	float rot[4][4];

	strcpy(buffer,"models/");
	TravMD3(buffer, 0);
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_NPC:
			G_Printf("ET_NPC              ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

int	ClientForString( const char *name ) {
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
			G_Printf("print \"Bad client slot: %i\n\"", num);
			return -1;
		}

		cl = &level.clients[num];
		if ( cl->pers.connected != CON_CONNECTED ) {
			G_Printf("print \"Client %i is not active\n\"", num);
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

	G_Printf( "print \"User %s is not on the server\n\"", s2 );
	return -1;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	int	cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( cl == -1 ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl], str );
}

/*
=================
Svcmd_NPC_Kill_f
=================
*/
void Svcmd_NPC_Kill_f( void ) 
{
	int			n;
	gentity_t	*player;
//	char		name[1024];
//	team_t		killTeam = TEAM_FREE;
//	qboolean	killNonSF = qfalse;
	qboolean	killEverything = qtrue;


//	killEverything = qtrue;


	for ( n = 1; n < ENTITYNUM_MAX_NORMAL; n++) 
	{
		player = &g_entities[n];
		if (!player->inuse ) {
			continue;
		}

		else if ( player && player->NPC && player->client )
		{
			if( killEverything == qtrue )
			{
				Com_Printf( S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname );
				player->health = 0;
				player->client->ps.stats[STAT_HEALTH] = 0;
				if (player->die)
				{
					player->die(player, player, player, 100, MOD_UNKNOWN);
				}
			}
		}
	}
}



/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return qtrue;
	}

/*	if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}
*/
	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "findmd3") == 0) {
		Svcmd_FindMD3_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
//		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		Svcmd_ListBans_f();
		return qtrue;
	}
/*
	if (Q_stricmp (cmd, "nav") == 0) {
		Svcmd_Nav_f();
		return qtrue;
	}*/

	if (Q_stricmp(cmd, "mpsay") == 0)
	{
		char* message;
		char client[MAX_STRING_CHARS];
		int target;

		if (trap_Argc() < 3 )
		{
			G_Printf("print \"Command usage: mpsay <client> <message>\n\"" );
			return qtrue;
		}

		trap_Argv( 1, client, sizeof(client) );
		
		if ( Q_stricmp(client, "all") == 0 )
		{
			target = -1;
		}
		else
		{
			target = ClientForString(client);
		
			if ( target == -1 )
				return qtrue;
		}

		message = ConcatArgs( 2 );

		if(strstr(message, "@@@"))	// Hotfix
		{
			G_Printf( "print \"Illegal message detected. Please choose a different message.\n\"");
			return qtrue;
		}
		
		trap_SendServerCommand( target, va("cp \"%s\n\"", message ) );

		if ( target == -1 )
			G_LogScreenPrintAll();
		else
			level.clients[target].pers.lastMessageTime = level.time;

		return qtrue;
	}

	if (Q_stricmp(cmd, "mannounce") == 0)
	{
		char* message;

		if (trap_Argc() < 2 )
		{
			G_Printf( "print \"Command usage: mannounce <message>\n\"" );
			return qtrue;
		}

		message = ConcatArgs( 1 );

		if(strstr(message, "@@@"))	// Hotfix
		{
			G_Printf( "print \"Illegal message detected. Please choose a different message.\n\"");
			return qtrue;
		}

		Q_strncpyz( level.announcement, message, sizeof(level.announcement) );
	
		level.announceStop = level.time + 20000; //Show message for 20 secs
		return qtrue;
	}

	if (Q_stricmp( cmd, "npc" ) == 0 && g_dedicated.integer)
	{
		Svcmd_NPC_Kill_f();
		return qtrue;
	}
#ifdef MM_BRUSH_OLD
	if (Q_stricmp( cmd, "writebsp" ) == 0 && g_dedicated.integer)
	{
		Cmd_WriteBsp_f(NULL);
		return qtrue;
	}
#endif
	if (Q_stricmp( cmd, "mremap" ) == 0)
	{
		Cmd_mRemap_f(NULL);
		return qtrue;
	}
	if (Q_stricmp( cmd, "mmapmusic" ) == 0)
	{
		Cmd_mMapMusic_f(NULL);
		return qtrue;
	}
	if (Q_stricmp( cmd, "mweather" ) == 0)
	{
		Cmd_mWeather_f(NULL);
		return qtrue;
	}
	if (Q_stricmp( cmd, "mclients" ) == 0)
	{
		gclient_t* cl;
		int i;
		char buffer[MAX_TOKEN_CHARS];

		for ( i=0, cl=level.clients ; i < level.maxclients ; i++, cl++ ) 
		{
			if ( cl->pers.connected != CON_CONNECTED && cl->pers.connected != CON_CONNECTING) 
				continue;
			SanitizeString2( cl->pers.netname, buffer );
			G_Printf("%i %-20.20s %13i\n", i, buffer, playerClients[i].version );
		}
		return qtrue;
	}
#ifndef MM_RELEASE
	if (Q_stricmp( cmd, "svsay2" ) == 0)
	{
		char		*p;

		/*if ( trap_Argc() != 2 )
		{
		//	G_Printf("Command usage: mmapshader - <Old-Shader> <New-Shader>\n\"");
			return qtrue;
		}*/

		
		p = ConcatArgs( 1 );

		ServerMessage( p, g_ServerName.string);
		return qtrue;
	}
	#endif

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}

