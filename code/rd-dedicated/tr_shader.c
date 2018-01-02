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
// tr_shader.c - Deals with the parsing and definition of shaders.

#include "tr_local.h"

/*
==================
R_AllocShader

Allocates shader in the shader list.
Returns pointer to new member.
==================
*/

static shader_t *R_AllocShader()
{
    shader_t    *shader;

    if(tr.numShaders == MAX_SHADERS){
        return NULL;
    }

    shader = Hunk_Alloc(sizeof(shader_t), h_low);
    tr.shaders[tr.numShaders] = shader;
    tr.numShaders++;

    return shader;
}

/*
==================
R_FindServerShader

Allocates shader in the shader list.
Returns pointer to new member.
==================
*/

shader_t *R_FindServerShader(const char *name)
{
    shader_t    *shader;
    int         i;

    //
    // See if the shader is already loaded.
    //
    for(i = 0; i < tr.numShaders; i++){
        if(tr.shaders[i] != NULL){
            // Does the name of the found shader match?
            if(!Q_stricmp(tr.shaders[i]->name, name)){
                // It does, meaning we found it.
                return tr.shaders[i];
            }
        }
    }

    //
    // Allocate a new shader.
    //
    if((shader = R_AllocShader()) == NULL){
        Com_DPrintf(S_COLOR_YELLOW "R_FindServerShader: R_AllocShader() failed for \"%s\".\n", name);
        return NULL;
    }

    // Set info after the shader has been successfully initialized.
    Q_strncpyz(shader->name, name, sizeof(shader->name));

    return shader;
}
