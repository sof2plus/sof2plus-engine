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
// tr_skin.c - Skin registration functions.

#include "tr_local.h"

/*
==================
R_AllocSkin

Allocates skin in the skin list.
Returns pointer to new member.
==================
*/

static skin_t *R_AllocSkin(void)
{
    skin_t  *skin;

    if(tr.numSkins == MAX_SKINS){
        return NULL;
    }

    skin = Z_TagMalloc(sizeof(skin_t), TAG_RENDERER);
    Com_Memset(skin, 0, sizeof(skin_t));
    tr.skins[tr.numSkins] = skin;
    tr.numSkins++;

    return skin;
}

/*
==================
RE_RegisterServerSkin

Loads in a skin with the given name.
The function expects a space separated
skin pair list, with the number of pairs
defined up-front.

Zero will be returned if the skin fails to load.
==================
*/

qhandle_t RE_RegisterServerSkin(const char *name, int numPairs, const char *skinPairs)
{
    skin_t          *skin;
    qhandle_t       hSkin;
    skinSurface_t   *surf;
    int             i, len;
    char            surfName[MAX_QPATH];
    char            surfShader[MAX_QPATH];
    char            cSkinPairs[20480];
    char            *pairs, *sName, *sShader;

    // Must be a valid name.
    if(!name || !name[0]){
        Com_Printf(S_COLOR_RED "RE_RegisterServerSkin: NULL name.\n");
        return 0;
    }
    if(strlen(name) >= MAX_QPATH){
        Com_Printf(S_COLOR_RED "RE_RegisterServerSkin: Model name exceeds MAX_QPATH.\n");
        return 0;
    }

    // Do we have a valid skin pair list?
    if(numPairs == 0 || skinPairs == NULL || !strlen(skinPairs) || !strstr(skinPairs, " ")){
        Com_Printf(S_COLOR_RED "RE_RegisterServerSkin: Invalid skin pair list.\n");
        return 0;
    }

    //
    // Search the currently loaded skins.
    //
    for(hSkin = 0; hSkin < tr.numSkins; hSkin++){
        skin = tr.skins[hSkin];
        if(!Q_stricmp(skin->name, name)){
            // Skin found. Does this skin contain surfaces?
            if(skin->numSurfaces == 0){
                // It doesn't, return the default skin.
                return 0;
            }

            // Found the skin we're trying to load.
            return hSkin;
        }
    }

    //
    // Allocate a new skin.
    //
    if((skin = R_AllocSkin()) == NULL){
        Com_Printf(S_COLOR_YELLOW "RE_RegisterServerSkin: R_AllocSkin() failed for \"%s\".\n", name);
        return 0;
    }

    // Set info after the skin has been successfully initialized.
    Q_strncpyz(skin->name, name, sizeof(skin->name));
    skin->numSurfaces = 0;

    //
    // Walk through the skin pairs.
    //
    Q_strncpyz(cSkinPairs, skinPairs, sizeof(cSkinPairs));
    pairs = cSkinPairs;

    for(i = 0; i < numPairs; i++){
        // Determine the surface name and shader name from the pair list.
        sName           = pairs;
        sShader         = strstr(sName, " ");

        // Were they correctly found?
        if(sName == NULL || sShader == NULL){
            // No, we're dealing with an invalid list here.
            Com_Printf(S_COLOR_YELLOW "RE_RegisterServerSkin: WARNING: Mangled skin pair list, parsed %d out of %d skin pairs successfully.\n", i, numPairs);
            break;
        }
        sShader++;

        // Determine where the next pair starts.
        pairs = strstr(sShader, " ");

        // Store the results.
        // Surface name.
        len = sShader - sName;
        Q_strncpyz(surfName, sName, len < sizeof(surfName) ? len : sizeof(surfName));

        // Surface shader.
        if(pairs != NULL){
            len = pairs - sShader + 1;
            pairs++;
        }else{
            len = strlen(sShader);
        }
        Q_strncpyz(surfShader, sShader, len < sizeof(surfShader) ? len : sizeof(surfShader));

        // Allocate a new surface.
        surf = skin->surfaces[skin->numSurfaces] = Z_TagMalloc(sizeof(skinSurface_t), TAG_RENDERER);
        skin->numSurfaces++;

        // Set surface name and shader.
        Q_strncpyz(surf->name, surfName, sizeof(surf->name));
        surf->shader = R_FindServerShader(surfShader);

        if(pairs == NULL || skin->numSurfaces == MAX_SKIN_SURFACES){
            break;
        }
    }

    // Return the handle to this skin.
    return hSkin;
}

/*
==================
R_GetSkinByHandle

Returns pointer to skin structure
based on skin handle.
==================
*/

skin_t *R_GetSkinByHandle(qhandle_t index)
{
    if(index < 0 || index >= tr.numSkins){
        return NULL;
    }

    return tr.skins[index];
}
