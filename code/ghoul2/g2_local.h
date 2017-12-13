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
#define     G2_MAX_BONES_IN_LIST            256

typedef     struct      surfaceInfo_s       surfaceInfo_t;
typedef     struct      boneInfo_s          boneInfo_t;

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

struct boneInfo_s {
    int                 boneNumber;                 // what bone are we overriding?
    mdxaBone_t          matrix;                     // details of bone angle overrides - some are pre-done on the server, some in ghoul2

    int                 startFrame;                 // start frame for animation
    int                 endFrame;                   // end frame for animation NOTE anim actually ends on endFrame + 1

    int                 startTime;                  // time we started this animation
    int                 pauseTime;                  // time we paused this animation - 0 if not paused

    float               animSpeed;                  // speed at which this anim runs. 1.0f means full speed of animation incoming - i.e. if anim is 20hrtz, we run at 20hrts
    float               blendFrame;                 // frame PLUS LERP value to blend from
    int                 flags;                      // flags for override
    int                 blendLerpFrame;             // frame to lerp the blend frame with
    int                 blendTime;                  // duration time for blending - used to calc amount each frame of new anim is blended with last frame of the last anim
    int                 blendStart;                 // time when blending starts - not necessarily the same as startTime since we might start half way through an anim
    int                 boneBlendTime;              // time for duration of bone angle blend with normal animation
    int                 boneBlendStart;             // time bone angle blend with normal animation began

    mdxaBone_t          newMatrix;                  // this is the lerped matrix that Ghoul2 uses - does not go across the network
};

struct CGhoul2Model_s {
    surfaceInfo_t       *mSurfaceList[G2_MAX_SURFACES_IN_LIST];
    boneInfo_t          *mBoneList[G2_MAX_BONES_IN_LIST];

    int                 numSurfaces;                // Allocated slots in mSurfaceList
    int                 numBones;                   // Allocated slots in mBoneList

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

void                    G2API_ListBones             ( CGhoul2Array_t *ghlInfo, int modelIndex );
void                    G2API_ListSurfaces          ( CGhoul2Array_t *ghlInfo, int modelIndex );
qboolean                G2API_HaveWeGhoul2Models    ( CGhoul2Array_t *ghlInfo );

int                     G2API_InitGhoul2Model       ( CGhoul2Array_t **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
                                                      qhandle_t customShader, int modelFlags, int lodBias );

qboolean                G2API_SetBoneAngles         ( CGhoul2Array_t *ghlInfo, const int modelIndex, const char *boneName, const vec3_t angles, const int flags,
                                                      const Eorientations up, const Eorientations left, const Eorientations forward,
                                                      int blendTime, int currentTime );

//
// g2_bones.c
//

extern mdxaBone_t       identityMatrix;

int                     G2_FindBoneInModel          ( const model_t *modAnim, const char *boneName );
int                     G2_IsBoneInList             ( const model_t *modAnim, const boneInfo_t **boneList, const int numBones, int boneNumber, const char *boneName );
int                     G2_AddBone                  ( const model_t *modAnim, boneInfo_t **boneList, int *numBones, const char *boneName );

void                    G2_BoneGenerateMatrix       ( const model_t *modAnim, boneInfo_t **boneList, int boneIndex, const float *angles, int flags,
                                                      const Eorientations up, const Eorientations left, const Eorientations forward );
//
// g2_misc.c
//

qboolean                G2_SetupModelPointers       ( CGhoul2Model_t *ghlInfo );
CGhoul2Model_t          *G2_IsModelIndexValid       ( CGhoul2Array_t *ghlInfo, const int modelIndex, const char *caller );

void                    G2_CreateMatrix             ( mdxaBone_t *matrix, const float *angle );
void                    G2_GenerateWorldMatrix      ( mdxaBone_t *worldMatrix, mdxaBone_t *worldMatrixInv, const vec3_t angles, const vec3_t origin );

void                    G2_SetTime                  ( int currentTime );
int                     G2_GetTime                  ( void );

//
// g2_surfaces.c
//

int                     G2_IsSurfaceLegal           ( const model_t *mod, const char *surfaceName, int *flags );

#endif // __G2_LOCAL_H
