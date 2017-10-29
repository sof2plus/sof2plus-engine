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

#endif // __MDX_FORMAT_H
