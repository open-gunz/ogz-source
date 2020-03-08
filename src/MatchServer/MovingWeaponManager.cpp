#include "stdafx.h"
#include "MovingWeaponManager.h"
#include "HitRegistration.h"
#include "MMatchStage.h"
#include "has_xxx.h"
#include "MMatchWorldItemDesc.h"
#include "MPickInfo.h"

HAS_XXX(Update);

template <typename T>
static std::enable_if_t<has_Update<std::remove_reference_t<T>>::value, bool> TryUpdate(T& Obj,
	MovingWeaponManager& Mgr, float Elapsed)
{ return Obj.Update(Mgr, Elapsed); }

template <typename T>
static std::enable_if_t<!has_Update<std::remove_reference_t<T>>::value, bool> TryUpdate(T& Obj,
	MovingWeaponManager& Mgr, float Elapsed)
{ return true; }

void MovingWeaponManager::Update(float Elapsed)
{
	using namespace RealSpace2;
	Weapons.apply([&](auto& Obj)
	{
		if (!TryUpdate(Obj, *this, Elapsed))
			return false;

		auto diff = Obj.Vel * Elapsed;
		auto dir = diff;
		Normalize(dir);
		auto dist = Magnitude(diff);

		auto PickFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;
		auto Time = MGetMatchServer()->GetGlobalClockCount() / 1000.0;

		v3 pickpos;
		MPICKINFO pi;
		bool bPicked = PickHistory(nullptr, Obj.Pos, Obj.Pos + diff, Stage->BspObject, pi,
			Stage->GetObjectList(), Time, PickFlag);
		if (bPicked)
		{
			if (pi.bBspPicked)
			{
				pickpos = pi.bpi.PickPos;
			}
			else if (pi.pObject)
			{
#ifdef REFLECT_ROCKETS
				if (zpi.pObject->IsGuard() && DotProduct(zpi.pObject->GetDirection(), m_Dir) < 0)
				{
					auto ReflectedDir = RealSpace2::GetReflectionVector(m_Dir, zpi.pObject->GetDirection());
					auto ReflectedVel = RealSpace2::GetReflectionVector(m_Velocity, zpi.pObject->GetDirection());

					m_Dir = ReflectedDir;
					m_Velocity = ReflectedVel;

					diff = m_Velocity * fElapsedTime;
					dir = diff;
					Normalize(dir);

					bPicked = false;
				}
#endif

				pickpos = pi.info.vOut;
			}
		}

		v3 Normal;
		if (pi.bBspPicked)
		{
			rplane& plane = pi.bpi.pNode->pInfo[pi.bpi.nIndex].plane;
			Normal = rvector(plane.a, plane.b, plane.c);
		}
		else if (pi.pObject)
		{
			v3 Origin;
			pi.pObject->GetPositions(nullptr, &Origin, Time);
			if (Origin.z + 30.f <= pickpos.z && pickpos.z <= Origin.z + 160.f)
			{
				Normal = pickpos - Origin;
				Normal.z = 0;
			}
			else
			{
				Normal = pickpos - (Origin + v3(0, 0, 90));
			}
			Normalize(Normal);
		}

		if (bPicked && Magnitude(pickpos - Obj.Pos) < dist && !Obj.OnCollision(*this, pickpos, Normal, pi))
			return false;
		else
			Obj.Pos += diff;

		return true;
	});
}

void MovingWeaponManager::AddRocket(MMatchObject* Owner, MMatchItemDesc* ItemDesc,
	const v3 & Pos, const v3 & Dir)
{
	Weapons.emplace<Rocket>(Pos, Dir, Dir * 2700, ItemDesc, Owner);
}

void MovingWeaponManager::AddItemKit(MMatchObject * Owner, MMatchItemDesc * ItemDesc,
	const v3 & Pos, const v3 & Dir)
{
	Weapons.emplace<ItemKit>(Pos, Dir, v3{ 0, 0, 0 }, ItemDesc, Owner);
}

