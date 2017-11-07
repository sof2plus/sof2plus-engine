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

void G2API_ListBones(CGhoul2Model_t *ghlInfo, int frame)
{
    if(!G2_SetupModelPointers(ghlInfo)){
        return;
    }

    // FIXME BOE
    //G2_List_Model_Bones(ghlInfo->mFileName, frame);
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
