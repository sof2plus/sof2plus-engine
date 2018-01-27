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
// mdx_format.h - MDX file format (typically uses file extension GLX for mesh, and GLA for anim/skeleton file).

#ifndef __MDX_FORMAT_H
#define __MDX_FORMAT_H

#include "../qcommon/q_shared.h"

#define MDXM_IDENT          (('M'<<24)+('G'<<16)+('L'<<8)+'2')
#define MDXA_IDENT          (('A'<<24)+('G'<<16)+('L'<<8)+'2')

//
// normal version numbers
//

#define MDXM_VERSION 6
#define MDXA_VERSION 6

//
// In-game stuff, never generated by Carcass.
//
#define G2SURFACEFLAG_NODESCENDANTS 0x00000100

//
// Triangle side-ordering stuff for tags...
//

#define fG2_BONEWEIGHT_RECIPROCAL_MULT  ((float)(1.0f/1023.0f))
#define iG2_BITS_PER_BONEREF            5
#define iMAX_G2_BONEWEIGHTS_PER_VERT    4       // can't just be blindly increased, affects cache size etc
#define iG2_BONEWEIGHT_TOPBITS_AND      0x300   // 2 bits, giving 10 total, or 10 bits, for 1023/1024 above

#define iG2_BONEWEIGHT_TOPBITS_SHIFT    ((iG2_BITS_PER_BONEREF * iMAX_G2_BONEWEIGHTS_PER_VERT) - 8) // 8 bits because of 8 in the BoneWeight[] array entry

//
// mdxaCompQuatBone_t
//
// For compressing and uncompressing bones.
//

typedef struct {
    unsigned char Comp[14]; // MC_COMP_BYTES
} mdxaCompQuatBone_t;

//
// mdxHeader_t
//
// This contains the header for the file,
// with sanity checking and version checking,
// plus number of lod's to be expected.
//

typedef struct mdxmHeader_s {
    //
    // ( first 3 fields are same format as MD3/MDR so we can apply easy model-format-type checks )
    //
    int         ident;              // "IDP3" = MD3, "RDM5" = MDR, "2LGM"(GL2 Mesh) = MDX   (cruddy char order I know, but I'm following what was there in other versions)
    int         version;            // 1,2,3 etc as per format revision
    char        name[MAX_QPATH];    // model name (eg "models/players/marine.glm")  // note: extension supplied
    char        animName[MAX_QPATH];// name of animation file this mesh requires    // note: extension missing
    int         animIndex;          // filled in by game (carcass defaults it to 0)

    int         numBones;           // (for ingame version-checks only, ensure we don't ref more bones than skel file has)

    int         numLODs;
    int         ofsLODs;

    int         numSurfaces;        // now that surfaces are drawn hierarchically, we have same # per LOD
    int         ofsSurfHierarchy;

    int         ofsEnd;             // EOF, which of course gives overall file size
} mdxmHeader_t;

typedef struct mdxaHeader_s {
    //
    // ( first 3 fields are same format as MD3/MDR so we can apply easy model-format-type checks )
    //
    int         ident;              //  "IDP3" = MD3, "RDM5" = MDR, "2LGA"(GL2 Anim) = MDXA
    int         version;            // 1,2,3 etc as per format revision
    //
    char        name[MAX_QPATH];    // GLA name (eg "skeletons/marine") // note: extension missing
    float       fScale;             // will be zero if build before this field was defined, else scale it was built with

    // frames and bones are shared by all levels of detail
    //
    int         numFrames;
    int         ofsFrames;          // points at mdxaFrame_t array
    int         numBones;           // (no offset to these since they're inside the frames array)
    int         ofsCompBonePool;    // offset to global compressed-bone pool that all frames use
    int         ofsSkel;            // offset to mdxaSkel_t info

    int         ofsEnd;             // EOF, which of course gives overall file size

} mdxaHeader_t;

//
// mdxaSkel_t
//
// Contains hierarchical info for bones.
//

typedef struct mdxaSkelOffsets_s {
    int offsets[1];                 // variable sized (mdxaHeader_t->numBones), each offset points to an mdxaSkel_t below
} mdxaSkelOffsets_t;

typedef struct mdxaSkel_s {
    char        name[MAX_QPATH];    // name of bone
    unsigned int flags;
    int         parent;             // index of bone that is parent to this one, -1 = NULL/root
    mdxaBone_t  BasePoseMat;        // base pose
    mdxaBone_t  BasePoseMatInv;     // inverse, to save run-time calc
    int         numChildren;        // number of children bones
    int         children[1];        // [mdxaSkel_t->numChildren] (variable sized)
} mdxaSkel_t;   // struct size = (int)( &((mdxaSkel_t *)0)->children[ mdxaSkel_t->numChildren ] );

