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
// tr_model.c - Model loading and caching.

#include "tr_local.h"

static char             sPrevMapName[MAX_QPATH]         = { 0 };
static int              giRegisterMedia_CurrentLevel    = 0;

static cachedModels_t   cachedModels                    = { 0 };
static modelHash_t      *mhHashTable[FILE_HASH_SIZE]    = { 0 };

//=============================================================================

/*
==================
R_GenerateHashValue

Returns a hash value for the filename.
==================
*/

static long RE_GenerateHashValue(const char *fname, const int size)
{
    int     i;
    long    hash;
    char    letter;

    hash = 0;
    i = 0;

    // Iterate through filename.
    while(fname[i] != '\0'){
        letter = tolower((unsigned char)fname[i]);

        // Don't include extension.
        if(letter =='.'){
            break;
        }

        // Ensure directories are always divided with a forward slash.
        if(letter =='\\'){
            letter = '/';
        }

        hash += (long)(letter)*(i+119);
        i++;
    }

    hash &= (size-1);
    return hash;
}

/*
==================
RE_RegisterModels_GetDiskFile

Returns qtrue if loaded, and sets the supplied boolean to true if it was from cache (instead of disk).
==================
*/

static qboolean RE_RegisterModels_GetDiskFile(const char *psModelFileName, void **ppvBuffer, qboolean *pqbAlreadyCached)
{
    char            sModelName[MAX_QPATH];
    cachedModel_t   *modelBin;

    assert(cachedModels.models != NULL);

    Q_strncpyz(sModelName,psModelFileName,sizeof(sModelName));
    Q_strlwr(sModelName);

    // FIXME BOE
    return qfalse;
}

/*
==================
RE_RegisterMedia_LevelLoadBegin


==================
*/

void RE_RegisterMedia_LevelLoadBegin(const char *psMapName)
{
    tr.numBSPModels = 0;

    // Only bump level number if we're not on the same level.
    if(Q_stricmp(psMapName,sPrevMapName)){
        Q_strncpyz(sPrevMapName, psMapName, sizeof(sPrevMapName));
        giRegisterMedia_CurrentLevel++;
    }
}

/*
==================
RE_RegisterServerModel

Routine is used by the server to handle ghoul2 instance models.
==================
*/

qhandle_t RE_RegisterServerModel(const char *name)
{
    model_t         *mod;
    unsigned int    *buf;
    int             lod;
    int             ident;
    qboolean        loaded;
    int             hash;
    modelHash_t     *mh;
    int             numLoaded = 0;
    int             iLODStart = 0;

    // Must be a valid name.
    if(!name || !name[0]){
        return 0;
    }
    if(strlen(name) >= MAX_QPATH){
        return 0;
    }

    // Generate a hash based on file name.
    hash = RE_GenerateHashValue(name, FILE_HASH_SIZE);

    //
    // See if the model is already loaded.
    //
    for(mh = mhHashTable[hash]; mh; mh=mh->next){
        if(Q_stricmp(mh->name, name) == 0){
            return mh->handle;
        }
    }

    // Only continue if we have free slots left.
    if((mod = R_AllocModel()) == NULL){
        return 0;
    }

    // Set the name after the model has been successfully loaded.
    Q_strncpyz(mod->name, name, sizeof(mod->name));
    if(FS_IsExt(name, ".md3", strlen(name))){
        // This loads the md3s in reverse so they can be biased.
        iLODStart = MD3_MAX_LODS - 1;
    }
    mod->numLods = 0;

    //
    // Load the files.
    //
    for(lod = iLODStart; lod >= 0; lod--){
        char        filename[1024];
        qboolean    bAlreadyCached;

        memset(filename, 0, sizeof(filename));
        strncpy(filename, name, sizeof(filename));

        if(lod != 0){
            char namebuf[80];

            if(strrchr(filename, '.')){
                *strrchr(filename, '.') = 0;
            }
            snprintf(namebuf, sizeof(namebuf), "_%d.md3", lod);
            strncat(filename, namebuf, sizeof(filename) - strlen(filename) - 1);
        }

        bAlreadyCached = qfalse;
        if(!RE_RegisterModels_GetDiskFile(filename, (void **)&buf, &bAlreadyCached)){
            continue;
        }
    }
}

/*
==================
R_ModelInit

Clears models and (re-)initializes the main NULL model.
==================
*/

void R_ModelInit()
{
    model_t     *mod;

    // Leave a space for NULL model.
    tr.numModels = 0;
    memset(mhHashTable, 0, sizeof(mhHashTable));

    mod = R_AllocModel();
    mod->type = MOD_BAD;
}

/*
==================
R_AllocModel

Allocates model in the model list.
Returns pointer to new member.
==================
*/

model_t *R_AllocModel()
{
    model_t     *mod;

    if(tr.numModels == MAX_MOD_KNOWN){
        return NULL;
    }

    mod = Hunk_Alloc(sizeof(*tr.models[tr.numModels]), h_low);
    mod->index = tr.numModels;
    tr.models[tr.numModels] = mod;
    tr.numModels++;

    return mod;
}
