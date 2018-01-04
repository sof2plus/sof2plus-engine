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
    shader->hitLocation = -1;
    shader->hitMaterial = -1;

    return shader;
}

/*
=============================================
----------------------
Shader parse functions
----------------------
=============================================
*/

/*
==================
R_FindHitData

See if we already registered this
hit location or material data.
==================
*/

static int R_FindHitData(const char *fileName)
{
    int i;

    // Iterate through the already registered
    // hit materials.
    for(i = 0; i < tr.hitRegDataCount; i++)
    {
        if(!Q_stricmp(tr.hitRegData[i].name, fileName)){
            return i;
        }
    }

    return -1;
}

/*
==================
R_ParseHitData

Parses hit data location or
material file (dependant on
location qboolean).

Returns index to global
hitRegData array or -1
upon failure.
==================
*/

static int R_ParseHitData(char *token, shader_t *shader, qboolean location)
{
    char            fileName[MAX_QPATH];
    byte            *buffer;
    hitRegData_t    *hitRegData;
    int             numPixels;
    int             hitData;

    // Have we already loaded the hit data location or material?
    hitData = R_FindHitData(token);
    if(hitData == -1){
        // We haven't, load it in and register it.
        if(tr.hitRegDataCount == MAX_HITDATA_ENTRIES){
            Com_Error(ERR_DROP, "Not enough entry space for hit %s data file \"%s\".",
                                 (location) ? "location" : "material", token);
        }

        // Append the hit data file extension (hit files are always PNG).
        Com_sprintf(fileName, sizeof(fileName), "%s.png", token);

        // Load that file in.
        hitRegData  = &tr.hitRegData[tr.hitRegDataCount];
        buffer      = 0;

        R_LoadPNG(fileName, &buffer, &hitRegData->width, &hitRegData->height);
        if(buffer){
            // Determine number of pixels.
            numPixels = hitRegData->width * hitRegData->height;

            // Allocate data on the hunk to store the image.
            hitRegData->loc = Hunk_Alloc(numPixels, h_low);

            // Copy data into new space.
            memcpy(hitRegData->loc, buffer, numPixels);

            // Also store the name of the image.
            Q_strncpyz(hitRegData->name, token, sizeof(hitRegData->name));

            // Now free the original space we loaded it into.
            Z_Free(buffer);

            // Hit material slot used, take note of this slot and increase count.
            hitData = tr.hitRegDataCount;
            tr.hitRegDataCount++;
        }else{
            Com_Printf(S_COLOR_YELLOW "WARNING: Missing hit %s data file \"%s\" in shader \"%s\".\n",
                                       (location) ? "location" : "material", token, shader->name);
        }
    }

    // Return the index in the hitRegData array.
    return hitData;
}

/*
==================
R_ParseShader

Parses shader. Searches only for
Ghoul II relevant information
in the shader.

Only if such info is found, the
shader is then registered on the
server.
==================
*/

static void R_ParseShader(char *text, char *shaderName)
{
    char        *token;
    shader_t    *shader;


    //
    // Iterate through the shader.
    //
    shader = NULL;
    while(1){
        token = COM_ParseExt(text, qtrue);

        //
        // Last token found?
        //
        if(token[0] == '}'){
            break;
        }
        //
        // Location hit mesh load.
        //
        else if(Q_stricmp(token, "hitLocation") == 0){
            // Grab the filename of the hit location texture.
            token = COM_ParseExt(text, qfalse);
            if(token[0] == 0){
                break;
            }

            // We've got a valid file name for this image.
            // Is the parent shader already found/registered?
            if(shader == NULL){
                // Find or allocate it now.
                shader = R_FindServerShader(shaderName);

                // Don't continue unless we've got a valid shader.
                if(shader == NULL){
                    return;
                }
            }

            // Parse the hit data location.
            shader->hitLocation = R_ParseHitData(token, shader, qtrue);
        }
        //
        // Location hit material mesh load.
        //
        else if(Q_stricmp(token, "hitMaterial") == 0){
            // Grab the filename of the hit location texture.
            token = COM_ParseExt(text, qfalse);
            if(token[0] == 0){
                break;
            }

            // We've got a valid file name for this image.
            // Is the parent shader already found/registered?
            if(shader == NULL){
                // Find or allocate it now.
                shader = R_FindServerShader(shaderName);

                // Don't continue unless we've got a valid shader.
                if(shader == NULL){
                    return;
                }
            }

            // Parse the hit data material.
            shader->hitMaterial = R_ParseHitData(token, shader, qfalse);
        }
        //
        // Other group.
        //
        else if(token[0] == '{'){
            SkipBracedSection(text, 1);
        }
    }
}

/*
==================
R_ParseShaderFile

Parses .shader file, searches individual
shaders to be parsed.
==================
*/

static void R_ParseShaderFile(char **text)
{
    char    *shaderNameToken;
    char    *genericToken;
    char    shaderName[MAX_QPATH];

    //
    // Iterate through the shader file.
    //
    while(1){
        // We start off by getting the current shader name.
        shaderNameToken = COM_ParseExt(text, qtrue);
        if(!shaderNameToken[0]){
            // Stop if there's no data left.
            break;
        }

        // Keep a copy of the shader name in case we need it.
        Q_strncpyz(shaderName, shaderNameToken, sizeof(shaderName));

        // Continue parsing the shader.
        genericToken = COM_ParseExt(text, qtrue);
        if(genericToken[0] == '{'){
            R_ParseShader(text, &shaderName);
        }
    }
}

/*
==================
R_ShaderInit

Searches all shader files found in the
"shaders" directory to parse them
individually.
==================
*/

void R_ShaderInit()
{
    char    **shaderFiles;
    union {
        int             *i;
        void            *v;
    } fileBuf;
    int     fileLength;
    char    *shaderBuf;
    void    *shaderBufOrg;
    int     numShaderFiles;

    int     i;
    char    *p;
    char    *token;

    char    fileName[MAX_QPATH];
    char    shaderName[MAX_QPATH];
    int     shaderLine;

    Com_Printf("------- Shader Initialization -------\n");

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
        Com_Printf("... loading \"%s\"\n", fileName);
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

        // Copy file to a temporary buffer.
        shaderBuf = Z_Malloc(fileLength + 1);
        shaderBufOrg = shaderBuf;
        strncpy(shaderBuf, fileBuf.v, fileLength);

        // We are done with the file, free it.
        FS_FreeFile(fileBuf.v);

        // Compress the contents.
        COM_Compress(shaderBuf);

        // Parse this shader file.
        if(strcmp(fileName, "shaders/average_sleeves.shader") == 0)
            R_ParseShaderFile(&shaderBuf);

        // Free temporary buffer.
        Z_Free(shaderBufOrg);
    }

    //
    // Free up memory.
    //

    FS_FreeFileList(shaderFiles);

    Com_Printf("--- Shader Initialization Complete ---\n");
}
