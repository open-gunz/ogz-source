#ifndef _ZEffectAniMesh_h
#define _ZEffectAniMesh_h

#include "ZEffectManager.h"
#include "RVisualMeshMgr.h"

#include "mempool.h"

class ZEffectAniMesh : public ZEffect{
protected:
	RealSpace2::RVisualMesh m_VMesh;

	rvector	m_Scale;
	rvector	m_Pos;
	rvector	m_Dir,m_DirOrg;
	rvector	m_Up;

	float	m_nStartTime;
	float	m_fRotateAngle;
	int		m_nAlignType;

	float	m_nStartAddTime;

	int		m_nLifeTime;

	MUID	m_uid;
	bool	m_bDelay;
	bool	m_isCheck;
	bool	m_bLoopType;
	
	rvector m_vBackupPos;

public:

	ZEffectAutoAddType	m_nAutoAddEffect;

public:
	ZEffectAniMesh(RMesh* pMesh, const rvector& Pos, const rvector& Dir);
	virtual bool Draw(u64 nTime) override;

	void CheckCross(rvector& Dir,rvector& Up);

	RVisualMesh* GetVMesh();

	virtual void SetUpVector(rvector& v);

	void SetUid(MUID uid);
	MUID GetUID() { return m_uid; }
	void SetDelayPos(MUID id);
	void SetScale(rvector s);
	void SetRotationAngle(float a);
	void SetAlignType(int type);
	void SetStartTime(DWORD _time);
	void SetLifeTime(int nLifeTime) { m_nLifeTime = nLifeTime; }
	virtual rvector GetSortPos() override;

};

class ZEffectSlash : public ZEffectAniMesh , public CMemPoolSm<ZEffectSlash> {
	MUID UID;
public:
	ZEffectSlash(RMesh* pMesh, const rvector& Pos, const rvector& Dir);
};

class ZEffectDash : public ZEffectAniMesh , public CMemPoolSm<ZEffectDash> {
public:
	ZEffectDash(RMesh* pMesh, const rvector& Pos, const rvector& Dir,MUID uidTarget);
	virtual bool Draw(u64 nTime) override;
};

class ZEffectLevelUp : public ZEffectAniMesh , public CMemPoolSm<ZEffectLevelUp> {
public:
	ZEffectLevelUp(RMesh* pMesh, const rvector& Pos, const rvector& Dir, const rvector& AddPos,ZObject* pObj);
	virtual bool Draw(u64 nTime) override;

	RMeshPartsPosInfoType m_type;
	rvector m_vAddPos;
};

class ZEffectPartsTypePos : public ZEffectAniMesh , public CMemPoolSm<ZEffectPartsTypePos> {
public:
	ZEffectPartsTypePos(RMesh* pMesh, const rvector& Pos, const rvector& Dir, const rvector& AddPos,ZObject* pObj);
	virtual bool Draw(u64 nTime) override;

	RMeshPartsPosInfoType m_type;
	rvector m_vAddPos;
};

class ZEffectWeaponEnchant : public ZEffectAniMesh , public CMemPoolSm<ZEffectWeaponEnchant> {
public:
	ZEffectWeaponEnchant(RMesh* pMesh, const rvector& Pos, const rvector& Dir, ZObject* pObj);
	virtual bool Draw(u64 nTime) override;

};

class ZEffectIcon : public ZEffectAniMesh {
public:
	ZEffectIcon(RMesh* pMesh, ZObject* pObj);
	virtual bool Draw(u64 nTime) override;
	RMeshPartsPosInfoType m_type;
};

class ZEffectShot : public ZEffectAniMesh , public CMemPoolSm<ZEffectShot> {
public:
	ZEffectShot(RMesh* pMesh, const rvector& Pos, const rvector& Dir,ZObject* pObj);
	virtual bool Draw(u64 nTime) override;

	void SetStartTime(DWORD _time) {
		m_nStartAddTime = _time;
	}

	void SetIsLeftWeapon(bool b) {
		m_isLeftWeapon = b;
	}

public:
//	MUID		m_uid;
//	ZCharacter* m_pCharacter;

	float		m_nStartAddTime;
	bool		m_isMovingPos;
	MUID		m_uid;
	bool		m_isLeftWeapon;

};


class ZEffectBerserkerIconLoop : public ZEffectIcon, public CMemPoolSm<ZEffectBerserkerIconLoop> {
private:
	unsigned int m_nElapsedTime;
public:
	ZEffectBerserkerIconLoop(RMesh* pMesh, ZObject* pObj);
	virtual bool Draw(u64 nTime) override;
};


#endif//_ZEffectAniMesh_h