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

typedef     struct      CGhoul2Info_s       CGhoul2Info_t;
typedef     struct      CGhoul2InfoArray_s  CGhoul2InfoArray_t;

//=============================================
//
// Main Ghoul II structures
//

struct CGhoul2Info_s {
    int                 mModelIndex;
    qhandle_t           mModel;
    char                mFileName[MAX_QPATH];

    qboolean            mValid;
    //const mdxaHeader_t  *aHeader;
};

struct CGhoul2InfoArray_s {

};

typedef struct CGhoul2Model_s {

} CGhoul2Model_t;

typedef struct CGhoul2ModelHash_s {
    char        name[MAX_QPATH];
    qhandle_t   handle;
    struct      CGhoul2ModelHash_s  *next;

} CGhoul2ModelHash_t;

//=============================================

//
// g2_api.c
//

void                G2API_ListBones         ( CGhoul2Info_t *ghlInfo, int frame );

//
// g2_misc.c
//

qboolean            G2_SetupModelPointers   ( CGhoul2Info_t *ghlInfo );

#endif // __G2_LOCAL_H
