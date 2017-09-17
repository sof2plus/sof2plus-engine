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
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

// Product information.
#define PRODUCT_NAME                "SoF2Plus"
#define BASEGAME                    "base"
#define CLIENT_WINDOW_TITLE         "SoF2+"
#define HOMEPATH_NAME_UNIX          ".sof2plus"
#define HOMEPATH_NAME_WIN           "SoF2Plus"
#define HOMEPATH_NAME_MACOSX        HOMEPATH_NAME_WIN
#define GAMENAME_FOR_MASTER         "sof2mp"

// Heartbeat for dpmaster protocol. You shouldn't change this unless you know what you're doing
#define HEARTBEAT_FOR_MASTER        "sof2mp"

// When com_gamename is LEGACY_MASTER_GAMENAME, use quake3 master protocol.
// You shouldn't change this unless you know what you're doing
#define LEGACY_MASTER_GAMENAME      "SoF2MP"
#define LEGACY_HEARTBEAT_FOR_MASTER "SoF2MP-1"

#define BASETA              "missionpack"

#ifndef PRODUCT_VERSION
  #define PRODUCT_VERSION "0.01"
#endif

#ifndef PRODUCT_DATE
#  define PRODUCT_DATE __DATE__
#endif

#define Q3_VERSION PRODUCT_NAME " " PRODUCT_VERSION

#define MAX_TEAMNAME        32
#define MAX_MASTER_SERVERS      5   // number of supported master servers

#define DEMOEXT "dm_"           // standard demo extension

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)     // slightly different base types
#pragma warning(disable : 4100)     // unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)     // decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)     // conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)     // nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)     // benign redefinition
//#pragma warning(disable : 4305)       // truncation from const double to float
//#pragma warning(disable : 4310)       // cast truncates constant value
//#pragma warning(disable:  4505)   // unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)     // unreachable code
#pragma warning(disable : 4711)     // selected for automatic inline expansion
#pragma warning(disable : 4220)     // varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifdef __GNUC__
#define UNUSED_VAR __attribute__((unused))
#else
#define UNUSED_VAR
#endif

#if (defined _MSC_VER)
#define Q_EXPORT __declspec(dllexport)
#elif (defined __SUNPRO_C)
#define Q_EXPORT __global
#elif ((__GNUC__ >= 3) && (!__EMX__) && (!sun))
#define Q_EXPORT __attribute__((visibility("default")))
#else
#define Q_EXPORT
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
  #include <io.h>

  typedef __int64 int64_t;
  typedef __int32 int32_t;
  typedef __int16 int16_t;
  typedef __int8 int8_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int8 uint8_t;

  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
  int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap);
#else
  #include <stdint.h>

  #define Q_vsnprintf vsnprintf
#endif


#include "q_platform.h"

//=============================================================

typedef unsigned char       byte;

typedef enum {qfalse, qtrue}    qboolean;

typedef union {
    float f;
    int i;
    unsigned int ui;
} floatint_t;

typedef int     qhandle_t;
typedef int     sfxHandle_t;
typedef int     fileHandle_t;
typedef int     clipHandle_t;

#define PAD(base, alignment)    (((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment) (PAD((base), (alignment)) - (base))

#define PADP(base, alignment)   ((void *) PAD((intptr_t) (base), (alignment)))

#ifdef __GNUC__
#define QALIGN(x) __attribute__((aligned(x)))
#else
#define QALIGN(x)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define STRING(s)           #s
// expand constants before stringifying them
#define XSTRING(s)          STRING(s)

#define MAX_QINT            0x7fffffff
#define MIN_QINT            (-MAX_QINT-1)

#define ARRAY_LEN(x)            (sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)         (ARRAY_LEN(x) - 1)

// angle indexes
#define PITCH               0       // up / down
#define YAW                 1       // left / right
#define ROLL                2       // fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define MAX_STRING_CHARS    1024    // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS   1024    // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS     1024    // max length of an individual token

#define MAX_INFO_STRING     1024
#define MAX_INFO_KEY          1024
#define MAX_INFO_VALUE      1024

#define BIG_INFO_STRING     8192  // used for system info key only
#define BIG_INFO_KEY          8192
#define BIG_INFO_VALUE      8192


#define MAX_QPATH           64      // max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH          PATH_MAX
#else
#define MAX_OSPATH          256     // max length of a filesystem pathname
#endif

