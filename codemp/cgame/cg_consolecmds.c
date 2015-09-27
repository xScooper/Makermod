// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "bg_saga.h"
extern menuDef_t *menuScoreboard;

#ifdef MM_BRUSH_OLD
#include "../qcommon/mm_brushmodels.h"
#endif


#ifdef MM_BRUSH_OLD
clipMap_t *cm = (clipMap_t*)0xB2AEE0; //0x4C8400;

cmodel_t	*box_model = (cmodel_t*)0xB2AE88;//0x4C83C0;
cplane_t	**box_planes = (cplane_t**)0xB2AE84;//0x4C84D8;
cbrush_t	**box_brush = (cbrush_t**)0xB2AE80;//0x4C84C0;

cbrushside_t *brushsidesNew;
cplane_t	*planesNew;
cLeaf_t		*leafsNew;
int			*leafbrushesNew;
cmodel_t	*cmodelsNew;
cbrush_t	*brushesNew;

int			*numSubModels2 = (int*)0x00B29678;
//int			*numSubModels3 = (int*)0x30A3F568;

void CG_CheckMemory_f( void )
{
	char buffer[1024];
	int start, stop;

	if(trap_Argc() != 3)
	{
		CG_Printf("Wrong number of arguments\n");
		return;
	}

	trap_Argv(1, buffer, sizeof(buffer));
	start = atoi(buffer);
	trap_Argv(2, buffer, sizeof(buffer));
	stop = atoi(buffer);

	for(;start < stop;start++)
		CG_Printf("cm->planes[%d] = %d\n",start,cm->planes[start]);


	return;
}

int priority = 0;
void CG_ApplyMemory_f( void )
{
	char buffer[1024];
	int version;

	if(trap_Argc() != 2)
	{
		CG_Printf("Wrong number of arguments\n");
		return;
	}

	trap_Argv(1, buffer, sizeof(buffer));
	version = atoi(buffer);

	if(priority)
		version = priority;

	if(version == 1 || version == 8)
		cm->brushsides = brushsidesNew;
	if(version == 2 || version == 8)
		cm->planes = planesNew;
	if(version == 3 || version == 8)
		cm->leafs = leafsNew;
	if(version == 4 || version == 8)
		cm->leafbrushes = leafbrushesNew;
	if(version == 5 || version == 8)
		cm->cmodels = cmodelsNew;
	if(version == 6 || version == 8)
		cm->brushes = brushesNew;
	if(version == 7 || version == 8)
	{
		(*box_planes) = &cm->planes[cm->numPlanes];
		(*box_brush) = &cm->brushes[cm->numBrushes];
	}

	CG_Printf("Client: Success.\n");
}


