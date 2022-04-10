#include "stdafx.h"
#include "BasicInfo.h"
#include "MMatchUtil.h"

bool UnpackNewBasicInfo(NewBasicInfo& nbi, const u8* pbi, size_t BlobSize)
{
	auto& Flags = nbi.Flags;
	nbi.Flags = *pbi;
	auto Size = 1;
	// Time
	Size += sizeof(float);
	// Position
	if (Flags & BasicInfoFlags::LongPos)
		Size += sizeof(v3);
	else
		Size += sizeof(MShortVector);
	// Direction
	Size += sizeof(PackedDirection);
	// Velocity
	Size += sizeof(MShortVector);
	if (Flags & BasicInfoFlags::CameraDir)
		Size += sizeof(PackedDirection);
	if (Flags & BasicInfoFlags::Animations)
		Size += sizeof(u8) * 2;
	if (Flags & BasicInfoFlags::SelItem)
		Size += sizeof(u8);

	if (Size != BlobSize)
		return false;

	size_t Offset = 1;
#define READ(type) [&](){ auto ret = *(type*)(pbi + Offset); Offset += sizeof(type); return ret; }()

	nbi.Time = READ(float);

	auto& bi = nbi.bi;

	if (Flags & BasicInfoFlags::LongPos)
		bi.position = READ(v3);
	else
		bi.position = static_cast<v3>(READ(MShortVector));

	bi.direction = UnpackDirection(READ(PackedDirection));
	bi.velocity = static_cast<v3>(READ(MShortVector));
	bool HasCameraDir = (Flags & static_cast<int>(BasicInfoFlags::CameraDir)) != 0;
	if (HasCameraDir)
		bi.cameradir = UnpackDirection(READ(PackedDirection));
	else
		bi.cameradir = bi.direction;
	if (Flags & BasicInfoFlags::Animations)
	{
		bi.lowerstate = static_cast<ZC_STATE_LOWER>(READ(u8));
		bi.upperstate = static_cast<ZC_STATE_UPPER>(READ(u8));

		// Reject invalid animations.
		if (bi.lowerstate < ZC_STATE_LOWER_NONE || bi.lowerstate >= ZC_STATE_LOWER_END ||
			bi.upperstate < ZC_STATE_UPPER_NONE || bi.upperstate >= ZC_STATE_UPPER_END)
			return false;
	}
	else
	{
		bi.lowerstate = static_cast<ZC_STATE_LOWER>(-1);
		bi.upperstate = static_cast<ZC_STATE_UPPER>(-1);
	}
	if (Flags & BasicInfoFlags::SelItem)
	{
		bi.SelectedSlot = static_cast<MMatchCharItemParts>(READ(u8));

		// Reject invalid slots.
		if (bi.SelectedSlot < MMCIP_MELEE || bi.SelectedSlot > MMCIP_CUSTOM2)
			return false;
	}
	else
	{
		bi.SelectedSlot = static_cast<MMatchCharItemParts>(-1);
	}

	// Reject basicinfos that have NaN values
	for (auto&& vec : { bi.position, bi.direction, bi.velocity })
		if (isnan(vec.x) || isnan(vec.y) || isnan(vec.z))
			return false;

	return true;
}

MCommandParameterBlob* PackNewBasicInfo(const CharacterInfo& Input, BasicInfoNetState& State, float Time)
{
	char buf[64];
	size_t Size = 1;
	auto Write = [&](auto&& Val)
	{
		memcpy(buf + Size, &Val, sizeof(Val));
		Size += sizeof(Val);
	};

	u8 Flags{};

	Write(Time);

	auto&& Pos = Input.Pos;
	for (auto&& Val : { Pos.x, Pos.y, Pos.z })
	{
		if (Val > SHRT_MAX || Val < SHRT_MIN)
		{
			Flags |= BasicInfoFlags::LongPos;
			break;
		}
	}

	if (Flags & BasicInfoFlags::LongPos)
		Write(Pos);
	else
		Write(MShortVector(Pos));

	auto Dir = PackDirection(Input.Dir);
	Write(Dir);

	auto Velocity = MShortVector{ Input.Vel };
	Write(Velocity);

	if (Input.HasCamDir)
	{
		Flags |= BasicInfoFlags::CameraDir;
		auto CamDir = PackDirection(Input.CamDir);
		Write(CamDir);
	}

	auto Lower = static_cast<u8>(Input.LowerAni);
	auto Upper = static_cast<u8>(Input.UpperAni);
	if (State.LastNetLowerAni != Lower ||
		State.LastNetUpperAni != Upper)
	{
		Flags |= BasicInfoFlags::Animations;
		Write(Lower);
		Write(Upper);
		State.LastNetLowerAni = static_cast<ZC_STATE_LOWER>(Lower);
		State.LastNetUpperAni = static_cast<ZC_STATE_UPPER>(Upper);
	}

	auto Slot = static_cast<u8>(Input.Slot);
	if (State.LastNetSlot != Slot)
	{
		Flags |= BasicInfoFlags::SelItem;
		Write(Slot);
		State.LastNetSlot = Slot;
	}

	buf[0] = Flags;

	return new MCommandParameterBlob{ buf, static_cast<int>(Size) };
}