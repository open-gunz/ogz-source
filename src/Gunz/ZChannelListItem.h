#ifndef _ZCHANNELLISTITEM_H
#define _ZCHANNELLISTITEM_H


#include "MListBox.h"
class ZChannelListItem : public MListItem {
	MUID			m_uidChannel;
	char			m_szChannelName[64];
	
	int				m_nChannelType;
	int				m_nChannelNumber;
	int				m_nPlayers,m_nMaxPlayers;

	char			m_szItemString[256];
public:
	ZChannelListItem(const MUID& uid, const int nChannelNumber, const char* szChannelName, 
		const int nChannelType, const int nPlayers, const int nMaxPlayers) 
	{ 
		m_uidChannel = uid; strcpy_safe(m_szChannelName, szChannelName); 
		m_nChannelType = nChannelType;
		m_nChannelNumber = nChannelNumber; 
		m_nPlayers = nPlayers;
		m_nMaxPlayers = nMaxPlayers;
	}
	virtual ~ZChannelListItem()			{}
	virtual const char* GetString(void)	{ 
		sprintf_safe(m_szItemString,"%s (%d/%d)",m_szChannelName,m_nPlayers,m_nMaxPlayers);
		return m_szItemString; 
	}
	MUID GetUID()						{ return m_uidChannel; }
	char* GetName()						{ return m_szChannelName; }
};




#endif