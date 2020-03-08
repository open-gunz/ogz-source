#pragma once

#include "AnimationStuff.h"
#include "RMath.h"
#include "MMatchItem.h"

struct BasicInfo {
	v3 position;
	v3 velocity;
	v3 direction;
	v3 cameradir;
	ZC_STATE_UPPER upperstate;
	ZC_STATE_LOWER lowerstate;
	MMatchCharItemParts SelectedSlot;

	void Validate()
	{
		if (SelectedSlot < MMCIP_MELEE || SelectedSlot > MMCIP_CUSTOM2)
			SelectedSlot = MMCIP_PRIMARY;
	}
};

struct ZBasicInfo {
	v3 position;
	v3 velocity;
	v3 direction;
	v3 targetdir;
};

#pragma pack(push, 1)

struct ZPACKEDBASICINFO {
	float	fTime;
	short	posx, posy, posz;
	short	velx, vely, velz;
	short	dirx, diry, dirz;
	u8	upperstate;
	u8	lowerstate;
	u8	selweapon;

	void Unpack(ZBasicInfo& bi)
	{
		bi.position = v3(posx, posy, posz);
		bi.velocity = v3(velx, vely, velz);
		bi.direction = 1.f / 32000.f * v3(dirx, diry, dirz);
	}

	void Pack(const ZBasicInfo& bi)
	{
		posx = short(bi.position.x);
		posy = short(bi.position.y);
		posz = short(bi.position.z);

		velx = short(bi.velocity.x);
		vely = short(bi.velocity.y);
		velz = short(bi.velocity.z);

		dirx = short(bi.direction.x * 32000);
		diry = short(bi.direction.y * 32000);
		dirz = short(bi.direction.z * 32000);
	}

	void Unpack(BasicInfo& bi)
	{
		bi.position = v3(posx, posy, posz);
		bi.velocity = v3(velx, vely, velz);
		bi.direction = 1.f / 32000.f * v3(dirx, diry, dirz);
		bi.upperstate = ZC_STATE_UPPER(upperstate);
		bi.lowerstate = ZC_STATE_LOWER(lowerstate);
		bi.SelectedSlot = MMatchCharItemParts(selweapon);
		bi.Validate();
	}

	void Pack(const BasicInfo& bi)
	{
		posx = short(bi.position.x);
		posy = short(bi.position.y);
		posz = short(bi.position.z);

		velx = short(bi.velocity.x);
		vely = short(bi.velocity.y);
		velz = short(bi.velocity.z);

		dirx = short(bi.direction.x * 32000);
		diry = short(bi.direction.y * 32000);
		dirz = short(bi.direction.z * 32000);

		upperstate = bi.upperstate;
		lowerstate = bi.lowerstate;
	}
};

#pragma pack(pop)

struct BasicInfoItem : BasicInfo
{
	double SentTime;
	double RecvTime;
	float LowerFrameTime;
	float UpperFrameTime;

	BasicInfoItem() = default;
	BasicInfoItem(const BasicInfo& a, float b, float c)
		: BasicInfo(a), SentTime(b), RecvTime(c)
	{ }
};

struct NewBasicInfo
{
	u8 Flags;
	float Time;
	BasicInfoItem bi;
};

namespace BasicInfoFlags
{
enum Type : u8
{
	LongPos = 1 << 0,
	CameraDir = 1 << 1,
	Animations = 1 << 2,
	SelItem = 1 << 3,
};
}

bool UnpackNewBasicInfo(NewBasicInfo& nbi, const u8* pbi, size_t BlobSize);

struct CharacterInfo
{
	v3 Pos, Dir, Vel, CamDir;
	ZC_STATE_LOWER LowerAni;
	ZC_STATE_UPPER UpperAni;
	MMatchCharItemParts Slot;
	bool HasCamDir;
};

// Since the new RG basic info format only sends certain pieces of information when they change,
// this stores some of the data from the last transmission to track whether it has changed.
struct BasicInfoNetState
{
	BasicInfoNetState()
	{
		QueueSync();
	}

	ZC_STATE_LOWER LastNetLowerAni;
	ZC_STATE_UPPER LastNetUpperAni;
	int LastNetSlot;

	// Sets all the cached data to invalid values such that the next basic info packing will
	// necessarily explicitly contain all the current data.
	void QueueSync()
	{
		LastNetLowerAni = ZC_STATE_LOWER(-1);
		LastNetUpperAni = ZC_STATE_UPPER(-1);
		LastNetSlot = -1;
	}
};

class MCommandParameterBlob* PackNewBasicInfo(const CharacterInfo& Input,
	BasicInfoNetState& State, float Time);