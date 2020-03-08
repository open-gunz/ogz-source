#pragma once

#include "RTypes.h"

_USING_NAMESPACE_REALSPACE2

#define CAMERA_DEFAULT_DISTANCE		290.f

#define CAMERA_DEFAULT_ANGLEX		(PI_FLOAT/2.f)
#define CAMERA_DEFAULT_ANGLEZ		(PI_FLOAT/4.f)

#define CAMERA_ANGLEX_MIN	0.01f
#define CAMERA_ANGLEX_MAX	(PI_FLOAT-0.01f)

#define CAMERA_DIST_MAX		1000.f
#define CAMERA_DIST_MIN		150.f

enum ZCAMERALOOKMODE
{
	ZCAMERA_DEFAULT		= 0,
	ZCAMERA_FREEANGLE	= 1,
	ZCAMERA_FREELOOK	= 2,
	ZCAMERA_MINIMAP		= 3,
	
	ZCAMERA_END
};

class ZCamera {
public:
	ZCamera();

	void Update(float fElapsed);

	bool m_bAutoAiming;

	auto& GetPosition() const { return m_Position; }
	v3 GetCurrentDir();
	void SetPosition(const v3 &pos) { m_Position = pos; }
	void SetDirection(const v3& dir);

	void Shock(float fPower, float fDuration, const v3& vDir);
	void StopShock();
	void Init();

	auto GetLookMode() const { return m_nLookMode; }
	void SetLookMode(ZCAMERALOOKMODE mode);
	void SetNextLookMode();

	float GetAngleX() const { return m_fAngleX; }
	float GetAngleZ() const { return m_fAngleZ; }

	static v3 GetTargetOffset(const v3& Dir, float Scale = 1);

	ZCharacter* TargetCharacterOverride{};

private:
	friend class Portal;
	friend class ZGameInput;
	friend void LoadRGCommands(class ZChatCmdManager& CmdManager);

	bool CheckCollisionWall(float &fRealDist, v3& pos, v3& dir);
	void CalcMaxPayneCameraZ(float &fRealDist, float& fAddedZ, float fAngleX);

	ZCAMERALOOKMODE m_nLookMode;

	bool m_bShocked;
	float m_fShockStartTime;
	float m_fShockPower;
	float m_fShockDuration;

	float m_fDist;

	// Pitch
	float m_fAngleX;
	// Yaw
	float m_fAngleZ;

	float m_fCurrentDist;

	v3 m_Position;
	v3 m_Target;
	v3 m_CurrentTarget;
};