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
// g2_bones.c - Bone and skeleton handling Ghoul II routines.

#include "g2_local.h"

// The identity matrix is used across Ghoul II files.
mdxaBone_t identityMatrix =
{
    {
        { 0.0f, -1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f }
    }
};

/*
=============================================
-------------------
Bone list functions
-------------------

Please note that the modAnim model_t pointer
on many of these routines must point at the
Ghoul II animation file (.gla), not the
Ghoul II mesh file (.glm).
=============================================
*/

/*
==================
G2_FindFreeBoneSlot

Returns the first free slot to use in the bone list,
or -1 if there are no more free slots available.

NOTE: The returned slot may be unallocated.
==================
*/

static int G2_FindFreeBoneSlot(boneInfo_t **boneList, int numBones)
{
    int i;

    // First off, check what index we can use.
    if(numBones){
        // Iterate through the list, check which slot is unallocated.
        for(i = 0; i < G2_MAX_BONES_IN_LIST; i++){
            if(boneList[i] == NULL || boneList[i]->boneNumber == -1){
                return i;
            }
        }

        // No free slot found.
        Com_Printf(S_COLOR_RED "G2_FindFreeBoneSlot: Bone list full! It cannot hold more than %d entries.\n", G2_MAX_BONES_IN_LIST);
        return -1;
    }

    // No slots allocated yet. Return the first slot.
    return 0;
}

/*
==============
G2_FindBoneInModel

Finds the bone by name from the given
Ghoul II animation file model.

Returns the bone number upon success,
or -1 upon failure.
==============
*/

int G2_FindBoneInModel(const model_t *modAnim, const char *boneName)
{
    mdxaHeader_t        *mdxaHeader;
    mdxaSkel_t          *skel;
    mdxaSkelOffsets_t   *offsets;
    int                 i;

    // Get the Ghoul II animation file header.
    mdxaHeader = (mdxaHeader_t *)modAnim->modelData;

    // Determine the offsets in the supplied Ghoul II animation file.
    offsets = (mdxaSkelOffsets_t *)((byte *)modAnim->modelData + sizeof(mdxaHeader_t));

    // Walk the entire list of bones in the Ghoul II animation file.
    for(i = 0; i < mdxaHeader->numBones; i++){
        // Point to the next bone.
        skel = (mdxaSkel_t *)((byte *)modAnim->modelData + sizeof(mdxaHeader_t) + offsets->offsets[i]);

        // If the name is the same, we found it.
        if(!Q_stricmp(skel->name, boneName)){
            // Return the bone number.
            return i;
        }
    }

    // We didn't find it.
    return -1;
}

/*
==============
G2_IsBoneInList

Checks if the bone from the supplied model
is already present in our bone list.

If the bone number from the file is already known,
the boneNumber parameter must be > -1.
Otherwise the bone number will be determined
from the supplied bone name.

If the bone is found, this function returns
the index of this bone in the bone list.
If not, -1 is returned.
==============
*/

int G2_IsBoneInList(const model_t *modAnim, boneInfo_t **boneList, const int numBones, int boneNumber, const char *boneName)
{

    int i, numValid;

    // Are there bones present in our list?
    if(!numBones){
        // No, so no need to continue.
        return -1;
    }

    // Do we already have a bone index to use?
    if(boneNumber == -1){
        // No, get the bone index from the Ghoul II animation file.
        boneNumber = G2_FindBoneInModel(modAnim, boneName);
    }

    // Now look through the entire list.
    numValid = 0;
    for(i = 0; i < G2_MAX_BONES_IN_LIST; i++){
        // Check if this slot is allocated.
        if(boneList[i] != NULL){
            // If the bone number is the same, we found it.
            if(boneList[i]->boneNumber == boneNumber){
                // Return the bone index.
                return i;
            }

            numValid++;
            if(numValid == numBones){
                // Don't continue if we've iterated
                // through all allocated slots.
                break;
            }
        }
    }

    // We didn't find it.
    return -1;
}

/*
==================
G2_AddBone

Finds and adds a bone to the bone list.
Returns the new index in the bone list,
or -1 upon failure.
==================
*/

