/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors
Copyright (C) 2017, SoF2Plus contributors

This file is part of the SoF2Plus source code.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/
// g2_local.h

#ifndef __G2_LOCAL_H
#define __G2_LOCAL_H

#include "../rd-dedicated/tr_local.h"

#define     G2_VERT_SPACE_SIZE              256
#define     G2_MAX_MODELS_IN_LIST           1024
#define     G2_MAX_SURFACES_IN_LIST         256
#define     G2_MAX_BOLTS_IN_LIST            256

typedef     struct      surfaceInfo_s       surfaceInfo_t;
typedef     struct      boneInfo_s          boneInfo_t;
typedef     struct      boltInfo_s          boltInfo_t;

typedef     struct      CGhoul2Model_s      CGhoul2Model_t;
typedef     struct      CGhoul2Array_s      CGhoul2Array_t;

//=============================================
//
// Main Ghoul II structures
//

struct surfaceInfo_s {
    int                 offFlags;                   // what the flags are for this model
    int                 surface;                    // index into array held inside the model definition of pointers to the actual surface data loaded in - used by both client and game
    float               genBarycentricJ;            // point 0 barycentric coors
    float               genBarycentricI;            // point 1 barycentric coors - point 2 is 1 - point0 - point1
    int                 genPolySurfaceIndex;        // used to point back to the original surface and poly if this is a generated surface
    int                 genLod;                     // used to determine original lod of original surface and poly hit location
};

struct boneInfo_s {

};

struct boltInfo_s {
    int                 boneNumber;                 // bone number bolt attaches to
    int                 surfaceNumber;              // surface number bolt attaches to
    int                 surfaceType;                // if we attach to a surface, this tells us if it is an original surface or a generated one - doesn't go across the network
    int                 boltUsed;                   // nor does this
};

struct CGhoul2Model_s {
    surfaceInfo_t       *mSurfaceList[G2_MAX_SURFACES_IN_LIST];
    boltInfo_t          *mBoltList[G2_MAX_BOLTS_IN_LIST];

    int                 numSurfaces;                // Allocated slots in mSurfaceList
    int                 numBolts;                   // Allocated slots in mBoltList

    int                 mModelIndex;
    qhandle_t           mModel;
    char                mFileName[MAX_QPATH];

    qboolean            mValid;
    const model_t       *currentModel;
    int                 currentModelSize;
    const model_t       *animModel;
    int                 currentAnimModelSize;
    const mdxaHeader_t  *aHeader;
};

struct CGhoul2Array_s {
    CGhoul2Model_t      *models[G2_MAX_MODELS_IN_LIST];
    int                 numModels;
};

//=============================================

//
// g2_api.c
//

void                G2API_ListBones             ( CGhoul2Array_t *ghlInfo, int modelIndex );
void                G2API_ListSurfaces          ( CGhoul2Array_t *ghlInfo, int modelIndex );
qboolean            G2API_HaveWeGhoul2Models    ( CGhoul2Array_t *ghlInfo );

qboolean            G2API_GetBoltMatrix         ( CGhoul2Array_t *ghlInfo, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles,
                                                  const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale);
int                 G2API_InitGhoul2Model       ( CGhoul2Array_t **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
                                                  qhandle_t customShader, int modelFlags, int lodBias);

int                 G2API_AddBolt               ( CGhoul2Array_t *ghlInfo, const int modelIndex, const char *boneName );

//
// g2_bolts.c
//

int                 G2_AddBolt                  ( CGhoul2Model_t *ghlInfo, const char *boneName );
qboolean            G2_IsBoltIndexValid         ( CGhoul2Model_t *ghlInfo, const int boltIndex, const char *caller );

//
// g2_misc.c
//

qboolean            G2_SetupModelPointers       ( CGhoul2Model_t *ghlInfo );
CGhoul2Model_t      *G2_IsModelIndexValid       ( CGhoul2Array_t *ghlInfo, const int modelIndex, const char *caller );

void                G2_GenerateWorldMatrix      ( mdxaBone_t *worldMatrix, mdxaBone_t *worldMatrixInv, const vec3_t angles, const vec3_t origin );

void                G2_SetTime                  ( int currentTime );
int                 G2_GetTime                  ( void );

//
// g2_surfaces.c
//

int                 G2_IsSurfaceLegal           ( const model_t *mod, const char *surfaceName, int *flags );

#endif // __G2_LOCAL_H
