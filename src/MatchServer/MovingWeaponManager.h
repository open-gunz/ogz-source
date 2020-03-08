#pragma once

#include "GlobalTypes.h"
#include <vector>
#include "MMatchItem.h"
#include "MultiVector.h"

class MovingWeaponManager;
struct MPICKINFO;

struct MovingWeapon
{
	v3 Pos;
	v3 Dir;
	v3 Vel;
	MMatchItemDesc* ItemDesc;
	MMatchObject* Owner;
	MovingWeapon(const v3& Pos, const v3& Dir, const v3& Vel, MMatchItemDesc* ItemDesc, MMatchObject* Owner)
		: Pos(Pos), Dir(Dir), Vel(Vel), ItemDesc(ItemDesc), Owner(Owner)
	{ }
};

struct ItemKit : MovingWeapon
{
	using MovingWeapon::MovingWeapon;

	bool OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const v3& Normal, const MPICKINFO&);
	bool Update(MovingWeaponManager& Mgr, float Elapsed);
};

struct Rocket : MovingWeapon
{
	using MovingWeapon::MovingWeapon;

	bool OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const v3& Normal, const MPICKINFO&);
};

struct Grenade : MovingWeapon
{
	using MovingWeapon::MovingWeapon;

	float Lifetime = 0.0f;

	bool OnCollision(MovingWeaponManager& Mgr, const v3& ColPos, const v3& Normal, const MPICKINFO&);
	bool Update(MovingWeaponManager& Mgr, float Elapsed);
};

class MovingWeaponManager
{
public:
	MovingWeaponManager(MMatchStage& Stage)
		: Stage(&Stage)
	{ }
	void Update(float Elapsed);
	void AddRocket(MMatchObject* Owner, MMatchItemDesc* ItemDesc, const v3& Pos, const v3& Dir);
	void AddItemKit(MMatchObject* Owner, MMatchItemDesc* ItemDesc, const v3& Pos, const v3& Dir);
	void AddGrenade(MMatchObject* Owner, MMatchItemDesc* ItemDesc,
		const v3& Pos, const v3& Dir, const v3& Vel);

	class MMatchStage* Stage;

private:
	MultiVector<ItemKit, Rocket, Grenade> Weapons;
};