int G2_AddBone(const model_t *modAnim, boneInfo_t **boneList, int *numBones, const char *boneName)
{
    int boneNumber, boneIndex;

    // See if we can find the bone in the Ghoul II animation file.
    boneNumber = G2_FindBoneInModel(modAnim, boneName);
    if(boneNumber == -1){
        Com_DPrintf(S_COLOR_RED "G2_AddBone: Bone \"%s\" not found on skeleton.\n", boneName);
        return -1;
    }

    // Bone found, is it already present in our list?
    boneIndex = G2_IsBoneInList(modAnim, boneList, *numBones, boneNumber, boneName);
    if(boneIndex != -1){
        // We found it, return the index in our bone list.
        return boneIndex;
    }

    // Not found, find a slot we can use.
    boneIndex = G2_FindFreeBoneSlot(boneList, *numBones);
    if(boneIndex == -1){
        // No more slots available.
        return -1;
    }

    // Do we need to allocate this slot?
    if(boneList[boneIndex] == NULL){
        // Allocate this slot.
        boneList[boneIndex] = Hunk_Alloc(sizeof(boneInfo_t), h_low);
        (*numBones)++;
    }

    // Set the required info.
    boneList[boneIndex]->boneNumber = boneNumber;

    // Return the new bone index.
    return boneIndex;
}

/*
=============================================
--------------------
Bone cache functions
--------------------
=============================================
*/

/*
==============
G2_GetCalcBone

Returns allocated CBoneCalc_t member
from the mBones array.
==============
*/

static ID_INLINE CBoneCalc_t *G2_GetCalcBone(void *mBones, int index)
{
    return (CBoneCalc_t *)(mBones + index * sizeof(CBoneCalc_t));
}

/*
==============
G2_GetTransformBone

Returns allocated CTransformBone_t member
from the mFinalBones array.
==============
*/

static ID_INLINE CTransformBone_t *G2_GetTransformBone(void *mFinalBones, int index)
{
    return (CTransformBone_t *)(mFinalBones + index * sizeof(CTransformBone_t));
}

/*
==============
G2_InitBoneCache

Initializes the bone cache if not done already.
Returns qtrue upon success.
==============
*/

qboolean G2_InitBoneCache(CGhoul2Model_t *model)
{
    CBoneCache_t        *boneCache;
    int                 i;
    mdxaSkelOffsets_t   *offsets;
    mdxaSkel_t          *skel;
    CTransformBone_t    *transformBone;

    // Is the supplied model valid?
    if(model == NULL || !model->mValid){
        Com_Printf(S_COLOR_RED "G2_InitBoneCache: Supplied model not valid. Failed to create the bone cache.\n");
        return qfalse;
    }

    // The bone cache doesn't exist yet, allocate memory for it now.
    boneCache = model->mBoneCache = Hunk_Alloc(sizeof(CBoneCache_t), h_high);

    // Set the number of bones.
    boneCache->numBones = model->aHeader->numBones;

    // Allocate memory for our internal bone lists.
    boneCache->mBones = Hunk_Alloc(sizeof(CBoneCalc_t) * boneCache->numBones, h_high);
    boneCache->mFinalBones = Hunk_Alloc(sizeof(CTransformBone_t) * boneCache->numBones, h_high);

    // Determine the skeleton offsets.
    offsets = (mdxaSkelOffsets_t *)((byte *)model->aHeader + sizeof(mdxaHeader_t));

    // Iterate through the bones.
    for(i = 0; i < boneCache->numBones; i++){
        skel            = (mdxaSkel_t *)((byte *)model->aHeader + sizeof(mdxaHeader_t) + offsets->offsets[i]);
        transformBone   = G2_GetTransformBone(boneCache->mFinalBones, i);

        // Save what parent belongs to this bone.
        transformBone->parent = skel->parent;
    }

    // Ensure this bone knows the parent Ghoul II model.
    boneCache->parent = model;

    // Set the identity matrix as our initial root matrix.
    boneCache->rootMatrix = identityMatrix;

    // Set initial touched values so we re-render upon the next evaluation.
    boneCache->mCurrentTouch = 0;

    return qtrue;
}

/*
==============
G2_IsBoneIndexValid

Checks whether the specified bone index
is valid in the bone cache.

Returns qtrue if valid.
==============
*/

