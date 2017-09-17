// Copyright (C) 2001-2002 Raven Software
//
// bg_weapons.h - weapons data loading

#ifndef __BG_WEAPONS_H__
#define __BG_WEAPONS_H__

// means of death
typedef enum 
{
    MOD_UNKNOWN,

    // Knife
    MOD_KNIFE,

    // Pistols
    MOD_M1911A1_PISTOL,
    MOD_USSOCOM_PISTOL,         
    MOD_SILVER_TALON,

    // Secondarys
    MOD_M590_SHOTGUN,
    MOD_MICRO_UZI_SUBMACHINEGUN, 
    MOD_M3A1_SUBMACHINEGUN,      
    MOD_MP5,

    // Primaries
    MOD_USAS_12_SHOTGUN,         
    MOD_M4_ASSAULT_RIFLE,        
    MOD_AK74_ASSAULT_RIFLE,      
    MOD_SIG551,

    MOD_MSG90A1_SNIPER_RIFLE,    
    MOD_M60_MACHINEGUN,          
    MOD_MM1_GRENADE_LAUNCHER,    
    MOD_RPG7_LAUNCHER,           

    // Grenades
    MOD_M84_GRENADE,
    MOD_SMOHG92_GRENADE,
    MOD_ANM14_GRENADE,
    MOD_M15_GRENADE,

    MOD_WATER,
    MOD_CRUSH,
    MOD_TELEFRAG,
    MOD_FALLING,
    MOD_SUICIDE,
    MOD_TEAMCHANGE,
    MOD_TARGET_LASER,
    MOD_TRIGGER_HURT,
    MOD_TRIGGER_HURT_NOSUICIDE

} meansOfDeath_t;

typedef enum 
{
    WP_NONE,

    // Knife
    WP_KNIFE,

    // Pistols
    WP_M1911A1_PISTOL,
    WP_USSOCOM_PISTOL,
    WP_SILVER_TALON,

    // Secondarys
    WP_M590_SHOTGUN,
    WP_MICRO_UZI_SUBMACHINEGUN,
    WP_M3A1_SUBMACHINEGUN,
    WP_MP5,

    // Primaries
    WP_USAS_12_SHOTGUN,
    WP_M4_ASSAULT_RIFLE,
    WP_AK74_ASSAULT_RIFLE,
    WP_SIG551,

    WP_MSG90A1,
    WP_M60_MACHINEGUN,
    WP_MM1_GRENADE_LAUNCHER,
    WP_RPG7_LAUNCHER,

    // Grenades
    WP_M84_GRENADE,
    WP_SMOHG92_GRENADE,
    WP_ANM14_GRENADE,
    WP_M15_GRENADE,

    WP_NUM_WEAPONS
} weapon_t;

#define WP_DELAYED_CHANGE_BIT   (1<<5)
                
typedef enum 
{                           
    AMMO_KNIFE,
    AMMO_045,
    AMMO_556,
    AMMO_9,
    AMMO_12 ,
    AMMO_762,
    AMMO_40,
    AMMO_RPG7,
    AMMO_M15,
    AMMO_M84,
    AMMO_SMOHG92,
    AMMO_ANM14,

    AMMO_762_BELT,

    AMMO_MP5_9,

    AMMO_MAX,

    AMMO_NONE,

} ammo_t;

#define WP_FIREMODE_NONE        0
#define WP_FIREMODE_AUTO        1
#define WP_FIREMODE_BURST       2
#define WP_FIREMODE_SINGLE      3
#define WP_FIREMODE_MAX         4

#define PROJECTILE_FIRE         0x0010  // projectile NOT bullet
#define PROJECTILE_TIMED        0x0020  // projectile ONLY explodes after time is up
#define PROJECTILE_GRAVITY      0x0040  // projectile obeys gravity
#define PROJECTILE_DAMAGE_AREA  0x0080 // projectile does area damage over time
#define UNLOCK_MUZZLEFLASH      0x0100  // muzzle flash is locked to muzzle bolt by default
#define PROJECTILE_LIGHTGRAVITY 0x0200  // projectile has light gravity

typedef enum
{
    CAT_NONE = 0,
    CAT_KNIFE,
    CAT_PISTOL,
    CAT_SHOTGUN,
    CAT_SUB,
    CAT_ASSAULT,
    CAT_SNIPER,
    CAT_HEAVY,
    CAT_GRENADE,
    CAT_MAX

} ECategory;

#define MAX_ZOOMNAME    8
#define ZOOMLEVEL_MAX   3

