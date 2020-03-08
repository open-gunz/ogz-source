#ifndef ZActionKey_H
#define ZActionKey_H

#include "MEdit.h"
#include <set>

using namespace std;

typedef set<int> ActionKeyForbidSet;

class ZActionKeyLook;

/// ActionKey widget
// 쓰이는 모든 key 값은 ZVIRTUALKEY 값이나, header include 하기 싫어서 걍 int 로 씀
class ZActionKey : public MEdit{
	friend ZActionKeyLook;
protected:
	DECLARE_LOOK(ZActionKeyLook)
//	DECLARE_LOOK_CLIENT()

protected:
	bool				m_bReadyInput;
	int					m_nKey;
	int					m_nAltKey;	// 한 액션에 2개의 키를 맵핑할수 있다

public:

	static ActionKeyForbidSet m_ForbidKey;

protected:
//	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual void OnSetFocus(void);
	virtual void OnReleaseFocus(void);

	static MGLOBALEVENTCALLBACK*	m_oldEventListener;
	static ZActionKey*				m_exclusiveActionKey;

public:
	ZActionKey(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL );

#define MINT_ACTIONKEY	"ActionKey"
	virtual const char* GetClassName(void){ return MINT_ACTIONKEY; }

	static bool		OnExclusiveEvent(MEvent* pEvent);

//	void GetActionKeyName(char* szActionKeyName);
	void GetActionKey(int* pKey);
	void GetActionAltKey(int* pKey);

	void ClearActionKey();
	void SetActionKey(int nKey);
	bool DeleteActionKey(int nKey);

	void UpdateText();	// text 내용을 갱신한다

	//bool RegisterActionKey(int nActionID);
	//bool UnregisterActionKey(void);
	
	static void RegisterForbidKey(int nkey);
	bool IsForbidKey(int nkey);

	virtual bool OnEvent(MEvent* pEvent, MListener*) override;
};


#endif