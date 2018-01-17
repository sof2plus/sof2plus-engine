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
// tr_g2_misc.c - Miscellaneous Ghoul II routines.

#include "tr_g2_local.h"

/*
==================
G2_SetModelInfo

Model is invalidated, re-setup the model.
Returns true if the model is properly setup.
==================
*/

static qboolean G2_SetModelInfo(CGhoul2Model_t *model)
{
    mdxmHeader_t    *mdxmHeader;
    mdxaHeader_t    *mdxaHeader;

    // First off, check if the model is properly registered on the server.
    if(model->mModelIndex != -1){
        // Check whether the model is valid, and if not, re-register it.
        if(!model->mModel){
            model->mModel = RE_RegisterServerModel(model->mFileName);
        }
        model->currentModel = R_GetModelByHandle(model->mModel);
    }

    // Check whether the model data is valid.
    if(!model->currentModel){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: NULL Ghoul II model (%s).\n", model->mFileName);
        return qfalse;
    }

    // Start checking the model data and set info along the way.
    // Check if the mesh file header is valid.
    mdxmHeader = (mdxmHeader_t *)model->currentModel->modelData;
    if(!mdxmHeader){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II mesh file has no valid header (%s).\n", model->mFileName);
        return qfalse;
    }
    model->numTransformedVerts = mdxmHeader->numSurfaces;

    // Check if the mesh file contains data.
    if(!mdxmHeader->ofsEnd){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II mesh file is zero-sized (%s).\n", model->mFileName);
        return qfalse;
    }
    model->currentModelSize = mdxmHeader->ofsEnd;

    // Check if the Ghoul II animation file is properly loaded.
    if(!mdxmHeader->animIndex){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model has no associated Ghoul II animation file (%s).\n", model->mFileName);
        return qfalse;
    }
    model->animModel = R_GetModelByHandle(mdxmHeader->animIndex);

    // Check if the animation file header is valid.
    mdxaHeader = (mdxaHeader_t *)model->animModel->modelData;
    if(!mdxaHeader){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II animation file has no valid header (%s).\n", model->mFileName);
        return qfalse;
    }
    model->aHeader = mdxaHeader;

    // Check if the animation file contains data.
    if(!model->aHeader->ofsEnd){
        Com_Printf(S_COLOR_YELLOW "G2_SetModelInfo: Model associated Ghoul II animation file is zero-sized (%s).\n", model->mFileName);
        return qfalse;
    }
    model->currentAnimModelSize = model->aHeader->ofsEnd;

    // Everything checks out.
    model->mValid = qtrue;
    return qtrue;
}

/*
==================
G2_SetupModelPointers

Setup the model if this hasn't been done already.
Returns true if the model is properly setup.
==================
*/

qboolean G2_SetupModelPointers(CGhoul2Model_t *model)
{
    if(!model){
        Com_Printf(S_COLOR_RED "G2_SetupModelPointers: NULL model pointer.\n");
        return qfalse;
    }

    // Invalidate Ghoul II model.
    model->mValid = qfalse;

    // Setup the model, see if everything checks out.
    if(!G2_SetModelInfo(model)){
        // Info isn't valid, invalidate all Ghoul II specifics.
        model->currentModel = 0;
        model->currentModelSize = 0;
        model->numTransformedVerts = 0;
        model->animModel = 0;
        model->currentAnimModelSize = 0;
        model->aHeader = 0;
        return qfalse;
    }

    // All valid.
    return qtrue;
}

/*
==================
G2_IsModelIndexValid

Returns qtrue if the specified
model is valid.

Returns qfalse and prints what
went wrong otherwise.
==================
*/

qboolean G2_IsModelValid(CGhoul2Model_t *model, const char *caller)
{
    if(!model){
        Com_Printf(S_COLOR_RED "%s/G2_IsModelIndexValid: model pointer is NULL.\n", caller);
        return qfalse;
    }

    if(!G2_SetupModelPointers(model)){
        Com_Printf(S_COLOR_RED "%s/G2_IsModelIndexValid: Failed to setup model pointers.\n", caller);
        return qfalse;
    }

    // All valid.
    return qtrue;
}

//=============================================
// Ghoul II matrix routines.
//=============================================

/*
==================
G2_CreateMatrix

Create a matrix using a
set of angles.
==================
*/

void G2_CreateMatrix(mdxaBone_t *matrix, const float *angle)
{
    matrix3_t   axis;

    // Convert angles to axis.
    AnglesToAxis(angle, axis);
    matrix->matrix[0][0] = axis[0][0];
    matrix->matrix[1][0] = axis[0][1];
    matrix->matrix[2][0] = axis[0][2];

    matrix->matrix[0][1] = axis[1][0];
    matrix->matrix[1][1] = axis[1][1];
    matrix->matrix[2][1] = axis[1][2];

    matrix->matrix[0][2] = axis[2][0];
    matrix->matrix[1][2] = axis[2][1];
    matrix->matrix[2][2] = axis[2][2];

    matrix->matrix[0][3] = 0;
    matrix->matrix[1][3] = 0;
    matrix->matrix[2][3] = 0;
}

/*
==================
G2_InverseMatrix

Given a matrix, generate the
inverse of that matrix.
==================
*/

