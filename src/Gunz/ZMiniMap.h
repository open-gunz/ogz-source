#ifndef _ZMINIMAP_H
#define _ZMINIMAP_H

#include "RBaseTexture.h"

_USING_NAMESPACE_REALSPACE2

class MDrawContext;

class ZMiniMap {

	rvector			m_LeftTop;
	rvector			m_RightBottom;
	RBaseTexture	*m_pBaseTexture;
	RBaseTexture	*m_pTeamColorTexture;
	RBaseTexture	*m_pPlayerTexture;
	float			m_fCameraHeightMin;
	float			m_fCameraHeightMax;

public:

	ZMiniMap();
	~ZMiniMap();

	void Destroy();

	bool Create(const char *szName);

	virtual void OnDraw(MDrawContext* pDC);

	float GetHeightMin() { return m_fCameraHeightMin; }
	float GetHeightMax() { return m_fCameraHeightMax; }
};


#endif