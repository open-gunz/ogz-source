#pragma	once

#include "RTypes.h"
#include "RMesh.h"
#include "RVisualMeshMgr.h"
#include "MRTTI.h"

class RealSoundEffectPlay;

_USING_NAMESPACE_REALSPACE2

class ZObject;

// rtti 까지는 필요없고..단순히 구분할것들

enum ZWeaponType {
	ZWeaponType_None = 0,

	ZWeaponType_MovingWeapon,

	ZWeaponType_End
};

class ZWeapon {
public:
	MDeclareRootRTTI;

	ZWeapon();
	virtual ~ZWeapon();

	virtual void Create(RMesh* pMesh);
	virtual void Render();
	virtual bool Update(float fElapsedTime);

	int	 GetWeaponType()		{ return m_WeaponType; }
	void SetItemUID(int nUID)	{ m_nItemUID = nUID; }
	int  GetItemUID()			{ return m_nItemUID; }

public:
	RVisualMesh* m_pVMesh;

	int			m_nWorldItemID;

protected:
	int			m_WeaponType;
	MUID		m_uidOwner;
	MMatchTeam	m_nTeamID;
	float		m_fDamage;
	int			m_SLSid; // 스텐실 라이트 아이디
	int			m_nItemUID;
};

class ZMovingWeapon : public ZWeapon {
public:
	MDeclareRTTI;

	ZMovingWeapon();
	~ZMovingWeapon();

	void Create(RMesh* pMesh, const rvector &pos, const rvector &dir,ZObject* pOwner);

	virtual void Explosion();

public:
	rvector m_Position;
	rvector m_Velocity;
	rvector m_Dir,m_Up;
	rvector	m_PostPos;
};

class ZEffectRocket;

class ZWeaponRocket : public ZMovingWeapon {
public:
	MDeclareRTTI;

	ZWeaponRocket();
	~ZWeaponRocket();

	void Create(RMesh* pMesh, const rvector &pos, const rvector &dir,ZObject* pOwner);
	void Render();
	void Explosion();
	
	bool Update(float fElapsedTime);

	float	m_fStartTime;
	float	m_fLastAddTime;
};

class ZWeaponItemkit : public ZMovingWeapon { 
public:
	MDeclareRTTI;

	ZWeaponItemkit();
	~ZWeaponItemkit();

	void Create(RMesh* pMesh, const rvector &pos, const rvector &velocity,ZObject* pOwner);

	virtual void Explosion();
	
	virtual bool Update(float fElapsedTime);
	virtual void Render();

private:

	void UpdateFirstPos();
	void UpdatePost(DWORD dwPickPassFlag);
	void UpdatePos(float fElapsedTime,DWORD dwPickPassFlag);

public:
	bool	m_bInit;
	bool	m_bSendMsg;
	int		m_nWorldItemID;
	float	m_fStartTime;
	float	m_fDelayTime;

	float	m_fDeathTime;
	bool	m_bDeath;
	rvector m_RotAxis;
//	int		m_nSpItemID;
	
//	static int m_nMySpItemID;
};

class ZWeaponGrenade : public ZMovingWeapon { 
public:
	MDeclareRTTI;

	ZWeaponGrenade() : ZMovingWeapon() { }

	void Create(RMesh* pMesh, const rvector &pos, const rvector &velocity,ZObject* pOwner);

	virtual void Explosion();

	virtual bool Update(float fElapsedTime);

public:

	int		m_nSoundCount;
	float	m_fStartTime;
	
	rvector m_RotAxis;
};

class ZWeaponFlashBang : public ZWeaponGrenade 
{
public:
	MDeclareRTTI;

	ZWeaponFlashBang::ZWeaponFlashBang() : ZWeaponGrenade(), mRotVelocity(0.0f), mbLand(false), mbIsExplosion(false)
	{
		GetIdentityMatrix(mRotMatrix );
	}


	bool	mbIsExplosion;
	bool	mbIsLineOfSight;
	bool	mbLand;
	float	mRotVelocity;
	rmatrix	mRotMatrix;	
	
public:
	bool	Update( float fElapsedTime );
	void	Explosion();
};

#define NUM_SMOKE	7
class ZWeaponSmokeGrenade : public ZWeaponGrenade
{
protected:
	MDeclareRTTI;

	ZWeaponSmokeGrenade() : ZWeaponGrenade(), miSmokeIndex(0), mRotVelocity(0.0f), mbLand(false) 
	{
		GetIdentityMatrix(mRotMatrix );
	};

	static const float mcfTrigerTimeList[NUM_SMOKE];
	int		miSmokeIndex;
	bool	mbLand;
	float	mRotVelocity;
	rmatrix	mRotMatrix;					
public:
	bool	Update( float fElapsedTime );
	void	Explosion();
};

class ZSkill;
class ZSkillDesc;

enum WeaponMagicExplosionType
{
	WMET_MAP = 0,
	WMET_OBJECT,
	WMET_END
};

class ZWeaponMagic : public ZMovingWeapon {
	MUID		m_uidTarget;
	bool		m_bGuide;
	ZSkillDesc	*m_pSkillDesc;
	float		m_fMagicScale;
public:
	ZWeaponMagic() : ZMovingWeapon() { m_fMagicScale = 1.f; }

	void Create(RMesh* pMesh, ZSkill* pSkill, const rvector &pos, const rvector &dir,float fMagicScale, ZObject* pOwner);

	void Render();
	void Explosion(WeaponMagicExplosionType type, ZObject* pVictim, const rvector& vDir);
	
	bool Update(float fElapsedTime);

	const MUID& GetTarget() { return m_uidTarget; }
	ZSkillDesc *GetDesc() { return m_pSkillDesc; }

	float	m_fStartTime;
	float	m_fLastAddTime;
};