#include "stdafx.h"
#include "ZGame.h"
#include "ZWeapon.h"
#include "RBspObject.h"
#include "ZEffectBillboard.h"
#include "ZEffectSmoke.h"
#include "ZSoundEngine.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZConfiguration.h"
#include "RealSoundEffect.h"
#include "ZEffectFlashBang.h"
#include "ZPost.h"
#include "ZStencilLight.h"
#include "ZScreenDebugger.h"
#include "Config.h"
#include "Portal.h"
#include "RBspObject.h"
#include "ZPickInfo.h"

#define BOUND_EPSILON	5
#define LANDING_VELOCITY	20
#define MAX_ROT_VELOCITY	50


#define ROCKET_VELOCITY			2700.f
#define ROCKET_SPLASH_RANGE		350.f
#define ROCKET_MINIMUM_DAMAGE	.3f
#define ROCKET_KNOCKBACK_CONST	.5f

MImplementRootRTTI(ZWeapon);

ZWeapon::ZWeapon() : m_pVMesh(NULL), m_nWorldItemID(0), m_WeaponType(ZWeaponType_None), m_SLSid(0) {
	m_nItemUID = -1;
}

ZWeapon::~ZWeapon() {
	if(m_pVMesh){
		delete m_pVMesh;
		m_pVMesh = NULL;
	}

	if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
		ZGetStencilLight()->DeleteLightSource( m_SLSid );
		m_SLSid = 0;
	}
}

void ZWeapon::Create(RMesh* pMesh) {
	m_pVMesh = new RVisualMesh;
	m_pVMesh->Create(pMesh);
}

void ZWeapon::Render() {
	m_pVMesh->Frame();
	m_pVMesh->Render();
}

bool ZWeapon::Update(float fElapsedTime)
{
	return true;
}

MImplementRTTI(ZMovingWeapon, ZWeapon);

ZMovingWeapon::ZMovingWeapon() : ZWeapon() {
	m_WeaponType = ZWeaponType_MovingWeapon;
	m_PostPos = rvector(-1,-1,-1);
}

ZMovingWeapon::~ZMovingWeapon() {
}

void ZMovingWeapon::Explosion() {
}

MImplementRTTI(ZWeaponRocket,ZMovingWeapon);

ZWeaponRocket::ZWeaponRocket() :ZMovingWeapon() {

}

ZWeaponRocket::~ZWeaponRocket() {

}

