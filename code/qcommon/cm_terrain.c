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
// cm_terrain.c - Clipmap terrain support.

#include "cm_local.h"
#include "genericparser2.h"

/*
==================
CM_SetTerrainShaders

Set height information based
on the passed shader.
==================
*/

static void CM_SetTerrainShaders(cTerrain_t *t, dshader_t *shader, int height)
{
    int i;

    // Shader must be valid.
    if(shader == NULL){
        return;
    }

    for(i = height; i < HEIGHT_RESOLUTION; i++){
        if(!t->mHeightDetails[i].mSurfaceFlags){
            // Copy surface flags from the shader.
            t->mHeightDetails[i].mSurfaceFlags = shader->surfaceFlags;

            // Set contents to be the default.
            t->mHeightDetails[i].mContents = CONTENTS_SOLID | CONTENTS_OPAQUE;
        }else{
            // FIXME BOE: Remove later on.
            if(t->mHeightDetails[i].mContents != (CONTENTS_SOLID | CONTENTS_OPAQUE)){
                Com_Printf(S_COLOR_RED "CM_SetTerrainShaders: Content flags: %s [%3d] -> %d\n", shader->shader, i, t->mHeightDetails[i].mContents);
            }
        }
    }
}

/*
==================
CM_LoadTerrainDef

Load and parse a GP2-based
terrain definition file.
==================
*/

static void CM_LoadTerrainDef(cTerrain_t *t, const char *configString)
{
    const char      *defFile;
    char            defFileFull[MAX_QPATH];
    char            itemGroupName[256];
    char            shaderName[MAX_QPATH];
    char            heightBuf[12];
    dshader_t       *shader;
    int             height;
    TGenericParser2 GP2;
    TGPGroup        topGroup;
    TGPGroup        subGroup;
    TGPGroup        itemGroup;

    //
    // Load the terrain definition file.
    //

    // Check if a terrain definition file is specified.
    defFile = Info_ValueForKey(configString, "terrainDef");
    if(!defFile[0]){
        Com_DPrintf("CM_LoadTerrainDef: No terrain defintion file specified.\n");
        return;
    }

    // The specified terrain definition file can be loaded
    // from one of the following directories,
    // prioritized in the following order:
    // - ext_data/RMG
    // - ext_data/arioche
    if(FS_ReadFile(va("ext_data/RMG/%s.terrain", defFile), NULL) > 0){
        Com_sprintf(defFileFull, sizeof(defFileFull), "ext_data/RMG/%s.terrain", defFile);
    }else if(FS_ReadFile(va("ext_data/arioche/%s.terrain", defFile), NULL) > 0){
        Com_sprintf(defFileFull, sizeof(defFileFull), "ext_data/arioche/%s.terrain", defFile);
    }else{
        // File not found or empty.
        Com_Printf(S_COLOR_YELLOW "CM_LoadTerrainDef: Could not load file: %s.terrain\n", defFile);
        return;
    }

    //
    // Parse the terrain definition file.
    //

    // Open the specified GP2 file.
    GP2 = GP_ParseFile(defFileFull);
    if(!GP2){
        Com_Printf(S_COLOR_YELLOW "CM_LoadTerrainDef: Could not parse GP2 file: %s.terrain\n", defFile);
        return;
    }

    // Grab the top group and the initial subgroup.
    topGroup = GP_GetBaseParseGroup(GP2);
    subGroup = GPG_GetSubGroups(topGroup);

    // Iterate through the sub group so we
    // can iterate through the item group.
    while(subGroup){
        itemGroup = GPG_GetSubGroups(subGroup);
        while(itemGroup){
            // Get the name of this item group.
            GPG_GetName(itemGroup, itemGroupName, sizeof(itemGroupName));

            if(Q_stricmp(itemGroupName, "altitudetexture") == 0){
                // Height must be defined.
                GPG_FindPairValue(itemGroup, "height", "0", heightBuf, sizeof(heightBuf));
                height = atoi(heightBuf);

                // Get the shader for this height.
                GPG_FindPairValue(itemGroup, "shader", "", shaderName, sizeof(shaderName));
                if(shaderName[0]){
                    // Shader defined, get its info.
                    shader = CM_FindShaderByName(shaderName);
                    if(shader){
                        // Valid shader. Set the height information.
                        CM_SetTerrainShaders(t, shader, height);
                    }
                }
            }else if(Q_stricmp(itemGroupName, "water") == 0){
                // Grab the height of the water.
                GPG_FindPairValue(itemGroup, "height", "0", heightBuf, sizeof(heightBuf));
                t->mBaseWaterHeight = atol(heightBuf);

                // Set the real water height.
                t->mWaterHeight = t->mBaseWaterHeight * t->mTerxelSize[2];

                // Get the shader to determine the material.
                GPG_FindPairValue(itemGroup, "shader", "", shaderName, sizeof(shaderName));
                if(shaderName[0]){
                    // Shader defined, get its info.
                    shader = CM_FindShaderByName(shaderName);
                    if(shader){
                        t->mWaterContents = shader->contentFlags;
                        t->mWaterSurfaceFlags = shader->surfaceFlags;
                    }
                }
            }

            itemGroup = GPG_GetNext(itemGroup);
        }
        subGroup = GPG_GetNext(subGroup);
    }

    //
    // Free the terrain definition file.
    //
    GP_Delete(&GP2);
}

