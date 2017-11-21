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
// g2_misc.c - Miscellaneous Ghoul II routines.

#include "g2_local.h"

/*
==================
G2_SetModelInfo

Model is invalidated, re-setup the model.
Returns true if the model is properly setup.
==================
*/

static qboolean G2_SetModelInfo(CGhoul2Model_t *ghlInfo)
{
    mdxmHeader_t    *mdxmHeader;
    mdxaHeader_t    *mdxaHeader;

    // First off, check if the model is properly registered on the server.
    if(ghlInfo->mModelIndex != -1){
        // Check whether the model is valid, and if not, re-register it.
        if(!ghlInfo->mModel){
            ghlInfo->mModel = RE_RegisterServerModel(ghlInfo->mFileName);
        }
        ghlInfo->currentModel = R_GetModelByHandle(ghlInfo->mModel);
    }

    // Check whether the model data is valid.
    if(!ghlInfo->currentModel){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: NULL Ghoul II model (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }

    // Start checking the model data and set info along the way.
    // Check if the mesh file header is valid.
    mdxmHeader = (mdxmHeader_t *)ghlInfo->currentModel->modelData;
    if(!mdxmHeader){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II mesh file has no valid header (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }

    // Check if the mesh file contains data.
    if(!mdxmHeader->ofsEnd){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II mesh file is zero-sized (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }
    ghlInfo->currentModelSize = mdxmHeader->ofsEnd;

    // Check if the Ghoul II animation file is properly loaded.
    if(!mdxmHeader->animIndex){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model has no associated Ghoul II animation file (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }
    ghlInfo->animModel = R_GetModelByHandle(mdxmHeader->animIndex);

    // Check if the animation file header is valid.
    mdxaHeader = (mdxaHeader_t *)ghlInfo->animModel->modelData;
    if(!mdxaHeader){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II animation file has no valid header (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }
    ghlInfo->aHeader = mdxaHeader;

    // Check if the animation file contains data.
    if(!ghlInfo->aHeader->ofsEnd){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II animation file is zero-sized (%s).\n", ghlInfo->mFileName);
        return qfalse;
    }
    ghlInfo->currentAnimModelSize = ghlInfo->aHeader->ofsEnd;

    // Everything checks out.
    ghlInfo->mValid = qtrue;
    return qtrue;
}

/*
==================
G2_SetupModelPointers

Setup the model if this hasn't been done already.
Returns true if the model is properly setup.
==================
*/

qboolean G2_SetupModelPointers(CGhoul2Model_t *ghlInfo)
{
    if(!ghlInfo){
        Com_Printf(S_COLOR_RED "G2_SetupModelPointers: NULL ghlInfo.\n");
        return qfalse;
    }

    // Invalidate Ghoul II model.
    ghlInfo->mValid = qfalse;

    // Setup the model, see if everything checks out.
    if(!G2_SetModelInfo(ghlInfo)){
        // Info isn't valid, invalidate all Ghoul II specifics.
        ghlInfo->currentModel = 0;
        ghlInfo->currentModelSize = 0;
        ghlInfo->animModel = 0;
        ghlInfo->currentAnimModelSize = 0;
        ghlInfo->aHeader = 0;
        return qfalse;
    }

    // All valid.
    return qtrue;
}

/*
==================
G2_IsModelIndexValid

Checks whether the specified model index
is valid in our Ghoul II array.

Returns the Ghoul II model if valid,
NULL otherwise.
==================
*/

CGhoul2Model_t *G2_IsModelIndexValid(CGhoul2Array_t *ghlInfo, const int modelIndex, const char *caller)
{
    CGhoul2Model_t  *model;

    if(!ghlInfo){
        Com_Printf(S_COLOR_RED "%s/G2_IsModelIndexValid: ghlInfo pointer is NULL.\n", caller);
        return NULL;
    }

    if(modelIndex < 0 || modelIndex >= G2_MAX_MODELS_IN_LIST){
        Com_Printf(S_COLOR_RED "%s/G2_IsModelIndexValid: Model %d is out of bounds (Ghoul II instance has %d model(s)).\n",
            caller, modelIndex, ghlInfo->numModels);
        return NULL;
    }

    model = ghlInfo->models[modelIndex];
    if(!G2_SetupModelPointers(model)){
        Com_Printf(S_COLOR_RED "%s/G2_IsModelIndexValid: Failed to setup model pointers.\n", caller);
        return NULL;
    }

    // All valid.
    return model;
}
