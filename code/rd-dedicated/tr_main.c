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
// tr_main.c - Main control flow for each frame.

#include "tr_local.h"

trGlobals_t     tr;

// CVAR definitions.
cvar_t  *r_verbose;

// Local function definitions.
static void          R_Register                      ( void );

/*
==================
R_Register

Registers renderer CVARs.
==================
*/

static void R_Register()
{
    #ifdef _DEBUG
    r_verbose = Cvar_Get("r_verbose", "1", CVAR_CHEAT);
    #else
    r_verbose = Cvar_Get("r_verbose", "0", CVAR_CHEAT);
    #endif // _DEBUG
}

/*
==================
R_Init

Main routine to initialize renderer.
==================
*/

void R_Init()
{
    // Clear all our internal state.
    Com_Memset(&tr, 0, sizeof(tr));

    // Free all memory from the tagged zone
    // for the renderer, if any allocated.
    Z_FreeTags(TAG_RENDERER);

    // Register CVARs.
    R_Register();

    // Initialize main NULL model.
    R_ModelInit();

    // Initialize shaders.
    R_ShaderInit();
}
