#ifndef __MM_BRUSHMODELS_H__
#define __MM_BRUSHMODELS_H__

#include "../qcommon/qfiles.h"

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
/*#define	BOX_BRUSHES2		5//1
#define	BOX_SIDES2		48//6
#define	BOX_LEAFS2		6//2
#define	BOX_PLANES2		60//12*/

#define	BOX_BRUSHES		1
#define	BOX_SIDES		6
#define	BOX_LEAFS		2
#define	BOX_PLANES		12

#define MD3_SCALE		  (1.0f / 64.0f)

#define MAX_ADD_MEMORY 512
#define BRUSHSIDE_ADD_MEMORY 400*MAX_ADD_MEMORY	// 6
#define PLANE_ADD_MEMORY 400*MAX_ADD_MEMORY		// 6 / 12
#define LEAFB_ADD_MEMORY MAX_ADD_MEMORY			// 1
#define CMODELS_ADD_MEMORY MAX_ADD_MEMORY		// 1
#define BRUSH_ADD_MEMORY 10*MAX_ADD_MEMORY		// 1


typedef struct {
	int		cluster;
	int		area;

	int		firstLeafBrush;
	int		numLeafBrushes;

	int		firstLeafSurface;
	int		numLeafSurfaces;
}cLeaf_t;

typedef struct {
	cplane_t	*plane;
	int			shadernum;	
}cbrushside_t;

typedef struct {
	cplane_t	*plane;
	int			children[2];
}cNode_t;

typedef struct cmodel_s {
	vec3_t		mins, maxs;
	cLeaf_t		leaf;			// submodels don't reference the main tree
	int			counter;
} cmodel_t;

typedef struct {
	int			shaderNum;		// the shader that determined the contents
	int			contents;
	vec3_t		bounds[2];
	cbrushside_t	*sides;
	int			numSides;
}cbrush_t;

typedef struct {
	int			floodnum;
	int			floodvalid;
}cArea_t;

typedef struct {
	int			checkcount;				// to avoid repeated testings
	int			surfaceFlags;
	int			contents;
	struct patchCollide_s	*pc;
} cPatch_t;

typedef struct {
	float	plane[4];
	int		signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
} patchPlane_t;

typedef struct {
	int			surfacePlane;
	int			numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int			borderPlanes[4+6+16];
	int			borderInward[4+6+16];
	qboolean	borderNoAdjust[4+6+16];
} facet_t;

typedef struct patchCollide_s {
	vec3_t	bounds[2];
	int		numPlanes;			// surface planes plus edge planes
	patchPlane_t	*planes;
	int		numFacets;
	facet_t	*facets;
} patchCollide_t;

typedef struct {
	char		name[MAX_QPATH];

	int			numShaders;
	dshader_t	*shaders;

	int			numBrushSides;
	cbrushside_t *brushsides;

	int			numPlanes;
	cplane_t	*planes;

	int			numNodes;
	cNode_t		*nodes;

	int			numLeafs;
	cLeaf_t		*leafs;

	int			numLeafBrushes;
	int			*leafbrushes;

	int			numLeafSurfaces;
	int			*leafsurfaces;

	int			numSubModels;
	cmodel_t	*cmodels;

	int			numBrushes;
	cbrush_t	*brushes;

	int			numClusters;
	int			clusterBytes;
	byte		*visibility;
	qboolean	vised;			// if false, visibility is just a single cluster of ffs

	int			numEntityChars;
	char		*entityString;

	int			numAreas;
	cArea_t		*areas;
	int			*areaPortals;	// [ numAreas*numAreas ] reference counts

	int			numSurfaces;
	cPatch_t	**surfaces;			// non-patches will be NULL

	int			floodvalid;
	int			checkcount;					// incremented on each trace
	int			UnknownValue;
} clipMap_t;

#ifdef MM_BRUSH
extern clipMap_t *cm;
#endif

