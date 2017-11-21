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
// g2_surfaces.c - Surface list handling Ghoul II routines.

#include "g2_local.h"

/*
==================
G2_IsSurfaceLegal

Checks if the given surface name is legal
in the supplemented model.

Returns the surface index if valid,
-1 if the surface wasn't found.
==================
*/

int G2_IsSurfaceLegal(model_t *mod, const char *surfaceName, int *flags)
{
    mdxmSurfHierarchy_t *surf;
    mdxmHeader_t        *mdxmHeader;
    int                 i;

    mdxmHeader = mod->modelData;
    surf = (mdxmSurfHierarchy_t *) ( (byte *)mdxmHeader + mdxmHeader->ofsSurfHierarchy );

    // Iterate through the surfaces of this model.
    for (i = 0; i < mdxmHeader->numSurfaces; i++){
        if(Q_stricmp(surfaceName, surf->name) == 0){
            // Found it! Set the flags and return the index.
            *flags = surf->flags;
            return i;
        }

        // Determine the next surface.
        surf = (mdxmSurfHierarchy_t *)( (byte *)surf + (size_t)(&((mdxmSurfHierarchy_t *)0)->childIndexes[surf->numChildren]));
    }

    // The surface wasn't found.
    return -1;
}
