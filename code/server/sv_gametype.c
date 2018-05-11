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
// sv_gametype.c - Interface to the gametype DLL.

#include "server.h"
#include "../gametype/gt_public.h"

// Local variable definitions.
static vm_t         *gtvm                   = NULL;   // Gametype virtual machine.

//==============================================

/*
====================
SV_GametypeSystemCalls

The module is making
a system call.
====================
*/

intptr_t SV_GametypeSystemCalls(intptr_t *args)
{
    switch(args[0]){
        case GT_PRINT:
            Com_Printf("%s", (const char*)VMA(1));
            return 0;
        case GT_ERROR:
            Com_Error(ERR_DROP, "%s", (const char*)VMA(1));
            return 0;
        case GT_MILLISECONDS:
            return Sys_Milliseconds();
        case GT_CVAR_REGISTER:
            Cvar_Register(VMA(1), VMA(2), VMA(3), args[4]);
            return 0;
        case GT_CVAR_UPDATE:
            Cvar_Update(VMA(1));
            return 0;
        case GT_CVAR_SET:
            Cvar_SetSafe((const char *)VMA(1), (const char *)VMA(2));
            return 0;
        case GT_CVAR_VARIABLE_INTEGER_VALUE:
            return Cvar_VariableIntegerValue((const char *)VMA(1));
        case GT_CVAR_VARIABLE_STRING_BUFFER:
            Cvar_VariableStringBuffer(VMA(1), VMA(2), args[3]);
            return 0;

        default:
            break;
    }

    // The game module handles the remaining gametype syscalls.
    return VM_Call(gvm, GAME_GAMETYPE_COMMAND, args[0], args[1], args[2], args[3], args[4], args[5]);
}

/*
===================
SV_RestartGametypeProgs

Called on a map_restart, but
not on a normal map change.
===================
*/

static void SV_RestartGametypeProgs(void)
{
    if(!gtvm){
        return;
    }

    // Do a restart instead of a free.
    gtvm = VM_Restart(gtvm, qtrue);
    if(!gtvm){
        Com_Error(ERR_FATAL, "VM_Restart on gametype failed");
    }
}

/*
===============
SV_InitGametypeProgs

Called on both a normal map change
and a map_restart.
===============
*/

static void SV_InitGametypeProgs(const char *gametype)
{
    // Load the specified gametype DLL.
    gtvm = VM_Create(va("gt_%s", gametype), SV_GametypeSystemCalls);
    if(!gtvm){
        Com_Error(ERR_FATAL, "VM_Create on gametype failed");
    }
}

/*
===============
SV_GT_Init

Load and initialize the requested
gametype module.
===============
*/

void SV_GT_Init(const char *gametype, qboolean restart)
{
    // Do a restart or a full initialization.
    if(restart){
        SV_RestartGametypeProgs();
    }else{
        SV_InitGametypeProgs(gametype);
    }

    // Execute the initialization routine
    // of the loaded gametype module.
    VM_Call(gtvm, GAMETYPE_INIT);
}

/*
===============
SV_GT_RunFrame

Advance non-player objects
in the gametype.
===============
*/

void SV_GT_RunFrame(int time)
{
    VM_Call(gtvm, GAMETYPE_RUN_FRAME, time);
}

/*
===============
SV_GT_Start

Start the gametype.
===============
*/

void SV_GT_Start(int time)
{
    VM_Call(gtvm, GAMETYPE_START, time);
}

/*
===============
SV_GT_SendEvent

Send an event to the
gametype module.
===============
*/

int SV_GT_SendEvent(int event, int time, int arg0, int arg1, int arg2, int arg3, int arg4)
{
    return VM_Call(gtvm, GAMETYPE_EVENT, event, time, arg0, arg1, arg2, arg3, arg4);
}

/*
===============
SV_GT_Shutdown

Called every time a map
changes.
===============
*/

void SV_GT_Shutdown(void)
{
    if(!gtvm){
        return;
    }

    VM_Call(gtvm, GAMETYPE_SHUTDOWN);
    VM_Free(gtvm);
    gtvm = NULL;
}
