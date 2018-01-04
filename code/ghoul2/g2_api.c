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
// g2_api.c - Main Ghoul II API routines.

#include "g2_local.h"

/*
==================
G2API_ListBones

Lists all model bones.
==================
*/

void G2API_ListBones(CGhoul2Array_t *ghlInfo, int modelIndex)
{
    CGhoul2Model_t      *model;
    mdxaSkel_t          *skel, *childSkel;
    mdxaSkelOffsets_t   *offsets;
    int                 i, x;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_ListBones");
    if(!model){
        return;
    }

    //
    // Print function information.
    //
    Com_Printf("-----------------------------------------\n");
    Com_Printf("Listing Ghoul II model bone info\n");
    Com_Printf("Index in Ghoul II instance: %d\n", modelIndex);
    Com_Printf("Filename: %s\n", model->mFileName);
    Com_Printf("-----------------------------------------\n");

    //
    // Figure out where the offset list is.
    //
    offsets = (mdxaSkelOffsets_t *)((byte *)model->aHeader + sizeof(mdxaHeader_t));

    //
    // Walk each bone and list its info.
    //
    for(i = 0; i < model->aHeader->numBones; i++){
        skel = (mdxaSkel_t *)((byte *)model->aHeader + sizeof(mdxaHeader_t) + offsets->offsets[i]);

        // Print bone info.
        Com_Printf("== Bone %d ==\n", i);
        Com_Printf("Name: %s\n", skel->name);
        Com_Printf("X pos: %f\n", skel->BasePoseMat.matrix[0][3]);
        Com_Printf("Y pos: %f\n", skel->BasePoseMat.matrix[1][3]);
        Com_Printf("Z pos: %f\n", skel->BasePoseMat.matrix[2][3]);

        // If we are in verbose mode, give us more details.
        if(r_verbose->integer){
            Com_Printf("Flags: %d\n", skel->flags);
            Com_Printf("Num descendants: %d\n", skel->numChildren);

            // Print information for each child bone of the referenced bone.
            // We don't print information of their children, if any.
            for(x = 0; x < skel->numChildren; x++){
                childSkel = (mdxaSkel_t *)((byte *)model->aHeader + sizeof(mdxaHeader_t) + offsets->offsets[skel->children[x]]);

                Com_Printf("=== Bone %d, child bone %d ===\n", i, x);
                Com_Printf("Name: %s\n", childSkel->name);
                Com_Printf("X pos: %f\n", childSkel->BasePoseMat.matrix[0][3]);
                Com_Printf("Y pos: %f\n", childSkel->BasePoseMat.matrix[1][3]);
                Com_Printf("Z pos: %f\n", childSkel->BasePoseMat.matrix[2][3]);
                Com_Printf("Flags: %d\n", childSkel->flags);
                Com_Printf("Num descendants: %d\n", childSkel->numChildren);
            }
        }
    }
}

/*
==================
G2API_ListSurfaces

Lists all surfaces associated with a Ghoul II model.
==================
*/

void G2API_ListSurfaces(CGhoul2Array_t *ghlInfo, int modelIndex)
{
    CGhoul2Model_t      *model;
    mdxmHeader_t        *mdxmHeader;
    mdxmSurfHierarchy_t *surf;
    int                 i, x;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_ListSurfaces");
    if(!model){
        return;
    }

    //
    // Print function information.
    //
    Com_Printf("-----------------------------------------\n");
    Com_Printf("Listing Ghoul II associated surfaces\n");
    Com_Printf("Index in Ghoul II instance: %d\n", modelIndex);
    Com_Printf("Filename: %s\n", model->mFileName);
    Com_Printf("-----------------------------------------\n");

    //
    // Find the first surface.
    //
    mdxmHeader = model->currentModel->modelData;
    surf = (mdxmSurfHierarchy_t *)((byte *)mdxmHeader + mdxmHeader->ofsSurfHierarchy);

    //
    // Walk each surface and list its info.
    //
    for(i = 0; i < mdxmHeader->numSurfaces; i++){
        Com_Printf("== Surface %d ==\n", i);
        Com_Printf("Name: %s\n", surf->name);

        // If we are in verbose mode, give us more details.
        if(r_verbose->integer){
            Com_Printf("Flags: %d\n", surf->flags);
            Com_Printf("Num descendants: %d\n", surf->numChildren);

            // Print the index for each child surface of the referenced surface.
            if(surf->numChildren){
                Com_Printf("=== Descendants ===\n");
                for(x = 0; x < surf->numChildren; x++){
                    Com_Printf("Index: %d\n", surf->childIndexes[x]);
                }
            }
        }

        // Find the next surface.
        surf = (mdxmSurfHierarchy_t *)((byte *)surf + (size_t)(&((mdxmSurfHierarchy_t *)0)->childIndexes[surf->numChildren]));
    }
}