void ZWeaponRocket::Create(RMesh* pMesh, const rvector &pos, const rvector &dir,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	m_Velocity=dir*ROCKET_VELOCITY;

	m_fStartTime = g_pGame->GetTime();
	m_fLastAddTime = g_pGame->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage;
	
	if( Z_VIDEO_DYNAMICLIGHT ) {
		_ASSERT(m_SLSid==0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}
}

#define ROCKET_LIFE			10.f

bool ZWeaponRocket::Update(float fElapsedTime)
{
	rvector diff = m_Velocity*fElapsedTime;

#ifdef PORTAL
	g_pPortal->Move(*this, diff);
#endif

	rvector oldPos = m_Position;

	if(g_pGame->GetTime() - m_fStartTime > ROCKET_LIFE ) {
		Explosion();

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	constexpr u32 dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	rvector dir = diff;
	Normalize(dir);

	float fDist = Magnitude(diff);

	rvector pickpos;
	ZPICKINFO zpi;
	bool bPicked = ZGetGame()->Pick(nullptr, m_Position, dir, &zpi, dwPickPassFlag);
	if (bPicked)
	{
		if (zpi.bBspPicked)
		{
			pickpos = zpi.bpi.PickPos;
		}
		else
		{
			if (zpi.pObject)
			{
#ifdef REFLECT_ROCKETS
				if (zpi.pObject->IsGuard() && DotProduct(zpi.pObject->GetDirection(), m_Dir) < 0)
				{
					auto ReflectedDir = GetReflectionVector(m_Dir, zpi.pObject->GetDirection());
					auto ReflectedVel = GetReflectionVector(m_Velocity, zpi.pObject->GetDirection());

					m_Dir = ReflectedDir;
					m_Velocity = ReflectedVel;

					diff = m_Velocity * fElapsedTime;
					dir = diff;
					Normalize(dir);

					bPicked = false;
				}
#endif

				pickpos = zpi.info.vOut;
			}
		}

	}

	if (bPicked && Magnitude(pickpos - m_Position) < fDist)
	{
		Explosion();

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
		{
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
			ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
		}

		return false;
	}else
		m_Position+=diff;

	rmatrix mat;
	dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	float this_time = g_pGame->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

		ZGetEffectManager()->AddRocketSmokeEffect(pos);
		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponRocket::Render()
{
	ZWeapon::Render();}

void ZWeaponRocket::Explosion()
{
	rvector v = m_Position;

	rvector dir = -RealSpace2::RCameraDirection;
	ZGetEffectManager()->AddRocketEffect(v,dir);

	g_pGame->OnExplosionGrenade(m_uidOwner,
		v, m_fDamage,
		ROCKET_SPLASH_RANGE, ROCKET_MINIMUM_DAMAGE,
		ROCKET_KNOCKBACK_CONST, m_nTeamID);

	ZGetSoundEngine()->PlaySound("fx_explosion01",v);

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );

}

MImplementRTTI(ZWeaponItemkit,ZMovingWeapon);

ZWeaponItemkit::ZWeaponItemkit() :ZMovingWeapon()
{
	m_fDeathTime = 0;
	m_nWorldItemID = 0;

	m_bInit = false;
	m_bDeath = false;
	m_bSendMsg = false;
}

ZWeaponItemkit::~ZWeaponItemkit() 
{

}

void ZWeaponItemkit::Create(RMesh* pMesh, const rvector &pos, const rvector &velocity,ZObject* pOwner)
{
	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=g_pGame->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	m_bInit = false;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage;

	m_bSendMsg = false;
}

void ZWeaponItemkit::Render()
{
	if(m_bInit) {
		if(m_pVMesh->m_pMesh) {

		rmatrix mat;
		MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);
		m_pVMesh->SetWorldMatrix(mat);
		ZMovingWeapon::Render();

		}
	}
}

void ZWeaponItemkit::UpdateFirstPos()
{
	m_bInit = true;
	return;

	if(m_bInit==false) {

		ZCharacter* pC = ZGetCharacterManager()->Find(m_uidOwner);

		if(pC) {
			if(pC->m_pVMesh) {

				rvector vWeapon[1];

				vWeapon[0] = pC->m_pVMesh->GetCurrentWeaponPosition();

				rvector nPos = pC->m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
				rvector nDir = vWeapon[0] - nPos;

				Normalize(nDir);

				RBSPPICKINFO bpi;
				if(ZGetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
				{
					if (DotProduct(bpi.pInfo->plane, vWeapon[0]) < 0) {
						vWeapon[0] = bpi.PickPos - nDir;
					}
				}

				m_Position = vWeapon[0];
			}
		}

		m_bInit = true;
	}
}

void ZWeaponItemkit::UpdatePost(DWORD dwPickPassFlag)
{
	RBSPPICKINFO rpi;
	bool bPicked = ZGetWorld()->GetBsp()->Pick(m_Position,rvector(0,0,-1),&rpi, dwPickPassFlag );

	if(bPicked && fabsf(Magnitude(rpi.PickPos - m_Position)) < 5.0f ) {

		if(m_bSendMsg==false) {

			if(m_uidOwner == ZGetGameClient()->GetPlayerUID()) {
				if (ZGetGameClient()->GetMatchStageSetting()->GetNetcode() != NetcodeType::ServerBased)
					ZPostRequestSpawnWorldItem(ZGetGameClient()->GetPlayerUID(), m_nWorldItemID, m_Position);
				m_PostPos = m_Position;
			}

			m_bSendMsg = true;
			m_fDeathTime = g_pGame->GetTime() + 2.0f;
		}
	}
}

void ZWeaponItemkit::UpdatePos(float fElapsedTime,DWORD dwPickPassFlag)
{
	rvector diff = m_Velocity * fElapsedTime;

	rvector dir = diff;
	Normalize(dir);

	float fDist = Magnitude(diff);

	rvector pickpos;
	rvector normal=rvector(0,0,1);

	ZPICKINFO zpi;

	bool bPicked = g_pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);

	if (bPicked) {
		if(zpi.bBspPicked)	{
			pickpos=zpi.bpi.PickPos;
			rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
			normal=rvector(plane.a,plane.b,plane.c);
		}
		else if(zpi.pObject) {
			pickpos=zpi.info.vOut;
			if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
			{
				normal=pickpos-zpi.pObject->GetPosition();
				normal.z=0;
			}
			else
				normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
			Normalize(normal);
		}
	}

	if(bPicked && fabsf(Magnitude(pickpos-m_Position)) < fDist)
	{
		m_Position = pickpos + normal;
		m_Velocity = GetReflectionVector(m_Velocity,normal);
		m_Velocity *= zpi.pObject ? 0.1f: 0.2f;
		m_Velocity *= 0.2f;

		Normalize(normal);
		float fAbsorb=DotProduct(normal,m_Velocity);
		m_Velocity -= 0.1*fAbsorb*normal;

		float fA = RANDOMFLOAT * TAU;
		float fB=RANDOMFLOAT * TAU;
		m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

	} else {
		m_Position+=diff;
	}
}

bool ZWeaponItemkit::Update(float fElapsedTime)
{
	if(m_bDeath) {
		return false;
	}

	if(m_bSendMsg) {
		return true;
	}

	if(g_pGame->GetTime() - m_fStartTime < m_fDelayTime) {
		return true;
	}

	UpdateFirstPos();

	m_Velocity.z -= 500.f * fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	UpdatePos( fElapsedTime , dwPickPassFlag );

	UpdatePost( dwPickPassFlag );

	rmatrix mat;
	rvector dir = m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	return true;
}

void ZWeaponItemkit::Explosion()
{
}

MImplementRTTI(ZWeaponGrenade,ZMovingWeapon);

void ZWeaponGrenade::Create(RMesh* pMesh, const rvector &pos, const rvector &velocity,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=g_pGame->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage;

	m_nSoundCount = rand() % 2 + 2;
}

#define GRENADE_LIFE			2.f

bool ZWeaponGrenade::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;
	if(g_pGame->GetTime() - m_fStartTime > GRENADE_LIFE) {
		Explosion();
		if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );
		return false;
	}

	m_Velocity.z-=1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos,normal;

		ZPICKINFO zpi;
		bool bPicked=g_pGame->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos=zpi.bpi.PickPos;
				rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal=rvector(plane.a,plane.b,plane.c);
			}
			else
			if(zpi.pObject)
			{
				pickpos=zpi.info.vOut;
				if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
				{
					normal=pickpos-zpi.pObject->GetPosition();
					normal.z=0;
				}else
					normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
				Normalize(normal);
			}
		}

		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;

			if(zpi.bBspPicked && m_nSoundCount>0) {
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA = RANDOMFLOAT*TAU;
			float fB = RANDOMFLOAT*TAU;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

		}else
			m_Position+=diff;
	}

	float fRotSpeed=Magnitude(m_Velocity)*0.04f;

	rmatrix rotmat;
	auto q = AngleAxisToQuaternion(m_RotAxis, fRotSpeed * fElapsedTime);
	rotmat = QuaternionToMatrix(q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponGrenade::Explosion()
{
	rvector v = m_Position;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	ZGetEffectManager()->AddGrenadeEffect(v, dir);

	g_pGame->OnExplosionGrenade(m_uidOwner, v, m_fDamage, 400.f, .2f, 1.f, m_nTeamID);

	ZGetSoundEngine()->PlaySound("we_grenade_explosion", v);

	ZGetWorld()->GetFlags()->SetExplosion(v, EXPLOSION_EMBLEM_POWER);
}

MImplementRTTI(ZWeaponFlashBang, ZWeaponGrenade);

#define FLASHBANG_LIFE	3
#define FLASHBANG_DISTANCE	2000

void ZWeaponFlashBang::Explosion()
{
	rvector dir = RCameraPosition - m_Position;
	float dist = Magnitude(dir);
	Normalize(dir);
	RBSPPICKINFO pick;

	if (dist > FLASHBANG_DISTANCE)
	{
		return;
	}

	if (g_pGame->m_pMyCharacter->IsDie())
	{
		return;
	}

	if (!ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position, dir, &pick))
	{
		mbIsLineOfSight = true;
	}
	else
	{
		auto vec = pick.PickPos - m_Position;
		float distMap	= MagnitudeSq(vec);
		rvector temp = g_pGame->m_pMyCharacter->m_Position - m_Position;
		float distChar	= MagnitudeSq(temp);
		if( distMap > distChar )
		{
			mbIsLineOfSight	= true;
		}
		else
		{
			mbIsLineOfSight	= false;
		}
	}

	if( !mbIsExplosion && mbIsLineOfSight )
	{
		rvector pos		= g_pGame->m_pMyCharacter->GetPosition();
		rvector dir		= g_pGame->m_pMyCharacter->GetTargetDir();
		mbIsExplosion	= true;
		CreateFlashBangEffect( m_Position, pos, dir, 10 );
	}
	ZGetSoundEngine()->PlaySound("we_flashbang_explosion",m_Position);
}