#define MAX_NAME_LENGTH     32      // max length of a client name

#define MAX_SAY_TEXT    150

// paramters for command buffer stuffing
typedef enum {
    EXEC_NOW,           // don't return until completed, a VM should NEVER use this,
                        // because some commands might cause the VM to be unloaded...
    EXEC_INSERT,        // insert at current position, but don't run yet
    EXEC_APPEND         // add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define MAX_MAP_AREA_BYTES      32      // bit vector of area visibility

#define LS_STYLES_START         0
#define LS_NUM_STYLES           32
#define MAX_LIGHT_STYLES        64

// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
    PRINT_ALL,
    PRINT_DEVELOPER,        // only print when "developer 1"
    PRINT_WARNING,
    PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL            // this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
    ERR_FATAL,                  // exit the entire game with a popup window
    ERR_DROP,                   // print to console and disconnect from game
    ERR_SERVERDISCONNECT,       // don't kill server
    ERR_DISCONNECT,             // client disconnected from the server
    ERR_NEED_CD                 // pop up the need-cd dialog
} errorParm_t;

#if !defined(NDEBUG) && !defined(BSPC)
    #define HUNK_DEBUG
#endif

typedef enum {
    h_high,
    h_low,
    h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#define Hunk_Alloc( size, preference )              Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
void *Hunk_AllocDebug( int size, ha_pref preference, char *label, char *file, int line );
#else
void *Hunk_Alloc( int size, ha_pref preference );
#endif

#define Com_Memset memset
#define Com_Memcpy memcpy

/*
==============================================================

MATHLIB

==============================================================
*/

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef int fixed4_t;
typedef int fixed8_t;
typedef int fixed16_t;

#ifndef M_PI
#define M_PI        3.14159265358979323846f // matches value in gcc v2 math.h
#endif

#define NUMVERTEXNORMALS    162
extern  vec3_t  bytedirs[NUMVERTEXNORMALS];

extern  vec4_t      colorBlack;
extern  vec4_t      colorRed;
extern  vec4_t      colorGreen;
extern  vec4_t      colorBlue;
extern  vec4_t      colorYellow;
extern  vec4_t      colorMagenta;
extern  vec4_t      colorCyan;
extern  vec4_t      colorWhite;
extern  vec4_t      colorLtGrey;
extern  vec4_t      colorMdGrey;
extern  vec4_t      colorDkGrey;

#define Q_COLOR_ESCAPE  '^'
#define Q_IsColorString(p)  ((p) && *(p) == Q_COLOR_ESCAPE && *((p)+1) && isalnum(*((p)+1))) // ^[0-9a-zA-Z]

#define COLOR_BLACK '0'
#define COLOR_RED   '1'
#define COLOR_GREEN '2'
#define COLOR_YELLOW    '3'
#define COLOR_BLUE  '4'
#define COLOR_CYAN  '5'
#define COLOR_MAGENTA   '6'
#define COLOR_WHITE '7'
#define ColorIndexForNumber(c) ((c) & 0x07)
#define ColorIndex(c) (ColorIndexForNumber((c) - '0'))

#define S_COLOR_BLACK   "^0"
#define S_COLOR_RED "^1"
#define S_COLOR_GREEN   "^2"
#define S_COLOR_YELLOW  "^3"
#define S_COLOR_BLUE    "^4"
#define S_COLOR_CYAN    "^5"
#define S_COLOR_MAGENTA "^6"
#define S_COLOR_WHITE   "^7"

extern vec4_t   g_color_table[8];

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

struct cplane_s;

extern  vec3_t  vec3_origin;
extern  vec3_t  axisDefault[3];

#define nanmask (255<<23)

#define IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

int Q_isnan(float x);

#if idx64
  extern long qftolsse(float f);
  extern int qvmftolsse(void);
  extern void qsnapvectorsse(vec3_t vec);

  #define Q_ftol qftolsse
  #define Q_SnapVector qsnapvectorsse

  extern int (*Q_VMftol)(void);
#elif id386
  extern long QDECL qftolx87(float f);
  extern long QDECL qftolsse(float f);
  extern int QDECL qvmftolx87(void);
  extern int QDECL qvmftolsse(void);
  extern void QDECL qsnapvectorx87(vec3_t vec);
  extern void QDECL qsnapvectorsse(vec3_t vec);

  extern long (QDECL *Q_ftol)(float f);
  extern int (QDECL *Q_VMftol)(void);
  extern void (QDECL *Q_SnapVector)(vec3_t vec);
#else
  // Q_ftol must expand to a function name so the pluggable renderer can take
  // its address
  #define Q_ftol lrintf
  #define Q_SnapVector(vec)\
    do\
    {\
        vec3_t *temp = (vec);\
        \
        (*temp)[0] = round((*temp)[0]);\
        (*temp)[1] = round((*temp)[1]);\
        (*temp)[2] = round((*temp)[2]);\
    } while(0)
#endif
/*
// if your system does not have lrintf() and round() you can try this block. Please also open a bug report at bugzilla.icculus.org
// or write a mail to the ioq3 mailing list.
#else
  #define Q_ftol(v) ((long) (v))
  #define Q_round(v) do { if((v) < 0) (v) -= 0.5f; else (v) += 0.5f; (v) = Q_ftol((v)); } while(0)
  #define Q_SnapVector(vec) \
    do\
    {\
        vec3_t *temp = (vec);\
        \
        Q_round((*temp)[0]);\
        Q_round((*temp)[1]);\
        Q_round((*temp)[2]);\
    } while(0)
#endif
*/

#if idppc

static ID_INLINE float Q_rsqrt( float number ) {
        float x = 0.5f * number;
                float y;
#ifdef __GNUC__
                asm("frsqrte %0,%1" : "=f" (y) : "f" (number));
#else
        y = __frsqrte( number );
#endif
        return y * (1.5f - (x * y * y));
    }

#ifdef __GNUC__
static ID_INLINE float Q_fabs(float x) {
    float abs_x;

    asm("fabs %0,%1" : "=f" (abs_x) : "f" (x));
    return abs_x;
}
#else
#define Q_fabs __fabsf
#endif

#else
float Q_fabs( float f );
float Q_rsqrt( float f );       // reciprocal square root
#endif

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

signed char ClampChar( int i );
signed short ClampShort( int i );

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#define DotProduct(x,y)         ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)   ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)        ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)         ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define VectorScale(v, s, o)    ((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define VectorMA(v, s, b, o)    ((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))

#define VectorClear(a)          ((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)       ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)   ((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)        ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define Byte4Copy(a,b)          ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
// just in case you don't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

static ID_INLINE int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
    if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
        return 0;
    }
    return 1;
}

