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
// g2_bolts.c - Bolt list handling Ghoul II routines.

#include "g2_local.h"

/*
==================
G2_FindFreeBoltSlot

Returns the first free slot to use in the bolt list,
or -1 if there are no more free slots available.
==================
*/

static int G2_FindFreeBoltSlot(boltInfo_t **boltList, int numBolts)
{
    int i, newIndex;

    // First off, check what index we can use.
    if(numBolts){
        newIndex = -1;
        // Iterate through the list, check which slot is unallocated.
        for(i = 0; i < G2_MAX_BOLTS_IN_LIST; i++){
            if(boltList[i] == NULL){
                newIndex = i;
                break;
            }
        }

        // No free slot found.
        if(newIndex == -1){
            Com_Printf(S_COLOR_RED "G2_FindFreeBoltSlot: Bolt list full! It cannot hold more than %d entries.\n", G2_MAX_BOLTS_IN_LIST);
            return -1;
        }

        return newIndex;
    }

    // No slots allocated yet.
    return 0;
}

/*
==================
G2_AddBolt

Finds and adds a bone or surface to the bolt list.
Returns the new index in the bolt list,
or -1 upon failure.
==================
*/

int G2_AddBolt(CGhoul2Model_t *ghlInfo, const char *boneName)
{
    boltInfo_t          **boltList;
    boltInfo_t          *newBolt;
    mdxaSkelOffsets_t   *offsets;
    mdxaSkel_t          *skel;
    int                 surfNum, boneNum;
    int                 flags;
    int                 i, numValid;
    int                 newIndex;

    boltList = ghlInfo->mBoltList;

    //
    // First up, we'll search for that which this bolt names in all the surfaces.
    //
    surfNum = G2_IsSurfaceLegal(ghlInfo->currentModel, boneName, &flags);

    //
    // Did we find it as a surface?
    //
    if(surfNum != -1){
        if(ghlInfo->numBolts){
            // Look through the existing list to see
            // if it's already there first.
            numValid = 0;
            for(i = 0; i < G2_MAX_BOLTS_IN_LIST; i++){
                // Check if this slot is allocated.
                if(boltList[i] != NULL){
                    // Check if the surface number matches the one we want.
                    if(boltList[i]->surfaceNumber == surfNum){
                        // Increment the usage count.
                        boltList[i]->boltUsed++;

                        return i;
                    }

                    numValid++;
                    if(numValid == ghlInfo->numBolts){
                        // Don't continue if we've iterated
                        // through all allocated slots.
                        break;
                    }
                }
            }

            // Look through the existing list to see if we
            // can re-use a previously allocated slot.
            numValid = 0;
            for(i = 0; i < G2_MAX_BOLTS_IN_LIST; i++){
                // Check if this slot is allocated.
                if(boltList[i] != NULL){
                    // Check if the surface number matches the one we want.
                    if(boltList[i]->boneNumber == -1 && boltList[i]->surfaceNumber == -1){
                        // Empty slot, set and use this slot.
                        boltList[i]->surfaceNumber = surfNum;
                        boltList[i]->boltUsed = 1;
                        boltList[i]->surfaceType = 0;

                        return i;
                    }

                    numValid++;
                    if(numValid == ghlInfo->numBolts){
                        // Don't continue if we've iterated
                        // through all allocated slots.
                        break;
                    }
                }
            }
        }

        // No existing surface of that name
        // OR an empty initialized slot was found.
        // We now allocate and add an entry.
        newIndex = G2_FindFreeBoltSlot(boltList, ghlInfo->numBolts);
        if(newIndex == -1){
            return -1;
        }

        // Allocate this slot.
        ghlInfo->mBoltList[newIndex] = Hunk_Alloc(sizeof(boltInfo_t), h_low);
        ghlInfo->numBolts++;

        // Set the required info.
        newBolt = ghlInfo->mBoltList[newIndex];
        newBolt->surfaceNumber = surfNum;
        newBolt->boneNumber = -1;
        newBolt->boltUsed = 1;
        newBolt->surfaceType = 0;

        return newIndex;
    }

    //
    // Not found as a surface.
    // Check if it is a bone instead.
    //
    offsets = (mdxaSkelOffsets_t *)((byte *)ghlInfo->aHeader + sizeof(mdxaHeader_t));

    //
    // Walk the entire list of bones in the Ghoul II animation file
    // for this model and see if any match the name of the bone
    // we want to find.
    //
    boneNum = -1;
    for(i = 0; i < ghlInfo->aHeader->numBones; i++){
        skel = (mdxaSkel_t *)((byte *)ghlInfo->aHeader + sizeof(mdxaHeader_t) + offsets->offsets[i]);
        // If the name is the same, we found it.
        if(Q_stricmp(skel->name, boneName) == 0){
            boneNum = i;
            break;
        }
    }

    // Check if we found the bone we're looking for.
    if(boneNum == -1){
        #ifdef _DEBUG
        Com_Printf(S_COLOR_YELLOW "G2_AddBolt: %s not found on skeleton.\n", boneName);
        #endif // _DEBUG

        return -1;
    }

    if(ghlInfo->numBolts){
        // Look through the existing list to see
        // if it's already there first.
        numValid = 0;
        for(i = 0; i < G2_MAX_BOLTS_IN_LIST; i++){
            // Check if this slot is allocated.
            if(boltList[i] != NULL){
                // Check if the bone number matches the one we want.
                if(boltList[i]->boneNumber == boneNum){
                    // Increment the usage count.
                    boltList[i]->boltUsed++;

                    return i;
                }

                numValid++;
                if(numValid == ghlInfo->numBolts){
                    // Don't continue if we've iterated
                    // through all allocated slots.
                    break;
                }
            }
        }

        // Look through the existing list to see if we
        // can re-use a previously allocated slot.
        numValid = 0;
        for(i = 0; i < G2_MAX_BOLTS_IN_LIST; i++){
            // Check if this slot is allocated.
            if(boltList[i] != NULL){
                // Check if the surface number matches the one we want.
                if(boltList[i]->boneNumber == -1 && boltList[i]->surfaceNumber == -1){
                    // Empty slot, set and use this slot.
                    boltList[i]->boneNumber = boneNum;
                    boltList[i]->boltUsed = 1;
                    boltList[i]->surfaceType = 0;

                    return i;
                }

                numValid++;
                if(numValid == ghlInfo->numBolts){
                    // Don't continue if we've iterated
                    // through all allocated slots.
                    break;
                }
            }
        }
    }

    // No existing bone of that name
    // OR an empty initialized slot was found.
    // We now allocate and add an entry.
    newIndex = G2_FindFreeBoltSlot(boltList, ghlInfo->numBolts);
    if(newIndex == -1){
        return -1;
    }

    // Allocate this slot.
    ghlInfo->mBoltList[newIndex] = Hunk_Alloc(sizeof(boltInfo_t), h_low);
    ghlInfo->numBolts++;

    // Set the required info.
    newBolt = ghlInfo->mBoltList[newIndex];
    newBolt->boneNumber = boneNum;
    newBolt->surfaceNumber = -1;
    newBolt->boltUsed = 1;
    newBolt->surfaceType = 0;

    return newIndex;
}