/*
==================
CM_GetPatch

Get the patch on the
given coordinate.
==================
*/

static ID_INLINE cTerrainPatch_t *CM_GetPatch(cTerrain_t *t, int x, int y)
{
    return(t->mPatches + ((y * t->mBlockWidth) + x));
}

/*
==================
CM_GetAdjacentBrushY

Finds adjacent y-axis
brush.
==================
*/

static cbrush_t *CM_GetAdjacentBrushY(cTerrainPatch_t *p, int x, int y)
{
    int                 yo1;
    int                 yo2;
    int                 xo;
    cTerrainPatch_t     *patch;
    cbrush_t            *brush;

    yo1                 = y % p->owner->mTerxels;
    yo2                 = (y - 1) % p->owner->mTerxels;
    xo                  = x % p->owner->mTerxels;

    // Different patch?
    if(yo2 > yo1){
        patch = CM_GetPatch(p->owner, x / p->owner->mTerxels, (y - 1) / p->owner->mTerxels);
    }else{
        patch = p;
    }

    brush = patch->mPatchBrushData;
    brush += ((yo2 * p->owner->mTerxels + xo) * 2);
    brush++;

    return brush;
}

/*
==================
CM_GetAdjacentBrushX

Finds adjacent x-axis
brush.
==================
*/

static cbrush_t *CM_GetAdjacentBrushX(cTerrainPatch_t *p, int x, int y)
{
    int                 xo1;
    int                 xo2;
    int                 yo;
    cTerrainPatch_t     *patch;
    cbrush_t            *brush;

    xo1                 = x % p->owner->mTerxels;
    xo2                 = (x - 1) % p->owner->mTerxels;
    yo                  = y % p->owner->mTerxels;

    // Different patch?
    if(xo2 > xo1){
        patch = CM_GetPatch(p->owner, (x - 1) / p->owner->mTerxels, y / p->owner->mTerxels);
    }else{
        patch = p;
    }

    brush = patch->mPatchBrushData;
    brush += ((yo * p->owner->mTerxels + xo2) * 2);
    if(!((x+y) & 1)){
        brush++;
    }

    return brush;
}

/*
==================
CM_InitPlane

Initialize a plane from
three coordinates.
==================
*/

static void CM_InitPlane(cbrushside_t *side, cplane_t *plane, vec3_t p0, vec3_t p1, vec3_t p2)
{
    vec3_t  dx, dy;

    // Determine plane X and Y.
    VectorSubtract(p1, p0, dx);
    VectorSubtract(p2, p0, dy);

    // Set plane normal.
    CrossProduct(dx, dy, plane->normal);
    VectorNormalize(plane->normal);

    // Set misc. plane properties.
    plane->dist = DotProduct(p0, plane->normal);
    plane->type = PlaneTypeForNormal(plane->normal);

    // Set side.
    SetPlaneSignbits(plane);
    side->plane = plane;
}

/*
==================
CM_CreatePatchPlaneData

Create the planes required
for collision detection.

Two brushes per terxel,
each brush has five sides
and five planes.
==================
*/

