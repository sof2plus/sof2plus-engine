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
// tr_local.h

#ifndef __TR_LOCAL_H
#define __TR_LOCAL_H

#include "../qcommon/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/qcommon.h"
#include "mdx_format.h"

//=============================================

typedef struct {

} world_t;

//=============================================

#define     FILE_HASH_SIZE                  1024
#define     MAX_MOD_KNOWN                   1024

typedef struct {
    int                     key;
    char                    *value;
} cachedModel_t;

typedef struct {
    cachedModel_t           **models;
    int                     numModels;
} cachedModels_t;

typedef struct modelHash_s {
    char                    name[MAX_QPATH];
    qhandle_t               handle;
    struct                  modelHash_s *next;
} modelHash_t;

typedef enum {
    MOD_BAD,
    MOD_BRUSH,// FIXME BOE REVIEW
    MOD_MESH, // FIXME BOE REVIEW
    MOD_MDXM,
    MOD_MDXA
} modtype_t;

typedef struct {
    char                    name[MAX_QPATH];
    modtype_t               type;
    int                     index;              // model = tr.models[model->index]

    int                     dataSize;           // just for listing purposes
    void                    *modelData;         // Only if type == MOD_GL2A (Ghoul II animation file) or type == MOD_GL2M (Ghoul II mesh file).

    int                     numLods;
} model_t;

//=============================================

/*
** trGlobals_t
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct {
    model_t                 *models[MAX_MOD_KNOWN];
    int                     numModels;

    world_t                 bspModels[MAX_SUB_BSP];
    int                     numBSPModels;
} trGlobals_t;

extern      trGlobals_t                     tr;

//=============================================

//
// tr_model.c
//

void                RE_RegisterMedia_LevelLoadBegin ( const char *psMapName );
qhandle_t           RE_RegisterServerModel          ( const char *name );
void                R_ModelInit                     ( void );
model_t             *R_GetModelByHandle             ( qhandle_t index );
model_t             *R_AllocModel                   ( void );

#endif // __TR_LOCAL_H
