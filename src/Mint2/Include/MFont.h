#pragma once

#include "MTypes.h"

#define MFONT_NAME_LENGTH		32

class MFont {
public:
#ifdef _DEBUG
	int		m_nTypeID;
#endif

public:
	char	m_szName[MFONT_NAME_LENGTH];

	int				m_nOutlineStyle;
	u32	m_ColorArg1;	// For Custom FontStyle like OutlineStyle
	u32	m_ColorArg2;

public:
	MFont();
	virtual ~MFont();
	virtual bool Create(const char* szName);
	virtual void Destroy();
	virtual int GetHeight() = 0;
	virtual int GetWidth(const char* szText, int nSize=-1) = 0;
	virtual int GetWidthWithoutAmpersand(const char* szText, int nSize=-1);
};