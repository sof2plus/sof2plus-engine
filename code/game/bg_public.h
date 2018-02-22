// Copyright (C) 2001-2002 Raven Software.
//
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#include "bg_weapons.h"
#include "inv.h"

#define GAME_VERSION        "sof2mp-1.03"

#define DEFAULT_GRAVITY     800
#define ARMOR_PROTECTION    0.55

#define MAX_ITEMS           256
#define MAX_HEALTH          100
#define MAX_ARMOR           100

#define BULLET_SPACING      95

#define DISMEMBER_HEALTH    -20

#define RANK_TIED_FLAG      0x4000

#define ITEM_RADIUS         15      // item sizes are needed for client side pickup detection

#define SCORE_NOT_PRESENT   -9999   // for the CS_SCORES[12] when only one player is present

#define DEFAULT_PLAYER_Z_MAX    43
#define CROUCH_PLAYER_Z_MAX     18
#define PRONE_PLAYER_Z_MAX      -12
#define DEAD_PLAYER_Z_MAX       -30

#define DUCK_ACCURACY_MODIFIER  0.75f
#define JUMP_ACCURACY_MODIFIER  2.0f

#define MINS_Z              -46

#define DEFAULT_VIEWHEIGHT  37
#define CROUCH_VIEWHEIGHT   8
#define PRONE_VIEWHEIGHT    -22
#define DEAD_VIEWHEIGHT     -38

#define BODY_SINK_DELAY     10000
#define BODY_SINK_TIME      1500

#define LEAN_TIME           250
#define LEAN_OFFSET         30

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO, CS_SYSTEMINFO and CS_PLAYERS are defined in q_shared.h

enum
{
    CS_MUSIC = CS_CUSTOM,

    CS_MESSAGE,
    CS_MOTD,
    CS_WARMUP,

    CS_VOTE_TIME,
    CS_VOTE_STRING,
    CS_VOTE_YES,
    CS_VOTE_NO,
    CS_VOTE_NEEDED,

    CS_GAME_VERSION,
    CS_GAME_ID,
    CS_LEVEL_START_TIME,
    CS_INTERMISSION,
    CS_SHADERSTATE,
    CS_BOTINFO,

    CS_GAMETYPE_TIMER,
    CS_GAMETYPE_MESSAGE,
    CS_GAMETYPE_REDTEAM,
    CS_GAMETYPE_BLUETEAM,

    CS_ITEMS,

    CS_PICKUPSDISABLED,

    // Config string ranges
    CS_MODELS,
    CS_SOUNDS               = CS_MODELS + MAX_MODELS,
    CS_LOCATIONS            = CS_SOUNDS + MAX_SOUNDS,
    CS_LADDERS              = CS_LOCATIONS + MAX_LOCATIONS,
    CS_BSP_MODELS           = CS_LADDERS + MAX_LADDERS,
    CS_TERRAINS             = CS_BSP_MODELS + MAX_SUB_BSP,
    CS_EFFECTS              = CS_TERRAINS + MAX_TERRAINS,
    CS_LIGHT_STYLES         = CS_EFFECTS + MAX_FX,
    CS_ICONS                = CS_LIGHT_STYLES + (MAX_LIGHT_STYLES*3),
    CS_TEAM_INFO            = CS_ICONS + MAX_ICONS,
    CS_AMBIENT_SOUNDSETS    = CS_TEAM_INFO + TEAM_NUM_TEAMS,

    CS_HUDICONS             = CS_AMBIENT_SOUNDSETS + MAX_AMBIENT_SOUNDSETS,

    CS_MAX                  = CS_HUDICONS + MAX_HUDICONS,
};

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

