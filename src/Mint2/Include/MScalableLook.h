#ifndef _MSCALABLELOOK_H
#define _MSCALABLELOOK_H

#include "Mint.h"

#define DEFAULT_BASIC_WIDTH	1600

class MScalableLook {

public:

	MScalableLook() { m_bScale = false; m_nBasicWidth = DEFAULT_BASIC_WIDTH; }

	bool		m_bScale;		// 해상도에 따라 (프레임을) 스케일한다
	int			m_nBasicWidth;	// 기준 해상도 Width

	void SetScaleEnable(bool b) {
		m_bScale = b;
	}

	void SetBasicWidth(int n) {
		m_nBasicWidth = n;
	}


	// 기준 해상도에 대한 현재해상도의 비율
	float GetScale() { 
		return m_bScale ? 
			(float)MGetWorkspaceWidth()/(float)m_nBasicWidth : 
			1.f ; 
	}
};

#endif