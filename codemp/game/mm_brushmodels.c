//#include "../qcommon/mm_brushmodels.h"
#include "q_shared.h"
/*
// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3
*/
/*
=================
PlaneTypeForNormal
=================
*/ //InsertModel / model.c
int PlaneTypeForNormal (vec3_t normal) {
	if (normal[0] == 1.0 /*|| normal[0] == -1.0*/)
		return PLANE_X;
	if (normal[1] == 1.0 /*|| normal[1] == -1.0*/)
		return PLANE_Y;
	if (normal[2] == 1.0 /*|| normal[2] == -1.0*/)
		return PLANE_Z;
	
	return PLANE_NON_AXIAL;
}

int SignbitsForNormal (vec3_t normal) {
	int signbits;

	if ( normal[0] < 0 )
		signbits = 1;
	if ( normal[1] < 0)
		signbits |= 2;
	if ( normal[2] < 0)
		signbits |= 4;

	return signbits;
}

void MM_Expand_Bounds( vec3_t p, vec3_t mins, vec3_t maxs )
{
	int i;
	for (i=0; i<3; i++)
	{
		float value = p[i];
		if (value < mins[i]) mins[i] = value;
		if (value > maxs[i]) maxs[i] = value;
	}
}
#define	MAX_MAP_PLANES			0x100000
cplane_t *CreateNewFloatPlane (vec3_t normal, vec_t dist, int *numplanes, cplane_t *planes)
{
	cplane_t *p, temp;

	if (VectorLength(normal) < 0.5)
	{
		Com_Printf( "FloatPlane: bad normal\n");
		return -1;
	}

	// create a new plane
	if ((*numplanes)+2 > MAX_MAP_PLANES)
		Com_Error (ERR_FATAL,"MAX_MAP_PLANES");

	p = &planes[(*numplanes)];
	VectorCopy (normal, p->normal);
	p->dist = dist;
	p->type = (p+1)->type = PlaneTypeForNormal (p->normal);
	p->signbits = SignbitsForNormal(p->normal);
	(p+1)->signbits = SignbitsForNormal((p+1)->normal);

	VectorSubtract (vec3_origin, normal, (p+1)->normal);
	(p+1)->dist = -dist;

	(*numplanes) += 2;

	// allways put axial planes facing positive first
	if (p->type < 3)
	{
		if (p->normal[0] < 0 || p->normal[1] < 0 || p->normal[2] < 0)
		{
			// flip order
			temp = *p;
			*p = *(p+1);
			*(p+1) = temp;

		//	AddPlaneToHash (p);
		//	AddPlaneToHash (p+1);
			return &planes[(*numplanes) - 1];
		}
	}

//	AddPlaneToHash (p);
//	AddPlaneToHash (p+1);
	return &planes[(*numplanes) - 2];
}