static ID_INLINE vec_t VectorLength( const vec3_t v ) {
    return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t VectorLengthSquared( const vec3_t v ) {
    return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
    vec3_t  v;

    VectorSubtract (p2, p1, v);
    return VectorLength( v );
}

static ID_INLINE vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
    vec3_t  v;

    VectorSubtract (p2, p1, v);
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
static ID_INLINE void VectorNormalizeFast( vec3_t v )
{
    float ilength;

    ilength = Q_rsqrt( DotProduct( v, v ) );

    v[0] *= ilength;
    v[1] *= ilength;
    v[2] *= ilength;
}

static ID_INLINE void VectorInverse( vec3_t v ){
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

static ID_INLINE void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
    cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
    cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

vec_t VectorNormalize (vec3_t v);       // returns vector length
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );
int Q_log2(int val);

float Q_acos(float c);

int     Q_rand( int *seed );
float   Q_random( int *seed );
float   Q_crandom( int *seed );

#define random()    ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()   (2.0 * (random() - 0.5))

void vectoangles( const vec3_t value1, vec3_t angles);
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );

void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

qboolean BoundsIntersect(const vec3_t mins, const vec3_t maxs,
        const vec3_t mins2, const vec3_t maxs2);
qboolean BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
        const vec3_t origin, vec_t radius);
qboolean BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
        const vec3_t origin);

float   AngleMod(float a);
float   LerpAngle (float from, float to, float frac);
float   AngleSubtract( float a1, float a2 );
void    AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

//int   PlaneTypeForNormal (vec3_t normal);

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

//=============================================

float Com_Clamp( float min, float max, float value );

char    *COM_SkipPath( char *pathname );
const char  *COM_GetExtension( const char *name );
void    COM_StripExtension(const char *in, char *out, int destsize);
qboolean COM_CompareExtension(const char *in, const char *ext);
void    COM_DefaultExtension( char *path, int maxSize, const char *extension );