/*
==================
G2API_HaveWeGhoul2Models

Returns qtrue if there are valid Ghoul II models present in the Ghoul II instance.
==================
*/

qboolean G2API_HaveWeGhoul2Models(CGhoul2Array_t *ghlInfo)
{
    int i;

    // Don't continue if there are no models present in the list.
    if(!ghlInfo->numModels){
        return qfalse;
    }

    //
    // Walk through the list.
    //
    for(i = 0; i < G2_MAX_MODELS_IN_LIST; i++){
        // Continue if this model isn't allocated.
        if(!ghlInfo->models[i]){
            continue;
        }

        // The model is properly setup if the model index isn't -1.
        if(ghlInfo->models[i]->mModelIndex != -1){
            return qtrue;
        }
    }

    return qfalse;
}

/*
==================
G2API_InitGhoul2Model

Initialize all that needs to be on a new Ghoul II model.
==================
*/

int G2API_InitGhoul2Model(CGhoul2Array_t **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin,
                          qhandle_t customShader, int modelFlags, int lodBias)
{
    CGhoul2Model_t  *ghoul2;

    // Are we actually asking for a model to be loaded?
    if(!fileName || !fileName[0]){
        return -1;
    }

    if(*ghoul2Ptr){
        Com_Printf(S_COLOR_RED "G2API_InitGhoul2Model: ghoul2Ptr already initialized. This function should not be used to create additional models.\n");
        return -1;
    }

    // New Ghoul II model. Allocate main memory of our array object on the hunk.
    *ghoul2Ptr = Hunk_Alloc(sizeof(CGhoul2Array_t), h_low);

    // Now allocate memory for the actual Ghoul II model.
    ghoul2 = Hunk_Alloc(sizeof(CGhoul2Model_t), h_low);

    // Set new Ghoul II model info.
    strncpy(ghoul2->mFileName, fileName, sizeof(ghoul2->mFileName));
    ghoul2->mModelIndex = 0;

    if(!G2_SetupModelPointers(ghoul2) || !G2_InitBoneCache(ghoul2)){
        ghoul2->mFileName[0] = 0;
        ghoul2->mModelIndex = -1;
    }else{
        // Is a custom skin set to be used?
        if(R_GetSkinByHandle(customSkin) != NULL){
            ghoul2->mCustomSkin = customSkin;
        }else{
            ghoul2->mCustomSkin = -1;
        }
    }

    // Update our new Ghoul II model array object.
    (*ghoul2Ptr)->models[0] = ghoul2;
    (*ghoul2Ptr)->numModels = 1;

    return ghoul2->mModelIndex;
}

/*
==================
G2API_SetBoneAngles

Given a model handle and a bone name, set
the angles specifically for overriding.
==================
*/

qboolean G2API_SetBoneAngles(CGhoul2Array_t *ghlInfo, const int modelIndex, const char *boneName, const vec3_t angles, const int flags,
                             const Eorientations up, const Eorientations left, const Eorientations forward,
                             int blendTime, int currentTime)
{
    CGhoul2Model_t      *model;
    const model_t       *modAnim;
    int                 boneIndex;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_SetBoneAngles");
    if(!model){
        return qfalse;
    }

    // Get the Ghoul II animation model from the specified model.
    modAnim = model->animModel;

    // Find the bone inside Ghoul II animation model.
    boneIndex = G2_IsBoneInList(modAnim, model->mBoneList, model->numBones, -1, boneName);

    // Did we find it?
    if(boneIndex == -1){
        // No, we didn't. Let's try to add this bone in.
        boneIndex = G2_AddBone(modAnim, model->mBoneList, &model->numBones, boneName);

        // If we couldn't add this bone, don't bother setting angles and flags.
        if(boneIndex == -1){
            return qfalse;
        }
    }

    // Found or created, set the angles and flags correctly.
    model->mBoneList[boneIndex]->flags &= ~(BONE_ANGLES_TOTAL);
    model->mBoneList[boneIndex]->flags |= flags;

    // Generate the end matrix.
    G2_BoneGenerateMatrix(modAnim, model->mBoneList, boneIndex, angles, flags, up, left, forward);

    return qtrue;
}

