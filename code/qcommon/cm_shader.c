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

/*
==================
CM_ParseSurfaceParm

Update shader flags from
the predefined surface
info parameters.
==================
*/

static shaderInfoParm_t shaderInfoParms[] =
{
    // Game surface flags
    {"sky",                     -1,                 SURF_SKY,           0                   },  // emit light from an environment map
    {"slick",                   -1,                 SURF_SLICK,         0                   },

    {"nodamage",                -1,                 SURF_NODAMAGE,      0                   },
    {"noimpact",                -1,                 SURF_NOIMPACT,      0                   },  // don't make impact explosions or marks
    {"nomarks",                 -1,                 SURF_NOMARKS,       0                   },  // don't make impact marks, but still explode
    {"nodraw",                  -1,                 SURF_NODRAW,        0                   },  // don't generate a drawsurface (or a lightmap)
    {"nosteps",                 -1,                 SURF_NOSTEPS,       0                   },
    {"nodlight",                -1,                 SURF_NODLIGHT,      0                   },  // don't ever add dynamic lights
    {"nomiscents",              -1,                 SURF_NOMISCENTS,    0                   },

    // Game content flags
    {"nonsolid",                ~CONTENTS_SOLID,    0,                  0                   },  // special hack to clear solid flag
    {"nonopaque",               ~CONTENTS_OPAQUE,   0,                  0                   },  // special hack to clear opaque flag
    {"lava",                    ~CONTENTS_SOLID,    0,                  CONTENTS_LAVA       },  // very damaging
    {"water",                   ~CONTENTS_SOLID,    0,                  CONTENTS_WATER      },
    {"fog",                     ~CONTENTS_SOLID,    0,                  CONTENTS_FOG        },  // carves surfaces entering
    {"playerclip",              ~CONTENTS_SOLID,    0,                  CONTENTS_PLAYERCLIP },
    {"monsterclip",             ~CONTENTS_SOLID,    0,                  CONTENTS_MONSTERCLIP},
    {"botclip",                 ~CONTENTS_SOLID,    0,                  CONTENTS_BOTCLIP    },  // for bots
    {"shotclip",                ~CONTENTS_SOLID,    0,                  CONTENTS_SHOTCLIP   },
    {"trigger",                 ~CONTENTS_SOLID,    0,                  CONTENTS_TRIGGER    },
    {"nodrop",                  ~CONTENTS_SOLID,    0,                  CONTENTS_NODROP     },  // don't drop items or leave bodies (death fog, lava, etc)
    {"terrain",                 ~CONTENTS_SOLID,    0,                  CONTENTS_TERRAIN    },  // use special terrain collsion
    {"ladder",                  ~CONTENTS_SOLID,    0,                  CONTENTS_LADDER     },  // climb up in it like water
    {"abseil",                  ~CONTENTS_SOLID,    0,                  CONTENTS_ABSEIL     },  // can abseil down this brush
    {"outside",                 ~CONTENTS_SOLID,    0,                  CONTENTS_OUTSIDE    },  // volume is considered to be in the outside (i.e. not indoors)

    {"detail",                  -1,                 0,                  CONTENTS_DETAIL     },  // don't include in structural bsp
    {"trans",                   -1,                 0,                  CONTENTS_TRANSLUCENT},  // surface has an alpha component
};

static void CM_ParseSurfaceParm(dshader_t *shader, char **text)
{
    int     i;
    char    *token;

    token = COM_ParseExt(text, qfalse);
    if(!token){
        Com_Printf(S_COLOR_YELLOW "WARNING: Missing surface in shader: \"%s\".\n", shader->shader);
        return;
    }

    // Iterate through all surface parameters.
    // Update shader flags when we get a match.
    for(i = 0; i < ARRAY_LEN(shaderInfoParms); i++){
        if(Q_stricmp(shaderInfoParms[i].name, token) == 0){
            shader->surfaceFlags |= shaderInfoParms[i].surfaceFlags;
            shader->contentFlags |= shaderInfoParms[i].contents;
            shader->contentFlags &= shaderInfoParms[i].clearSolid;
            return;
        }
    }

    // Surface not found in the list.
    Com_DPrintf(S_COLOR_RED "CM_ParseSurfaceParm: No such surface defined: %s\n", token);
}

/*
==================
CM_ParseMaterial

Parse which material is
used in this shader.
==================
*/

#define NUM_MATERIALS           32

