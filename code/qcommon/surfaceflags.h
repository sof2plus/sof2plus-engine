/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

#define CONTENTS_SOLID          (0x00000001u)   // Default setting. An eye is never valid in a solid
#define CONTENTS_LAVA           (0x00000002u)
#define CONTENTS_WATER          (0x00000004u)
#define CONTENTS_FOG            (0x00000008u)
#define CONTENTS_PLAYERCLIP     (0x00000010u)
#define CONTENTS_MONSTERCLIP    (0x00000020u)
#define CONTENTS_BOTCLIP        (0x00000040u)
#define CONTENTS_SHOTCLIP       (0x00000080u)
#define CONTENTS_BODY           (0x00000100u)   // should never be on a brush, only in game
#define CONTENTS_CORPSE         (0x00000200u)   // should never be on a brush, only in game
#define CONTENTS_TRIGGER        (0x00000400u)
#define CONTENTS_NODROP         (0x00000800u)   // don't leave bodies or items (death fog, lava)
#define CONTENTS_TERRAIN        (0x00001000u)   // volume contains terrain data
#define CONTENTS_LADDER         (0x00002000u)
#define CONTENTS_ABSEIL         (0x00004000u)   // (SOF2) used like ladder to define where an NPC can abseil
#define CONTENTS_OPAQUE         (0x00008000u)   // defaults to on, when off, solid can be seen through
#define CONTENTS_OUTSIDE        (0x00010000u)   // volume is considered to be in the outside (i.e. not indoors)
#define CONTENTS_MISSILECLIP    (0x00020000u)   // missile clip
#define CONTENTS_TELEPORTER     (0x00080000u)   // ""
#define CONTENTS_ITEM           (0x00100000u)   // ""
#define CONTENTS_DETAIL         (0x08000000u)   // brushes not used for the bsp
#define CONTENTS_TRANSLUCENT    (0x80000000u)   // don't consume surface fragments inside

#define SURF_HINT               (0x00000100u)   // make a primary bsp splitter
#define SURF_SKIP               (0x00000200u)   // completely ignore, allowing non-clused brushes
#define SURF_SKY                (0x00002000u)   // lighting from environment map
#define SURF_SLICK              (0x00004000u)   // affects game physics
#define SURF_METALSTEPS         (0x00008000u)   // CHC needs this since we use same tools (though this flag is temp?)
#define SURF_FORCEFIELD         (0x00010000u)   // CHC ""           (but not temp)
#define SURF_NODAMAGE           (0x00040000u)   // never give falling damage
#define SURF_NOIMPACT           (0x00080000u)   // don't make missile explosions
#define SURF_NOMARKS            (0x00100000u)   // don't leave missile marks
#define SURF_NODRAW             (0x00200000u)   // don't generate a drawsurface at all
#define SURF_NOSTEPS            (0x00400000u)   // no footstep sounds
#define SURF_NODLIGHT           (0x00800000u)   // don't dlight even if solid (solid lava, skies)
#define SURF_NOMISCENTS         (0x01000000u)   // no client models allowed on this surface

#define MATERIAL_BITS           5
#define MATERIAL_NONE           0               // we only use the no material mask server-side
#define MATERIAL_MASK           0x1f            // mask to get the material type
