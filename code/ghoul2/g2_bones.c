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