static void CM_CreatePatchPlaneData(cTerrainPatch_t *p)
{
    cbrush_t        *brush;
    cbrushside_t    *side;
    cplane_t        *plane;
    int             realWidth;
    int             i, j;
    int             x, y;
    int             offsets[4];
    vec3_t          *coords;
    vec3_t          localCoords[8];
    float           V;

    brush           = p->mPatchBrushData;
    side            = (cbrushside_t *)(p->mPatchBrushData + p->mNumBrushes);
    plane           = (cplane_t *)(side + (p->mNumBrushes * BRUSH_SIDES_PER_TERXEL * 2));

    realWidth       = p->owner->mRealWidth;
    coords          = p->owner->mCoords;

    //
    // Iterate through terxels.
    //

    for(y = p->mHy; y < p->mHy + p->owner->mTerxels; y++){
        for(x = p->mHx; x < p->mHx + p->owner->mTerxels; x++){
            // Set terxel brush offsets.
            if((x+y) & 1){
                offsets[0] = (y * realWidth) + x;           // TL
                offsets[1] = (y * realWidth) + x + 1;       // TR
                offsets[2] = ((y + 1) * realWidth) + x;     // BL
                offsets[3] = ((y + 1) * realWidth) + x + 1; // BR
            }else{
                offsets[2] = (y * realWidth) + x;           // TL
                offsets[0] = (y * realWidth) + x + 1;       // TR
                offsets[3] = ((y + 1) * realWidth) + x;     // BL
                offsets[1] = ((y + 1) * realWidth) + x + 1; // BR
            }

            // Set local coordinates.
            for(i = 0; i < 4; i++){
                VectorCopy(coords[offsets[i]], localCoords[i]);
                VectorCopy(coords[offsets[i]], localCoords[i + 4]);

                // Set Z of base of brush to bottom of the terrain brush.
                localCoords[i + 4][2] = p->owner->mBounds[0][2];
            }

            // Set the bounds for the terxel.
            VectorSet(brush[0].bounds[0], MAX_WORLD_COORD, MAX_WORLD_COORD, MAX_WORLD_COORD);
            VectorSet(brush[0].bounds[1], MIN_WORLD_COORD, MIN_WORLD_COORD, MIN_WORLD_COORD);

            // Set mins and maxs.
            for(i = 0; i < 8; i++){
                for(j = 0; j < 3; j++){
                    // Mins.
                    if(localCoords[i][j] < brush[0].bounds[0][j]){
                        brush[0].bounds[0][j] = localCoords[i][j];
                    }
                    // Maxs.
                    if(localCoords[i][j] > brush[0].bounds[1][j]){
                        brush[0].bounds[1][j] = localCoords[i][j];
                    }
                }
            }

            for(i = 0; i < 3; i++){
                brush[0].bounds[0][i] -= 1.0f;
                brush[0].bounds[1][i] += 1.0f;
            }

            VectorCopy(brush[0].bounds[0], brush[1].bounds[0]);
            VectorCopy(brush[0].bounds[1], brush[1].bounds[1]);

            // Set brush content flags.
            brush[0].contents = p->mContentFlags;
            brush[1].contents = p->mContentFlags;

            // Set up sides of the brushes.
            brush[0].numsides = 5;
            brush[0].sides = side;
            brush[1].numsides = 5;
            brush[1].sides = side + 8;

            // Create the planes of the two triangles that make up the tops of the brushes.
            CM_InitPlane(side + 0, plane + 0, localCoords[0], localCoords[1], localCoords[2]);
            CM_InitPlane(side + 8, plane + 8, localCoords[3], localCoords[2], localCoords[1]);

            // Create the bottom face of the brushes.
            CM_InitPlane(side + 1, plane + 1, localCoords[4], localCoords[6], localCoords[5]);
            CM_InitPlane(side + 9, plane + 9, localCoords[7], localCoords[5], localCoords[6]);

            // Create the three vertical faces.
            CM_InitPlane(side + 2, plane + 2, localCoords[0], localCoords[2], localCoords[4]);
            CM_InitPlane(side + 10, plane + 10, localCoords[3], localCoords[1], localCoords[7]);

            CM_InitPlane(side + 3, plane + 3, localCoords[0], localCoords[4], localCoords[1]);
            CM_InitPlane(side + 11, plane + 11, localCoords[3], localCoords[7], localCoords[2]);

            CM_InitPlane(side + 4, plane + 4, localCoords[2], localCoords[1], localCoords[6]);
            CM_InitPlane(side + 12, plane + 12, localCoords[5], localCoords[1], localCoords[6]);

            V = DotProduct((plane + 8)->normal, localCoords[0]) - (plane + 8)->dist;

            if(V < 0){
                CM_InitPlane(brush[0].sides + brush[0].numsides, plane + brush[0].numsides, localCoords[3], localCoords[2], localCoords[1]);
                brush[0].numsides++;

                CM_InitPlane(brush[1].sides + brush[1].numsides, plane + 8 + brush[1].numsides, localCoords[0], localCoords[1], localCoords[2]);
                brush[1].numsides++;
            }

            // Determine if we need to smooth the brush transition from the brush above us.
            if(y > 0 && y < p->owner->mPatchSize[1] - 1){
                cbrush_t    *aboveBrush;
                cplane_t    *abovePlane;

                aboveBrush  = CM_GetAdjacentBrushY(p, x, y);
                abovePlane  = aboveBrush->sides->plane;

                if((x + y) & 1){
                    V = DotProduct(abovePlane->normal, localCoords[2]) - abovePlane->dist;
                }else{
                    V = DotProduct(abovePlane->normal, localCoords[1]) - abovePlane->dist;
                }

                if(V < 0){
                    Com_Memcpy(brush[0].sides + brush[0].numsides, aboveBrush->sides, sizeof(cbrushside_t));
                    brush[0].numsides++;

                    Com_Memcpy(aboveBrush->sides + aboveBrush->numsides, side, sizeof(cbrushside_t));
                    aboveBrush->numsides++;
                }
            }

            // Determine if we need to smooth the brush transition from the brush left of us.
            if(x > 0 && x < p->owner->mPatchSize[0] - 1){
                cbrush_t    *aboveBrush;
                cplane_t    *abovePlane;

                aboveBrush  = CM_GetAdjacentBrushX(p, x, y);
                abovePlane  = aboveBrush->sides->plane;

                V = DotProduct(abovePlane->normal, localCoords[1]) - abovePlane->dist;

                if(V < 0){
                    if((x + y) & 1){
                        Com_Memcpy(brush[0].sides + brush[0].numsides, aboveBrush->sides, sizeof(cbrushside_t));
                        brush[0].numsides++;

                        Com_Memcpy(aboveBrush->sides + aboveBrush->numsides, side, sizeof(cbrushside_t));
                        aboveBrush->numsides++;
                    }else{
                        Com_Memcpy(brush[1].sides + brush[1].numsides, aboveBrush->sides, sizeof(cbrushside_t));
                        brush[1].numsides++;

                        Com_Memcpy(aboveBrush->sides + aboveBrush->numsides, side + 8, sizeof(cbrushside_t));
                        aboveBrush->numsides++;
                    }
                }
            }

            // Increment to the next terxel.
            brush += 2;
            side += 16;
            plane += 16;
        }
    }
}

