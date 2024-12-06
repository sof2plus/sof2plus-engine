/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cmodel.c -- model loading

#include "cm_local.h"

#ifdef BSPC

#include "../bspc/l_qfiles.h"

void SetPlaneSignbits (cplane_t *out) {
    int bits, j;

    // for fast box on planeside test
    bits = 0;
    for (j=0 ; j<3 ; j++) {
        if (out->normal[j] < 0) {
            bits |= 1<<j;
        }
    }
    out->signbits = bits;
}
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
#define BOX_BRUSHES     1
#define BOX_SIDES       6
#define BOX_LEAFS       2
#define BOX_PLANES      12

#define LL(x) x=LittleLong(x)

#define MAX_CLIPMAP_BSP (MAX_SUB_BSP + 1)


clipMap_t   cmBSPs[MAX_CLIPMAP_BSP];    // Clipmap of the main BSP and any sub-BSPs.
clipMap_t   *cmg = &cmBSPs[0];          // The clipmap of the main BSP.

int         c_pointcontents;
int         c_traces, c_brush_traces, c_patch_traces;

int         totalSubModels;             // Total amount of models present in the world.

byte        *cmod_base;

#ifndef BSPC
cvar_t      *cm_noAreas;
cvar_t      *cm_noCurves;
cvar_t      *cm_playerCurveClip;
#endif

cmodel_t    box_model;
cplane_t    *box_planes;
cbrush_t    *box_brush;



void    CM_InitBoxHull (void);
void    CM_FloodAreaConnections (clipMap_t *cm);


/*
===============================================================================

                    MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders(clipMap_t *cm, lump_t *l)
{
    dshader_t   *in, *out;
    int         i, count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in)) {
        Com_Error (ERR_DROP, "CMod_LoadShaders: funny lump size");
    }
    count = l->filelen / sizeof(*in);

    if (count < 1) {
        Com_Error (ERR_DROP, "Map with no shaders");
    }
    cm->shaders = Hunk_Alloc( count * sizeof( *cm->shaders ), h_high );
    cm->numShaders = count;

    Com_Memcpy( cm->shaders, in, count * sizeof( *cm->shaders ) );

    out = cm->shaders;
    for ( i=0 ; i<count ; i++, in++, out++ ) {
        out->contentFlags = LittleLong( out->contentFlags );
        out->surfaceFlags = LittleLong( out->surfaceFlags );
    }
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels(clipMap_t *cm, lump_t *l)
{
    dmodel_t    *in;
    cmodel_t    *out;
    int         i, j, count;
    int         *indexes;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "CMod_LoadSubmodels: funny lump size");
    count = l->filelen / sizeof(*in);

    if (count < 1)
        Com_Error (ERR_DROP, "Map with no models");
    cm->cmodels = Hunk_Alloc( count * sizeof( *cm->cmodels ), h_high );
    cm->numSubModels = count;

    if ( count > MAX_SUBMODELS ) {
        Com_Error( ERR_DROP, "MAX_SUBMODELS exceeded" );
    }

    for ( i=0 ; i<count ; i++, in++)
    {
        out = &cm->cmodels[i];

        for (j=0 ; j<3 ; j++)
        {   // spread the mins / maxs by a pixel
            out->mins[j] = LittleFloat (in->mins[j]) - 1;
            out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
        }

        // FIXME BOE: Review if a "cm == cmg" check is required here,
        // similar to JK:JA.
        if ( i == 0 ) {
            out->firstNode = 0;
            continue;   // world model doesn't need other info
        }
        out->firstNode = -1;

        // make a "leaf" just to hold the model's brushes and surfaces
        out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
        indexes = Hunk_Alloc( out->leaf.numLeafBrushes * 4, h_high );
        out->leaf.firstLeafBrush = indexes - cm->leafbrushes;
        for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
            indexes[j] = LittleLong( in->firstBrush ) + j;
        }

        out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
        indexes = Hunk_Alloc( out->leaf.numLeafSurfaces * 4, h_high );
        out->leaf.firstLeafSurface = indexes - cm->leafsurfaces;
        for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
            indexes[j] = LittleLong( in->firstSurface ) + j;
        }
    }
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes(clipMap_t *cm, lump_t *l)
{
    dnode_t     *in;
    int         child;
    cNode_t     *out;
    int         i, j, count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    count = l->filelen / sizeof(*in);

    if (count < 1)
        Com_Error (ERR_DROP, "Map has no nodes");
    cm->nodes = Hunk_Alloc( count * sizeof( *cm->nodes ), h_high );
    cm->numNodes = count;

    out = cm->nodes;

    for (i=0 ; i<count ; i++, out++, in++)
    {
        out->plane = cm->planes + LittleLong( in->planeNum );
        for (j=0 ; j<2 ; j++)
        {
            child = LittleLong (in->children[j]);
            out->children[j] = child;
        }
    }

}

/*
=================
CM_BoundBrush

=================
*/
void CM_BoundBrush( cbrush_t *b ) {
    b->bounds[0][0] = -b->sides[0].plane->dist;
    b->bounds[1][0] = b->sides[1].plane->dist;

    b->bounds[0][1] = -b->sides[2].plane->dist;
    b->bounds[1][1] = b->sides[3].plane->dist;

    b->bounds[0][2] = -b->sides[4].plane->dist;
    b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes(clipMap_t *cm, lump_t *l)
{
    dbrush_t    *in;
    cbrush_t    *out;
    int         i, count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in)) {
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    }
    count = l->filelen / sizeof(*in);

    cm->brushes = Hunk_Alloc( ( BOX_BRUSHES + count ) * sizeof( *cm->brushes ), h_high );
    cm->numBrushes = count;

    out = cm->brushes;

    for ( i=0 ; i<count ; i++, out++, in++ ) {
        out->sides = cm->brushsides + LittleLong(in->firstSide);
        out->numsides = LittleLong(in->numSides);

        out->shaderNum = LittleLong( in->shaderNum );
        if ( out->shaderNum < 0 || out->shaderNum >= cm->numShaders ) {
            Com_Error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
        }
        out->contents = cm->shaders[out->shaderNum].contentFlags;

        CM_BoundBrush( out );
    }

}