bool	ZWeaponFlashBang::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;

	float lap	= g_pGame->GetTime() - m_fStartTime;

	if( lap >= FLASHBANG_LIFE )
	{
		Explosion();
		return false;
	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= g_pGame->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );

		if( bPicked )
		{
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA = RANDOMFLOAT * TAU;
			float fB = RANDOMFLOAT * TAU;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
		mRotVelocity	= std::min( Magnitude( m_Velocity ), static_cast<float>(MAX_ROT_VELOCITY) );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			m_Up	= rvector( 0, 1, 0 );
			auto right = CrossProduct(m_Dir, m_Up);
			m_Dir = CrossProduct(right, m_Up);
			GetIdentityMatrix(mRotMatrix );
		}
		else
		{
			rmatrix	Temp;
			Temp = RotationMatrix(m_RotAxis, mRotVelocity * 0.001f);
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp = RGetRotXRad(mRotVelocity * 0.001f);
		mRotMatrix	= mRotMatrix * Temp;
		mRotVelocity	*= 0.97f;
	}

	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat		= mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position	, ROCKET_SPEAR_EMBLEM_POWER );

	return true;
}

MImplementRTTI(ZWeaponSmokeGrenade,ZWeaponGrenade);

#define SMOKE_GRENADE_LIFETIME 30
#define SMOKE_GRENADE_EXPLOSION	3