/*
==================
CM_InitTerrainPatch

Initializes a terrain
patch.
==================
*/

static void CM_InitTerrainPatch(cTerrain_t *t, cTerrainPatch_t *patch, int heightX,
                                int heightY, vec3_t world, byte *patchBrushData)
{
    int x, y;
    int min, max;
    int height;
    int avgHeight;

    // Set owning terrain.
    patch->owner = t;

    // Store the base of the top left corner.
    VectorCopy(world, patch->mWorldCoords);

    // Store pointer to the first byte of
    // the height data for this patch.
    patch->mHx = heightX;
    patch->mHy = heightY;
    patch->mHeightMap = t->mHeightMap + ((heightY * t->mRealWidth) + heightX);

    // Calculate the bounds for culling. Use the real dimensions
    // to allow for sloping of edge terxels.
    min = 256;
    max = -1;
    for(y = heightY -1; y < heightY + t->mTerxels + 1; y++){
        // Only allow positive coordinates for Y.
        if(y < 0){
            continue;
        }

        for(x = heightX - 1; x < heightX + t->mTerxels + 1; x++){
            // Only allow positive coordinates for X.
            if(x < 0){
                continue;
            }

            // Calculate height.
            height = t->mHeightMap[y * t->mRealWidth + x];

            // Check if we can increase our bounds.
            if(height > max){
                max = height;
            }

            if(height < min){
                min = height;
            }
        }
    }

    // Set remaining patch info.
    // Mins.
    patch->mBounds[0][0] = world[0];
    patch->mBounds[0][1] = world[1];
    patch->mBounds[0][2] = world[2] + (min * t->mTerxelSize[2]);

    // Maxs.
    patch->mBounds[1][0] = world[0] + t->mPatchSize[0];
    patch->mBounds[1][1] = world[1] + t->mPatchSize[1];
    patch->mBounds[1][2] = world[2] + (max * t->mTerxelSize[2]);

    // Corner heights.
    patch->mCornerHeights[0] = patch->mHeightMap[0];
    patch->mCornerHeights[1] = patch->mHeightMap[t->mTerxels];
    patch->mCornerHeights[2] = patch->mHeightMap[t->mTerxels * t->mRealWidth];
    patch->mCornerHeights[3] = patch->mHeightMap[t->mTerxels * t->mRealWidth + t->mRealWidth];

    // Surface flags using average height.
    avgHeight = (min + max) >> 1;
    patch->mSurfaceFlags = t->mHeightDetails[avgHeight].mSurfaceFlags;
    patch->mContentFlags = t->mHeightDetails[avgHeight].mContents;

    // Base of brush data.
    patch->mPatchBrushData = (cbrush_t *)patchBrushData;

    // Number of brushes.
    patch->mNumBrushes = t->mTerxels * t->mTerxels * 2;

    // Continue creating patch plane data.
    CM_CreatePatchPlaneData(patch);
}