// animations
typedef enum
{
    BOTH_DEATH_NORMAL,

    ANIM_START_DEATHS,

    BOTH_DEATH_NECK,
    BOTH_DEATH_CHEST_1,
    BOTH_DEATH_CHEST_2,
    BOTH_DEATH_GROIN_1,
    BOTH_DEATH_GROIN_2,
    BOTH_DEATH_GUT_1,
    BOTH_DEATH_GUT_2,
    BOTH_DEATH_HEAD_1,
    BOTH_DEATH_HEAD_2,
    BOTH_DEATH_SHOULDER_LEFT_1,
    BOTH_DEATH_SHOULDER_LEFT_2,
    BOTH_DEATH_ARMS_LEFT_1,
    BOTH_DEATH_ARMS_LEFT_2,
    BOTH_DEATH_LEGS_LEFT_1,
    BOTH_DEATH_LEGS_LEFT_2,
    BOTH_DEATH_LEGS_LEFT_3,
    BOTH_DEATH_THIGH_LEFT_1,
    BOTH_DEATH_THIGH_LEFT_2,
    BOTH_DEATH_ARMS_RIGHT_1,
    BOTH_DEATH_ARMS_RIGHT_2,
    BOTH_DEATH_LEGS_RIGHT_1,
    BOTH_DEATH_LEGS_RIGHT_2,
    BOTH_DEATH_LEGS_RIGHT_3,
    BOTH_DEATH_SHOULDER_RIGHT_1,
    BOTH_DEATH_SHOULDER_RIGHT_2,
    BOTH_DEATH_THIGH_RIGHT_1,
    BOTH_DEATH_THIGH_RIGHT_2,

    ANIM_END_DEATHS,

    TORSO_DROP,
    TORSO_DROP_ONEHANDED,
    TORSO_DROP_KNIFE,
    TORSO_RAISE,
    TORSO_RAISE_ONEHANDED,
    TORSO_RAISE_KNIFE,

    LEGS_IDLE,
    LEGS_IDLE_CROUCH,
    LEGS_WALK,
    LEGS_WALK_BACK,
    LEGS_WALK_CROUCH,
    LEGS_WALK_CROUCH_BACK,

    LEGS_RUN,
    LEGS_RUN_BACK,

    LEGS_SWIM,

    LEGS_JUMP,
    LEGS_JUMP_BACK,

    LEGS_TURN,

    LEGS_LEAN_LEFT,
    LEGS_LEAN_RIGHT,
    LEGS_LEAN_CROUCH_LEFT,
    LEGS_LEAN_CROUCH_RIGHT,

    LEGS_LEANLEFT_WALKLEFT,
    LEGS_LEANLEFT_WALKRIGHT,
    LEGS_LEANRIGHT_WALKLEFT,
    LEGS_LEANRIGHT_WALKRIGHT,

    LEGS_LEANLEFT_CROUCH_WALKLEFT,
    LEGS_LEANLEFT_CROUCH_WALKRIGHT,
    LEGS_LEANRIGHT_CROUCH_WALKLEFT,
    LEGS_LEANRIGHT_CROUCH_WALKRIGHT,

    TORSO_IDLE_KNIFE,
    TORSO_IDLE_PISTOL,
    TORSO_IDLE_RIFLE,
    TORSO_IDLE_MSG90A1,
    TORSO_IDLE_MSG90A1_ZOOMED,
    TORSO_IDLE_M4,
    TORSO_IDLE_M590,
    TORSO_IDLE_USAS12,
    TORSO_IDLE_RPG,
    TORSO_IDLE_M60,
    TORSO_IDLE_MM1,
    TORSO_IDLE_GRENADE,

    TORSO_ATTACK_KNIFE,
    TORSO_ATTACK_KNIFE_THROW,
    TORSO_ATTACK_PISTOL,
    TORSO_ATTACK_RIFLE,
    TORSO_ATTACK_MSG90A1_ZOOMED,
    TORSO_ATTACK_M4,
    TORSO_ATTACK_M590,
    TORSO_ATTACK_USAS12,
    TORSO_ATTACK_RIFLEBUTT,
    TORSO_ATTACK_RPG,
    TORSO_ATTACK_M60,
    TORSO_ATTACK_MM1,
    TORSO_ATTACK_GRENADE_START,
    TORSO_ATTACK_GRENADE_END,
    TORSO_ATTACK_BAYONET,
    TORSO_ATTACK_PISTOLWHIP,

    TORSO_RELOAD_M60,
    TORSO_RELOAD_PISTOL,
    TORSO_RELOAD_RIFLE,
    TORSO_RELOAD_MSG90A1,
    TORSO_RELOAD_RPG,
    TORSO_RELOAD_USAS12,

    TORSO_RELOAD_M590_START,
    TORSO_RELOAD_M590_SHELL,
    TORSO_RELOAD_M590_END,

    TORSO_RELOAD_MM1_START,
    TORSO_RELOAD_MM1_SHELL,
    TORSO_RELOAD_MM1_END,

    TORSO_USE,

    MAX_ANIMATIONS

} animNumber_t;

typedef struct animation_s {
    int     firstFrame;
    int     numFrames;
    int     loopFrames;         // 0 to numFrames
    int     frameLerp;          // msec between frames
    int     initialLerp;        // msec to get to first frame
    int     reversed;           // true if animation is reversed
    int     flipflop;           // true if animation should flipflop back to base
} animation_t;

