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
// g2_gore_shared.h

#ifndef __G2_GORE_SHARED
#define __G2_GORE_SHARED

// these are the actual shaders
typedef enum {
    PGORE_NONE,
    PGORE_ARMOR,
    PGORE_BULLETBIG,
    PGORE_KNIFESLASH,
    PGORE_PUNCTURE,
    PGORE_SHOTGUN,
    PGORE_SHOTGUNBIG,
    PGORE_IMMOLATE,
    PGORE_BURN,
    PGORE_SPURT,
    PGORE_SPLATTER,
    PGORE_BLOODY_GLASS,
    PGORE_BLOODY_GLASS_B,
    PGORE_BLOODY_ICK,
    PGORE_BLOODY_DROOP,
    PGORE_BLOODY_MAUL,
    PGORE_BLOODY_DROPS,
    PGORE_BULLET_E,
    PGORE_BULLET_F,
    PGORE_BULLET_G,
    PGORE_BULLET_H,
    PGORE_BULLET_I,
    PGORE_BULLET_J,
    PGORE_BULLET_K,
    PGORE_BLOODY_HAND,
    PGORE_POWDER_BURN_DENSE,
    PGORE_POWDER_BURN_CHUNKY,
    PGORE_KNIFESLASH2,
    PGORE_KNIFESLASH3,
    PGORE_CHUNKY_SPLAT,
    PGORE_BIG_SPLATTER,
    PGORE_BLOODY_SPLOTCH,
    PGORE_BLEEDER,
    PGORE_PELLETS,
    PGORE_KNIFE_SOAK,
    PGORE_BLEEDER_DENSE,
    PGORE_BLOODY_SPLOTCH2,
    PGORE_BLOODY_DRIPS,
    PGORE_DRIPPING_DOWN,
    PGORE_GUTSHOT,
    PGORE_SHRAPNEL,
    PGORE_COUNT
} goreEnum_t;

struct goreEnumShader_t
{
    int             maxLODBias;   //if r_lodBias (and the other 3 convars) =x then shaders with this larger than x will not be used
    goreEnum_t      shaderEnum;  // why is this even in here?
    char            shaderName[MAX_QPATH];
};

typedef struct SSkinGoreData_s
{
    vec3_t          angles;
    vec3_t          position;
    int             currentTime;
    int             entNum;
    vec3_t          rayDirection;   // in world space
    vec3_t          hitLocation;    // in world space
    vec3_t          scale;
    float           SSize;          // size of splotch in the S texture direction in world units
    float           TSize;          // size of splotch in the T texture direction in world units
    float           theta;          // angle to rotate the splotch

    // growing stuff
    int             growDuration;           // time over which we want this to scale up, set to -1 for no scaling
    float           goreScaleStartFraction; // fraction of the final size at which we want the gore to initially appear

    qboolean        frontFaces;
    qboolean        backFaces;
    qboolean        baseModelOnly;
    int             lifeTime;               // effect expires after this amount of time
    int             fadeOutTime;            // unimplemented
    int             shrinkOutTime;          // unimplemented
    float           alphaModulate;          // unimplemented
    vec3_t          tint;                   // unimplemented
    float           impactStrength;         // unimplemented

    goreEnum_t      shaderEnum; // enum that'll get switched over to the shader's actual handle 

    int             myIndex; // used internally

    /*
    // Constructor for the CPP version of this struct in SP.
    SSkinGoreData()
    {
        growDuration=-1;
        goreScaleStartFraction=1.0f;
        frontFaces=true;
        backFaces=true;
        theta=0.0f;
        SSize=0.0f;
        TSize=0.0f;
        lifeTime=0;
        baseModelOnly=false;
    }
    */
} SSkinGoreData;

#endif // __G2_GORE_SHARED