/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs(clipMap_t *cm, lump_t *l)
{
    int         i;
    cLeaf_t     *out;
    dleaf_t     *in;
    int         count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    count = l->filelen / sizeof(*in);

    if (count < 1)
        Com_Error (ERR_DROP, "Map with no leafs");

    cm->leafs = Hunk_Alloc( ( BOX_LEAFS + count ) * sizeof( *cm->leafs ), h_high );
    cm->numLeafs = count;

    out = cm->leafs;
    for ( i=0 ; i<count ; i++, in++, out++)
    {
        out->cluster = LittleLong (in->cluster);
        out->area = LittleLong (in->area);
        out->firstLeafBrush = LittleLong (in->firstLeafBrush);
        out->numLeafBrushes = LittleLong (in->numLeafBrushes);
        out->firstLeafSurface = LittleLong (in->firstLeafSurface);
        out->numLeafSurfaces = LittleLong (in->numLeafSurfaces);

        if (out->cluster >= cm->numClusters)
            cm->numClusters = out->cluster + 1;
        if (out->area >= cm->numAreas)
            cm->numAreas = out->area + 1;
    }

    cm->areas = Hunk_Alloc( cm->numAreas * sizeof( *cm->areas ), h_high );
    cm->areaPortals = Hunk_Alloc( cm->numAreas * cm->numAreas * sizeof( *cm->areaPortals ), h_high );
}

