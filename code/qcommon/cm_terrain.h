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
// cm_terrain.h

#ifndef __CM_TERRAIN_H
#define __CM_TERRAIN_H

//=============================================
//
// Global terrain definitions.
//

#define HEIGHT_RESOLUTION           256
#define BRUSH_SIDES_PER_TERXEL      8

//=============================================
//
// Forward structure declarations.
//

typedef     struct      cTerrain_s          cTerrain_t;

//=============================================
//
// Main terrain structures.
//

typedef struct {
    int                     mContents;                                  // Contents of a height.
    int                     mSurfaceFlags;                              // Surfaceflags of a height.
} cHeightDetails_t;

typedef struct {
    cTerrain_t              *owner;                                     // Owning terrain.
    int                     mHx, mHy;                                   // Terxel coords of patch.
    byte                    *mHeightMap;                                // Pointer to the height map to use.
    byte                    mCornerHeights[4];                          // Heights at the corners of the patch.
    vec3_t                  mWorldCoords;                               // World coordinate offset of this patch.
    vec3_t                  mBounds[2];                                 // Mins and maxs of the patch for culling.
    int                     mNumBrushes;                                // Number of brushes to collide with in the patch.
    struct cbrush_s         *mPatchBrushData;                           // List of brushes that make up the patch.
    int                     mSurfaceFlags;                              // Surfaceflags of the heightshader.
    int                     mContentFlags;                              // Contents of the heightshader.
} cTerrainPatch_t;

struct cTerrain_s {
    byte                    *mHeightMap;                                // Pointer to byte array of height samples.
    void                    *mFlattenMap;                               // Pointer to byte array of flatten samples.
    int                     mWidth;                                     // Width of heightMap excluding the 1 pixel edge.
    int                     mHeight;                                    // Height of heightMap excluding the 1 pixel edge.
    int                     mArea;                                      // The area of heightMap excluding the 1 pixel edge.
    int                     mRealWidth;                                 // Width of heightMap including the 1 pixel edge.
    int                     mRealHeight;                                // Height of heightMap including the 1 pixel edge.
    int                     mRealArea;                                  // The area of heightMap including the 1 pixel edge.
    int                     mTerxels;                                   // Number of terxels per patch side.
    vec3_t                  mTerxelSize;                                // Vector to scale heightMap samples to real world coords.
    vec3_t                  mBounds[2];                                 // Real world bounds of terrain brush.
    vec3_t                  mSize;                                      // Size of terrain brush in real world coords excluding 1 patch edge.
    vec3_t                  mPatchSize;                                 // Size of each patch in the x and y directions only.
    float                   mPatchScalarSize;                           // Horizontal size of the patch.
    int                     mBlockWidth;                                // Width of heightfield on blocks.
    int                     mBlockHeight;                               // Height of heightfield on blocks.
    int                     mBlockCount;                                // Total amount of blocks.
    cTerrainPatch_t         *mPatches;                                  // Terrain patches.
    byte                    *mPatchBrushData;                           // Base memory from which the patch brush data is taken.
    qboolean                mHasPhysics;                                // Set to qtrue unless disabled.

    int                     mBaseWaterHeight;                           // Base water height in terxels.
    float                   mWaterHeight;                               // Real world height of the water.
    int                     mWaterContents;                             // Contents of the water shader.
    int                     mWaterSurfaceFlags;                         // Surface flags of the water shader.

    cHeightDetails_t        mHeightDetails[HEIGHT_RESOLUTION];          // Surfaceflags per height.
    vec3_t                  *mCoords;                                   // Temp storage for real world coords.
};

#endif // __CM_TERRAIN_H