void CG_FixMemory_f( void )
{
	/*cbrushside_t *brushsidesNew;
	cplane_t	*planesNew;
	cLeaf_t		*leafsNew;
	int			*leafbrushesNew;
	cmodel_t	*cmodelsNew;
	cbrush_t	*brushesNew;*/
	int i, j, *indexes;
	char buffer[1024];
	int version = 0;

	if(!cm)
		return;

	if(trap_Argc() == 2)
	{
		trap_Argv(1, buffer, sizeof(buffer));
		version = atoi(buffer);
	}

	brushsidesNew = malloc((cm->numBrushSides + 6 + BOX_SIDES) * sizeof(cbrushside_t));		// + 48
	memset(brushsidesNew, 0, (cm->numBrushSides + 6 + BOX_SIDES) * sizeof(cbrushside_t));
	memcpy(brushsidesNew, cm->brushsides, sizeof(cbrushside_t) * (cm->numBrushSides + BOX_SIDES));

	planesNew = malloc((cm->numPlanes + 12 + BOX_PLANES) * sizeof(cplane_t));				// + 60
	memset(planesNew, 0, (cm->numPlanes + 12 + BOX_PLANES) * sizeof(cplane_t));
	memcpy(planesNew, cm->planes, sizeof(cplane_t) * (cm->numPlanes + BOX_PLANES));
//	CG_Printf("%d\n",cm->planes[60]);
//	CG_Printf("%d\n",planesNew[60]);

	leafsNew = malloc((cm->numLeafs + 1 + BOX_LEAFS) * sizeof(cLeaf_t));					// + 6
	memset(leafsNew, 0,(cm->numLeafs + 1 + BOX_LEAFS) * sizeof(cLeaf_t));
	memcpy(leafsNew, cm->leafs, sizeof(cLeaf_t) * (cm->numLeafs + BOX_LEAFS));

	leafbrushesNew = malloc((cm->numLeafBrushes + 1 + BOX_BRUSHES) * sizeof(int));			// + 4
	memset(leafbrushesNew, 0,(cm->numLeafBrushes + 1 + BOX_BRUSHES) * sizeof(int));
	memcpy(leafbrushesNew, cm->leafbrushes, sizeof(int) * (cm->numLeafBrushes + BOX_BRUSHES));

	cmodelsNew = malloc((cm->numSubModels + 1) * sizeof(cmodel_t));
	memset(cmodelsNew, 0,(cm->numSubModels + 1) * sizeof(cmodel_t));
	memcpy(cmodelsNew, cm->cmodels, sizeof(cmodel_t) * cm->numSubModels);

	brushesNew = malloc((cm->numBrushes + 1 + BOX_BRUSHES) * sizeof(cbrush_t));		// + 5
	memset(brushesNew, 0,(cm->numBrushes + 1 + BOX_BRUSHES) * sizeof(cbrush_t));
	memcpy(brushesNew, cm->brushes, sizeof(cbrush_t) * (cm->numBrushes + BOX_BRUSHES));


	for(i = 0; i < cm->numBrushSides+BOX_SIDES; i++)
		brushsidesNew[i].plane = &planesNew[cm->brushsides[i].plane - cm->planes];

	for(i = 0; i < cm->numBrushes+BOX_BRUSHES; i++)
	{
		cbrush_t *brush = &brushesNew[i];

		//for(j = 0; j < brush->numSides; j++)
			brush->sides = &brushsidesNew[cm->brushes[i].sides - cm->brushsides];
	}

	
	for(i = 1; i < cm->numSubModels; i++)
	{
		cLeaf_t *leaf = &cmodelsNew[i].leaf;

		indexes = malloc(leaf->numLeafBrushes * 4);
		leaf->firstLeafBrush = indexes - leafbrushesNew;
		for( j = 0; j < leaf->numLeafBrushes; j++)
			indexes[j] = cm->leafbrushes[cm->cmodels[i].leaf.firstLeafBrush + j];
	}


	/*cm->brushsides = brushsidesNew;
	cm->planes = planesNew;
	cm->leafs = leafsNew;
	cm->leafbrushes = leafbrushesNew;
	cm->cmodels = cmodelsNew;
	cm->brushes = brushesNew;*/

/*	for(i = 0; i < cm->numLeafs; i++)
	{
		cLeaf_t *temp = cm->leafs[i];

		temp->firstLeafSurface = cm->leafs - 
	}*/
	
	if(version == 1)
	{
		priority = 8;
		CG_ApplyMemory_f();
		priority = 0;
	}
	

	CG_Printf("Client: Success.\n");


}

void CG_AddBrushModel_f(void)
{
	cmodel_t *cmodel;
	cplane_t *plane;
	cbrushside_t *brushside;
	cbrush_t *brush;
	cLeaf_t *leaf;
	int i, *indexes;

	if(!cm)
		return;

	//Planes Start

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 0];		// 46
	VectorSet(plane->normal, 1.0f, 0.0f, 0.0f);
	plane->dist = -64.0f;
	

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 1];		// 47
	VectorSet(plane->normal, -1.0f, 0.0f, 0.0f);
	plane->dist = 64.0f;
	plane->type = 3;
	plane->signbits = 1;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 2];		// 48
	VectorSet(plane->normal, 1.0f, 0.0f, 0.0f);
	plane->dist = 64.0f;
//	plane->dist = 72.0f;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 3];		// 49
	VectorSet(plane->normal, -1.0f, 0.0f, 0.0f);
	plane->dist = -64.0f;