void MovingWeaponManager::AddGrenade(MMatchObject * Owner, MMatchItemDesc * ItemDesc,
	const v3 & Pos, const v3 & Dir, const v3& Vel)
{
	MGetMatchServer()->Log(MMatchServer::LOG_ALL, "Creating grenade!");
	Weapons.emplace<Grenade>(Pos, Dir, Vel, ItemDesc, Owner);
}

static void Explosion(MovingWeaponManager& Mgr, const v3& Pos, MMatchObject* Owner,
	MMatchItemDesc* ItemDesc, float Range, float MinimumDamage, float Knockback)
{
	if (Mgr.Stage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	auto GetOrigin = [&](const auto& Obj, auto& Origin)
	{
		u64 RealTime = MGetMatchServer()->GetGlobalClockCount();
		float CompensatedTime = 0;
		if (&Obj == Owner)
			CompensatedTime = RealTime / 1000.f;
		else
			CompensatedTime = (RealTime - Owner->GetPing()) / 1000.f;

		Obj.GetPositions(nullptr, &Origin, CompensatedTime);
	};

	GrenadeExplosion(*Owner, Mgr.Stage->GetObjectList(), Pos, ItemDesc->m_nDamage,
		Range, MinimumDamage, Knockback,
		GetOrigin);
}

bool Rocket::OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const v3& Normal, const MPICKINFO& pi)
{
	auto Range = 350.0f;
	auto MinimumDamage = 0.3f;
	auto Knockback = 0.5f;
	Explosion(Mgr, ColPos, Owner, ItemDesc, Range, MinimumDamage, Knockback);
	return false;
}

bool Grenade::OnCollision(MovingWeaponManager & Mgr, const v3 & ColPos, const v3& Normal, const MPICKINFO& pi)
{
	Pos = ColPos + Normal;
	Vel = RealSpace2::GetReflectionVector(Vel, Normal);
	Vel *= pi.pObject ? 0.4f : 0.8f;
	return true;
}

bool Grenade::Update(MovingWeaponManager& Mgr, float Elapsed)
{
	// Gravity!
	Vel.z += -1000.0f * Elapsed;

	Lifetime += Elapsed;
	if (Lifetime > 2.0f)
	{
		auto Range = 400.0f;
		auto MinimumDamage = 0.2f;
		auto Knockback = 1.0f;
		Explosion(Mgr, Pos, Owner, ItemDesc, Range, MinimumDamage, Knockback);
		return false;
	}

	return true;
}

bool ItemKit::OnCollision(MovingWeaponManager & Mgr, const v3 & ColPos, const v3 & Normal, const MPICKINFO & pi)
{
	Pos = ColPos + Normal;
	Vel = RealSpace2::GetReflectionVector(Vel, Normal);
	Vel *= pi.pObject ? 0.4f : 0.8f;
	return true;
}

static int GetWorldItemID(MMatchItemDesc* ItemDesc)
{
	// TODO: Change this god awful algorithm
	for (auto* WorldItemDesc : MakePairValueAdapter(*MGetMatchWorldItemDescMgr()))
	{
		if (!strcmp(ItemDesc->m_szMeshName, WorldItemDesc->m_szModelName))
		{
			return WorldItemDesc->m_nID;
		}
	}

	MLog("Can't find world item for item ID %d\n", ItemDesc->m_nID);
	return 0;
}

bool ItemKit::Update(MovingWeaponManager & Mgr, float Elapsed)
{
	Vel.z += -1000.0f * Elapsed;

	RealSpace2::RBSPPICKINFO rpi;
	auto PickFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;
	bool bPicked = Mgr.Stage->BspObject->Pick(Pos, rvector(0, 0, -1), &rpi, PickFlag);

	if (bPicked && fabsf(RealSpace2::Magnitude(rpi.PickPos - Pos)) > 5.0f)
		return true;

	Mgr.Stage->m_WorldItemManager.SpawnDynamicItem(GetWorldItemID(ItemDesc), Pos.x, Pos.y, Pos.z);

	return false;
}