/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors
Copyright (C) 2017, Ane-Jouke Schat

This file is part of the gp2-c source code.

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
// genericparser2.h

#ifndef __GP2_H
#define __GP2_H

#define     TOP_LEVEL_NAME                  "Top Level"
#define     TOPPOOL_SIZE                    10240
#define     MAX_TOKEN_SIZE                  1024

typedef     void    *TGenericParser2;
typedef     void    *TGPGroup;
typedef     void    *TGPValue;

typedef     struct  CGPObject               CGPObject;
typedef     struct  CGPGroup                CGPGroup;
typedef     struct  CGPValue                CGPValue;
typedef     struct  CTextPool               CTextPool;

//=============================================

struct CGPObject {
    const char      *mName;
    void            *mNext, *mInOrderNext, *mInOrderPrevious;
};

struct CGPGroup {
    CGPObject       mBase;
    CGPValue        *mPairs, *mInOrderPairs;
    CGPValue        *mCurrentPair;
    CGPGroup        *mSubGroups, *mInOrderSubGroups;
    CGPGroup        *mCurrentSubGroup;
    CGPGroup        *mParent;
};

struct CGPValue {
    CGPObject       mBase;
    CGPValue        *mList;
};

struct CTextPool {
    char            *mPool;
    CTextPool       *mNext;
    int             mSize, mUsed;
};

typedef struct {
    CGPGroup        mTopLevel;
    CTextPool       *mTextPool;
} CGenericParser2;

//=============================================

// CGenericParser2 (void *) routines.
//=============================================
TGenericParser2     GP_Parse                ( char **dataPtr );
void                GP_Clean                ( TGenericParser2 GP2 );
void                GP_Delete               ( TGenericParser2 *GP2 );
TGPGroup            GP_GetBaseParseGroup    ( TGenericParser2 GP2 );

// CGPGroup (void *) routines.
//=============================================
qboolean            GPG_GetName             ( TGPGroup GPG, char *dest, int destSize );
TGPGroup            GPG_GetNext             ( TGPGroup GPG );
TGPGroup            GPG_GetInOrderNext      ( TGPGroup GPG );
TGPGroup            GPG_GetInOrderPrevious  ( TGPGroup GPG );
TGPGroup            GPG_GetPairs            ( TGPGroup GPG );
TGPGroup            GPG_GetInOrderPairs     ( TGPGroup GPG );
TGPGroup            GPG_GetSubGroups        ( TGPGroup GPG );
TGPGroup            GPG_GetInOrderSubGroups ( TGPGroup GPG );
TGPGroup            GPG_FindSubGroup        ( TGPGroup GPG, const char *name );
TGPValue            GPG_FindPair            ( TGPGroup GPG, const char *key );
void                GPG_FindPairValue       ( TGPGroup GPG, const char *key, const char *defaultVal, char *dest, int destSize );

// CGPValue (void *) routines.
//=============================================
qboolean            GPV_GetName             ( TGPValue GPV, char *dest, int destSize );
TGPValue            GPV_GetNext             ( TGPValue GPV );
TGPValue            GPV_GetInOrderNext      ( TGPValue GPV );
TGPValue            GPV_GetInOrderPrevious  ( TGPValue GPV );
qboolean            GPV_IsList              ( TGPValue GPV );
qboolean            GPV_GetTopValue         ( TGPValue GPV, char *dest, int destSize );
TGPValue            GPV_GetList             ( TGPValue GPV );

#endif // __GP2_H