typedef struct ladder_s
{
    vec3_t  origin;
    vec3_t  fwd;

} ladder_t;

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define ANIM_TOGGLEBIT              2048        // Note that there are 12 bits (max 4095) for animations.
#define ITEM_AUTOSWITCHBIT          (1<<31)
#define ITEM_QUIETPICKUP            (1<<30)

typedef enum
{
    PM_NORMAL,          // can accelerate and turn
    PM_NOCLIP,          // noclip movement
    PM_SPECTATOR,       // still run into walls
    PM_DEAD,            // no acceleration or turning, but free falling
    PM_FREEZE,          // stuck in place with no control
    PM_INTERMISSION,    // no movement or status bar

} pmtype_t;

typedef enum {
    WEAPON_READY,
    WEAPON_SPAWNING,
    WEAPON_RAISING,
    WEAPON_DROPPING,
    WEAPON_RELOADING,
    WEAPON_RELOADING_ALT,
    WEAPON_FIRING,
    WEAPON_FIRING_ALT,
    WEAPON_CHARGING,
    WEAPON_CHARGING_ALT,
    WEAPON_ZOOMIN,
    WEAPON_ZOOMOUT,
} weaponstate_t;

// pmove->pm_flags
#define PMF_DUCKED              0x00000001
#define PMF_BACKWARDS_JUMP      0x00000002      // go into backwards land
#define PMF_JUMPING             0x00000004      // executing a jump
#define PMF_BACKWARDS_RUN       0x00000008      // coast down to backwards run
#define PMF_TIME_LAND           0x00000010      // pm_time is time before rejump
#define PMF_TIME_KNOCKBACK      0x00000020      // pm_time is an air-accelerate only time
#define PMF_TIME_WATERJUMP      0x00000040      // pm_time is waterjump
#define PMF_RESPAWNED           0x00000080      // clear after attack and jump buttons come up
#define PMF_CAN_USE             0x00000100      // The server updated the animation, the pmove should set the ghoul2 anim to match.
#define PMF_FOLLOW              0x00000200      // spectate following another player
#define PMF_SCOREBOARD          0x00000400      // spectate as a scoreboard
#define PMF_GHOST               0x00000800      // Your a ghost. scarry!!
#define PMF_LADDER              0x00001000      // On a ladder
#define PMF_LADDER_JUMP         0x00002000      // Jumped off a ladder

#define PMF_ZOOMED              0x00004000
#define PMF_ZOOM_LOCKED         0x00008000      // Zoom mode cant be changed
#define PMF_ZOOM_REZOOM         0x00010000      // Rezoom after reload done
#define PMF_ZOOM_DEFER_RELOAD   0x00020000      // Reload after zoomout

#define PMF_LIMITED_INVENTORY   0x00040000      // inventory is limited for this player

#define PMF_CROUCH_JUMP         0x00080000      // crouch jumping
#define PMF_GOGGLES_ON          0x00100000      // goggles are on
#define PMF_LEANING             0x00200000      // currently leaning

#define PMF_AUTORELOAD          0x00400000      // autoreloading enabled

#define PMF_SIAMESETWINS        0x00800000
#define PMF_FOLLOWFIRST         0x01000000      // First person following

#define PMF_ALL_TIMES   (PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)
#define PMF_ZOOM_FLAGS  (PMF_ZOOMED|PMF_ZOOM_LOCKED|PMF_ZOOM_REZOOM|PMF_ZOOM_DEFER_RELOAD)

// pmove->pm_debounce

#define PMD_JUMP                0x0001
#define PMD_ATTACK              0x0002
#define PMD_FIREMODE            0x0004
#define PMD_USE                 0x0008
#define PMD_ALTATTACK           0x0010
#define PMD_GOGGLES             0x0020

#define MAXTOUCH    32