//	plane->dist = -72.0f;
	plane->type = 3;
	plane->signbits = 1;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 4];		// 50
	VectorSet(plane->normal, 0.0f, 1.0f, 0.0f);
	plane->dist = -64.0f;
	plane->type = 1;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 5];		// 51
	VectorSet(plane->normal, 0.0f, -1.0f, 0.0f);
	plane->dist = 64.0f;
	plane->type = 3;
	plane->signbits = 2;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 6];		// 52
	VectorSet(plane->normal, 0.0f, 1.0f, 0.0f);
	plane->dist = 64.0f;
	plane->type = 1;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 7];		// 53
	VectorSet(plane->normal, 0.0f, -1.0f, 0.0f);
	plane->dist = -64.0f;
	plane->type = 3;
	plane->signbits = 2;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 8];		// 54
	VectorSet(plane->normal, 0.0f, 0.0f, 1.0f);
//	plane->dist = -80.0f;
	plane->dist = -0.0f;
	plane->type = 2;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 9];		// 55
	VectorSet(plane->normal, 0.0f, 0.0f, -1.0f);
//	plane->dist = 80.0f;
	plane->dist = 0.0f;
	plane->type = 3;
	plane->signbits = 4;

	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 10];	// 56
	VectorSet(plane->normal, 0.0f, 0.0f, 1.0f);
//	plane->dist = -72.0f;
	plane->dist = 8.0f;
	plane->type = 2;
	
	plane = &cm->planes[cm->numPlanes + BOX_PLANES + 11];	// 57
	VectorSet(plane->normal, 0.0f, 0.0f, -1.0f);
//	plane->dist = 72.0f;
	plane->dist = -8.0f;
	plane->type = 3;
	plane->signbits = 4;

	//Planes End

	//Brushsides Start
	
	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 0];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 1]; // 47
	brushside->shadernum = 1;

	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 1];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 2]; // 48
	brushside->shadernum = 1;

	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 2];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 5]; // 51
	brushside->shadernum = 1;

	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 3];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 6]; // 52
	brushside->shadernum = 1;

	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 4];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 9]; // 55
	brushside->shadernum = 1;

	brushside = &cm->brushsides[cm->numBrushSides + BOX_SIDES + 5];
	brushside->plane = &cm->planes[cm->numPlanes + BOX_PLANES + 10]; // 56
	brushside->shadernum = 1;

	//Brushsides End

	// cbrush_t start

	brush = &cm->brushes[cm->numBrushes + BOX_BRUSHES + 0];

	brush->shaderNum = 1;
	brush->contents = 32769;
//	VectorSet(brush->bounds[0], -64.0f, -64.0f, -80.0f);
//	VectorSet(brush->bounds[1], 64.0f, 64.0f, 0.0f);
	VectorSet(brush->bounds[0], -64.0f, -64.0f, -0.0f );
	VectorSet(brush->bounds[1], 64.0f, 64.0f, 8.0f);
	brush->numSides = 6;
	brush->sides = &cm->brushsides[cm->numBrushSides + BOX_SIDES];

	// cbrush_t end

	// cleaf_t start

	leaf = &cm->leafs[cm->numLeafs + BOX_LEAFS + 0];
	leaf->firstLeafBrush = cm->numLeafBrushes + BOX_BRUSHES;//26;
	leaf->numLeafBrushes = 1;
	leaf->firstLeafSurface = 30;
	leaf->numLeafSurfaces = 0;

	// cleaf_t end

	// leafbrushes start

	cm->leafbrushes[cm->numLeafBrushes + BOX_BRUSHES + 0] = cm->numBrushes + BOX_BRUSHES;//8;

	// leafbrushes end

	// cmodel start

	cmodel = &cm->cmodels[cm->numSubModels /*+ 1*/ + 0];
	cmodel->counter = -1;
	cmodel->leaf.area = 0;
	cmodel->leaf.numLeafBrushes = 1;

	indexes = malloc(cmodel->leaf.numLeafBrushes * 4);
	cmodel->leaf.firstLeafBrush = indexes - cm->leafbrushes;
	for( i = 0; i < cmodel->leaf.numLeafBrushes; i++)
		indexes[i] = cm->leafbrushes[cm->numLeafBrushes + BOX_BRUSHES + i];
	cmodel->leaf.firstLeafSurface = 30;
	cmodel->leaf.numLeafSurfaces = 0;
