#ifndef MDRAGOBJECT_H
#define MDRAGOBJECT_H

#define MDRAGOBJECT_STRING_LENGTH	256

class MBitmap;

class MDragObject{
	int			m_nID;
	char		m_szString[MDRAGOBJECT_STRING_LENGTH];
	MBitmap*	m_pBitmap;

public:
	MDragObject(int nID, const char* szString, MBitmap* pBitmap);
	virtual ~MDragObject(void);

	int GetID(void){
		return m_nID;
	}
	const char* GetString(void){
		return m_szString;
	}
	MBitmap* GetBitmap(void){
		return m_pBitmap;
	}
};

#endif