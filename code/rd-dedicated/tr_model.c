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
// tr_model.c - Server-side model routines.

#include "tr_local.h"

#define LL(x) x=LittleLong(x)

// Local function definitions.
static qboolean     R_LoadMDXA              ( model_t *mod, void *buffer, int bufferSize, const char *modName );
static qboolean     R_LoadMDXM              ( model_t *mod, void *buffer, int bufferSize, const char *modName );

// Local variable definitions.
static char         sPrevMapName[MAX_QPATH]         = { 0 };
static int          giRegisterMedia_CurrentLevel    = 0;

//=============================================================================

/*
==================
RE_RegisterMedia_LevelLoadBegin


==================
*/

void RE_RegisterMedia_LevelLoadBegin(const char *psMapName)
{
    tr.numBSPModels = 0;

    // Only bump level number if we're not on the same level.
    if(Q_stricmp(psMapName,sPrevMapName)){
        Q_strncpyz(sPrevMapName, psMapName, sizeof(sPrevMapName));
        giRegisterMedia_CurrentLevel++;
    }
}

/*
==================
R_ModelInit

Clears models and (re-)initializes the main NULL model.
==================
*/

void R_ModelInit()
{
    model_t     *mod;

    // Leave a space for NULL model.
    tr.numModels = 0;

    mod = R_AllocModel();
    mod->type = MOD_BAD;
}

//=============================================

/*
==================
R_GetModelByHandle

Returns existing model in the list,
or the default model if out of range.
==================
*/

model_t *R_GetModelByHandle(qhandle_t index)
{
    model_t     *mod;

    // Out of range returns the default model.
    if(index < 1 || index >= tr.numModels){
        return tr.models[0];
    }

    mod = tr.models[index];
    return mod;
}

/*
==================
R_AllocModel

Allocates model in the model list.
Returns pointer to new member.
==================
*/

model_t *R_AllocModel()
{
    model_t     *mod;

    if(tr.numModels == MAX_MOD_KNOWN){
        return NULL;
    }

    mod = Hunk_Alloc(sizeof(*tr.models[tr.numModels]), h_low);
    mod->index = tr.numModels;
    tr.models[tr.numModels] = mod;
    tr.numModels++;

    return mod;
}

/*
==================
RE_RegisterServerModel

Loads in a model for the given name.

Zero will be returned if the model fails to load.
An entry will be retained for failed models as an
optimization to prevent disk rescanning if they are
asked for again.
==================
*/

qhandle_t RE_RegisterServerModel(const char *name)
{
    model_t     *mod;
    qhandle_t   hModel;
    int         ident;
    int         filesize;
    char        localName[MAX_QPATH];
    qboolean    loaded = qfalse;
    union {
        unsigned int    *u;
        void            *v;
    } buf;

    // Must be a valid name.
    if(!name || !name[0]){
        Com_Printf(S_COLOR_RED "RE_RegisterServerModel: NULL name\n");
        return 0;
    }
    if(strlen(name) >= MAX_QPATH){
        Com_Printf(S_COLOR_RED "RE_RegisterServerModel: Model name exceeds MAX_QPATH\n");
        return 0;
    }

    //
    // Search the currently loaded models.
    //
   for(hModel = 1 ; hModel < tr.numModels; hModel++){
        mod = tr.models[hModel];
        if(!strcmp( mod->name, name)){
            if(mod->type == MOD_BAD){
                return 0;
            }
            return hModel;
        }
    }

    // Allocate a new model_t.
    if((mod = R_AllocModel()) == NULL){
        Com_Printf(S_COLOR_YELLOW "RE_RegisterServerModel: R_AllocModel() failed for \"%s\".\n", name);
        return 0;
    }

    // Only set the name after the model has been successfully loaded.
    Q_strncpyz(mod->name, name, sizeof(mod->name));

    mod->type = MOD_BAD;
    mod->numLods = 0;

    //
    // Load the files.
    //
    Q_strncpyz(localName, name, MAX_QPATH);

    // Try to load the file.
    filesize = FS_ReadFile(name, (void **)&buf.v);
    if(!buf.u){
        // Not loaded, try again but without extension.
        COM_StripExtension(name, localName, MAX_QPATH);
        filesize = FS_ReadFile(name, (void **)&buf.v);

        if(!buf.u){
            // No way to load the file.
            Com_Printf(S_COLOR_YELLOW "RE_RegisterServerModel: \"%s\" not present.\n", name);
            return hModel;
        }else{
            // Loaded, but not ideal.
            Com_Printf(S_COLOR_YELLOW "RE_RegisterServerModel: WARNING: \"%s\" not present, using \"%s\" instead.", name, localName);
        }
    }

    // Determine file type.
    ident = LittleLong(*(unsigned int*)buf.u);
    if(ident == MDXA_IDENT){
        loaded = R_LoadMDXA(mod, buf.u, filesize, name);
    }else if(ident == MDXM_IDENT){
        loaded = R_LoadMDXM(mod, buf.u, filesize, name);
    }

    if(!loaded){
        Com_Printf(S_COLOR_YELLOW "RE_RegisterServerModel: Couldn't load file \"%s\".\n", name);
    }

    FS_FreeFile(buf.v);

    return mod->index;
}