static qboolean G2_IsBoneIndexValid(CBoneCache_t *mBoneCache, const int boneIndex, const char *caller)
{
    if(!mBoneCache){
        Com_DPrintf(S_COLOR_RED "%s/G2_IsBoneIndexValid: boneCache pointer is NULL.\n", caller);
        return qfalse;
    }

    if(!mBoneCache->numBones){
        Com_DPrintf(S_COLOR_RED "%s/G2_IsBoneIndexValid: No bones present in bone cache.\n", caller);
        return qfalse;
    }

    if(boneIndex < 0 || boneIndex >= mBoneCache->numBones){
        Com_DPrintf(S_COLOR_RED "%s/G2_IsBoneIndexValid: Bone slot %d is out of bounds (bone cache can hold %d bones(s)).\n",
            caller, boneIndex, mBoneCache->numBones);
        return qfalse;
    }

    // All valid.
    return qtrue;
}

/*
==============
G2_GetBonePoolIndex

Returns the bone pool index.
==============
*/

static int G2_GetBonePoolIndex(const mdxaHeader_t *pMDXAHeader, int iFrame, int iBone)
{
    const int iOffsetToIndex = (iFrame * pMDXAHeader->numBones * 3) + (iBone * 3);
    mdxaIndex_t *pIndex;

    pIndex = (mdxaIndex_t *) ((byte*) pMDXAHeader + pMDXAHeader->ofsFrames + iOffsetToIndex);

    #ifndef Q3_LITTLE_ENDIAN
    // FIXME BOE
    #error "G2_GetBonePoolIndex: Big endian fix not yet implemented."
    #endif
    return pIndex->iIndex & 0x00FFFFFF;
}

/*
==============
G2_UncompressBone

Uncompresses bone for the given frame.
==============
*/

static void G2_UncompressBone(float mat[3][4], int iBoneIndex, const mdxaHeader_t *pMDXAHeader, int iFrame)
{
    float                   w,x,y,z,f;
    float                   fTx;
    float                   fTy;
    float                   fTz;
    float                   fTwx;
    float                   fTwy;
    float                   fTwz;
    float                   fTxx;
    float                   fTxy;
    float                   fTxz;
    float                   fTyy;
    float                   fTyz;
    float                   fTzz;
    unsigned short          *pwIn;
    unsigned char           *comp;
    mdxaCompQuatBone_t      *pCompBonePool;

    pCompBonePool   = (mdxaCompQuatBone_t *) ((byte *)pMDXAHeader + pMDXAHeader->ofsCompBonePool);
    comp            = pCompBonePool[G2_GetBonePoolIndex(pMDXAHeader, iFrame, iBoneIndex)].Comp;
    pwIn            = (unsigned short *)comp;

    w           =   *pwIn++;
    w           /=  16383.0f;
    w           -=  2.0f;
    x           =   *pwIn++;
    x           /=  16383.0f;
    x           -=  2.0f;
    y           =   *pwIn++;
    y           /=  16383.0f;
    y           -=  2.0f;
    z           =   *pwIn++;
    z           /=  16383.0f;
    z           -=  2.0f;

    fTx         =   2.0f * x;
    fTy         =   2.0f * y;
    fTz         =   2.0f * z;
    fTwx        =   fTx * w;
    fTwy        =   fTy * w;
    fTwz        =   fTz * w;
    fTxx        =   fTx * x;
    fTxy        =   fTy * x;
    fTxz        =   fTz * x;
    fTyy        =   fTy * y;
    fTyz        =   fTz * y;
    fTzz        =   fTz * z;

    //
    // Rot.
    //
    mat[0][0]   =   1.0f - (fTyy + fTzz);
    mat[0][1]   =   fTxy - fTwz;
    mat[0][2]   =   fTxz + fTwy;
    mat[1][0]   =   fTxy + fTwz;
    mat[1][1]   =   1.0f - (fTxx + fTzz);
    mat[1][2]   =   fTyz - fTwx;
    mat[2][0]   =   fTxz - fTwy;
    mat[2][1]   =   fTyz + fTwx;
    mat[2][2]   =   1.0f - (fTxx + fTyy);

    //
    // Xlat.
    //
    f           =   *pwIn++;
    f           /=  64;
    f           -=  512;
    mat[0][3]   =   f;

    f           =   *pwIn++;
    f           /=  64;
    f           -=  512;
    mat[1][3]   =   f;

    f           =   *pwIn++;
    f           /=  64;
    f           -=  512;
    mat[2][3]   =   f;
}