/*
==================
G2API_SetBoneAnim

Set up or modify an existing bone entry
for a new set of animations.
==================
*/

qboolean G2API_SetBoneAnim(CGhoul2Array_t *ghlInfo, const int modelIndex, const char *boneName, const int AstartFrame, const int AendFrame,
                           const int flags, const float animSpeed, const int currentTime, const float AsetFrame)
{
    CGhoul2Model_t  *model;
    const model_t   *modAnim;
    int             startFrame, endFrame;
    float           setFrame;
    int             boneNumber, boneIndex;

    startFrame = AstartFrame;
    endFrame = AendFrame;
    setFrame = AsetFrame;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_SetBoneAnim");
    if(!model){
        return qfalse;
    }

    //
    // Check if frame counts are out of bounds.
    //
    if(startFrame < 0 || startFrame >= 100000){
        startFrame = 0;
    }

    if(endFrame <= 0 || endFrame >= 100000){
        endFrame = 1;
    }

    if((setFrame < 0.0f && setFrame != -1.0f)
        || setFrame > 100000.0f)
    {
        setFrame = 0.0f;
    }

    //
    // Find the bone in the bone list.
    //

    // Get the Ghoul II animation model from the specified model.
    modAnim = model->animModel;

    // Get the bone number we need.
    boneNumber = G2_FindBoneInModel(modAnim, boneName);

    // Is the bone present in our bone list?
    boneIndex = G2_IsBoneInList(modAnim, model->mBoneList, model->numBones, boneNumber, boneName);
    if(boneIndex == -1){
        // No, it is not. Add it to the bone list.
        boneIndex = G2_AddBone(modAnim, model->mBoneList, &model->numBones, boneName);

        // Is the bone added to the bone list?
        if(boneIndex == -1){
            Com_Printf(S_COLOR_RED "G2API_SetBoneAnim: Failed to add bone %d to the specified Ghoul II model.\n", boneNumber);
            return qfalse;
        }
    }

    //
    // Set the anim data and flags.
    //
    model->mBoneList[boneIndex]->startFrame = startFrame;
    model->mBoneList[boneIndex]->endFrame = endFrame;
    model->mBoneList[boneIndex]->animSpeed = animSpeed;
    model->mBoneList[boneIndex]->pauseTime = 0;

    model->mBoneList[boneIndex]->flags &= ~(BONE_ANIM_TOTAL);
    if(model->mBoneList[boneIndex]->flags < 0){
        model->mBoneList[boneIndex]->flags = 0;
    }
    model->mBoneList[boneIndex]->flags |= flags;

    //
    // Start the animation.
    //
    if(setFrame != -1){
        model->mBoneList[boneIndex]->lastTime = model->mBoneList[boneIndex]->startTime = (currentTime - (((setFrame - (float)startFrame) * 50.0) / animSpeed));
    }else{
        model->mBoneList[boneIndex]->lastTime = model->mBoneList[boneIndex]->startTime = currentTime;
    }

    return qtrue;
}

/*
==================
G2API_GetGLAName

Gets the name of the Ghoul II
animation file associated with
the specified model from the
Ghoul II array.
==================
*/

qboolean G2API_GetGLAName(CGhoul2Array_t *ghlInfo, int modelIndex, char *dest, int destSize)
{
    CGhoul2Model_t  *model;
    mdxmHeader_t    *mdxmHeader;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_SetBoneAnim");
    if(!model){
        return qfalse;
    }

    //
    // Get the Ghoul II animation file name.
    //

    // First, get the Ghoul II mesh file header.
    mdxmHeader = model->currentModel->modelData;

    // Copy the animation file name from the loaded header
    // to the destination buffer.
    Q_strncpyz(dest, mdxmHeader->animName, destSize);

    return qtrue;
}

/*
==================
G2API_CollisionDetect

Detailed trace of what clients have been hit at
the given origin. Stores result in the supplied
collision records.

NOTE: The incoming collision records are reset
and are sorted on distance at the end of the
collision detection.
==================
*/