//	VectorSet(cmodel->mins, -65.0f, -65.0f, -81.0f);
//	VectorSet(cmodel->maxs, 65.0f, 65.0f, -71.0f);
	VectorSet(cmodel->mins, -65.0f, -65.0f, -1.0f );
	VectorSet(cmodel->maxs, 65.0f, 65.0f, 9.0f);

	//cmodel end

	cm->numBrushes += (1 + BOX_BRUSHES);
	cm->numBrushSides += (6 + BOX_SIDES);
	cm->numPlanes += (12 + BOX_PLANES);
	cm->numLeafs += (1 + BOX_LEAFS);
	cm->numLeafBrushes += (1 + BOX_BRUSHES);
	cm->numSubModels += (1 /*+ 1*/);
	(*numSubModels2) += 1;
//	(*numSubModels3) += 1;

	CG_Printf("Client: Success.\n");
}

#endif 


void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("%s (%i %i %i) : %i\n", cgs.mapname, (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdef.viewangles[YAW]);
}


static void CG_ScoresDown_f( void ) {

	CG_BuildSpectatorString();
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

extern menuDef_t *menuScoreboard;
void Menu_Reset();			// FIXME: add to right include file

static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}


static void CG_spWin_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint(CG_GetStringEdString("MP_INGAME", "YOU_WIN"), SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
	CG_AddBufferedSound(cgs.media.loserSound);
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint(CG_GetStringEdString("MP_INGAME", "YOU_LOSE"), SCREEN_HEIGHT * .30, 0);
}


static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}


/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		trap_Cvar_Set ("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

void CG_SiegeBriefingDisplay(int team, int dontshow);
static void CG_SiegeBriefing_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 0);
}

static void CG_SiegeCvarUpdate_f(void)
{
	int team;

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	team = cg.predictedPlayerState.persistant[PERS_TEAM];

	if (team != SIEGETEAM_TEAM1 &&
		team != SIEGETEAM_TEAM2)
	{ //cannot be displayed if not on a valid team
		return;
	}

	CG_SiegeBriefingDisplay(team, 1);
}
static void CG_SiegeCompleteCvarUpdate_f(void)
{

	if (cgs.gametype != GT_SIEGE)
	{ //Cannot be displayed unless in this gametype
		return;
	}

	// Set up cvars for both teams
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM1, 1);
	CG_SiegeBriefingDisplay(SIEGETEAM_TEAM2, 1);
}
/*
static void CG_Camera_f( void ) {
	char name[1024];
	trap_Argv( 1, name, sizeof(name));
	if (trap_loadCamera(name)) {
		cg.cameraMode = qtrue;
		trap_startCamera(cg.time);
	} else {
		CG_Printf ("Unable to load camera %s\n",name);
	}
}
*/


#ifdef MM_BRUSH_OLD
//char *(__fastcall *Key_KeynumToString)(int keynum, int destsize, char* dest) = (char *(__fastcall *)(int keynum,int destsize, char* dest))0x423E10;
//void (__usercall *Key_GetBindingBuf)( int keynum, char *buf, int buflen ) = (void (__cdecl *)( int keynum, char *buf, int buflen ))0x423E10;
//char *names = (char*)0x570BCC;

void Key_GetBindingBuf(int keynum, char *buf, int buflen )
{
	__asm {
		pushad;
		pushfd;
		mov ebx, buflen;
		mov ecx, keynum;
		mov edi, buf;
		mov eax, 0x423E10;
		call eax;
		popfd;
		popad;
	};
}

static void CG_mKey_f (void)
{
	char buffer[100] = { 0 };
	char bigBuffer[320000] = { 0 };
	int i;
	fileHandle_t fh;
	trap_FS_FOpenFile("TheList.txt",&fh,FS_WRITE);
	for(i = 0; i < MAX_KEYS;i++)
	{
		Key_GetBindingBuf(i, buffer, 100);
		strcat(bigBuffer,va("%i \"%s\"\n",i,buffer));
	}
	trap_FS_Write(bigBuffer,sizeof(bigBuffer),fh);
	trap_FS_FCloseFile(fh);
}
#endif

//iojamp start

typedef struct {
	char	*name;
	int		offset;
	int		bits;		// 0 = float
} netField_t;