/*
==============
G2_TimingModel

Animate a model.
==============
*/

static void G2_TimingModel(boneInfo_t *bone, int currentTime, int numFramesInFile, int *currentFrame, int *newFrame, float *lerp)
{
    float   animSpeed;
    float   time;
    float   newFrame_g;
    float   endFrame;
    int     animSize;

    // Only continue once the animation parameters are validated.
    if(bone->startFrame < 0 || bone->endFrame < 0){
        return;
    }
    if(bone->startFrame > numFramesInFile || bone->endFrame > numFramesInFile){
        return;
    }

    // Add in animation speed to the current frame.
    animSpeed = bone->animSpeed;

    if(bone->pauseTime){
        time = (bone->pauseTime - bone->startTime) / 50.0f;
    }else{
        time = (currentTime - bone->startTime) / 50.0f;
    }

    // Time cannot be negative.
    if(time < 0.0f){
        time = 0.0f;
    }

    newFrame_g = bone->startFrame + (time * animSpeed);
    animSize = bone->endFrame - bone->startFrame;
    endFrame = bone->endFrame;

    // We are supposed to be animating.. right?
    if(animSize){
        // Yes we are, did we run off the end?
        if(((animSpeed > 0.0f) && (newFrame_g > endFrame - 1)) ||
           ((animSpeed < 0.0f) && (newFrame_g < endFrame + 1)))
        {
            // We are, decide what to do.
            if(bone->flags & BONE_ANIM_OVERRIDE_LOOP){
                // Get our new animation frame back within the bounds of the animation set.
                if(animSpeed < 0.0f){
                    // We don't use this case, or so I am told.
                    // FIXME BOE
                    Com_DPrintf(S_COLOR_MAGENTA "G2_TimingModel: animSpeed < 0.0f inside BONE_ANIM_OVERRIDE_LOOP, report to boe@1fxmod.org\n");
                }else{
                    // Should we be creating a virtual frame?
                    if((newFrame_g > endFrame - 1) && (newFrame_g < endFrame)){
                        // Now figure out what we are lerping between.
                        // Delta is the fraction between this frame and the next, since the new anim is always at a .0f
                        *lerp = (newFrame_g - (int)newFrame_g);

                        // Frames are easy to calculate.
                        *currentFrame = (int)newFrame_g;
                        *newFrame = bone->startFrame;
                    }else{
                        if(newFrame_g >= endFrame){
                            newFrame_g = endFrame + fmod(newFrame_g-endFrame,animSize) - animSize;
                        }

                        // Now figure out what we are lerping between.
                        // Delta is the fraction between this frame and the next, since the new anim is always at a .0f
                        *lerp = (newFrame_g - (int)newFrame_g);

                        // Frames are easy to calculate.
                        *currentFrame = (int)newFrame_g;

                        // Should we be creating a virtual frame?
                        if(newFrame_g >= endFrame - 1){
                            *newFrame = bone->startFrame;
                        }else{
                            *newFrame = *currentFrame + 1;
                        }
                    }
                }
            }else{
                if(((bone->flags & (BONE_ANIM_OVERRIDE_FREEZE)) == (BONE_ANIM_OVERRIDE_FREEZE))){
                    // If we are supposed to reset the default anim, then do so.
                    if(animSpeed > 0.0f){
                        *currentFrame = bone->endFrame - 1;
                    }else{
                        *currentFrame = bone->endFrame + 1;
                    }

                    *newFrame = *currentFrame;
                    *lerp = 0;
                }else{
                    bone->flags &= ~(BONE_ANIM_TOTAL);
                }
            }
        }else{
            if(animSpeed > 0.0f){
                // Frames are easy to calculate.
                *currentFrame = (int)newFrame_g;

                // Figure out the difference between the two frames.
                // We have to decide what frame and what percentage of that
                // frame we want to display.
                *lerp = (newFrame_g - *currentFrame);
                *newFrame = *currentFrame + 1;

                // Are we now on the end frame?
                if(*newFrame >= endFrame){
                    // We only want to lerp with the first frame of the anim if we are looping.
                    if(bone->flags & BONE_ANIM_OVERRIDE_LOOP){
                        *newFrame = bone->startFrame;
                    }else{
                        // If we intend to end this anim or freeze after this, then just keep on the last frame.
                        *newFrame = bone->endFrame - 1;
                    }
                }
            }else{
                *lerp = (ceil(newFrame_g) - newFrame_g);

                // Frames are easy to calculate.
                *currentFrame = ceil(newFrame_g);

                if(*currentFrame > bone->startFrame){
                    *currentFrame = bone->startFrame;
                    *newFrame = *currentFrame;
                    *lerp = 0.0f;
                }else{
                    *newFrame = *currentFrame - 1;

                    // Are we now on the end frame?
                    if(*newFrame < endFrame + 1){
                        // We only want to lerp with the first frame of the anim if we are looping.
                        if(bone->flags & BONE_ANIM_OVERRIDE_LOOP){
                            *newFrame = bone->startFrame;
                        }else{
                            // If we intend to end this anim or freeze after this, then just keep on the last frame.
                            *newFrame = bone->endFrame + 1;
                        }
                    }
                }
            }
        }
    }else{
        if(animSpeed < 0.0f){
            *currentFrame = bone->endFrame + 1;
        }else{
            *currentFrame = bone->endFrame - 1;
        }

        if(currentFrame < 0){
            *currentFrame = 0;
        }

        *newFrame = *currentFrame;
        *lerp = 0;
    }
}