typedef struct zoomData_s
{
    int     fov;
    char    name[MAX_ZOOMNAME];

} zoomData_t;

typedef struct attackData_s
{
    char            name[MAX_QPATH];
    char            icon[MAX_QPATH];

    const char*     melee;

    meansOfDeath_t  mod;                        // means of death
    int             ammoIndex;                  // Index to proper ammo slot
    union 
    {
        int     range;                          // Range of weapon 
        int     velocity;                       // speed of projectile
    } rV;

    int         clipSize;                       // how large is a clip
    int         fireAmount;                     // how much ammo to use per shot
    int         fireFromClip;                   // 0 = fire from approp. ammo pool, 1 = fire from clip
    int         damage;                         // how much damage is done per hit
    float       inaccuracy;                     // how inaccurate is weapon
    float       zoomInaccuracy;                 // how inaccurate is the weapon when zoomed
    float       maxInaccuracy;                  // maximum lvl of inaccuracy
    int         pellets;                        // how many individual 'bullets' are shot with one trigger pull?
    int         weaponFlags;                    // which fire modes are available, projectiles timed or impact, .etc
    int         projectileLifetime;             // how long does projectile live (before exploding)
    int         splashRadius;                   // how large is splash damage radius
    int         fireDelay;                      // Extra delay when firing
    qboolean    gore;                           // is gore enabled for this attack?
    int         extraClips;                     // Extra clips you get when starting
    float       bounceScale;                    // how much something bounces

    vec3_t      minKickAngles;
    vec3_t      maxKickAngles;

    // Names of effects, sounds, models, bones
    char        muzzleEffect[MAX_QPATH];
    char        muzzleEffectBone[MAX_QPATH];
    char        muzzleEffectInWorld[MAX_QPATH];
    char        tracerEffect[MAX_QPATH];
    char        ejectBone[MAX_QPATH];
    char        shellEject[MAX_QPATH];
    char        explosionSound[MAX_QPATH];
    char        explosionEffect[MAX_QPATH];
    char        missileG2Model[MAX_QPATH];

    int         animFire;
    int         animFireZoomed;

} attackData_t;

typedef struct weaponData_s
{
    char            *classname;                 // Spawning name
    ECategory       category;                   // what group of weapons is this one part of?
    qboolean        safe;
    char            worldModel[MAX_QPATH];      // world model
    char            menuImage[MAX_QPATH];       // names of the icon files

    int             animDrop;
    int             animRaise;
    int             animIdle;
    int             animIdleZoomed;
    int             animReload;
    int             animReloadStart;
    int             animReloadEnd;

    attackData_t    attack[ATTACK_MAX];

    zoomData_t      zoom[ZOOMLEVEL_MAX];

} weaponData_t;

typedef struct  ammoData_s
{
    char    *name;              // name of ammo
    char    icon[32];           // Name of ammo icon file
    int     max;                // Max amount player can hold of ammo
    float   goreScale;

} ammoData_t;

extern char *weaponNames[WP_NUM_WEAPONS];
extern weaponData_t weaponData[WP_NUM_WEAPONS];
extern char *ammoNames[AMMO_MAX];
extern ammoData_t ammoData[AMMO_MAX];

// Specific weapon information

#define WP_FIRST_RANGED_WEAPON      WP_M1911A1_PISTOL   // this is the first weapon for next and prev weapon switching
#define WP_FIRST_MELEE_WEAPON       WP_KNIFE
#define MAX_PLAYER_WEAPONS          (WP_NUM_WEAPONS-1)  // this is the max you can switch to and get with the give all.

#define MAX_WEAPON_SOUNDS       12
#define MAX_WEAPON_SOUND_SLOTS  3

#define MAX_SIDE_SURFACES 16

typedef struct SOptionalWeapon
{
    char                    mName[MAX_QPATH];
    char                    mMuzzle[MAX_QPATH];
    char                    *mSurfaces[MAX_SIDE_SURFACES];

    struct SOptionalWeapon  *mNext;
} TOptionalWeapon;

typedef struct SBoltonWeapon
{
    char                mName[MAX_QPATH];
    char                mModel[MAX_QPATH];
    char                mParent[MAX_QPATH];
    char                mBoltToBone[MAX_QPATH];
    char                *mRightSide[MAX_SIDE_SURFACES];

    char                mJointBone[MAX_QPATH];
    char                mJointParentBone[MAX_QPATH];
    char                mJointForward[10];
    char                mJointRight[10];
    char                mJointUp[10];
} TBoltonWeapon;