typedef struct {
    // state (in / out)
    playerState_t   *ps;

    // command (in)
    usercmd_t   cmd;
    int         tracemask;          // collide against these types of surfaces
    int         debugLevel;         // if set, diagnostic output will be printed
    qboolean    noFootsteps;        // if the game is setup for no footsteps by the server
    qboolean    gauntletHit;        // true if a gauntlet attack would actually hit something

    int         framecount;

    // results (out)
    int         numtouch;
    int         touchents[MAXTOUCH];

    int         useEvent;

    vec3_t      mins, maxs;         // bounding box size

    int         watertype;
    int         waterlevel;

    animation_t *animations;

    float       xyspeed;

    // for fixed msec Pmove
    int         pmove_fixed;
    int         pmove_msec;

    // callbacks to test the world
    // these will be different functions during game and cgame
    void        (*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
    int         (*pointcontents)( const vec3_t point, int passEntityNum );

    int         weaponAnimIdx;
    char        weaponAnim[MAX_QPATH];
    char        weaponEndAnim[MAX_QPATH];
} pmove_t;

extern  pmove_t     *pm;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum
{
    STAT_HEALTH,
    STAT_WEAPONS,                   // 16 bit fields
    STAT_ARMOR,
    STAT_DEAD_YAW,                  // look this direction when dead (FIXME: get rid of?)
    STAT_CLIENTS_READY,             // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
    STAT_FROZEN,
    STAT_GOGGLES,                   // Which visual enhancing device they have
    STAT_GAMETYPE_ITEMS,            // Which gametype items they have
    STAT_SEED,                      // seed used to keep weapon firing in sync
    STAT_OUTFIT_GRENADE,            // indicates which greande is chosen in the outfitting
    STAT_USEICON,                   // icon to display when able to use a trigger or item
    STAT_USETIME,                   // elased time for using
    STAT_USETIME_MAX,               // total time required to use
    STAT_USEWEAPONDROP,             // value to drop weapon out of view when using

} statIndex_t;


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum
{
    PERS_SCORE,                     // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
    PERS_RANK,                      // player rank or team rank
    PERS_TEAM,                      // player team
    PERS_SPAWN_COUNT,               // incremented every respawn
    PERS_PLAYEREVENTS,              // 16 bits that can be flipped for events
    PERS_ATTACKER,                  // clientnum of last damage inflicter

    PERS_RED_SCORE,                 // Blue team score
    PERS_BLUE_SCORE,                // red team score

    PERS_RED_ALIVE_COUNT,           // number of alive people on the red team
    PERS_BLUE_ALIVE_COUNT,          // number of alive people on the blue team

} persEnum_t;


typedef enum
{
    GOGGLES_NONE,
    GOGGLES_NIGHTVISION,
    GOGGLES_INFRARED,
    GOGGLES_MAX

} goggleType_t;

// entityState_t->eFlags
#define EF_DEAD                 0x00000001      // don't draw a foe marker over players with EF_DEAD
#define EF_EXPLODE              0x00000002      // ready to explode
#define EF_TELEPORT_BIT         0x00000004      // toggled every time the origin abruptly changes

#define EF_PLAYER_EVENT         0x00000008
#define EF_BOUNCE               0x00000008      // for missiles

#define EF_BOUNCE_HALF          0x00000010      // bounce and retain half velocity each time
#define EF_BOUNCE_SCALE         0x00000020      // bounces using the bounce scale
#define EF_NODRAW               0x00000040      // may have an event, but no model (unspawned items)
#define EF_FIRING               0x00000080      // for lightning gun
#define EF_ALT_FIRING           0x00000100      // for alt-fires, mostly for lightning guns though
#define EF_MOVER_STOP           0x00000200      // will push otherwise
#define EF_TALK                 0x00000400      // draw a talk balloon
#define EF_CONNECTION           0x00000800      // draw a connection trouble sprite
#define EF_VOTED                0x00001000      // already cast a vote
#define EF_ANGLE_OVERRIDE       0x00002000      // angle coming from the server is absolute
#define EF_PERMANENT            0x00004000      // this entity is permanent and is never updated (sent only in the game state)
                                                // this should only be used in RMG!
#define EF_NOPICKUP             0x00008000      // entity cannot be picked up
#define EF_NOSHADOW             0x00008000      // used for bodies when they are sinking

#define EF_REDTEAM              0x00010000      // Red team only
#define EF_BLUETEAM             0x00020000      // Blue team only
#define EF_INSKY                0x00040000      // In a sky brush

#define EF_GOGGLES              0x00080000      // goggles on or not

#define EF_DUCKED               0x00100000      // ducked?
#define EF_INVULNERABLE         0x00200000      // cant be shot

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define EV_EVENT_BIT1       0x00000100
#define EV_EVENT_BIT2       0x00000200
#define EV_EVENT_BITS       (EV_EVENT_BIT1|EV_EVENT_BIT2)

#define EVENT_VALID_MSEC    300

typedef enum
{
    EV_NONE,

    EV_FOOTSTEP,
    EV_FOOTWADE,
    EV_SWIM,

    EV_STEP_4,
    EV_STEP_8,
    EV_STEP_12,
    EV_STEP_16,

    EV_FALL_SHORT,
    EV_FALL_MEDIUM,
    EV_FALL_FAR,

    EV_JUMP,
    EV_WATER_FOOTSTEP,
    EV_WATER_TOUCH, // foot touches
    EV_WATER_LAND,  // landed in water
    EV_WATER_CLEAR,

    EV_ITEM_PICKUP,         // normal item pickups are predictable
    EV_ITEM_PICKUP_QUIET,   // quiet pickup

    EV_NOAMMO,
    EV_CHANGE_WEAPON,
    EV_CHANGE_WEAPON_CANCELLED,
    EV_READY_WEAPON,
    EV_FIRE_WEAPON,
    EV_ALT_FIRE,

    EV_USE,         // +Use key

    EV_ITEM_RESPAWN,
    EV_ITEM_POP,
    EV_PLAYER_TELEPORT_IN,
    EV_PLAYER_TELEPORT_OUT,

    EV_GRENADE_BOUNCE,      // eventParm will be the soundindex

    EV_PLAY_EFFECT,

    EV_GENERAL_SOUND,
    EV_GLOBAL_SOUND,        // no attenuation
    EV_ENTITY_SOUND,

    EV_GLASS_SHATTER,

    EV_MISSILE_HIT,
    EV_MISSILE_MISS,

    EV_BULLET_HIT_WALL,
    EV_BULLET_HIT_FLESH,
    EV_BULLET,              // otherEntity is the shooter

    EV_EXPLOSION_HIT_FLESH,

    EV_PAIN,
    EV_PAIN_WATER,
    EV_OBITUARY,

    EV_DESTROY_GHOUL2_INSTANCE,

    EV_WEAPON_CHARGE,
    EV_WEAPON_CHARGE_ALT,

    EV_DEBUG_LINE,
    EV_TESTLINE,
    EV_STOPLOOPINGSOUND,

    EV_BODY_QUEUE_COPY,
    EV_BOTWAYPOINT,

    // Procedural gore event.
    EV_PROC_GORE,

    EV_GAMETYPE_RESTART,            // gametype restarting
    EV_GAME_OVER,                   // game is over

    EV_GOGGLES,                     // goggles turning on/off

    EV_WEAPON_CALLBACK,

} entity_event_t;           // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)

typedef enum
{
    VEV_TALKSTART,
    VEV_TALKSTOP,

} voice_event_t;

typedef enum
{
    GAME_OVER_TIMELIMIT,
    GAME_OVER_SCORELIMIT,

} game_over_t;

typedef enum {
    GTS_RED_CAPTURE,
    GTS_BLUE_CAPTURE,
    GTS_RED_RETURN,
    GTS_BLUE_RETURN,
    GTS_RED_TAKEN,
    GTS_BLUE_TAKEN,
    GTS_REDTEAM_SCORED,
    GTS_BLUETEAM_SCORED,
    GTS_REDTEAM_TOOK_LEAD,
    GTS_BLUETEAM_TOOK_LEAD,
    GTS_TEAMS_ARE_TIED
} global_team_sound_t;


// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME       1000

// How many players on the overlay
#define TEAM_MAXOVERLAY     32

//---------------------------------------------------------

typedef enum
{
    IT_BAD,
    IT_WEAPON,      // Weapon item
    IT_AMMO,        // Ammo item
    IT_ARMOR,       // Armor item
    IT_HEALTH,      // Healh item
    IT_GAMETYPE,    // Custom gametype related item
    IT_BACKPACK,    // replenish backpack item
    IT_PASSIVE,     // Passive items

} itemType_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s
{
    char        *classname;                     // spawning name
    char        *pickup_sound;
    char        *world_model[MAX_ITEM_MODELS];

    char        *icon;
    char        *render;
    char        *pickup_prefix;                 // an, some, a, the, etc..
    char        *pickup_name;                   // for printing on pickup

    int         quantity;                       // for ammo how much
    itemType_t  giType;                         // IT_* flags

    int         giTag;

    char        *precaches;                     // string of all models and images this item will use
    char        *sounds;                        // string of all sounds this item will use

    int         outfittingGroup;

} gitem_t;

// included in both the game dll and the client
extern  gitem_t bg_itemlist[];
extern  int     bg_numItems;

gitem_t*    BG_FindItem             ( const char *pickupName );
gitem_t*    BG_FindClassnameItem    ( const char *classname );
gitem_t*    BG_FindWeaponItem       ( weapon_t weapon );
gitem_t*    BG_FindGametypeItem     ( int index );
gitem_t*    BG_FindGametypeItemByID ( int itemid );


#define ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean    BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );

// g_dmflags->integer flags
#define DF_NO_FALLING           8
#define DF_FIXED_FOV            16
#define DF_NO_FOOTSTEPS         32

// content masks
#define MASK_ALL                (-1)
#define MASK_SOLID              (CONTENTS_SOLID|CONTENTS_TERRAIN)
#define MASK_PLAYERSOLID        (CONTENTS_SOLID|CONTENTS_TERRAIN|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_SHOTCLIP)
#define MASK_DEADSOLID          (CONTENTS_SOLID|CONTENTS_TERRAIN|CONTENTS_PLAYERCLIP)
#define MASK_WATER              (CONTENTS_WATER)
#define MASK_OPAQUE             (CONTENTS_SOLID|CONTENTS_TERRAIN|CONTENTS_SLIME|CONTENTS_LAVA)
#define MASK_SHOT               (CONTENTS_SOLID|CONTENTS_TERRAIN|CONTENTS_BODY|CONTENTS_SHOTCLIP)

//
// entityState_t->eType
//
typedef enum
{
    ET_GENERAL,
    ET_PLAYER,
    ET_ITEM,
    ET_MISSILE,
    ET_MOVER,
    ET_BEAM,
    ET_PORTAL,
    ET_SPEAKER,
    ET_PUSH_TRIGGER,
    ET_TELEPORT_TRIGGER,
    ET_INVISIBLE,
    ET_GRAPPLE,             // grapple hooked on wall
    ET_BODY,
    ET_DAMAGEAREA,
    ET_TERRAIN,

    ET_DEBUG_CYLINDER,

    ET_GAMETYPE_TRIGGER,

    ET_WALL,

    ET_EVENTS               // any of the EV_* events can be added freestanding
                            // by setting eType to ET_EVENTS + eventNum
                            // this avoids having to set eFlags and eventNum
} entityType_t;


void        BG_AddLadder                ( vec3_t absmin, vec3_t absmax, vec3_t fwd );
int         BG_FindLadder               ( vec3_t pos );

void        BG_EvaluateTrajectory       ( const trajectory_t *tr, int atTime, vec3_t result );
void        BG_EvaluateTrajectoryDelta  ( const trajectory_t *tr, int atTime, vec3_t result );
void        BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void        BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void        BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );
void        BG_PlayerAngles( vec3_t startAngles, vec3_t legs[3],
                         vec3_t legsAngles, vec3_t lowerTorsoAngles, vec3_t upperTorsoAngles, vec3_t headAngles,
                         int leanOffset, int painTime, int painDirection, int currentTime,
                         animInfo_t* torsoInfo, animInfo_t* legsInfo,
                         int frameTime, vec3_t velocity, qboolean dead, float movementDir, void *ghoul2 );