/*
==============
G2_TransformBone

Transforms bone in bone cache.
==============
*/

static void G2_TransformBone(CBoneCache_t *mBoneCache, int boneIndex)
{
    int                         i;
    int                         angleOverride;
    int                         parent;
    float                       frontlerp;
    boneInfo_t                  *boneFound;
    CBoneCalc_t                 *boneCalc;
    CTransformBone_t            *transformBone;
    CTransformBone_t            *transformBoneParent;
    static int                  j;
    static mdxaBone_t           tbone[6];
    static mdxaSkel_t           *skel;
    static mdxaSkelOffsets_t    *offsets;

    angleOverride   = 0;
    boneFound       = NULL;
    boneCalc        = G2_GetCalcBone(mBoneCache->mBones, boneIndex);
    transformBone   = G2_GetTransformBone(mBoneCache->mFinalBones, boneIndex);

    //
    // Find the bone in the list.
    //
    if(mBoneCache->parent->mBoneList != NULL){
        for(i = 0; i < mBoneCache->parent->numBones; i++){
            if(mBoneCache->parent->mBoneList[i]->boneNumber == boneIndex){
                boneFound = mBoneCache->parent->mBoneList[i];
                break;
            }
        }
    }

    // Should this bone be overridden by a bone in the bone list?
    if(boneFound){
        // Yes, we found a bone in the list.
        angleOverride = 0;

        // Do we override the rotational angles?
        if(boneFound->flags & BONE_ANGLES_TOTAL){
            angleOverride = boneFound->flags & BONE_ANGLES_TOTAL;
        }

        // Should this animation be overridden by an animation in the bone list?
        if(boneFound->flags & (BONE_ANIM_OVERRIDE_LOOP | BONE_ANIM_OVERRIDE)){
            G2_TimingModel(boneFound, mBoneCache->incomingTime, mBoneCache->parent->aHeader->numFrames, &boneCalc->currentFrame, &boneCalc->newFrame, &boneCalc->backlerp);
        }
    }

    // Figure out where the location of the bone animation data is.
    if(!(boneCalc->newFrame >= 0 && boneCalc->newFrame < mBoneCache->parent->aHeader->numFrames)){
        boneCalc->newFrame = 0;
    }
    if(!(boneCalc->currentFrame >= 0 && boneCalc->currentFrame < mBoneCache->parent->aHeader->numFrames)){
        boneCalc->currentFrame = 0;
    }

    //
    // Lerp this bone - use the temp space on the ref entity to put the bone transforms into.
    //
    if(!boneCalc->backlerp)
    {
        G2_UncompressBone(tbone[2].matrix, boneIndex, mBoneCache->parent->aHeader, boneCalc->currentFrame);

        if(!boneIndex){
            // Now multiply by the root matrix, so we can offset this model should we need to.
            G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &mBoneCache->rootMatrix, &tbone[2]);
        }
    }else{
        frontlerp = 1.0f - boneCalc->backlerp;

        G2_UncompressBone(tbone[0].matrix, boneIndex, mBoneCache->parent->aHeader, boneCalc->newFrame);
        G2_UncompressBone(tbone[1].matrix, boneIndex, mBoneCache->parent->aHeader, boneCalc->currentFrame);

        for(j = 0; j < 12; j++){
            ((float *)&tbone[2])[j] = (boneCalc->backlerp * ((float *)&tbone[0])[j])
                + (frontlerp * ((float *)&tbone[1])[j]);
        }

        if(!boneIndex){
            // Now multiply by the root matrix, so we can offset this model should we need to.
            G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &mBoneCache->rootMatrix, &tbone[2]);
        }
    }

    //
    // Figure out where the bone hierarchy info is.
    //
    offsets             = (mdxaSkelOffsets_t *)((byte *)mBoneCache->parent->aHeader + sizeof(mdxaHeader_t));
    skel                = (mdxaSkel_t *)((byte *)mBoneCache->parent->aHeader + sizeof(mdxaHeader_t) + offsets->offsets[boneIndex]);
    parent              = transformBone->parent;
    transformBoneParent = G2_GetTransformBone(mBoneCache->mFinalBones, parent);

    if(angleOverride & BONE_ANGLES_REPLACE){
        mdxaBone_t  *bone;
        mdxaBone_t  temp, firstPass;
        mdxaBone_t  newMatrixTemp;
        float       matrixScale;

        bone = &transformBone->boneMatrix;

        // Give us the matrix the animation thinks we should have, so we can get the correct X&Y coors.
        G2_Multiply_3x4Matrix(&firstPass, &transformBone->boneMatrix, &tbone[2]);

        // Override it directly.
        G2_Multiply_3x4Matrix(&temp, &firstPass, &skel->BasePoseMat);
        matrixScale = VectorLength((float *)&temp);

        for(i = 0; i < 3; i++){
            for(j = 0; j < 3; j++){
                newMatrixTemp.matrix[i][j] = boneFound->matrix.matrix[i][j] * matrixScale;
            }
        }

        newMatrixTemp.matrix[0][3] = temp.matrix[0][3];
        newMatrixTemp.matrix[1][3] = temp.matrix[1][3];
        newMatrixTemp.matrix[2][3] = temp.matrix[2][3];

        G2_Multiply_3x4Matrix(bone, &newMatrixTemp, &skel->BasePoseMatInv);

    }else if(angleOverride & BONE_ANGLES_PREMULT){
        if(!boneIndex){
            // Use the incoming root matrix as our basis.
            G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &transformBoneParent->boneMatrix, &boneFound->matrix);
        }else{
            // Convert from 3x4 matrix to a 4x4 matrix.
            G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &transformBoneParent->boneMatrix, &boneFound->matrix);
        }

    }else{
        // Now transform the matrix by its parent, assuming we have a parent, and we aren't overriding the angles absolutely.
        if(boneIndex){
            G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &transformBoneParent->boneMatrix, &tbone[2]);
        }
    }

    // Now multiply our resulting bone by an override matrix, should we need to.
    if(angleOverride & BONE_ANGLES_POSTMULT){
        mdxaBone_t  tempMatrix;

        memcpy(&tempMatrix,&transformBone->boneMatrix, sizeof(mdxaBone_t));
        G2_Multiply_3x4Matrix(&transformBone->boneMatrix, &tempMatrix, &boneFound->matrix);
    }
}