const float ZWeaponSmokeGrenade::mcfTrigerTimeList[NUM_SMOKE] =
{
	0.f, 0.5f, 1.f, 1.7f, 2.3f, 2.5f, 3.f
};

bool ZWeaponSmokeGrenade::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;
	float lap	= g_pGame->GetTime() - m_fStartTime;

	if( lap >= SMOKE_GRENADE_LIFETIME )
	{
		return false;
	}
	
	if( miSmokeIndex < NUM_SMOKE && lap - SMOKE_GRENADE_EXPLOSION >= mcfTrigerTimeList[miSmokeIndex] )
	{
		Explosion();
		++miSmokeIndex;
	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= g_pGame->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );
		
		if( bPicked )
		{
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA = RANDOMFLOAT * TAU;
			float fB = RANDOMFLOAT * TAU;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
 		mRotVelocity	= std::min( Magnitude( m_Velocity ), static_cast<float>(MAX_ROT_VELOCITY) );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			m_Up	= rvector( 0, 1, 0 );
			auto right = CrossProduct(m_Dir, m_Up);
			m_Dir = CrossProduct(right, m_Up);
			GetIdentityMatrix(mRotMatrix);
		}
		else
		{
			rmatrix	Temp = RotationMatrix(m_RotAxis, mRotVelocity * 0.001f);
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp = RGetRotXRad(mRotVelocity * 0.001f);
		mRotMatrix	= mRotMatrix * Temp;
 		mRotVelocity	*= 0.97f;
	}
	
	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat		= mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );
	
	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponSmokeGrenade::Explosion()
{
	ZGetEffectManager()->AddSmokeGrenadeEffect( m_Position );
	mRotVelocity	*= 10;

	ZGetSoundEngine()->PlaySound("we_gasgrenade_explosion",m_Position);
}

