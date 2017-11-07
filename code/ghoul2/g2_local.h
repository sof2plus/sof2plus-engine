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
// g2_local.h

#ifndef __G2_LOCAL_H
#define __G2_LOCAL_H

#include "../rd-dedicated/tr_local.h"

#define     G2_VERT_SPACE_SIZE              256

typedef     struct      CGhoul2Model_s      CGhoul2Model_t;
typedef     struct      CGhoul2Array_s      CGhoul2Array_t;

//=============================================
//
// Main Ghoul II structures
//

struct CGhoul2Model_s {
    int                 mModelIndex;
    qhandle_t           mModel;
    char                mFileName[MAX_QPATH];

    qboolean            mValid;
    const model_t       *currentModel;
    int                 currentModelSize;
    const model_t       *animModel;
    int                 currentAnimModelSize;
    const mdxaHeader_t  *aHeader;
};

struct CGhoul2Array_s {
    CGhoul2Model_t      *models[MAX_MOD_KNOWN];
    int                 numModels;
};

//=============================================

//
// g2_api.c
//

void                G2API_ListBones         ( CGhoul2Model_t *ghlInfo, int frame );

//
// g2_misc.c
//

qboolean            G2_SetupModelPointers   ( CGhoul2Model_t *ghlInfo );

#endif // __G2_LOCAL_H
