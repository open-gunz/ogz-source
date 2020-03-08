#include "stdafx.h"
#include "MResourceManager.h"

// MBitmapManager Implementation
//////////////////////////////
CMLinkedList<MBitmap> MBitmapManager::m_Bitmaps;
CMLinkedList<MAniBitmap> MBitmapManager::m_AniBitmaps;

void MBitmapManager::Destroy(void)
{
	for(int i=0; i<m_Bitmaps.GetCount(); i++){
		MBitmap* pBitmap = m_Bitmaps.Get(i);
		pBitmap->Destroy();
	}
	m_Bitmaps.DeleteAll();
}

void MBitmapManager::DestroyAniBitmap(void)
{
	for(int i=0; i<m_AniBitmaps.GetCount(); i++){
		MAniBitmap* pAniBitmap = m_AniBitmaps.Get(i);
		pAniBitmap->Destroy();
	}
	m_AniBitmaps.DeleteAll();
}

void MBitmapManager::Add(MBitmap* pBitmap)
{
	m_Bitmaps.Add(pBitmap);
}

void MBitmapManager::Add(MAniBitmap* pAniBitmap)
{
	m_AniBitmaps.Add(pAniBitmap);
}

void MBitmapManager::Delete(const char* szName)
{
	for(int i=0; i<m_Bitmaps.GetCount(); i++){
		MBitmap* pBitmap = m_Bitmaps.Get(i);
		if(strcmp(pBitmap->m_szName, szName)==0){
			m_Bitmaps.Delete(i);
			return;
		}
	}
	for(int i=0; i<m_AniBitmaps.GetCount(); i++){
		MAniBitmap* pAniBitmap = m_AniBitmaps.Get(i);
		if(strcmp(pAniBitmap->m_szName, szName)==0){
			m_AniBitmaps.Delete(i);
			return;
		}
	}
}

MBitmap* MBitmapManager::Get(const char* szBitmapName)
{
	if(szBitmapName==NULL)	// Default Font
		if(m_Bitmaps.GetCount()>0)
			return m_Bitmaps.Get(0);

	for(int i=0; i<m_Bitmaps.GetCount(); i++){
		MBitmap* pBitmap = m_Bitmaps.Get(i);
		if(_stricmp(pBitmap->m_szName, szBitmapName)==0) return pBitmap;
	}

	return NULL;
}

MBitmap* MBitmapManager::Get(int i)
{
	return m_Bitmaps.Get(i);
}

int MBitmapManager::GetCount(void)
{
	return m_Bitmaps.GetCount();
}

MAniBitmap* MBitmapManager::GetAniBitmap(const char* szBitmapName)
{
	if(szBitmapName==NULL)	// Default Font
		if(m_AniBitmaps.GetCount()>0)
			return m_AniBitmaps.Get(0);

	for(int i=0; i<m_AniBitmaps.GetCount(); i++){
		MAniBitmap* pBitmap = m_AniBitmaps.Get(i);
		if(strcmp(pBitmap->m_szName, szBitmapName)==0) return pBitmap;
	}

	return NULL;
}


// MFontManager Implementation
//////////////////////////////
//CMLinkedList<MFont> MFontManager::m_Fonts;
map<string,MFont*>	MFontManager::m_Fonts;
MFont *MFontManager::m_pDefaultFont=NULL;

void MFontManager::SetDefaultFont(MFont *pFont)
{
	m_pDefaultFont=pFont;
}

void MFontManager::Destroy(void)
{
	/*
	for(int i=0; i<m_Fonts.GetCount(); i++){
		MFont* pFont = m_Fonts.Get(i);
		pFont->Destroy();
	}
	m_Fonts.DeleteAll();
	*/

	while(!m_Fonts.empty())
	{
		delete m_Fonts.begin()->second;
		m_Fonts.erase(m_Fonts.begin());
	}
}

void MFontManager::Add(MFont* pFont)
{
//	m_Fonts.Add(pFont);

	map<string,MFont*>::iterator i=m_Fonts.find(string(pFont->m_szName));
	if(i!=m_Fonts.end())
	{
		delete i->second;
		m_Fonts.erase(i);
	}

	m_Fonts.insert(map<string,MFont*>::value_type(string(pFont->m_szName),pFont));
}

MFont* MFontManager::Get(const char* szFontName)
{
	if(szFontName)
	{
		map<string,MFont*>::iterator i=m_Fonts.find(string(szFontName));
		if(i!=m_Fonts.end())
		{
			return i->second;
		}
	}

	if(m_pDefaultFont) return m_pDefaultFont;

	return NULL;

	/*
	if(szFontName==NULL)	// Default Font
		if(m_Fonts.GetCount()>0)
			return m_Fonts.Get(0);

	for(int i=0; i<m_Fonts.GetCount(); i++){
		MFont* pFont = m_Fonts.Get(i);
		if(strcmp(pFont->m_szName, szFontName)==0) return pFont;
	}

	return NULL;
	*/
}

/*
MFont* MFontManager::Get(int i)
{
	if(i<0 || i>=m_Fonts.GetCount()) return NULL;
	return m_Fonts.Get(i);
}
*/

int MFontManager::GetCount(void)
{
//	return m_Fonts.GetCount();
	return m_Fonts.size();
}

MCursor* MCursorSystem::m_pCursor;
CMLinkedList<MCursor> MCursorSystem::m_Cursors;
bool MCursorSystem::m_bShow = true;

void MCursorSystem::Destroy(void)
{
	m_Cursors.DeleteAll();
	m_pCursor=NULL;
}

MCursor* MCursorSystem::Set(MCursor* pCursor)
{
	MCursor* temp = m_pCursor;
	m_pCursor = pCursor;
	return temp;
}

MCursor* MCursorSystem::Set(const char* szName)
{
	if(szName==NULL){
		//...
	}
	MCursor* pCursor = Get(szName);
	if(pCursor!=NULL) return Set(pCursor);
	return NULL;
}

MCursor* MCursorSystem::Get(void)
{
	return m_pCursor;
}

MCursor* MCursorSystem::Get(const char* szName)
{
	for(int i=0; i<m_Cursors.GetCount(); i++){
		MCursor* pCursor = m_Cursors.Get(i);
		if(strcmp(pCursor->m_szName, szName)==0) return pCursor;
	}
	return NULL;
}

void MCursorSystem::Add(MCursor* pCursor)
{
	m_Cursors.Add(pCursor);
}

void MCursorSystem::Draw(MDrawContext* pDC, int x, int y)
{
	if(m_bShow==false) return;
	if(m_pCursor!=NULL) m_pCursor->Draw(pDC, x, y);
}

void MCursorSystem::Show(bool bShow)
{
	m_bShow = bShow;

	if (Get() == nullptr)
		Set("");
}

bool MCursorSystem::IsVisible(void)
{
	return m_bShow;
}