//=============================================
// Actual model file load routines.
//=============================================

/*
==================
R_LoadMDXA

Load a Ghoul II animation file.
==================
*/

static qboolean R_LoadMDXA(model_t *mod, void *buffer, int bufferSize, const char *modName)
{
    mdxaHeader_t        *mdxaHeader, *mdxa;
    int                 version;
    int                 size;

    mdxaHeader = (mdxaHeader_t *)buffer;

    //
    // Read some fields from the binary.
    //
    version = LittleLong(mdxaHeader->version);
    if(version != MDXA_VERSION){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXA: \"%s\" has wrong version (%d should be %d).\n", modName, version, MDXA_VERSION);
        return qfalse;
    }
    size = LittleLong(mdxaHeader->ofsEnd);
    if(size > bufferSize){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXA: Header of \"%s\" is broken. Wrong filesize declared (%d should be %d).\n", modName, size, bufferSize);
    }

    mod->type = MOD_MDXA;
    mod->dataSize += size;
    mod->modelData = mdxa = Hunk_Alloc(size, h_low);

    // Copy all the values over from the file.
    Com_Memcpy(mdxa, mdxaHeader, sizeof(mdxaHeader_t));
    LL(mdxa->ident);
    LL(mdxa->version);
    LL(mdxa->numFrames);
    LL(mdxa->numBones);
    LL(mdxa->ofsFrames);
    LL(mdxa->ofsEnd);

    if(mdxa->numFrames < 1){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXA: \"%s\" has no frames.\n", modName);
        return qfalse;
    }

    return qtrue;
}

/*
==================
R_LoadMDXM

Load a Ghoul II mesh file.
==================
*/

static qboolean R_LoadMDXM(model_t *mod, void *buffer, int bufferSize, const char *modName)
{
    mdxmHeader_t        *mdxmHeader, *mdxm;
    mdxmLOD_t           *lod;
    mdxmSurface_t       *surf;
    int                 version;
    int                 size;
    int                 i, l;

    mdxmHeader = (mdxmHeader_t *)buffer;

    //
    // Read some fields from the binary.
    //
    version = LittleLong(mdxmHeader->version);
    if(version != MDXM_VERSION){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has wrong version (%d should be %d).\n", modName, version, MDXM_VERSION);
        return qfalse;
    }
    size = LittleLong(mdxmHeader->ofsEnd);
    if(size > bufferSize){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: Header of \"%s\" is broken. Wrong filesize declared (%d should be %d).\n", modName, size, bufferSize);
    }

    mod->type = MOD_MDXM;
    mod->dataSize += size;
    mod->modelData = mdxm = Hunk_Alloc(size, h_low);

    // Copy the buffer contents.
    Com_Memcpy(mod->modelData, buffer, size);
    LL(mdxm->ident);
    LL(mdxm->version);
    LL(mdxm->numLODs);
    LL(mdxm->ofsLODs);
    LL(mdxm->numSurfaces);
    LL(mdxm->ofsSurfHierarchy);
    LL(mdxm->ofsEnd);

    // First up, go load in the animation file we need that has the skeletal animation info for this model.
    mdxm->animIndex = RE_RegisterServerModel(va("%s.gla", mdxm->animName));
    if(!mdxm->animIndex){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has no skeletal animation info.\n", modName);
        return qfalse;
    }

    //
    // Swap all the LOD's.
    //
    lod = (mdxmLOD_t *)((byte *)mdxm + mdxm->ofsLODs);
    for(l = 0 ; l < mdxm->numLODs ; l++){
        int triCount = 0;

        LL(lod->ofsEnd);

        // Swap all the surfaces.
        surf = (mdxmSurface_t *)((byte *)lod + sizeof (mdxmLOD_t) + (mdxm->numSurfaces * sizeof(mdxmLODSurfOffset_t)));
        for(i = 0 ; i < mdxm->numSurfaces ; i++){
            LL(surf->numTriangles);
            LL(surf->ofsTriangles);
            LL(surf->numVerts);
            LL(surf->ofsVerts);
            LL(surf->ofsEnd);
            LL(surf->ofsHeader);
            LL(surf->numBoneReferences);
            LL(surf->ofsBoneReferences);

            triCount += surf->numTriangles;

            if(surf->numVerts > SHADER_MAX_VERTEXES){
                Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has more than %d verts on a surface (%d).\n",
                    modName, SHADER_MAX_VERTEXES - 1, surf->numVerts);

                return qfalse;
            }
            if(surf->numTriangles*3 > SHADER_MAX_INDEXES){
                Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has more than %d triangles on a surface (%d).\n",
                    modName, (SHADER_MAX_INDEXES / 3) - 1, surf->numTriangles);

                return qfalse;
            }

            // Find the next surface.
            surf = (mdxmSurface_t *)((byte *)surf + surf->ofsEnd);
        }

        // Find the next LOD.
        lod = (mdxmLOD_t *)((byte *)lod + lod->ofsEnd);
    }

    return qtrue;
}