netField_t *entitystate = (netField_t*)0x0056E7F0;
netField_t *playerstate = (netField_t*)0x0056EE20;
netField_t *playerstate2 = (netField_t*)0x56FB20;
netField_t *playerstate3 = (netField_t*)0x56F490;
int numEntityFields = 132;
int numPlayerFields = 137;
int numPlayerFields2 = 69;
int numPlayerFields3 = 140;
void CG_IOJamp_f()
{
	int i;
	for(i = 0; i < numPlayerFields2; i++)
	{
		//{ NETF( pos.trTime ),                   32  },
		//Com_Printf("{ NETF( %s ),%22i  },\n", entitystate[i].name, entitystate[i].bits);
		//{ PSF( commandTime ),                    32 },
		Com_Printf("{ PSF( %s ),%30i  },\n", playerstate2[i].name, playerstate2[i].bits);

		//Com_Printf("#%i %s %i\n",i,entitystate[i].name, entitystate[i].bits);
	}
	Com_Printf("SPLIT");
	for(i = 0; i < numPlayerFields3; i++)
	{
		//{ NETF( pos.trTime ),                   32  },
		//Com_Printf("{ NETF( %s ),%22i  },\n", entitystate[i].name, entitystate[i].bits);
		//{ PSF( commandTime ),                    32 },
		Com_Printf("{ PSF( %s ),%30i  },\n", playerstate3[i].name, playerstate3[i].bits);

		//Com_Printf("#%i %s %i\n",i,entitystate[i].name, entitystate[i].bits);
	}
}

//iojamp end

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	//{ "teststuff", CG_TestStuff_f },
	//{ "testpatch", CG_TestPatch_f },
	{ "iojamp}", CG_IOJamp_f },					//iojamp
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponclean", CG_WeaponClean_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "spWin", CG_spWin_f },
	{ "spLose", CG_spLose_f },
	{ "scoresDown", CG_scrollScoresDown_f },
	{ "scoresUp", CG_scrollScoresUp_f },
	{ "startOrbit", CG_StartOrbit_f },
	//{ "camera", CG_Camera_f },
	{ "loaddeferred", CG_LoadDeferredPlayers },
	{ "invnext", CG_NextInventory_f },
	{ "invprev", CG_PrevInventory_f },
	{ "forcenext", CG_NextForcePower_f },
	{ "forceprev", CG_PrevForcePower_f },
	{ "briefing", CG_SiegeBriefing_f },
	{ "siegeCvarUpdate", CG_SiegeCvarUpdate_f },
	{ "siegeCompleteCvarUpdate", CG_SiegeCompleteCvarUpdate_f },
#ifdef MM_BRUSH_OLD
	{ "c_mfixmemory", CG_FixMemory_f },
	{ "c_maddbrush", CG_AddBrushModel_f },
	{ "c_mapplymemory", CG_ApplyMemory_f },
	{ "c_mem", CG_CheckMemory_f },
	{ "mkey", CG_mKey_f },
#endif
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("forcechanged");
	trap_AddCommand ("sv_invnext");
	trap_AddCommand ("sv_invprev");
	trap_AddCommand ("sv_forcenext");
	trap_AddCommand ("sv_forceprev");
	trap_AddCommand ("sv_saberswitch");
	trap_AddCommand ("engage_duel");
	trap_AddCommand ("force_heal");
	trap_AddCommand ("force_speed");
	trap_AddCommand ("force_throw");
	trap_AddCommand ("force_pull");
	trap_AddCommand ("force_distract");
	trap_AddCommand ("force_rage");
	trap_AddCommand ("force_protect");
	trap_AddCommand ("force_absorb");
	trap_AddCommand ("force_healother");
	trap_AddCommand ("force_forcepowerother");
	trap_AddCommand ("force_seeing");
	trap_AddCommand ("use_seeker");
	trap_AddCommand ("use_field");
	trap_AddCommand ("use_bacta");
	trap_AddCommand ("use_electrobinoculars");
	trap_AddCommand ("zoom");
	trap_AddCommand ("use_sentry");
	trap_AddCommand ("bot_order");
	trap_AddCommand ("saberAttackCycle");
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("tell");
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("callvote");
	trap_AddCommand ("vote");
	trap_AddCommand ("callteamvote");
	trap_AddCommand ("teamvote");
	trap_AddCommand ("stats");
	trap_AddCommand ("teamtask");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
}