/*
==================
CM_UpdateTerrainPatches

Updates all terrain patches
on the given terrain.
==================
*/

static void CM_UpdateTerrainPatches(cTerrain_t *t)
{
    int                 i, x, y, ix, iy;
    int                 numBrushesPerPatch;
    int                 size;
    cTerrainPatch_t     *patch;
    vec3_t              world;

    //
    // Calculate the real world coordinates of each heightmap entry.
    //

    // Allocate some temporary space for the coordinates.
    t->mCoords = Z_Malloc(sizeof(vec3_t) * t->mRealArea);

    // Work out the real world coordinates.
    for(y = 0; y < t->mRealHeight; y++){
        for(x = 0; x < t->mRealWidth; x++){
            int coordinates[3];
            int offset;

            // Determine offset.
            offset = (y * t->mRealWidth) + x;

            // Calculate coordinates.
            VectorSet(coordinates, x, y, t->mHeightMap[offset]);
            for(i = 0; i < 3; i++){
                t->mCoords[offset][i] = t->mBounds[0][i] + (coordinates[i] * t->mTerxelSize[i]);
            }
        }
    }

    //
    // Initialize the patches.
    //
    numBrushesPerPatch = t->mTerxels * t->mTerxels * 2;
    size = (numBrushesPerPatch * sizeof(cbrush_t)) + (numBrushesPerPatch * BRUSH_SIDES_PER_TERXEL * 2 * (sizeof(cbrushside_t) + sizeof(cplane_t)));

    // Iterate through patches.
    patch = t->mPatches;
    for(y = 0, iy = 0; y < t->mHeight; y += t->mTerxels, iy++){
        for(x = 0, ix = 0; x < t->mWidth; x += t->mTerxels, ix++, patch++){
            // Set world size.
            VectorSet(world,
                t->mBounds[0][0] + (x * t->mTerxelSize[0]),
                t->mBounds[0][1] + (y * t->mTerxelSize[1]),
                t->mBounds[0][2]);

            // Initialize this patch.
            CM_InitTerrainPatch(t, patch, x, y, world,
                t->mPatchBrushData + (size * (ix + (iy * t->mBlockWidth))));
        }
    }

    //
    // Free allocated resources.
    //
    Z_Free(t->mCoords);
}

/*
==================
CM_InitTerrain

Initializes a new terrain
based on the supplied
configuration string.

Returns the new terrain instance,
or NULL if the terrain config
is invalid.
==================
*/