static void G2_InverseMatrix(mdxaBone_t *src, mdxaBone_t *dest)
{
    int i, j;

    for(i = 0; i < 3; i++){
        for(j = 0; j < 3; j++){
            dest->matrix[i][j]=src->matrix[j][i];
        }
    }

    for(i = 0; i < 3; i++){
        dest->matrix[i][3] = 0;
        for(j = 0; j < 3; j++){
            dest->matrix[i][3] -= dest->matrix[i][j] * src->matrix[j][3];
        }
    }
}

/*
==================
G2_GenerateWorldMatrix

Generate the world matrix for a
given set of angles and origin.
==================
*/

void G2_GenerateWorldMatrix(mdxaBone_t *worldMatrix, mdxaBone_t *worldMatrixInv, const vec3_t angles, const vec3_t origin)
{
    // Check if the supplied pointers are valid.
    if(!worldMatrix){
        Com_Printf(S_COLOR_RED "G2_GenerateWorldMatrix: worldMatrix pointer is NULL.\n");
        return;
    }
    if(!worldMatrixInv){
        Com_Printf(S_COLOR_RED "G2_GenerateWorldMatrix: worldMatrixInv pointer is NULL.\n");
        return;
    }

    // Create the initial matrix.
    G2_CreateMatrix(worldMatrix, angles);

    // Update the world matrix with the supplied origin.
    worldMatrix->matrix[0][3] = origin[0];
    worldMatrix->matrix[1][3] = origin[1];
    worldMatrix->matrix[2][3] = origin[2];

    // Generate the inverse of the world matrix.
    G2_InverseMatrix(worldMatrix, worldMatrixInv);
}

/*
==================
G2_Multiply_3x4Matrix

Multiply multiple bone
matrixes into one.
==================
*/

void G2_Multiply_3x4Matrix(mdxaBone_t *out, mdxaBone_t *in2, mdxaBone_t *in)
{
    // First row of out.
    out->matrix[0][0] = (in2->matrix[0][0] * in->matrix[0][0]) + (in2->matrix[0][1] * in->matrix[1][0]) + (in2->matrix[0][2] * in->matrix[2][0]);
    out->matrix[0][1] = (in2->matrix[0][0] * in->matrix[0][1]) + (in2->matrix[0][1] * in->matrix[1][1]) + (in2->matrix[0][2] * in->matrix[2][1]);
    out->matrix[0][2] = (in2->matrix[0][0] * in->matrix[0][2]) + (in2->matrix[0][1] * in->matrix[1][2]) + (in2->matrix[0][2] * in->matrix[2][2]);
    out->matrix[0][3] = (in2->matrix[0][0] * in->matrix[0][3]) + (in2->matrix[0][1] * in->matrix[1][3]) + (in2->matrix[0][2] * in->matrix[2][3]) + in2->matrix[0][3];
    // Second row of out.
    out->matrix[1][0] = (in2->matrix[1][0] * in->matrix[0][0]) + (in2->matrix[1][1] * in->matrix[1][0]) + (in2->matrix[1][2] * in->matrix[2][0]);
    out->matrix[1][1] = (in2->matrix[1][0] * in->matrix[0][1]) + (in2->matrix[1][1] * in->matrix[1][1]) + (in2->matrix[1][2] * in->matrix[2][1]);
    out->matrix[1][2] = (in2->matrix[1][0] * in->matrix[0][2]) + (in2->matrix[1][1] * in->matrix[1][2]) + (in2->matrix[1][2] * in->matrix[2][2]);
    out->matrix[1][3] = (in2->matrix[1][0] * in->matrix[0][3]) + (in2->matrix[1][1] * in->matrix[1][3]) + (in2->matrix[1][2] * in->matrix[2][3]) + in2->matrix[1][3];
    // Third row of out.
    out->matrix[2][0] = (in2->matrix[2][0] * in->matrix[0][0]) + (in2->matrix[2][1] * in->matrix[1][0]) + (in2->matrix[2][2] * in->matrix[2][0]);
    out->matrix[2][1] = (in2->matrix[2][0] * in->matrix[0][1]) + (in2->matrix[2][1] * in->matrix[1][1]) + (in2->matrix[2][2] * in->matrix[2][1]);
    out->matrix[2][2] = (in2->matrix[2][0] * in->matrix[0][2]) + (in2->matrix[2][1] * in->matrix[1][2]) + (in2->matrix[2][2] * in->matrix[2][2]);
    out->matrix[2][3] = (in2->matrix[2][0] * in->matrix[0][3]) + (in2->matrix[2][1] * in->matrix[1][3]) + (in2->matrix[2][2] * in->matrix[2][3]) + in2->matrix[2][3];
}

/*
==================
G2_TransformPoint

Transforms the ray to
model space.
==================
*/

void G2_TransformPoint(const vec3_t in, vec3_t out, mdxaBone_t *mat)
{
    int i;

    for(i = 0; i < 3; i++){
        out[i] = in[0] * mat->matrix[i][0] + in[1] * mat->matrix[i][1] + in[2] * mat->matrix[i][2];
    }
}

/*
==================
G2_TransformTranslatePoint

Transforms and translates
the ray to model space.
==================
*/

void G2_TransformTranslatePoint(const vec3_t in, vec3_t out, mdxaBone_t *mat)
{
    int i;

    for(i = 0; i < 3; i++){
        out[i] = in[0] * mat->matrix[i][0] + in[1] * mat->matrix[i][1] + in[2] * mat->matrix[i][2] + mat->matrix[i][3];
    }
}