qboolean    BG_ParseAnimationFile ( const char *filename, animation_t* animations );

float       BG_CalculateLeanOffset ( int leanTime );

qboolean    BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );

TAnimWeapon *BG_GetWeaponAnim(int weaponAction,playerState_t *ps,int *animIndex);
TNoteTrack *BG_GetWeaponNote( playerState_t* ps, int weapon, int anim, int animChoice, int callbackStep );

typedef enum
{
    WACT_READY,
    WACT_IDLE,
    WACT_FIRE,
    WACT_FIRE_END,      // Special fire-end action for some weapons.
    WACT_ALTFIRE,
    WACT_ALTFIRE_END,   // Special altfire-end action for some weapons.
    WACT_RELOAD,
    WACT_ALTRELOAD,
    WACT_RELOAD_END,    // Special reload-end for M590 shotgun
    WACT_PUTAWAY,
    WACT_ZOOMIN,
    WACT_ZOOMOUT,
    WACT_CHARGE,
    WACT_ALTCHARGE
} WACT;


#define ARENAS_PER_TIER     4
#define MAX_ARENAS          1024
#define MAX_ARENAS_TEXT     8192

#define MAX_BOTS            1024
#define MAX_BOTS_TEXT       8192


#define HL_NONE         0x00000000  // 0