static cTerrain_t *CM_InitTerrain(const char *configString)
{
    int             numPatches, numBrushesPerPatch;
    int             size;
    char            heightMap[MAX_QPATH];
    cTerrain_t      *t;

    //
    // Check if certain terrain properties are valid.
    //

    Q_strncpyz(heightMap, Info_ValueForKey(configString, "heightMap"), sizeof(heightMap));

    // A valid heightmap file must be specified.
    if(!heightMap[0] || !strstr(heightMap, "random_")){
        Com_Printf(S_COLOR_RED "CM_InitTerrain: Heightmap filename must contain \"random_\".\n");
        return NULL;
    }
    if(FS_ReadFile(va("%s.png", heightMap), NULL) <= 0){
        Com_Printf(S_COLOR_RED "CM_InitTerrain: Specified heightmap file is empty or nonexistent.\n");
        return NULL;
    }

    //
    // Initialize the new terrain.
    //

    // Allocate memory for this terrain.
    t = Hunk_Alloc(sizeof(cTerrain_t), h_high);

    //
    // Extract the relevant information from the configuration string.
    //

    numPatches = atoi(Info_ValueForKey(configString, "numPatches"));

    t->mTerxels = atoi(Info_ValueForKey(configString, "terxels"));
    t->mHasPhysics = atoi(Info_ValueForKey(configString, "physics")) > 0;

    t->mBounds[0][0] = atof(Info_ValueForKey(configString, "minx"));
    t->mBounds[0][1] = atof(Info_ValueForKey(configString, "miny"));
    t->mBounds[0][2] = atof(Info_ValueForKey(configString, "minz"));
    t->mBounds[1][0] = atof(Info_ValueForKey(configString, "maxx"));
    t->mBounds[1][1] = atof(Info_ValueForKey(configString, "maxy"));
    t->mBounds[1][2] = atof(Info_ValueForKey(configString, "maxz"));

    //
    // Set the aspects of the new terrain.
    //

    // Calculate size of the brush.
    VectorSubtract(t->mBounds[1], t->mBounds[0], t->mSize);

    // Work out the dimensions of the brush in blocks.
    // We want to make the blocks as square as possible.
    t->mBlockWidth = roundf(sqrtf(numPatches * t->mSize[0] / t->mSize[1]));
    t->mBlockHeight = roundf(sqrtf(numPatches * t->mSize[1] / t->mSize[0]));
    t->mBlockCount = t->mBlockWidth * t->mBlockHeight;

    // With the dimensions we can get the size of the heightmap.
    t->mWidth = t->mBlockWidth * t->mTerxels;
    t->mHeight = t->mBlockHeight * t->mTerxels;
    t->mArea = t->mWidth * t->mHeight;

    // Also keep reference of the "real" dimensions,
    // including the 1 pixel edge.
    t->mRealWidth = t->mWidth + 1;
    t->mRealHeight = t->mHeight + 1;
    t->mRealArea = t->mRealWidth * t->mRealHeight;

    // Allocate memory for the heightmap and the
    // flattenmap now the dimensions are known.
    t->mHeightMap = Hunk_Alloc(t->mRealArea, h_high);
    t->mFlattenMap = Hunk_Alloc(t->mRealArea, h_high);

    // Work out the dimensions of the terxel.
    // It should be almost square.
    t->mTerxelSize[0] = t->mSize[0] / t->mWidth;
    t->mTerxelSize[1] = t->mSize[1] / t->mHeight;
    t->mTerxelSize[2] = t->mSize[2] / 255.0f;

    // Work out the patchsize.
    t->mPatchSize[0] = t->mSize[0] / t->mBlockWidth;
    t->mPatchSize[1] = t->mSize[1] / t->mBlockHeight;
    t->mPatchSize[2] = 1.0f;
    t->mPatchScalarSize = VectorLength(t->mPatchSize);

    //
    // Load in the water height and properties.
    //

    // Get the shader properties for the blended shaders.
    CM_LoadTerrainDef(t, configString);

    //
    // Create the terrain patches.
    //

    // Allocate some memory for the patches.
    t->mPatches = Hunk_Alloc(sizeof(cTerrainPatch_t) * t->mBlockCount, h_high);
    numBrushesPerPatch = t->mTerxels * t->mTerxels * 2;
    size = (numBrushesPerPatch * sizeof(cbrush_t)) + (numBrushesPerPatch * BRUSH_SIDES_PER_TERXEL * 2 * (sizeof(cbrushside_t) + sizeof(cplane_t)));
    t->mPatchBrushData = Hunk_Alloc(size * t->mBlockCount, h_high);

    // Update the patches.
    CM_UpdateTerrainPatches(t);

    return t;
}

/*
==================
CM_RegisterTerrain

Registering a terrain on the server
allows physics to examine the
terrain data.

Returns the terrain ID instance on
existing terrains and valid new
terrain instances. If the terrain
configuration is invalid, or the
server has no capacity to store
more terrains, 0 is returned
instead.
==================
*/

