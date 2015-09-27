// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_mem.c
//


#include "g_local.h"


#define POOLSIZE	(256 * 1024)

static char		memoryPool[POOLSIZE];
static int		allocPoint;
int test = 0;

void *G_Alloc( int size ) {
	char	*p;

	if ( g_debugAlloc.integer ) {
		G_Printf( "G_Alloc of %i bytes (%i left)\n", size, POOLSIZE - allocPoint - ( ( size + 31 ) & ~31 ) );
	}

	if ( allocPoint + size > POOLSIZE ) {
	  G_Error( "G_Alloc: failed on allocation of %i bytes\n", size ); // bk010103 - was %u, but is signed
		return NULL;
	}

	p = &memoryPool[allocPoint];
	//G_Printf("%s",p);

	allocPoint += ( size + 31 ) & ~31;

	return p;
}

char *G_FindAlloc( char *name  ) {
	int i, l1, l2;
	char test[MAX_STRING_CHARS];
	char *buffer;


	l1 = strlen(name);
	for(i = 0;i < allocPoint;i++)
	{
		sscanf(&memoryPool[i],"%s",test);
		l2 = strlen(test);
		if(l1 == l2)
		{
			if ( !strcmp( test, name )) 
			{
				buffer = &memoryPool[i];
//				G_Printf("%s - %s",test, buffer);
				return buffer;
			}
		}
	}
	return 0;

}

char* stringAllocationTable[MAX_GENTITIES];

void G_Free ( int i )
{
	if ( stringAllocationTable[i] != NULL )
	{
		trap_TrueFree ((void**)&stringAllocationTable[i]);
		stringAllocationTable[i] = NULL;
	}
	/*if (!(AllocPoints[i].AllocPointer == 0) && (Allocated[i] == 1))
	{
		trap_TrueFree((void **)&AllocPoints[i].AllocPointer);
		AllocPoints[i].AllocPointer = 0;
		Allocated[i] = 0;
		return;
	}*/
}

void G_InitAllocation ( void )
{
	memset (stringAllocationTable, 0, sizeof (stringAllocationTable));
}

char* G_RequestAllocation ( int entityNum, int size )
{
	G_Free (entityNum);
	trap_TrueMalloc ((void**)&stringAllocationTable[entityNum], size);
	
	if ( stringAllocationTable[entityNum] == NULL )
	{
		G_Error ("Failed to allocate %d bytes of memory for entity %d\n", size, entityNum);
	}
	
	return stringAllocationTable[entityNum];
}

void G_ClearAllocationTable ( void )
{
	int i;
	
	for ( i = 0; i < MAX_GENTITIES; ++i )
	{
		G_Free (i);
	}
}

void G_InitMemory( void ) {
	allocPoint = 0;
}

void Svcmd_GameMem_f( void ) {
	G_Printf( "Game memory status: %i out of %i bytes allocated\n", allocPoint, POOLSIZE );
}