#define HL_FOOT_RT      0x00000001
#define HL_FOOT_LT      0x00000002
#define HL_LEG_UPPER_RT 0x00000004
#define HL_LEG_UPPER_LT 0x00000008
#define HL_LEG_LOWER_RT 0x00000010  // 5

#define HL_LEG_LOWER_LT 0x00000020
#define HL_HAND_RT      0x00000040
#define HL_HAND_LT      0x00000080
#define HL_ARM_RT       0x00000100
#define HL_ARM_LT       0x00000200  // 10

#define HL_HEAD         0x00000400
#define HL_WAIST        0x00000800
#define HL_BACK_RT      0x00001000
#define HL_BACK_LT      0x00002000
#define HL_BACK         0x00004000  // 15

#define HL_CHEST_RT     0x00008000
#define HL_CHEST_LT     0x00010000
#define HL_CHEST        0x00020000
#define HL_NECK         0x00040000
#define HL_DEBUG        0x00080000  // 20

#define HL_MAX          0x00100000

#define HL_DISMEMBERBIT 0x80000000

#define GORE_NONE       0x80000000


/*******************************************************************************
 *
 *  Dynamic gametype system
 *
 *******************************************************************************/

#define MAX_GAMETYPES           128
#define MAX_GAMETYPE_PHOTOS     4