int CM_RegisterTerrain(const char *configString)
{
    int             terrainId;
    int             modelIndex;
    int             brushNum;
    clipHandle_t    h;
    cmodel_t        *cmod;
    cbrush_t        *b;
    cTerrain_t      *t;

    // Determine terrain ID.
    terrainId = atoi(Info_ValueForKey(configString, "terrainId"));

    // Is this terrain already registered?
    if(terrainId && terrainId < MAX_TERRAINS && cmg->terrains[terrainId] != NULL){
        // Return the valid terrain ID.
        return terrainId;
    }

    // Only continue if we have room to store more terrains.
    if(cmg->numTerrains == MAX_TERRAINS - 1){
        return 0;
    }

    // Proceed initializing a new terrain.
    t = CM_InitTerrain(configString);

    // Don't continue unless we have a valid terrain.
    if(t == NULL){
        return 0;
    }

    // Store the newly initialized terrain in the clipmap terrain array.
    cmg->terrains[++cmg->numTerrains] = t;

    // Get the associated model.
    modelIndex = atoi(Info_ValueForKey(configString, "modelIndex"));
    h = CM_InlineModel(modelIndex);
    cmod = CM_ClipHandleToModel(h);

    // Get the brush.
    brushNum = cmg->leafbrushes[cmod->leaf.firstLeafBrush];

    // Only link this terrain to the brush if it is within bounds.
    if(brushNum >= 0 && brushNum < cmg->numBrushes){
        b = &cmg->brushes[brushNum];
        b->terrain = t;
    }

    // Return the new terrain ID.
    return cmg->numTerrains;
}

/*
===============================================================================

COLLISION TESTING

===============================================================================
*/

/*
==================
CM_TerrainPatchCollide

Checks if the trace is
about to collide with
anything on the terrain
patches.
==================
*/

