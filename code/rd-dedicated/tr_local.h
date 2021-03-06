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

#include "tr_mdx_format.h"

//=============================================

#define     FILE_HASH_SIZE                  1024
#define     MAX_MOD_KNOWN                   1024

#define     SHADERNUM_BITS                  14
#define     MAX_SHADERS                     (1<<SHADERNUM_BITS)
#define     MAX_SHADER_FILES                4096
#define     MAX_HITDATA_ENTRIES             128

#define     MAX_SKINS                       1024
#define     MAX_SKIN_SURFACES               128

typedef enum {
    MOD_BAD,
    MOD_MDXM,
    MOD_MDXA
} modtype_t;

typedef struct {
    char                    name[MAX_QPATH];
    modtype_t               type;
    int                     index;              // model = tr.models[model->index]

    void                    *modelData;         // Only if type == MOD_GL2A (Ghoul II animation file) or type == MOD_GL2M (Ghoul II mesh file).

    int                     numLods;
} model_t;

typedef struct {
    char                    name[MAX_QPATH];
    int                     hitMaterial;
    int                     hitLocation;
} shader_t;

typedef struct {
    char                    name[MAX_QPATH];
    shader_t                *shader;
} skinSurface_t;

typedef struct {
    char                    name[MAX_QPATH];

    skinSurface_t           *surfaces[MAX_SKIN_SURFACES];
    int                     numSurfaces;
} skin_t;

typedef struct {
    byte                    *loc;
    int                     width;
    int                     height;
    char                    name[MAX_QPATH];
} hitRegData_t;

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

    skin_t                  *skins[MAX_SKINS];
    int                     numSkins;

    shader_t                *shaders[MAX_SHADERS];
    int                     numShaders;

    hitRegData_t            hitRegData[MAX_HITDATA_ENTRIES];
    int                     hitRegDataCount;
} trGlobals_t;


extern trGlobals_t          tr;

//=============================================

//
// tr_image_png.c
//

void                R_LoadPNG                       ( const char *name, byte **pic, int *width, int *height );

//
// tr_main.c
//

// CVARs.
extern cvar_t       *r_verbose;                     // Used for verbose debug spew.

// Functions.
void                R_Init                          ( void );

//
// tr_model.c
//

qhandle_t           RE_RegisterServerModel          ( const char *name );
model_t             *R_GetModelByHandle             ( qhandle_t index );
model_t             *R_AllocModel                   ( void );

//
// tr_shader.c
//

shader_t            *R_FindServerShader             ( const char *name );
void                R_ShaderInit                    ( void );

//
// tr_skin.c
//

qhandle_t           RE_RegisterServerSkin           ( const char *name, int numPairs, const char *skinPairs );
skin_t              *R_GetSkinByHandle              ( qhandle_t index );

#endif // __TR_LOCAL_H
