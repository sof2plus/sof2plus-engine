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
// tr_g2_local.h

#ifndef __TR_G2_LOCAL_H
#define __TR_G2_LOCAL_H

#include "tr_local.h"

//=============================================
//
// Global Ghoul II definitions.
//

#define BONE_ANGLES_PREMULT         0x0001
#define BONE_ANGLES_POSTMULT        0x0002
#define BONE_ANGLES_REPLACE         0x0004
#define BONE_ANGLES_REPLACE_TO_ANIM 0x0400

#define BONE_ANGLES_TOTAL           (BONE_ANGLES_PREMULT | BONE_ANGLES_POSTMULT | BONE_ANGLES_REPLACE | BONE_ANGLES_REPLACE_TO_ANIM)
#define BONE_ANIM_OVERRIDE          0x0008
#define BONE_ANIM_OVERRIDE_LOOP     0x0010
#define BONE_ANIM_OVERRIDE_DEFAULT  (0x0020 + BONE_ANIM_OVERRIDE)
#define BONE_ANIM_OVERRIDE_FREEZE   (0x0040 + BONE_ANIM_OVERRIDE)
#define BONE_ANIM_TOTAL             (BONE_ANIM_OVERRIDE | BONE_ANIM_OVERRIDE_LOOP | BONE_ANIM_OVERRIDE_DEFAULT | BONE_ANIM_OVERRIDE_FREEZE)


#define ENTITY_WIDTH                12
#define MODEL_WIDTH                 10

#define MODEL_AND                   ((1<<MODEL_WIDTH)-1)
#define ENTITY_AND                  ((1<<ENTITY_WIDTH)-1)

#define BOLT_SHIFT                  0
#define ENTITY_SHIFT                (MODEL_SHIFT + MODEL_WIDTH)

//
// Collision detection definitions.
//

#define G2_BACKFACE                 0
#define G2_FRONTFACE                1

// Calling defines for the trace function.
typedef enum {
    G2_NOCOLLIDE,
    G2_COLLIDE,
    G2_RETURNONHIT
} EG2_Collision;

//=============================================

#define     G2_MAX_BONES_IN_LIST            256

typedef     struct      boneInfo_s          boneInfo_t;
typedef     struct      CBoneCalc_s         CBoneCalc_t;
typedef     struct      CTransformBone_s    CTransformBone_t;
typedef     struct      CBoneCache_s        CBoneCache_t;
typedef     struct      CTraceSurface_s     CTraceSurface_t;

typedef     struct      CGhoul2Model_s      CGhoul2Model_t;

//=============================================
//
// Main Ghoul II structures
//

struct boneInfo_s {
    int                 boneNumber;                 // what bone are we overriding?
    mdxaBone_t          matrix;                     // details of bone angle overrides - some are pre-done on the server, some in ghoul2

    int                 startFrame;                 // start frame for animation
    int                 endFrame;                   // end frame for animation NOTE anim actually ends on endFrame + 1

    int                 startTime;                  // time we started this animation
    int                 pauseTime;                  // time we paused this animation - 0 if not paused

    float               animSpeed;                  // speed at which this anim runs. 1.0f means full speed of animation incoming - i.e. if anim is 20hrtz, we run at 20hrts
    int                 flags;                      // flags for override

    mdxaBone_t          newMatrix;                  // this is the lerped matrix that Ghoul2 uses - does not go across the network
};

struct CBoneCalc_s {
    int                 newFrame;
    int                 currentFrame;
    float               backlerp;
};

struct CTransformBone_s {
    int                 touch;                      // for minimal recalculation
    mdxaBone_t          boneMatrix;                 // final matrix
    int                 parent;                     // only set once
};

struct CBoneCache_s {
    void                *mBones;
    void                *mFinalBones;
    int                 numBones;

    CGhoul2Model_t      *parent;
    mdxaBone_t          rootMatrix;
    int                 incomingTime;

    int                 mCurrentTouch;
};

struct CTraceSurface_s {
    int                 surfaceNum;
    const model_t       *currentModel;
    int                 lod;
    vec3_t              rayStart;
    vec3_t              rayEnd;
    mdxaBone_t          *worldMatrix;
    CollisionRecord_t   *collRecMap;
    int                 entNum;
    skin_t              *skin;
    void                **transformedVertsArray;
    int                 traceFlags;

    qboolean            stopRec;
};

struct CGhoul2Model_s {
    boneInfo_t          *mBoneList[G2_MAX_BONES_IN_LIST];
    int                 numBones;