static const char *materialNames[NUM_MATERIALS] =
{
    "none",
    "solidwood",
    "hollowwood",
    "solidmetal",
    "hollowmetal",
    "shortgrass",
    "longgrass",
    "dirt",
    "sand",
    "gravel",
    "glass",
    "concrete",
    "marble",
    "water",
    "snow",
    "ice",
    "flesh",
    "mud",
    "bpglass",
    "dryleaves",
    "greenleaves",
    "fabric",
    "canvas",
    "rock",
    "rubber",
    "plastic",
    "tiles",
    "carpet",
    "plaster",
    "shatterglass",
    "armor",
    "computer"
};

static void CM_ParseMaterial(dshader_t *shader, char **text)
{
    int     i;
    char    *token;

    token = COM_ParseExt(text, qfalse);
    if(!token){
        Com_Printf(S_COLOR_YELLOW "WARNING: Missing material in shader: \"%s\".\n", shader->shader);
        return;
    }

    // Iterate through all materials.
    // Update surface flags when we get a match.
    for(i = 0; i < NUM_MATERIALS; i++){
        if(Q_stricmp(materialNames[i], token) == 0){
            shader->surfaceFlags |= i;
            return;
        }
    }

    // Material not found in the list.
    Com_DPrintf(S_COLOR_RED "CM_ParseMaterial: No such material defined: %s\n", token);
}

/*
==================
CM_ParseShader

Parses additional shader information.
Shaders are only parsed if they are
actually used on the server.

This extracts all the info from the
shader required for physics and
collision.
==================
*/

static void CM_ParseShader(dshader_t *shader, char **text)
{
    char        *token;

    //
    // Iterate through the shader.
    //
    while(1){
        token = COM_ParseExt(text, qtrue);

        //
        // Last token found?
        //
        if(token[0] == '}'){
            break;
        }
        //
        // Material load.
        //
        else if(Q_stricmp(token, "material") == 0 || Q_stricmp(token, "q3map_material") == 0){
            CM_ParseMaterial(shader, text);
        }
        //
        // Surface parameters.
        //
        else if(Q_stricmp(token, "surfaceParm") == 0){
            CM_ParseSurfaceParm(shader, text);
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
CM_ParseShaderFile

Parses .shader file, searches individual
shaders that need to be parsed.
==================
*/

static void CM_ParseShaderFile(char **text)
{
    int         i;
    char        *shaderNameToken;
    char        *genericToken;
    dshader_t   *shader;

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

        // Is this shader already present?
        shader = NULL;
        for(i = 0; i < cm.numShaders; i++){
            if(strcmp(shaderNameToken, cm.shaders[i].shader) == 0){
                shader = &cm.shaders[i];
                break;
            }
        }

        // Continue parsing the shader.
        genericToken = COM_ParseExt(text, qtrue);
        if(genericToken[0] == '{'){
            if(shader != NULL){
                // Parse this shader.
                CM_ParseShader(shader, text);
            }else{
                // No need to continue parsing information
                // if the shader isn't used.
                SkipBracedSection(text, 1);
            }
        }
    }
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

void CM_LoadShaderFiles(void)
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

    Com_DPrintf("--- Clipmap Shader Initialization ---\n");

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

        // Copy file to a temporary buffer.
        shaderBuf = Z_Malloc(fileLength + 1);
        shaderBufOrg = shaderBuf;
        strncpy(shaderBuf, fileBuf.v, fileLength);

        // We are done with the file, free it.
        FS_FreeFile(fileBuf.v);

        // Compress the contents.
        COM_Compress(shaderBuf);

        // Parse this shader file.
        CM_ParseShaderFile(&shaderBuf);

        // Free temporary buffer.
        Z_Free(shaderBufOrg);
    }

    //
    // Free up memory.
    //

    FS_FreeFileList(shaderFiles);

    Com_DPrintf("--- Shader Initialization Complete ---\n");
}

/*
==================
CM_FindShaderByName

Searches for shader by name.
Returns existing shader if
it can be found, or NULL
upon error.
==================
*/

dshader_t *CM_FindShaderByName(const char *name)
{
    int i;

    for(i = 0; i < cm.numShaders; i++){
        if(Q_stricmp(cm.shaders[i].shader, name) == 0){
            // Shader found.
            return &cm.shaders[i];
        }
    }

    // Not found, return NULL by default.
    return NULL;
}
