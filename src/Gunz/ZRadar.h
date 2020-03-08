#ifndef _ZRADAR_H
#define _ZRADAR_H

#include "ZInterface.h"
#include "MPicture.h"

struct ZRadarNode
{
	bool			bShoted;
	u32	nLastShotTime;
//	float			fRot;
	int				x[4];
	int				y[4];
};
class ZRadar : public ZInterface
{
protected:
	MBitmapR2*		m_pBitmap;
	float			m_fMaxDistance;

	ZRadarNode		m_Nodes[8];
	void RotatePixel(int* poutX, int* poutY, int sx, int sy, int nHotSpotX, int nHotSpotY, float fAngle);
public:
	ZRadar(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZRadar();
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(MDrawContext* pDC);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	void OnAttack(rvector& pAttackerPos);

	void SetMaxDistance(float fDist);
	float GetMaxDistance() { return m_fMaxDistance; }
};

#endif