    int                 mModelIndex;
    qhandle_t           mCustomSkin;
    int                 mLodBias;

    qhandle_t           mModel;
    char                mFileName[MAX_QPATH];

    void                **mTransformedVertsArray;
    int                 numTransformedVerts;

    qboolean            mValid;
    const model_t       *currentModel;
    int                 currentModelSize;
    const model_t       *animModel;
    int                 currentAnimModelSize;
    const mdxaHeader_t  *aHeader;

    CBoneCache_t        *mBoneCache;
};

//=============================================

//
// tr_g2_api.c
//

void                    G2API_ListBones             ( CGhoul2Model_t *model );
void                    G2API_ListSurfaces          ( CGhoul2Model_t *model );

qboolean                G2API_InitGhoul2Model       ( CGhoul2Model_t **modelPtr, const char *fileName, qhandle_t customSkin, int lodBias );
qboolean                G2API_RemoveGhoul2Model     ( CGhoul2Model_t **modelPtr );

qboolean                G2API_SetBoneAngles         ( CGhoul2Model_t *model, const char *boneName, const vec3_t angles, const int flags,
                                                      const Eorientations up, const Eorientations left, const Eorientations forward );
qboolean                G2API_SetBoneAnim           ( CGhoul2Model_t *model, const char *boneName, const int AstartFrame, const int AendFrame,
                                                      const int flags, const float animSpeed, const float AsetFrame );

qboolean                G2API_GetAnimFileName       ( CGhoul2Model_t *model, char *dest, int destSize );

void                    G2API_CollisionDetect       ( CollisionRecord_t *collRecMap, CGhoul2Model_t *model, const vec3_t angles, const vec3_t position,
                                                      int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod );

qhandle_t               G2API_RegisterSkin          ( const char *skinName, int numPairs, const char *skinPairs );
qboolean                G2API_SetSkin               ( CGhoul2Model_t *model, qhandle_t customSkin );

//
// tr_g2_bones.c
//

extern mdxaBone_t       identityMatrix;

int                     G2_FindBoneInModel          ( const model_t *modAnim, const char *boneName );
int                     G2_IsBoneInList             ( const model_t *modAnim, boneInfo_t **boneList, const int numBones, int boneNumber, const char *boneName );
int                     G2_AddBone                  ( const model_t *modAnim, boneInfo_t **boneList, int *numBones, const char *boneName );

qboolean                G2_InitBoneCache            ( CGhoul2Model_t *model );
mdxaBone_t              *G2_BoneEval                ( CBoneCache_t *mBoneCache, int boneIndex );

void                    G2_BoneGenerateMatrix       ( const model_t *modAnim, boneInfo_t **boneList, int boneIndex, const float *angles, int flags,
                                                      const Eorientations up, const Eorientations left, const Eorientations forward );

void                    G2_TransformSkeleton        ( CGhoul2Model_t *model, const int frameNum );

//
// tr_g2_collision.c
//

void                    G2_TransformModel           ( CGhoul2Model_t *model, vec3_t scale, int useLod );

void                    G2_TraceModel               ( CGhoul2Model_t *model, vec3_t rayStart, vec3_t rayEnd, mdxaBone_t *worldMatrix,
                                                      CollisionRecord_t *collRecMap, int entNum, int traceFlags, int useLod );

//
// tr_g2_misc.c
//

qboolean                G2_SetupModelPointers       ( CGhoul2Model_t *model );
qboolean                G2_IsModelValid             ( CGhoul2Model_t *model, const char *caller );

void                    G2_CreateMatrix             ( mdxaBone_t *matrix, const float *angle );
void                    G2_GenerateWorldMatrix      ( mdxaBone_t *worldMatrix, mdxaBone_t *worldMatrixInv, const vec3_t angles, const vec3_t origin );
void                    G2_Multiply_3x4Matrix       ( mdxaBone_t *out, mdxaBone_t *in2, mdxaBone_t *in );
void                    G2_TransformPoint           ( const vec3_t in, vec3_t out, mdxaBone_t *mat );
void                    G2_TransformTranslatePoint  ( const vec3_t in, vec3_t out, mdxaBone_t *mat );

//
// tr_g2_surfaces.c
//

int                     G2_IsSurfaceLegal           ( const model_t *mod, const char *surfaceName, int *flags );
mdxmSurface_t           *G2_FindSurfaceFromModel    ( const model_t *model, int surfaceIndex, int lod );

#endif // __TR_G2_LOCAL_H