void    COM_BeginParseSession( const char *name );
int     COM_GetCurrentParseLine( void );
char    *COM_Parse( char **data_p );
char    *COM_ParseExt( char **data_p, qboolean allowLineBreak );
int     COM_Compress( char *data_p );
void    COM_ParseError( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void    COM_ParseWarning( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
//int       COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

#define MAX_TOKENLENGTH     1024

#ifndef TT_STRING
//token types
#define TT_STRING                   1           // string
#define TT_LITERAL                  2           // literal
#define TT_NUMBER                   3           // number
#define TT_NAME                     4           // name
#define TT_PUNCTUATION              5           // punctuation
#endif

typedef struct pc_token_s
{
    int type;
    int subtype;
    int intvalue;
    float floatvalue;
    char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

void    COM_MatchToken( char**buf_p, char *match );

qboolean SkipBracedSection (char **program, int depth);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);
int Com_HexStrToInt( const char *str );

int QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

char *Com_SkipTokens( char *s, int numTokens, char *sep );
char *Com_SkipCharset( char *s, char *sep );

void Com_RandomBytes( byte *string, int len );

// mode parm for FS_FOpenFile
typedef enum {
    FS_READ,
    FS_WRITE,
    FS_APPEND,
    FS_APPEND_SYNC
} fsMode_t;

typedef enum {
    FS_SEEK_CUR,
    FS_SEEK_END,
    FS_SEEK_SET
} fsOrigin_t;

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
qboolean Q_isanumber( const char *s );
qboolean Q_isintegral( float f );

// portable case insensitive compare
int     Q_stricmp (const char *s1, const char *s2);
int     Q_strncmp (const char *s1, const char *s2, int n);
int     Q_stricmpn (const char *s1, const char *s2, int n);
char    *Q_strlwr( char *s1 );
char    *Q_strupr( char *s1 );
const char  *Q_stristr( const char *s, const char *find);

// buffer size safe library replacements
void    Q_strncpyz( char *dest, const char *src, int destsize );
void    Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
    byte    b0;
    byte    b1;
    byte    b2;
    byte    b3;
    byte    b4;
    byte    b5;
    byte    b6;
    byte    b7;
} qint64;

//=============================================
/*
short   BigShort(short l);
short   LittleShort(short l);
int     BigLong (int l);
int     LittleLong (int l);
qint64  BigLong64 (qint64 l);
qint64  LittleLong64 (qint64 l);
float   BigFloat (const float *l);
float   LittleFloat (const float *l);

void    Swap_Init (void);
*/
char    * QDECL va(char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define TRUNCATE_LENGTH 64
void Com_TruncateLongString( char *buffer, const char *s );

//=============================================

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_Big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link
void    QDECL Com_Error( int level, const char *error, ... ) __attribute__ ((noreturn, format(printf, 2, 3)));
void    QDECL Com_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define CVAR_ARCHIVE        0x0001  // set to cause it to be saved to vars.rc
                    // used for system variables, not for player
                    // specific configurations
#define CVAR_USERINFO       0x0002  // sent to server on connect or change
#define CVAR_SERVERINFO     0x0004  // sent in response to front end requests
#define CVAR_SYSTEMINFO     0x0008  // these cvars will be duplicated on all clients
#define CVAR_INIT       0x0010  // don't allow change from console at all,
                    // but can be set from the command line
#define CVAR_LATCH      0x0020  // will only change when C code next does
                    // a Cvar_Get(), so it can't be changed
                    // without proper initialization.  modified
                    // will be set, even though the value hasn't
                    // changed yet
#define CVAR_ROM        0x0040  // display only, cannot be set by user at all
#define CVAR_USER_CREATED   0x0080  // created by a set command
#define CVAR_TEMP       0x0100  // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT      0x0200  // can not be changed if cheats are disabled
#define CVAR_NORESTART      0x0400  // do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED 0x0800  // cvar was created by a server the client connected to.
#define CVAR_VM_CREATED     0x1000  // cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED      0x2000  // prevent modifying this var from VMs or the server
// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED       0x40000000  // Cvar was modified
#define CVAR_NONEXISTENT    0x80000000  // Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s cvar_t;

struct cvar_s {
    char            *name;
    char            *string;
    char            *resetString;       // cvar_restart will reset to this value
    char            *latchedString;     // for CVAR_LATCH vars
    int             flags;
    qboolean    modified;           // set each time the cvar is changed
    int             modificationCount;  // incremented each time the cvar is changed
    float           value;              // atof( string )
    int             integer;            // atoi( string )
    qboolean    validate;
    qboolean    integral;
    float           min;
    float           max;
    char            *description;

    cvar_t *next;
    cvar_t *prev;
    cvar_t *hashNext;
    cvar_t *hashPrev;
    int         hashIndex;
};

#define MAX_CVAR_VALUE_STRING   256

typedef int cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
    cvarHandle_t    handle;
    int         modificationCount;
    float       value;
    int         integer;
    char        string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"           // shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define PLANE_X         0
#define PLANE_Y         1
#define PLANE_Z         2
#define PLANE_NON_AXIAL 3


/*
=================
PlaneTypeForNormal
=================
*/

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
    vec3_t  normal;
    float   dist;
    byte    type;           // for fast side tests: 0,1,2 = axial, 3 = nonaxial
    byte    signbits;       // signx + (signy<<1) + (signz<<2), used as lookup during collision
    byte    pad[2];
} cplane_t;


// a trace is returned when a box is swept through the world
typedef struct {
    qboolean    allsolid;   // if true, plane is not valid
    qboolean    startsolid; // if true, the initial point was in a solid area
    float       fraction;   // time completed, 1.0 = didn't hit anything
    vec3_t      endpos;     // final position
    cplane_t    plane;      // surface normal at impact, transformed to world space
    int         surfaceFlags;   // surface hit
    int         contents;   // contents on other side of surface hit
    int         entityNum;  // entity the contacted sirface is a part of
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by R_MarkFragments()
typedef struct {
    int     firstPoint;
    int     numPoints;
} markFragment_t;

typedef struct {
    vec3_t      origin;
    vec3_t      axis[3];
} orientation_t;

typedef struct
{
    float       yawAngle;
    qboolean    yawing;

    float       pitchAngle;
    qboolean    pitching;

    int         anim;
    int         animTime;

} animInfo_t;

/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define ANGLE2SHORT(x)  ((int)((x)*65536/360) & 65535)
#define SHORT2ANGLE(x)  ((x)*(360.0/65536))

#define SNAPFLAG_RATE_DELAYED   1
#define SNAPFLAG_NOT_ACTIVE     2   // snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT    4   // toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define MAX_CLIENTS         64      // absolute limit
#define MAX_LOCATIONS       64
#define MAX_TERRAINS        32
#define MAX_LADDERS         64

#define MAX_INSTANCE_TYPES      16

#define GENTITYNUM_BITS     10      // don't need to send any more
#define MAX_GENTITIES       (1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define ENTITYNUM_NONE      (MAX_GENTITIES-1)
#define ENTITYNUM_WORLD     (MAX_GENTITIES-2)
#define ENTITYNUM_MAX_NORMAL    (MAX_GENTITIES-2)


#define MAX_MODELS          256     // these are sent over the net as 8 bits
#define MAX_SOUNDS          256     // so they cannot be blindly increased
#define MAX_AMBIENT_SOUNDSETS   64
#define MAX_FX                  64      // max effects strings, I'm hoping that 64 will be plenty
#define MAX_SUB_BSP             32
#define MAX_ICONS               32
#define MAX_CHARSKINS           64      // character skins
#define MAX_HUDICONS            16      // icons on hud

#define MAX_CONFIGSTRINGS       1400

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define CS_SERVERINFO       0       // an info string with all the serverinfo cvars
#define CS_SYSTEMINFO       1       // an info string for server system to client system configuration (timescale, etc)
#define CS_PLAYERS          2
#define CS_CUSTOM           (CS_PLAYERS + MAX_CLIENTS )

#define MAX_GAMESTATE_CHARS 16000
typedef struct {
    int         stringOffsets[MAX_CONFIGSTRINGS];
    char        stringData[MAX_GAMESTATE_CHARS];
    int         dataCount;
} gameState_t;

//=========================================================

// bit field limits
#define MAX_STATS               16
#define MAX_PERSISTANT          16
#define MAX_AMMO                16
#define MAX_POWERUPS            16
#define MAX_WEAPONS             32
#define MAX_GAMETYPE_ITEMS      5

#define MAX_PS_EVENTS           4

#define PS_PMOVEFRAMECOUNTBITS  6

typedef enum
{
    ATTACK_NORMAL,
    ATTACK_ALTERNATE,
    ATTACK_MAX

} attackType_t;

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s
{
    int         commandTime;    // cmd->serverTime of last executed command
    int         pm_type;
    int         bobCycle;       // for view bobbing and footstep generation
    int         pm_flags;       // ducked, etc
    int         pm_debounce;    // debounce buttons
    int         pm_time;

    vec3_t      origin;
    vec3_t      velocity;

    int         weaponTime;
    int         weaponFireBurstCount;
    int         weaponAnimId;
    int         weaponAnimIdChoice;
    int         weaponAnimTime;
    int         weaponCallbackTime;
    int         weaponCallbackStep;

    int         gravity;
    int         speed;
    int         delta_angles[3];                // add to command angles to get view direction
                                                // changed by spawns, rotating objects, and teleporters
    int         groundEntityNum;                // ENTITYNUM_NONE = in air

    int         legsAnim;                       // mask off ANIM_TOGGLEBIT

    int         torsoTimer;                     // don't change low priority animations until this runs out
    int         torsoAnim;                      // mask off ANIM_TOGGLEBIT

    int         movementDir;                    // a number 0 to 7 that represents the reletive angle
                                                // of movement to the view angle (axial and diagonals)
                                                // when at rest, the value will remain unchanged
                                                // used to twist the legs during strafing

    int         eFlags;                         // copied to entityState_t->eFlags

    int         eventSequence;                  // pmove generated events
    int         events[MAX_PS_EVENTS];
    int         eventParms[MAX_PS_EVENTS];

    int         externalEvent;                  // events set on player from another source
    int         externalEventParm;
    int         externalEventTime;

    int         clientNum;                      // ranges from 0 to MAX_CLIENTS-1
    int         weapon;                         // copied to entityState_t->weapon
    int         weaponstate;

    vec3_t      viewangles;                     // for fixed views
    int         viewheight;

    // damage feedback
    int         damageEvent;                    // when it changes, latch the other parms
    int         damageYaw;
    int         damagePitch;
    int         damageCount;

    int         painTime;                       // used for both game and client side to process the pain twitch - NOT sent across the network
    int         painDirection;                  // NOT sent across the network

    int         stats[MAX_STATS];
    int         persistant[MAX_PERSISTANT];     // stats that aren't cleared on death
    int         ammo[MAX_AMMO];
    int         clip[ATTACK_MAX][MAX_WEAPONS];
    int         firemode[MAX_WEAPONS];

    int         generic1;
    int         loopSound;

    // Incaccuracy values for firing
    int         inaccuracy;
    int         inaccuracyTime;
    int         kickPitch;

    // not communicated over the net at all
    int         ping;                           // server to game info for scoreboard
    int         pmove_framecount;               // FIXME: don't transmit over the network
    int         jumppad_frame;
    int         entityEventSequence;
    vec3_t      pvsOrigin;                      // view origin used to calculate PVS (also the lean origin)
                                                // THIS VARIABLE MUST AT LEAST BE THE PLAYERS ORIGIN ALL OF THE
                                                // TIME OR THE PVS CALCULATIONS WILL NOT WORK.

                                                // Zooming
    int         zoomTime;
    int         zoomFov;

    // Ladders
    int         ladder;
    int         leanTime;

    // Timers
    int         grenadeTimer;
    int         respawnTimer;
} playerState_t;

typedef enum
{
    TEAM_FREE,
    TEAM_RED,
    TEAM_BLUE,
    TEAM_SPECTATOR,

    TEAM_NUM_TEAMS

} team_t;

//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define BUTTON_ATTACK       (1<<0)
#define BUTTON_TALK         (1<<1)          // displays talk balloon and disables actions
#define BUTTON_GOGGLES      (1<<2)          // turns nv or therm goggles on/off
#define BUTTON_LEAN         (1<<3)          // lean modifier, when held strafe left and right will lean
#define BUTTON_WALKING      (1<<4)          // walking can't just be infered from MOVE_RUN
// because a key pressed late in the frame will
// only generate a small move value for that frame
// walking will use different animations and
// won't generate footsteps
#define BUTTON_USE          (1<<5)          // the ol' use key returns!
#define BUTTON_RELOAD       (1<<6)          // reloads current weapon
#define BUTTON_ALT_ATTACK   (1<<7)
#define BUTTON_ANY          (1<<8)          // any key whatsoever
#define BUTTON_ZOOMIN       (1<<9)
#define BUTTON_ZOOMOUT      (1<<10)
#define BUTTON_FIREMODE     (1<<11)

#define BUTTON_LEAN_RIGHT   (1<<12)
#define BUTTON_LEAN_LEFT    (1<<13)

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
    int             serverTime;
    int             angles[3];
    int             buttons;
    byte            weapon;           // weapon
    signed char forwardmove, rightmove, upmove;
} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define SOLID_BMODEL    0xffffff

typedef enum {
    TR_STATIONARY,
    TR_INTERPOLATE,             // non-parametric, but interpolate between snapshots
    TR_LINEAR,
    TR_LINEAR_STOP,
    TR_SINE,                    // value = base + sin( time / duration ) * delta
    TR_GRAVITY,
    TR_HEAVYGRAVITY,
    TR_LIGHTGRAVITY
} trType_t;

typedef struct {
    trType_t    trType;
    int     trTime;
    int     trDuration;         // if non 0, trTime + trDuration = stop time
    vec3_t  trBase;
    vec3_t  trDelta;            // velocity, etc
} trajectory_t;

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s
{
    int             number;         // entity index
    int             eType;          // entityType_t
    int             eFlags;

    trajectory_t    pos;            // for calculating position
    trajectory_t    apos;           // for calculating angles

    int             time;
    int             time2;

    vec3_t          origin;
    vec3_t          origin2;

    vec3_t          angles;
    vec3_t          angles2;

    int             otherEntityNum; // shotgun sources, etc
    int             otherEntityNum2;

    int             groundEntityNum;    // -1 = in air

    int             loopSound;      // constantly loop this sound
    int             mSoundSet;

    int             modelindex;
    int             modelindex2;
    int             clientNum;      // 0 to (MAX_CLIENTS - 1), for players and corpses
    int             frame;

    int             solid;          // for client side prediction, trap_linkentity sets this properly

    int             event;          // impulse events -- muzzle flashes, footsteps, etc
    int             eventParm;

    int             generic1;

    // for players
    // these fields are only transmitted for client entities!!!!!
    int             gametypeitems;  // bit flags indicating which items are carried
    int             weapon;         // determines weapon and flash model, etc
    int             legsAnim;       // mask off ANIM_TOGGLEBIT
    int             torsoAnim;      // mask off ANIM_TOGGLEBIT
    int             torsoTimer;     // time the animation will play for
    int             leanOffset;     // Lean direction
} entityState_t;

#define Square(x) ((x)*(x))

// real time
//=============================================


typedef struct qtime_s {
    int tm_sec;     /* seconds after the minute - [0,59] */
    int tm_min;     /* minutes after the hour - [0,59] */
    int tm_hour;    /* hours since midnight - [0,23] */
    int tm_mday;    /* day of the month - [1,31] */
    int tm_mon;     /* months since January - [0,11] */
    int tm_year;    /* years since 1900 */
    int tm_wday;    /* days since Sunday - [0,6] */
    int tm_yday;    /* days since January 1 - [0,365] */
    int tm_isdst;   /* daylight savings time flag */
} qtime_t;

#define SAY_ALL     0
#define SAY_TEAM    1
#define SAY_TELL    2

/*
========================================================================

Ghoul2

========================================================================
*/

/*
Ghoul2 Insert Start
*/

typedef struct {
    float       matrix[3][4];
} mdxaBone_t;

// For ghoul2 axis use

typedef enum
{
    ORIGIN = 0,
    POSITIVE_X,
    POSITIVE_Z,
    POSITIVE_Y,
    NEGATIVE_X,
    NEGATIVE_Z,
    NEGATIVE_Y
} Eorientations;
/*
Ghoul2 Insert End
*/

// define the new memory tags for the zone, used by all modules now
//
#define TAGDEF(blah) TAG_ ## blah
typedef enum {
#include "../qcommon/tags.h"
} memtag_t;

typedef struct
{
    int     isValid;
    void    *ghoul2;
    int     modelNum;
    int     boltNum;
    vec3_t  angles;
    vec3_t  origin;
    vec3_t  scale;
    vec3_t  dir;
    vec3_t  forward;
} CFxBoltInterface;

/*
========================================================================

String ID Tables

========================================================================
*/
#define ENUM2STRING(arg)   #arg,arg
typedef struct stringID_table_s
{
    char    *name;
    int     id;
} stringID_table_t;

int GetIDForString(stringID_table_t *table, const char *string);
const char *GetStringForID(stringID_table_t *table, int id);

#endif  // __Q_SHARED_H
