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
// cm_shader.c

#include "cm_local.h"

// Local variable definitions.
static cShader_t    *cmShaders[MAX_SHADERS];
static int          numShaders                  = 0;

//=============================================================================

/*
==================
CM_AllocShader

Allocates shader in the shader list.
Returns pointer to new member.
==================
*/

static cShader_t *CM_AllocShader(void)
{
    cShader_t   *shader;

    if(numShaders == MAX_SHADERS){
        return NULL;
    }

    shader = Hunk_Alloc(sizeof(cShader_t), h_high);
    cmShaders[numShaders] = shader;
    numShaders++;

    return shader;
}

/*
==================
CM_LoadShaderFiles

Finds and loads all .shader files,
combining them into a single
large text block that can be
scanned for shader names.
==================
*/

static void CM_LoadShaderFiles(void)
{
    char        **shaderFiles;
    union {
        int             *i;
        void            *v;
    } fileBuf;
    int         fileLength;
    int         numShaderFiles;
    cShader_t   *shader;

    int         i;
    char        *p;
    char        *token;

    char        fileName[MAX_QPATH];
    char        shaderName[MAX_QPATH];
    int         shaderLine;

    //
    // Load up all .shader files.
    //
    shaderFiles = FS_ListFiles("shaders", ".shader", &numShaderFiles);

    if(!shaderFiles || !numShaderFiles){
        Com_Printf(S_COLOR_YELLOW "WARNING: No shader files found!\n");
        return;
    }

    if(numShaderFiles > MAX_SHADER_FILES){
        Com_Printf(S_COLOR_YELLOW "WARNING: %d shaders are found, this exceeds the limit of %d shaders!\n", numShaderFiles, MAX_SHADER_FILES);
        Com_Printf(S_COLOR_YELLOW "%d shaders won't be parsed.\n", numShaderFiles - MAX_SHADER_FILES);
        numShaderFiles = MAX_SHADER_FILES;
    }

    //
    // Load and parse shader files.
    //
    for(i = 0; i < numShaderFiles; i++){
        // Load the file.
        Com_sprintf(fileName, sizeof(fileName), "shaders/%s", shaderFiles[i]);
        Com_DPrintf("... loading \"%s\"\n", fileName);
        fileLength = FS_ReadFile(fileName, &fileBuf.v);

        if(!fileBuf.i){
            Com_Error(ERR_FATAL, "Failed to load shader \"%s\".", fileName);
        }

        // Do a simple check on the shader structure.
        p = fileBuf.v;
        COM_BeginParseSession(fileName);
        while(1){
            token = COM_ParseExt(&p, qtrue);

            if(!*token){
                break;
            }

            Q_strncpyz(shaderName, token, sizeof(shaderName));
            shaderLine = COM_GetCurrentParseLine();

            token = COM_ParseExt(&p, qtrue);
            if(token[0] != '{' || token[1] != '\0'){
                Com_Printf(S_COLOR_YELLOW "WARNING: Ignoring shader file %s. Shader \"%s\" on line %d missing opening brace",
                           fileName, shaderName, shaderLine);

                if(token[0]){
                    Com_Printf(S_COLOR_YELLOW " (found \"%s\" on line %d)", token, COM_GetCurrentParseLine());
                }
                Com_Printf(S_COLOR_YELLOW ".\n");
                FS_FreeFile(fileBuf.v);
                break;
            }

            if(!SkipBracedSection(&p, 1)){
                Com_Printf(S_COLOR_YELLOW "WARNING: Ignoring shader file %s. Shader \"%s\" on line %d missing closing brace.\n",
                           fileName, shaderName, shaderLine);
                FS_FreeFile(fileBuf.v);
                break;
            }
        }

        // Don't continue if this file is already freed
        // due to it being invalid.
        if(!fileBuf.v){
            continue;
        }

        // Allocate a new shader.
        shader = CM_AllocShader();

        // If allocation fails, we've hit our shader limit.
        if(shader == NULL){
            Com_Printf(S_COLOR_RED "ERROR: Allocation failed for shader. Not continuing to load more shaders.\n");
            break;
        }

        // Copy shader text to the new shader.
        shader->shaderText = Hunk_Alloc(fileLength + 1, h_high);
        strncpy(shader->shaderText, fileBuf.v, fileLength);

        // We are done with the file, free it.
        FS_FreeFile(fileBuf.v);

        // Compress the shader text contents.
        COM_Compress(shader->shaderText);
    }

    //
    // Free up memory.
    //

    FS_FreeFileList(shaderFiles);
}

/*
==================
CM_LoadShaderText

Loads all .shader files so it can
be accessed by the server.
==================
*/

void CM_LoadShaderText()
{
    Com_DPrintf("Loading shader text .....\n");
    CM_LoadShaderFiles();
}