typedef enum
{
    RT_NORMAL,
    RT_DELAYED,
    RT_INTERVAL,
    RT_NONE,
    RT_MAX

} respawnType_t;

typedef struct gametypePhoto_s
{
    const char*     name;
    const char*     displayName;

} gametypePhoto_t;

typedef struct gametypeData_s
{
    const char*     name;
    const char*     displayName;
    const char*     script;
    const char*     description;
    const char*     basegametype;

    respawnType_t   respawnType;
    qboolean        pickupsDisabled;
    qboolean        teams;
    qboolean        showKills;

    int             backpack;

    gametypePhoto_t photos[4];

} gametypeData_t;

extern gametypeData_t   bg_gametypeData[];
extern int              bg_gametypeCount;

qboolean    BG_BuildGametypeList    ( void );
int         BG_FindGametype         ( const char* name );

/*******************************************************************************
 *
 *  Player model templates
 *
 *******************************************************************************/

#define MAX_OUTFITTING_NAME     64

typedef struct goutfitting_s
{
    char        name  [ MAX_OUTFITTING_NAME ];
    int         items [ OUTFITTING_GROUP_MAX ];

} goutfitting_t;

typedef struct SSurfaceList
{
    const char              *mName;

    struct SSurfaceList     *mNext;

} TSurfaceList;

typedef struct SItemTemplate
{
    const char              *mName;
    const char              *mModel;
    TSurfaceList            *mOnList;
    TSurfaceList            *mOffList;

    struct SItemTemplate    *mNext;

} TItemTemplate;

typedef struct SInventoryTemplate
{
    const char                  *mName;
    const char                  *mBolt;
    TItemTemplate               *mItem;

    struct SInventoryTemplate   *mNext;

    qboolean                    mOnBack;

    int                         mBoltIndex;
    int                         mModelIndex;

} TInventoryTemplate;

typedef struct SSkinTemplate
{
    const char              *mSkin;
    TInventoryTemplate      *mInventory;

    struct SSkinTemplate    *mNext;

} TSkinTemplate;

#define MAX_MODEL_SOUNDS            8
#define MAX_IDENTITIES              256
#define MAX_OUTFITTINGS             64
#define MAX_OUTFITTING_GROUPITEM    12

typedef struct SModelSounds
{
    const char          *mName;
    const char          *mSounds[MAX_MODEL_SOUNDS];
    int                 mCount;

    struct SModelSounds *mNext;

} TModelSounds;

typedef struct SCharacterTemplate
{
    const char      *mName;
    const char      *mModel;
    const char      *mParentName;
    const char      *mFormalName;
    int             mSoundCount;
    qboolean        mDeathmatch;

    TInventoryTemplate  *mInventory;
    TSkinTemplate       *mSkins;
    TModelSounds        *mSounds;

    struct SCharacterTemplate   *mParent;
    struct SCharacterTemplate   *mNext;

} TCharacterTemplate;

typedef struct SIdentity
{
    const char          *mName;
    const char          *mTeam;
    TCharacterTemplate  *mCharacter;
    TSkinTemplate       *mSkin;
    qhandle_t           mIcon;

} TIdentity;