int	PlaneTypeForNormal (vec3_t normal);
int SignbitsForNormal (vec3_t normal);
void MM_Expand_Bounds ( vec3_t p, vec3_t mins, vec3_t maxs );
cplane_t *CreateNewFloatPlane (vec3_t normal, vec_t dist, int *numplanes, cplane_t *planes);


// Remap shader

typedef unsigned int GLuint;

#define MAX_SHADER_STAGES 8

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH, 
	GF_INVERSE_SAWTOOTH, 

	GF_NOISE

} genFunc_t;


typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7
} deform_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST
} alphaGen_t;


typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,	// tr.identityLight
	CGEN_IDENTITY,			// always (1,1,1,1)
	CGEN_ENTITY,			// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,	// grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,		// tess.vertexColors
	CGEN_VERTEX,			// tess.vertexColors * tr.identityLight
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,			// programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_FOG,				// standard fog
	CGEN_CONST				// fixed color
} colorGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;


typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,			// clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
	TCGEN_FOG,
	TCGEN_VECTOR			// S and T from world coordinates
} texCoordGen_t;

typedef struct {
	genFunc_t	func;

	float base;
	float amplitude;
	float phase;
	float frequency;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE
} texMod_t;

#define	MAX_SHADER_DEFORMS	3
typedef struct {
	deform_t	deformation;			// vertex coordinate modification type

	vec3_t		moveVector;
	waveForm_t	deformationWave;
	float		deformationSpread;

	float		bulgeWidth;
	float		bulgeHeight;
	float		bulgeSpeed;
} deformStage_t;

typedef struct {
	texMod_t		type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t		wave;

	// used for TMOD_TRANSFORM
	float			matrix[2][2];		// s' = s * m[0][0] + t * m[1][0] + trans[0]
	float			translate[2];		// t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float			scale[2];			// s *= scale[0]
	                                    // t *= scale[1]

	// used for TMOD_SCROLL
	float			scroll[2];			// s' = s + scroll[0] * time
										// t' = t + scroll[1] * time

	// + = clockwise
	// - = counterclockwise
	float			rotateSpeed;

} texModInfo_t;

typedef struct image_s {
	char		imgName[MAX_QPATH];		// game path, including extension
	int			width, height;				// source image
	int			uploadWidth, uploadHeight;	// after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE
	GLuint		texnum;					// gl texture binding

	int			frameUsed;			// for texture usage in frame statistics

	int			internalFormat;
	int			TMU;				// only needed for voodoo2

	qboolean	mipmap;
	qboolean	allowPicmip;
	int			wrapClampMode;		// GL_CLAMP or GL_REPEAT

	struct image_s*	next;
} image_t;

#define	MAX_IMAGE_ANIMATIONS	8
#define NUM_TEXTURE_BUNDLES 2

typedef struct {
	image_t			*image[MAX_IMAGE_ANIMATIONS];
	int				numImageAnimations;
	float			imageAnimationSpeed;

	texCoordGen_t	tcGen;
	vec3_t			tcGenVectors[2];

	int				numTexMods;
	texModInfo_t	*texMods;

	int				videoMapHandle;
	qboolean		isLightmap;
	qboolean		vertexLightmap;
	qboolean		isVideoMap;
} textureBundle_t;


typedef enum {
	FP_NONE,		// surface is translucent and will just be adjusted properly
	FP_EQUAL,		// surface is opaque but possibly alpha tested
	FP_LE			// surface is trnaslucent, but still needs a fog pass (fog surface)
} fogPass_t;


typedef struct {
	float		cloudHeight;
	image_t		*outerbox[6], *innerbox[6];
} skyParms_t;

typedef struct {
	vec3_t	color;
	float	depthForOpaque;
} fogParms_t;

typedef enum {
	CT_FRONT_SIDED,
	CT_BACK_SIDED,
	CT_TWO_SIDED
} cullType_t;