void ZWeaponMagic::Create(RMesh* pMesh, ZSkill* pSkill, const rvector &pos, const rvector &dir, float fMagicScale,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_fMagicScale = fMagicScale;

	m_pVMesh->SetAnimation("play");

	m_pSkillDesc = pSkill->GetDesc();

	m_Position=pos;

	if (m_pSkillDesc) m_Velocity=dir * m_pSkillDesc->fVelocity;
	else m_Velocity=dir*ROCKET_VELOCITY;
	

	m_fStartTime = g_pGame->GetTime();
	m_fLastAddTime = g_pGame->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fDamage = pSkill->GetDesc()->nModDamage;

	m_uidTarget = pSkill->GetTarget();
	m_bGuide = pSkill->GetDesc()->bGuidable;
	
	if( Z_VIDEO_DYNAMICLIGHT ) {
		_ASSERT( m_SLSid == 0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}
}

#define MAGIC_WEAPON_LIFE			10.f


bool ZWeaponMagic::Update(float fElapsedTime)
{
	if(m_bGuide) {
		ZObject *pTarget = ZGetObjectManager()->GetObject(m_uidTarget);

		float fCurrentSpeed = Magnitude(m_Velocity);
		rvector currentDir = m_Velocity;
		Normalize(currentDir);

		rvector dir = (pTarget->GetPosition()+rvector(0,0,100)) - m_Position;
		Normalize(dir);

		float fCos = DotProduct(dir,currentDir);
		float fAngle = acos(fCos);
		if (fAngle > 0.01f) {

#define ANGULAR_VELOCITY	0.01f
			float fAngleDiff = min(1000.f * fElapsedTime * ANGULAR_VELOCITY, fAngle);

			rvector newDir = Slerp(m_Dir, dir, fAngleDiff / fAngle);
			m_Dir = newDir;

			m_Velocity = fCurrentSpeed * newDir;
			_ASSERT(!_isnan(m_Velocity.x) &&!_isnan(m_Velocity.y) && !_isnan(m_Velocity.z) );
		}
	}

	rvector oldPos = m_Position;

	if(g_pGame->GetTime() - m_fStartTime > MAGIC_WEAPON_LIFE ) {
		Explosion( WMET_MAP, NULL , rvector(0,1,0));

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos;
		rvector pickdir;

		ZPICKINFO zpi;
		WeaponMagicExplosionType type = WMET_MAP;

		ZObject* pOwnerObject = ZGetObjectManager()->GetObject(m_uidOwner);
		ZObject* pPickObject = NULL;

		bool bPicked=g_pGame->Pick(pOwnerObject, m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked) {
				pickpos=zpi.bpi.PickPos;
				pickdir.x=zpi.bpi.pInfo->plane.a;
				pickdir.y=zpi.bpi.pInfo->plane.b;			
				pickdir.z=zpi.bpi.pInfo->plane.c;
				Normalize(pickdir);
			}	
			else
				if(zpi.pObject) {
					pPickObject = zpi.pObject;
					pickpos=zpi.info.vOut;
					type = WMET_OBJECT;
				}

		}

		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			Explosion( type, pPickObject,pickdir);

			if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
			{
				ZGetStencilLight()->DeleteLightSource( m_SLSid );
				m_SLSid = 0;
				ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
			}

			return false;
		}
		else
		{
			rvector to = m_Position + diff;

			if (g_pGame->ObjectColTest(pOwnerObject, m_Position, to, m_pSkillDesc->fColRadius, &pPickObject))
			{
				pickdir = to - m_Position;
				Normalize(pickdir);

				Explosion( WMET_OBJECT, pPickObject,pickdir);

				if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
				{
					ZGetStencilLight()->DeleteLightSource( m_SLSid );
					m_SLSid = 0;
					ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
				}
				
				return false;
			}
			else
			{
				m_Position+=diff;
			}
		}
	}

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetScale(rvector(m_fMagicScale,m_fMagicScale,m_fMagicScale));
	m_pVMesh->SetWorldMatrix(mat);

	float this_time = g_pGame->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

		ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

		nEffectType = m_pSkillDesc->nTrailEffectType;

		if (m_pSkillDesc->bDrawTrack)
		{
				if(nEffectType==ZSTE_FIRE)		ZGetEffectManager()->AddTrackFire(pos);
			else if(nEffectType==ZSTE_COLD)		ZGetEffectManager()->AddTrackCold(pos);
			else if(nEffectType==ZSTE_MAGIC)	ZGetEffectManager()->AddTrackMagic(pos);
		}

		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponMagic::Render()
{
	ZWeapon::Render();

#ifndef _PUBLISH
	if ((ZApplication::GetGameInterface()->GetScreenDebugger()->IsVisible()) && (ZIsLaunchDevelop()))
	{
		if (m_pSkillDesc)
		{
			RDrawSphere(m_Position, m_pSkillDesc->fColRadius, 10);
		}
	}
#endif

}