/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes(clipMap_t *cm, lump_t *l)
{
    int         i, j;
    cplane_t    *out;
    dplane_t    *in;
    int         count;
    int         bits;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    count = l->filelen / sizeof(*in);

    if (count < 1)
        Com_Error (ERR_DROP, "Map with no planes");
    cm->planes = Hunk_Alloc( ( BOX_PLANES + count ) * sizeof( *cm->planes ), h_high );
    cm->numPlanes = count;

    out = cm->planes;

    for ( i=0 ; i<count ; i++, in++, out++)
    {
        bits = 0;
        for (j=0 ; j<3 ; j++)
        {
            out->normal[j] = LittleFloat (in->normal[j]);
            if (out->normal[j] < 0)
                bits |= 1<<j;
        }

        out->dist = LittleFloat (in->dist);
        out->type = PlaneTypeForNormal( out->normal );
        out->signbits = bits;
    }
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes(clipMap_t *cm, lump_t *l)
{
    int         i;
    int         *out;
    int         *in;
    int         count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    count = l->filelen / sizeof(*in);

    cm->leafbrushes = Hunk_Alloc( (count + BOX_BRUSHES) * sizeof( *cm->leafbrushes ), h_high );
    cm->numLeafBrushes = count;

    out = cm->leafbrushes;

    for ( i=0 ; i<count ; i++, in++, out++) {
        *out = LittleLong (*in);
    }
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces(clipMap_t *cm, lump_t *l)
{
    int         i;
    int         *out;
    int         *in;
    int         count;

    in = (void *)(cmod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    count = l->filelen / sizeof(*in);

    cm->leafsurfaces = Hunk_Alloc( count * sizeof( *cm->leafsurfaces ), h_high );
    cm->numLeafSurfaces = count;

    out = cm->leafsurfaces;

    for ( i=0 ; i<count ; i++, in++, out++) {
        *out = LittleLong (*in);
    }
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides(clipMap_t *cm, lump_t *l)
{
    int             i;
    cbrushside_t    *out;
    dbrushside_t    *in;
    int             count;
    int             num;

    in = (void *)(cmod_base + l->fileofs);
    if ( l->filelen % sizeof(*in) ) {
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    }
    count = l->filelen / sizeof(*in);

    cm->brushsides = Hunk_Alloc( ( BOX_SIDES + count ) * sizeof( *cm->brushsides ), h_high );
    cm->numBrushSides = count;

    out = cm->brushsides;

    for ( i=0 ; i<count ; i++, in++, out++) {
        num = LittleLong( in->planeNum );
        out->plane = &cm->planes[num];
        out->shaderNum = LittleLong( in->shaderNum );
        if ( out->shaderNum < 0 || out->shaderNum >= cm->numShaders ) {
            Com_Error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
        }
        out->surfaceFlags = cm->shaders[out->shaderNum].surfaceFlags;
    }
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString(clipMap_t *cm, lump_t *l)
{
    cm->entityString = Hunk_Alloc( l->filelen, h_high );
    cm->numEntityChars = l->filelen;
    Com_Memcpy (cm->entityString, cmod_base + l->fileofs, l->filelen);
}

/*
=================
CMod_LoadVisibility
=================
*/
#define VIS_HEADER  8
void CMod_LoadVisibility(clipMap_t *cm, lump_t *l)
{
    int     len;
    byte    *buf;

    len = l->filelen;
    if ( !len ) {
        cm->clusterBytes = ( cm->numClusters + 31 ) & ~31;
        cm->visibility = Hunk_Alloc( cm->clusterBytes, h_high );
        Com_Memset( cm->visibility, 255, cm->clusterBytes );
        return;
    }
    buf = cmod_base + l->fileofs;

    cm->vised = qtrue;
    cm->visibility = Hunk_Alloc( len, h_high );
    cm->numClusters = LittleLong( ((int *)buf)[0] );
    cm->clusterBytes = LittleLong( ((int *)buf)[1] );
    Com_Memcpy (cm->visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define MAX_PATCH_VERTS     1024
void CMod_LoadPatches(clipMap_t *cm, lump_t *surfs, lump_t *verts)
{
    drawVert_t  *dv, *dv_p;
    dsurface_t  *in;
    int         count;
    int         i, j;
    int         c;
    cPatch_t    *patch;
    vec3_t      points[MAX_PATCH_VERTS];
    int         width, height;
    int         shaderNum;

    in = (void *)(cmod_base + surfs->fileofs);
    if (surfs->filelen % sizeof(*in))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
    cm->numSurfaces = count = surfs->filelen / sizeof(*in);
    cm->surfaces = Hunk_Alloc( cm->numSurfaces * sizeof( cm->surfaces[0] ), h_high );

    dv = (void *)(cmod_base + verts->fileofs);
    if (verts->filelen % sizeof(*dv))
        Com_Error (ERR_DROP, "MOD_LoadBmodel: funny lump size");

    // scan through all the surfaces, but only load patches,
    // not planar faces
    for ( i = 0 ; i < count ; i++, in++ ) {
        if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
            continue;       // ignore other surfaces
        }
        // FIXME: check for non-colliding patches

        cm->surfaces[ i ] = patch = Hunk_Alloc( sizeof( *patch ), h_high );

        // load the full drawverts onto the stack
        width = LittleLong( in->patchWidth );
        height = LittleLong( in->patchHeight );
        c = width * height;
        if ( c > MAX_PATCH_VERTS ) {
            Com_Error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
        }

        dv_p = dv + LittleLong( in->firstVert );
        for ( j = 0 ; j < c ; j++, dv_p++ ) {
            points[j][0] = LittleFloat( dv_p->xyz[0] );
            points[j][1] = LittleFloat( dv_p->xyz[1] );
            points[j][2] = LittleFloat( dv_p->xyz[2] );
        }

        shaderNum = LittleLong( in->shaderNum );
        patch->contents = cm->shaders[shaderNum].contentFlags;
        patch->surfaceFlags = cm->shaders[shaderNum].surfaceFlags;

        // create the internal facet structure
        patch->pc = CM_GeneratePatchCollide( width, height, points );
    }
}

//==================================================================

unsigned CM_LumpChecksum(lump_t *lump) {
    return LittleLong (Com_BlockChecksum (cmod_base + lump->fileofs, lump->filelen));
}

unsigned CM_Checksum(dheader_t *header) {
    unsigned checksums[16];
    checksums[0] = CM_LumpChecksum(&header->lumps[LUMP_SHADERS]);
    checksums[1] = CM_LumpChecksum(&header->lumps[LUMP_LEAFS]);
    checksums[2] = CM_LumpChecksum(&header->lumps[LUMP_LEAFBRUSHES]);
    checksums[3] = CM_LumpChecksum(&header->lumps[LUMP_LEAFSURFACES]);
    checksums[4] = CM_LumpChecksum(&header->lumps[LUMP_PLANES]);
    checksums[5] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHSIDES]);
    checksums[6] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHES]);
    checksums[7] = CM_LumpChecksum(&header->lumps[LUMP_MODELS]);
    checksums[8] = CM_LumpChecksum(&header->lumps[LUMP_NODES]);
    checksums[9] = CM_LumpChecksum(&header->lumps[LUMP_SURFACES]);
    checksums[10] = CM_LumpChecksum(&header->lumps[LUMP_DRAWVERTS]);

    return LittleLong(Com_BlockChecksum(checksums, 11 * 4));
}

/*
==================
CM_LoadBSPFile

Load the actual BSP file contents
to the specified clipmap.

Returns qfalse upon any error,
or qtrue upon success.
==================
*/

static qboolean CM_LoadBSPFile(clipMap_t *cm, const char *name, int *checksum)
{
    union {
        int             *i;
        void            *v;
    } buf;
    int             i;
    dheader_t       header;
    int             length;

    //
    // load the file
    //
#ifndef BSPC
    length = FS_ReadFile( name, &buf.v );
#else
    length = LoadQuakeFile((quakefile_t *) name, &buf.v);
#endif

    if ( !buf.i ) {
        return qfalse;
    }

    if(checksum != NULL){
        *checksum = LittleLong (Com_BlockChecksum (buf.i, length));
    }

    header = *(dheader_t *)buf.i;
    for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
        ((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);
    }

    if ( header.version != BSP_VERSION ) {
        Com_Error (ERR_DROP, "CM_LoadMap: %s has wrong version number (%i should be %i)"
        , name, header.version, BSP_VERSION );
    }

    cmod_base = (byte *)buf.i;

    // load into heap
    CMod_LoadShaders(cm, &header.lumps[LUMP_SHADERS]);
    CMod_LoadLeafs(cm, &header.lumps[LUMP_LEAFS]);
    CMod_LoadLeafBrushes(cm, &header.lumps[LUMP_LEAFBRUSHES]);
    CMod_LoadLeafSurfaces(cm, &header.lumps[LUMP_LEAFSURFACES]);
    CMod_LoadPlanes(cm, &header.lumps[LUMP_PLANES]);
    CMod_LoadBrushSides(cm, &header.lumps[LUMP_BRUSHSIDES]);
    CMod_LoadBrushes(cm, &header.lumps[LUMP_BRUSHES]);
    CMod_LoadSubmodels(cm, &header.lumps[LUMP_MODELS]);
    CMod_LoadNodes(cm, &header.lumps[LUMP_NODES]);
    CMod_LoadEntityString(cm, &header.lumps[LUMP_ENTITIES]);
    CMod_LoadVisibility(cm, &header.lumps[LUMP_VISIBILITY]);
    CMod_LoadPatches(cm, &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS]);

    // Increment the total amount of sub-models loaded across loaded clipmap BSPs.
    totalSubModels += cm->numSubModels;

    // we are NOT freeing the file, because it is cached for the ref
    FS_FreeFile(buf.v);

    return qtrue;
}

/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/

void CM_LoadMap(const char *name, int *checksum)
{
    static unsigned last_checksum;

    if(!name || !name[0]) {
        Com_Error(ERR_DROP, "CM_LoadMap: NULL name");
    }

#ifndef BSPC
    cm_noAreas = Cvar_Get("cm_noAreas", "0", CVAR_CHEAT);
    cm_noCurves = Cvar_Get("cm_noCurves", "0", CVAR_CHEAT);
    cm_playerCurveClip = Cvar_Get("cm_playerCurveClip", "1", CVAR_ARCHIVE|CVAR_CHEAT);
#endif

    Com_DPrintf("CM_LoadMap: %s\n", name);

    // Check if we're attempting to load an already loaded map.
    if(strcmp(cmg->name, name) == 0){
        *checksum = last_checksum;
        return;
    }

    // Load in the file contents.
    if(!CM_LoadBSPFile(cmg, name, &last_checksum)){
        Com_Error(ERR_DROP, "Couldn't load %s", name);
    }
    *checksum = last_checksum;

#ifndef BSPC
    // FIXME BOE: Only load when a custom terrain is initialized (?)
    CM_LoadShaderFiles();
#endif // !BSPC

    CM_InitBoxHull ();

    CM_FloodAreaConnections (cmg);

    // allow this to be cached
    Q_strncpyz(cmg->name, name, sizeof(cmg->name));
}

/*
==================
CM_LoadSubBSP

Loads in the specified sub-BSP
and all of its data.
==================
*/

int CM_LoadSubBSP(const char *name)
{
    int         i;
    int         count;
    clipMap_t   *cm;

    // Iterate through all sub-BSPs. Determine clipmap array index
    // and whether it is loaded already.
    count = cmg->numSubModels;
    for(i = 1; i < MAX_CLIPMAP_BSP; i++){
        // Do the names match?
        if(Q_stricmp(name, cmBSPs[i].name) == 0){
            return count;
        }

        // Empty name means this index can be used.
        if(cmBSPs[i].name[0] == 0){
            break;
        }

        // Keep track of model count to set the proper model index later on.
        count += cmBSPs[i].numSubModels;
    }

    if(i == MAX_CLIPMAP_BSP){
        Com_Error(ERR_DROP, "CM_LoadSubBSP: Too many unique sub-BSPs!");
    }

    // Load the BSP file.
    Com_DPrintf("CM_LoadSubBSP: %s\n", name);

    // Set the active clipmap based on the determined index.
    cm = &cmBSPs[i];

    // Load in the file contents.
    if(!CM_LoadBSPFile(cm, name, NULL)){
        Com_Error(ERR_DROP, "CM_LoadSubBSP: Couldn't load: %s", name);
    }

    CM_FloodAreaConnections(cm);

    // write the BSP name into the clipmap struct so that it could be reused.
    Q_strncpyz(cm->name, name, sizeof(cm->name));

    // Return the model index.
    return count;
}

/*
==================
CM_ClearMap
==================
*/
void CM_ClearMap( void ) {
    Com_Memset( &cmBSPs, 0, sizeof( cmBSPs ) );
    CM_ClearLevelPatches();

    // Reset total model count.
    totalSubModels = 0;
}

/*
==================
CM_ClipHandleToModel
==================
*/
cmodel_t    *CM_ClipHandleToModel( clipHandle_t handle ) {
    int         i;
    int         modelCount;

    if ( handle < 0 ) {
        Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle );
    }
    if ( handle < cmg->numSubModels ) {
        return &cmg->cmodels[handle];
    }
    if ( handle == BOX_MODEL_HANDLE ) {
        return &box_model;
    }

    // Iterate through all sub-BSPs, check
    // if the model belongs to any.
    modelCount = cmg->numSubModels;
    for(i = 1; i < MAX_CLIPMAP_BSP; i++){
        clipMap_t   *currBSP = &cmBSPs[i];

        if(handle < (modelCount + currBSP->numSubModels)){
            return &currBSP->cmodels[handle - modelCount];
        }

        modelCount += currBSP->numSubModels;
    }

    if ( handle < MAX_SUBMODELS ) {
        Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i < %i < %i",
            cmg->numSubModels, handle, MAX_SUBMODELS );
    }
    Com_Error( ERR_DROP, "CM_ClipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

    return NULL;

}