static int QDECL G2_CollisionDetectSortDistance(const void *recA, const void *recB)
{
    float distA, distB;

    distA = ((CollisionRecord_t *)recA)->mDistance;
    distB = ((CollisionRecord_t *)recB)->mDistance;

    if(distA < distB){
        return -1;
    }

    return 1;
}

void G2API_CollisionDetect(CollisionRecord_t *collRecMap, CGhoul2Array_t *ghoul2, const vec3_t angles, const vec3_t position,
                           int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod)
{
    vec3_t      transRayStart;
    vec3_t      transRayEnd;
    mdxaBone_t  worldMatrix;
    mdxaBone_t  worldMatrixInv;
    int         i;
    qboolean    mValid;

    //
    // Initialize collision trace records
    // before anything else.
    //
    memset(collRecMap, 0, sizeof(G2Trace_t));
    for(i = 0; i < MAX_G2_COLLISIONS; i++){
        collRecMap[i].mEntityNum = -1;
    }

    //
    // Don't continue if we cannot properly setup the Ghoul II models
    // on at least one model in the array.
    //
    if(ghoul2 == NULL || ghoul2->numModels == 0){
        return;
    }

    // Walk the array.
    mValid = qfalse;
    for(i = 0; i < G2_MAX_MODELS_IN_LIST; i++){
        // Continue if this model isn't allocated.
        if(!ghoul2->models[i]){
            continue;
        }

        // Setup model pointers for this model.
        if(!G2_SetupModelPointers(ghoul2->models[i])){
            continue;
        }

        mValid = qtrue;
        break;
    }

    // Did we find *any* valid model?
    // If not, useless to continue.
    if(mValid == qfalse){
        return;
    }

    //
    // Build model.
    //

    // Make sure we have transformed the whole skeletons for each model.
    G2_ConstructGhoulSkeleton(ghoul2, frameNumber);

    // Pre-generate the world matrix.
    // We use this to transform the incoming ray.
    G2_GenerateWorldMatrix(&worldMatrix, &worldMatrixInv, angles, position);

    // Having done that, it's time to build the model.
    G2_TransformModel(ghoul2, frameNumber, scale, useLod);

    //
    // Model built.
    // Check if any triangles are actually hit.
    //

    // Translate the rays to model space.
    G2_TransformTranslatePoint(rayStart, transRayStart, &worldMatrixInv);
    G2_TransformTranslatePoint(rayEnd, transRayEnd, &worldMatrixInv);

    // Now walk each model and check the ray against each poly.
    G2_TraceModels(ghoul2, transRayStart, transRayEnd, &worldMatrix, collRecMap, entNum, traceFlags, useLod);

    // Check how many collision records we have.
    for(i = 0; i < MAX_G2_COLLISIONS; i++){
        if(collRecMap[i].mEntityNum == -1){
            break;
        }
    }

    // Sort the resulting array of collision records so they are distance sorted.
    qsort(collRecMap, i, sizeof(CollisionRecord_t), G2_CollisionDetectSortDistance);
}

/*
==================
G2API_RegisterSkin

Registers the specified skin
based on the skin pairs.
==================
*/

qhandle_t G2API_RegisterSkin(const char *skinName, int numPairs, const char *skinPairs)
{
    //
    // Let the server renderer handle the parsing of the skin.
    // Return the handle to the skin upon completion.
    //
    return RE_RegisterServerSkin(skinName, numPairs, skinPairs);
}

/*
==================
G2API_SetSkin

Sets a custom skin on a Ghoul II model
from the array.

Returns qtrue if the model and skin are
both found valid.
==================
*/

qboolean G2API_SetSkin(CGhoul2Array_t *ghlInfo, int modelIndex, qhandle_t customSkin)
{
    CGhoul2Model_t  *model;

    //
    // Check whether the specified model is valid.
    //
    model = G2_IsModelIndexValid(ghlInfo, modelIndex, "G2API_SetSkin");
    if(!model){
        return qfalse;
    }

    //
    // Check if the specified skin is valid.
    //
    if(customSkin < 0 || customSkin >= tr.numSkins){
        return qfalse;
    }

    // Set the custom skin.
    model->mCustomSkin = customSkin;

    // All valid.
    return qtrue;
}
