#pragma once

#include "RTypes.h"

#define RTOK_NAME			"name"

#define RTOK_MATERIALLIST	"MATERIALLIST"
#define RTOK_MATERIAL		"MATERIAL"
#define RTOK_AMBIENT		"AMBIENT"
#define RTOK_DIFFUSE		"DIFFUSE"
#define RTOK_SPECULAR		"SPECULAR"
#define RTOK_POWER			"POWER"
#define RTOK_DIFFUSEMAP		"DIFFUSEMAP"
#define RTOK_ADDITIVE		"ADDITIVE"
#define RTOK_TWOSIDED		"TWOSIDED"
#define RTOK_USEOPACITY		"USEOPACITY"
#define RTOK_USEALPHATEST	"USEALPHATEST"

#define RTOK_LIGHTLIST		"LIGHTLIST"
#define RTOK_LIGHT			"LIGHT"
#define RTOK_POSITION		"POSITION"
#define RTOK_DIRECTION		"DIRECTION"
#define RTOK_COLOR			"COLOR"
#define RTOK_INTENSITY		"INTENSITY"
#define RTOK_ATTNSTART		"ATTENUATIONSTART"
#define RTOK_ATTNEND		"ATTENUATIONEND"
#define RTOK_CASTSHADOW		"CASTSHADOW"
#define RTOK_RECEIVESHADOW	"RECEIVESHADOW"

#define RTOK_OBJECTLIST		"OBJECTLIST"
#define RTOK_OBJECT			"OBJECT"

#define RTOK_SPAWNPOSITIONLIST	"SPAWNPOSITIONLIST"
#define RTOK_SPAWNPOSITION		"SPAWNPOSITION"

#define RTOK_OCCLUSIONLIST	"OCCLUSIONLIST"
#define RTOK_OCCLUSION		"OCCLUSION"

#define RTOK_DUMMYLIST		"DUMMYLIST"
#define RTOK_DUMMY			"DUMMY"

#define RTOK_FLAG					"FLAG"
#define RTOK_FLAG_NAME				"NAME"
#define RTOK_FLAG_DIRECTION			"DIRECTION"		
#define RTOK_FLAG_POWER				"POWER"
#define RTOK_RESTRICTION			"RESTRICTION"
#define RTOK_RESTRICTION_AXIS		"AXIS"
#define RTOK_RESTRICTION_POSITION	"POSITION"
#define RTOK_RESTRICTION_COMPARE	"COMPARE"
#define RTOK_WINDTYPE				"TYPE"
#define RTOK_WINDDELAY				"DELAY"

#define RTOK_SMOKE					"SMOKE"
#define RTOK_SMOKE_NAME				"NAME"
#define RTOK_SMOKE_DIRECTION		"DIRECTION"
#define RTOK_SMOKE_POWER			"POWER"
#define RTOK_SMOKE_DELAY			"DELAY"
#define RTOK_SMOKE_SIZE				"SIZE"
#define RTOK_SMOKE_COLOR			"COLOR"
#define RTOK_SMOKE_LIFE				"LIFE"
#define RTOK_SMOKE_TOGMINTIME		"TOGGLEMINTIME"

#define RTOK_FOG		"FOG"
#define RTOK_GLOBAL		"GLOBAL"

#define FORMAT_FLOAT	"%.7f"

char *Format(char *buffer, int maxlen, const rvector &v);
char *Format(char *buffer, int maxlen, float f);
char *Format(char *buffer, int maxlen, u32 dw);
template<size_t size> char *Format(char(&buffer)[size], const rvector &v) {
	return Format(buffer, size, v);
}
template<size_t size> char *Format(char(&buffer)[size], float f) {
	return Format(buffer, size, f);
}
template<size_t size> char *Format(char(&buffer)[size], u32 dw) {
	return Format(buffer, size, dw);
}

#define RTOK_MAX_OBJLIGHT		"obj_"
#define RTOK_MAX_NOPATH			"nopath_"
#define RTOK_MAX_EXPORTOBJ		"obj_"
#define RTOK_MAX_SPAWNPOINT		"spawn_"
#define RTOK_MAX_PASSTHROUGH	"pass_"
#define RTOK_MAX_HIDE			"hide_"
#define RTOK_MAX_OCCLUSION		"wall_"
#define RTOK_MAX_PASSBULLET		"passb_"
#define RTOK_MAX_PASSROCKET		"passr_"
#define RTOK_MAX_OCCLUSION		"wall_"
#define RTOK_MAX_PARTITION		"partition_"
#define RTOK_MAX_NAVIGATION		"hnav_"
#define RTOK_MAX_SMOKE_SS		"smk_ss_"