/*
==================
CM_InlineModel
==================
*/
clipHandle_t    CM_InlineModel( int index ) {
    if ( index < 0 || index >= totalSubModels ) {
        Com_Error (ERR_DROP, "CM_InlineModel: bad number");
    }
    return index;
}

int     CM_NumClusters( void ) {
    return cmg->numClusters;
}

int     CM_NumInlineModels( void ) {
    return cmg->numSubModels;
}

char    *CM_EntityString( int cmIndex ) {
    return cmBSPs[cmIndex].entityString;
}

int     CM_LeafCluster( int leafnum ) {
    if (leafnum < 0 || leafnum >= cmg->numLeafs) {
        Com_Error (ERR_DROP, "CM_LeafCluster: bad number");
    }
    return cmg->leafs[leafnum].cluster;
}

int     CM_LeafArea( int leafnum ) {
    if ( leafnum < 0 || leafnum >= cmg->numLeafs ) {
        Com_Error (ERR_DROP, "CM_LeafArea: bad number");
    }
    return cmg->leafs[leafnum].area;
}

//=======================================================================


/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CM_InitBoxHull (void)
{
    int         i;
    int         side;
    cplane_t    *p;
    cbrushside_t    *s;

    box_planes = &cmg->planes[cmg->numPlanes];

    box_brush = &cmg->brushes[cmg->numBrushes];
    box_brush->numsides = 6;
    box_brush->sides = cmg->brushsides + cmg->numBrushSides;
    box_brush->contents = CONTENTS_BODY;

    box_model.firstNode = -1;
    box_model.leaf.numLeafBrushes = 1;
//  box_model.leaf.firstLeafBrush = cm.numBrushes;
    box_model.leaf.firstLeafBrush = cmg->numLeafBrushes;
    cmg->leafbrushes[cmg->numLeafBrushes] = cmg->numBrushes;

    for (i=0 ; i<6 ; i++)
    {
        side = i&1;

        // brush sides
        s = &cmg->brushsides[cmg->numBrushSides+i];
        s->plane =  cmg->planes + (cmg->numPlanes+i*2+side);
        s->surfaceFlags = 0;

        // planes
        p = &box_planes[i*2];
        p->type = i>>1;
        p->signbits = 0;
        VectorClear (p->normal);
        p->normal[i>>1] = 1;

        p = &box_planes[i*2+1];
        p->type = 3 + (i>>1);
        p->signbits = 0;
        VectorClear (p->normal);
        p->normal[i>>1] = -1;

        SetPlaneSignbits( p );
    }
}