typedef struct SNoteTrack
{
    char                    mNote[64];
    int                     mFrame;

    struct SNoteTrack       *mNext;
} TNoteTrack;

#define MAX_WEAPON_ANIM_CHOICES     4

typedef struct SAnimInfoWeapon
{
    char                    mName[MAX_QPATH];
    char                    mType[MAX_QPATH];
    char                    *mAnim[MAX_WEAPON_ANIM_CHOICES];
    char                    *mTransition[MAX_WEAPON_ANIM_CHOICES];
    char                    *mEnd[MAX_WEAPON_ANIM_CHOICES];
    float                   mSpeed;
    int                     mLODBias;
    int                     mNumChoices;
                            
    int                     mStartFrame[MAX_WEAPON_ANIM_CHOICES];
    int                     mNumFrames[MAX_WEAPON_ANIM_CHOICES];
    int                     mFPS[MAX_WEAPON_ANIM_CHOICES];

    struct SNoteTrack       *mNoteTracks[MAX_WEAPON_ANIM_CHOICES];
    struct SAnimInfoWeapon  *mNext;
} TAnimInfoWeapon;

typedef struct SAnimWeapon
{
    char                    mName[MAX_QPATH];
    char                    mMuzzle[MAX_QPATH];

    struct SAnimInfoWeapon  *mInfos;
    struct SAnimInfoWeapon  *mWeaponModelInfo;      // "weaponmodel" info
    struct SAnimWeapon      *mNext;

} TAnimWeapon;

typedef struct SWeaponModel
{
    char                        mName[MAX_QPATH];
    char                        mModel[MAX_QPATH];
    char                        mBufferName[MAX_QPATH];
    char                        mBufferModel[MAX_QPATH];
    char                        mBufferBoltToBone[MAX_QPATH];
    char                        mBufferMuzzle[MAX_QPATH];
    char                        mBufferAltMuzzle[MAX_QPATH];
    char                        mLeftHandsBoltToBone[MAX_QPATH];
    char                        mRightHandsBoltToBone[MAX_QPATH];
    char                        *mFrontSurfaces[MAX_SIDE_SURFACES],
                                *mRightSideSurfaces[MAX_SIDE_SURFACES],
                                *mLeftSideSurfaces[MAX_SIDE_SURFACES];

    struct SOptionalWeapon      *mOptionalList;
    struct SBoltonWeapon        *mBolton;
} TWeaponModel;

#define MAX_CALLBACK_SURFACES   4

typedef struct SOnOffSurface
{
    char    mName[64];
    int     mStatus;
} TOnOffSurface;

typedef struct SSurfaceCallback
{
    char                    mName[64];
    struct SOnOffSurface    mOnOffSurfaces[MAX_CALLBACK_SURFACES];
} TSurfaceCallback;

#define MAX_SURFACE_CALLBACKS   2

typedef struct SWeaponInfo
{
    char                        *mName;
    float                       mForeshorten;
    vec3_t                      mViewOffset;
    char                        mSoundNames[MAX_WEAPON_SOUNDS][MAX_QPATH];
    char                        mSounds[MAX_WEAPON_SOUNDS][MAX_WEAPON_SOUND_SLOTS][MAX_QPATH];
    struct SSurfaceCallback     mSurfaceCallbacks[MAX_SURFACE_CALLBACKS];
    struct SAnimWeapon          *mAnimList;
    struct SWeaponModel         mWeaponModel;
} TWeaponParseInfo;

extern TWeaponParseInfo weaponParseInfo[WP_NUM_WEAPONS];
extern char             weaponLeftHand[MAX_QPATH];
extern char             weaponRightHand[MAX_QPATH];

qboolean            BG_ParseInviewFile          ( qboolean );
TAnimWeapon*        BG_GetInviewAnim            ( int weaponIdx,const char *animKey,int *animIndex);
TAnimWeapon*        BG_GetInviewAnimFromIndex   ( int weaponIdx,int animIndex);
TAnimInfoWeapon*    BG_GetInviewModelAnim       ( int weaponIdx,const char *modelKey,const char *animKey);
qboolean            BG_WeaponHasAlternateAmmo   ( int weapon );
int                 BG_FindFireMode             ( weapon_t weapon, attackType_t attack, int firemode );

void                BG_CalculateBulletEndpoint  ( vec3_t muzzlePoint, vec3_t fireAngs, float inaccuracy, float range, vec3_t end, int *seed );
int                 BG_GetMaxAmmo               ( const playerState_t* ps, int ammoIndex );

#endif