/*
==============
G2_BoneEvalLow

Evaluate the bone and its parent.
==============
*/

static void G2_BoneEvalLow(CBoneCache_t *mBoneCache, int boneIndex)
{
    CBoneCalc_t         *calcBone;
    CTransformBone_t    *transformBone;

    // Check if the specified bone index is valid.
    G2_IsBoneIndexValid(mBoneCache, boneIndex, "G2_BoneEvalLow");

    // Get bones.
    calcBone        = G2_GetCalcBone(mBoneCache->mBones, boneIndex);
    transformBone   = G2_GetTransformBone(mBoneCache->mFinalBones, boneIndex);

    // Do we need to evaluate the bone?
    if(transformBone->touch != mBoneCache->mCurrentTouch){
        // Do we need to evaluate the parent?
        if(transformBone->parent != -1){
            CBoneCalc_t *calcBoneParent;

            G2_BoneEvalLow(mBoneCache, transformBone->parent);
            calcBoneParent = G2_GetCalcBone(mBoneCache->mBones, transformBone->parent);

            calcBone->newFrame = calcBoneParent->newFrame;
            calcBone->currentFrame = calcBoneParent->currentFrame;
        }

        G2_TransformBone(mBoneCache, boneIndex);
        transformBone->touch = mBoneCache->mCurrentTouch;
    }
}