/*
===================
CM_TempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule ) {

    VectorCopy( mins, box_model.mins );
    VectorCopy( maxs, box_model.maxs );

    if ( capsule ) {
        return CAPSULE_MODEL_HANDLE;
    }

    box_planes[0].dist = maxs[0];
    box_planes[1].dist = -maxs[0];
    box_planes[2].dist = mins[0];
    box_planes[3].dist = -mins[0];
    box_planes[4].dist = maxs[1];
    box_planes[5].dist = -maxs[1];
    box_planes[6].dist = mins[1];
    box_planes[7].dist = -mins[1];
    box_planes[8].dist = maxs[2];
    box_planes[9].dist = -maxs[2];
    box_planes[10].dist = mins[2];
    box_planes[11].dist = -mins[2];

    VectorCopy( mins, box_brush->bounds[0] );
    VectorCopy( maxs, box_brush->bounds[1] );

    return BOX_MODEL_HANDLE;
}

/*
===================
CM_ModelBounds
===================
*/
void CM_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
    cmodel_t    *cmod;

    cmod = CM_ClipHandleToModel( model );
    VectorCopy( cmod->mins, mins );
    VectorCopy( cmod->maxs, maxs );
}

//=======================================================================

/*
==================
CM_FindSubBSP

Finds what BSP the specified model
index belongs to and, if valid, returns
the main or sub-BSP index.

Also returns the main BSP index if
the model index is out of range.
==================
*/

int CM_FindSubBSP(int modelIndex)
{
    int i;
    int modelCount;

    // Check if the model index is within range.
    if(modelIndex < 0 || modelIndex >= totalSubModels){
        return 0;
    }

    // Does the model index belong the main BSP or any of the sub-BSPs?
    modelCount  = 0;
    for(i = 0; i < MAX_CLIPMAP_BSP; i++){
        modelCount += cmBSPs[i].numSubModels;

        if(modelIndex < modelCount){
            // Model belongs to this BSP.
            break;
        }
    }

    return i;
}

/*
==================
CM_ClipmapFromModel

Gets the clipmap of which the specified
model index belongs to.

Returns the main BSP clipmap if the
model index is out of range.
==================
*/

clipMap_t *CM_ClipmapFromModel(int modelIndex)
{
    return &cmBSPs[CM_FindSubBSP(modelIndex)];
}