typedef struct {
	qboolean		active;
	
	textureBundle_t	bundle[NUM_TEXTURE_BUNDLES];

	waveForm_t		rgbWave;
	colorGen_t		rgbGen;

	waveForm_t		alphaWave;
	alphaGen_t		alphaGen;

	byte			constantColor[4];			// for CGEN_CONST and AGEN_CONST

	unsigned		stateBits;					// GLS_xxxx mask

	acff_t			adjustColorsForFog;

	qboolean		isDetail;
} shaderStage_t;

#define MAX_STATES_PER_SHADER 32
#define MAX_STATE_NAME 32


typedef struct shader_s {
	char		name[MAX_QPATH];		// game path, including extension
	int			lightmapIndex;			// for a shader to match, both name and lightmapIndex must match

	int			index;					// this shader == tr.shaders[index]
	int			sortedIndex;			// this shader == tr.sortedShaders[sortedIndex]

	float		sort;					// lower numbered shaders draw before higher numbered

	qboolean	defaultShader;			// we want to return index 0 if the shader failed to
										// load for some reason, but R_FindShader should
										// still keep a name allocated for it, so if
										// something calls RE_RegisterShader again with
										// the same name, we don't try looking for it again

	qboolean	explicitlyDefined;		// found in a .shader file

	int			surfaceFlags;			// if explicitlyDefined, this will have SURF_* flags
	int			contentFlags;

	qboolean	entityMergable;			// merge across entites optimizable (smoke, blood)

	qboolean	isSky;
	skyParms_t	sky;
	fogParms_t	fogParms;

	float		portalRange;			// distance to fog out at

	int			multitextureEnv;		// 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	cullType_t	cullType;				// CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean	polygonOffset;			// set for decals and other items that must be offset 
	qboolean	noMipMaps;				// for console fonts, 2D elements, etc.
	qboolean	noPicMip;				// for images that must always be full resolution

	fogPass_t	fogPass;				// draw a blended pass, possibly with depth test equals

	qboolean	needsNormal;			// not all shaders will need all data to be gathered
	qboolean	needsST1;
	qboolean	needsST2;
	qboolean	needsColor;

	int			numDeforms;
	deformStage_t	deforms[MAX_SHADER_DEFORMS];

	int			numUnfoggedPasses;
	shaderStage_t	*stages[MAX_SHADER_STAGES];		

	void		(*optimalStageIteratorFunc)( void );

  float clampTime;                                  // time this shader is clamped to
  float timeOffset;                                 // current time offset for this shader

  int numStates;                                    // if non-zero this is a state shader
  struct shader_s *currentShader;                   // current state if this is a state shader
  struct shader_s *parentShader;                    // current state if this is a state shader
  int currentState;                                 // current state index for cycle purposes
  long expireTime;                                  // time in milliseconds this expires

  struct shader_s *remappedShader;                  // current shader this one is remapped too

  int shaderStates[MAX_STATES_PER_SHADER];          // index to valid shader states

	struct	shader_s	*next;
} shader_t;

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_MESH,
	MOD_MD4
} modtype_t;

typedef enum {
	SF_BAD,
	SF_SKIP,				// ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_POLY,
	SF_MD3,
	SF_MD4,
	SF_FLARE,
	SF_ENTITY,				// beams, rails, lightning, etc that can be determined by entity
	SF_DISPLAY_LIST,

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct msurface_s {
	int					viewCount;		// if == tr.viewCount, already added
	struct shader_s		*shader;
	int					fogIndex;

	surfaceType_t		*data;			// any of srf*_t
} msurface_t;

typedef struct {
	vec3_t		bounds[2];		// for culling
	msurface_t	*firstSurface;
	int			numSurfaces;
} bmodel_t;

typedef struct model_s {
	char		name[MAX_QPATH];
	modtype_t	type;
	int			index;				// model = tr.models[model->index]

	int			dataSize;			// just for listing purposes
	bmodel_t	*bmodel;			// only if type == MOD_BRUSH
	md3Header_t	*md3[MD3_MAX_LODS];	// only if type == MOD_MESH
	int	*md4;				// only if type == MOD_MD4

	int			 numLods;
	char		 padding[8];
} model_t;

#endif