/*
==============
G2_BoneEval

Evaluate the bone.
==============
*/

mdxaBone_t *G2_BoneEval(CBoneCache_t *mBoneCache, int boneIndex)
{
    CTransformBone_t    *transformBone;

    transformBone = G2_GetTransformBone(mBoneCache->mFinalBones, boneIndex);
    if(transformBone->touch != mBoneCache->mCurrentTouch){
        // Evaluate this bone and its parent.
        G2_BoneEvalLow(mBoneCache, boneIndex);
    }

    return &transformBone->boneMatrix;
}

/*
=============================================
---------------------
Bone matrix functions
---------------------
=============================================
*/

/*
==============
G2_BoneGenerateMatrix

Generate a matrix for a given bone
using the supplied new angles.
==============
*/

void G2_BoneGenerateMatrix(const model_t *modAnim, boneInfo_t **boneList, int boneIndex, const float *angles, int flags,
                           const Eorientations up, const Eorientations left, const Eorientations forward)
{
    mdxaSkel_t          *skel;
    mdxaSkelOffsets_t   *offsets;
    vec3_t              newAngles;
    mdxaBone_t          temp;
    mdxaBone_t          permutation;
    mdxaBone_t          *boneOverride;

    // Determine the bone to override from the given bone from the bone list.
    boneOverride = &boneList[boneIndex]->matrix;

    if(flags & (BONE_ANGLES_PREMULT | BONE_ANGLES_POSTMULT)){
        // Build us a matrix out of the angles we are fed.
        // But swap y and z because of wacky Quake setup.

        // Determine what axis newAngles yaw should revolve around.
        switch(up){
            case NEGATIVE_X:
                newAngles[1] = angles[2] + 180;
                break;
            case POSITIVE_X:
                newAngles[1] = angles[2];
                break;
            case NEGATIVE_Y:
                newAngles[1] = angles[0];
                break;
            case POSITIVE_Y:
                newAngles[1] = angles[0];
                break;
            case NEGATIVE_Z:
                newAngles[1] = angles[1] + 180;
                break;
            case POSITIVE_Z:
                newAngles[1] = angles[1];
                break;
            default:
                break;
        }

        // Determine what axis newAngles pitch should revolve around.
        switch(left){
            case NEGATIVE_X:
                newAngles[0] = angles[2];
                break;
            case POSITIVE_X:
                newAngles[0] = angles[2] + 180;
                break;
            case NEGATIVE_Y:
                newAngles[0] = angles[0];
                break;
            case POSITIVE_Y:
                newAngles[0] = angles[0] + 180;
                break;
            case NEGATIVE_Z:
                newAngles[0] = angles[1];
                break;
            case POSITIVE_Z:
                newAngles[0] = angles[1];
                break;
            default:
                break;
        }

        // Determine what axis newAngles roll should revolve around.
        switch(forward){
            case NEGATIVE_X:
                newAngles[2] = angles[2];
                break;
            case POSITIVE_X:
                newAngles[2] = angles[2];
                break;
            case NEGATIVE_Y:
                newAngles[2] = angles[0];
                break;
            case POSITIVE_Y:
                newAngles[2] = angles[0] + 180;
                break;
            case NEGATIVE_Z:
                newAngles[2] = angles[1];
                break;
            case POSITIVE_Z:
                newAngles[2] = angles[1] + 180;
                break;
            default:
                break;
        }

        // Create the matrix.
        G2_CreateMatrix(boneOverride, newAngles);

        // Figure out where the bone hierarchy info is.
        offsets = (mdxaSkelOffsets_t *)((byte *)modAnim->modelData + sizeof(mdxaHeader_t));
        skel = (mdxaSkel_t *)((byte *)modAnim->modelData + sizeof(mdxaHeader_t) + offsets->offsets[boneList[boneIndex]->boneNumber]);

        // Multiply the matrix.
        G2_Multiply_3x4Matrix(&temp, boneOverride, &skel->BasePoseMatInv);
        G2_Multiply_3x4Matrix(boneOverride, &skel->BasePoseMat, &temp);
    }else{
        VectorCopy(angles, newAngles);

        // Create the matrix.
        if(left == POSITIVE_Y){
            newAngles[0] += 180;
        }
        G2_CreateMatrix(&temp, newAngles);

        permutation.matrix[0][0] = permutation.matrix[0][1] = permutation.matrix[0][2] = permutation.matrix[0][3] = 0;
        permutation.matrix[1][0] = permutation.matrix[1][1] = permutation.matrix[1][2] = permutation.matrix[1][3] = 0;
        permutation.matrix[2][0] = permutation.matrix[2][1] = permutation.matrix[2][2] = permutation.matrix[2][3] = 0;

        // Determine what axis newAngles yaw should revolve around.
        switch(forward){
            case NEGATIVE_X:
                permutation.matrix[0][0] = -1;
                break;
            case POSITIVE_X:
                permutation.matrix[0][0] = 1;
                break;
            case NEGATIVE_Y:
                permutation.matrix[1][0] = -1;
                break;
            case POSITIVE_Y:
                permutation.matrix[1][0] = 1;
                break;
            case NEGATIVE_Z:
                permutation.matrix[2][0] = -1;
                break;
            case POSITIVE_Z:
                permutation.matrix[2][0] = 1;
                break;
            default:
                break;
        }

        // Determine what axis newAngles pitch should revolve around.
        switch(left){
            case NEGATIVE_X:
                permutation.matrix[0][1] = -1;
                break;
            case POSITIVE_X:
                permutation.matrix[0][1] = 1;
                break;
            case NEGATIVE_Y:
                permutation.matrix[1][1] = -1;
                break;
            case POSITIVE_Y:
                permutation.matrix[1][1] = 1;
                break;
            case NEGATIVE_Z:
                permutation.matrix[2][1] = -1;
                break;
            case POSITIVE_Z:
                permutation.matrix[2][1] = 1;
                break;
            default:
                break;
        }

        // Determine what axis newAngles roll should revolve around.
        switch(up){
            case NEGATIVE_X:
                permutation.matrix[0][2] = -1;
                break;
            case POSITIVE_X:
                permutation.matrix[0][2] = 1;
                break;
            case NEGATIVE_Y:
                permutation.matrix[1][2] = -1;
                break;
            case POSITIVE_Y:
                permutation.matrix[1][2] = 1;
                break;
            case NEGATIVE_Z:
                permutation.matrix[2][2] = -1;
                break;
            case POSITIVE_Z:
                permutation.matrix[2][2] = 1;
                break;
            default:
                break;
        }

        // Multiply the matrix.
        G2_Multiply_3x4Matrix(boneOverride, &temp, &permutation);
    }

    // Keep a copy of the matrix in the newMatrix variable.
    memcpy(&boneList[boneIndex]->newMatrix, &boneList[boneIndex]->matrix, sizeof(mdxaBone_t));
}

/*
==============
G2_ConstructGhoulSkeleton

Builds a complete skeleton for all Ghoul II models
in a Ghoul II array, using LOD 0.
==============
*/

void G2_ConstructGhoulSkeleton(CGhoul2Array_t *ghlInfo, const int frameNum)
{
    CGhoul2Model_t  *model;
    int             i, numValid;

    // Walk through all the models in our array.
    numValid = 0;
    for(i = 0; i < G2_MAX_MODELS_IN_LIST; i++){
        model = ghlInfo->models[i];

        // Check if this slot is allocated.
        if(model != NULL){
            // Make sure the the bone is re-rendered.
            model->mBoneCache->mCurrentTouch++;

            // Set the incoming time based on the frame number.
            model->mBoneCache->incomingTime = frameNum;

            numValid++;
            if(numValid == ghlInfo->numModels){
                // Don't continue if we've iterated
                // through all allocated slots.
                break;
            }
        }
    }
}