void CM_TerrainPatchCollide(cTerrain_t *t, traceWork_t *tw, const vec3_t start, const vec3_t end, int checkcount)
{
    vec3_t  tBounds[2];
    float   slope, offset;
    float   startPatchLoc, endPatchLoc;
    float   patchDirection, checkDirection;
    float   startPos, endPos;
    float   fraction;
    int     count, countPatches;

    // Set direction defaults.
    patchDirection = 1;
    checkDirection = 1;

    // Save original fraction.
    fraction = tw->trace.fraction;

    // Convert to a valid bounding box.
    CM_CalcExtents(start, end, tw, tBounds);

    // X travels more than Y?
    if(fabs(end[0] - start[0]) >= fabs(fabs(end[1] - start[1]))){
        // Calculate line slope.
        if(end[0] - start[0]){
            slope = (end[1] - start[1]) / (end[0] - start[0]);
        }else{
            slope = 0;
        }

        // Calculate offset.
        offset = start[1] - (start[0] * slope);

        // Find the starting location on the patch.
        startPatchLoc = floor((start[0] - t->mBounds[0][0]) / t->mPatchSize[0]);
        endPatchLoc = floor((end[0] - t->mBounds[0][0]) / t->mPatchSize[0]);

        // In what direction are we moving along slope?
        if(startPatchLoc <= endPatchLoc){
            // Positive direction.
            endPatchLoc++;
            startPatchLoc--;
            countPatches = endPatchLoc - startPatchLoc + 1;
        }else{
            // Negative direction.
            endPatchLoc--;
            startPatchLoc++;
            patchDirection = -1;
            countPatches = startPatchLoc - endPatchLoc + 1;
        }

        // Should we check in the negative direction?
        if(slope < 0.0f){
            checkDirection = -1;
        }

        // Now calculate the real world location.
        startPos = ((startPatchLoc * t->mPatchSize[0] + t->mBounds[0][0]) * slope) + offset;

        // Calculate it back into patch coordinates.
        startPos = floor((startPos - t->mBounds[0][1] + tw->size[0][1]) / t->mPatchSize[1]);

        do{
            // Valid location?
            if(startPatchLoc >= 0 && startPatchLoc < t->mBlockWidth){
                // Calculate the real world location.
                endPos = (((startPatchLoc + patchDirection) * t->mPatchSize[0] + t->mBounds[0][0]) * slope) + offset;

                // Calculate it back into patch coordinates.
                endPos = floor((endPos - t->mBounds[0][1] + tw->size[1][1]) / t->mPatchSize[1]);

                if(checkDirection < 0){
                    startPos++;
                    endPos--;
                }else{
                    startPos--;
                    endPos++;
                }

                // Collide with patches.
                count = fabs(endPos - startPos) + 1;
                while(count > 0){
                    // Valid location?
                    if(startPos >= 0 && startPos < t->mBlockHeight){
                        // Collide with every patch to find the minimum fraction.
                        CM_HandleTerrainPatchCollide(tw, CM_GetPatch(t, startPatchLoc, startPos), checkcount);

                        if(tw->trace.fraction <= 0.0f){
                            return;
                        }
                    }

                    startPos += checkDirection;
                    count--;
                }

                if(tw->trace.fraction < fraction){
                    return;
                }
            }

            // Move to the next spot.
            // We will stay one behind, to get the opposite edge of the terrain patch.

            // Calculate the real world location.
            startPos = ((startPatchLoc * t->mPatchSize[0] + t->mBounds[0][0]) * slope) + offset;
            startPatchLoc += patchDirection;

            // Calculate it back into patch coordinates.
            startPos = floor((startPos - t->mBounds[0][1] + tw->size[0][1]) / t->mPatchSize[1]);
            countPatches--;
        }while(countPatches > 0);
    }else{
        // Calculate line slope and offset.
        slope = (end[0] - start[0]) / (end[1] - start[1]);
        offset = start[0] - (start[1] * slope);

        // Find the starting location on the patch.
        startPatchLoc = floor((start[1] - t->mBounds[0][1]) / t->mPatchSize[1]);
        endPatchLoc = floor((end[1] - t->mBounds[0][1]) / t->mPatchSize[1]);

        // In what direction are we moving along slope?
        if(startPatchLoc <= endPatchLoc){
            // Positive direction.
            endPatchLoc++;
            startPatchLoc--;
            countPatches = endPatchLoc - startPatchLoc + 1;
        }else{
            // Negative direction.
            endPatchLoc--;
            startPatchLoc++;
            patchDirection = -1;
            countPatches = startPatchLoc - endPatchLoc + 1;
        }

        // Should we check in the negative direction?
        if(slope < 0.0f){
            checkDirection = -1;
        }

        // Now calculate the real world location.
        startPos = ((startPatchLoc * t->mPatchSize[1] + t->mBounds[0][1]) * slope) + offset;

        // Calculate it back into patch coordinates.
        startPos = floor((startPos - t->mBounds[0][0] + tw->size[0][0]) / t->mPatchSize[0]);

        do{
           // Valid location?
            if(startPatchLoc >= 0 && startPatchLoc < t->mBlockHeight){
                // Calculate the real world location.
                endPos = (((startPatchLoc + patchDirection) * t->mPatchSize[1] + t->mBounds[0][1]) * slope) + offset;

                // Calculate it back into patch coordinates.
                endPos = floor((endPos - t->mBounds[0][0] + tw->size[1][0]) / t->mPatchSize[0]);

                if(checkDirection < 0){
                    startPos++;
                    endPos--;
                }else{
                    startPos--;
                    endPos++;
                }

                // Collide with patches.
                count = fabs(endPos - startPos) + 1;
                while(count > 0){
                    // Valid location?
                    if(startPos >= 0 && startPos < t->mBlockWidth){
                        // Collide with every patch to find the minimum fraction.
                        CM_HandleTerrainPatchCollide(tw, CM_GetPatch(t, startPos, startPatchLoc), checkcount);

                        if(tw->trace.fraction <= 0.0f){
                            return;
                        }
                    }

                    startPos += checkDirection;
                    count--;
                }

                if(tw->trace.fraction < fraction){
                    return;
                }
            }

            // Move to the next spot.
            // We will stay one behind, to get the opposite edge of the terrain patch.

            // Calculate the real world location.
            startPos = ((startPatchLoc * t->mPatchSize[1] + t->mBounds[0][1]) * slope) + offset;
            startPatchLoc += patchDirection;

            // Calculate it back into patch coordinates.
            startPos = floor((startPos - t->mBounds[0][0] + tw->size[0][0]) / t->mPatchSize[0]);
            countPatches--;
        }while(countPatches > 0);
    }
}

/*
==================
CM_TerrainWaterCollide

Checks if the trace is about
to enter or leave the water.

Returns original fraction
if the trace is completely
above or below the water.
==================
*/

float CM_TerrainWaterCollide(cTerrain_t *t, const vec3_t begin, const vec3_t end, float fraction)
{
    // Check if we are completely above water.
    if((begin[2] > t->mWaterHeight) && (end[2] > t->mWaterHeight)){
        return fraction;
    }

    // Check if we are completely below water.
    if((begin[2] < t->mWaterHeight) && (end[2] < t->mWaterHeight)){
        return fraction;
    }

    // Check for starting in water and leaving the water.
    if(begin[2] < t->mWaterHeight - SURFACE_CLIP_EPSILON)
    {
        fraction = ((t->mWaterHeight - SURFACE_CLIP_EPSILON) - begin[2]) / (end[2] - begin[2]);
        return fraction;
    }

    // By now the trace must be entering the water.
    if(begin[2] > t->mWaterHeight + SURFACE_CLIP_EPSILON){
        fraction = (begin[2] - (t->mWaterHeight + SURFACE_CLIP_EPSILON)) / (begin[2] - end[2]);
    }

    return fraction;
}
