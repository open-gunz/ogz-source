#ifndef MHOTKEY_H
#define MHOTKEY_H

#include "MEdit.h"

/// HotKey
class MHotKey : public MEdit{
protected:
	DECLARE_LOOK(MEditLook)	// Edit Look을 그대로 쓴다.
	DECLARE_LOOK_CLIENT()

protected:
	unsigned int	m_nKey;
	bool			m_bCtrl;
	bool			m_bAlt;
	bool			m_bShift;

protected:
	virtual void OnRun(void) override;
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener) override;

public:
	MHotKey(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

#define MINT_HOTKEY	"HotKey"
	virtual const char* GetClassName(void) override { return MINT_HOTKEY; }

	template <size_t size>
	void GetHotKeyName(char (&szHotKeyName)[size])
	{
		if (m_bCtrl == true) {
			strcat_safe(szHotKeyName, "Ctrl");
		}
		if (m_bAlt == true) {
			if (szHotKeyName[0] != 0) strcat_safe(szHotKeyName, "+");
			strcat_safe(szHotKeyName, "Alt");
		}
		if (m_bShift == true) {
			if (szHotKeyName[0] != 0) strcat_safe(szHotKeyName, "+");
			strcat_safe(szHotKeyName, "Shift");
		}

		if (m_nKey>0) {
			char szKey[128];
			GetKeyName(szKey, 128, m_nKey, false);
			if (szKey[0] != 0) {
				if (szHotKeyName[0] != 0) strcat_safe(szHotKeyName, "+");
				strcat_safe(szHotKeyName, szKey);
			}
		}
	}

	void GetHotKey(unsigned int* pKey, bool* pCtrl, bool* pAlt, bool* pShift);

	int RegisterHotKey(void);
	void UnregisterHotKey(int nID);
};


#endif