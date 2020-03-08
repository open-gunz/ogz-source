#include "stdafx.h"
#include "MDragObject.h"
#include <string.h>

MDragObject::MDragObject(int nID, const char* szString, MBitmap* pBitmap)
{
	m_nID = nID;
	strcpy_safe(m_szString, szString);
	m_pBitmap = pBitmap;
}

MDragObject::~MDragObject(void)
{
}
