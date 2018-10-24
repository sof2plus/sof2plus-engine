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
// tr_model.c - Server-side model functions.

#include "tr_local.h"

#define LL(x) x=LittleLong(x)
#define LF(x) x=LittleFloat(x)
#define LS(x) x=LittleShort(x)

// Local function definitions.
static qboolean     R_LoadMDXA              ( model_t *mod, void *buffer, int bufferSize, const char *modName );
static qboolean     R_LoadMDXM              ( model_t *mod, void *buffer, int bufferSize, const char *modName );

//=============================================================================

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
    if(index < 0 || index >= tr.numModels){
        return NULL;
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

    mod = Z_TagMalloc(sizeof(model_t), TAG_RENDERER);
    Com_Memset(mod, 0, sizeof(model_t));

    mod->index = tr.numModels;
    tr.models[tr.numModels] = mod;
    tr.numModels++;

    return mod;
}

/*
==================
RE_RegisterServerModel

Loads in a model for the given name.

-1 will be returned if the model fails to load,
to indicate an error has occurred.
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
        return -1;
    }
    if(strlen(name) >= MAX_QPATH){
        Com_Printf(S_COLOR_RED "RE_RegisterServerModel: Model name exceeds MAX_QPATH\n");
        return -1;
    }

   //
   // Search the currently loaded models.
   //
   for(hModel = 0; hModel < tr.numModels; hModel++){
        mod = tr.models[hModel];
        if(!Q_stricmp(mod->name, name)){
            if(mod->type == MOD_BAD){
                return -1;
            }
            return hModel;
        }
    }

    // Allocate a new model.
    if((mod = R_AllocModel()) == NULL){
        Com_Printf(S_COLOR_YELLOW "RE_RegisterServerModel: R_AllocModel() failed for \"%s\".\n", name);
        return -1;
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

/*
=============================================
--------------------------
Model file load functions.
--------------------------
=============================================
*/

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
#ifdef Q3_BIG_ENDIAN
    mdxaSkelOffsets_t   *skelOffsets;
    mdxaSkel_t          *skel;
    mdxaCompQuatBone_t  *pCompBonePool;
    unsigned short      *pwIn;
    int                 maxBoneIndex;
    int                 i, j, k;
#endif // Q3_BIG_ENDIAN

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
    mod->modelData = mdxa = Z_TagMalloc(size, TAG_RENDERER);

    // Copy all the values over from the file.
    Com_Memcpy(mod->modelData, buffer, size);

    // Animation file must contain frames.
    LL(mdxa->numFrames);
    if(mdxa->numFrames < 1){
        Com_Printf(S_COLOR_YELLOW "R_LoadMDXA: \"%s\" has no frames.\n", modName);
        return qfalse;
    }

#ifdef Q3_BIG_ENDIAN
    //
    // Swap remaining header information.
    //
    LL(mdxa->ident);
    LL(mdxa->version);
    LL(mdxa->ofsFrames);
    LL(mdxa->numBones);
    LL(mdxa->ofsCompBonePool);
    LL(mdxa->ofsEnd);

    //
    // Swap the skeleton information.
    //
    skelOffsets     = (mdxaSkelOffsets_t *)((byte *)mdxa + sizeof(mdxaHeader_t));
    for(i = 0; i < mdxa->numBones; i++){
        // Swap the offset.
        LL(skelOffsets->offsets[i]);

        // Get skeleton.
        skel = (mdxaSkel_t *)((byte *)mdxa + sizeof(mdxaHeader_t) + skelOffsets->offsets[i]);

        // Swap base info.
        LL(skel->flags);
        LL(skel->parent);
        LL(skel->numChildren);

        // Swap positional info.
        for(j = 0; j < 3; j++){
            for(k = 0; k < 4; k++){
                LF(skel->BasePoseMat.matrix[j][k]);
                LF(skel->BasePoseMatInv.matrix[j][k]);
            }
        }

        // Swap child array.
        for(j = 0; j < skel->numChildren; j++){
            LL(skel->children[j]);
        }
    }

    //
    // Swap the compressed bones.
    //
    maxBoneIndex    = (mdxa->ofsEnd - mdxa->ofsCompBonePool) / sizeof(mdxaCompQuatBone_t);
    pCompBonePool   = (mdxaCompQuatBone_t *)((byte *)mdxa + mdxa->ofsCompBonePool);
    for(i = 0; i < maxBoneIndex; i++){
        pwIn = (unsigned short *)((byte *)pCompBonePool[i].Comp);

        for(j = 0; j < 7; j++){
            LS(pwIn[j]);
        }
    }
#endif // Q3_BIG_ENDIAN

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
    mdxmHeader_t            *mdxmHeader, *mdxm;
    mdxmLOD_t               *lod;
    mdxmSurface_t           *surf;
    int                     version;
    int                     size;
    int                     i, l;
#ifdef Q3_BIG_ENDIAN
    mdxmSurfHierarchy_t     *surfInfo;
    mdxmHierarchyOffsets_t  *surfIndexes;
    mdxmLODSurfOffset_t     *lodIndexes;
    mdxmTriangle_t          *tris;
    mdxmVertex_t            *verts;
    mdxmVertexTexCoord_t    *pTexCoords;
    int                     *piBoneReferences;
    int                     j;
#endif // Q3_BIG_ENDIAN

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
    mod->modelData = mdxm = Z_TagMalloc(size, TAG_RENDERER);

    // Copy the buffer contents.
    Com_Memcpy(mod->modelData, buffer, size);

