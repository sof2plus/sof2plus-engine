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

void G2API_ListBones(CGhoul2Array_t *ghlInfo, int modelNumber)
{
    CGhoul2Model_t      *model;
    mdxaSkel_t          *skel, *childSkel;
    mdxaSkelOffsets_t   *offsets;
    int                 i, x;

    //
    // Check whether the specified model is valid.
    //
    if(!ghlInfo){
        Com_Printf(S_COLOR_RED "G2API_ListBones: ghlInfo pointer is NULL.\n");
        return;
    }

    if(modelNumber < 0 || modelNumber >= ghlInfo->numModels){
        Com_Printf(S_COLOR_RED "G2API_ListBones: Model %d is out of bounds (Ghoul II instance has %d models).\n",
            modelNumber, ghlInfo->numModels);
        return;
    }

    model = ghlInfo->models[modelNumber];
    if(!G2_SetupModelPointers(model)){
        Com_Printf(S_COLOR_RED "G2API_ListBones: Failed to setup model pointers.\n");
        return;
    }

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

    if(!G2_SetupModelPointers(ghoul2)){
        ghoul2->mFileName[0] = 0;
        ghoul2->mModelIndex = -1;
    }else{
        // FIXME BOE
        /*
        G2_Init_Bone_List(ghoul2[model].mBlist, ghoul2[model].aHeader->numBones);
        G2_Init_Bolt_List(ghoul2[model].mBltlist);
        ghoul2->mCustomShader = customShader;
        ghoul2->mCustomSkin = customSkin;
        ghoul2->mLodBias = lodBias;
        ghoul2->mAnimFrameDefault = 0;
        ghoul2->mFlags = 0;
        ghoul2->mModelBoltLink = -1;
        */
    }

    // Update our new Ghoul II model array object.
    (*ghoul2Ptr)->models[0] = ghoul2;
    (*ghoul2Ptr)->numModels = 1;

    return ghoul2->mModelIndex;
}
