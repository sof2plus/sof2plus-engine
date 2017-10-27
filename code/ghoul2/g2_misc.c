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
// g2_misc.c - Miscellaneous Ghoul II routines.

#include "g2_local.h"

/*
==================
G2_SetupModelPointers

Setup the model if this hasn't been done already.
==================
*/

qboolean G2_SetupModelPointers(CGhoul2Info_t *ghlInfo)
{
    if(!ghlInfo){
        return qfalse;
    }

    ghlInfo->mValid = qfalse;

    if(ghlInfo->mModelIndex != -1){
        // Check whether the model is valid, and if not, re-register it.
        if(!ghlInfo->mModel){
            ghlInfo->mModel = RE_RegisterServerModel(ghlInfo->mFileName);
        }
    }
    // BOE FIXME
}