#ifdef Q3_BIG_ENDIAN
    // Swap remaining header information.
    LL(mdxm->ident);
    LL(mdxm->version);
    LL(mdxm->numLODs);
    LL(mdxm->ofsLODs);
    LL(mdxm->numSurfaces);
    LL(mdxm->ofsSurfHierarchy);
    LL(mdxm->ofsEnd);
#endif // Q3_BIG_ENDIAN

    // Store how many LODs this model has.
    mod->numLods = mdxm->numLODs;

    // First up, go load in the animation file we need that has the skeletal animation info for this model.
    // Try the MP animation file variant first.
    mdxm->animIndex = RE_RegisterServerModel(va("%s_mp.gla", mdxm->animName));
    if(mdxm->animIndex < 0){
        // Not found, is the regular animation file present?
        mdxm->animIndex = RE_RegisterServerModel(va("%s.gla", mdxm->animName));

        if(mdxm->animIndex < 0){
            Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has no skeletal animation info.\n", modName);
            return qfalse;
        }
    }

#ifdef Q3_BIG_ENDIAN
    //
    // Swap the surface information.
    //
    surfInfo    = (mdxmSurfHierarchy_t *)((byte *)mdxm + mdxm->ofsSurfHierarchy);
    surfIndexes = (mdxmHierarchyOffsets_t *)((byte *)mdxm + sizeof(mdxmHeader_t));
    for(i = 0; i < mdxm->numSurfaces; i++){
        // Swap base info.
        LL(surfInfo->numChildren);
        LL(surfInfo->parentIndex);

        // Swap all children indexes.
        for(j = 0; j < surfInfo->numChildren; j++){
            LL(surfInfo->childIndexes[j]);
        }

        // Swap the surface offset.
        LL(surfIndexes->offsets[i]);

        // Find the next surface.
        surfInfo = (mdxmSurfHierarchy_t *)((byte *)surfInfo + (size_t)(&((mdxmSurfHierarchy_t *)0)->childIndexes[surfInfo->numChildren]));
    }
#endif // Q3_BIG_ENDIAN

    //
    // Swap all the LOD's.
    //
    lod = (mdxmLOD_t *)((byte *)mdxm + mdxm->ofsLODs);
    for(l = 0 ; l < mdxm->numLODs ; l++){
        int triCount = 0;

#ifdef Q3_BIG_ENDIAN
        LL(lod->ofsEnd);
#endif // Q3_BIG_ENDIAN

        // Swap all the surfaces.
        surf = (mdxmSurface_t *)((byte *)lod + sizeof(mdxmLOD_t) + (mdxm->numSurfaces * sizeof(mdxmLODSurfOffset_t)));
        for(i = 0 ; i < mdxm->numSurfaces ; i++){
#ifdef Q3_BIG_ENDIAN
            LL(surf->thisSurfaceIndex);
            LL(surf->numTriangles);
            LL(surf->ofsTriangles);
            LL(surf->numVerts);
            LL(surf->ofsVerts);
            LL(surf->ofsEnd);
            LL(surf->ofsHeader);
            LL(surf->numBoneReferences);
            LL(surf->ofsBoneReferences);
#endif // Q3_BIG_ENDIAN

            triCount += surf->numTriangles;

            if(surf->numVerts > SHADER_MAX_VERTEXES){
                Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has more than %d verts on a surface (%d).\n",
                    modName, SHADER_MAX_VERTEXES - 1, surf->numVerts);

                return qfalse;
            }
            if(surf->numTriangles * 3 > SHADER_MAX_INDEXES){
                Com_Printf(S_COLOR_YELLOW "R_LoadMDXM: \"%s\" has more than %d triangles on a surface (%d).\n",
                    modName, (SHADER_MAX_INDEXES / 3) - 1, surf->numTriangles);

                return qfalse;
            }

#ifdef Q3_BIG_ENDIAN
            // Swap the LOD offset.
            lodIndexes  = (mdxmLODSurfOffset_t *)((byte *)lod + sizeof(mdxmLOD_t));
            LL(lodIndexes->offsets[surf->thisSurfaceIndex]);

            // Swap the bone reference data.
            piBoneReferences = (int *)((byte *)surf + surf->ofsBoneReferences);
            for(j = 0; j < surf->numBoneReferences; j++){
                LL(piBoneReferences[j]);
            }

            // Swap the triangles.
            tris        = (mdxmTriangle_t *)((byte *)surf + surf->ofsTriangles);
            for(j = 0; j < surf->numTriangles; j++){
                LL(tris->indexes[0]);
                LL(tris->indexes[1]);
                LL(tris->indexes[2]);

                tris++;
            }

            // Swap the vertexes.
            verts       = (mdxmVertex_t *)((byte *)surf + surf->ofsVerts);
            pTexCoords  = (mdxmVertexTexCoord_t *)&verts[surf->numVerts];
            for(j = 0; j < surf->numVerts; j++){
                LF(verts->normal[0]);
                LF(verts->normal[1]);
                LF(verts->normal[2]);

                LF(verts->vertCoords[0]);
                LF(verts->vertCoords[1]);
                LF(verts->vertCoords[2]);

                LF(pTexCoords[j].texCoords[0]);
                LF(pTexCoords[j].texCoords[1]);

                LL(verts->uiNmWeightsAndBoneIndexes);

                verts++;
            }
#endif // Q3_BIG_ENDIAN

            // Find the next surface.
            surf = (mdxmSurface_t *)((byte *)surf + surf->ofsEnd);
        }

        // Find the next LOD.
        lod = (mdxmLOD_t *)((byte *)lod + lod->ofsEnd);
    }

    return qtrue;
}