void ZWeaponMagic::Explosion(WeaponMagicExplosionType type, ZObject* pVictim, const rvector& vDir)
{
	rvector v = m_Position-rvector(0,0,100.f);

#define MAGIC_MINIMUM_DAMAGE	0.2f
#define MAGIC_KNOCKBACK_CONST   .3f	

	if (!m_pSkillDesc->IsAreaTarget())
	{
		if ((pVictim) && (type == WMET_OBJECT))
		{
			g_pGame->OnExplosionMagicNonSplash(this, m_uidOwner, pVictim->GetUID(), v, m_pSkillDesc->fModKnockback);
		}
	}
	else
	{
		g_pGame->OnExplosionMagic(this,m_uidOwner,v,MAGIC_MINIMUM_DAMAGE,m_pSkillDesc->fModKnockback, m_nTeamID, true);
	}

	ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

	nEffectType = m_pSkillDesc->nTrailEffectType;

	rvector pos = m_Position;

	if(type == WMET_OBJECT) {
		float fScale = m_fMagicScale;
		
		if(nEffectType==ZSTE_FIRE)			
		{
			if (fScale > 1.0f)
			{
				fScale = 1.0f + fScale*0.2f;
				pos -= rvector(0.0f, 0.0f, fScale * 100.0f);
			}

			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_FIRE,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_COLD)		
		{
			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_COLD,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_MAGIC)	
		{
			ZGetEffectManager()->AddMagicEffect(m_Position,0, fScale);
		}
	}
	else {

		 if(nEffectType==ZSTE_FIRE)		 ZGetEffectManager()->AddMagicEffectWall(0,pos,vDir,0, m_fMagicScale);
		 else if(nEffectType==ZSTE_COLD) ZGetEffectManager()->AddMagicEffectWall(1,pos,vDir,0, m_fMagicScale);
		 else if(nEffectType==ZSTE_MAGIC)ZGetEffectManager()->AddMagicEffectWall(2,pos,vDir,0, m_fMagicScale);
	}

	if (m_pSkillDesc->szExplosionSound[0] != 0)
	{
		ZGetSoundEngine()->PlaySound(m_pSkillDesc->szExplosionSound, v);
	}

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );

}