extern TCharacterTemplate   *bg_characterTemplates;
extern TItemTemplate        *bg_itemTemplates;
extern TIdentity            bg_identities[];
extern int                  bg_identityCount;
extern goutfitting_t        bg_outfittings[];
extern int                  bg_outfittingCount;
extern int                  bg_outfittingGroups[][MAX_OUTFITTING_GROUPITEM];
extern char                 *bg_weaponNames[WP_NUM_WEAPONS];
extern stringID_table_t     bg_animTable [MAX_ANIMATIONS+1];

TIdentity*          BG_FindIdentity                     ( const char *identityName );
TIdentity*          BG_FindTeamIdentity                 ( const char *identityName, int index );
TCharacterTemplate* BG_FindCharacterTemplate            ( const char *name );
const char*         BG_GetModelSound                    ( const char *identityName, const char *SoundGroup, int index );
qboolean            BG_ParseNPCFiles                    ( void );
int                 BG_ParseSkin                        ( const char* filename, char* pairs, int pairsSize );

qboolean            BG_IsWeaponAvailableForOutfitting   ( weapon_t weapon, int level );
void                BG_SetAvailableOutfitting           ( const char* available );
void                BG_DecompressOutfitting             ( const char* compressed, goutfitting_t* outfitting );
void                BG_CompressOutfitting               ( goutfitting_t* outfitting, char* compressed, int size );
int                 BG_ParseOutfittingTemplates         ( qboolean force );
int                 BG_FindOutfitting                   ( goutfitting_t* outfitting);
void                BG_ApplyLeanOffset                  ( playerState_t* ps, vec3_t origin );

/*******************************************************************************
 *
 *  Trap calls that are shared in game, cgame, and user interface and accessed
 *  by the bg_ code.
 *
 *******************************************************************************/

typedef void    *TGenericParser2;
typedef void    *TGPGroup;
typedef void    *TGPValue;

// CGenericParser2 (void *) routines
TGenericParser2     trap_GP_Parse                   ( char **dataPtr );
TGenericParser2     trap_GP_ParseFile               ( char *fileName );
void                trap_GP_Clean                   ( TGenericParser2 GP2 );
void                trap_GP_Delete                  ( TGenericParser2 *GP2 );
TGPGroup            trap_GP_GetBaseParseGroup       ( TGenericParser2 GP2 );

// CGPGroup (void *) routines
qboolean            trap_GPG_GetName                ( TGPGroup GPG, char *dest, int destSize );
TGPGroup            trap_GPG_GetNext                ( TGPGroup GPG );
TGPGroup            trap_GPG_GetInOrderNext         ( TGPGroup GPG );
TGPGroup            trap_GPG_GetInOrderPrevious     ( TGPGroup GPG );
TGPValue            trap_GPG_GetPairs               ( TGPGroup GPG );
TGPGroup            trap_GPG_GetInOrderPairs        ( TGPGroup GPG );
TGPGroup            trap_GPG_GetSubGroups           ( TGPGroup GPG );
TGPGroup            trap_GPG_GetInOrderSubGroups    ( TGPGroup GPG );
TGPGroup            trap_GPG_FindSubGroup           ( TGPGroup GPG, const char *name );
TGPValue            trap_GPG_FindPair               ( TGPGroup GPG, const char *key );
qboolean            trap_GPG_FindPairValue          ( TGPGroup GPG, const char *key, const char *defaultVal, char *dest, int destSize );

// CGPValue (void *) routines
qboolean            trap_GPV_GetName                ( TGPValue GPV, char *dest, int destSize );
TGPValue            trap_GPV_GetNext                ( TGPValue GPV );
TGPValue            trap_GPV_GetInOrderNext         ( TGPValue GPV );
TGPValue            trap_GPV_GetInOrderPrevious     ( TGPValue GPV );
qboolean            trap_GPV_IsList                 ( TGPValue GPV );
qboolean            trap_GPV_GetTopValue            ( TGPValue GPV, char *dest, int destSize );
TGPValue            trap_GPV_GetList                ( TGPValue GPV );

extern int          trap_FS_GetFileList             ( const char *path, const char *extension, char *listbuf, int bufsize );

//
// Game module memory management system.
//

void        *G_Alloc                            ( int size );
void        *G_AllocUnaligned                   ( int size );
void        *G_TempAlloc                        ( int size );
void        G_TempFree                          ( int size );
char        *G_StringAlloc                      ( const char *source );

void        G_InitMemory                        ( void );

#endif // __BG_PUBLIC_H__