//
// mdxaIndex_s
//
// (offset @ mdxaHeader_t->ofsFrames)
// Array of 3 byte indices here (hey, 25% saving over 4-byte really adds up)..
// Access as follows to get the index for a given <iFrameNum, iBoneNum>
//
// (iFrameNum * mdxaHeader_t->numBones * 3) + (iBoneNum * 3)
//
// Then read the int at that location and AND it with 0x00FFFFFF.
// This struct is used for easy searches.
//
typedef struct mdxaIndex_s {
    int iIndex;                     // this struct for pointing purposes, need to and with 0x00FFFFFF to be meaningful
} mdxaIndex_t;

//
// mdxmHierarchyOffsets_t
//
// Variable sized (mdxmHeader_t->numSurfaces).
// Each offset points to a mdxmSurfHierarchy_t below.
//

typedef struct {
    int offsets[1];
} mdxmHierarchyOffsets_t;

//
// mdxmSurfHierarchy_t
//
// Contains hierarchical info for surfaces.
//

typedef struct mdxmSurfHierarchy_s {
    char        name[MAX_QPATH];
    unsigned int flags;
    char        shader[MAX_QPATH];
    int         shaderIndex;        // for in-game use (carcass defaults to 0)
    int         parentIndex;        // this points to the index in the file of the parent surface. -1 if null/root
    int         numChildren;        // number of surfaces which are children of this one
    int         childIndexes[1];    // [mdxmSurfHierarch_t->numChildren] (variable sized)
} mdxmSurfHierarchy_t;  // struct size = (int)( &((mdxmSurfHierarch_t *)0)->childIndexes[ mdxmSurfHierarch_t->numChildren ] );

//
// mdxLOD_t
//
// This contains the header for this LOD.
// Contains number of surfaces, offset to surfaces and offset to next LOD.
// Surfaces are shader sorted, so each surface = 1 shader.
//

typedef struct mdxmLOD_s {
    int         ofsEnd;             // offset to next LOD
} mdxmLOD_t;

//
// mdxmLODSurfOffset_t
//
// Added in GLM version 3 for in-game use.
//

typedef struct mdxmLODSurfOffset_s {
    int offsets[1];     // variable sized (mdxmHeader_t->numSurfaces), each offset points to surfaces below
} mdxmLODSurfOffset_t;

//
// mdxSurface_t
//
// Reuse of header format containing surface name, number of bones,
// offset to poly data and number of polys, offset to vertex information,
// and number of verts.
// NOTE: offsets are relative to this header.
//

typedef struct mdxmSurface_s {
    int         ident;              // this one field at least should be kept, since the game-engine may switch-case (but currently=0 in carcass)

    int         thisSurfaceIndex;   // 0...mdxmHeader_t->numSurfaces-1 (because of how ingame renderer works)

    int         ofsHeader;          // this will be a negative number, pointing back to main header

    int         numVerts;
    int         ofsVerts;

    int         numTriangles;
    int         ofsTriangles;

    // Bone references are a set of ints representing all the bones
    // present in any vertex weights for this surface.  This is
    // needed because a model may have surfaces that need to be
    // drawn at different sort times, and we don't want to have
    // to re-interpolate all the bones for each surface.

    int         numBoneReferences;
    int         ofsBoneReferences;

    int         ofsEnd;             // next surface follows
} mdxmSurface_t;

//
// mdxTriangle_t
//
// Contains indexes into verts. One struct entry per poly.
//

typedef struct mdxmTriangle_s {
    int         indexes[3];
} mdxmTriangle_t;

//
// mdxVertex_t
//
// this is an array with number of verts from the surface definition as its bounds.
// It contains normal info, texture coors and number of weightings for this bone.
//

typedef struct mdxmVertex_s {
    vec3_t          normal;
    vec3_t          vertCoords;

    // packed int...
    unsigned int    uiNmWeightsAndBoneIndexes;  // 32 bits.  format:
                                                // 31 & 30:  0..3 (= 1..4) weight count
                                                // 29 & 28 (spare)
                                                //  2 bit pairs at 20,22,24,26 are 2-bit overflows from 4 BonWeights below (20=[0], 22=[1]) etc)
                                                //  5-bits each (iG2_BITS_PER_BONEREF) for boneweights
    // Effectively a packed int, each bone weight converted from 0..1 float to 0..255 int...
    // Promote each entry to float and multiply by fG2_BONEWEIGHT_RECIPROCAL_MULT to convert.
    byte            BoneWeightings[iMAX_G2_BONEWEIGHTS_PER_VERT];   // 4
} mdxmVertex_t;

//
// mdxmVertexTexCoord_s
//
// Separated from mdxmVertex_t structure for cache reasons.
//

typedef struct mdxmVertexTexCoord_s {
    vec2_t          texCoords;
} mdxmVertexTexCoord_t;

#endif // __MDX_FORMAT_H